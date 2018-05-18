/*-
 * Copyright (c) 2016 Hongyan Xia
 * Copyright (c) 2016 Hadrien Barral
 * Copyright (c) 2011 Robert N. M. Watson
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

#include"mips.h"
#include"stdarg.h"
#include"stdio.h"
#include"object.h"
#include"assert.h"
#include"namespace.h"
#include"cheric.h"
#include"string.h"

// Non-user tasks will not see UART ready, so just use syscall to do printf.
// User tasks do CCall printf into UART module.
int non_user = 0;
int uart_init_done = 0;

static void
buf_puts(char * str) {
  static void*__capability uart_PCC = NULLCAP;
  static void*__capability uart_IDC = NULLCAP;
  if(non_user) {
    /* Syscall version */
    __asm__ __volatile__ (
      "li   $v1, 34 \n"
      "move $a0, %[str] \n"
      "syscall      \n"
      :: [str]"r" (str): "v1", "a0");
  } else {
    if(!uart_init_done) {
      uart_PCC = namespace_get_PCC(PORT_UART);
      uart_IDC = namespace_get_IDC(PORT_UART);
      if(helper_cap == NULLCAP)
        helper_cap = namespace_get_PCC(PORT_CCALL);
      assert(uart_PCC != NULLCAP);
      assert(uart_IDC != NULLCAP);
      assert(helper_cap != NULLCAP);
      uart_init_done = 1;
    }
    // CCall version
    // Problem is that UART does not have DDC to the entire address space like
    // the kernel, so we have to pass him a capability of the string.
    void*__capability strCap = cheri_getdefault();
    strCap = cheri_setoffset(strCap, (size_t)str);
    strCap = cheri_setbounds(strCap, strlen(str)+1); // include the last '\0'
    ccall_real_4_c(1, REG_MAX, 0, 0, 0, strCap, NULLCAP, NULLCAP, uart_PCC, uart_IDC, helper_cap);
  }
}

void buf_putc(char chr) {
	static size_t offset = 0;
	const size_t buf_size = 0x100;
	static char buf[buf_size+1];
	buf[offset++] = chr;
	if((chr == '\n') || (offset == buf_size)) {
		buf[offset] = '\0';
		buf_puts(buf);
		offset = 0;
	}
}


/*
 * Provide a kernel-compatible version of printf, which invokes the UART
 * driver.
 */
static void
uart_putchar(int c, __attribute__((unused)) void *arg)
{
	buf_putc(c);
}

int
vprintf(const char *fmt, va_list ap)
{
	return (kvprintf(fmt, uart_putchar, NULL, 10, ap));
}

int puts(const char *s) {
	while(*s) {
		uart_putchar(*s++, NULL);
	}
	uart_putchar('\n', NULL);
	return 0;
}

int putc(int character, FILE *f) {
	return fputc(character, f);
}

int putchar(int character) {
    uart_putchar(character, NULL);
    return 0;
}

int fputc(int character, FILE *f) {
	if(f != NULL) {
		panic("fprintf not implememted");
	}
	buf_putc((unsigned char)character);
	return character;
}



int
printf(const char *fmt, ...)
{
	va_list ap;
	int retval;

	va_start(ap, fmt);
	retval = vprintf(fmt, ap);
	va_end(ap);

	return (retval);
}

/* maps to printf */
int
fprintf(FILE *f, const char *fmt, ...)
{
	if(f != NULL) {
		panic("fprintf not implememted");
	}

	va_list ap;
	int retval;

	va_start(ap, fmt);
	retval = vprintf(fmt, ap);
	va_end(ap);

	return (retval);
}
