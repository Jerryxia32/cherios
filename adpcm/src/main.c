#include"adpcm.h"
#include<stdio.h>
#include<string.h>
#include<cheric.h>
#include<mibench_iter.h>
#include<statcounters.h>

struct adpcm_state state;

#define NSAMPLES 1024

extern char __adpcm_c_start, __adpcm_c_end;
extern char __adpcm_d_start, __adpcm_d_end;

int
main() {
    char	abuf[NSAMPLES/2] __attribute__((aligned(CAP_SIZE)));
    short	sbuf[NSAMPLES] __attribute__((aligned(CAP_SIZE)));

    size_t totalProcessed = 0;
    size_t remain = 0;
    size_t cSize = &__adpcm_c_end - &__adpcm_c_start;
    size_t dSize = &__adpcm_d_end - &__adpcm_d_start;

    stats_init();
    for(int n=0; n<ADPCM_ITER; n++) {

        /* encode stage */
        totalProcessed = 0;
        remain = 0;
        while((remain = cSize - totalProcessed) > NSAMPLES*2) {
            memcpy(sbuf, &__adpcm_c_start + totalProcessed, NSAMPLES*2);
            adpcm_coder(sbuf, abuf, NSAMPLES, &state);
            memcpy(cheri_getdefault(), abuf, NSAMPLES/2);
            totalProcessed += NSAMPLES*2;
        }
        memcpy(sbuf, &__adpcm_c_start + totalProcessed, remain);
        adpcm_coder(sbuf, abuf, remain/2, &state);
        memcpy(cheri_getdefault(), abuf, remain/4);
        totalProcessed += remain;
        printf("adpcm encoded %ld bytes in total.\n", totalProcessed);

        /* decode stage, reinitialize variables. */
        totalProcessed = 0;
        remain = 0;
        while((remain = dSize - totalProcessed) > NSAMPLES/2) {
            memcpy(abuf, &__adpcm_d_start + totalProcessed, NSAMPLES/2);
            adpcm_decoder(abuf, sbuf, NSAMPLES, &state);
            memcpy(cheri_getdefault(), sbuf, NSAMPLES*2);
            totalProcessed += NSAMPLES/2;
        }
        memcpy(abuf, &__adpcm_d_start + totalProcessed, remain);
        adpcm_decoder(abuf, sbuf, remain*2, &state);
        memcpy(cheri_getdefault(), sbuf, remain*4);
        totalProcessed += remain;
        printf("adpcm decoded %ld bytes in total.\n", totalProcessed);
    }
    stats_display();

    return 0;
}
