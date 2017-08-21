#-
# Copyright (c) 2016 Hongyan Xia
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

.macro  csyscall   jmpreg
    .word (0x12 << 26) | (0x14 << 21)  | (\jmpreg << 11)
.endm

# Dispatch CCalls. $a3 is used as a temp register because we know there are
# only 3 arguments in CCall.

# kernel msg passing syscall
	.text
	.global msg_entry
	.ent msg_entry
msg_entry:
    li          $v1, MUTEX_ADDR
    # mutex to prevent module from receiving another message while still
    # processing the previous one
    lw          $a3, 0($v1)
    bne         $a3, $0, locked
    li          $t9, 1
    ll          $a3, 0($v1)
    sc          $t9, 0($v1)
    beq         $t9, $0, locked
    nop

    bltz        $v0, control
    nop

    la          $v1, msg_methods
    la          $a3, msg_methods_nb
    b           call
    nop

control:
    la          $v1, ctrl_methods
    la          $a3, ctrl_methods_nb
    sub         $v0, $0, $v0
call:
    lw          $a3, 0($a3)
    sub         $a3, $a3, $v0
    blez        $a3, overflow
    nop

    #Save sync_token
    la          $a3, sync_token
    cgetdefault $c1
    csc         $c2, $a3, 0($c1)

    sll         $v0, $v0, SIZET_SIZE_BITS
    add         $v1, $v1, $v0
    lw          $t9, 0($v1)
    jalr        $t9
    nop
    b           end
    nop

overflow:
    move        $v0, $0
    cfromptr    $c3, $c3, $0
    b           end
    move        $v1, $0

end:
    #Restore sync_token
    la          $a3, sync_token
    cgetdefault $c1
    clc         $c2, $a3, 0($c1)
    
    # release lock
    li          $v1, MUTEX_ADDR
    sw          $0, 0($v1)
    
    li          $v1, 1010
    syscall
    
    #We are scheduled again: new message
    b           msg_entry
    nop

locked:
    li          $v1, 13
    move        $a3, $a0
    li          $a0, 0
    syscall
    b           msg_entry
    move        $a0, $a3

msg_entry_end:

# CCallFast
	.text
	.global msg_entry_ccall
	.ent msg_entry_ccall
msg_entry_ccall:
    cgetoffset  $sp, $idc
    csetoffset  $idc, $idc, $0
    csetdefault $idc
mutex_loop_ccall:
    li          $v1, MUTEX_ADDR
    # mutex to prevent module from receiving another message while still
    # processing the previous one
    lw          $a3, 0($v1)
    bne         $a3, $0, locked_ccall
    li          $t9, 1
    ll          $a3, 0($v1)
    sc          $t9, 0($v1)
    beq         $t9, $0, locked_ccall
    nop

    bltz		$v0, control_ccall
    addiu       $sp, $sp, -CAP_SIZE

    la		    $v1, msg_methods
    la		    $a3, msg_methods_nb
    b           call_ccall
    nop

control_ccall:
    la		    $v1, ctrl_methods
    la		    $a3, ctrl_methods_nb
    sub		    $v0, $0, $v0
call_ccall:
    lw          $a3, 0($a3)
    sub         $a3, $a3, $v0
    blez        $a3, overflow_ccall
    nop

    sd          $ra, 0($sp)
    sll         $v0, $v0, SIZET_SIZE_BITS
    add         $v1, $v1, $v0
    lw          $t9, 0($v1)
    jalr        $t9
    nop
    ld          $ra, 0($sp)
    b           end_ccall
    nop

overflow_ccall:
    move        $v0, $0
    cfromptr    $c3, $c3, $0
    b           end_ccall
    move        $v1, $0

end_ccall:
    # release lock
    li          $v1, MUTEX_ADDR
    sw          $0, 0($v1)
    daddiu      $sp, $sp, CAP_SIZE

    /* the callee should never trust the caller, as the callee can be called
     * by anyone. However, I think the caller should be able to choose whether
     * to trust the callee, as he knows who he is going to call. Therefore,
     * callee should always clear registers, except for callee saved ones and
     * return values.
     */
    dla         $v1, return_cap
    clc         $c25, $v1, 0($idc)
    clearlo     0xfff3 // keep v0, v1 for return values
    clearhi     0x0f00 // clear t8, t9, k0, k1
    csyscall    25
    cclearlo    0xff36 // keep c3, c6, c7

locked_ccall:
    li          $v1, 13
    move        $a3, $a0
    li          $a0, 0
    syscall
    b           mutex_loop_ccall
    move        $a0, $a3

msg_entry_ccall_end:
