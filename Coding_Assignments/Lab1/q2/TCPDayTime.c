/* TCPdaytime.c - TCPdaytime, main */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>
#include <ctype.h>

#define LINELEN 128
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */

//extern int errno;
int TCPdaytime(const char *host, const char *service);

int errexit(const char *format, ...);

int connectTCP(const char *host, const char *service);

typedef unsigned short u_short;

/*------------------------------------------------------------------------
 * main - TCP client for DAYTIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[]) {
    char *host1 = "localhost"; /* host to use if none supplied */
    char *host2 = "localhost"; /* default service port */
    switch (argc) {
        case 1:
            host1 = "localhost";
            host2 = "localhost";
            break;
        case 3:
            host1 = argv[1];
            host2 = argv[2];
/*             FALL THROUGH */
        case 2:
            host1 = argv[1];
            host2 = argv[2];
            break;
        default:
            fprintf(stderr, "usage: TCPdaytime [host [port]]\n");
            exit(1);
    }
    TCPdaytime(host1, host2);

    exit(0);
}

/*------------------------------------------------------------------------
 * TCPdaytime - invoke Daytime on specified host and print results
 *------------------------------------------------------------------------
 */
int TCPdaytime(const char *host1, const char *host2) {
    char buf[LINELEN + 1];
    char buf2[LINELEN + 1];
    int s1, s2; /* socket, read count */
    ssize_t n;
    char *time1, *time2 = NULL;
    int len = 0;

    s1 = connectTCP(host1, "daytime");
    while ((n = read(s1, buf, LINELEN)) > 0) {
        time1 = realloc(time1, len + n);
        memcpy(time1 + len, buf, n);
        len += n;

    } /* to get daytime from host1 */

    time1[len] = '\0';
    printf("%s, %d", time1, len);

    len = 0;
    n = 0;
    s2 = connectTCP(host2, "daytime");

    while ((n = read(s2, buf, LINELEN)) > 0) {
        time2 = realloc(time2, len + n );
        memcpy(time2 + len, buf, n);
        len += n;
        time1[len] = '\0';

//        printf("%s", time2);

    }/* to get daytime from host2 */
    time2[len] = '\0';
    printf("%s, %d, %zu", time2, len, strlen(time2));


    //    differences between host1, host2
    struct tm result, result2;
    char formatedTime1[22] = {0};
    char formatedTime2[22] = {0};

    strcat(strncpy(formatedTime1, (time1 + 7), 17), " 0000");
    strcat(strncpy(formatedTime2, (time2 + 7), 17), " 0000");

    memset(time1, 0, sizeof(time1));
    memset(time2, 0, sizeof(time2));
    free(time1);
    free(time2);

    strptime(formatedTime1, "%y-%m-%d %H:%M:%S %z", &result);
    strptime(formatedTime2, "%y-%m-%d %H:%M:%S %z", &result2);
    int diff = timegm(&result) - timegm(&result2);

//    printf("%lu, %lu", mktime(&result), mktime(&result2));
//    printf("%d, %d", result.tm_hour, result2.tm_hour);
    printf("\n The different between the time is %d second(s)\n", diff);
}

int
errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

int connectsock(const char *host, const char *service,
                const char *transport);

/*------------------------------------------------------------------------
 * connectTCP - connect to a specified TCP service on a specified host
 *------------------------------------------------------------------------
 */
int
connectTCP(const char *host, const char *service)
/*
 * Arguments:
 * host - name of host to which connection is desired
 * service - service associated with the desired port
 */
{
    return connectsock(host, service, "tcp");
}

/*------------------------------------------------------------------------
 * connectsock - allocate & connect a socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int
connectsock(const char *host, const char *service, const char *transport)
/*
 * Arguments:
 * host - name of host to which connection is desired
 * service - service associated with the desired port
 * transport - name of transport protocol to use ("tcp" or "udp")
 */
{
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