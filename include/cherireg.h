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
