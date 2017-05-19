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
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int connectTP(const char* host, const char* filename);
void writebytestofile(int *connsocksd,const char * filename);
//this will take 1 argument only filename
int main(int argc,char *argv[]) {
  if(argc!=3) {
    fprintf(stderr,"correct command : command_name IP PORT MESSAGE");
    exit(1);
   }
   int s1 = connectTP(argv[1],argv[2]);
   close(s1);
   return 0;
}

void errorprinter(const char *errstrg) {
fprintf(stderr,"The error is:%s",errstrg);
}

int connectTP(const char* host, const char* message) 
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
if(sent = getservbyname("ftp","tcp")) {
//sin.sin_port = sent->s_port;
if((sin.sin_port = htons((u_short)atoi("3000")))==0) {errorprinter("Port number is not valid");exit(1);}

printf("Port is %d \n",ntohs(sent->s_port));
}
else {errorprinter("We are using fixed ftp port"); exit(1);}
/*else if((sin.sin_port = htons((u_short)atoi(port)))==0) {
    errorprinter("Port number is not valid");
    exit(1);
}*/

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
    errorprinter("Connect failed gracefully due to wrong Host Name/IP or port\n");
        printf("\nerror number is %s",strerror(errno));
    exit(1);
}
char wr[128];
int readcnt,length;
strcpy(wr,message);

if ( (n= write(s,wr,sizeof wr)) > 0) {
     printf("Message by client: %s\n",message);
     printf("size by client: %zu\n",strlen(message));
    }
writebytestofile(&s,message);
return s;
}//end of connectTCP

void writebytestofile(int *connsocksd,const char * filename) {
  fflush(stdout);  
  printf("socket descriptor is %d",*connsocksd);
  fflush(stdout);
  int bytesread=0,cnt=0,i=0;
  char buffer[128],qualifiedName[128];
  memset(buffer,0,128);
  memset(qualifiedName,0,128);
  strcpy(qualifiedName,"./client_");
  strcat(qualifiedName,filename);
  printf("name is %s",qualifiedName);
  fflush(stdout);
  char ch;
  bool isFirstAttempt = true; 
  FILE *fd;
  while(1) {

        bytesread= read(*(connsocksd),buffer+cnt,128-cnt);
     if(bytesread <= 0){ printf("some error while reading");fflush(stdout);break;}        
        printf("Read is waiting at client's side for file");
    fflush(stdout);
    cnt=cnt+bytesread;
        if(isFirstAttempt==true) {
    fd = fopen(qualifiedName,"a");
    isFirstAttempt=false;    
    }
     for(i=0;i<cnt;i++) {
               printf("%c",buffer[i]);
            fputc(buffer[i],fd);    
           fflush(stdout);
     }
    if(cnt>=128) {    
     cnt=0; bytesread=0;
     memset(buffer,0,128);
    }
            
  }
  fclose(fd);
  close(*connsocksd);
}