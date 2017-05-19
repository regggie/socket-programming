#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#define UNIXEPOCH 2208988800;

using namespace std;
//#include <sys/types.h>

void printUsageMessage() {

cout<<endl<<"*******************************************************";
cout<<endl<<"Please Enter port Number and server address like below";
cout<<endl<<"*******************************************************";
cout<<endl<<"command port_number server_ip";
}


int main(int argc,char* argv[]){
//step 1 : Accept proper command line arguements
int port=0,s,n;
//char line[129];
time_t now;
if(argc!=3 || (port=atoi(argv[1]))==0 || argv[2]=='\0'){
printUsageMessage();
return 0;
}
string server=argv[2];
cout<<endl<<port<<server;
// step 2: create UDP socket
//1. create sock_addr
struct sockaddr_in sin;
//2.Decide which udp Application service to be used in order to get port number
struct servent *ste; // servent is used to populate remote server port by demanding service (daytime,ftp,telnet) with (tcp/udp)
//3. Hostent required to fill sock_addr
struct hostent *hte;
//4. Tranport level protocol information
struct protoent *pte;

//initialize sockaddr_in
memset(&sin,0,sizeof(sin));

//set port by using servent methods like getserverbyname(servername,transport)
sin.sin_family=AF_INET; // setting family to use IPV4
if(ste=getservbyname("daytime","udp")) {
cout<<endl<<"service info"<<ste->s_name<<ntohs(ste->s_port);
sin.sin_port=ste->s_port;

}
else if (sin.sin_port=htons((u_short)port)==0) {
cout<<endl<<"port number is zero";
return 0;
}

//now seet the Host IP address in sockaddr_in /etc/host
if(hte=gethostbyname(server.c_str())) {
cout<<endl<<"Host Addess is : "<<hte->h_name;
memcpy(&sin.sin_addr,hte->h_addr,hte->h_length);
cout<<endl<<inet_ntoa(sin.sin_addr);

}
else if((sin.sin_addr.s_addr=inet_addr(server.c_str())) == INADDR_NONE) //sockaddr_in has in_addr has uint32_t s_addr
cout<<endl<<"can not parse or get IP address";

//Setting up transport level protocol attributes whether TCP or UDP
if((pte=getprotobyname("udp"))==0) {
cout<<endl<<"Can not get specified protocol";
}

// Since we have serice,host and protocol information we can create socket
cout<<endl<<"Protocol: "<<pte->p_proto;
s= socket(PF_INET,SOCK_DGRAM,pte->p_proto); //socket(family,transport_protocol,p->proto)
if(s==0) {
cout<<endl<<"error in allocating socket";
return 0;
}
cout<<endl<<s;

if( connect(s,(struct sockaddr *)&sin,sizeof(sin)) < 0 ){
 cout<<endl<<"something went wrong! can not connect";
}
else {
cout<<endl<<"connected";
}

n = read(s,(char*)&now,sizeof(now));
now-=UNIXEPOCH;
printf("%s",ctime(&now));

/*if( (n=read(s,line,128)) >0) {
 line[128]='\0';
 cout<<endl<<line;
}*/

return 0;
}