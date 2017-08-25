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

/*
 * These functions abstract the syscall register convention
 */
static void syscall_sleep(void) {
	int time = kernel_exception_framep_ptr->mf_a0;
	if(time != 0) {
		KERNEL_ERROR("sleep >0 not implemented");
	} else {
		sched_reschedule(0);
	}
}

static void syscall_act_register(void) {
	reg_frame_t * frame = (void *)kernel_exception_framep_ptr->mf_a0;
	char * name = (void *)kernel_exception_framep_ptr->mf_a1;
	kernel_exception_framep_ptr->mf_a0 = (register_t)act_register(frame, name);
}

static void syscall_act_ctrl_get_ref(void) {
	kernel_exception_framep_ptr->mf_a0 = (register_t)act_get_ref((void *)kernel_exception_framep_ptr->mf_a0);
}

static void syscall_act_ctrl_get_status(void) {
	kernel_exception_framep_ptr->mf_v0 = (register_t)act_get_status((void *)kernel_exception_framep_ptr->mf_a0);
}

static void syscall_act_revoke(void) {
	kernel_exception_framep_ptr->mf_v0 = act_revoke((void *)kernel_exception_framep_ptr->mf_a0);
}

static void syscall_act_terminate(void) {
	int ret = act_terminate((void *)kernel_exception_framep_ptr->mf_a0);
	if(ret == 1) {
		sched_reschedule(0);
	} else {
		kernel_exception_framep_ptr->mf_v0 = ret;
	}
}

static void syscall_puts() {
	void * msg = (void *)(kernel_exception_framep_ptr->mf_a0 + cheri_getbase(kernel_exception_framep_ptr->cf_c0));
	#ifndef __LITE__
	kernel_printf(KGRN"%s" KREG KRST, msg);
	#else
	kernel_puts(msg);
	#endif
}

static void syscall_panic(void) { //fixme: temporary
	kernel_freeze();
}

static void syscall_interrupt_register(void) {
	kernel_exception_framep_ptr->mf_v0 =
		kernel_interrupt_register(kernel_exception_framep_ptr->mf_a0);
}

static void syscall_interrupt_enable(void) {
	kernel_exception_framep_ptr->mf_v0 =
		kernel_interrupt_enable(kernel_exception_framep_ptr->mf_a0);
}

/*
 * Syscall demux
 */
void kernel_exception_syscall(void)
{
	long sysn = kernel_exception_framep_ptr->mf_v1;
	//KERNEL_TRACE("exception", "Syscall number %ld", sysn);
	aid_t kca = kernel_curr_act;
	switch(sysn) {
	case 13:
		KERNEL_TRACE("exception", "Syscall %ld (sleep)", sysn);
		syscall_sleep();
		break;
	case 20:
		KERNEL_TRACE("exception", "Syscall %ld (act_register)", sysn);
		syscall_act_register();
		break;
	case 21:
		KERNEL_TRACE("exception", "Syscall %ld (act_ctrl_get_ref)", sysn);
		syscall_act_ctrl_get_ref();
		break;
	case 23:
		KERNEL_TRACE("exception", "Syscall %ld (act_ctrl_get_status)", sysn);
		syscall_act_ctrl_get_status();
		break;
	case 25:
		KERNEL_TRACE("exception", "Syscall %ld (act_revoke)", sysn);
		syscall_act_revoke();
		break;
	case 26:
		KERNEL_TRACE("exception", "Syscall %ld (act_terminate)", sysn);
		syscall_act_terminate();
		break;
	case 34:
		KERNEL_TRACE("exception", "Syscall %ld (puts)", sysn);
		syscall_puts();
		break;
	case 42:
		KERNEL_TRACE("exception", "Syscall %ld (panic)", sysn);
		syscall_panic();
		break;
	case 50:
		KERNEL_TRACE("exception", "Syscall %ld (interrupt_register)", sysn);
		syscall_interrupt_register();
		break;
	case 51:
		KERNEL_TRACE("exception", "Syscall %ld (interrupt_enable)", sysn);
		syscall_interrupt_enable();
		break;
	case 66:
		KERNEL_TRACE("exception", "Syscall %ld (gc)", sysn);
		//syscall_gc();
		break;
	case 1001:
		KERNEL_TRACE("exception", "(CCall1)Syscall %ld", sysn);
        kernel_ccall_fake(1);
		break;
	case 1002:
		KERNEL_TRACE("exception", "(CCall2)Syscall %ld", sysn);
        kernel_ccall_fake(2);
		break;
	case 1004:
		KERNEL_TRACE("exception", "(CCall4)Syscall %ld", sysn);
        kernel_ccall_fake(4);
		break;
	case 1010:
		KERNEL_TRACE("exception", "(CReturn)Syscall %ld", sysn);
        kernel_creturn_fake();
		break;
	default:
		KERNEL_ERROR("unknown syscall '%d'", sysn);
		kernel_freeze();
	}

	kernel_skip_instr(kca);
}

/*
 * Fake CCall/CReturn handler
 */

/* Creates a token for synchronous CCalls. This ensures the answer is unique. */
static void * __capability get_sync_token(aid_t ccaller) {
	static uint16_t unique = 0;
	unique++;
	kernel_acts[ccaller].sync_token.expected_reply  = unique;

	uint32_t token_offset = (((u32)ccaller) << 16) + unique;
	void * __capability sync_token = cheri_andperm(cheri_getdefault(), 0);
	#ifdef _CHERI256_
	sync_token = cheri_setbounds(sync_token, 0);
	#endif
	sync_token = cheri_setoffset(sync_token, token_offset);
	return kernel_seal(sync_token, 62);
}

void kernel_ccall_fake(int cflags) {
    if(!(cflags & 4) && !(cflags & 2) && !(cflags & 1))
        KERNEL_ERROR(KRED"unknown fake ccall selector '%x'"KRST"\n", cflags);

	// cb is the activation.
	act_t * cb = (void *)kernel_exception_framep_ptr->mf_t9;

	if(cb->status != status_alive) {
		KERNEL_ERROR("Trying to CCall revoked activation %s-%d",
		             cb->name, cb->aid);
        kernel_panic("revoked activation.\n");
		return;
	}

	void * __capability sync_token = NULLCAP;
	if(cflags & 4) {
		sync_token = get_sync_token(kernel_curr_act);
	}

	/* Push the message on the queue */
	if(msg_push(cb->aid, kernel_curr_act, sync_token)) {
		//KERNEL_ERROR("Queue full");
		if(cflags & 4) {
			kernel_panic("queue full (csync)");
		}
		kernel_exception_framep_ptr->mf_v0 = 1;
	} else {
		kernel_exception_framep_ptr->mf_v0 = 0;
    }

	if(cflags & 4) {
		KERNEL_TRACE(__func__, "%s : sync-call %s",
			     kernel_acts[kernel_curr_act].name,
			     kernel_acts[cb->aid].name);
		sched_a2d(kernel_curr_act, sched_sync_block);
		sched_reschedule(cb->aid);
	}
	else if(cflags & 2) {
		KERNEL_TRACE(__func__, "%s : send-switch %s",
			     kernel_acts[kernel_curr_act].name,
			     kernel_acts[cb->aid].name);

		act_wait(kernel_curr_act, cb->aid);
	}
	else {
		KERNEL_TRACE(__func__, "%s : send %s",
			     kernel_acts[kernel_curr_act].name,
			     kernel_acts[cb->aid].name);
	}
}

void kernel_creturn_fake(void) {
  	KERNEL_TRACE(__func__, "in %s", kernel_acts[kernel_curr_act].name);

	void * __capability sync_token = kernel_exception_framep_ptr->cf_c2;
	if(sync_token == NULLCAP) {
		/* Used by asynchronous primitives */
		//act_wait(kernel_curr_act, 0);
		act_wait(kernel_curr_act, kernel_curr_act);
		return;
	}

	/* Check if we expect this anwser */
	sync_token = kernel_unseal(sync_token, 62);
	size_t sync_offset = cheri_getoffset(sync_token);
	aid_t ccaller = sync_offset >> 16;
	uint16_t unique = sync_offset & 0xFFFF;
	if(kernel_acts[ccaller].sync_token.expected_reply != unique ) {
		KERNEL_ERROR("bad sync creturn");
		kernel_freeze();
	}

	/* Make the caller runnable again */
	kernel_assert(kernel_acts[ccaller].sched_status == sched_sync_block);
	sched_d2a(ccaller, sched_runnable);

	/* Copy return values */
	kernel_exception_framep[ccaller].cf_c3 =
	   kernel_exception_framep_ptr->cf_c3;
	kernel_exception_framep[ccaller].mf_v0 =
	   kernel_exception_framep_ptr->mf_v0;
	kernel_exception_framep[ccaller].mf_v1 =
	   kernel_exception_framep_ptr->mf_v1;

	/* Try to set the callee in waiting mode */
	act_wait(kernel_curr_act, ccaller);
}
