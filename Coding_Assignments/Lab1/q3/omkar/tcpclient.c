/**
   author : Omkar M. Rege   
   This is a simple TCP server program that accepts 4 parameters, 
   1. SERVER's name or IP
   2. SERVER's PORT
   3. It sends Greeting message to the server and read server's response which is length of the message and prints both
   4. For example: USAGE : ./tcpclient.o localhost 3333 "Hey there I am using whatsapp"
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int connectTP(const char* host, const char* port, const char* greetme);
//this will take 3 arguments Host Port greetmessage
int main(int argc,char *argv[]) {
   if(argc!=4) {
    fprintf(stderr,"correct command : command_name IP PORT MESSAGE");
    exit(1);
   }
   int s1 = connectTP(argv[1],argv[2],argv[3]);
   close(s1);
   return 0;
}

void errorprinter(const char *errstrg) {
fprintf(stderr,"The error is:%s",errstrg);
}

int connectTP(const char* host, const char* port, const char* greetme) 
{
//char host[12];
//strcpy(host,"10.0.0.32");
int s,n;
struct sockaddr_in sin;
struct servent *sent;
struct hostent *hent;
struct protoent *pent;
memset(&sin,0,sizeof(sin));
sin.sin_family = AF_INET;
if(sent = getservbyname("","tcp")) {
sin.sin_port = sent->s_port;
printf("Port is %d \n",ntohs(sent->s_port));
}
else if((sin.sin_port = htons((u_short)atoi(port)))==0) {
    errorprinter("Port number is not valid");
    exit(1);
}

if(hent = gethostbyname(host)) {
memcpy(&sin.sin_addr,hent->h_addr,hent->h_length);
printf("Host is %s \n",inet_ntoa(sin.sin_addr));
}
else if(inet_aton(host,&sin.sin_addr)==0) {
    errorprinter("Port number is not valid");
    exit(1);
}

if((pent=getprotobyname("tcp"))==0) {
    errorprinter("Transport protocol is bad");
    exit(1);
}

s = socket(PF_INET,SOCK_STREAM,pent->p_proto);
if(connect(s,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    errorprinter("Connect failed gracefully due to wrong Host Name/IP or port");
    exit(1);
}
char wr[128];
int readcnt,length;
strcpy(wr,greetme);
if ( (n= write(s,wr,sizeof wr)) > 0) {
     printf("Message by client: %s\n",greetme);
     printf("size by client: %zu\n",strlen(greetme));
     if((readcnt= read(s,&length,sizeof length)) < 0) {
    printf("can not read!");
    exit(1);
     }
     printf("size by server: %d\n",ntohl(length));
}

return s;
}//end of connectTCP