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

extern int errno;

typedef unsigned short u_short;

int getFileSize(FILE *fp);

int processRequest(int fd, const char *buf, struct sockaddr_in sin);

int errexit(const char *format, ...);

//int passivesock(const char *service, const char *transport, int qlen, struct sockaddr_in *sin);
int passivesock(const char *service, const char *transport, int qlen);

int passiveUDP(const char *service);

int main(int argc, char *argv[]) {
    char *service = "9200"; /* service name or port number */
    struct sockaddr_in fsin; /* the address of a client */
    int alen; /* from-address length */
    int sock; /* server socket */
    char buf[BUFSIZ];
    char fileBuf[BUFSIZ];
    int n;
    char const *filepath = "files/";
    FILE *file;

    sock = passiveUDP(service);

//    fputs("a", stdout);
//    fflush(stdout);

    while (1) {
        alen = sizeof(fsin);
        if (recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *) &fsin, &alen) < 0) {
            errexit("Fail to receive in the server ", strerror(errno), "\n");
        }

        char fullpath[sizeof(filepath) + sizeof(buf) + 1];
        bzero(fullpath, sizeof(fullpath));
        strcat(fullpath, filepath);
        strcat(fullpath, buf);

        file = fopen(fullpath, "r");
        int n = 0;

        FILE *ft = fopen(fullpath, "r");
        int size = getFileSize(ft);
        char sizeSt[128];
        sprintf(sizeSt, "%d", size);
        fclose(ft);

        sendto(sock, sizeSt, strlen(sizeSt), 0, (struct sockaddr *) &fsin, sizeof(fsin));

        if (file) {
            while ((n = fread(fileBuf, 1, sizeof(fileBuf), file)) > 0) {

                if ((sendto(sock, fileBuf, n, 0, (struct sockaddr *) &fsin, sizeof(fsin))) < 0) {
                    errexit("Fail to send file with content: %d", sock);
                }

//                printf("----- %d", n);
//                fputs(fileBuf, stdout);
                fflush(stdout);
            }

            fputs("--------------", stdout);
            fflush(stdout);
            fclose(file);
        } else {
            errexit("File %s does not exist", fullpath);
        }
    }
}

int getFileSize(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    rewind(fp);
    return res;
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

    /* Map protocol name to protocol number */
    if ((ppe = getprotobyname(transport)) == 0)
        errexit("can't get \"%s\" protocol entry\n", transport);

    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    /* Allocate a socket */
    s = socket(PF_INET, SOCK_DGRAM, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(errno));

    /* Bind the socket */
    if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0)
        errexit("can't bind to %s port: %s\n", service,
                strerror(errno));

    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("can't listen on %s port: %s\n", service,
                strerror(errno));

    return s;
}

int passiveUDP(const char *service) {
    return passivesock(service, "udp", QLEN);
}

/*--------------------------------------------------HELPER--------------------------------------------------*/

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}
