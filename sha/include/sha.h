#ifndef SHA_H
#define SHA_H

#include<sha_info.h>

/* NIST Secure Hash Algorithm */
/* heavily modified from Peter C. Gutmann's implementation */

/* Useful defines & typedefs */

#define SHA_BLOCKSIZE		64
#define SHA_DIGESTSIZE		20

void sha_init(SHA_INFO * __capability);
void sha_update(SHA_INFO * __capability, BYTE * __capability, int);
void sha_final(SHA_INFO * __capability);

void sha_stream(SHA_INFO * __capability, char * __capability, size_t);
void sha_print(SHA_INFO * __capability);

#endif /* SHA_H */
