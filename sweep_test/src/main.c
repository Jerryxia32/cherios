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

uint32_t
gen_rand() {
  static uint64_t initial = 1727000011183999444ULL;
  initial = initial*6364136223846793005ULL + 1;
  return initial>>32;
}

void sweep_line(void*__capability, void*__capability);

int
main() {
  void*__capability thePCC = cheri_getpcc();
  void*__capability*__capability thePool = calloc_c(32U << 20, 1);
  printf("The length of the pool is %zd\n", cheri_getlen(thePool));
  void*__capability testCap = calloc_c(32U << 10, 1);
  for(int i=0; i<(1 << 19); i++) {
    *(thePool+gen_rand()%(2 << 20)) = testCap;
    //*(thePool+i) = testCap;
  }

  stats_init();
  void*__capability (*__capability sweeper)[16] = (void*__capability (*__capability)[16])thePool;
  for(uint32_t i=0; i<(32U<<13); i++) {
    if(cheri_getmultitag(sweeper)) {
      sweep_line((void*__capability)sweeper, thePCC);
    }
    sweeper++;
  }
  stats_display();

  stats_init();
  void*__capability*__capability sweeper2 = thePool;
  for(uint32_t i=0; i<(32U<<13); i++) {
    sweep_line((void*__capability)sweeper2, thePCC);
    sweeper2 += 16;
  }
  stats_display();

  return 0;
}
