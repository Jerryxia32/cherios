#ifndef SHA_H
#define SHA_H

#include<mips.h>

/* NIST Secure Hash Algorithm */
/* heavily modified from Peter C. Gutmann's implementation */

/* Useful defines & typedefs */

typedef uint8_t BYTE;
typedef uint32_t LONG;

#define SHA_BLOCKSIZE		64
#define SHA_DIGESTSIZE		20

typedef struct {
    LONG digest[5];		/* message digest */
    LONG count_lo, count_hi;	/* 64-bit bit count */
    LONG data[16];		/* SHA data buffer */
} SHA_INFO;

void sha_init(SHA_INFO * __capability);
void sha_update(SHA_INFO * __capability, BYTE * __capability, int);
void sha_final(SHA_INFO * __capability);

void sha_stream(SHA_INFO * __capability, char * __capability, size_t);
void sha_print(SHA_INFO * __capability);

#endif /* SHA_H */
