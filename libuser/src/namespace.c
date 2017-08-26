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

#include "mips.h"
#include"cheric.h"
#include "object.h"
#include "assert.h"

aid_t namespace_aid = 0;

void
namespace_init(aid_t ns_aid) {
	namespace_aid = ns_aid;
}

int
namespace_register(int nb, aid_t aid,
        void*__capability PCC, void*__capability IDC) {
    return ccall_4(namespace_aid, 0, nb, aid, 0, PCC, IDC, NULLCAP).rret;
}

aid_t
namespace_get_aid(int nb) {
    return ccall_4(namespace_aid, 1, nb, 0, 0, NULLCAP, NULLCAP, NULLCAP).rret;
}

void*__capability
namespace_get_PCC(int nb) {
	return ccall_4(namespace_aid, 3, nb, 0, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}

void*__capability
namespace_get_IDC(int nb) {
	return ccall_4(namespace_aid, 4, nb, 0, 0, NULLCAP, NULLCAP, NULLCAP).cret;
}
