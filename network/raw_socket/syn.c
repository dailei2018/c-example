/* 
* Dai Lei . 2018
* raw TCP packets
* thank Silver Moon
*
*/

#include<stdio.h>	    //for printf
#include<string.h>      //memset
#include<sys/socket.h>	//for socket ofcourse
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdlib.h>      //for exit(0);
#include<stdarg.h>      //for va_start
#include<errno.h>       //For errno - the error number
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header

#define BUFSIZE 1024

#define SRCIP "192.168.1.111"
#define SRCPORT 8881

#define DSTIP "192.168.1.242"
#define DSTPORT 1234

void err_msg(const char *format, ...){
    char buf[BUFSIZE];
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

	while(sum >> 16) sum = (sum >> 16) + (sum & 0xffff);

	answer=(short)~sum;
	
	return(answer);
}

int main(){
    int res;

    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sfd == -1) err_msg("socket:%s", strerror(errno));
        
    char packet[4096] , *data , *pseudogram;
    u_int32_t src_ip;
    struct iphdr *iph;
    struct tcphdr *tcph;
    
    memset(packet, 0, 4096);
    
    // IP header
    iph = (struct iphdr *)packet;
    
    // TCP header
    tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
    
    // Data
    char *msg = "";
    int msg_len = strlen(msg);
    data = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
    memcpy(data, msg, msg_len);


    // Fill in the IP header-----------------------------

    /*
     * internet header length, 5 * 32 bits = 160 bits = 20 bytes;
     * The minimum value for this field is 5, means no options
     */
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + msg_len;

    // used for uniquely identifying the group of fragments of a single IP datagram
    iph->id = htons(1111);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;    // set to 0 before calculating checksum

    res = inet_pton(AF_INET, SRCIP, &iph->saddr);
    if(res == -1) err_msg("inet_pton:%s", strerror(errno));

    res = inet_pton(AF_INET, DSTIP, &iph->daddr);
    if(res == -1) err_msg("inet_pton:%s", strerror(errno));

    // Fill in the TCP header

    tcph->source = htons(SRCPORT);
    tcph->dest = htons(DSTPORT);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    /*
     *
     * data offset(tcp header size), 5 * 32 bits = 160 bits = 20 bytes;
     * The minimum value for this field is 5, means no options
     * the maximum is 15, 15 * 32 bits = 480 bits = 60 bytes;
     */
    tcph->doff = 5;

    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons(2048);  /* maximum allowed window size */
    tcph->check = 0;  // set to 0, filled later by pseudo header
    tcph->urg_ptr = 0;

    //Now the TCP checksum
    struct pseudo_header psh;
    int psize;
    psh.source_address = inet_addr(SRCIP);
    psh.dest_address = inet_addr(DSTIP);
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + msg_len);

    psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + msg_len;
    char *pseudo_packet = malloc(psize);
    memcpy(pseudo_packet, &psh, sizeof(struct pseudo_header));
    memcpy(pseudo_packet + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + msg_len);
    tcph->check = csum( (unsigned short*) pseudo_packet , psize);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DSTPORT);
    sin.sin_addr.s_addr = inet_addr(DSTIP);

    res = sendto (sfd, packet, iph->tot_len , 0, (struct sockaddr *) &sin, sizeof (sin));
    if(res == -1) err_msg("sendto:%s", strerror(errno));

    printf ("Syn packet sent len:%d\n" , iph->tot_len);

    return 0;
}
