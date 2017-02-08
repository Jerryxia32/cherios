/*-
 * Copyright (c) 1983 Regents of the University of California.
 * Copyright (c) 2015 SRI International
 * Copyright (c) 2016 Robert N. M. Watson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
/*static char *sccsid = "from: @(#)malloc.c	5.11 (Berkeley) 2/23/91";*/
static char *rcsid = "$FreeBSD$";
#endif /* LIBC_SCCS and not lint */

/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long.
 * This is designed for use in a virtual memory environment.
 */

#include "lib.h"
#include "mips.h"
#include "cheric.h"
#include "malloc_heap.h"

#pragma clang diagnostic ignored "-Wsign-compare"

union overhead;
static void morecore(int);
void init_pagebucket(void);

/*
 * The overhead on a block is one pointer. When free, this space
 * contains a pointer to the next free block. When in use, the first
 * byte is set to MAGIC, and the second byte is the size index.
 */
union	overhead {
	__capability union	overhead *ov_next;	/* when free */
	struct {
		u_char	ovu_magic;	/* magic number */
		u_char	ovu_index;	/* bucket # */
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_size		ovu.ovu_size
};

#define	MAGIC		0xef		/* magic # on accounting info */

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define	NBUCKETS 30
static	__capability union overhead *nextf[NBUCKETS];

static	int pagebucket;			/* page size bucket */


#if defined(MALLOC_DEBUG) || defined(RCHECK)
#define	ASSERT(p)   if (!(p)) botch("p")
#include <stdio.h>
static void
botch(char *s)
{
	fprintf(stderr, "\r\nassertion botched: %s\r\n", s);
	(void) fflush(stderr);		/* just in case user buffered it */
	panic("%s", __func__);
}
#else
#define	ASSERT(p)
#endif

inline __capability void *
malloc_core(size_t nbytes)
{
	__capability union overhead *op;
	int bucket;
	size_t amt;

	assert(pagesz != 0);
	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 */
	if (nbytes <= pagesz - sizeof (*op)) {
		amt = 32;	/* size of first bucket */
		bucket = 2;
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}
	while (nbytes > (size_t)amt - sizeof(*op)) {
		amt <<= 1;
		if (amt == 0)
			return (NULLCAP);
		bucket++;
	}
	/*
	 * If nothing in hash bucket right now,
	 * request more memory from the system.
	 */
	if ((op = nextf[bucket]) == NULLCAP) {
		morecore(bucket);
		if ((op = nextf[bucket]) == NULLCAP)
			return (NULLCAP);
	}
	/* remove from linked list */
	nextf[bucket] = op->ov_next;
	op->ov_magic = MAGIC;
	op->ov_index = bucket;
	//return cheri_setbounds(op + 1, nbytes);
    op += 1;
	void __capability *ret = cheri_setbounds(op, (1<<(bucket+3)) - MALLOC_HEADER_SIZE);
    return ret;
}

__capability void *
malloc_c_c(size_t nbytes) {
    __capability void *ret = malloc_core(nbytes);
    return(cheri_andperm(ret, ~CHERI_PERM_STORE_LOCAL_CAP));
}

__capability void *
malloc_c(size_t nbytes) {
    __capability void *ret = malloc_c_c(nbytes);
    ret = cheri_andperm(ret, cheri_getperm(ret) & ~CHERI_PERM_LOAD_CAP & ~CHERI_PERM_STORE_CAP);
    return ret;
}

__capability void *
calloc_core(size_t num, size_t size)
{
	__capability void *ret;

	if (size != 0 && (num * size) / size != num) {
		/* size_t overflow. */
		return (NULLCAP);
	}

	if ((ret = malloc_core(num * size)) != NULLCAP)
		memset_c(ret, 0, num * size);

	return (ret);
}

__capability void *
calloc_c_c(size_t num, size_t size)
{
	__capability void *ret;

	if (size != 0 && (num * size) / size != num) {
		/* size_t overflow. */
		return (NULLCAP);
	}

	if ((ret = malloc_c_c(num * size)) != NULLCAP)
		memset_c(ret, 0, num * size);

	return (ret);
}

__capability void *
calloc_c(size_t num, size_t nbytes) {
    __capability void *ret = calloc_c_c(num, nbytes);
    ret = cheri_andperm(ret, cheri_getperm(ret) & ~CHERI_PERM_LOAD_CAP & ~CHERI_PERM_STORE_CAP);
    return ret;
}

/*
 * Allocate more memory to the indicated bucket.
 */
static void
morecore(int bucket)
{
	__capability char *buf;
	__capability union overhead *op;
	size_t sz;			/* size of desired block */
	int amt;			/* amount to allocate */
	int nblks;			/* how many blocks we get */

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests (about
	 * 2^30 bytes on a VAX, I think) or for a negative arg.
	 */
	sz = 1 << (bucket + 3);
#ifdef MALLOC_DEBUG
	ASSERT(sz > 0);
#else
	if (sz <= 0)
		return;
#endif
	if (sz < pagesz) {
		amt = pagesz;
		nblks = amt / sz;
	} else {
		amt = sz + pagesz;
		nblks = 1;
	}
	if (amt > pagepool_end - pagepool_start)
		if (__morepages(amt/pagesz) == 0)
			return;

	buf = cheri_setoffset(pool, pagepool_start);
	buf = cheri_setbounds(buf, amt);
	pagepool_start += amt;

	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
	nextf[bucket] = op = cheri_setbounds(buf, sz);
	while (--nblks > 0) {
		op->ov_next = (__capability union overhead *)cheri_setbounds(buf + sz, sz);
		buf += sz;
		op = op->ov_next;
	}
	op->ov_next = NULLCAP;
}

static __capability union overhead *
find_overhead(__capability void * cp)
{
	__capability union overhead *op;

	if (!cheri_gettag(cp))
		return (NULLCAP);
	op = __rederive_pointer(cp);
	if (op == NULLCAP) {
		printf("%s: no region found for %#p\n", __func__, (void *)(cheri_getbase(cp) + cheri_getoffset(cp)));
		return (NULLCAP);
	}
	op--;

	if (op->ov_magic == MAGIC)
		return (op);

	/*
	 * XXX: the above will fail if the users calls free or realloc
	 * with a pointer that has had CSetBounds applied to it.  We
	 * should save all allocation ranges to allow us to find the
	 * metadata.
	 */
	CHERI_PRINT_PTR(cp);
	panic(
	    KRED"malloc: attempting to free or realloc unallocated memory!\n");
	return (NULLCAP);
}

void
free_c(__capability void *cp)
{
    if(!(cheri_getperm(cp) & CHERI_PERM_SOFT_0)) {
        CHERI_PRINT_PTR(cp);
        panic(KRED"malloc: capability permit free not set!\n");
    }
	int bucket;
	__capability union overhead *op;

	if (cp == NULLCAP)
		return;
	op = find_overhead(cp);
	if (op == NULLCAP)
		return;
	bucket = op->ov_index;
	ASSERT(bucket < NBUCKETS);
	op->ov_next = nextf[bucket];	/* also clobbers ov_magic */
	nextf[bucket] = op;
}

__capability void *
realloc_c(__capability void *cp, size_t nbytes)
{
#if 0
	size_t cur_space;	/* Space in the current bucket */
	size_t smaller_space;	/* Space in the next smaller bucket */
	union overhead *op;
	char *res;

	if (cp == NULL)
		return (malloc(nbytes));
	op = find_overhead(cp);
	if (op == NULL)
		return (NULL);
	cur_space = (1 << (op->ov_index + 3)) - sizeof(*op);

	/* avoid the copy if same size block */
	/*
	 * XXX-BD: Arguably we should be tracking the actual allocation
	 * not just the bucket size so that we can do a full malloc+memcpy
	 * when the caller has restricted the length of the pointer passed
	 * realloc() but is growing the buffer within the current bucket.
	 *
	 * As it is, this code contains a leak where realloc recovers access
	 * to the contents in foo:
	 * char *foo = malloc(10);
	 * strcpy(foo, "abcdefghi");
	 * cheri_setbouds(foo, 5);
	 * foo = realloc(foo, 10);
	 */
	smaller_space = (1 << (op->ov_index + 2)) - sizeof(*op);
	if (nbytes <= cur_space && nbytes > smaller_space)
		return (cheri_andperm(cheri_setbounds(op + 1, nbytes),
		    cheri_getperm(cp)));

	if ((res = malloc(nbytes)) == NULL)
		return (NULL);
	/*
	 * Only copy data the caller had access to even if this is less
	 * than the size of the original allocation.  This risks surprise
	 * for some programmers, but to do otherwise risks information leaks.
	 */
	memcpy(res, cp, (nbytes <= cheri_getlen(cp)) ? nbytes : cheri_getlen(cp));
	res = cheri_andperm(res, cheri_getperm(cp));
	free(cp);
	return (res);
#endif
    __capability void *res;
	if((res = malloc_c_c(nbytes)) == NULLCAP)
		return (NULLCAP);
	memcpy_c(res, cp, (nbytes <= cheri_getlen(cp)) ? nbytes : cheri_getlen(cp));
	res = cheri_andperm(res, cheri_getperm(cp));
	free_c(cp);
	return(res);
}


void
init_pagebucket(void)
{
	int bucket;
	size_t amt;

	bucket = 0;
	amt = 8;
	while ((unsigned)pagesz > amt) {
		amt <<= 1;
		bucket++;
	}
	pagebucket = bucket;
}
