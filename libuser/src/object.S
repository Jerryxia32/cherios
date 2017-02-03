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
.set C17_OFFSET, 128
.set REG_SIZE, (64/8)
.set MIPS_FRAME_SIZE, (32*REG_SIZE)

.set noreorder
.set nobopt
.set noat

#
# A few useful macros
#

# Saves capability register 'creg' to offset 'offset' of frame 'frame'
.macro csave creg, offset, frame
	csc	\creg, $0, \offset*CAP_SIZE+MIPS_FRAME_SIZE(\frame)
.endm

# Restores capability register 'creg' to offset 'offset' of frame 'frame'
.macro crestore creg, offset, frame
	clc	\creg, $0, \offset*CAP_SIZE+MIPS_FRAME_SIZE(\frame)
.endm

# Saves general-purpose register 'greg' to offset 'offset' of frame 'frame'
.macro gsave greg, offset, frame
	csd	\greg, $0, \offset*REG_SIZE(\frame)
.endm

# Restores general-purpose register 'greg' to offset 'offset' of frame 'frame'
.macro grestore greg, offset, frame
	cld	\greg, $0, \offset*REG_SIZE(\frame)
.endm

# Calls 'macro elem, index, global' for each 'elem' of the input list
.macro foreachi macro, index, global, head, tail:vararg
	\macro \head, \index, \global
	.ifnb  \tail
	foreachi \macro, "(\index+1)", \global, \tail
	.endif
.endm

	.align 5
obj_save_frame:
	.skip 4*CAP_SIZE + 16*REG_SIZE, 0 #Should be enough
obj_save_frame_end:
	.size obj_save_frame, obj_save_frame_end - obj_save_frame

	.text
	.global ccall_real_4_first
	.ent ccall_real_4_first
ccall_real_4_first:
    cmove   $c1, $c3
    cmove   $c2, $c4
    cmove   $idc, $c5
    jr  $ra
    nop
ccall_real_4_first_end:
	.size ccall_real_4_first, ccall_real_4_first_end - ccall_real_4_first
	.end ccall_real_4_first

	.text
	.ent ccall_real_4_second
ccall_real_4_second:
    move $v0, $a0
    move $a0, $a1
    move $a1, $a2
    move $a2, $a3
    ccall   $c1, $c2, 0
    jr  $ra
    nop
ccall_real_4_second_end:
	.size ccall_real_4_second, ccall_real_4_second_end - ccall_real_4_second
	.end ccall_real_4_second

	.text
	.global ccall_real_4_second_r
	.ent ccall_real_4_second_r
ccall_real_4_second_r:
    j ccall_real_4_second
    nop
ccall_real_4_second_r_end:
	.size ccall_real_4_second_r, ccall_real_4_second_r_end - ccall_real_4_second_r
	.end ccall_real_4_second_r

	.text
	.global ccall_real_4_second_c
	.ent ccall_real_4_second_c
ccall_real_4_second_c:
    j ccall_real_4_second
    nop
ccall_real_4_second_c_end:
	.size ccall_real_4_second_c, ccall_real_4_second_c_end - ccall_real_4_second_c
	.end ccall_real_4_second_c

	.text
	.global ccall_real_4_second_strong
	.ent ccall_real_4_second_strong
ccall_real_4_second_strong:
	# Save register frame
	dla	$t0, obj_save_frame
    cgetdefault $c25
	csetoffset $c25, $c25, $t0

	# Expands to a list of the form
	# csd	$greg, $0, offset*REG_SIZE($c25)
	foreachi gsave, 0, $c25,					\
		$s0, $s1, $s2, $s3, $s4, $s5, $s6,	\
		$s7, $gp, $sp, $fp, $ra
    csc $c17, $t0, C17_OFFSET($c25)

    move $v0, $a0
    move $a0, $a1
    move $a1, $a2
    move $a2, $a3
    clearlo 0xff0b # keep four mips arg regs and v0
    clearhi 0xffff
    cclearlo 0xffc0 # keep c0-c5
    cclearhi 0x03ff # keep kernel cap regs and IDC
    ccall   $c1, $c2, 0

	# Load register frame
	dla	$t0, obj_save_frame
    cgetdefault $c25
	csetoffset $c25, $c25, $t0

	# Expands to a list of the form
	# csd	$greg, $0, offset*REG_SIZE($c25)
	foreachi grestore, 0, $c25,					\
		$s0, $s1, $s2, $s3, $s4, $s5, $s6,	\
		$s7, $gp, $sp, $fp, $ra
    clc $c17, $t0, C17_OFFSET($c25)

    jr  $ra
    nop
ccall_real_4_second_strong_end:
	.size ccall_real_4_second_strong, ccall_real_4_second_strong_end - ccall_real_4_second_strong
	.end ccall_real_4_second_strong

	.text
	.global ccall_real_4_second_strong_r
	.ent ccall_real_4_second_strong_r
ccall_real_4_second_strong_r:
    j ccall_real_4_second_strong
    nop
ccall_real_4_second_strong_r_end:
	.size ccall_real_4_second_strong_r, ccall_real_4_second_strong_r_end - ccall_real_4_second_strong_r
	.end ccall_real_4_second_strong_r

	.text
	.global ccall_real_4_second_strong_c
	.ent ccall_real_4_second_strong_c
ccall_real_4_second_strong_c:
    j ccall_real_4_second_strong
    nop
ccall_real_4_second_strong_c_end:
	.size ccall_real_4_second_strong_c, ccall_real_4_second_strong_c_end - ccall_real_4_second_strong_c
	.end ccall_real_4_second_strong_c
