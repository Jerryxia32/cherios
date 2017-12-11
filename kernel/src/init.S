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

.set noat
.set noreorder
#include<asm.h>

# Assembly stub on the way to running C code.  Assumes that $c0 and $pcc are
# global and suitable.  Enable capability coprocessor.  Set up a stack.  Jump
# to various startup functions to initialise the C runtime, and then call
# cherios_main().

	.text
	.section .init
	.global start
	.ent start
start:
	#Add 4 to EPCC
    dmfc0       $k1, $MIPS_CP0_REG_EPC
    addi        $k1, $k1, 4
    dmtc0       $k1, $MIPS_CP0_REG_EPC

	#Save register frame
    la          $k0, kernel_init_save_frame

    csc         $c1, $k0, 1*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c2, $k0, 2*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c3, $k0, 3*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c4, $k0, 4*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c5, $k0, 5*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c6, $k0, 6*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c7, $k0, 7*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $c25, $k0, 8*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $idc, $k0, 9*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $kr1c, $k0, 10*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csc         $epcc, $k0, 11*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    # avoid "direct c0 access deprecated" warning
    cgetdefault $c1
    csc         $c1, $k0, 0*CAP_SIZE+MIPS_FRAME_SIZE($kdc)

    csd         $at, $k0, 0*REG_SIZE($kdc)
    csd         $v0, $k0, 1*REG_SIZE($kdc)
    csd         $v1, $k0, 2*REG_SIZE($kdc)
    csd         $a0, $k0, 3*REG_SIZE($kdc)
    csd         $a1, $k0, 4*REG_SIZE($kdc)
    csd         $a2, $k0, 5*REG_SIZE($kdc)
    csd         $a3, $k0, 6*REG_SIZE($kdc)
    csd         $s0, $k0, 7*REG_SIZE($kdc)
    csd         $s1, $k0, 8*REG_SIZE($kdc)
    csd         $s2, $k0, 9*REG_SIZE($kdc)
    csd         $s3, $k0, 10*REG_SIZE($kdc)
    csd         $t9, $k0, 11*REG_SIZE($kdc)
    csd         $gp, $k0, 12*REG_SIZE($kdc)
    csd         $sp, $k0, 13*REG_SIZE($kdc)
    csd         $fp, $k0, 14*REG_SIZE($kdc)
    csd         $ra, $k0, 15*REG_SIZE($kdc)
    mfhi        $k1
    csd         $k1, $k0, 16*REG_SIZE($kdc)
    mflo        $k1
    csd         $k1, $k0, 17*REG_SIZE($kdc)
    dmfc0       $k1, $MIPS_CP0_REG_EPC
    csd         $k1, $k0, 18*REG_SIZE($kdc)

    # Set up a boot-time stack using memory allocated by the linker;
    # assume that $c0 is a sensible starting capability for this.
    la          $sp, __stop_exception_stack
    # using small sections in the kernel, $gp has to be initialized.
    la          $gp, _gp

    # Initialise BSS and globals before running C constructors, so that C
    # constructors can take them (and other aspects of the conventional C
    # runtime) for granted.

    # Zero BSS.
    jal         crt_init_bss
    nop

    # Initialise globals.
    jal         crt_init_globals
    nop

    # Call C constructors.
    jal         crt_call_constructors
    nop

    # Pass arguments to cherios_main(int, void*) in a0 and c3,
    # and call cherios_main.
    dli         $a0, 1
    jal         cherios_main
    cld         $a1, $k0, 4*REG_SIZE($kdc)

    # Restore context from the register frame set by the scheduler,
    # typically pointing to an init activation.
    la          $k0, kernel_exception_framep_ptr
    clw         $k0, $k0, 0($kdc)

    cld         $at, $k0, 0*REG_SIZE($kdc)
    cld         $v0, $k0, 1*REG_SIZE($kdc)
    cld         $v1, $k0, 2*REG_SIZE($kdc)
    cld         $a0, $k0, 3*REG_SIZE($kdc)
    cld         $a1, $k0, 4*REG_SIZE($kdc)
    cld         $a2, $k0, 5*REG_SIZE($kdc)
    cld         $a3, $k0, 6*REG_SIZE($kdc)
    cld         $s0, $k0, 7*REG_SIZE($kdc)
    cld         $s1, $k0, 8*REG_SIZE($kdc)
    cld         $s2, $k0, 9*REG_SIZE($kdc)
    cld         $s3, $k0, 10*REG_SIZE($kdc)
    cld         $t9, $k0, 11*REG_SIZE($kdc)
    cld         $gp, $k0, 12*REG_SIZE($kdc)
    cld         $sp, $k0, 13*REG_SIZE($kdc)
    cld         $fp, $k0, 14*REG_SIZE($kdc)
    cld         $ra, $k0, 15*REG_SIZE($kdc)
    cld         $k1, $k0, 16*REG_SIZE($kdc)
    mthi        $k1
    cld         $k1, $k0, 17*REG_SIZE($kdc)
    mtlo        $k1
    cld         $k1, $k0, 18*REG_SIZE($kdc)
    dmtc0       $k1, $MIPS_CP0_REG_EPC

    # avoid "direct c0 access deprecated" warning
    clc         $c1, $k0, 0*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    csetdefault $c1
    clc         $c1, $k0, 1*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c2, $k0, 2*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c3, $k0, 3*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c4, $k0, 4*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c5, $k0, 5*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c6, $k0, 6*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c7, $k0, 7*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $c25, $k0, 8*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $idc, $k0, 9*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $kr1c, $k0, 10*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    clc         $epcc, $k0, 11*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
    
    # Kernel init done, clear $k0, $k1
    clearhi     0x0c00
    eret
start_end:

	.align 5
kernel_init_save_frame:
	.skip 16*CAP_SIZE + 32*REG_SIZE, 0 #Should be enough
kernel_init_save_frame_end:
