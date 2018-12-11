/*
 * Dai Lei . 2018
 * you may test it with syn.c
 *
 * [reference]
 * 		https://www.binarytides.com/tcp-syn-portscan-in-c-with-linux-sockets/
 */

#include<stdio.h>	    //for printf
#include<string.h>      //memset
#include<sys/socket.h>	//for socket ofcourse
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdlib.h>      //for exit(0);
#include<unistd.h>
#include<stdarg.h>      //for va_start
#include<errno.h>       //For errno - the error number
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header

void process_packet(unsigned char* buffer, int size)
{
	//Get the IP Header part of this packet
	struct iphdr *iph = (struct iphdr*)buffer;
	struct sockaddr_in source,dest;
	unsigned short iphdrlen;

	if(iph->protocol == IPPROTO_TCP)
	{

		// filter
		if(iph->saddr == (u_int32_t)inet_addr("192.168.1.100")){
			return;
		}
		// ip header size in bytes
		iphdrlen = iph->ihl * 4;

		struct tcphdr *tcph=(struct tcphdr*)(buffer + iphdrlen);

		printf("syn:%d ack:%d rst:%d\n", tcph->syn, tcph->ack, tcph->rst);
		if(tcph->syn == 1 && tcph->ack == 1)
		{
			char srcbuf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &iph->saddr, srcbuf, INET_ADDRSTRLEN);

			printf("ip:%s port:%d open \n" ,srcbuf, ntohs(tcph->source));
		}else if(tcph->rst == 1){
			char srcbuf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &iph->saddr, srcbuf, INET_ADDRSTRLEN);

			printf("ip:%s port:%d reset \n" ,srcbuf, ntohs(tcph->source));
		}
	}

	//printf("protocol:%d\n", iph->protocol);
}

int main(){
	int sock_raw;

	int saddr_size , data_size;
	struct sockaddr saddr;

	unsigned char *buffer = malloc(65536); //Its Big!

	//read all tcp packet
	sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);

	if(sock_raw < 0){
		printf("Socket Error\n");
		return 1;
	}

	saddr_size = sizeof(struct sockaddr);

	while(1)
	{
		//Receive a packet
		data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &saddr , &saddr_size);

		if(data_size <0 ){
			printf("Recvfrom error , failed to get packets\n");
			return 1;
		}

		//Now process the packet
		process_packet(buffer , data_size);
	}

	close(sock_raw);
	printf("recv finished.");
	return 0;

}
