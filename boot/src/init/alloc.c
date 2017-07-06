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
#include "string.h"
#include "stdlib.h"
#include "sys/mman.h"
#include "object.h"
#include"stdio.h"
#include"precision.h"

static inline void *align_upwards(void *p, uintptr_t align)
{
	uint8_t * addr = (uint8_t *)p;
	uintptr_t offset = (uintptr_t)addr - ((uintptr_t)addr & ~(align-1));
	if(offset > 0) {
		addr += align - offset;
	}
	return (void *)addr;
}

static const size_t pool_size = 1024*160;
static char pool[pool_size] __attribute__((aligned(0x1000)));

static char * pool_start = NULL;
static char * pool_end = NULL;
static char * pool_next = NULL;

static int system_alloc = 0;

static void * __capability init_alloc_core(size_t s) {
    size_t roundedSize = round_size(s, CHERI_SEAL_TB_WIDTH-1);
	pool_next = align_upwards(pool_next, align_chunk(s, CHERI_SEAL_TB_WIDTH-1));
	if(pool_next + roundedSize >= pool_end) {
		return NULLCAP;
	}
    void * __capability p = cheri_getdefault();
    p = cheri_setoffset(p, (size_t)pool_next);
    p = cheri_setbounds(p, roundedSize);
	pool_next = (void *)((size_t)pool_next + roundedSize);
	return p;
}

void init_alloc_init(void) {
	pool_start = (char *)(pool);
	pool_end = pool + pool_size;
	pool_next = pool_start;
	bzero(pool, pool_size);
	system_alloc = 0;
}

void init_alloc_enable_system(void * c_memmgt) {
	memmgt_set_act(act_ctrl_get_ref(c_memmgt), act_ctrl_get_id(c_memmgt));
	system_alloc = 1;
    printf("System alloc (memmgt module) enabled.\n");
}

void * __capability init_alloc(size_t s) {
	if(system_alloc == 1) {
		void * __capability p = calloc_core(1, s);
		if(!p) {
			return NULLCAP;
		}
		return p;
	}
	return init_alloc_core(s);
}

void init_free(void * __capability p) {
	if(system_alloc == 1) {
        free_core(p);
	}
	/* init alloc has no free */
}
