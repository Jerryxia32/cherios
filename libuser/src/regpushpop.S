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

# Assembly stub on the way to running C code.  Assumes that $c0 and $pcc are
# global and suitable.  Enable capability coprocessor.  Set up a stack.  Jump
# to various startup functions to initialise the C runtime, and then call
# cherios_main().

	.text
	.global saveframe
	.ent saveframe
saveframe:
	#Save register frame
	dla	$t0, obj_save_frame_end
    daddi   $t0, -CAP_SIZE
    csc $c17, $t0, 0($c0)
	dla	$t0, obj_save_frame
    cgetdefault $c17
	csetoffset $c17, $c17, $t0

	# Expands to a list of the form
	# csd	$greg, $0, offset*REG_SIZE($kr1c)
	foreachi gsave, 0, $c17,					\
		$s0, $s1, $s2, $s3, $s4, $s5, $s6,	\
		$s7, $gp, $sp, $fp #, $ra

	dla	$t0, obj_save_frame_end
    daddi   $t0, -CAP_SIZE
    clc $c17, $t0, 0($c0)
    jr  $ra
saveframe_end:
	.size saveframe, saveframe_end - saveframe
	.end saveframe

	.text
	.global loadframe
	.ent loadframe
loadframe:
	#Save register frame
	dla	$t0, obj_save_frame_end
    daddi   $t0, -CAP_SIZE
    csc $c17, $t0, 0($c0)
	dla	$t0, obj_save_frame
    cgetdefault $c17
	csetoffset $c17, $c17, $t0

	# Expands to a list of the form
	# cld	$greg, $0, offset*REG_SIZE($kr1c)
	foreachi grestore, 0, $c17,					\
		$s0, $s1, $s2, $s3, $s4, $s5, $s6,	\
		$s7, $gp, $sp, $fp #, $ra

	dla	$t0, obj_save_frame_end
    daddi   $t0, -CAP_SIZE
    clc $c17, $t0, 0($c0)
    jr  $ra
loadframe_end:
	.size loadframe, loadframe_end - loadframe
	.end loadframe

	.align 5
obj_save_frame:
	.skip 4*CAP_SIZE + 16*REG_SIZE, 0 #Should be enough
obj_save_frame_end:
	.size obj_save_frame, obj_save_frame_end - obj_save_frame
