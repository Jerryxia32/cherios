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

/* turn 'schedulable' activation 'act' in 'runnable' state */
static void sched_schedule(aid_t act) {
	kernel_assert(kernel_acts[act].sched_status == sched_schedulable);
	/* Set message */
	msg_pop(act);

	/* Activation ready to be run */
	kernel_acts[act].sched_status = sched_runnable;
}

/* todo: sleep cpu */
static void sched_nothing_to_run(void) {
	KERNEL_ERROR("No activation to schedule");
	kernel_freeze();
}

static uint32_t aqueue[PRIORITY_MAX][MAX_ACTIVATIONS];
static aid_t squeue_a[PRIORITY_MAX][MAX_ACTIVATIONS];
static uint32_t squeue_a_idx[PRIORITY_MAX] = {0};
static uint32_t squeue_a_end[PRIORITY_MAX] = {0};

static uint32_t (*__capability aqueue_cap)[MAX_ACTIVATIONS];
static aid_t (*__capability squeue_a_cap)[MAX_ACTIVATIONS];
static uint32_t*__capability squeue_a_idx_cap;
static uint32_t*__capability squeue_a_end_cap;

void
sched_caps_init() {
    void*__capability theDefault = cheri_getdefault();
    aqueue_cap = cheri_setbounds(cheri_setoffset(theDefault,
            (size_t)&aqueue[0][0]), sizeof(u32)*PRIORITY_MAX*MAX_ACTIVATIONS);
    squeue_a_cap = cheri_setbounds(cheri_setoffset(theDefault,
            (size_t)&squeue_a[0][0]), sizeof(aid_t)*PRIORITY_MAX*MAX_ACTIVATIONS);
    squeue_a_idx_cap = cheri_setbounds(cheri_setoffset(theDefault,
            (size_t)&squeue_a_idx[0]), sizeof(u32)*PRIORITY_MAX);
    squeue_a_end_cap = cheri_setbounds(cheri_setoffset(theDefault,
            (size_t)&squeue_a_end[0]), sizeof(u32)*PRIORITY_MAX);
}

#define QADD(act, squeue, aqueue) { \
    size_t prio = kernel_acts[act].priority; \
    aqueue##_cap[prio][act] = squeue##_end_cap[prio];  \
    squeue##_cap[prio][squeue##_end_cap[prio]++] = act; \
}

#define QDEL(act, squeue, aqueue) { \
    size_t prio = kernel_acts[act].priority; \
    kernel_assert(squeue##_cap[prio][aqueue##_cap[prio][act]] == act); \
    squeue##_end_cap[prio]--;                            \
    aid_t replacement = squeue##_cap[prio][squeue##_end_cap[prio]];  \
    squeue##_cap[prio][aqueue##_cap[prio][act]] = replacement;         \
    aqueue##_cap[prio][replacement] = aqueue##_cap[prio][act]; \
}

void sched_create(aid_t act) {
	KERNEL_TRACE("sched", "create %s-%ld", kernel_acts[act].name, act);
	kernel_acts[act].sched_status = sched_waiting;
}

void sched_delete(aid_t act) {
	KERNEL_TRACE("sched", "delete %s-%ld", kernel_acts[act].name, act);
    size_t prioTemp = kernel_acts[act].priority;
	if(squeue_a_cap[prioTemp][aqueue_cap[prioTemp][act]] == act) {
		QDEL(act, squeue_a, aqueue);
	}
	kernel_acts[act].status = status_terminated;
}

void sched_d2a(aid_t act, sched_status_e status) {
	KERNEL_TRACE("sched", "add %s-%ld", kernel_acts[act].name, act);
	QADD(act, squeue_a, aqueue);
	kernel_assert((status == sched_runnable) || (status == sched_schedulable));
	kernel_acts[act].sched_status = status;
}

void sched_a2d(aid_t act, sched_status_e status) {
	KERNEL_TRACE("sched", "rem %s-%ld", kernel_acts[act].name, act);
	QDEL(act, squeue_a, aqueue);
	kernel_assert((status == sched_sync_block) || (status == sched_waiting));
	kernel_acts[act].sched_status = status;
}

void
sched_prio_change(aid_t act, prio_t newPrio) {
    KERNEL_TRACE("The act is: %d\n", act);
    KERNEL_TRACE("Old priority is: %d\n", kernel_acts[act].priority);
    QDEL(act, squeue_a, aqueue);
    kernel_acts[act].priority = newPrio;
    QADD(act, squeue_a, aqueue);
    KERNEL_TRACE("New priority is: %d\n", kernel_acts[act].priority);
}

// from top to low priorities, find the next schedulable or runnable act.
static aid_t
sched_picknext(void) {
    for(ssize_t prioLvl = PRIORITY_MAX-1; prioLvl != -1; prioLvl--) {
        size_t thisLevelEnd = squeue_a_end_cap[prioLvl];
        if(thisLevelEnd == 0) continue;
        size_t currIdx = squeue_a_idx_cap[prioLvl]+1;
        if(currIdx >= thisLevelEnd) currIdx = 0;
        squeue_a_idx_cap[prioLvl] = currIdx;
        return squeue_a_cap[prioLvl][currIdx];
    }
    return 0;
}

aid_t sched_reschedule(aid_t hint) {
	#ifdef __TRACE__
	size_t old_kernel_curr_act = kernel_curr_act;
	#endif
	if(!hint) {
again:
		hint = sched_picknext();
	}
	if(!hint) {
		sched_nothing_to_run();
	}
	if(kernel_acts[hint].sched_status == sched_schedulable) {
		sched_schedule(hint);
	}
    if(kernel_acts[hint].sched_status != sched_runnable) {
        goto again;
    }

	kernel_curr_act = hint;
	kernel_exception_framep_ptr = kernel_exception_framep + hint;
	KERNEL_TRACE("sched", "Reschedule from task '%s-%ld' to task '%s-%ld'",
	        kernel_acts[old_kernel_curr_act].name, old_kernel_curr_act,
	        kernel_acts[kernel_curr_act].name, kernel_curr_act);

    return hint;
}