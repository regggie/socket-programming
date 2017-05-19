/**
   author : Omkar M. Rege   
   This is a simple TCP server program that accepts 2 parameters, 
   1. IP on which it should run
   2. Port which it should use
   3. The server return the length of the string of client's greeting message back to the client
   4. For example: USAGE : ./tcpserver.o localhost 3333
*/

/*common*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdarg.h>
#include<string.h>
#include<strings.h>
#include <errno.h>
/*network related*/
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

int defineListenSock(char *ip ,char * port);
void returncount(int connsockfd);


/*
argv[1] is IP argv[2] Port
*/
int main(int argc,char *argv[]) {
//1st do error arg checking
   if(argc!=3) {
    fprintf(stderr,"USAGE: command_name IP PORT");
    exit(1);
   }

int lfd, connsockfd, connsocksize;
struct sockaddr_in connsock;
lfd=defineListenSock(argv[1],argv[2]);

    while(1) {
     //accept the connection if any and populate connection socket connsock
     connsocksize = sizeof connsock;
     connsockfd = accept(lfd,(struct sockaddr *)&connsock,&connsocksize); 
     if(connsockfd > 0) {
      switch(fork()) {
        case 0:
         //close the copy for listening socket child has got
         close(lfd);
        // call the fuction which reads message from client and retuns count
        returncount(connsockfd);
        exit(0);
        default:
        close(connsockfd);
        break;
        case -1:
        printf("something fishy! :-(");
        exit(1);
       }
     }
     else {
      printf("Something wrong with the connsock");
      exit(1);
     }
    }//end of while
}//end of main

/*this function creates Listening Sock and returns*/
int defineListenSock(char *ip ,char *port) {
int lsock;
struct sockaddr_in listensock;
struct servent *sent;
struct protoent *pent;
memset(&listensock,0,sizeof listensock);
listensock.sin_family = AF_INET;
if(inet_aton(ip,&listensock.sin_addr) == 0) {
    printf("Your IP adrress is weird %s",ip);
}
else if(strcasecmp("localhost",ip)==0) {
    printf("You have a Localhost");
    if(inet_aton("127.0.0.1",&listensock.sin_addr) == 0) {
       printf("It's better to leave this world!");
       exit(1);
    }
}
else { //listening to any/all incoming address
    printf("Now I can accept anything since you do not know IPV4");
    listensock.sin_addr.s_addr = INADDR_ANY;
}
printf("server address is %s",inet_ntoa(listensock.sin_addr));
 

//assigning port of service to listening sock
    if(sent = getservbyname("","tcp")) {
    listensock.sin_port = sent->s_port;
    printf("Port is %d \n",ntohs(sent->s_port)); //just printing for debugging
    }
    else if( (listensock.sin_port = htons((u_short)atoi(port)))==0 ){
    printf("something wrong with port %s",port); //just printing for debugging
        exit(1);
    }

    if((pent=getprotobyname("tcp"))==0) {
    printf("can not get protocol");
    exit(1);
    }

lsock = socket(PF_INET,SOCK_STREAM,pent->p_proto);
    if(bind(lsock,(struct sockaddr *)&listensock,sizeof listensock) < 0) {
    printf("something went wrong in bind");
    
    }
//listen system call
    if(listen(lsock,5) < 0)
    {
    printf("Something went wrong with lock");
    exit(1);
    }

return lsock;
}

void returncount(int connsockfd) {
char buffer[128];
int bytes;
    //read as long as number of bytes read are > 0
    if( bytes=read(connsockfd,buffer,sizeof buffer) ) {
        if(bytes < 0) {
        printf("number of bytes read are less wrong");    
        exit(1);
        }
        printf("%s bytes read by server",buffer);
        int length = strlen(buffer);
        length = htonl(length); //Counting the length of the string sent by client
        if(write(connsockfd,&length,sizeof length) < 0){
        printf("Error in Writting count");    
        exit(1);
        }//end of if

    }//end of while
} //end of function