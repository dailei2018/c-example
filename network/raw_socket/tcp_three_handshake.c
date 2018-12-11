/*
* Dai Lei . 2018
* raw TCP packets
* reference
* 	https://en.wikipedia.org/wiki/Transmission_Control_Protocol
*   http://www.enderunix.org/docs/en/rawipspoof/
*
*/

/*
 * before run this program,you should use iptables to temporarily drop outgoing rst packets
 *
 * iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
 *
 * compile with gcc
 * gcc tcp_three_handshake.c -lpcap -lpthread
 *
 * this program is just for testing, have fun
 */

#include<stdio.h>	    //for printf
#include<string.h>      //memset
#include<sys/socket.h>	//for socket ofcourse
#include<stdlib.h>      //for exit(0);
#include<stdarg.h>      //for va_start
#include<errno.h>       //For errno - the error number
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header
#include <arpa/inet.h>

#include <unistd.h>   //for sleep

#include <pthread.h>

#include <pcap.h>

#define BUFSIZE 1024

#define SRCIP "192.168.1.111"
#define SRCPORT 8881

#define DSTIP "192.168.1.242"
#define DSTPORT 888

void err_msg(const char *format, ...){
    char buf[BUFSIZE];
    va_list argList;
    va_start(argList, format);

    vsnprintf(buf, BUFSIZE, format, argList);

    puts(buf);
    exit(1);
}

struct psd_tcp {
	struct in_addr src;
	struct in_addr dst;
	unsigned char pad;
	unsigned char proto;
	unsigned short tcp_len;
	struct tcphdr tcp;
};

struct tcphdr_opt {
	struct tcphdr tcp;
	int mss;
	/*
	unsigned char nop;
	unsigned char[3] ws;
	unsigned char[2] nop;
	unsignet char[10] tstamp;
	*/
};

struct psd_tcp_opt {
	struct in_addr src;
	struct in_addr dst;
	unsigned char pad;
	unsigned char proto;
	unsigned short tcp_len;
	struct tcphdr_opt tcp;
};

int sfd, raw_fd;
struct iphdr iph;
struct tcphdr tcph;

struct psd_tcp p_tcp;


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

void send_syn()
{
	int res;

	struct iphdr *ip = &iph;
	struct tcphdr *tcp = &tcph;

	u_char *packet;
	packet = (u_char *)malloc(60);

	// IP
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);

    // used for uniquely identifying the group of fragments of a single IP datagram
    ip->id = htons(1111);
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_TCP;
    ip->check = 0;    // set to 0 before calculating checksum
    ip->saddr = inet_addr(SRCIP);
    ip->daddr = inet_addr(DSTIP);
    ip->check = csum((unsigned short *)ip, sizeof(struct iphdr));
	memcpy(packet, ip, sizeof(struct iphdr));

	// TCP
    tcp->source = htons(SRCPORT);
    tcp->dest = htons(DSTPORT);
    tcp->seq = htonl(0x28e482b7);
    tcp->ack_seq = 0;
    tcp->doff = 5;
    tcp->fin=0;
    tcp->syn=1;
    tcp->rst=0;
    tcp->psh=0;
    tcp->ack=0;
    tcp->urg=0;
    tcp->window = htons(29200);  /* maximum allowed window size */
    tcp->check = 0;  // set to 0, filled later by pseudo header
    tcp->urg_ptr = 0;

    // checksum (pseudo header + tcp)
    p_tcp.src.s_addr = inet_addr(SRCIP);
    p_tcp.dst.s_addr = inet_addr(DSTIP);
    p_tcp.pad = 0;
    p_tcp.proto = IPPROTO_TCP;
    p_tcp.tcp_len = htons(sizeof(struct tcphdr));
    p_tcp.tcp = tcph;

    tcp->check = csum((unsigned short *)&p_tcp, sizeof(struct psd_tcp));

    memcpy((packet + sizeof(struct iphdr)), tcp, sizeof(struct tcphdr));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DSTPORT);
    sin.sin_addr.s_addr = inet_addr(DSTIP);

	res = sendto(sfd, packet, sizeof(struct iphdr)+sizeof(struct tcphdr), 0, (struct sockaddr *)&sin, sizeof(struct sockaddr));
	if(res == -1) err_msg("sendto:%s", strerror(errno));
}

void send_syn_ack(uint32_t s_seq)
{
	int res;

	struct iphdr *ip = &iph;
	struct tcphdr *tcp = &tcph;

	u_char *packet;
	packet = (u_char *)malloc(60);

	// IP

    // used for uniquely identifying the group of fragments of a single IP datagram
    ip->id = htons(2222);
    ip->check = 0;    // set to 0 before calculating checksum
    ip->check = csum((unsigned short *)ip, sizeof(struct iphdr));
	memcpy(packet, ip, sizeof(struct iphdr));

	s_seq++;
	// TCP
	printf("seq:%u\n", s_seq);
    tcp->seq = htonl(0x28e482b8);
    tcp->ack_seq = htonl(s_seq);

    printf("seq:%u\n", ntohl(tcp->ack_seq));

    tcp->syn=0;
    tcp->ack=1;
    tcp->check = 0;  // set to 0, filled later by pseudo header

    // checksum (pseudo header + tcp)
    p_tcp.src.s_addr = inet_addr(SRCIP);
    p_tcp.dst.s_addr = inet_addr(DSTIP);
    p_tcp.pad = 0;
    p_tcp.proto = IPPROTO_TCP;
    p_tcp.tcp_len = htons(sizeof(struct tcphdr));
    p_tcp.tcp = tcph;

    tcp->check = csum((unsigned short *)&p_tcp, sizeof(struct psd_tcp));

    memcpy((packet + sizeof(struct iphdr)), tcp, sizeof(struct tcphdr));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DSTPORT);
    sin.sin_addr.s_addr = inet_addr(DSTIP);

	res = sendto(sfd, packet, sizeof(struct iphdr)+sizeof(struct tcphdr), 0, (struct sockaddr *)&sin, sizeof(struct sockaddr));
	if(res == -1) err_msg("sendto:%s", strerror(errno));
}

void raw_packet_receiver(u_char *udata, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
	struct iphdr *ip;
	struct tcphdr *tcp;
	u_char *ptr;
	int l1_len = *udata;
	uint32_t s_seq;

	ip = (struct iphdr *)(packet + l1_len);
	tcp = (struct tcphdr *)(packet + l1_len + sizeof(struct iphdr));

	printf("%d\n", l1_len);

	printf("a packet came, ack_seq is: %d\n", ntohl(tcp->ack_seq));
	printf("a packet came, seq is: %u\n", ntohl(tcp->seq));
	s_seq = ntohl(tcp->seq);

	send_syn_ack(s_seq);

	sleep(100);
}

void *pth_capture_run(void *arg)
{
	pcap_t *pd;

	char filter[128];
	sprintf(filter, "dst host %s and ip", SRCIP);
	char *dev = "enp9s0";
	char errbuf[PCAP_ERRBUF_SIZE];
	bpf_u_int32	netp;
	bpf_u_int32	maskp;
	struct bpf_program	fprog;					/* Filter Program	*/
	int dl = 0, dl_len = 0;

	if ((pd = pcap_open_live(dev, 1514, 1, 500, errbuf)) == NULL) {
		fprintf(stderr, "cannot open device %s: %s\n", dev, errbuf);
		exit(1);
	}

	pcap_lookupnet(dev, &netp, &maskp, errbuf);
	pcap_compile(pd, &fprog, filter, 0, netp);
	if (pcap_setfilter(pd, &fprog) == -1) {
		fprintf(stderr, "cannot set pcap filter %s: %s\n", filter, errbuf);
		exit(1);
	}
	pcap_freecode(&fprog);
	dl = pcap_datalink(pd);

	switch(dl) {
		case 1:
			dl_len = 14;
			break;
		default:
			dl_len = 14;
			break;
	}

	if (pcap_loop(pd, -1, raw_packet_receiver, (u_char *)&dl_len) < 0) {
		fprintf(stderr, "cannot get raw packet: %s\n", pcap_geterr(pd));
		exit(1);
	}
}


int main(){
    int res;

    pthread_t tid_pr;
	if (pthread_create(&tid_pr, NULL, pth_capture_run, NULL) != 0) {
		fprintf(stderr, "cannot create raw packet reader: %s\n", strerror(errno));
		exit(1);
	}

	sleep(1);

    sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sfd == -1) err_msg("socket:%s", strerror(errno));

    send_syn();

    pthread_join(tid_pr, NULL);
    return 0;
}
