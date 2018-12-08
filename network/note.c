#include <sys/socket.h>

enum __socket_type
{
  SOCK_STREAM = 1,		/* Sequenced, reliable, connection-based byte streams.  */
  SOCK_DGRAM = 2,		/* Connectionless, unreliable datagrams of fixed maximum length.  */
  SOCK_RAW = 3,			/* Raw protocol interface.  */
  
  SOCK_RDM = 4,			/* Reliably-delivered messages.  */
  SOCK_SEQPACKET = 5	/* Sequenced, reliable, connection-based,datagrams of fixed maximum length.  */
};

// protocol families

#define	AF_UNSPEC	0
#define	AF_UNIX		1
#define	AF_INET		2
#define AF_INET6	26

int socket(int domain, int type, int protocol);

struct sockaddr {
    sa_family_t sa_family;   /* Address family (AF_* constant) */
    char sa_data[14];     /* Socket address (size varies according to socket domain) */
};

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#define SOMAXCONN	128
int listen(int sockfd, int backlog);

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);




#include <arpa/inet.h>

Network byte order - big endian
Host byte order - little endian | big endian

big endian - the most significant byte first
little endian - the least significant byte first

//converted to network byte order
uint16_t htons(uint16_t host_uint16);
uint32_t htonl(uint32_t host_uint32);

// converted to host byte order
uint16_t ntohs(uint16_t net_uint16);
uint32_t ntohl(uint32_t net_uint32);



#include <netinet/in.h>

struct in_addr {          /* IPv4 4-byte address */
    in_addr_t s_addr;     /* Unsigned 32-bit integer */
};

struct sockaddr_in {            /* IPv4 socket address */
    sa_family_t sin_family;     /* Address family (AF_INET) */
    in_port_t sin_port;         /* Port number */
    struct in_addr sin_addr;    /* IPv4 address */
    unsigned char __pad[X];     /* Pad to size of 'sockaddr' structure (16 bytes) */
};

struct sockaddr_un {
    sa_family_t sun_family;    /* Always AF_UNIX */
    char sun_path[108];        /* Null-terminated socket pathname */
};


struct in6_addr {         /* IPv6 address structure */
    uint8_t s6_addr[16];  /* 16 bytes == 128 bits */
};

struct sockaddr_in6 {           /* IPv6 socket address */
    sa_family_t sin6_family;    /* Address family (AF_INET6) */
    in_port_t sin6_port;        /* Port number */
    uint32_t sin6_flowinfo;     /* IPv6 flow information */
    struct in6_addr sin6_addr;  /* IPv6 address */
    uint32_t sin6_scope_id;     /* Scope ID (new in kernel 2.4) */
};



#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46



#include <arpa/inet.h>

int inet_pton(int domain, const char *src_str, void *addrptr);
//Returns 1 on successful conversion, 0 if src_str is not in presentation format, or –1 on error

const char *inet_ntop(int domain, const void *addrptr, char *dst_str, size_t len);
//Returns pointer to dst_str on success, or NULL on error