#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>

void err_msg(const char *format, ...){
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsnprintf(buf, 1024, format, argList);

    puts(buf);
    exit(1);
}

int main(){
	int res;
	struct ifaddrs *ifaddr, *ifa;

    char host[NI_MAXHOST];
    int family, s, n;

    res = getifaddrs(&ifaddr);
    if(res == -1) err_msg("getifaddrs:%s", strerror(errno));

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    	if (ifa->ifa_addr == NULL) continue;

    	family = ifa->ifa_addr->sa_family;

    	printf("%-8s %s (%d) %s\n", ifa->ifa_name,
    			(family == AF_PACKET) ? "AF_PACKET" :
    			(family == AF_INET) ? "AF_INET" :
    	        (family == AF_INET6) ? "AF_INET6" : "???",
    	        family,
				ifa->ifa_flags & IFF_UP ? "on" : "off"
    	);

    	if (family == AF_INET || family == AF_INET6) {
    		s = getnameinfo(ifa->ifa_addr,
    						family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
    						host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

    		if (s != 0) err_msg("getnameinfo:%s", strerror(errno));

    		printf("\t\taddress: %s\n", host);

    	} else if (family == AF_PACKET && ifa->ifa_data != NULL) {
    		struct rtnl_link_stats *stats = ifa->ifa_data;
    	    printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
    	           "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
				   stats->tx_packets, stats->rx_packets,
				   stats->tx_bytes, stats->rx_bytes);
    	}
	}

	freeifaddrs(ifaddr);

	return 0;
}
