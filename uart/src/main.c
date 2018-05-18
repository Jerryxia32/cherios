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

#include"lib.h"
#include"uart.h"
#include<cheric.h>

void*__capability uart_cap = NULLCAP;
void*__capability sealing_tool = NULLCAP;

extern void msg_entry;
void (*msg_methods[]) = {uart_putc, uart_puts_wrapper};
size_t msg_methods_nb = countof(msg_methods);
void (*ctrl_methods[]) = {NULL, ctor_null, dtor_null};
size_t ctrl_methods_nb = countof(ctrl_methods);

extern int non_user;
int main(void)
{
  non_user = 1;
	printf("UART Hello world\n");

	/* Get capability to use uart */
  uart_cap = act_get_cap();
  sealing_tool = *(capability*__capability)cheri_getdefault();
  act_self_PCC = cheri_seal(act_self_PCC, sealing_tool);
  act_self_IDC = cheri_seal(act_self_IDC, sealing_tool);
  int ret = namespace_register(PORT_UART, act_self_aid, act_self_PCC, act_self_IDC);
  return_cap = namespace_get_IDC(PORT_CCALL);
  if(ret!=0) {
    printf("UART: register failed\n");
    return -1;
  }

	printf("UART: setup OK\n");

	msg_enable = 1; /* Go in waiting state instead of exiting */
	return 0;
}
