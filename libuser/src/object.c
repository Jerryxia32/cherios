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
#include "object.h"
#include "assert.h"
#include "namespace.h"

void * act_self_ctrl = NULL;
void * act_self_ref  = NULL;
void * __capability act_self_PCC = NULLCAP;
void * __capability act_self_IDC = NULLCAP;
void * __capability act_self_cap   = NULLCAP;
void * __capability helper_cap   = NULLCAP;
void * __capability return_cap   = NULLCAP;

void object_init(void * self_ctrl, void * __capability self_cap, void * __capability self_PCC, void * __capability self_IDC) {
	assert(self_ctrl != NULL);
	act_self_ctrl = self_ctrl;
	act_self_ref  = act_ctrl_get_ref(self_ctrl);
    act_self_PCC = self_PCC;
    act_self_IDC = self_IDC;

	act_self_cap = self_cap;
}

void * __capability act_get_cap(void) {
	return act_self_cap;
}

void * act_ctrl_get_ref(void * ctrl) {
	void * ref;
	__asm__ (
		"li    $v1, 21      \n"
		"move $a0, %[ctrl] \n"
		"syscall            \n"
		"move %[ref], $a0  \n"
		: [ref]"=r" (ref)
		: [ctrl]"r" (ctrl)
		: "v0", "v1", "a0");
	return ref;
}

int act_ctrl_revoke(void * ctrl) {
	int ret;
	__asm__ (
		"li    $v1, 25      \n"
		"move $a0, %[ctrl] \n"
		"syscall            \n"
		"move %[ret], $v0   \n"
		: [ret]"=r" (ret)
		: [ctrl]"r" (ctrl)
		: "v0", "v1", "a0");
	return ret;
}

int act_ctrl_terminate(void * ctrl) {
	int ret;
	__asm__ (
		"li    $v1, 26      \n"
		"move $a0, %[ctrl] \n"
		"syscall            \n"
		"move %[ret], $v0   \n"
		: [ret]"=r" (ret)
		: [ctrl]"r" (ctrl)
		: "v0", "v1", "a0");
	return ret;
}

void
ctor_null() {
}

void
dtor_null() {
}

/*
 * CCall helpers
 */

#define CCALL_ASM_CSCB "move $t9, %[cb] \n" "move $v0, %[method_nb] \n"
#define CCALL_INSTR(n) \
        "li $v1, " #n "\n" \
        "syscall \n" \

#define CCALL_INOPS [cb]"r" (cb), [method_nb]"r" (method_nb)
#define CCALL_CLOBS "$c3","$c4","$c5", "v0","v1","a0","a1","a2", "t9"
#define CCALL_TOP \
    ret_t ret; \
    __asm__ __volatile__ ( \
        CCALL_ASM_CSCB \
        "move  $a0, %[rarg1] \n" \
        "move  $a1, %[rarg2] \n" \
        "move  $a2, %[rarg3] \n" \
        "cmove $c3, %[carg1] \n" \
        "cmove $c4, %[carg2] \n" \
        "cmove $c5, %[carg3] \n" \

#define CCALL_BOTTOM \
        "move  %[rret], $v0  \n" \
        "cmove %[cret], $c3  \n" \
        : [rret]"=r" (ret.rret), [cret]"=C" (ret.cret) \
        : CCALL_INOPS, [rarg1]"r" (rarg1), [rarg2]"r" (rarg2), [rarg3]"r" (rarg3), \
        [carg1]"C" (carg1), [carg2]"C" (carg2), [carg3]"C" (carg3) \
        : CCALL_CLOBS);

#define CCALLS(...) CCALL(4, __VA_ARGS__)

register_t ccall_1(void * cb, int method_nb,
        register_t rarg1, register_t rarg2, register_t rarg3,
        const void * __capability carg1, const void * __capability carg2, const void * __capability carg3) {
    CCALL_TOP
        CCALL_INSTR(1001)
    CCALL_BOTTOM
    return ret.rret;
}

register_t ccall_2(void * cb, int method_nb,
        register_t rarg1, register_t rarg2, register_t rarg3,
        const void * __capability carg1, const void * __capability carg2, const void * __capability carg3) {
    CCALL_TOP
        CCALL_INSTR(1002)
    CCALL_BOTTOM
    return ret.rret;
}

ret_t ccall_4(void * cb, int method_nb,
        register_t rarg1, register_t rarg2, register_t rarg3,
        const void * __capability carg1, const void * __capability carg2, const void * __capability carg3) {
    CCALL_TOP
        CCALL_INSTR(1004)
    CCALL_BOTTOM
    return ret;
}
