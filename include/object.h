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

#ifndef _OBJECT_H_
#define	_OBJECT_H_

#include "mips.h"

extern void * act_self_ctrl;
extern void * act_self_ref;
extern void * act_self_id;
extern __capability void * act_self_cap;
void *	act_ctrl_get_ref(void * ctrl);
void *	act_ctrl_get_id(void * ctrl);
int	act_ctrl_revoke(void * ctrl);
int	act_ctrl_terminate(void * ctrl);
__capability void *	act_get_cap(void);
void *	act_seal_id(void * id);

typedef struct {
	__capability void * cret;
	register_t rret;
}  ret_t;


void	object_init(void * self_ctrl, __capability void * self_cap);

void	ctor_null(void);
void	dtor_null(void);
void *	get_curr_cookie(void);
void	set_curr_cookie(void * cookie);

void * get_cookie(void * cb, void * cs);

extern void * sync_token;
extern long msg_enable;

#define CCALL(selector, ...) ccall_##selector(__VA_ARGS__)
register_t ccall_1(void * cb, void * cs, int method_nb,
        register_t rarg1, register_t rarg2, register_t rarg3,
        const __capability void * carg1, const __capability void * carg2, const __capability void * carg3);
register_t ccall_2(void * cb, void * cs, int method_nb,
        register_t rarg1, register_t rarg2, register_t rarg3,
        const __capability void * carg1, const __capability void * carg2, const __capability void * carg3);

void    ccall_c_n(void * cb, void * cs, int method_nb, const __capability void * carg);
__capability void *  ccall_n_c(void * cb, void * cs, int method_nb);
__capability void *  ccall_r_c(void * cb, void * cs, int method_nb, int rarg);
__capability void *  ccall_c_c(void * cb, void * cs, int method_nb, const __capability void * carg);
__capability void *  ccall_rr_c(void * cb, void * cs, int method_nb, int rarg, int rarg2);
register_t ccall_n_r(void * cb, void * cs, int method_nb);
register_t ccall_r_r(void * cb, void * cs, int method_nb, int rarg);
register_t ccall_c_r(void * cb, void * cs, int method_nb, __capability void * carg);
register_t ccall_rr_r(void * cb, void * cs, int method_nb, int rarg, int rarg2);
void ccall_rr_n(void * cb, void * cs, int method_nb, int rarg, int rarg2);
register_t ccall_rrr_r(void * cb, void * cs, int method_nb, int rarg1, int rarg2, int rarg3);
register_t ccall_rc_r(void * cb, void * cs, int method_nb, int rarg, const __capability void * carg);
void    ccall_cc_n(void * cb, void * cs, int method_nb, __capability void * carg1, __capability void * carg2);
void    ccall_rc_n(void * cb, void * cs, int method_nb, int rarg, __capability void * carg);
__capability void *ccall_rc_c(void * cb, void * cs, int method_nb, int rarg, const __capability void * carg);
register_t ccall_rcc_r(void * cb, void * cs, int method_nb, register_t rarg1, __capability void * carg1, __capability void * carg2);
__capability void *  ccall_rrrc_c(void * cb, void * cs, int method_nb,
        register_t, register_t, register_t, __capability void * carg);
register_t ccall_rrcc_r(void * cb, void * cs, int method_nb,
        register_t rarg1, register_t rarg2, __capability void * carg1, __capability void * carg2);
register_t ccall_rccc_r(void * cb, void * cs, int method_nb,
        register_t rarg1, __capability void *carg1, __capability void * carg2, const __capability void * carg3);

#endif
