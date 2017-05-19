//file transfer server for testing FTP hope it will be easy :-)
//This is concurrent multithreading server

/**
   author : Omkar M. Rege  
   This is a simple TCP server program that accepts 1 parameter,
   1. IP on which it should run
   2. The server return the file the client has specified
   4. For example: USAGE : ./udpserver.o localhost
*/
//Common header
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdbool.h>

//Network specific
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

//thread library
#include<pthread.h>

int definelisteningsocket(char *ip);

void *filehandler(void *arg);

int getFileSize(FILE *fp);

// A struct to pass arguements to the Pthread
struct ThreadPara {
    int mainSock;
    struct sockaddr_in *clientSockPtr;
    char *filename;

};

int main(int argc, char *argv[]) {
    int lsd, clientsocksd, clientsocklength;
    char buffer[1024];
    struct sockaddr_in clientsock;
    lsd = definelisteningsocket("localhost");
    pthread_t clientthread;

    fflush(stdout);
    printf("\nreturned from definelisteningsocket\n");

    while (1) {
        clientsocklength = sizeof clientsock;
        if (recvfrom(lsd, buffer, sizeof buffer, 0, (struct sockaddr *) &clientsock, &clientsocklength) < 0) {
            printf("Receive failed");
            fflush(stdout);
            exit(1);
        }
        struct ThreadPara myPara;
        myPara.mainSock = lsd;
        myPara.clientSockPtr = &clientsock;
        myPara.filename = buffer;

        if (pthread_create(&clientthread, NULL, filehandler, (void *) &myPara) != 0) {
            printf("\nerror is spawning thread\n");
        }
        printf("\nA thread has been spawned!!!\n");
        fflush(stdout);

    }

}//end of main

void *filehandler(void *arg) {
    fflush(stdout);
    printf("\nThis is thread");
    const char *const DIR_PATH = "./files/";
    struct ThreadPara param = *((struct ThreadPara *) arg);
    fflush(stdout);
    printf("\nThis is thread");

    char *message = param.filename;
    printf("\nFile name client sent is %s \n", message);
    fflush(stdout);
    int mainSock = param.mainSock;
    struct sockaddr_in *clientSocket = param.clientSockPtr;
    //send filesize to client
    char filename[128];
    memset(filename, 0, 128);
    strcpy(filename, DIR_PATH);
    strcat(filename, message);

    FILE *ft = fopen(filename, "r");
    int size = getFileSize(ft);
    char sizeSt[128];
    sprintf(sizeSt, "%d", size);

//    printf("\n Size is %d", size);
    fclose(ft);
    sendto(mainSock, sizeSt, strlen(sizeSt), 0, (struct sockaddr *) clientSocket, sizeof *(clientSocket));


    int bytesread = 0, cnt = 0;
    char buffer[128];
    char ch;
    //writting part
    printf("\nFile name client sent is %s \n", message);
    fflush(stdout);
    //resetting buffer and counter
    memset(buffer, 0, sizeof buffer);
    cnt = 0;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        sendto(mainSock, "FILE DOES NOT EXIST", 21, 0, (struct sockaddr *) clientSocket, sizeof *(clientSocket));
        exit(0);
    }
    int totalwritten = 0, bytesWritten = 0;
    while (1) {
        ch = fgetc(fp);
        if (feof(fp)) {
            fflush(stdout);
            printf("cnt is %d", cnt);
            fflush(stdout);
            break;
        }
        buffer[cnt] = ch;
        cnt++;
        printf("\n%c %d", ch, cnt);
        fflush(stdout);
        if (cnt == 128) {
            while (cnt != 0) {
                bytesWritten = sendto(mainSock, buffer + totalwritten, 128 - totalwritten, 0,
                                      (struct sockaddr *) clientSocket, sizeof *(clientSocket));
                totalwritten += bytesWritten;
                cnt -= bytesWritten;
            }
            cnt = 0;
            bytesWritten = 0;
            totalwritten = 0;
            memset(buffer, 0, sizeof buffer); //reset the buffer
        }
    }
//      printf("Reached Here"); fflush(stdout);
    bytesWritten = 0;
    totalwritten = 0;
    while (cnt > 0) {
        printf("Writting to the client %s:", buffer);
        fflush(stdout);
        bytesWritten = sendto(mainSock, buffer + totalwritten, cnt, 0, (struct sockaddr *) clientSocket,
                              sizeof *(clientSocket));
        totalwritten += bytesWritten;
        cnt -= bytesWritten;
    }
    fclose(fp);
//  free(message);
//  printf("Thread has closed connected socket");
//  fflush(stdout);
}

int getFileSize(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    rewind(fp);
    return res;
}

int definelisteningsocket(char *ip) {

    int lsocksd;
    struct sockaddr_in listensock;
    struct servent *service;
    struct protoent *protocol;
    memset(&listensock, 0, sizeof listensock);
    listensock.sin_family = AF_INET;
//first we will convert ip from ascii string to network address
    if (inet_aton(ip, &listensock.sin_addr) != 0) {
        printf("Your IP adrress is weird %s", ip);
    } else if (strcasecmp("localhost", ip) == 0) {
        printf("You have a Localhost");
        if (inet_aton("127.0.0.1", &listensock.sin_addr) == 0) {
            printf("It's better to leave this world!");
            exit(1);
        }
    } else { //listening to any/all incoming address
        printf("Now I can accept anything since you do not know IPV4");
        listensock.sin_addr.s_addr = INADDR_ANY;
    }
    printf("server address is %s", inet_ntoa(listensock.sin_addr));

//getting port from service or servent

    listensock.sin_port = htons(9200);


//getting protocol by name
    if ((protocol = getprotobyname("udp")) == 0) {
        printf("Can not get protocol");
    }

    lsocksd = socket(PF_INET, SOCK_DGRAM, protocol->p_proto);

//its like connect for client
    if (bind(lsocksd, (struct sockaddr *) &listensock, sizeof listensock) < 0) {
        printf("\n Something wrong with bind");
        exit(1);
    }

    return lsocksd;
}


