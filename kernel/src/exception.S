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

.set noat
.set noreorder
#include<asm.h>

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
    .global kernel_exception_trampoline_end
    .ent kernel_exception_trampoline
kernel_exception_trampoline:
    j           kernel_cp2_exception_handler
    csetoffset  $kdc, $kdc, $0
kernel_exception_trampoline_end:

    .global kernel_ccall_trampoline
    .global kernel_ccall_trampoline_end
    .ent kernel_ccall_trampoline
kernel_ccall_trampoline:
    j           kernel_cp2_ccall_handler
    li          $k1, 5
kernel_ccall_trampoline_end:

    .global kernel_tlb_trampoline
    .global kernel_tlb_trampoline_end
    .ent kernel_tlb_trampoline
kernel_tlb_trampoline:
    j           kernel_tlb_handler
    csetoffset  $kdc, $kdc, $0
kernel_tlb_trampoline_end:

    .text
    .ent kernel_tlb_handler
kernel_tlb_handler:
    # set up a small stack
    la          $k1, __stop_exception_stack # $k1 becomes the stack pointer
    csd         $s0, $k1, -1*REG_SIZE($kdc)
    csd         $s1, $k1, -2*REG_SIZE($kdc)
    csc         $c1, $k1, -3*REG_SIZE-CAP_SIZE($kdc)
    # retrieve the no_store_cap bit in the ttable
    dmfc0       $k0, $MIPS_CP0_REG_ENTRYHI
    dsrl        $k0, $k0, 15
    andi        $k0, $k0, 0xfff
    la          $s0, ttableCap
    clc         $c1, $s0, 0($kdc)
    clbu        $s0, $k0, 0($c1) # load the byte into $k1
    xori        $s0, $s0, 0xff
    dmfc0       $k0, $MIPS_CP0_REG_ENTRYHI
    dsrl        $k0, $k0, 12
    andi        $k0, $k0, 0x7
    dsrl        $s0, $s0, $k0
    csd         $s0, $k1, -3*REG_SIZE($kdc) # the shifted byte is stored

    andi        $s0, $s0, 0x1
    dsll32      $s0, $s0, 31
    daddiu      $s0, $s0, 7
    dmfc0       $k0, $MIPS_CP0_REG_ENTRYHI
    dsrl        $k0, $k0, 6
    or          $s0, $k0, $s0
    ori         $s0, $s0, 0x18
    dmtc0       $s0, $MIPS_CP0_REG_ENTRYLO0
    dsll        $s0, $s0, 1
    dsrl        $s0, $s0, 1
    cld         $s1, $k1, -3*REG_SIZE($kdc) # the shifted byte is loaded
    andi        $s1, $s1, 0x2
    dsll32      $s1, $s1, 30
    or          $s1, $s1, $s0
    ori         $s1, $s1, 0x58
    dmtc0       $s1, $MIPS_CP0_REG_ENTRYLO1
    tlbwr
    cld         $s0, $k1, -1*REG_SIZE($kdc)
    cld         $s1, $k1, -2*REG_SIZE($kdc)
    clc         $c1, $k1, -3*REG_SIZE-CAP_SIZE($kdc)
    eret
kernel_tlb_handler_end:

#
# Assembly-level exception handler routine -- save context, invoke
# kernel_exception() with a reference to the frame, then restore context and
# return from the exception handler.
#
# XXXRW: Note that we use a global pointer for the exception frame, so this is
# inherrently single-CPU.
#
    .text
    .ent kernel_cp2_exception_handler
kernel_cp2_exception_handler:

    #
    # Since we are still running with user capabilities in most
    # capability registers, we must access all kernel data
    # relative to KDC.  First save and clear CP2 context, then
    # move on to general-purpose registers.
    #
    la          $k0, kernel_exception_framep_ptr
    clw         $k0, $k0, 0($kdc)

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
    mfc0        $k1, $MIPS_CP0_REG_EPC
    csd         $k1, $k0, 18*REG_SIZE($kdc)

    # Set up exception stack
    la          $sp, __stop_exception_stack
    
    # Restore the kernel's C0 after returning from the user
    # context -- this is the only capability used in the kernel
    # context other than KCC/KDC and T0 (for now).  In the future,
    # once the kernel uses capabilities in more intricate ways,
    # we'll need to restore other kernel capability context here.
    jal         kernel_exception
    csetdefault	$kdc
    
    # Restore context from exception frame, which may have had
    # its contents replaced by the kernel.  Notice correspondence
    # to the layout of struct reg_frame.
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
    mtc0        $k1, $MIPS_CP0_REG_EPC
    
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
    
    # On general principle, flush the contents of $k0 and $k1
    # before returning to "userspace".
    clearhi     0x0c00
    eret
kernel_cp2_exception_handler_end:

    .text
    .ent kernel_cp2_ccall_handler
kernel_cp2_ccall_handler:
    # We have removed the exception based ccall, trap if entered.
    teq         $0, $0
kernel_cp2_ccall_handler_end:
