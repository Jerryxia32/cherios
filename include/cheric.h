/*-
 * Copyright (c) 2013-2015 Robert N. M. Watson
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

#ifndef _MIPS_INCLUDE_CHERIC_H_
#define	_MIPS_INCLUDE_CHERIC_H_

#include "cdefs.h"
#include "cherireg.h"
#include "mips.h"

/*
 * Derive CHERI-flavor from capability size
 */

#if _MIPS_SZCAP == 256
	#define _CHERI256_
    #define CAP_SIZE 32
#elif _MIPS_SZCAP == 128
	#define _CHERI128_
    #define CAP_SIZE 16
#elif _MIPS_SZCAP == 64
	#define _CHERI64_
    #define CAP_SIZE 8
#else
	#error Unknown capability size
#endif

/*
 * Programmer-friendly macros for CHERI-aware C code -- requires use of
 * CHERI-aware Clang/LLVM, and full CP2 context switching, so not yet usable
 * in the kernel.
 */
#define	cheri_getlen(x)		__builtin_mips_cheri_get_cap_length(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_getbase(x)	__builtin_mips_cheri_get_cap_base(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_getoffset(x)	__builtin_mips_cheri_cap_offset_get(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_getperm(x)	__builtin_mips_cheri_get_cap_perms(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_getsealed(x)	__builtin_mips_cheri_get_cap_sealed(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_gettag(x)		__builtin_mips_cheri_get_cap_tag(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_gettype(x)	__builtin_mips_cheri_get_cap_type(		\
				    __DECONST(void * __capability, (x)))

#define	cheri_andperm(x, y)	__builtin_mips_cheri_and_cap_perms(		\
				    __DECONST(void * __capability, (x)), (y))
#define	cheri_cleartag(x)	__builtin_mips_cheri_clear_cap_tag(		\
				    __DECONST(void * __capability, (x)))
#define	cheri_incoffset(x, y)	__builtin_mips_cheri_cap_offset_increment(	\
				    __DECONST(void * __capability, (x)), (y))
#define	cheri_setoffset(x, y)	__builtin_mips_cheri_cap_offset_set(		\
				    __DECONST(void * __capability, (x)), (y))

#define	cheri_seal(x, y)	__builtin_mips_cheri_seal_cap(		 \
				    __DECONST(void * __capability, (x)), \
				    __DECONST(void * __capability, (y)))
#define	cheri_unseal(x, y)	__builtin_mips_cheri_unseal_cap(		 \
				    __DECONST(void * __capability, (x)), \
				    __DECONST(void * __capability, (y)))

#define	cheri_getcause()	__builtin_mips_cheri_get_cause()
#define	cheri_setcause(x)	__builtin_mips_cheri_set_cause(x)

#define	cheri_ccheckperm(c, p)	__builtin_mips_cheri_check_perms(		\
				    __DECONST(void * __capability, (c)), (p))
#define	cheri_cchecktype(c, t)	__builtin_mips_cheri_check_type(		\
				    __DECONST(void * __capability, (c)), (t))

#define	cheri_getdefault()	__builtin_mips_cheri_get_global_data_cap()
#define	cheri_getidc()		__builtin_mips_cheri_get_invoke_data_cap()
#define	cheri_getkr1c()		__builtin_mips_cheri_get_kernel_cap1()
#define	cheri_getkr2c()		__builtin_mips_cheri_get_kernel_cap2()
#define	cheri_getkcc()		__builtin_mips_cheri_get_kernel_code_cap()
#define	cheri_getkdc()		__builtin_mips_cheri_get_kernel_data_cap()
#define	cheri_getepcc()		__builtin_mips_cheri_get_exception_program_counter_cap()
#define	cheri_getpcc()		__builtin_mips_cheri_get_program_counter_cap()
#define	cheri_getstack()	__builtin_cheri_stack_get()

#define	cheri_local(c)		cheri_andperm((c), ~CHERI_PERM_GLOBAL)

#define	cheri_setbounds(x, y)	__builtin_cheri_bounds_set(		\
				    __DECONST(void * __capability, (x)), (y))

#define cheri_getddcoffset(cap) (cheri_getbase(cap) + cheri_getoffset(cap) \
                    - cheri_getbase(cheri_getdefault()))

/* Names for permission bits */
#define CHERI_PERM_GLOBAL		(1 <<  0)
#define CHERI_PERM_EXECUTE		(1 <<  1)
#define CHERI_PERM_LOAD			(1 <<  2)
#define CHERI_PERM_STORE		(1 <<  3)
#define CHERI_PERM_LOAD_CAP		(1 <<  4)
#define CHERI_PERM_STORE_CAP		(1 <<  5)
#define CHERI_PERM_STORE_LOCAL_CAP	(1 <<  6)
#define CHERI_PERM_SEAL			(1 <<  7)
#define CHERI_PERM_ACCESS_SYS_REGS	(1 << 10)
#if (_MIPS_SZCAP == 64)
#define CHERI_PERM_SOFT_0		(1 << 11)
#else
#define CHERI_PERM_SOFT_0		(1 << 15)
#define CHERI_PERM_SOFT_1		(1 << 16)
#define CHERI_PERM_SOFT_2		(1 << 17)
#define CHERI_PERM_SOFT_3		(1 << 18)
#endif

#define CHERI_TB_WIDTH 8
#if (_MIPS_SZCAP != 64)
#define CHERI_OTYPE_WIDTH 24
#endif
#define	NULLCAP		((void * __capability)0)

/*
 * Two variations on cheri_ptr() based on whether we are looking for a code or
 * data capability.  The compiler's use of CFromPtr will be with respect to
 * $c0 or $pcc depending on the type of the pointer derived, so we need to use
 * types to differentiate the two versions at compile time.  We don't provide
 * the full set of function variations for code pointers as they haven't
 * proven necessary as yet.
 *
 * XXXRW: Ideally, casting via a function pointer would cause the compiler to
 * derive the capability using CFromPtr on $pcc rather than on $c0.  This
 * appears not currently to be the case, so manually derive using
 * cheri_getpcc() for now.
 */
static __inline void * __capability
cheri_codeptr(const void *ptr, size_t len)
{
#ifdef NOTYET
	__capability void (*c)(void) = ptr;
#else
	void * __capability c = cheri_setoffset(cheri_getpcc(),
	    (register_t)ptr);
#endif

	/* Assume CFromPtr without base set, availability of CSetBounds. */
	return (cheri_setbounds(c, len));
}

static __inline void * __capability
cheri_codeptrperm(const void *ptr, size_t len, register_t perm)
{

	return (cheri_andperm(cheri_codeptr(ptr, len),
	    perm | CHERI_PERM_GLOBAL));
}

static __inline void * __capability
cheri_ptr(const void *ptr, size_t len)
{

	/* Assume CFromPtr without base set, availability of CSetBounds. */
	return (cheri_setbounds((const void * __capability)ptr, len));
}

static __inline void * __capability
cheri_ptrperm(const void *ptr, size_t len, register_t perm)
{

	return (cheri_andperm(cheri_ptr(ptr, len), perm | CHERI_PERM_GLOBAL));
}

static __inline void * __capability
cheri_ptrpermoff(const void *ptr, size_t len, register_t perm, off_t off)
{

	return (cheri_setoffset(cheri_ptrperm(ptr, len, perm), off));
}

/*
 * Construct a capability suitable to describe a type identified by 'ptr';
 * set it to zero-length with the offset equal to the base.  The caller must
 * provide a root capability (in the old world order, derived from $c0, but in
 * the new world order, likely extracted from the kernel using sysarch(2)).
 *
 * The caller may wish to assert various properties about the returned
 * capability, including that CHERI_PERM_SEAL is set.
 */
static __inline void * __capability
cheri_maketype(void * __capability root_type, register_t type)
{
	void * __capability c;

	c = root_type;
	c = cheri_setoffset(c, type);	/* Set type as desired. */
	c = cheri_setbounds(c, 1);	/* ISA implies length of 1. */
	c = cheri_andperm(c, CHERI_PERM_GLOBAL | CHERI_PERM_SEAL); /* Perms. */
	return (c);
}

static __inline void * __capability
cheri_zerocap(void)
{
	return (void * __capability)0;
}

#define	cheri_getreg(x) ({						\
	void * __capability _cap;					\
	__asm __volatile ("cmove %0, $c" #x : "=C" (_cap));		\
	_cap;								\
})

#define	cheri_setreg(x, cap) do {					\
	if ((x) == 0)							\
		__asm __volatile ("cmove $c" #x ", %0" : : "C" (cap) :	\
		    "memory");						\
	else								\
		__asm __volatile ("cmove $c" #x ", %0" : : "C" (cap));  \
} while (0)

#define CHERI_PRINT_PTR(ptr)						\
	printf("%s: " #ptr " b:%016jx l:%016zx o:%jx\n", __func__,	\
	   cheri_getbase((const void * __capability)(ptr)),		\
	   cheri_getlen((const void * __capability)(ptr)),		\
	   cheri_getoffset((const void * __capability)(ptr)))

#define CHERI_PRINT_CAP(cap)						\
	printf("%-20s: %-16s t:%lx s:%lx p:%08jx "			\
	       "b:%016jx l:%016zx o:%jx\n",				\
	   __func__,							\
	   #cap,							\
	   cheri_gettag(cap),						\
	   cheri_getsealed(cap),					\
	   cheri_getperm(cap),						\
	   cheri_getbase(cap),						\
	   cheri_getlen(cap),						\
	   cheri_getoffset(cap))

#define CHERI_PRINT_CAP_LITE(cap)					\
	printf("t:%x s:%x b:0x%16jx l:0x%16zx o:0x%jx",			\
	   cheri_gettag(cap),						\
	   cheri_getsealed(cap),					\
	   cheri_getbase(cap),						\
	   cheri_getlen(cap),						\
	   cheri_getoffset(cap))

#define CHERI_ELEM(cap, idx)						\
	cheri_setbounds(cap + idx, sizeof(cap[0]))

#define VCAP_X CHERI_PERM_EXECUTE
#define VCAP_R CHERI_PERM_LOAD
#define VCAP_W CHERI_PERM_STORE
#define VCAP_RW (VCAP_R | VCAP_W)

static inline int VCAP_I(const void * cap, size_t len, unsigned flags, u64 sealed) {
	if(!cheri_gettag(cap)) {
		return 0;
	}
	if(cheri_getsealed(cap) != sealed) {
		return 0;
	}
	if(cheri_getlen(cap) < len) {
		return 0;
	}
	if((cheri_getperm(cap) & flags) != flags) {
		return 0;
	}
	return 1;
}

static inline int VCAP(const void * cap, size_t len, unsigned flags) {
	return VCAP_I(cap, len, flags, 0);
}

static inline int VCAPS(const void * cap, size_t len, unsigned flags) {
	return VCAP_I(cap, len, flags, 1);
}

//todo: have real one in compiler
#ifdef _CHERI128_
#define	__sealable	__attribute__((aligned(0x1000)))
#else
#define	__sealable
#endif

/*
 * Canonical C-language representation of a capability.
 */
typedef void * __capability capability;

/*
 * Register frame to be preserved on context switching. The order of
 * save/restore is very important for both reasons of correctness and security.
 * Assembler routines know about this layout, so great care should be taken.
 */
typedef struct reg_frame {
	/*
	 * General-purpose MIPS registers.
	 */
	/* No need to preserve $zero. */
	register_t	mf_at, mf_v0, mf_v1;
	register_t	mf_a0, mf_a1, mf_a2, mf_a3;
	register_t	mf_t0, mf_t1;
	register_t	mf_s0, mf_s1, mf_s2, mf_s3;
	register_t	mf_t9;
	/* No need to preserve $k0, $k1. */
	register_t	mf_gp, mf_sp, mf_fp, mf_ra;

	/* Multiply/divide result registers. */
	register_t	mf_hi, mf_lo;

	/* Program counter. */
	register_t	mf_pc;
	register_t	padding0;
	register_t	padding1;
	register_t	padding2;

	/*
	 * Capability registers.
	 */
	/* c0 has special properties for MIPS load/store instructions. */
	capability	cf_c0;

	/*
	 * General purpose capability registers.
	 */
	capability	cf_c1, cf_c2;
    capability cf_c3, cf_c4, cf_c5, cf_c6, cf_c7;
    //capability cf_c9;
    //capability cf_c11, cf_c12;
    //capability  cf_c10, cf_c11, cf_c12, cf_c13;
	//capability	cf_c14, cf_c15, cf_c16, cf_c17;
	//capability	cf_c18, cf_c19, cf_c20, cf_c21, cf_c22;
	//capability	cf_c23, cf_c24;
    capability cf_c25;
    capability cf_idc;

	/*
	 * Special-purpose capability registers that must be preserved on a
	 * user context switch.  Note that kernel registers are omitted.
	 */
	capability cf_kr1c;

	/* Program counter capability. */
	capability	cf_pcc;


} reg_frame_t;
#endif /* _MIPS_INCLUDE_CHERIC_H_ */
