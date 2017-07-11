#include<stdio.h>
#include<object.h>
#include<namespace.h>
#include<stdlib.h>
#include<mips.h>
#include<string.h>
#include<cheric.h>
#include<assert.h>
#include<sha_info.h>
#include<statcounters.h>
#include<mibench_iter.h>

#define EACH_BLOCK_SIZE 131000

extern char __AES_start, __AES_end;

int
main() {
    for(int i=0; i<8; i++) {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }

	void * u_ref = namespace_get_ref(5);
	assert(u_ref != NULL);
	void * u_id  = namespace_get_id(5);

	void * sha_ref = namespace_get_ref(6);
	assert(sha_ref != NULL);
	void * sha_id  = namespace_get_id(6);

    size_t len = &__AES_end - &__AES_start;
    size_t encdecOffset = 0, totalDeced = 0, remain = 0;
    int64_t encret;
    uint64_t decret;

    uint8_t * __capability enc = (uint8_t * __capability)malloc(EACH_BLOCK_SIZE + 32);
    uint8_t * __capability encdec = (uint8_t * __capability)malloc((size_t)len + 32);

    /* Prepare capabilities that have no permit free permission */
    uint8_t * __capability enc_out = enc;
    uint8_t * __capability encdec_out = encdec;

    const char *theKey = "0123456789ABCDEFFEDCBA98765432100123456789ABCDEFFEDCBA9876543210";

    /* AES benchmark begins */
    stats_init();
    for(int i=0; i<AES_ITER; i++) {
        totalDeced = 0;
        encdecOffset = 0;
        remain = 0;
        while((remain = len-encdecOffset) > EACH_BLOCK_SIZE) {
            encret = ccall_rccc_r(u_ref, u_id, 0, EACH_BLOCK_SIZE, (&__AES_start + encdecOffset), enc_out, (char *)theKey);
            decret = ccall_rccc_r(u_ref, u_id, 0, -encret, enc_out, encdec_out + totalDeced, (char *)theKey);
            encdecOffset += EACH_BLOCK_SIZE;
            totalDeced += decret;
        }
        encret = ccall_rccc_r(u_ref, u_id, 0, remain, (&__AES_start + encdecOffset), enc_out, (char *)theKey);
        decret = ccall_rccc_r(u_ref, u_id, 0, -encret, enc_out, encdec_out + totalDeced, (char *)theKey);
        totalDeced += decret;
    }
    printf("Size of the original: %ld, Total bytes decrypted: %ld\n", len, totalDeced);
    stats_display();
    /* AES benchmark ends */

    /* SHA benchmark begins */
    SHA_INFO * __capability theinfo = (SHA_INFO * __capability)malloc(sizeof(SHA_INFO)+1);
    uint8_t * __capability theinfo_out = (uint8_t *__capability)theinfo;
    stats_init();
    for(int i=0; i<SHA_ITER; i++) {
        encdecOffset = 0;
        remain = 0;
        ccall_rcc_r(sha_ref, sha_id, 0, len, theinfo_out, (&__AES_start + encdecOffset));
        ccall_c_n(sha_ref, sha_id, 1, theinfo_out);
    }
    stats_display();
    /* SHA benchmark ends */
    return 0;
}
