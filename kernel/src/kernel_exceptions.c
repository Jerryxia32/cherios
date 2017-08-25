/*-
 * Copyright (c) 2016 Hadrien Barral
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
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
#include"stdio.h"

/*
 * Exception demux
 */

#ifndef __LITE__
static const char* mipscausestr[0x20] = {
	"Interrupt",
	"TLB modification exception",
	"TLB load/fetch exception",
	"TLB store exception",
	"Address load/fetch exception",
	"Address store exception",
	"Bus fetch exception",
	"Bus load/store exception",
	"System call",
	"Breakpoint",
	"Reserved instruction exception",
	"Coprocessor unusable exception",
	"Arithmetic overflow",
	"Trap",
	"Virtual coherency instr. exception",
	"Floating point exception",
	"Reserved",
	"Reserved",
	"Capability coprocessor exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Watchpoint",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Virtual coherency data exception",
};

static const char * capcausestr[0x20] = {
	"None",
	"Length Violation",
	"Tag Violation",
	"Seal Violation",
	"Type Violation",
	"Call Trap",
	"Return Trap",
	"Underflow of trusted system stack",
	"User-defined Permission Violation",
	"TLB prohibits store capability",
	"Requested bounds cannot be represented exactly",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"Global Violation",
	"Permit Execute Violation",
	"Permit Load Violation",
	"Permit Store Violation",
	"Permit Load Capability Violation",
	"Permit Store Capability Violation",
	"Permit Store Local Capability Violation",
	"Permit Seal Violation",
	"Access System Registers Violation",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved"
};

#define exception_printf kernel_printf
#else
#define exception_printf(...)
#endif

static inline const char * getcapcause(int cause) {
	#ifndef __LITE__
		return capcausestr[cause];
	#else
		return ""; cause++;
	#endif
}

extern char*__capability ttableCap;
static void kernel_exception_capability(void) {
	register_t capcause = cheri_getcause();
	int cause = (capcause >> 8) & 0x1F;

	if(cause == 9) { // cp2 TLB no cap store exception
        // clear the store flag and update entrylos
        // need to extract the correct EntryHi before probing
        register_t theBadVaddr = cp0_badvaddr_get();
        // check whether odd page or not
        register_t oddPage = theBadVaddr & PAGE_ALIGN;
        register_t tempLo;
        cp0_tlb_probe();
        cp0_tlb_read();
        // drop the prohibit_cap_store bit
        if(oddPage) {
            tempLo = cp0_entrylo1_get();
            tempLo <<= 1;
            tempLo >>= 1;
            cp0_entrylo1_set(tempLo);
        } else {
            tempLo = cp0_entrylo0_get();
            tempLo <<= 1;
            tempLo >>= 1;
            cp0_entrylo0_set(tempLo);
        }
        // write the corresponding bit in ttable
        theBadVaddr >>= PAGE_ALIGN_BITS;
        size_t charIndex = (theBadVaddr>>3) % TTABLE_SIZE;
        size_t bitIndex = theBadVaddr & 0x07;
        char tempChar = ttableCap[charIndex];
        tempChar |= (0x1 << bitIndex);
        ttableCap[charIndex] = tempChar;

        cp0_tlb_writeidx();
		return;
	}

	int reg_num = capcause & 0xFF;
	exception_printf(KRED "Capability exception caught for activation! %s-%d"
			 " (0x%X: %s) [Reg C%d]" KRST"\n",
			 kernel_acts[kernel_curr_act].name, kernel_curr_act,
			 cause, getcapcause(cause), reg_num);

	regdump(reg_num);
	kernel_freeze();
}

static void kernel_exception_data(register_t excode) {
	exception_printf(KRED"Data abort, %s, BadVAddr:0x%lx in %s-%d"KRST"\n",
			 mipscausestr[excode], cp0_badvaddr_get(),
			 kernel_acts[kernel_curr_act].name, kernel_curr_act);
	regdump(-1);
	kernel_freeze();
}


static void kernel_exception_others(register_t excode) {
	exception_printf(KRED"%s in  %s-%d"KRST"\n",
			 mipscausestr[excode], kernel_acts[kernel_curr_act].name, kernel_curr_act);
	regdump(-1);
    // If this is a breakpoint, just print a regdump and resume execution.
    if(excode != MIPS_CP0_EXCODE_BREAK) kernel_freeze();
    else kernel_skip_instr(kernel_curr_act);
}


/*
 * Exception handler demux to various more specific exception
 * implementations.
 */
void kernel_exception(void) {
	static int entered = 0;
	entered++;
	if(entered > 1) {
		KERNEL_ERROR("interrupt in interrupt: level %d in %s",
			     entered, kernel_acts[kernel_curr_act].name);
	}

	/*
	 * Check assumption that kernel is running at EXL=1.  The kernel is
	 * non-preemptive and will fail horribly if this isn't true.
	 */
	kernel_assert(cp0_status_exl_get() != 0);

	register_t excode = cp0_cause_excode_get();
	switch (excode) {
	case MIPS_CP0_EXCODE_INT:
		kernel_interrupt();
		break;

	case MIPS_CP0_EXCODE_SYSCALL:
		kernel_exception_syscall();
		break;

	case MIPS_CP0_EXCODE_C2E:
		kernel_exception_capability();
		break;

	case MIPS_CP0_EXCODE_TLBL:
	case MIPS_CP0_EXCODE_TLBS:
	case MIPS_CP0_EXCODE_ADEL:
	case MIPS_CP0_EXCODE_ADES:
	case MIPS_CP0_EXCODE_IBE:
	case MIPS_CP0_EXCODE_DBE:
		kernel_exception_data(excode);
		break;

	default:
		kernel_exception_others(excode);
		break;
	}

	entered--;
	if(entered) {
		KERNEL_ERROR("interrupt in interrupt: level %d in %s",
			     entered, kernel_acts[kernel_curr_act].name);
		kernel_freeze();
	}
}
