#include <stdlib.h>
/*
    atoi does not detect errors.
    "12aa" -> 12
    "a12a" -> 0
*/
int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);


/*
    base: 2-36, 0 means 10 unless '0x' prefix or '0' prefix
    endptr: If endptr is not NULL, stores the address of the first invalid character in *endptr
    
    "aaaa" -> 0
    "0x11" -> 17, base == 0 or base == 16
    "011" -> 9, base == 0 or base == 8
*/
long int strtol(const char *nptr, char **endptr, int base);
long long int strtoll(const char *nptr, char **endptr, int base);
