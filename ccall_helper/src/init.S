#-
# Copyright (c) 2016 Hadrien Barral
# All rights reserved.
#
# This software was developed by SRI International and the University of
# Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
# ("CTSRD"), as part of the DARPA CRASH research programme.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

.set noreorder
.set nobopt
.set noat
#include<asm.h>

	.text
	.global start
	.ent start
start:
    b start_common
    nop

	.text
	.global ccall_helper
	.ent ccall_helper
ccall_helper:
    csc $idc, $0, 0($kr1c)
    cgetdefault   $c25
    csetoffset  $c25, $c25, $sp
    csc $c25, $0, CAP_SIZE($kr1c)
    csd $v1, $0, 2*CAP_SIZE($kr1c)
//#define REAL_TIME
#ifdef REAL_TIME
    mfc0   $t9, $MIPS_CP0_REG_COUNT # read the current count register
    csw $t9, $0, 2*CAP_SIZE+REG_SIZE($kr1c)
#endif

    ccall   $c1, $c2, 42
    cincoffset  $kr1c, $kr1c, 4*CAP_SIZE
ccall_helper_end:
	.size ccall_helper, ccall_helper - ccall_helper_end
	.end ccall_helper

	.text
	.global creturn_helper
	.ent creturn_helper
creturn_helper:
    cincoffset  $kr1c, $kr1c, -4*CAP_SIZE
    clc $c1, $0, 0($kr1c)
    clc $c25, $0, CAP_SIZE($kr1c)
    # extract the stack pointer of the caller
    cgetoffset  $sp, $c25
    csetoffset  $c25, $c25, $0
    cjr $c1
    csetdefault $c25
creturn_helper_end:
	.size creturn_helper, creturn_helper - creturn_helper_end
	.end creturn_helper
