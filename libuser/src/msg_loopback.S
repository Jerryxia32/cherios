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

# Dispatch CCalls

	.text
	.global msg_entry_loopback
	.ent msg_entry_loopback
msg_entry_loopback:
    move    $v0, $a0
    move    $a0, $a1
    move    $a1, $a2
    move    $a2, $a3
    dla $t0, mutex
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
    dla $t0, mutex
    sw  $0, 0($t0)
    jr  $ra
    daddiu  $sp, $sp, 32

msg_entry_end_loopback:
	.end msg_entry_loopback
	.size msg_entry_loopback, msg_entry_end_loopback - msg_entry_loopback
