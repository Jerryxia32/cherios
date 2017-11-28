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

#include "klib.h"
#include"string.h"

/*
 * Routines to handle activations
 */

/* We only create activations for now, no delete */
struct reg_frame		kernel_exception_framep[MAX_ACTIVATIONS];
struct reg_frame *		kernel_exception_framep_ptr;
act_t				kernel_acts[MAX_ACTIVATIONS];
aid_t 				kernel_curr_act;
aid_t				kernel_next_act;

void act_init(boot_info_t *bi) {
	KERNEL_TRACE("init", "activation init");

	/*
	 * create kernel activation
	 * used to have a 'free' reg frame.
	 * cannot be scheduled: aid 0 is invalid
	 */
	kernel_next_act = 0;
	struct reg_frame dummy_frame;
	bzero(&dummy_frame, sizeof(struct reg_frame));
	act_register(&dummy_frame, "kernel", PRIORITY_DEFAULT);
	kernel_acts[0].status = status_terminated;
	kernel_acts[0].sched_status = sched_terminated;

	/* create the activation for init, passed in the boot_info */
	kernel_curr_act = kernel_next_act = 1;
	act_register(&bi->init_frame, "init", PRIORITY_MAX-1);
	kernel_exception_framep_ptr = &kernel_exception_framep[kernel_curr_act];
	sched_d2a(kernel_curr_act, sched_runnable);
}

void kernel_skip_instr(aid_t act) {
	kernel_exception_framep[act].mf_pc += 4; /* assumes no branch delay slot */
    // XXX: Currently the offset in EPCC is not used when eret, but this might
    // change in the future.
    //void*__capability pcc = kernel_exception_framep[act].cf_pcc;
    //pcc = cheri_incoffset(pcc, 4);
    //kernel_exception_framep[act].cf_pcc = pcc;
}

aid_t
act_register(const reg_frame_t* frame, const char* name, const prio_t priority) {
	aid_t aid = kernel_next_act;

	if(aid >= MAX_ACTIVATIONS) {
		kernel_panic("no act slot");
	}

	/* set aid */
	kernel_acts[aid].aid = aid;

#ifndef __LITE__
	/* set name */
	kernel_assert(ACT_NAME_MAX_LEN > 0);
    int name_len = imin(strlen(name), ACT_NAME_MAX_LEN-1);
	for(int i = 0; i < name_len; i++) {
		char c = name[i];
		kernel_acts[aid].name[i] = c; /* todo: sanitize the name if we do not trust it */
	}
	kernel_acts[aid].name[name_len] = '\0';
#endif

	/* set status */
	kernel_acts[aid].status = status_alive;
    void * __capability dest = cheri_getdefault();
    void * __capability src = cheri_getdefault();
    dest = cheri_setoffset(dest, (size_t)(kernel_exception_framep + aid));
    src = cheri_setoffset(src, (size_t)(frame));

	/* set register frame */
	memcpy_c(dest, src, sizeof(struct reg_frame));

	/* set queue */
	msg_queue_init(aid);
	kernel_acts[aid].queue_mask = MAX_MSG-1;
	kernel_acts[aid].priority = priority;

	/* set scheduling status */
	sched_create(aid);

	KERNEL_TRACE("act", "%s added %s OK! ", __func__, kernel_acts[aid].name);
	/* done, update next_act */
	kernel_next_act++;
	return aid;
}

int
act_revoke(aid_t aid) {
	if(kernel_acts[aid].status == status_terminated) {
		return -1;
	}
	kernel_acts[aid].status = status_revoked;
	return 0;
}

int
act_terminate(aid_t aid) {
    kernel_acts[aid].status = status_terminated;
    sched_delete(aid);
    kernel_acts[aid].sched_status = sched_terminated;
    KERNEL_TRACE("act", "Terminated %s:%d", kernel_acts[aid].name, aid);
	if(aid == kernel_curr_act) { /* terminated itself */
		return 1;
	}
	return 0;
}

int
act_get_status(aid_t aid) {
    //printf("Guy: %d is of status %d!\n", aid, kernel_acts[aid].status);
	return kernel_acts[aid].status;
}

void
act_wait(aid_t act, aid_t next_hint) {
	kernel_assert(kernel_acts[act].sched_status == sched_runnable);
	if(msg_queue_empty(act)) {
		sched_a2d(act, sched_waiting);
	} else {
		kernel_acts[act].sched_status = sched_schedulable;
	}
	sched_reschedule(next_hint);
}
