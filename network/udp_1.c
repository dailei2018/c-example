/* 
* Dai Lei . 2018
* udp client, no connect.
* you can use linux tool ncat as udp server to test.
* [root@izj6 ~]# nc -ul 88
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define HOST "192.168.1.242"
#define PORT 88 
#define BUFSIZE 1024

void err_msg(const char *format, ...){
    char buf[BUFSIZE];
    va_list argList;
    va_start(argList, format);
    
    vsnprintf(buf, BUFSIZE, format, argList);
    
    puts(buf);
    exit(1);
}

int main(){
    int sfd, res;
    struct sockaddr_in s_in;
    
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sfd == -1) err_msg("socket:%s", strerror(errno));
    
    s_in.sin_family = AF_INET;
    s_in.sin_port = htons(PORT);
    
    struct in_addr ser_addr;
    char buf[BUFSIZE];
    int n, len;
    
    res = inet_pton(AF_INET, HOST, &s_in.sin_addr);
    if(sfd == -1) err_msg("inet_pton:%s", strerror(errno));
    
    char *str = "hello from client";
    sendto(sfd, str, strlen(str), 0, (struct sockaddr *)&s_in, sizeof(s_in));
    
    printf("hello msg sent\n");
    
    // first len should be initialized to the size of the structure
    len = sizeof(s_in);
    // after return, len contains the number of bytes actually written to this structure
    n = recvfrom(sfd, buf, BUFSIZE, 0, (struct sockaddr *)&s_in, &len);
    
    buf[n] = '\0';
    printf("server: %s\n", buf);
    
    close(sfd);
    return 0;
}