#-
# Copyright (c) 2011 Robert N. M. Watson
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

.set CAP_SIZE, _MIPS_SZCAP/8
.set REG_SIZE, 64/8
.set MIPS_FRAME_SIZE, 32*REG_SIZE
.set noat
.set noreorder

#
# Capability-aware exception handler.
#
# Code in this file assumes that the kernel executes in a single global
# memory segment that will be described by KCC (and installed in PCC during
# entry to the exception vector) and KDC.  It is possible to imagine linking
# the kernel so that KCC describes only code, and KDC describes only data, but
# we don't yet do this.
#
# Exception code can safely use $k0, $k1, $kcc and $kdc to
# perform its context switch.  PC and PCC are both implicitly used, with
# pre-exception values preserved in EPC and EPCC respectively.
#

#
# Relocatable exception vector; jumps back to kernel_exception_handler() in
# normal program memory.  This runs with KCC installed in PCC.
#
		.global kernel_exception_trampoline
		.ent kernel_exception_trampoline
kernel_exception_trampoline:
		dla	$k0, kernel_cp2_exception_handler
		jr	$k0
        csetoffset  $kdc, $kdc, $0
kernel_exception_trampoline_end:
		nop
		.global kernel_exception_trampoline_end
		.end kernel_exception_trampoline
		.size kernel_exception_trampoline, kernel_exception_trampoline_end - kernel_exception_trampoline
		.size kernel_exception_trampoline_end, 1 # 0 triggers capsizefix warning

		.global kernel_ccall_trampoline
		.ent kernel_ccall_trampoline
kernel_ccall_trampoline:
		dla	$k0, kernel_cp2_ccall_handler
		jr	$k0
        li  $k1, 5
kernel_ccall_trampoline_end:
		nop
		.global kernel_ccall_trampoline_end
		.end kernel_ccall_trampoline
		.size kernel_ccall_trampoline, kernel_ccall_trampoline_end - kernel_ccall_trampoline
		.size kernel_ccall_trampoline_end, 1 # 0 triggers capsizefix warning

#
# Assembly-level exception handler routine -- save context, invoke
# kernel_exception() with a reference to the frame, then restore context and
# return from the exception handler.
#
# XXXRW: Note that we use a global pointer for the exception frame, so this is
# inherrently single-CPU.
#
		.text
		.global kernel_cp2_exception_handler
		.ent kernel_cp2_exception_handler
kernel_cp2_exception_handler:

		#
		# Since we are still running with user capabilities in most
		# capability registers, we must access all kernel data
		# relative to KDC.  First save and clear CP2 context, then
		# move on to general-purpose registers.
		#
		dla	$k0, kernel_exception_framep_ptr
        cld	$k0, $k0, 0($kdc)

        csc $c0, $k0, 0*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c1, $k0, 1*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c2, $k0, 2*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c3, $k0, 3*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c4, $k0, 4*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c5, $k0, 5*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c6, $k0, 6*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c7, $k0, 7*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c8, $k0, 8*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c17, $k0, 9*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c18, $k0, 10*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $c25, $k0, 11*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $kr1c, $k0, 12*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        csc $epcc, $k0, 13*CAP_SIZE+MIPS_FRAME_SIZE($kdc)

        csd $at, $k0, 0*REG_SIZE($kdc)
        csd $v0, $k0, 1*REG_SIZE($kdc)
        csd $v1, $k0, 2*REG_SIZE($kdc)
        csd $a0, $k0, 3*REG_SIZE($kdc)
        csd $a1, $k0, 4*REG_SIZE($kdc)
        csd $a2, $k0, 5*REG_SIZE($kdc)
        csd $a3, $k0, 6*REG_SIZE($kdc)
        csd $a4, $k0, 7*REG_SIZE($kdc)
        csd $a5, $k0, 8*REG_SIZE($kdc)
        csd $a6, $k0, 9*REG_SIZE($kdc)
        csd $a7, $k0, 10*REG_SIZE($kdc)
        csd $t0, $k0, 11*REG_SIZE($kdc)
        csd $t1, $k0, 12*REG_SIZE($kdc)
        csd $t2, $k0, 13*REG_SIZE($kdc)
        csd $t3, $k0, 14*REG_SIZE($kdc)
        csd $s0, $k0, 15*REG_SIZE($kdc)
        csd $s1, $k0, 16*REG_SIZE($kdc)
        csd $s2, $k0, 17*REG_SIZE($kdc)
        csd $s3, $k0, 18*REG_SIZE($kdc)
        csd $s4, $k0, 19*REG_SIZE($kdc)
        csd $s5, $k0, 20*REG_SIZE($kdc)
        csd $s6, $k0, 21*REG_SIZE($kdc)
        csd $s7, $k0, 22*REG_SIZE($kdc)
        csd $t8, $k0, 23*REG_SIZE($kdc)
        csd $t9, $k0, 24*REG_SIZE($kdc)
        csd $gp, $k0, 25*REG_SIZE($kdc)
        csd $sp, $k0, 26*REG_SIZE($kdc)
        csd $fp, $k0, 27*REG_SIZE($kdc)
        csd $ra, $k0, 28*REG_SIZE($kdc)
        mfhi	$k1
        csd	$k1, $k0, 29*REG_SIZE($kdc)
        mflo	$k1
        csd	$k1, $k0, 30*REG_SIZE($kdc)
        dmfc0	$k1, $14		# EPC
        csd	$k1, $k0, 31*REG_SIZE($kdc)

		# Set up exception stack
        dla     $sp, __stop_exception_stack

		#
		# Restore the kernel's C0 after returning from the user
		# context -- this is the only capability used in the kernel
		# context other than KCC/KDC and T0 (for now).  In the future,
		# once the kernel uses capabilities in more intricate ways,
		# we'll need to restore other kernel capability context here.
		#
		csetdefault	$kdc

		#
		# Normal function invocation of kernel_exception().
		#
		dla		$t9, kernel_exception
		jalr		$t9
		nop

		#
		# Restore context from exception frame, which may have had
		# its contents replaced by the kernel.  Notice correspondence
		# to the layout of struct reg_frame.
		#
		dla	$k0, kernel_exception_framep_ptr
        cld	$k0, $k0, 0($kdc)

        cld $at, $k0, 0*REG_SIZE($kdc)
        cld $v0, $k0, 1*REG_SIZE($kdc)
        cld $v1, $k0, 2*REG_SIZE($kdc)
        cld $a0, $k0, 3*REG_SIZE($kdc)
        cld $a1, $k0, 4*REG_SIZE($kdc)
        cld $a2, $k0, 5*REG_SIZE($kdc)
        cld $a3, $k0, 6*REG_SIZE($kdc)
        cld $a4, $k0, 7*REG_SIZE($kdc)
        cld $a5, $k0, 8*REG_SIZE($kdc)
        cld $a6, $k0, 9*REG_SIZE($kdc)
        cld $a7, $k0, 10*REG_SIZE($kdc)
        cld $t0, $k0, 11*REG_SIZE($kdc)
        cld $t1, $k0, 12*REG_SIZE($kdc)
        cld $t2, $k0, 13*REG_SIZE($kdc)
        cld $t3, $k0, 14*REG_SIZE($kdc)
        cld $s0, $k0, 15*REG_SIZE($kdc)
        cld $s1, $k0, 16*REG_SIZE($kdc)
        cld $s2, $k0, 17*REG_SIZE($kdc)
        cld $s3, $k0, 18*REG_SIZE($kdc)
        cld $s4, $k0, 19*REG_SIZE($kdc)
        cld $s5, $k0, 20*REG_SIZE($kdc)
        cld $s6, $k0, 21*REG_SIZE($kdc)
        cld $s7, $k0, 22*REG_SIZE($kdc)
        cld $t8, $k0, 23*REG_SIZE($kdc)
        cld $t9, $k0, 24*REG_SIZE($kdc)
        cld $gp, $k0, 25*REG_SIZE($kdc)
        cld $sp, $k0, 26*REG_SIZE($kdc)
        cld $fp, $k0, 27*REG_SIZE($kdc)
        cld $ra, $k0, 28*REG_SIZE($kdc)
        cld	$k1, $k0, 29*REG_SIZE($kdc)
        mthi	$k1
        cld	$k1, $k0, 30*REG_SIZE($kdc)
        mtlo	$k1
        cld	$k1, $k0, 31*REG_SIZE($kdc)
        dmtc0	$k1, $14		# EPC

        clc $c0, $k0, 0*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c1, $k0, 1*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c2, $k0, 2*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c3, $k0, 3*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c4, $k0, 4*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c5, $k0, 5*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c6, $k0, 6*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c7, $k0, 7*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c8, $k0, 8*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c17, $k0, 9*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c18, $k0, 10*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $c25, $k0, 11*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $kr1c, $k0, 12*CAP_SIZE+MIPS_FRAME_SIZE($kdc)
        clc $epcc, $k0, 13*CAP_SIZE+MIPS_FRAME_SIZE($kdc)

		#
		# On general principle, flush the contents of $k0 and $k1
		# before returning to "userspace".
		#
		move	$k0, $zero
		move	$k1, $zero

		eret

kernel_cp2_exception_handler_end:
		.size kernel_cp2_exception_handler, kernel_cp2_exception_handler_end - kernel_cp2_exception_handler
		.end kernel_cp2_exception_handler

		.text
		.global kernel_cp2_ccall_handler
		.ent kernel_cp2_ccall_handler
kernel_cp2_ccall_handler:
		#
		# On general principle, flush the contents of $k0 and $k1
		# before returning to "userspace".
		#
        cgetcause   $k0
        srl $k0, $k0, 8
        bne $k0, $k1, kernel_cp2_creturn
        nop
        cgetdefault   $c2
        csetoffset  $c2, $c2, $sp
        li  $k0, 4
        cincoffset  $epcc, $epcc, $k0
        csc $epcc, $0, 0($kr1c)
        csc $c2, $0, CAP_SIZE($kr1c)
        dmfc0   $k0, $9 # read the current count register
        csd $v1, $0, 2*CAP_SIZE($kr1c)
        csw $k0, $0, 2*CAP_SIZE+REG_SIZE($kr1c)
        cfromptr    $c2, $c2, $0
        li  $k0, 3*CAP_SIZE
        cincoffset  $kr1c, $kr1c, $k0

        cgettype    $k0, $c6
        csetoffset  $kdc, $kdc, $k0
        cunseal $epcc, $c6, $kdc
        cunseal $c7, $c7, $kdc
        cgetoffset  $sp, $c7
        # extract the stack pointer of the callee
        csetoffset  $c7, $c7, $0
        csetdefault $c7
        cgetoffset  $k0, $epcc
		dmtc0	$k0, $14		# EPC
		move	$k0, $zero
		move	$k1, $zero
		eret

kernel_cp2_creturn:

    /* the callee should never trust the caller, as the callee can be called
     * by anyone. However, I think the caller should be able to choose whether
     * to trust the callee, as he knows who he is going to call. Therefore,
     * callee should always clear registers, except for callee saved ones and
     * return values.
     */
    clearlo 0xfff3 // keep v0, v1 for return values
    clearhi 0x0f00 // clear t8, t9, k0, k1
    cclearlo 0xfff7 // keep c3
    cclearhi 0x17f9 // don't clear c17, c18 as it's callee save, don't clear kernel and kr1c

        li  $k0, -3*CAP_SIZE
        cincoffset  $kr1c, $kr1c, $k0
        clc $epcc, $0, 0($kr1c)
        clc $c7, $0, CAP_SIZE($kr1c)
        # extract the stack pointer of the caller
        cgetoffset  $sp, $c7
        csetoffset  $c7, $c7, $0
        csetdefault $c7
        cgetoffset  $k0, $epcc
		dmtc0	$k0, $14		# EPC
		eret

kernel_cp2_ccall_handler_end:
		.size kernel_cp2_ccall_handler, kernel_cp2_ccall_handler_end - kernel_cp2_ccall_handler
		.end kernel_cp2_ccall_handler
