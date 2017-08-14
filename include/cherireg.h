/*-
 * Copyright (c) 2017 Hongyan Xia
 * Copyright (c) 2011-2016 Robert N. M. Watson
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

#ifndef _MIPS_INCLUDE_CHERIREG_H_
#define	_MIPS_INCLUDE_CHERIREG_H_

/*
 * The size of in-memory capabilities in bytes; minimum alignment is also
 * assumed to be this size.
 */
#if defined(_MIPS_SZCAP) && (_MIPS_SZCAP != 64)
#error "_MIPS_SZCAP defined but not 64"
#endif

#if defined(CPU_CHERI64) || (defined(_MIPS_SZCAP) && (_MIPS_SZCAP == 64))
#define	CHERICAP_SIZE   8
#endif

/*
 * CHERI ISA-defined constants for capabilities -- suitable for inclusion from
 * assembly source code.
 *
 * XXXRW: CHERI_UNSEALED is not currently considered part of the perms word,
 * but perhaps it should be.
 */
#define	CHERI_PERM_GLOBAL			(1 << 0)	/* 0x00000001 */
#define	CHERI_PERM_EXECUTE			(1 << 1)	/* 0x00000002 */
#define	CHERI_PERM_LOAD				(1 << 2)	/* 0x00000004 */
#define	CHERI_PERM_STORE			(1 << 3)	/* 0x00000008 */
#define	CHERI_PERM_LOAD_CAP			(1 << 4)	/* 0x00000010 */
#define	CHERI_PERM_STORE_CAP			(1 << 5)	/* 0x00000020 */
#define	CHERI_PERM_STORE_LOCAL_CAP		(1 << 6)	/* 0x00000040 */
#define	CHERI_PERM_SEAL				(1 << 7)	/* 0x00000080 */
#define	CHERI_PERM_ACCESS_SYSTEM_REGISTERS			(1 << 8)	/* 0x00000100 */

/*
 * User-defined permission bits.
 */
#define	CHERI_PERM_USER0			(1 << 15)	/* 0x00008000 */
#define	CHERI_PERM_USER1			(1 << 16)	/* 0x00008000 */

/*
 * Macros defining initial permission sets for various scenarios; details
 * depend on the permissions available on 256-bit or 128-bit CHERI:
 *
 * CHERI_PERM_USER_PRIVS: Mask of all available user-defined permissions
 * CHERI_PERM_PRIV: Mask of all available hardware-defined permissions
 */
#define	CHERI_PERM_USER_PRIVS						\
	(CHERI_PERM_USER0 | CHERI_PERM_USER1)

#define	CHERI_PERM_PRIV							\
	(CHERI_PERM_GLOBAL | CHERI_PERM_EXECUTE |			\
	CHERI_PERM_LOAD | CHERI_PERM_STORE | CHERI_PERM_LOAD_CAP |	\
	CHERI_PERM_STORE_CAP | CHERI_PERM_STORE_LOCAL_CAP |		\
	CHERI_PERM_SEAL | CHERI_PERM_ACCESS_SYSTEM_REGISTERS \
    CHERI_PERM_USER_PRIVS)

/*
 * Basic userspace permission mask; CHERI_PERM_EXECUTE will be added for
 * executable capabilities ($pcc); CHERI_PERM_STORE, CHERI_PERM_STORE_CAP,
 * and CHERI_PERM_STORE_LOCAL_CAP will be added for data permissions ($c0).
 *
 * No variation required between 256-bit and 128-bit CHERI.
 */
#define	CHERI_PERM_USER							\
	(CHERI_PERM_GLOBAL | CHERI_PERM_LOAD | CHERI_PERM_LOAD_CAP |	\
	CHERI_PERM_USER_PRIVS)

#define	CHERI_PERM_USER_CODE	(CHERI_PERM_USER | CHERI_PERM_EXECUTE)
#define	CHERI_PERM_USER_DATA	(CHERI_PERM_USER | CHERI_PERM_STORE |	\
				CHERI_PERM_STORE_CAP |			\
				CHERI_PERM_STORE_LOCAL_CAP)

/*
 * Root "object-type" capability -- queried via sysarch(2) when libcheri needs
 * to allocate types.  This can be used neither as a data nor code capability.
 *
 * No variation required between 256-bit and 128-bit CHERI.
 */
#define	CHERI_PERM_USER_TYPE	(CHERI_PERM_GLOBAL | CHERI_PERM_SEAL)

/*
 * List of CHERI capability cause code constants, which are used to
 * disambiguate various CP2 exceptions.
 *
 * XXXRW: I wonder if we really need different permissions for each exception-
 * handling capability.
 *
 * XXXRW: Curiously non-contiguous.
 *
 * XXXRW: KDC is listed as 0x1a in the spec, which collides with EPCC.  Not
 * sure what is actually used.
 */
#define	CHERI_EXCCODE_NONE		0x00
#define	CHERI_EXCCODE_LENGTH		0x01
#define	CHERI_EXCCODE_TAG		0x02
#define	CHERI_EXCCODE_SEAL		0x03
#define	CHERI_EXCCODE_TYPE		0x04
#define	CHERI_EXCCODE_CALL		0x05
#define	CHERI_EXCCODE_RETURN		0x06
#define	CHERI_EXCCODE_UNDERFLOW		0x07
#define	CHERI_EXCCODE_USER_PERM		0x08
#define	CHERI_EXCCODE_TLBSTORE		0x09
#define	CHERI_EXCCODE_IMPRECISE		0x0a
#define	_CHERI_EXCCODE_RESERVED0b	0x0b
#define	_CHERI_EXCCODE_RESERVED0c	0x0c
#define	_CHERI_EXCCODE_RESERVED0d	0x0d
#define	_CHERI_EXCCODE_RESERVED0e	0x0e
#define	_CHERI_EXCCODE_RESERVED0f	0x0f
#define	CHERI_EXCCODE_GLOBAL		0x10
#define	CHERI_EXCCODE_PERM_EXECUTE	0x11
#define	CHERI_EXCCODE_PERM_LOAD		0x12
#define	CHERI_EXCCODE_PERM_STORE	0x13
#define	CHERI_EXCCODE_PERM_LOADCAP	0x14
#define	CHERI_EXCCODE_PERM_STORECAP	0x15
#define	CHERI_EXCCODE_STORE_LOCALCAP	0x16
#define	CHERI_EXCCODE_PERM_SEAL		0x17
#define	_CHERI_EXCCODE_RESERVED18	0x18
#define	_CHERI_EXCCODE_RESERVED19	0x19
#define	CHERI_EXCCODE_ACCESS_EPCC	0x1a
#define	CHERI_EXCCODE_ACCESS_KDC	0x1b	/* XXXRW */
#define	CHERI_EXCCODE_ACCESS_KCC	0x1c
#define	CHERI_EXCCODE_ACCESS_KR1C	0x1d
#define	CHERI_EXCCODE_ACCESS_KR2C	0x1e
#define	_CHERI_EXCCODE_RESERVED1f	0x1f

/*
 * How to turn the cause register into an exception code and register number.
 */
#define	CHERI_CAPCAUSE_EXCCODE_MASK	0xff00
#define	CHERI_CAPCAUSE_EXCCODE_SHIFT	8
#define	CHERI_CAPCAUSE_REGNUM_MASK	0xff

/*
 * Location of the CHERI CCall/CReturn software-path exception vector.
 */
#define	CHERI_CCALL_EXC_VEC	((intptr_t)(int32_t)0x80000280)

#endif /* _MIPS_INCLUDE_CHERIREG_H_ */
