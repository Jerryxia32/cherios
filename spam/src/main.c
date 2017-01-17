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

#define EACH_BLOCK_SIZE 256

extern char __AES_start, __AES_end;

int
main() {
    stats_init();
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
    __capability char *AES_data_cap = cheri_setbounds(cheri_setoffset(cheri_getdefault(), (size_t)&__AES_start), len+1);
    size_t encdecOffset = 0, totalDeced = 0, remain = 0;
    int64_t encret;
    uint64_t decret;

    __capability uint8_t *enc = (__capability uint8_t *)malloc_c(EACH_BLOCK_SIZE + 32);
    __capability uint8_t *encdec = (__capability uint8_t *)malloc_c((size_t)len + 32);

    const char *theKey = "0123456789ABCDEFFEDCBA98765432100123456789ABCDEFFEDCBA9876543210";
    const __capability char *theKeyCap = cheri_setbounds(cheri_setoffset(cheri_getdefault(), (size_t)theKey), strlen(theKey)+1);

    while((remain = len-encdecOffset) > EACH_BLOCK_SIZE) {
        encret = ccall_rccc_r(u_ref, u_id, 0, EACH_BLOCK_SIZE, (AES_data_cap + encdecOffset), enc, theKeyCap);
        decret = ccall_rccc_r(u_ref, u_id, 0, -encret, enc, encdec + totalDeced, theKeyCap);
        encdecOffset += EACH_BLOCK_SIZE;
        totalDeced += decret;
    }
    encret = ccall_rccc_r(u_ref, u_id, 0, remain, (AES_data_cap + encdecOffset), enc, theKeyCap);
    decret = ccall_rccc_r(u_ref, u_id, 0, -encret, enc, encdec + totalDeced, theKeyCap);
    totalDeced += decret;

    printf("Size of the original: %ld, Total bytes decrypted: %ld\n", len, totalDeced);

    __capability SHA_INFO *theinfo = (__capability SHA_INFO *)malloc_c(sizeof(SHA_INFO)+1);
    ccall_rcc_n(sha_ref, sha_id, 0, len, theinfo, AES_data_cap);
    ccall_c_n(sha_ref, sha_id, 1, theinfo);
    stats_display();

    return 0;
}
