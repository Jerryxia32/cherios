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

#include "lib.h"
#include<colors.h>

typedef struct {
	void * act_reference;
	void * act_default_id;
    __capability void *act_PCC;
    __capability void *act_IDC;
} bind_t;

const int bind_len = 0x80;
bind_t bind[bind_len];

void ns_init(void) {
	bzero(bind, sizeof(bind));
}

static int validate_idx(int nb) {
	if(nb <  0       ) { return 0; }
	if(nb >= bind_len) { return 0; }
	return 1;
}

static int validate_act_caps(__capability void * act_PCC, __capability void * act_IDC) {
	if(cheri_gettype(act_PCC) !=
	   cheri_gettype(act_IDC)) { return 0; }
    if(act_PCC != NULLCAP || act_IDC != NULLCAP) {
        if(cheri_gettag(act_PCC) == 0) { return 0; }
        if(cheri_gettag(act_IDC) == 0) { return 0; }
        if(cheri_getsealed(act_PCC) == 0) { return 0; }
        if(cheri_getsealed(act_IDC) == 0) { return 0; }
    }
	/* todo: check otype range and permissions */
	return 1;
}

/* Get reference for service 'n' */
void * ns_get_reference(int nb) {
	if(!validate_idx(nb))
		return NULL;

	/* If service not in use, will already return NULL */
	printf(KWHT"%s: ref request for port %d"KRST"\n", __func__, nb);
	return bind[nb].act_reference;
}

/* Get default identifier for service 'n' */
void * ns_get_identifier(int nb) {
	if(!validate_idx(nb))
		return NULL;

	/* If service not in use, will already return NULL */
	printf(KWHT"%s: id request for port %d"KRST"\n", __func__, nb);
	return bind[nb].act_default_id;
}

/* Get sealed PCC for service 'n' */
__capability void * ns_get_PCC(int nb) {
	if(!validate_idx(nb))
		return NULL;

	/* If service not in use, will already return NULL */
	printf(KWHT"%s: id request for port %d"KRST"\n", __func__, nb);
	return bind[nb].act_PCC;
}

/* Get sealed IDC for service 'n' */
__capability void * ns_get_IDC(int nb) {
	if(!validate_idx(nb))
		return NULL;

	/* If service not in use, will already return NULL */
	printf(KWHT"%s: id request for port %d"KRST"\n", __func__, nb);
	return bind[nb].act_IDC;
}

/* Register a module a service 'nb' */
static int ns_register_core(int nb, void * act_reference, void * act_default_id, __capability void *act_PCC, __capability void *act_IDC) {
	if(bind[nb].act_reference != NULL) {
		printf(KWHT"%s: port already in use"KRST"\n", __func__);
		return -1;
	}

	bind[nb].act_reference  = act_reference;
	bind[nb].act_default_id = act_default_id;
	bind[nb].act_PCC = act_PCC;
	bind[nb].act_IDC = act_IDC;
	printf(KWHT"%s: registered at port %d"KRST"\n", __func__, nb);
	return 0;
}

int ns_register(int nb, void * act_reference, void * act_default_id, __capability void *act_PCC, __capability void *act_IDC) {
	if(!validate_idx(nb) || !validate_act_caps(act_PCC, act_IDC))
		return -1;

	return ns_register_core(nb, act_reference, act_default_id, act_PCC, act_IDC);
}
