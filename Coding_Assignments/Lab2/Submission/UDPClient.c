#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define LINELEN 4096
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */

int requestFile(const char *host, const char *service, const char *filename);

int errexit(const char *format, ...);

int connectsock(const char *host, const char *service,
                const char *transport, struct sockaddr_in *fsin);

int main(int argc, char *argv[]) {
    char *fileName = NULL;
    char *host = "localhost"; /* host to use if none supplied */
    char *service = "9200"; /* default service name */
    switch (argc) {
        case 2:
            fileName = argv[1];
            break;
        default:
            fprintf(stderr, "usage: UDPCLient [filename]\n");
            exit(1);
    }

    requestFile(host, service, fileName);
    exit(0);
}

int requestFile(const char *host, const char *service, const char *filename) {
    int s, n; /* socket descriptor, read count*/
    char fn[sizeof(filename) + 2];
    FILE *file;
    char buf[LINELEN + 1]; /* buffer for one line of text */

    char const *path = "clientfiles/";
    char *fullpath[sizeof(path) + sizeof(filename)];
    struct sockaddr_in sin;

    struct sockaddr_in ser; /* the address of a client */
    socklen_t addrlen = sizeof(ser);            /* length of addresses */

    strcat(fullpath, path);
    strcat(fullpath, filename);

    strcpy(fn, filename);
    fn[sizeof(fn) - 1] = '\0';
    s = connectsock(host, service, "udp", &sin);

    if (n = (sendto(s, fn, sizeof(fn), 0, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
        errexit("Fail to send the file name: %s", fn);
    }

    memset(buf, 0, sizeof buf);
    int filesize = 0;

    char sizeBuf[LINELEN];
    recvfrom(s, sizeBuf, sizeof(sizeBuf), 0, (struct sockaddr *) &ser, &addrlen);
    filesize = atoi(sizeBuf);

//    filesize = ntohl(filesize);
//    printf("------------>file size is %s", sizeBuf);
    fflush(stdout);
    memset(buf, 0, sizeof buf);

    file = fopen(fullpath, "w+");

//    printf("1----- %d", filesize);
    fflush(stdout);

    while (filesize > 0) {

        int n = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &ser, &addrlen);
        filesize -= n;
        printf("----- %d", n);

        fputs(buf, stdout);
        fputs(buf, file);
        fflush(stdout);
        memset(buf, 0, sizeof buf);
    } /*else {
        errexit("Fail to receive in the server ");
    }*/

    fclose(file);
}

int connectsock(const char *host, const char *service, const char *transport, struct sockaddr_in *fsin) {
    struct sockaddr_in sin; /* an Internet endpoint address */
    struct hostent *phe; /* pointer to host information entry */
    struct servent *pse; /* pointer to service information entry */
    struct protoent *ppe; /* pointer to protocol information entry*/
    int s, type; /* socket descriptor and socket type */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    /* Map service name to port number */

    if (pse = getservbyname(service, transport))
        sin.sin_port = pse->s_port;
    else if ((sin.sin_port = htons((u_short) atoi(service))) == 0)
        errexit("can't get \"%s\" service entry\n", service);
    /* Map host name to IP address, allowing for dotted decimal */
    if (phe = gethostbyname(host))
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        errexit("can't get \"%s\" host entry\n", host);
    /* Map transport protocol name to protocol number */
    if ((ppe = getprotobyname(transport)) == 0)
        errexit("can't get \"%s\" protocol entry\n", transport);


    /* Allocate a socket */
    s = socket(PF_INET, SOCK_DGRAM, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(errno));

    *fsin = sin;

    return s;
}

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}
