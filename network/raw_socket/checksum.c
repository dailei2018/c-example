#include <stdio.h>
#include <string.h>

struct h_1{
	unsigned char a;
	unsigned char b;

	unsigned char checksum;
};

struct h_2{
	unsigned char a;
	unsigned char b;

	unsigned short checksum;
	unsigned char c;
};

unsigned char cksum_1(unsigned char *addr, int len)
{
	int sum = 0;
	unsigned char *w = addr;
	unsigned char answer = 0;

	while (len) {
		sum += *w++;
		len -= 1;
	}

	while(sum >> 8){
		sum = (sum >> 8) + (sum & 0xFF);
	}

	answer = ~sum;
	return (answer);
}

unsigned short cksum_2(char *buf, unsigned size)
{
	unsigned sum = 0;
	int i;

	/* Accumulate checksum */
	for (i = 0; i < size - 1; i += 2)
	{
		unsigned short word16 = *(unsigned short *) &buf[i];
		sum += word16;
	}

	/* Handle odd-sized case */
	if (size & 1)
	{
		unsigned short word16 = (unsigned char) buf[i];
		sum += word16;
	}

	/* Fold to get the ones-complement result */
	while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);

	/* Invert to get the negative in ones-complement arithmetic */
	return ~sum;
}

int main(){

	struct h_1 hdr;
	hdr.a = 127;
	hdr.b = 32;
	hdr.checksum = 0;

	unsigned char sum = cksum_1((unsigned char *)&hdr, sizeof(struct h_1));
	//printf("checksum: 0x%x\n", sum);

	hdr.checksum = sum;
	sum = cksum_1((unsigned char *)&hdr, sizeof(struct h_1));

	if(sum == 0){
		printf("checksum valid...\n");
	}else{
		printf("checksum invalid...\n");
	}


	struct h_2 hdr2;
	memset(&hdr2, 0, sizeof(struct h_2));
	hdr2.a = 1;
	hdr2.b = 2;
	hdr2.c = 3;
	hdr2.checksum = 0;

	unsigned short cksum = cksum_2((char *)&hdr2, sizeof(struct h_2));

	hdr2.checksum = cksum;
	cksum = cksum_2((char *)&hdr2, sizeof(struct h_2));

	if(cksum == 0){
		printf("checksum2 valid...\n");
	}else{
		printf("checksum2 invalid...\n");
	}

	return 0;
}


/*
[References]
http://www.netfor2.com/checksum.html
https://tools.ietf.org/html/rfc1071


assume host byte order is little endian

in memory
00000001 00000010    00000000 00000000    00000011 00000000

16-bits addition
00000010 00000001
+
00000000 00000000
------------------
00000010 00000001
+
00000000 00000011
------------------
00000010 00000100

One’s Complement, or Bitwise Negation
11111101 11111011

fill in the checksum
final packet in memory
00000001 00000010    11111011 11111101    00000011 00000000


validate
16-bits addition
00000010 00000001
+
00000000 00000000
------------------
00000010 00000001
+
00000000 00000011
------------------
00000010 00000100
+
11111101 11111011
------------------
11111111 11111111

successfully

--------------------------------------------

assume the receiving end host byte order is big endian
in memory
00000000 00000011 11111101 11111011 00000010 00000001

16-bits addition
00000000 00000011
+
00000010 00000001
------------------
00000010 00000100
+
11111101 11111011
------------------
11111111 11111111

successfully

so it is endian independent
 */
