#include<stdio.h>
#include<stdlib.h>
#include<mips.h>
#include<string.h>
#include<cheric.h>
#include<statcounters.h>

#define FOUR_CAPS // Sweep four mother capabilities at a time.

#define POOL_SIZE (32U << 20)
#define CAPS_PER_LINE (CACHELINE_SIZE/CAP_SIZE)

#define FP_RATE_INV (9999999) // the inverse false-positive rate

// This should be in .bss and zeroed.
static statcounters_bank_t counter_sum_bank_baseline;
static statcounters_bank_t counter_sum_bank_testsub;
static statcounters_bank_t counter_sum_bank_multitag;
static statcounters_bank_t counter_sum_bank_pagedirty;
static statcounters_bank_t counter_sum_bank_all;
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
void sweep_line_nosubset(void*__capability, void*__capability);
void sweep_line_four(void*__capability, void*__capability);

extern char __tags_bin_start, __tags_bin_end;

int
main() {
  size_t theLen = (&__tags_bin_end - &__tags_bin_start) << 6;
  size_t fpPages = (theLen / FP_RATE_INV) >> PAGE_ALIGN_BITS;
  printf("The number of false positive pages is %zd\n", fpPages);
  void*__capability thePCC = cheri_getpcc();
  void*__capability theDDC = cheri_getdefault();
  void*__capability*__capability thePool = calloc_c(POOL_SIZE, 1);
  char*__capability pageBitVec;
  printf("The length of the pool is %zd\n", cheri_getlen(thePool));
  void*__capability testCap = calloc_c(16, 1);

  // the offset at which the next scan begins
  size_t currentOffset = 0;

  // We have limited memory in CheriRTOS, so only do the sweeping for a small
  // fraction each time.
  while(theLen > currentOffset) {
    // We have to clear the pageBitVec each time. Use calloc to do that.
    // Each byte indicates whether a page is dirty.
    pageBitVec = calloc_c(POOL_SIZE>>PAGE_ALIGN_BITS, 1);
    size_t lenToScan = (theLen-currentOffset)>POOL_SIZE? POOL_SIZE :
        (theLen-currentOffset);

    // Populate the POOL with capabilities.
    for(size_t i=0; i<(lenToScan >> CAP_SIZE_BITS); i++) {
      if((*(&__tags_bin_start+(i>>3)+(currentOffset>>6)) & (1<<(i & 0x7))) != 0) {
        *(thePool+i) = testCap;
        *(pageBitVec+(i>>(PAGE_ALIGN_BITS-CAP_SIZE_BITS))) = 1;
      }
      else
        *(thePool+i) = NULLCAP;
    }

    // Add false positive pages.
    size_t tempOffset = 0;
    for(size_t remainCount = 0; remainCount < fpPages/(theLen/POOL_SIZE==0? 1:theLen/POOL_SIZE); remainCount++) {
      while(pageBitVec[tempOffset])
        tempOffset++;
      pageBitVec[tempOffset] = 1;
      tempOffset++;
    }

    // baseline, no magic
    void*__capability*__capability sweeper2 = thePool;
    sample_statcounters(&counter_start);
    for(uint32_t i=0; i<(lenToScan>>CACHELINE_SIZE_BITS); i++) {
      sweep_line_nosubset((void*__capability)sweeper2, thePCC);
#ifdef FOUR_CAPS
      sweep_line_nosubset((void*__capability)sweeper2, theDDC);
      sweep_line_nosubset((void*__capability)sweeper2, thePCC);
      sweep_line_nosubset((void*__capability)sweeper2, theDDC);
#endif // FOUR_CAPS
      sweeper2 += CAPS_PER_LINE;
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    // Add the diff to the final results.
    add_statcounters(&counter_diff, &counter_sum_bank_baseline, &counter_sum_bank_baseline);

    // with ctestsubset
    sweeper2 = thePool;
    sample_statcounters(&counter_start);
    for(uint32_t i=0; i<(lenToScan>>CACHELINE_SIZE_BITS); i++) {
#ifndef FOUR_CAPS
      sweep_line((void*__capability)sweeper2, thePCC);
#else // FOUR_CAPS
      sweep_line_four((void*__capability)sweeper2, thePCC);
#endif // FOUR_CAPS
      sweeper2 += CAPS_PER_LINE;
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    add_statcounters(&counter_diff, &counter_sum_bank_testsub, &counter_sum_bank_testsub);

    // Use creadmultitag instruction.
    sweeper2 = thePool;
    sample_statcounters(&counter_start);
    for(uint32_t i=0; i<(lenToScan>>CACHELINE_SIZE_BITS); i++) {
      if(cheri_getmultitag(sweeper2)) {
        sweep_line_nosubset((void*__capability)sweeper2, thePCC);
#ifdef FOUR_CAPS
        sweep_line_nosubset((void*__capability)sweeper2, theDDC);
        sweep_line_nosubset((void*__capability)sweeper2, thePCC);
        sweep_line_nosubset((void*__capability)sweeper2, theDDC);
#endif // FOUR_CAPS
      }
      sweeper2 += CAPS_PER_LINE;
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    add_statcounters(&counter_diff, &counter_sum_bank_multitag, &counter_sum_bank_multitag);

    // page dirty
    sweeper2 = thePool;
    sample_statcounters(&counter_start);
    for(uint32_t pageNum=0; pageNum<(lenToScan>>PAGE_ALIGN_BITS); pageNum++) {
      if(pageBitVec[pageNum]) {
        for(uint32_t i=0; i<PAGE_ALIGN/CAP_SIZE/CAPS_PER_LINE; i++) {
          sweep_line_nosubset((void*__capability)sweeper2, thePCC);
#ifdef FOUR_CAPS
          sweep_line_nosubset((void*__capability)sweeper2, theDDC);
          sweep_line_nosubset((void*__capability)sweeper2, thePCC);
          sweep_line_nosubset((void*__capability)sweeper2, theDDC);
#endif // FOUR_CAPS
          sweeper2 += CAPS_PER_LINE;
        }
      } else {
        sweeper2 += PAGE_ALIGN/CAP_SIZE;
      }
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    add_statcounters(&counter_diff, &counter_sum_bank_pagedirty, &counter_sum_bank_pagedirty);

    // Use all tricks.
    sweeper2 = thePool;
    sample_statcounters(&counter_start);
    for(uint32_t pageNum=0; pageNum<(lenToScan>>PAGE_ALIGN_BITS); pageNum++) {
      if(pageBitVec[pageNum]) {
        for(uint32_t i=0; i<PAGE_ALIGN/CAP_SIZE/CAPS_PER_LINE; i++) {
          if(cheri_getmultitag(sweeper2)) {
#ifndef FOUR_CAPS
            sweep_line((void*__capability)sweeper2, thePCC);
#else // FOUR_CAPS
            sweep_line_four((void*__capability)sweeper2, thePCC);
#endif // FOUR_CAPS
          }
          sweeper2 += CAPS_PER_LINE;
        }
      } else {
        sweeper2 += PAGE_ALIGN/CAP_SIZE;
      }
    }
    sample_statcounters(&counter_end);
    diff_statcounters(&counter_end, &counter_start, &counter_diff);
    add_statcounters(&counter_diff, &counter_sum_bank_all, &counter_sum_bank_all);

    currentOffset += POOL_SIZE;
    // Avoid memory leaks.
    free_c(pageBitVec);
  }

  printf("Baseline:\n");
  dump_statcounters(&counter_sum_bank_baseline, NULL, NULL);
  printf("With ctestsubset:\n");
  dump_statcounters(&counter_sum_bank_testsub, NULL, NULL);
  printf("With creadmultitag:\n");
  dump_statcounters(&counter_sum_bank_multitag, NULL, NULL);
  printf("With pagedirty:\n");
  dump_statcounters(&counter_sum_bank_pagedirty, NULL, NULL);
  printf("With all:\n");
  dump_statcounters(&counter_sum_bank_all, NULL, NULL);
  // the length of the total coredump memory size
  printf("The length of the coredump is %zd\n", theLen);

  return 0;
}
