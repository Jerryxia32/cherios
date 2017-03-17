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

#.ifeq _MIPS_SZCAP-256
#.set CAP_SIZE_BITS, 5
#.else
#.ifeq _MIPS_SZCAP-128
#.set CAP_SIZE_BITS, 4
#.else
#.error "Unknown cap size"
#.endif
#.endif

.set REG_SIZE_BITS, 3
.set CAP_SIZE, _MIPS_SZCAP/8
.set MUTEX_ADDR, 0x100

# Dispatch CCalls

	.text
	.global msg_entry
	.ent msg_entry
msg_entry:
    li $t0, MUTEX_ADDR
mutex_loop:
    # mutex to prevent module from receiving another message while still
    # processing the previous one
    ll  $t1, 0($t0)
    bne $t1, $0, mutex_loop
    li  $t2, 1
    sc  $t2, 0($t0)
    beq $t2, $0, mutex_loop
    nop

	bltz		$v0, control
	nop

	dla		$t0, msg_methods
	dla		$t1, msg_methods_nb
    b		call
	nop

control:
	dla		$t0, ctrl_methods
	dla		$t1, ctrl_methods_nb
	sub		$v0, $0, $v0
call:
	ld		$t1, 0($t1)
	sub		$t1, $t1, $v0
	blez		$t1, overflow
	nop

	#Save sync_token
	dla		$t1, sync_token
    cgetdefault $c1
	csc		$c2, $t1, 0($c1)

	sll		$v0, $v0, REG_SIZE_BITS
	add		$t0, $t0, $v0
	ld		$t9, 0($t0)
	jalr    $t9
	nop
    b		end
	nop

overflow:
	move		$v0, $0
    cfromptr	$c3, $c3, $0
    b		end
	move		$v1, $0

end:
	#Restore sync_token
	dla		$t1, sync_token
    cgetdefault $c1
	clc		$c2, $t1, 0($c1)

    # release lock
    li $t0, MUTEX_ADDR
    sw  $0, 0($t0)

    li $v1, 1010
    syscall
    nop

	#We are scheduled again: new message
    b msg_entry
	nop
msg_entry_end:
	.end msg_entry
	.size msg_entry, msg_entry_end - msg_entry

#    .data
#    .global mutex
#mutex:
#    .skip (1<<(REG_SIZE_BITS-1)), 0
#mutex_end:
#    .size mutex, mutex_end - mutex

	.text
	.global msg_entry_loopback
	.ent msg_entry_loopback
msg_entry_loopback:
    cgetdefault $c25
    clc $c1, $sp, 0($c25)
    clc $c2, $sp, CAP_SIZE($c25)
    move    $v0, $a0
    move    $a0, $a1
    move    $a1, $a2
    move    $a2, $a3
    li $t0, MUTEX_ADDR
mutex_loop_loopback:
    # mutex to prevent module from receiving another message while still
    # processing the previous one
    ll  $t1, 0($t0)
    bne $t1, $0, mutex_loop_loopback
    li  $t2, 1
    sc  $t2, 0($t0)
    beq $t2, $0, mutex_loop_loopback
    nop

	bltz		$v0, control_loopback
	nop

	dla		$t0, msg_methods
	dla		$t1, msg_methods_nb
    b		call_loopback
	nop

control_loopback:
	dla		$t0, ctrl_methods
	dla		$t1, ctrl_methods_nb
	sub		$v0, $0, $v0
call_loopback:
	ld		$t1, 0($t1)
	sub		$t1, $t1, $v0
	blez		$t1, overflow_loopback
	nop

    daddiu  $sp, $sp, -32
    sd  $ra, 0($sp)
	sll		$v0, $v0, REG_SIZE_BITS
	add		$t0, $t0, $v0
	ld		$t9, 0($t0)
	jalr    $t9
	nop
    b		end_loopback
	nop

overflow_loopback:
	move		$v0, $0
    cfromptr	$c3, $c3, $0
    b		end_loopback
	move		$v1, $0

end_loopback:
    # release lock
    ld  $ra, 0($sp)
    li $t0, MUTEX_ADDR
    sw  $0, 0($t0)
    jr  $ra
    daddiu  $sp, $sp, 32

msg_entry_end_loopback:
	.end msg_entry_loopback
	.size msg_entry_loopback, msg_entry_end_loopback - msg_entry_loopback

	.text
	.global msg_entry_ccall
	.ent msg_entry_ccall
msg_entry_ccall:
    cgetoffset  $sp, $idc
    csetoffset  $idc, $idc, $0
    csetdefault $idc
    li $t0, MUTEX_ADDR
mutex_loop_ccall:
    # mutex to prevent module from receiving another message while still
    # processing the previous one
    ll  $t1, 0($t0)
    bne $t1, $0, mutex_loop_ccall
    li  $t2, 1
    sc  $t2, 0($t0)
    beq $t2, $0, mutex_loop_ccall
    nop

	bltz		$v0, control_ccall
    daddiu  $sp, $sp, -CAP_SIZE

	dla		$t0, msg_methods
	dla		$t1, msg_methods_nb
    b   call_ccall
	nop

control_ccall:
	dla		$t0, ctrl_methods
	dla		$t1, ctrl_methods_nb
	sub		$v0, $0, $v0
call_ccall:
	ld		$t1, 0($t1)
	sub		$t1, $t1, $v0
	blez		$t1, overflow_ccall
	nop

    sd  $ra, 0($sp)
	sll		$v0, $v0, REG_SIZE_BITS
	add		$t0, $t0, $v0
	ld		$t9, 0($t0)
	jalr    $t9
	nop
    ld  $ra, 0($sp)
    b   end_ccall
	nop

overflow_ccall:
	move		$v0, $0
    cfromptr	$c3, $c3, $0
    b   end_ccall
	move		$v1, $0

end_ccall:
    # release lock
    li $t0, MUTEX_ADDR
    sw  $0, 0($t0)
    daddiu  $sp, $sp, CAP_SIZE

    /* the callee should never trust the caller, as the callee can be called
     * by anyone. However, I think the caller should be able to choose whether
     * to trust the callee, as he knows who he is going to call. Therefore,
     * callee should always clear registers, except for callee saved ones and
     * return values.
     */
    dla $t0, return_cap
    cgetdefault $c25
    clc $idc, $t0, 0($c25)
    clearlo 0xfff3 // keep v0, v1 for return values
    clearhi 0x0f00 // clear t8, t9, k0, k1
    ccall   $idc, $idc, 41
    cclearlo 0xff36 // keep c3, c6, c7

msg_entry_ccall_end:
	.end msg_entry_ccall
	.size msg_entry_ccall, msg_entry_ccall_end - msg_entry_ccall
