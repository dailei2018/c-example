/*
* Dai Lei . 2018
* raw TCP packets
*
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
#include<netinet/udp.h>	//Provides declarations for tcp header
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


struct psd_udp {
	struct in_addr src;
	struct in_addr dst;
	unsigned char pad;
	unsigned char proto;
	unsigned short len;
	struct udphdr udp;
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

struct iphdr iph;
struct udphdr udph;



int main(){
	int res;

	struct iphdr *ip = &iph;
	struct udphdr *udp = &udph;

	char *data = "hello";
	int d_len = strlen(data);

	char *packet;
	packet = malloc(60 + d_len);

	char *msg = packet + 60;
	memcpy(msg, data, d_len);

	// IP
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + d_len;

    // used for uniquely identifying the group of fragments of a single IP datagram
    ip->id = htons(1111);
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;    // set to 0 before calculating checksum
    ip->saddr = inet_addr(SRCIP);
    ip->daddr = inet_addr(DSTIP);
    ip->check = csum((unsigned short *)ip, sizeof(struct iphdr));
	memcpy(packet, ip, sizeof(struct iphdr));

	// UDP
	udp->source = htons(SRCPORT);
	udp->dest = htons(DSTPORT);
	udp->len = htons(sizeof(struct udphdr) + d_len);
	udp->check = 0;

	// udp checksum
	struct psd_udp *p_udp = malloc(sizeof(struct psd_udp) + d_len);
	p_udp->src.s_addr = inet_addr(SRCIP);
    p_udp->dst.s_addr = inet_addr(DSTIP);
    p_udp->pad = 0;
    p_udp->proto = IPPROTO_UDP;
    p_udp->len = htons(sizeof(struct udphdr));
    p_udp->udp = udph;
    char *tmp = ((char *)p_udp) + sizeof(struct psd_udp);
    memcpy(tmp, data, d_len);

    udp->check = csum((unsigned short *)p_udp, sizeof(struct psd_udp) + d_len);

	memcpy(packet + sizeof(struct iphdr), udp, sizeof(struct udphdr));
	memcpy(packet + sizeof(struct iphdr) + sizeof(struct udphdr), data, d_len);


    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DSTPORT);
    sin.sin_addr.s_addr = inet_addr(DSTIP);

    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sfd == -1) err_msg("socket:%s", strerror(errno));

	res = sendto(sfd, packet, sizeof(struct iphdr)+sizeof(struct udphdr)+d_len, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
	if(res == -1) err_msg("sendto:%s", strerror(errno));
}
