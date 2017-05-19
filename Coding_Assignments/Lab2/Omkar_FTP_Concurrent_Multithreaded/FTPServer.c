//file transfer server for testing FTP hope it will be easy :-)
//This is concurrent multithreading server

/**
   author : Omkar M. Rege   
   This is a simple TCP server program that accepts 2 parameters, 
   1. IP on which it should run
   2. Port which it should use
   3. The server return the length of the string of client's greeting message back to the client
   4. For example: USAGE : ./tcpserver.o localhost 3333
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

int definelisteningsocket(char* ip);
void * filehandler(void * arg);

int main(int argc,char *argv[]) {

 //1st do error arg checking
   if(argc!=2) {
    fprintf(stderr,"USAGE: command_name IP");
    exit(1);
   }
int lsd, connsocksd, connsocklength;
struct sockaddr_in connsock;
lsd = definelisteningsocket(argv[1]);
pthread_t clientthread;

fflush(stdout);
printf("\nreturned from definelisteningsocket\n");

while(1) {
  connsocklength = sizeof connsock;
  connsocksd = accept(lsd,(struct sockaddr *)&connsock, &connsocklength);
  printf("\n the server says %d",connsocksd);
 if(connsocksd > 0) {
   fflush(stdout);
   if(pthread_create(&clientthread,NULL,filehandler,(void *)&connsocksd) !=0) {
    printf("error is spawning thread");    
   }
   printf("A thread has been spawned!!!");
 }
 else{
   printf("error in creating client socket by accept");
 }

}


}//end of main


int definelisteningsocket(char* ip) {

int lsocksd;
struct sockaddr_in listensock;
struct servent* service;
struct protoent* protocol; 
memset(&listensock,0,sizeof listensock);
listensock.sin_family = AF_INET;
//first we will convert ip from ascii string to network address
    if(inet_aton(ip, &listensock.sin_addr)!=0) {
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

//getting port from service or servent

if(service=getservbyname("ftp","tcp")) {
/*printf("Port is %d",ntohs(service->s_port));
listensock.sin_port = service->s_port;*/
if((listensock.sin_port = htons( (u_short)atoi("3000")))==0 ){
printf("something wrong with port"); //just printing for debugging
exit(1);
}

}

//getting protocol by name 
if((protocol=getprotobyname("tcp"))==0) {
 printf("Can not get protocol");
}

lsocksd = socket(PF_INET,SOCK_STREAM,protocol->p_proto);

//its like connect for client
if( bind(lsocksd,(struct sockaddr*) &listensock, sizeof listensock ) < 0) { printf("\n Something wrong with bind"); exit(1);}

    if(listen(lsocksd,5) < 0)
    {
    printf("Something went wrong with lock");
    exit(1);
    }
return lsocksd;
}

void * filehandler(void * arg) {
 fflush(stdout);
 printf("This is thread");
 const char* const DIR_PATH="./";
 int connsocksd = *((int *) arg);
 fflush(stdout);
 printf("This is thread");

 int bytesread=0,cnt=0;
 char buffer[128];
 char *message,*temp;
 char ch;
 bool isFirstAttempt = true;

//    while(1) {
    //reading part
          while( (bytesread= read(connsocksd,buffer,sizeof buffer) ) > 0) {
            cnt=cnt+bytesread;
                if(isFirstAttempt==true) {       
            message=(char *) malloc(bytesread+1);
            memcpy(message,buffer,bytesread);
                message[bytesread] ='\0';
                isFirstAttempt=false;
            }
          } //end reading
      

    //writting part
      fflush(stdout);
      printf("File name client sent is %s",message);
      fflush(stdout);
      //resetting buffer and counter
      memset(buffer,0,sizeof buffer);
      cnt=0;
      char filename[128];
      memset(filename,0,128);
      strcpy(filename,DIR_PATH);
      strcat(filename,message);
      FILE *fp = fopen(filename,"r");
      int totalwritten=0,bytesWritten=0;
      while(1) {
        ch=fgetc(fp);    
            if(feof(fp)) {
          fflush(stdout);
              printf("cnt is %d",cnt);
          fflush(stdout);
          break;        
        }    
    buffer[cnt]=ch;
    cnt++;      
     printf("\n%c %d",ch,cnt);     
        fflush(stdout);
            if(cnt == 128) {
             while(cnt != 0) {
             bytesWritten = write(connsocksd,buffer+totalwritten,128-totalwritten);
             totalwritten+=bytesWritten;
             cnt-=bytesWritten;
                 }
         cnt=0;bytesWritten=0;totalwritten=0;
             memset(buffer,0,sizeof buffer); //reset the buffer
        }
      }

      bytesWritten=0;totalwritten=0;
      while(cnt > 0) {
        printf("Writting to the client %s:",buffer); fflush(stdout);
    bytesWritten=write(connsocksd,buffer+totalwritten,cnt-totalwritten);
    totalwritten+=bytesWritten;
    cnt-=bytesWritten;
        }
  fclose(fp);
  free(message); 
  close(connsocksd); 
  printf("Thread has closed connected socket"); 
  fflush(stdout);
//  break;
//  } //end of while(1)
}