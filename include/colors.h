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

//#define NO_COLORS
#ifndef NO_COLORS
	#define COLORS
#endif

#ifdef COLORS
	#define KRST "\x1B[0m"
	#define KBLD "\x1B[1m"
	#define KFNT "\x1B[2m"
	#define KITC "\x1B[3m"
	#define KUND "\x1B[4m"
	#define KBLK "\x1B[5m"
	#define KREG "\x1B[22m"
	#define KRED "\x1B[31m"
	#define KGRN "\x1B[32m"
	#define KYLW "\x1B[33m"
	#define KBLU "\x1B[34m"
	#define KMAJ "\x1B[35m"
	#define KCYN "\x1B[36m"
	#define KWHT "\x1B[37m"
#else
	#define KRST ""
	#define KBLD ""
	#define KFNT ""
	#define KITC ""
	#define KUND ""
	#define KBLK ""
	#define KREG ""
	#define KRED ""
	#define KGRN "" "\x1B[32m"
	#define KYLW ""
	#define KBLU ""
	#define KMAJ ""
	#define KCYN ""
	#define KWHT ""
#endif
