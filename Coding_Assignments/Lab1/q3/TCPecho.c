//
//  TCPecho.c
//  
//
//  Created by Bing Shi on 3/3/17.
//
//
/* TCPecho.c - main, TCPecho */
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
#define LINELEN 128
#define __USE_BSD 1
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */
int TCPecho(const char *host, const char *service);
int errexit(const char *format, ...);
int connectTCP(const char *host, const char *service);


/*------------------------------------------------------------------------
 * main - TCP client for ECHO service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    char *host = "localhost"; /* host to use if none supplied */
    char *service = "echo"; /* default service name */
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
            fprintf(stderr, "usage: TCPecho [host [port]]\n");
            exit(1);
    }
    TCPecho(host, service);
    exit(0);
}
/*------------------------------------------------------------------------
 * TCPecho - send input to ECHO service on specified host and print reply
 *------------------------------------------------------------------------
 */
int
TCPecho(const char *host, const char *service)
{
    char buf[LINELEN+1]; /* buffer for one line of text */
    int s, n; /* socket descriptor, read count*/
    int outchars, inchars, chardif; /* characters sent and received */
    s = connectTCP(host, service);
    while (fgets(buf, sizeof(buf), stdin)) {
        buf[LINELEN] = '\0'; /* insure line null-terminated */
        outchars = strlen(buf);
        (void) write(s, buf, outchars);
        /* read it back */
        for (inchars = 0; inchars < outchars; inchars+=n ) {
            n = read(s, &buf[inchars], outchars - inchars);
            if (n < 0)
                errexit("socket read failed: %s\n",
                        strerror(errno));
        }
        chardif = inchars - outchars;
        fputs(buf, stdout);
        char buf1[] = "The client sent %d characters, the server responsed with %d characters, the count difference is %d.\n";
        printf(buf1, outchars -1 , inchars -1, chardif);
    }
}
int
errexit(const char *format, ...)
{
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
connectTCP(const char *host, const char *service )
/*
 * Arguments:
 * host - name of host to which connection is desired
 * service - service associated with the desired port
 */
{
    return connectsock( host, service, "tcp");
}
/*------------------------------------------------------------------------
 * connectsock - allocate & connect a socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int
connectsock(const char *host, const char *service, const char *transport )
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
    if ( pse = getservbyname(service, transport) )
        sin.sin_port = pse->s_port;
    else if ( (sin.sin_port = htons((u_short)atoi(service))) == 0 )
        errexit("can't get \"%s\" service entry\n", service);
    /* Map host name to IP address, allowing for dotted decimal */
    if ( phe = gethostbyname(host) )
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
        errexit("can't get \"%s\" host entry\n", host);
    /* Map transport protocol name to protocol number */
    if ( (ppe = getprotobyname(transport)) == 0)
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
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("can't connect to %s.%s: %s\n", host, service,
                strerror(errno));
    return s;
}
