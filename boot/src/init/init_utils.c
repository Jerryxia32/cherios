/*-
 * Copyright (c) 2016 Hongyan Xia
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

#include "mips.h"
#include"cheric.h"
#include "cp0.h"
#include "plat.h"
#include "init.h"
#include "object.h"
#include "string.h"
#include "uart.h"
#include "assert.h"
#include "stdio.h"
#include "elf.h"
#include"precision.h"
#include"sched.h"

static uint32_t sealing_tool_no = 1;

static aid_t
init_act_register(reg_frame_t* frame, const char* name, prio_t priority) {
	aid_t ret;
	__asm__ __volatile__ (
		"li    $v1, 20       \n"
		"move $a0, %[frame] \n"
		"move $a1, %[name]  \n"
		"move $a2, %[priority]  \n"
		"syscall             \n"
		"move %[ret], $v0   \n"
		: [ret] "=r" (ret)
		: [frame] "r" (frame), [name] "r" (name), [priority] "r" (priority)
		: "v0", "v1", "a0", "a1", "a2");
	return ret;
}

static aid_t
init_act_create(const char* name, void*__capability c0,
        void* __capability pcc, void* stack, void*__capability act_cap,
        aid_t ns_aid, register_t rarg, const void* carg, prio_t priority) {
	reg_frame_t frame;
	memset(&frame, 0, sizeof(reg_frame_t));

	/* set pc */
    frame.cf_pcc = pcc;
	frame.mf_pc	= addr_to_reg(cheri_getoffset(pcc));

	/* set stack */
	frame.mf_sp	= addr_to_reg((size_t)stack);

    /* set up per activation trusted stack
     * the trusted stack is put in cached, unmapped kernel address space
     * as we definitely do not want to sweep that region
     */
    void*__capability temp = init_alloc(28*CAP_SIZE);
    frame.cf_kr1c = cheri_setbounds(cheri_setoffset(cheri_getdefault(),
            cheri_getbase(temp) | 0x80000000), cheri_getlen(temp));

	/* set c0 */
    frame.cf_c0 = c0;

	/* set cap */
	frame.cf_c6	= act_cap;

	/* set namespace */
	frame.mf_s0	= (register_t)ns_aid;

	aid_t thisAid = init_act_register(&frame, name, priority);
	CCALL(1, thisAid, 0,
	      rarg, (register_t)carg, thisAid, NULLCAP, NULLCAP, NULLCAP);
	return thisAid;
}

/* Return the capability needed by the activation */
static void * __capability get_act_cap(module_t type) {
	void * __capability cap = NULLCAP;
	switch(type) {
	case m_uart:{}

#ifdef CONSOLE_malta
#define	UART_BASE	0x180003f8
#define	UART_SIZE	0x40
#elif defined(CONSOLE_altera)
#define	UART_BASE	0xff000000
#define	UART_SIZE	0x08
#else
#error UART type not found
#endif
    cap = cheri_getdefault();
    cap = cheri_setoffset(cap, mips_phys_to_uncached(UART_BASE));
    cap = cheri_setbounds(cap, UART_SIZE);
    break;
	case m_memmgt:{}
        size_t heaplen = (size_t)&__stop_heap - (size_t)&__start_heap;
        void * __capability heap = cheri_setoffset(cheri_getdefault(), (size_t)&__start_heap);
        heap = cheri_setbounds(heap, heaplen);
        cap = cheri_andperm(heap, (CHERI_PERM_GLOBAL | CHERI_PERM_LOAD
                | CHERI_PERM_STORE | CHERI_PERM_CCALL | CHERI_PERM_LOAD_CAP
                | CHERI_PERM_STORE_CAP | CHERI_PERM_STORE_LOCAL_CAP
                | CHERI_PERM_SOFT_0));
        break;

		break;
	case m_fs:{}
        /*
		void * mmio_cap = (void *)mips_phys_to_uncached(0x1e400000);
        cap = mmio_cap;
		break;
         */
	case m_namespace:
        break;
	case m_core:
	case m_user:
	case m_ccall_helper:
        /* return a capability with permit_seal permission of otype aid */
        sealing_tool_no++;
        if(sealing_tool_no == 1L<<CHERI_OTYPE_WIDTH) {
            panic("Used all otypes");
        }
        cap = cheri_getdefault();
        cap = cheri_setoffset(cap, sealing_tool_no);
        cap = cheri_setbounds(cap, 1);
        cap = cheri_andperm(cap, (CHERI_PERM_SEAL));
        break;
	case m_fence:
	default:{}
	}
	return cap;
}

static aid_t ns_aid = 0;

static void*__capability
elf_loader(Elf_Env* env, const char* file, size_t* maxaddr, size_t* entry,
        void*__capability* allocPCC) {
	int filelen=0;
	char * addr = load(file, &filelen);
	if(!addr) {
		printf("Could not read file %s", file);
		return NULLCAP;
	}
	return elf_loader_mem(env, addr, NULL, maxaddr, entry, allocPCC, 0);
}

static void * __capability init_memcpy(void * __capability dest, const void * __capability src, size_t n) {
	return memcpy_c(dest, src, n);
}

aid_t
load_module(module_t type, const char* file, int arg, const void* carg) {
	size_t entry;
    size_t allocsize;
    void*__capability allocPCC;
	Elf_Env env = {
	  .alloc   = init_alloc,
	  .free    = init_free,
	  .printf  = printf,
	  .vprintf = vprintf,
	  .memcpy_c  = init_memcpy,
	};

	char * __capability prgmp =
            elf_loader(&env, file, &allocsize, &entry, &allocPCC);
    printf(KWHT"Module PCC loaded at %p, size: %x"KRST"\n", (void *)cheri_getbase(allocPCC), cheri_getlen(allocPCC));
    printf(KWHT"Module DDC loaded at %p, size: %x"KRST"\n", (void *)cheri_getbase(prgmp), allocsize);
	if(!prgmp || !allocPCC) {
		assert(0);
		return 0;
	}

	/* Invalidate the whole range; elf_loader only returns a
	   pointer to the entry point. */
	caches_invalidate((void *)cheri_getbase(allocPCC), cheri_getlen(allocPCC));

    /* Make the stack pointer cap size aligned */
	void * stack = (void *)(cheri_getlen(prgmp) - 2*_MIPS_SZCAP/8);
    printf(KWHT"Stack bottom at %p"KRST"\n", stack);
	void * __capability pcc = cheri_getpcc();
	pcc = cheri_setbounds(cheri_setoffset(pcc, cheri_getbase(allocPCC)), cheri_getlen(allocPCC));
	pcc = cheri_incoffset(pcc, entry);
    pcc = cheri_andperm(pcc, (CHERI_PERM_ACCESS_SYS_REGS | CHERI_PERM_EXECUTE
            | CHERI_PERM_LOAD | CHERI_PERM_LOAD_CAP | CHERI_PERM_CCALL));
    /* make sure the permission given to c0 of each module is bounded */
    prgmp = cheri_andperm(prgmp, (CHERI_PERM_GLOBAL | CHERI_PERM_LOAD | CHERI_PERM_STORE
            | CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP | CHERI_PERM_CCALL
            | CHERI_PERM_STORE_LOCAL_CAP | CHERI_PERM_SOFT_0));

    prio_t priority = PRIORITY_DEFAULT;

    if(type != m_memmgt) {
        prgmp = cheri_andperm(prgmp, ~CHERI_PERM_GLOBAL);
    }
    if(type != m_ccall_helper) {
        pcc = cheri_andperm(pcc, ~CHERI_PERM_ACCESS_SYS_REGS);
    }
    if(type != m_user) {
        priority = PRIORITY_MAX-1;
    }
    if(type==m_uart || type==m_memmgt) {
      // Very hacky. We don't have enough cap registers, so just pass the
      // sealing capability at the first location of the user's DDC.
      sealing_tool_no++;
      if(sealing_tool_no == 1L<<CHERI_OTYPE_WIDTH) {
          panic("Used all otypes");
      }
      void*__capability cap = cheri_getdefault();
      cap = cheri_setoffset(cap, sealing_tool_no);
      cap = cheri_setbounds(cap, 1);
      cap = cheri_andperm(cap, (CHERI_PERM_SEAL));
      *(capability*__capability)prgmp = cap;
    }

    aid_t thisAid = init_act_create(file, cheri_setoffset(prgmp, 0),
            pcc, stack, get_act_cap(type), ns_aid, arg, carg, priority);
	if(thisAid == 0) {
		return 0;
	}
	if(type == m_namespace) {
		ns_aid = thisAid;
	}
	return thisAid;
}

static int
act_alive(aid_t aid) {
	if(!aid) {
		return 0;
	}
	int ret;
	__asm__ __volatile__ (
		"li    $v1, 23      \n"
		"move $a0, %[aid] \n"
		"syscall            \n"
		"move %[ret], $v0   \n"
		: [ret] "=r" (ret)
		: [aid] "r" (aid)
		: "v0", "v1", "a0");
	if(ret == 2) {
		return 0;
	}
	return 1;
}

int acts_alive(init_elem_t * init_list, size_t  init_list_len) {
	int nb = 0;
	for(size_t i=0; i<init_list_len; i++) {
		init_elem_t * be = init_list + i;
		if((!be->daemon) && act_alive(be->aid)) {
			nb++;
			break;
		}
	}
	//printf(KRED"%d still alive\n", nb);
	return nb;
}
