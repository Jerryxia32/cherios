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
#include "sys/mman.h"
#include "object.h"
#include "namespace.h"

static void * memmgt_ref = NULL;
static void * memmgt_id  = NULL;

#define MALLOC_FASTPATH

#ifndef MALLOC_FASTPATH
__capability void *malloc_c(size_t length) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	return ccall_4(memmgt_ref, memmgt_id, 0, length, 0, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}

__capability void *calloc_c(size_t items, size_t length) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	return ccall_4(memmgt_ref, memmgt_id, 1, items, length, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}

__capability void *realloc_c(__capability void *ptr, size_t length) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	return ccall_4(memmgt_ref, memmgt_id, 2, length, 0, 0, ptr, NULLCAP, NULLCAP).cret;
}

void free_c(__capability void *ptr) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	ccall_4(memmgt_ref, memmgt_id, 3, 0, 0, 0, ptr, NULLCAP, NULLCAP);
}

__capability void *malloc_c_c(size_t length) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	return ccall_4(memmgt_ref, memmgt_id, 4, length, 0, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}

__capability void *calloc_c_c(size_t items, size_t length) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	return ccall_4(memmgt_ref, memmgt_id, 5, items, length, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}

#else /* ifdef MALLOC_FASTPATH */
static int mallocInit = 0;
static __capability void * memmgt_PCC = NULLCAP;
static __capability void * memmgt_IDC = NULLCAP;
extern void __capability *helper_cap;

void
init_memmgt() {
    memmgt_PCC = namespace_get_PCC(3);
    memmgt_IDC  = namespace_get_IDC(3);
    helper_cap = namespace_get_PCC(7);
    mallocInit = 1;
}

__capability void *malloc_c(size_t length) {
    if(mallocInit == 0)
        init_memmgt();
	return ccall_real_4_c(0, LONG_MAX, length, 0, 0, NULLCAP, NULLCAP, NULLCAP, memmgt_PCC, memmgt_IDC, helper_cap);
}

__capability void *calloc_c(size_t items, size_t length) {
    if(mallocInit == 0)
        init_memmgt();
	return ccall_real_4_c(1, LONG_MAX, items, length, 0, NULLCAP, NULLCAP, NULLCAP, memmgt_PCC, memmgt_IDC, helper_cap);
}

__capability void *realloc_c(__capability void *ptr, size_t length) {
    if(mallocInit == 0)
        init_memmgt();
	return ccall_real_4_c(2, LONG_MAX, length, 0, 0, ptr, NULLCAP, NULLCAP, memmgt_PCC, memmgt_IDC, helper_cap);
}

void free_c(__capability void *ptr) {
    if(mallocInit == 0)
        init_memmgt();
	ccall_real_4_c(3, LONG_MAX, 0, 0, 0, ptr, NULLCAP, NULLCAP, memmgt_PCC, memmgt_IDC, helper_cap);
}

__capability void *malloc_c_c(size_t length) {
    if(mallocInit == 0)
        init_memmgt();
	return ccall_real_4_c(4, LONG_MAX, length, 0, 0, NULLCAP, NULLCAP, NULLCAP, memmgt_PCC, memmgt_IDC, helper_cap);
}

__capability void *calloc_c_c(size_t items, size_t length) {
    if(mallocInit == 0)
        init_memmgt();
	return ccall_real_4_c(5, LONG_MAX, items, length, 0, NULLCAP, NULLCAP, NULLCAP, memmgt_PCC, memmgt_IDC, helper_cap);
}
#endif /* MALLOC_FASTPATH */

void memmgt_set_act(void * ref, void * id) {
	memmgt_ref = ref;
	memmgt_id  = id;
}

__capability void *calloc_core(size_t items, size_t length) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	return ccall_4(memmgt_ref, memmgt_id, 6, items, length, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}

void free_core(__capability void *ptr) {
	if(memmgt_ref == NULL) {
		memmgt_ref = namespace_get_ref(3);
		memmgt_id  = namespace_get_id(3);
	}
	ccall_4(memmgt_ref, memmgt_id, 3, 0, 0, 0, ptr, NULLCAP, NULLCAP);
}
