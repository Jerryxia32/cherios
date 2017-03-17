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

#define EACH_BLOCK_SIZE 256
#define DOMAIN_TIMES 100000

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

	__capability void * aes_PCC = namespace_get_PCC(5);
	assert(aes_PCC != NULLCAP);
	__capability void * aes_IDC = namespace_get_IDC(5);
	assert(aes_IDC != NULLCAP);

	__capability void * helper_PCC = namespace_get_PCC(7);
	assert(helper_PCC != NULLCAP);
	__capability void * helper_IDC = namespace_get_IDC(7);
	assert(helper_IDC != NULLCAP);

    size_t len = &__AES_end - &__AES_start;
    __capability char *AES_data_cap = cheri_setbounds(cheri_setoffset(*((capability __capability *)0x200), (size_t)&__AES_start), len+1);
    size_t encdecOffset = 0, totalDeced = 0, remain = 0;
    int64_t encret;
    uint64_t decret;

    __capability uint8_t *enc = (__capability uint8_t *)malloc_c(EACH_BLOCK_SIZE + 32);
    __capability uint8_t *encdec = (__capability uint8_t *)malloc_c((size_t)len + 32);

    /* Prepare capabilities that have no permit free permission */
    __capability uint8_t *enc_out = cheri_andperm(enc, ~CHERI_PERM_SOFT_0);
    __capability uint8_t *encdec_out = cheri_andperm(encdec, ~CHERI_PERM_SOFT_0);

    const char *theKey = "0123456789ABCDEFFEDCBA98765432100123456789ABCDEFFEDCBA9876543210";
    const __capability char *theKeyCap = cheri_setbounds(cheri_setoffset(*((capability __capability *)0x200), (size_t)theKey), strlen(theKey)+1);

    stats_init();
    for(int i=0; i<AES_ITER; i++) {
        totalDeced = 0;
        encdecOffset = 0;
        remain = 0;
        while((remain = len-encdecOffset) > EACH_BLOCK_SIZE) {
            encret = ccall_real_4_strong_r(0, LONG_MAX, EACH_BLOCK_SIZE, 0, 0, (AES_data_cap + encdecOffset), enc_out, theKeyCap, aes_PCC, aes_IDC, helper_PCC);
            decret = ccall_real_4_strong_r(0, LONG_MAX, -encret, 0, 0, enc_out, encdec_out + totalDeced, theKeyCap, aes_PCC, aes_IDC, helper_PCC);
            encdecOffset += EACH_BLOCK_SIZE;
            totalDeced += decret;
        }
        encret = ccall_real_4_strong_r(0, LONG_MAX, remain, 0, 0, (AES_data_cap + encdecOffset), enc_out, theKeyCap, aes_PCC, aes_IDC, helper_PCC);
        decret = ccall_real_4_strong_r(0, LONG_MAX, -encret, 0, 0, enc_out, encdec_out + totalDeced, theKeyCap, aes_PCC, aes_IDC, helper_PCC);
        totalDeced += decret;
    }
    printf("Size of the original: %ld, Total bytes decrypted: %ld\n", len, totalDeced);
    stats_display();

    __capability SHA_INFO *theinfo = (__capability SHA_INFO *)malloc_c(sizeof(SHA_INFO)+1);
    __capability uint8_t *theinfo_out = cheri_andperm(theinfo, ~CHERI_PERM_SOFT_0);
    stats_init();
    for(int i=0; i<SHA_ITER; i++) {
        ccall_4(sha_ref, sha_id, 0, len, 0, 0, theinfo_out, AES_data_cap, NULLCAP);
    }
    ccall_4(sha_ref, sha_id, 1, 0, 0, 0, theinfo_out, NULLCAP, NULLCAP);
    stats_display();

    printf("Cross domain (ccall safe) for %d times.\n", DOMAIN_TIMES);
    stats_init();
    for(int i=0; i<DOMAIN_TIMES; i++) {
        ccall_real_4_strong_r(1, LONG_MAX, 0, 0, 0, NULLCAP, NULLCAP, NULLCAP, aes_PCC, aes_IDC, helper_PCC);
    }
    stats_display();

    printf("Cross domain (ccall unsafe) for %d times.\n", DOMAIN_TIMES);
    stats_init();
    for(int i=0; i<DOMAIN_TIMES; i++) {
        ccall_real_4_r(1, LONG_MAX, 0, 0, 0, NULLCAP, NULLCAP, NULLCAP, aes_PCC, aes_IDC, helper_PCC);
    }
    stats_display();

    printf("Cross domain (kernel msg queue) for %d times.\n", DOMAIN_TIMES);
    stats_init();
    for(int i=0; i<DOMAIN_TIMES; i++) {
        ccall_4(u_ref, u_id, 1, 0, 0, 0, NULLCAP, NULLCAP, NULLCAP);
    }
    stats_display();
    return 0;
}
