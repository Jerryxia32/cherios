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

static uint32_t sealing_tool_no = 1;

static void * init_act_register(reg_frame_t * frame, const char * name) {
	void * ret;
	__asm__ __volatile__ (
		"li    $v1, 20       \n"
		"move $a0, %[frame] \n"
		"move $a1, %[name]  \n"
		"syscall             \n"
		"move %[ret], $a0   \n"
		: [ret] "=r" (ret)
		: [frame] "r" (frame), [name] "r" (name)
		: "v0", "v1", "a0", "a1");
	return ret;
}

static void * init_act_create(const char * name, void * __capability c0, void * __capability pcc, void * stack,
			      void * __capability act_cap, void * ns_ref, void * ns_id,
			      register_t rarg, const void * carg) {
	reg_frame_t frame;
	memset(&frame, 0, sizeof(reg_frame_t));

	/* set pc */
    frame.cf_pcc = pcc;
	frame.mf_pc	= cheri_getoffset(pcc);

	/* set stack */
	frame.mf_sp	= (size_t)stack;

    /* set up per activation trusted stack */
    void * __capability temp = cheri_getdefault();
    temp = cheri_andperm(temp, cheri_getperm(c0));
    temp = cheri_setoffset(temp, cheri_getbase(c0) - PAGE_ALIGN + MALLOC_HEADER_SIZE);
    frame.cf_kr1c = cheri_setbounds(temp, PAGE_ALIGN - MALLOC_HEADER_SIZE);

	/* set c0 */
    frame.cf_c0 = c0;

    /* ugly hack, get an uncached cap at a fixed location of c0 */
    size_t uncachedBase = cheri_getbase(c0) + 0x20000000;
    size_t uncachedLen = cheri_getlen(c0);
    capability uncachedC0 = cheri_setbounds(cheri_setoffset(cheri_getdefault(), uncachedBase), uncachedLen);
    uncachedC0 = cheri_andperm(uncachedC0, cheri_getperm(c0));
    *((capability * __capability)c0 + 0x200/CAP_SIZE) = uncachedC0;

	/* set cap */
	frame.cf_c6	= act_cap;

	/* set namespace */
	frame.mf_s0	= (register_t)ns_ref;
	frame.mf_s1	= (register_t)ns_id;

	void * ctrl = init_act_register(&frame, name);
	CCALL(1, act_ctrl_get_ref(ctrl), act_ctrl_get_id(ctrl), 0,
	      rarg, (register_t)carg, (register_t)ctrl, NULLCAP, NULLCAP, NULLCAP);
	return ctrl;
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
#define	UART_BASE	0x7f000000
#define	UART_SIZE	0x08
#else
#error UART type not found
#endif

        /*
		cap = cheri_getdefault();
		cap = cheri_setoffset(cap, mips_phys_to_uncached(UART_BASE));
		cap = cheri_setbounds(cap, UART_SIZE);
         */
		break;
	case m_memmgt:{}
        size_t heaplen = (size_t)&__stop_heap - (size_t)&__start_heap;
        void * __capability heap = cheri_setoffset(cheri_getdefault(), (size_t)&__start_heap);
        heap = cheri_setbounds(heap, heaplen);
        cap = cheri_andperm(heap, (CHERI_PERM_GLOBAL | CHERI_PERM_LOAD | CHERI_PERM_STORE
                    | CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP
                    | CHERI_PERM_STORE_LOCAL_CAP | CHERI_PERM_SOFT_0));
        sealing_tool_no++;
        if(sealing_tool_no == 1L<<CHERI_OTYPE_WIDTH) {
            panic("Used all otypes");
        }
        void * __capability mem_seal_tool = cheri_getdefault();
        mem_seal_tool = cheri_setoffset(mem_seal_tool, sealing_tool_no);
        mem_seal_tool = cheri_setbounds(mem_seal_tool, 1);
        mem_seal_tool = cheri_andperm(mem_seal_tool, (CHERI_PERM_SEAL));
        *(capability * __capability)cap = mem_seal_tool;
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

static void * ns_ref = NULL;
static void * ns_id  = NULL;

static void * __capability elf_loader(Elf_Env *env, const char * file, size_t *maxaddr, size_t * entry) {
	int filelen=0;
	char * addr = load(file, &filelen);
	if(!addr) {
		printf("Could not read file %s", file);
		return NULLCAP;
	}
	return elf_loader_mem(env, addr, NULL, maxaddr, entry, 0);
}

static void * __capability init_memcpy(void * __capability dest, const void * __capability src, size_t n) {
	return memcpy_c(dest, src, n);
}

static void *make_aligned_data_addr(const char *start) {
	size_t desired_ofs = ((size_t)start + PAGE_ALIGN);
	desired_ofs &= ~(PAGE_ALIGN-1);

	char *cap = (char *)desired_ofs;
	return cap;
}

void * load_module(module_t type, const char * file, int arg, const void *carg) {
	size_t entry;
    size_t allocsize;
	Elf_Env env = {
	  .alloc   = init_alloc,
	  .free    = init_free,
	  .printf  = printf,
	  .vprintf = vprintf,
	  .memcpy_c  = init_memcpy,
	};

	char * __capability prgmp = elf_loader(&env, file, &allocsize, &entry);
    printf(KWHT"Module loaded at %p, entry: %lx, size: %lx"KRST"\n", (void *)cheri_getbase(prgmp), entry, allocsize);
	if(!prgmp) {
		assert(0);
		return NULL;
	}

	/* Invalidate the whole range; elf_loader only returns a
	   pointer to the entry point. */
	caches_invalidate((void *)cheri_getbase(prgmp), allocsize);

	//prgmp += entry;

    /* Make the stack pointer cap size aligned */
	void * stack = (void *)(cheri_getlen(prgmp) - 2*_MIPS_SZCAP/8);
    printf(KWHT"Stack bottom at %p"KRST"\n", stack);
	void * __capability pcc = cheri_getpcc();
	pcc = cheri_setbounds(cheri_setoffset(pcc, cheri_getbase(prgmp)), (allocsize + PAGE_ALIGN) & ~(PAGE_ALIGN-1));
	pcc = cheri_incoffset(pcc, entry);
	pcc = cheri_andperm(pcc, (CHERI_PERM_ACCESS_SYS_REGS | CHERI_PERM_EXECUTE | CHERI_PERM_LOAD
				  | CHERI_PERM_LOAD_CAP));
    /* make sure the permission given to c0 of each module is bounded */
    prgmp = cheri_andperm(prgmp, (CHERI_PERM_GLOBAL | CHERI_PERM_LOAD | CHERI_PERM_STORE
                | CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP
                | CHERI_PERM_STORE_LOCAL_CAP | CHERI_PERM_SOFT_0));
    if(type != m_memmgt) {
        prgmp = cheri_andperm(prgmp, ~CHERI_PERM_GLOBAL);
    }
    if(type != m_ccall_helper) {
        pcc = cheri_andperm(pcc, ~CHERI_PERM_ACCESS_SYS_REGS);
    }

	void * ctrl = init_act_create(file, cheri_setoffset(prgmp, 0),
				      pcc, stack, get_act_cap(type),
				      ns_ref, ns_id, arg, carg);
	if(ctrl == NULL) {
		return NULL;
	}
	if(type == m_namespace) {
		ns_ref = act_ctrl_get_ref(ctrl);
		ns_id = act_ctrl_get_id(ctrl);
	}
	return ctrl;
}

static int act_alive(void * ctrl) {
	if(!ctrl) {
		return 0;
	}
	int ret;
	__asm__ __volatile__ (
		"li    $v1, 23      \n"
		"move $a0, %[ctrl] \n"
		"syscall            \n"
		"move %[ret], $v0   \n"
		: [ret] "=r" (ret)
		: [ctrl] "r" (ctrl)
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
		if((!be->daemon) && act_alive(be->ctrl)) {
			nb++;
			break;
		}
	}
	//printf(KRED"%d still alive\n", nb);
	return nb;
}
