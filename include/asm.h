/*-
 * Copyright (c) 2017 Hongyan Xia
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

#ifndef _MIPS_INCLUDE_ASM_H_
#define	_MIPS_INCLUDE_ASM_H_

#if _MIPS_SZCAP == 64
	#define _CHERI64_
    #define CAP_SIZE 8
    #define CAP_SIZE_BITS 3
#else
	#error Unknown capability size
#endif

#define REG_SIZE (64/8)
#define SIZET_SIZE (32/8)
#define MIPS_FRAME_SIZE (20*REG_SIZE)
#define REG_SIZE_BITS 3
#define SIZET_SIZE_BITS 2
#define MUTEX_ADDR 0x100

#define	MIPS_BEV0_TLB_VECTOR	0x80000080
#define	MIPS_BEV0_EXCEPTION_VECTOR	0x80000180
#define	MIPS_BEV0_CCALL_VECTOR		0x80000280

/*
 * MIPS CP0 register numbers.
 */
#define	MIPS_CP0_REG_INDEX      0
#define	MIPS_CP0_REG_RANDOM     1
#define	MIPS_CP0_REG_ENTRYLO0   2
#define	MIPS_CP0_REG_ENTRYLO1   3
#define	MIPS_CP0_REG_CONTEXT    4
#define	MIPS_CP0_REG_PAGEMASK   5
#define	MIPS_CP0_REG_WIRED      6
#define	MIPS_CP0_REG_RESERVED7  7
#define	MIPS_CP0_REG_BADVADDR   8
#define	MIPS_CP0_REG_COUNT      9
#define	MIPS_CP0_REG_ENTRYHI    10
#define	MIPS_CP0_REG_COMPARE    11
#define	MIPS_CP0_REG_STATUS     12
#define	MIPS_CP0_REG_CAUSE      13
#define	MIPS_CP0_REG_EPC        14
#define	MIPS_CP0_REG_PRID       15
#define	MIPS_CP0_REG_CONFIG     16
#define	MIPS_CP0_REG_LLADDR     17
#define	MIPS_CP0_REG_WATCHLO    18
#define	MIPS_CP0_REG_WATCHHI    19
#define	MIPS_CP0_REG_XCONTEXT   20
#define	MIPS_CP0_REG_RESERVED21 21
#define	MIPS_CP0_REG_RESERVED22 22
#define	MIPS_CP0_REG_RESERVED23 23
#define	MIPS_CP0_REG_RESERVED24 24
#define	MIPS_CP0_REG_RESERVED25 25
#define	MIPS_CP0_REG_ECC        26
#define	MIPS_CP0_REG_CACHEERR   27
#define	MIPS_CP0_REG_TAGLO      28
#define	MIPS_CP0_REG_TAGHI      29
#define	MIPS_CP0_REG_ERROREPC   30
#define	MIPS_CP0_REG_RESERVED31 31

#endif /* _MIPS_INCLUDE_ASM_H_ */
