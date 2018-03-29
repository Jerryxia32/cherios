#include<stdio.h>
#include<stdlib.h>
#include<mips.h>
#include<string.h>
#include<cheric.h>
#include<statcounters.h>

#define POOL_SIZE (64U << 20)
#define CAPS_PER_LINE (CACHELINE_SIZE/CAP_SIZE)

// This should be in .bss and zeroed.
static statcounters_bank_t counter_sum_bank1;
static statcounters_bank_t counter_sum_bank2;
static statcounters_bank_t counter_start;
static statcounters_bank_t counter_end;
static statcounters_bank_t counter_diff;

// simple random number generator
static uint32_t
gen_rand() {
  static uint64_t initial = 1727000011183999444ULL;
  initial = initial*6364136223846793005ULL + 1;
  return initial>>32;
}

void sweep_line(void*__capability, void*__capability);

extern char __tags_bin_start, __tags_bin_end;

int
main() {
  // the length of the total coredump memory size
  size_t theLen = (&__tags_bin_end - &__tags_bin_start) << 6;
  printf("The length of the coredump is %zd\n", theLen);
  void*__capability thePCC = cheri_getpcc();
  void*__capability*__capability thePool = calloc_c(POOL_SIZE, 1);
  printf("The length of the pool is %zd\n", cheri_getlen(thePool));
  void*__capability testCap = calloc_c(16, 1);

  // the offset at which the next scan begins
  size_t currentOffset = 0;

  // We have limited memory in CheriRTOS, so only do the sweeping for a small
  // fraction each time.
  while(theLen > currentOffset) {
    size_t lenToScan = (theLen-currentOffset)>POOL_SIZE? POOL_SIZE :
        (theLen-currentOffset);

    // Populate the POOL with capabilities.
    for(size_t i=0; i<(lenToScan >> CAP_SIZE_BITS); i++) {
      if((*(&__tags_bin_start+(i>>3)+(currentOffset>>6)) & (1<<(i & 0x7))) != 0)
        *(thePool+i) = testCap;
      else
        *(thePool+i) = NULLCAP;
    }

    void*__capability (*__capability sweeper)[CAPS_PER_LINE] =
        (void*__capability (*__capability)[CAPS_PER_LINE])thePool;
    sample_statcounters(&counter_start);
    for(uint32_t i=0; i<(lenToScan>>CACHELINE_SIZE_BITS); i++) {
      if(cheri_getmultitag(sweeper)) {
        sweep_line((void*__capability)sweeper, thePCC);
      }
      sweeper++;
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    // Add the diff to the final results.
    add_statcounters(&counter_diff, &counter_sum_bank1, &counter_sum_bank1);

    void*__capability*__capability sweeper2 = thePool;
    sample_statcounters(&counter_start);
    for(uint32_t i=0; i<(lenToScan>>CACHELINE_SIZE_BITS); i++) {
      sweep_line((void*__capability)sweeper2, thePCC);
      sweeper2 += CAPS_PER_LINE;
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    add_statcounters(&counter_diff, &counter_sum_bank2, &counter_sum_bank2);

    currentOffset += POOL_SIZE;
  }

  dump_statcounters(&counter_sum_bank1, NULL, NULL);
  dump_statcounters(&counter_sum_bank2, NULL, NULL);

  return 0;
}
