#include"adpcm.h"
#include<stdio.h>
#include<string.h>
#include<cheric.h>
#include<mibench_iter.h>

struct adpcm_state state;

#define NSAMPLES 1000

char	abuf[NSAMPLES/2];
short	sbuf[NSAMPLES];

extern char __adpcm_c_start, __adpcm_c_end;
extern char __adpcm_d_start, __adpcm_d_end;

int
main() {
    // first, construct two capability to the buffers.
    char __capability *abufcap = cheri_getdefault();
    abufcap = cheri_setoffset(abufcap, (size_t)abuf);
    abufcap = cheri_setbounds(abufcap, NSAMPLES/2);
    short __capability *sbufcap = cheri_getdefault();
    sbufcap = cheri_setoffset(sbufcap, (size_t)sbuf);
    sbufcap = cheri_setbounds(sbufcap, NSAMPLES*2);
    
    // second, construct two capabilities to the two data regions.
    size_t cSize = &__adpcm_c_end - &__adpcm_c_start;
    size_t dSize = &__adpcm_d_end - &__adpcm_d_start;
    char __capability *adpcmCcap = cheri_getdefault();
    char __capability *adpcmDcap = cheri_getdefault();
    adpcmCcap = cheri_setoffset(adpcmCcap, (size_t)&__adpcm_c_start);
    adpcmCcap = cheri_setbounds(adpcmCcap, cSize);
    adpcmDcap = cheri_setoffset(adpcmDcap, (size_t)&__adpcm_d_start);
    adpcmDcap = cheri_setbounds(adpcmDcap, dSize);

    size_t totalProcessed = 0;
    size_t remain = 0;

    for(int n=0; n<ADPCM_ITER; n++) {

        /* encode stage */
        while((remain = cSize - totalProcessed) > NSAMPLES*2) {
            memcpy_c(sbufcap, adpcmCcap + totalProcessed, NSAMPLES*2);
            adpcm_coder(sbufcap, abufcap, NSAMPLES, &state);
            memcpy_c(cheri_getdefault(), abufcap, NSAMPLES/2);
            totalProcessed += NSAMPLES*2;
        }
        memcpy_c(sbufcap, adpcmCcap + totalProcessed, remain);
        adpcm_coder(sbufcap, abufcap, remain/2, &state);
        memcpy_c(cheri_getdefault(), abufcap, remain/4);
        totalProcessed += remain;
        printf("adpcm encoded %ld bytes in total.\n", totalProcessed);

        /* decode stage, reinitialize variables. */
        totalProcessed = 0;
        remain = 0;
        while((remain = dSize - totalProcessed) > NSAMPLES/2) {
            memcpy_c(abufcap, adpcmDcap + totalProcessed, NSAMPLES/2);
            adpcm_decoder(abufcap, sbufcap, NSAMPLES, &state);
            memcpy_c(cheri_getdefault(), sbufcap, NSAMPLES*2);
            totalProcessed += NSAMPLES/2;
        }
        memcpy_c(abufcap, adpcmDcap + totalProcessed, remain);
        adpcm_decoder(abufcap, sbufcap, remain*2, &state);
        memcpy_c(cheri_getdefault(), sbufcap, remain*4);
        totalProcessed += remain;
        printf("adpcm decoded %ld bytes in total.\n", totalProcessed);
    }

    return 0;
}
