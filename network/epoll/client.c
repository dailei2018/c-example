#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define MAX_BUF 64
#define BUF_SIZE 16
#define MAX_EVENTS 5

typedef struct obj_s obj_t;
typedef void (*handler_pt)(obj_t *);

int inetAsyncConnect(char *h, char *ser, int type, obj_t *o);
int ep_process(int timer);
void obj_init(obj_t *o, char *ip, char *port);
int ep_ctl(obj_t *obj, int op);
void handler_in(obj_t *obj);
void handler_out(obj_t *obj);

int my_send(obj_t *obj, int options);
int my_recv(obj_t *obj, int options);

typedef struct my_buf my_buf_t;
struct my_buf{
    char *start;
    char *end;
    
    char *pos;
    char *last;
};

struct obj_s{
    char *ip;
    char *port;
    int fd;
    int status;     //-2:closed -1:error 1:connected - sending 3:send done-reading 5:done
    handler_pt send;
    handler_pt recv;
    struct epoll_event ev;
    
    int max_in;     // maximum bytes allowed receive from the remote
    my_buf_t in_buf;
    my_buf_t out_buf;
};

struct epoll_event ev;
struct epoll_event evlist[MAX_EVENTS];
int epfd;



char * host[] = {
    "192.168.1.242",
    "192.168.1.242",
    "192.168.1.242"
};

char * port[] = {
    "80",
    "81",
    "82"
};

int main(){
    setbuf(stdout, NULL);
    obj_t obj1;
    obj_t obj2;
    obj_t obj3;
    
    epfd = epoll_create(1024);
    
    obj_init(&obj1, host[0], port[0]);
    obj_init(&obj2, host[1], port[1]);
    obj_init(&obj3, host[2], port[2]);
    
    char *str = "hello1";
    strcpy(obj1.out_buf.start, str);
    obj1.out_buf.last += strlen(str);
    
    str = "hello2";
    strcpy(obj2.out_buf.start, str);
    obj2.out_buf.last += strlen(str);
    
    str = "hello3";
    strcpy(obj3.out_buf.start, str);
    obj3.out_buf.last += strlen(str);
    
    inetAsyncConnect(obj1.ip, obj1.port, SOCK_STREAM, &obj1);
    inetAsyncConnect(obj2.ip, obj2.port, SOCK_STREAM, &obj2);
    inetAsyncConnect(obj3.ip, obj3.port, SOCK_STREAM, &obj3);
    
    ep_ctl(&obj1, EPOLL_CTL_ADD);
    ep_ctl(&obj2, EPOLL_CTL_ADD);
    ep_ctl(&obj3, EPOLL_CTL_ADD);
    
    int res;
    while(1){
        res = ep_process(-1);
        if(res == -1) return 1;
        
    }
    
}

void obj_init(obj_t *o, char *ip, char *port){
    o->status = 0;
    o->ev.events = EPOLLOUT | EPOLLIN;
    o->ev.data.ptr = o;
    o->send = handler_out;
    o->recv = handler_in;
    o->ip = ip;
    o->port = port;
    
    o->in_buf.start = malloc(BUF_SIZE);
    o->in_buf.end = o->in_buf.start + BUF_SIZE;
    o->in_buf.pos = o->in_buf.start;
    o->in_buf.last = o->in_buf.start;
    
    o->out_buf.start = malloc(BUF_SIZE);
    o->out_buf.end = o->out_buf.start + BUF_SIZE;
    o->out_buf.pos = o->out_buf.start;
    o->out_buf.last = o->out_buf.start;
    
    o->max_in = MAX_BUF;
}

void handler_in(obj_t *obj){
    
    switch(obj->status){
        case -1:  //connect error
            close(obj->fd);
            obj->status = -2;
            return; 
    }
    
    my_recv(obj, 0);
    
}

void handler_out(obj_t *obj){
    
    switch(obj->status){
        case 0: //connect to server successfully
            obj->status = 1;
            my_send(obj, 0);
            break;
        case 1: //sending
            my_send(obj, 0);
            break;
        case -1:  //connect error
            close(obj->fd);
            obj->status = -2;
            return;
    }
    
}

int my_send(obj_t *obj, int options) {
    int n;
    int size;
    size = obj->out_buf.last - obj->out_buf.pos;
    
    //simulate partial send
    if(size > 1) size--;
    
    n = send(obj->fd, obj->out_buf.pos, size, options);
    if(n < 0){
        close(obj->fd);
        obj->status = -2;
        return 0;
    }
    
    if(n == 0) return 0;
    
    obj->out_buf.pos += n;
    if(obj->out_buf.pos == obj->out_buf.last){
        obj->status = 3;
        
        //send done
        obj->ev.events = EPOLLIN;
        ep_ctl(obj, EPOLL_CTL_MOD);
    }

    return n;
}

int my_recv(obj_t *obj, int options) {
    int n, size;
    
    size = obj->in_buf.end - obj->in_buf.pos;
    
    n = recv(obj->fd, obj->in_buf.pos, size, options);
    
    printf("%s:%s buf:%s\n", obj->ip, obj->port, obj->in_buf.start);
    
    if(n < 0){
        close(obj->fd);
        obj->status = -2;
        return 0;
    }
    
    if(n == 0){
        printf("closed by peer\n");
        close(obj->fd);
        obj->status = -2;
        return n;
    }
    
    obj->in_buf.pos += n;
    
    //no bytes more
    if(n == 1 && *(obj->in_buf.pos-1) == '\n'){
        printf("%s:%s closed\n", obj->ip, obj->port);
        obj->status = 5;
        close(obj->fd);
        return n;
    }
    
    int used = obj->in_buf.pos - obj->in_buf.start;
    // we may need more buffer size
    if(obj->in_buf.pos == obj->in_buf.end){
        if(used < obj->max_in){
            int m_size = used * 2;    // double buffer size
            if(m_size > obj->max_in) m_size = obj->max_in;
            char *buf = malloc(m_size);
            
            memcpy(buf, obj->in_buf.start, used);
            obj->in_buf.pos = buf + used;
            free(obj->in_buf.start);
            obj->in_buf.start = buf;
            obj->in_buf.end = buf + used * 2;
        }else{
            //too many bytes
            close(obj->fd);
            obj->status = -2;
        }
    }
    
    return n;

}

int ep_ctl(obj_t *obj, int op){
    if(epoll_ctl(epfd, op, obj->fd, &obj->ev)){
        perror("epoll_ctl");
        return 1;
    }
    
    return 0;
}

int ep_process(int timer){
    int e_sum;
    int revents;
    obj_t *o;

    e_sum = epoll_wait(epfd, evlist, 1024, -1);
    if(e_sum == -1){
        perror("epoll_wait");
        return -1;
    }
    
    if(e_sum == 0){
        if(timer != -1) return 0;
        
        printf("epoll_wait() returned no events without timeout\n");
        return -1;
    }
    
    int i;
    for (i = 0; i < e_sum; i++) {
        o = evlist[i].data.ptr;

        revents = evlist[i].events;

        if (revents & (EPOLLHUP|EPOLLERR)){
            //fprintf(leo_conf.errfs,"EPOLLHUP|EPOLLERR fd %d\n", evlist[i].data.fd);
            if(o->status == 0){
                printf("%s:%s connect failed\n", o->ip, o->port);
            }
            o->status = -1;
        }

        if (revents & EPOLLIN) {
            o->recv(o);
        }

        if (revents & EPOLLOUT) {
            o->send(o);
        }

    }
    
    return 0;

}

int inetAsyncConnect(char *h, char *ser, int type, obj_t *o){
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, res = -1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;        /* Allows IPv4 or IPv6 */
    hints.ai_socktype = type;

    s = getaddrinfo((char *)h, (char *)ser, &hints, &result);
    if (s != 0) {
        errno = ENOSYS;
        return -1;
    }

    /* Walk through returned list until we find an address structure
       that can be used to successfully connect a socket */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol);
        if (sfd == -1)
            continue;                   /* On error, try next address */

        s = connect(sfd, rp->ai_addr, rp->ai_addrlen);

        //successful
        if(s == 0){
            o->fd = sfd;
            o->status = 1;
            res = sfd;
            break;
        }

        //wait to be notified
        if(s == -1 && errno == EINPROGRESS){
            o->fd = sfd;
            res = sfd;
            break;
        }

        /* falied */

        close(sfd);
        o->status = -2;
    }

    freeaddrinfo(result);

    return res;
}
