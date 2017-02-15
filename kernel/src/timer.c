/*-
 * Copyright (c) 2011 Robert N. M. Watson
 * Copyright (c) 2016 Hadrien Barral
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "klib.h"
#include "cp0.h"

static register_t		kernel_last_timer;

void kernel_timer_init(void) {
	/*
	 * Start timer.
	 */
	KERNEL_TRACE("init", "starting timer");
	kernel_last_timer = cp0_count_get();
	kernel_last_timer += TIMER_INTERVAL;
	cp0_compare_set(kernel_last_timer);
}

static inline register_t TMOD(register_t count) {
	return count & 0xFFFFFFFF;
}

/*
 * Kernel timer handler -- reschedule, reset timer.
 */
void kernel_timer(void)
{
	KERNEL_TRACE(__func__, "in %x", cp0_count_get());

	/*
	 * Forced context switch of user process.
	 */
	aid_t hint = sched_reschedule(0);

    /*
     * Scan the context's trusted stack to check for expired ccalls.
     * hint 1 is init which doesn't need a trusted stack
     */
    if(hint > 1) {
        capability __capability *oriStack = kernel_exception_framep_ptr->cf_kr1c;
        capability __capability *tStack = cheri_setoffset(oriStack, 0);

        // scan and pop the stack
        while(tStack < oriStack) {
            //check if the current frame expires
            int32_t __capability *lastTimep = (int32_t __capability *)((char __capability *)tStack + 2*CAP_SIZE + sizeof(register_t));
            int64_t __capability *remainTimep = (int64_t __capability *)((char __capability *)tStack + 2*CAP_SIZE);
            int32_t elapsed = cp0_count_get() - *lastTimep;
            *lastTimep += elapsed;
            *remainTimep -= elapsed;

            // timeout reached, force return
            if(*remainTimep <= 0) {
                //kernel_printf(KRED"CCall expired in %s, force pop."KRST"\n", kernel_acts[hint].name);

                // first, release the current callee mutex
                *((int __capability *)kernel_exception_framep_ptr->cf_c0 + 0x100/sizeof(int)) = 0;

                // the callee failed to meet timing, clear almost all registers.
                for(size_t i=0; i<(sizeof(register_t)*32 + CAP_SIZE*12)/CAP_SIZE; i++) {
                    *((capability __capability *)kernel_exception_framep_ptr + i) = NULLCAP;
                } 
                kernel_exception_framep_ptr->cf_pcc = *tStack;
                kernel_exception_framep_ptr->mf_pc = cheri_getoffset(*tStack);
                void __capability *theC0 = *(tStack + 1);
                kernel_exception_framep_ptr->mf_sp = cheri_getoffset(theC0);
                kernel_exception_framep_ptr->cf_c0 = cheri_setoffset(theC0, 0);
                
                // release all held mutexes
                for(size_t theOffset = 3; tStack + theOffset < oriStack; theOffset += 3) {
                    *(int __capability *)cheri_setoffset(*(tStack + theOffset + 1), 0x100) = 0;
                }
                break;
            }

            //check the next
            tStack += 3;
        }

        // reinstall kr1c
        kernel_exception_framep_ptr->cf_kr1c = tStack;
    }

	/*
	 * Reschedule timer for a future date -- if we've almost missed a
	 * tick, better to defer.
	 */
	/* count register is 32 bits */
    /*
	register_t next_timer = TMOD(kernel_last_timer + TIMER_INTERVAL);
	while (next_timer < TMOD(cp0_count_get() + TIMER_INTERVAL_MIN)) {
		next_timer = TMOD(next_timer + TIMER_INTERVAL);
	}
	cp0_compare_set(next_timer);	//Also clears pending interrupt.

	kernel_last_timer = next_timer;
     */
	kernel_last_timer = TMOD(cp0_count_get() + TIMER_INTERVAL);
	cp0_compare_set(kernel_last_timer);	//Also clears pending interrupt.
}
