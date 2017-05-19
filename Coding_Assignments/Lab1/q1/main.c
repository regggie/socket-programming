#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFSIZE 64
#define UNIXEPOCH 2208988800 /* UNIX epoch, in UCT secs */
#define MSG "what time is it?\n"
typedef unsigned long u_long;

int
errexit(const char *format, ...) {
    va_list args;
    vfprintf(stderr, format, args);
    exit(1);
}

typedef unsigned short u_short;


/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[]) {
    char *host = "localhost"; /* host to use if none supplied */
    char *service = "time"; /* default service name */
    char *transport = "udp";
    time_t now; /* 32-bit integer to hold time */
    int s, n, type; /* socket descriptor, read count*/

    struct hostent *phe; /* pointer to host information entry */
    struct servent *pse; /* pointer to service information entry */
    struct protoent *ppe; /* pointer to protocol information entry*/
    struct sockaddr_in sin; /* an Internet endpoint address */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    
    switch (argc) {
        case 1:
            host = "localhost";
            break;
        case 3:
            service = argv[2];
            /* FALL THROUGH */
        case 2:
            host = argv[1];
            break;
        default:
            fprintf(stderr, "usage: UDPtime [host [port]]\n");
            exit(1);
    }

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

    (void) write(s, MSG, strlen(MSG));

    /* Read the time */
    n = read(s, (char *) &now, sizeof(now));
    if (n < 0)
        errexit("read failed: %s\n", strerror(errno));
    now = ntohl((u_long) now); /* put in host byte order */
    now -= UNIXEPOCH; /* convert UCT to UNIX epoch */
    printf("%s", ctime(&now));
    exit(0);
}