#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#include <net/if.h>           // struct ifreq

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stropts.h>  //ioctl

#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.

void err_msg(const char *format, ...){
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsnprintf(buf, 1024, format, argList);

    puts(buf);
    exit(1);
}

int main(){
	int sfd, res;
	struct ifreq ifr;

	sfd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(sfd == -1) err_msg("socket:%s", strerror(errno));

	char *interface = "wlp3s0";

	memset (&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, interface);

	res = ioctl(sfd, SIOCGIFFLAGS, &ifr);
	if(res == -1) err_msg("ioctl:%s", strerror(errno));

	puts(interface);
	printf("  status:%s ", ifr.ifr_flags & IFF_UP ? "on" : "off");

	res = ioctl(sfd, SIOCGIFHWADDR, &ifr);
	if(res == -1) err_msg("ioctl:%s", strerror(errno));

	unsigned char mac[6];
	memcpy (mac, ifr.ifr_hwaddr.sa_data, 6);

	printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
		   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
	);

}
