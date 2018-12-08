/* 
* Dai Lei . 2018
* udp client, connect.
* you can use networking utility ncat as udp server to test.
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

#include <stdarg.h>
#include <errno.h>

#define HOST "192.168.1.242"
//#define HOST "192.168.1.123"
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
    if(res == -1) err_msg("inet_pton:%s", strerror(errno));
    
    len = sizeof(s_in);
    
    // bind to remote address, if you want to bind another address, change sockaddr and call connect again.
    res = connect(sfd, (struct sockaddr *)&s_in, len);
    if(res == -1) err_msg("connect:%s", strerror(errno));
    
    char *str = "hello from client";
    
    // if remote server on but do not listen specific port, we will receive an icmp packet
    // afterwards, read system call will fail
    n = write(sfd, str, strlen(str));
    
    if(n != strlen(str)){
        err_msg("partial/failed write");
    }
    
    printf("hello msg sent\n");

    // after return, len contains the number of bytes actually written to this structure
    n = read(sfd, buf, BUFSIZE);
    if(n == -1){
        err_msg("read:%s", strerror(errno));
    }
    
    buf[n] = '\0';
    printf("server: %s\n", buf);
    
    close(sfd);
    return 0;
}