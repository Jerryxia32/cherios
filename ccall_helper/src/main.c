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
#include<mips.h>
#include<object.h>
#include<namespace.h>
#include<cheric.h>
#include<stdio.h>

extern char ccall_helper;
extern char creturn_helper;

int main(void)
{
	puts("CCall helper Hello world.\n");
    void * __capability call_helper = cheri_getpcc();
    void * __capability return_helper = cheri_getpcc();
    call_helper = cheri_setoffset(call_helper, (size_t)&ccall_helper);
    return_helper = cheri_setoffset(return_helper, (size_t)&creturn_helper);

    call_helper = cheri_seal(call_helper, act_self_cap);
    return_helper = cheri_seal(return_helper, act_self_cap);
    int ret = namespace_register(7, act_self_ref, act_self_id, call_helper, return_helper);
    if(ret!=0) {
        printf("CCall_helper: register failed\n");
        return -1;
    }
    printf("CCall_helper: register OK\n");

	msg_enable = 1; /* Go in waiting state instead of exiting */
	return 0;
}
