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
                const char *transport);

int main(int argc, char *argv[]) {
    char *fileName = NULL;
    char *host = "localhost"; /* host to use if none supplied */
    char *service = "9200"; /* default service name */
    switch (argc) {
        case 2:
            fileName = argv[1];
            break;
        default:
            fprintf(stderr, "usage: TCPCLient [filename]\n");
            exit(1);
    }

    requestFile(host, service, fileName);
    exit(0);
}

int requestFile(const char *host, const char *service, const char *filename) {
    char buf[LINELEN + 1]; /* buffer for one line of text */
    int s, n; /* socket descriptor, read count*/
    char fn[sizeof(filename) + 2];
    FILE *file;

    char const *path = "clientfiles/";
    char *fullpath[sizeof(path) + sizeof(filename)];

    strcat(fullpath, path);
    strcat(fullpath, filename);

    strcpy(fn, filename);
    fn[sizeof(fn) - 1] = '\0';
    s = connectsock(host, service, "tcp");

    (void) write(s, fn, sizeof(fn));

    file = fopen(fullpath, "w+");
    while ((n = read(s, buf, sizeof(buf))) > 0) {
        fputs(buf, stdout);
        fputs(buf, file);
        memset(buf, 0, sizeof(buf));
    }

    fclose(file);
}

int connectsock(const char *host, const char *service, const char *transport) {
    struct hostent *phe; /* pointer to host information entry */
    struct servent *pse; /* pointer to service information entry */
    struct protoent *ppe; /* pointer to protocol information entry*/
    struct sockaddr_in sin; /* an Internet endpoint address */
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
    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    /* Allocate a socket */
    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(errno));
    /* Connect the socket */
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0)
        errexit("can't connect to %s.%s: %s\n", host, service,
                strerror(errno));
    return s;
}

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}
