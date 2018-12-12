#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, INET_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_ARP = 0x0806
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <linux/if_arp.h>
#include <net/ethernet.h>

#include <errno.h>            // errno, perror()
#include <stdarg.h>

#include <sys/stat.h>
#include <fcntl.h>

void err_msg(const char *format, ...){
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsnprintf(buf, 1024, format, argList);

    puts(buf);
    exit(1);
}

// Define a struct for ARP header
typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t htype;
  uint16_t ptype;
  uint8_t hlen;
  uint8_t plen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

// Define some constants.
#define ETH_HDRLEN 14      // Ethernet header length
#define IP4_HDRLEN 20      // IPv4 header length
#define ARP_HDRLEN 28      // ARP header length

#define SRCIP "192.168.1.111"
#define DSTIP "192.168.1.243"

int main (int argc, char **argv){
	int s, sfd;
	char *interface;
	unsigned char src_mac[6], dst_mac[6];
	char src_ip[INET_ADDRSTRLEN], dst_ip[INET_ADDRSTRLEN];

	struct ifreq ifr;
	struct sockaddr_ll device;
	memset (&ifr, 0, sizeof(ifr));
	memset (&device, 0, sizeof(device));

	interface = "enp9s0";
	strcpy(ifr.ifr_name, interface);

	// get src mac address
	sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(sfd == -1) err_msg("socket:%s", strerror(errno));

	s = ioctl(sfd, SIOCGIFHWADDR, &ifr);
	if(s == -1) err_msg("ioctl:%s", strerror(errno));

	memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6);
	memset(dst_mac, 0xff, 6);
	close(sfd);

	//printf("mac:%02x:%02x:%02x:%02x:%02x:%02x ",
	//		src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);

	device.sll_ifindex = if_nametoindex(interface);
	device.sll_family = AF_PACKET;
	memcpy(device.sll_addr, src_mac, 6);
	device.sll_halen = 6;
	//printf("index:%d\n", device.sll_ifindex);


	arp_hdr arphdr;
	unsigned int tmp;

	tmp = inet_addr(SRCIP);
	memcpy(arphdr.sender_ip, &tmp, 4);
	tmp = inet_addr(DSTIP);
	memcpy(arphdr.target_ip, &tmp, 4);

	arphdr.htype = htons(1);
	arphdr.ptype = htons(ETH_P_IP);
	arphdr.hlen = 6;
	arphdr.plen = 4;
	arphdr.opcode = htons(ARPOP_REQUEST);  //1 for arp request
	memcpy(arphdr.sender_mac, src_mac, 6);

	memset(arphdr.target_mac, 0, 6);

	char ether_frame[60];
	int frame_len, n;
	unsigned short *type;

	frame_len = 6 + 6 + 2 + 28;

	memcpy(ether_frame, dst_mac, 6);
	memcpy(ether_frame+6, src_mac, 6);
	type = (unsigned short *)(ether_frame + 12);
	*type = htons(ETH_P_ARP);

	// arp header
	memcpy (ether_frame + 14, &arphdr, 28);

	sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sfd == -1) err_msg("socket:%s", strerror(errno));

	n = sendto(sfd, ether_frame, 60, 0, (struct sockaddr *) &device, sizeof (device));
	if(n <= 0) err_msg("sendto:%s", strerror(errno));

	close(sfd);

	sfd = open("arp.pcap", O_WRONLY|O_CREAT);
	if(sfd <= 0) err_msg("open:%s", strerror(errno));
	write(sfd, ether_frame, 60);
	close(sfd);

	return 0;
}






