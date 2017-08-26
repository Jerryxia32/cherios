/*-
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

#include "lib.h"
#include "malloc_heap.h"

void * __capability sealing_tool = NULLCAP;

extern void msg_entry;
extern char msg_entry_ccall;
void (*msg_methods[]) = {__malloc_c, __calloc_c, __realloc_c, __free_c, __calloc_core};
size_t msg_methods_nb = countof(msg_methods);
void (*ctrl_methods[]) = {NULL, ctor_null, dtor_null, register_ns};
size_t ctrl_methods_nb = countof(ctrl_methods);

size_t pagesz;			/* page size */

void
register_ns(aid_t ns_aid) {
	namespace_init(ns_aid);
	int ret = namespace_register(PORT_MEMMGT, act_self_aid, act_self_PCC, act_self_IDC);
	if(ret!=0) {
		syscall_puts(KRED"Register failed\n");
	}
}

int main(void) {
	syscall_puts("memmgt Hello world\n");

	/* Get capability to heap */
	void * __capability heap = act_get_cap();
	CHERI_PRINT_CAP(heap);
	assert(heap != NULLCAP);
    sealing_tool = *(capability * __capability)heap;
	CHERI_PRINT_CAP(sealing_tool);
    act_self_PCC = cheri_setoffset(act_self_PCC, (size_t)&msg_entry_ccall);
    act_self_PCC = cheri_seal(act_self_PCC, sealing_tool);
    act_self_IDC = cheri_seal(act_self_IDC, sealing_tool);

	/*
	 * setup memory and
	 * align break pointer so all data will be page aligned.
	 */
	pagesz = CHERIOS_PAGESIZE;
	init_pagebucket();
	__init_heap(heap);

	/* init release mecanism */
	// XXX no gc for now
    //release_init();

	syscall_puts("memmgt: setup done\n");

	msg_enable = 1; /* Go in waiting state instead of exiting */
	return 0;
}
