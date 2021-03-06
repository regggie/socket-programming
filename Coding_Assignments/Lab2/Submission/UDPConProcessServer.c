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

int getFileSize(FILE *fp);

int processRequest(int fd, const char *buf, struct sockaddr_in sin);

int errexit(const char *format, ...);

int passivesock(const char *service, const char *transport, int qlen, struct sockaddr_in *sin);

int main(int argc, char *argv[]) {
    char *service = "9200"; /* service name or port number */
    struct sockaddr_in fsin; /* the address of a client */
    struct sockaddr_in ser; /* the address of a client */
    socklen_t addrlen = sizeof(ser);            /* length of addresses */
    int msock; /* master server socket */
    char buf[BUFSIZ];

    msock = passivesock(service, "udp", QLEN, &fsin);

    (void) signal(SIGCHLD, reaper);

    while (1) {
        if (recvfrom(msock, buf, sizeof(buf), 0, (struct sockaddr *) &ser, &addrlen) < 0) {
            errexit("Fail to receive in the server ");
        }

        switch (fork()) {
            case 0: /* child */
                exit(processRequest(msock, buf, ser));
            default: /* parent */
                break;
            case -1:
                errexit("fork: %s\n", strerror(errno));
        }
    }
}

int processRequest(int fd, const char *buf, const struct sockaddr_in sin) {
    char fileBuf[BUFSIZ];
    int n;
    int count = 0;
    char const *filepath = "files/";
    FILE *file;

    char fullpath[sizeof(filepath) + sizeof(buf) + 1];
    strcat(fullpath, filepath);
    strcat(fullpath, buf);

    file = fopen(fullpath, "r");
    n = 0;

    FILE *ft = fopen(fullpath, "r");
    int size = getFileSize(ft);
    char sizeSt[128];
    sprintf(sizeSt, "%d", size);
    fclose(ft);

    sendto(fd, sizeSt, strlen(sizeSt), 0, (struct sockaddr *) &sin, sizeof(sin));

    if (file) {
        while ((n = fread(fileBuf, 1, sizeof(fileBuf), file)) > 0) {
//            fputs(fileBuf, stdout);
            if (sendto(fd, fileBuf, n, 0, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
                errexit("Fail to send file with content: %d", fd);
            }
        }

        fclose(file);
    } else {
        errexit("File %s does not exist", fullpath);
    }

    return 0;
}

int passivesock(const char *service, const char *transport, int qlen, struct sockaddr_in *fsin) {
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

    s = socket(PF_INET, SOCK_DGRAM, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(errno));

    if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0)
        errexit("can't bind to %s port: %s\n", service,
                strerror(errno));

    *fsin = sin;
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

int getFileSize(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    rewind(fp);
    return res;
}
