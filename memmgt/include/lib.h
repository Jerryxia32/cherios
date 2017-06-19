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

#ifndef	_LIB_H_
#define	_LIB_H_

#include "mips.h"
#include"cheric.h"
#include "assert.h"
#include "cdefs.h"
#include "debug.h"
#include "misc.h"
#include "namespace.h"
#include "object.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

void register_ns(void * ns_ref, void * ns_id);

#define MMAP 0

#if !MMAP
#define malloc_c __malloc_c
#define calloc_c __calloc_c
#define malloc_core __malloc_core
#define calloc_core __calloc_core
#define realloc_c __realloc_c
#define free_c __free_c

void * __capability	malloc_c(size_t nbytes);
void * __capability	calloc_c(size_t num, size_t size);
void * __capability	malloc_core(size_t nbytes);
void * __capability	calloc_core(size_t num, size_t size);
void * __capability	realloc_c(void * __capability cp, size_t nbytes);
void	free_c(void * __capability cp);
#endif

//void	release(void * __capability p);
//void	release_init(void);

//void * __capability	__mmap(void * __capability addr, size_t length, int prot, int flags);
//int	__munmap(void * __capability addr, size_t length);
//void	minit(char * __capability heap, size_t heaplen);
//void	mfree(void * __capability addr);

extern	size_t pagesz;
extern	char * __capability pool;

#endif /* !_LIB_H_ */
