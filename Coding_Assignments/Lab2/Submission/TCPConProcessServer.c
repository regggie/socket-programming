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
#define BUFSIZE 4096
u_short portbase = 0; /* port base, for non-root servers */
void reaper(int);

int processRequest(int fd);

int errexit(const char *format, ...);

int passivesock(const char *service, const char *transport, int qlen);

int main(int argc, char *argv[]) {
    char *service = "9200"; /* service name or port number */
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
    int n;
    int count = 0;
    char const *filepath = "files/";
    FILE *file;

    if (read(fd, buf, BUFSIZ) < 0) {

        errexit("Fail to read from Socket");
    }



    char fullpath[sizeof(filepath) + sizeof(buf) +1];
    strcpy(fullpath, filepath);
    strcat(fullpath, buf);
    strcat(fullpath, "\0");
    file = fopen(fullpath, "r");
    n = 0;

    if (file) {
        while ((n = fread(fileBuf, 1, sizeof(fileBuf) - 1, file)) > 0) {
          fileBuf[n] = '\0';
          printf("------====== %s", fileBuf);
          fflush(stdout);
          write(fd, fileBuf, n + 1);
        }

        fclose(file);
        close(fd);
    } else {
        errexit("File %s does not exist", fullpath);
    }

    return 0;
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
