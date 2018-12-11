/*
* Dai Lei . 2018
* raw TCP packets
*
*
*/

#include <stdio.h>	    //for printf
#include <string.h>      //memset
#include <sys/socket.h>	//for socket ofcourse
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>      //for exit(0);
#include <unistd.h>
#include <stdarg.h>      //for va_start
#include <errno.h>       //For errno - the error number
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>	//Provides declarations for ip header

#define BUFSIZE 1024

#define SRCIP "192.168.1.111"

#define DSTIP "192.168.1.242"

void err_msg(const char *format, ...){
    char buf[BUFSIZE];
    va_list argList;
    va_start(argList, format);

    vsnprintf(buf, BUFSIZE, format, argList);

    puts(buf);
    exit(1);
}

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
struct icmphdr icmph;

int main(){
	int res;

	struct iphdr *ip = &iph;
	struct icmphdr *icmp = &icmph;

	char *data = "hello";
	int d_len = strlen(data);

	char *packet;
	packet = malloc(60);

	// IP
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + d_len;

    // used for uniquely identifying the group of fragments of a single IP datagram
    ip->id = 0;
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_ICMP;
    ip->check = 0;    // set to 0 before calculating checksum
    ip->saddr = inet_addr(SRCIP);
    ip->daddr = inet_addr(DSTIP);
    ip->check = csum((unsigned short *)ip, sizeof(struct iphdr));
	memcpy(packet, ip, sizeof(struct iphdr));

	// UDP
	icmp = malloc(sizeof(struct icmphdr) + d_len);
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = 0;
	icmp->un.echo.sequence = 0;
	char *tmp = (char *)icmp + sizeof(struct icmphdr);
	memcpy(tmp, data, d_len);

	icmp->checksum = csum((unsigned short *)icmp, sizeof(struct icmphdr) + d_len);
	memcpy(packet + sizeof(struct iphdr), icmp, sizeof(struct icmphdr) + d_len);


    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(DSTIP);

    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sfd == -1) err_msg("socket:%s", strerror(errno));

	res = sendto(sfd, packet, 60, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
	if(res == -1) err_msg("sendto:%s", strerror(errno));

	icmp->checksum = 0;
	icmp->un.echo.id = 0;
	icmp->un.echo.sequence = htons(1);

	icmp->checksum = csum((unsigned short *)icmp, sizeof(struct icmphdr) + d_len);
	memcpy(packet + sizeof(struct iphdr), icmp, sizeof(struct icmphdr) + d_len);

	sleep(1);

	res = sendto(sfd, packet, 60, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
	if(res == -1) err_msg("sendto:%s", strerror(errno));
}
