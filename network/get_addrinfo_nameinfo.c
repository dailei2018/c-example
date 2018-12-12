#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>		//exit

#include <sys/socket.h>
#include <netdb.h>

void err_msg(const char *format, ...){
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsnprintf(buf, 1024, format, argList);

    puts(buf);
    exit(1);
}

int main(){
	struct addrinfo hints;
	struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;

    char *host = "192.168.1.242";
    char *port = "3306";

    int s,sfd;
    s = getaddrinfo(host, port, &hints, &result);
    if(s != 0) err_msg("getaddrinfo:%s", strerror(errno));

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;                   /* On error, try next address */

        s = connect(sfd, rp->ai_addr, rp->ai_addrlen);

        //直接成功
        if(s == 0){
            puts("successfully");
        }else{
            puts("falied");
        }

        //-------------------------------------
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        s = getnameinfo(rp->ai_addr, rp->ai_addrlen, host, NI_MAXHOST,
        				service, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV );

        if(s != 0){
        	puts("getnameinfo falied");
        }else{
        	printf("host:%s, service:%s\n", host, service);
        }
        close(sfd);
    }

    freeaddrinfo(result);



    //-------------------------------------------------------------------

}
