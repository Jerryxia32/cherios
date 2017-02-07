#-
# Copyright (c) 2016 Robert N. M. Watson
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

#Fixme: clean-up code && comment

.set MIPS_SZCAP, _MIPS_SZCAP
.set CAP_SIZE, (MIPS_SZCAP/8)
.set C17_OFFSET, 96
.set REG_SIZE, (64/8)

.set noreorder
.set nobopt
.set noat

	.text
	.ent ccall_real_4
ccall_real_4:
    move $a0, $a1
    move $a1, $a2
    move $a2, $a3
    ccall   $c6, $c7, 0
    jr  $ra
    nop
ccall_real_4_end:
	.size ccall_real_4, ccall_real_4_end - ccall_real_4
	.end ccall_real_4

	.text
	.global ccall_real_4_r
	.ent ccall_real_4_r
ccall_real_4_r:
    j ccall_real_4
    move $v0, $a0
ccall_real_4_r_end:
	.size ccall_real_4_r, ccall_real_4_r_end - ccall_real_4_r
	.end ccall_real_4_r

	.text
	.global ccall_real_4_c
	.ent ccall_real_4_c
ccall_real_4_c:
    j ccall_real_4
    move $v0, $a0
ccall_real_4_c_end:
	.size ccall_real_4_c, ccall_real_4_c_end - ccall_real_4_c
	.end ccall_real_4_c

	.text
	.global ccall_real_4_strong
	.ent ccall_real_4_strong
ccall_real_4_strong:
	# Save register frame
    cgetdefault $c25
    sd $s0, 0($sp)
    sd $s1, 8($sp)
    sd $s2, 16($sp)
    sd $s3, 24($sp)
    sd $s4, 32($sp)
    sd $s5, 40($sp)
    sd $s6, 48($sp)
    sd $s7, 56($sp)
    sd $gp, 64($sp)
#sd $sp, 72($sp)
    sd $fp, 80($sp)
    sd $ra, 88($sp)
    csc $c17, $sp, C17_OFFSET($c25)
    csc $c18, $sp, (C17_OFFSET+CAP_SIZE)($c25)

    move $v0, $a0
    move $a0, $a1
    move $a1, $a2
    move $a2, $a3
    clearlo 0xff0b # keep four mips arg regs and v0
    clearhi 0xdfff # keep $sp for seal
    cclearlo 0xfe06 # keep c0 c3-c8
    cclearhi 0x07ff # keep kernel cap regs
    ccall   $c6, $c7, 0

	# Load register frame
    cgetdefault $c25
    ld $s0, 0($sp)
    ld $s1, 8($sp)
    ld $s2, 16($sp)
    ld $s3, 24($sp)
    ld $s4, 32($sp)
    ld $s5, 40($sp)
    ld $s6, 48($sp)
    ld $s7, 56($sp)
    ld $gp, 64($sp)
#ld $sp, 72($sp)
    ld $fp, 80($sp)
    ld $ra, 88($sp)
    clc $c17, $sp, C17_OFFSET($c25)
    clc $c18, $sp, (C17_OFFSET+CAP_SIZE)($c25)

    jr  $ra
    daddiu  $sp, $sp, 160
ccall_real_4_strong_end:
	.size ccall_real_4_strong, ccall_real_4_strong_end - ccall_real_4_strong
	.end ccall_real_4_strong

	.text
	.global ccall_real_4_strong_r
	.ent ccall_real_4_strong_r
ccall_real_4_strong_r:
    j ccall_real_4_strong
    daddiu  $sp, $sp, -160
ccall_real_4_strong_r_end:
	.size ccall_real_4_strong_r, ccall_real_4_strong_r_end - ccall_real_4_strong_r
	.end ccall_real_4_strong_r

	.text
	.global ccall_real_4_strong_c
	.ent ccall_real_4_strong_c
ccall_real_4_strong_c:
    j ccall_real_4_strong
    daddiu  $sp, $sp, -160
ccall_real_4_strong_c_end:
	.size ccall_real_4_strong_c, ccall_real_4_strong_c_end - ccall_real_4_strong_c
	.end ccall_real_4_strong_c
