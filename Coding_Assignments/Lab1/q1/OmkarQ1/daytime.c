/**
   author : Omkar M. Rege   
   This is a simple UDP client program that accepts 1 parameters host
   1. HOST name or IP from which it should get time from
   2. The client reads time from daytime server and gives current time.
   4. For example: USAGE : ./tcpserver.o localhost 3333
*/

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
#include <string.h>
#include <time.h>
#define UNIXEPOCH 2208988800;

int connectTP(const char* host, const char* service);
//this will take 2 arguments host1 and host2
int main(int argc,char *argv[]) {
   if(argc!=2) {
    fprintf(stderr,"correct command : command_name param1");
    exit(1);
   }
   int s1 = connectTP(argv[1],"daytime");
   close(s1);
   return 0;
}

void errorprinter(const char *errstrg) {
fprintf(stderr,"The error is:%s",errstrg);
exit(1);
}

int connectTP(const char* host, const char* service) 
{

int s,n;
struct sockaddr_in sin;
struct servent *sent;
struct hostent *hent;
struct protoent *pent;
memset(&sin,0,sizeof(sin));
sin.sin_family = AF_INET;
if(sent = getservbyname(service,"udp")) {
sin.sin_port = sent->s_port;
printf("Port is %d \n",ntohs(sent->s_port));
}
else if((sin.sin_port = htons((u_short)atoi(service)))==0) {
    errorprinter("Port number is not valid");
}

if(hent = gethostbyname(host)) {
memcpy(&sin.sin_addr,hent->h_addr,hent->h_length);
printf("Host is %s \n",inet_ntoa(sin.sin_addr));
}
else if(inet_aton(host,&sin.sin_addr)==0) {
    errorprinter("Port number is not valid");
}

if((pent=getprotobyname("tcp"))==0) {
    errorprinter("Transport protocol is bad");
}

s = socket(PF_INET,SOCK_STREAM,pent->p_proto);
if(connect(s,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    errorprinter("Connect MalFunctioned due to wrong Host Name");
}
char buffer1[10];
time_t test;
n= read(s,(char *)&test,sizeof test);
/*socklen_t length = sizeof(sin); n = recvfrom(s,buffer1,sizeof(buffer1),0,(struct sockaddr *)&sin,&length);*/
test = ntohl((u_long)test);
test-=UNIXEPOCH;
printf("No of bytes read: %d\n",n);
printf("Response is: %s",ctime(&test));
return s;
}//end of connectTCP
