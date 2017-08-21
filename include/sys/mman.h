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

#include "cdefs.h"
#include "errno.h"

/*
void *	mmap(void *addr, size_t length, int prot, int flags, __unused int fd, __unused off_t offset);
int	munmap(void *addr, size_t length);
 */

void	memmgt_set_act(void *ref);
void * __capability malloc_c(size_t n);
void * __capability calloc_c(size_t n, size_t s);
void * __capability realloc_c(void * __capability oldmem, size_t s);
void    free_c(void * __capability p);

/*
enum mmap_prot
{
  PROT_READ		= 1 << 0,
  PROT_WRITE		= 1 << 1,
  PROT_NO_READ_CAP	= 1 << 2,
  PROT_NO_WRITE_CAP	= 1 << 3
};
#define PROT_RW (PROT_READ | PROT_WRITE)

enum mmap_flags
{
  map_private	= 1 << 0,
  map_shared	= 1 << 1,
  map_anonymous	= 1 << 2
};
#define MAP_PRIVATE map_private
#define MAP_ANONYMOUS map_anonymous
#define MAP_SHARED map_shared

enum mmap_return
{
  ENOMEM = 1
};

#define MAP_FAILED ((void *) -1)
 */
