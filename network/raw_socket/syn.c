/* 
* Dai Lei . 2018
* raw TCP packets
* thank Silver Moon
*
*/

#include<stdio.h>	    //for printf
#include<string.h>      //memset
#include<sys/socket.h>	//for socket ofcourse
#include<stdlib.h>      //for exit(0);
#include<errno.h>       //For errno - the error number
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header

void err_msg(const char *format, ...){
    char buf[1024];
    va_list argList;
    va_start(argList, format);
    
    vsnprintf(buf, BUFSIZE, format, argList);
    
    puts(buf);
    exit(1);
}

/* 
	96 bit (12 bytes) pseudo header needed for tcp header checksum calculation 
*/
struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

/*
	Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}

int main(){
    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sfd == -1) err_msg("socket:%s", strerror(errno));
        
    char packet[4096] , *data , *pseudogram;
    u_int32_t src_ip;
    struct ip *iph;
    struct tcphdr *tcph;
    
    memset(packet, 0, 4096);
    
    // IP header
    iph = (struct ip *)packet;
    
    // TCP header
    tcph = (struct tcphdr)(packet + sizeof(struct ip));
    
    // Data
    data = packet + sizeof(struct ip) + sizeof(struct tcphdr);
    memcpy(data, "hello", 5);
}