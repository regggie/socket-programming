//
// Created by lam on 4/6/17.
//

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <arpa/inet.h>

#define QLEN 5 /* maximum connection queue length */
u_short portbase = 0; /* port base, for non-root servers */
void reaper(int);

int processRequest(int fd);

int errexit(const char *format, ...);

int passivesock(const char *service, const char *transport, int qlen);

int main(int argc, char *argv[]) {
    char *service = "80"; /* service name or port number */
    struct sockaddr_in fsin; /* the address of a client */
    int alen; /* length of client's address */
    int msock; /* master server socket */
    int ssock; /* slave server socket */

    msock = passivesock(service, "tcp", QLEN);
    (void) signal(SIGCHLD, reaper);
    while (1) {
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr *) &fsin, &alen);
        if (ssock < 0) {
            if (errno == EINTR)
                continue;
            errexit("ACCEPT: %s\n", strerror(errno));
        }

        switch (fork()) {
            case 0: /* child */
                (void) close(msock);
                exit(processRequest(ssock));
            default: /* parent */
                (void) close(ssock);
                break;
            case -1:
                errexit("fork: %s\n", strerror(errno));
        }
    }
}

int processRequest(int fd) {
    char buf[BUFSIZ];
    char fileBuf[BUFSIZ];
    char *reqHeader, *httpVer, *requestFile;
    int n;
    int count = 0;
    char const *filepath = "files";
    FILE *file;

    memset((void *) buf, (int) '\0', BUFSIZ);
    while (recv(fd, buf, BUFSIZ, 0) < 0) {
        printf("Fail to read from Socket");
    }

    printf(" ----- %s \n", buf);
    fflush(stdout);

    reqHeader = strtok(buf, " \n");

    if (strncmp(reqHeader, "GET\0", 4) == 0) {
        requestFile = strtok(NULL, " \t");
        httpVer = strtok(NULL, " \t\n");

        printf("ver = %s \n %s", requestFile, httpVer);
        if (strncmp(httpVer, "HTTP/1.0", 8) != 0 && strncmp(httpVer, "HTTP/1.1", 8) != 0) {
            send(fd, "HTTP/1.0 400 Bad Request\n", 25, 0);
            printf("HTTP/1.0 400 Bad Request");
        }

        char fullpath[sizeof(filepath) + sizeof(requestFile) + 1];
        strcpy(fullpath, filepath);
        strcat(fullpath, requestFile);
        strcat(fullpath, "\0");
        file = fopen(fullpath, "r");
        n = 0;

        if (file) {
            send(fd, "HTTP/1.0 200 OK\n\n", 17, 0);

            while ((n = fread(fileBuf, 1, sizeof(fileBuf) - 1, file)) > 0) {
                printf("\n %s", fileBuf);
                fflush(stdout);
                send(fd, fileBuf, n, 0);
            }

            fclose(file);
            close(fd);
        } else {
            printf("File %s does not exis\n", fullpath);
            send(fd, "HTTP/1.0 404 Not Found\n", 23, 0);
            printf("HTTP/1.0 404 Not Found\n");
        }

        close(fd);
        return 0;
    }
}

int passivesock(const char *service, const char *transport, int qlen) {
    struct servent *pse; /* pointer to service information entry */
    struct protoent *ppe; /* pointer to protocol information entry*/
    struct sockaddr_in sin; /* an Internet endpoint address */
    int s, type; /* socket descriptor and socket type */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    /* Map service name to port number */
    if (pse = getservbyname(service, transport))
        sin.sin_port = htons(ntohs((u_short) pse->s_port) + portbase);
    else if ((sin.sin_port = htons((u_short) atoi(service))) == 0)
        errexit("can't get \"%s\" service entry\n", service);

    if ((ppe = getprotobyname(transport)) == 0)
        errexit("can't get \"%s\" protocol entry\n", transport);

    if (strcmp(transport, "udp") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(errno));

    int reuse = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEPORT) failed");

    if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0)
        errexit("can't bind to %s port: %s\n", service,
                strerror(errno));

    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("can't listen on %s port: %s\n", service,
                strerror(errno));
    return s;
}

/*--------------------------------------------------HELPER--------------------------------------------------*/
void reaper(int sig) {
    int status;
    while (wait3(&status, WNOHANG, (struct rusage *) 0) >= 0)
        /* empty */;
}

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

