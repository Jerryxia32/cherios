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

  .text
  .global start
  .ent start
start:
  b           start_common
  nop

  .text
  .global sweep_line
  .ent sweep_line
sweep_line:
  clc         $c1, $zero, 0($c3)
  clc         $c2, $zero, 8($c3)
  ctestsubset $t9, $c4, $c1
  beqz        $t9, one
  clc         $c5, $zero, 16($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 0($c3)
one:
  ctestsubset $t9, $c4, $c2
  beqz        $t9, two
  clc         $c1, $zero, 24($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 8($c3)
two:
  ctestsubset $t9, $c4, $c5
  beqz        $t9, three
  clc         $c2, $zero, 32($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 16($c3)
three:
  ctestsubset $t9, $c4, $c1
  beqz        $t9, four
  clc         $c5, $zero, 40($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 24($c3)
four:
  ctestsubset $t9, $c4, $c2
  beqz        $t9, five
  clc         $c1, $zero, 48($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 32($c3)
five:
  ctestsubset $t9, $c4, $c5
  beqz        $t9, six
  clc         $c2, $zero, 56($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 40($c3)
six:
  ctestsubset $t9, $c4, $c1
  beqz        $t9, seven
  clc         $c5, $zero, 64($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 48($c3)
seven:
  ctestsubset $t9, $c4, $c2
  beqz        $t9, eight
  clc         $c1, $zero, 72($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 56($c3)
eight:
  ctestsubset $t9, $c4, $c5
  beqz        $t9, nine
  clc         $c2, $zero, 80($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 64($c3)
nine:
  ctestsubset $t9, $c4, $c1
  beqz        $t9, ten
  clc         $c5, $zero, 88($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 72($c3)
ten:
  ctestsubset $t9, $c4, $c2
  beqz        $t9, eleven
  clc         $c1, $zero, 96($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 80($c3)
eleven:
  ctestsubset $t9, $c4, $c5
  beqz        $t9, twelve
  clc         $c2, $zero, 104($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 88($c3)
twelve:
  ctestsubset $t9, $c4, $c1
  beqz        $t9, thirteen
  clc         $c5, $zero, 112($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 96($c3)
thirteen:
  ctestsubset $t9, $c4, $c2
  beqz        $t9, fourteen
  clc         $c1, $zero, 120($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 104($c3)
fourteen:
  ctestsubset $t9, $c4, $c5
  beqz        $t9, fifteen
  ctestsubset $t9, $c4, $c1
  ccleartag   $c5, $c5
  csc         $c5, $zero, 112($c3)
fifteen:
  beqz        $t9, sixteen
  ccleartag   $c1, $c1
  csc         $c1, $zero, 120($c3)
sixteen:
  jr          $ra
  nop

  .text
  .global sweep_line_nosubset
  .ent sweep_line_nosubset
sweep_line_nosubset:
  sd          $ra, -8($sp)
  clc         $c1, $zero, 0($c3)
  clc         $c2, $zero, 8($c3)
  jal         check_subset
  cmove       $c25, $c1
  beqz        $v0, one_no
  clc         $c5, $zero, 16($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 0($c3)
one_no:
  jal         check_subset
  cmove       $c25, $c2
  beqz        $v0, two_no
  clc         $c1, $zero, 24($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 8($c3)
two_no:
  jal         check_subset
  cmove       $c25, $c5
  beqz        $v0, three_no
  clc         $c2, $zero, 32($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 16($c3)
three_no:
  jal         check_subset
  cmove       $c25, $c1
  beqz        $v0, four_no
  clc         $c5, $zero, 40($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 24($c3)
four_no:
  jal         check_subset
  cmove       $c25, $c2
  beqz        $v0, five_no
  clc         $c1, $zero, 48($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 32($c3)
five_no:
  jal         check_subset
  cmove       $c25, $c5
  beqz        $v0, six_no
  clc         $c2, $zero, 56($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 40($c3)
six_no:
  jal         check_subset
  cmove       $c25, $c1
  beqz        $v0, seven_no
  clc         $c5, $zero, 64($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 48($c3)
seven_no:
  jal         check_subset
  cmove       $c25, $c2
  beqz        $v0, eight_no
  clc         $c1, $zero, 72($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 56($c3)
eight_no:
  jal         check_subset
  cmove       $c25, $c5
  beqz        $v0, nine_no
  clc         $c2, $zero, 80($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 64($c3)
nine_no:
  jal         check_subset
  cmove       $c25, $c1
  beqz        $v0, ten_no
  clc         $c5, $zero, 88($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 72($c3)
ten_no:
  jal         check_subset
  cmove       $c25, $c2
  beqz        $v0, eleven_no
  clc         $c1, $zero, 96($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 80($c3)
eleven_no:
  jal         check_subset
  cmove       $c25, $c5
  beqz        $v0, twelve_no
  clc         $c2, $zero, 104($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 88($c3)
twelve_no:
  jal         check_subset
  cmove       $c25, $c1
  beqz        $v0, thirteen_no
  clc         $c5, $zero, 112($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 96($c3)
thirteen_no:
  jal         check_subset
  cmove       $c25, $c2
  beqz        $v0, fourteen_no
  clc         $c1, $zero, 120($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 104($c3)
fourteen_no:
  jal         check_subset
  cmove       $c25, $c5
  beqz        $v0, fifteen_no
  nop
  jal         check_subset
  cmove       $c25, $c1
  ccleartag   $c5, $c5
  csc         $c5, $zero, 112($c3)
fifteen_no:
  beqz        $v0, sixteen_no
  ccleartag   $c1, $c1
  csc         $c1, $zero, 120($c3)
sixteen_no:
  ld          $ra, -8($sp)
  jr          $ra
  nop

check_subset:
  cgettag     $a3, $c4
  cgettag     $v1, $c25
  bne         $a3, $v1, check_subset_ret
  addiu       $v0, $zero, 0
  cgetbase    $v1, $c4
  cgetbase    $a0, $c25
  sltu        $a3, $a0, $v1
  bnez        $a3, check_subset_ret
  nop
  cgetlen     $a3, $c25
  cgetlen     $a1, $c4
  addu        $a3, $a3, $a0
  addu        $v1, $a1, $v1
  sltu        $a3, $v1, $a3
  bnez        $a3, check_subset_ret
  nop
  cgetperm    $a3, $c4
  cgetperm    $v0, $c25
  and         $a3, $v0, $a3
  xor         $a3, $a3, $v0
  sltiu       $v0, $a3, 1
check_subset_ret:
  jr          $ra
  nop

  .text
  .global sweep_line_four
  .ent sweep_line_four
  # $c4, c11, c12, c13 are the four mother capabilities to sweep.
  # XXX: This function uses more registers than CheriRTOS supports!!!
  # So please disable interrupts before using it.
sweep_line_four:
  clc         $c1, $zero, 0($c3)
  cgetdefault $c11
  cgetpcc     $c12
  clc         $c2, $zero, 8($c3)
  cgetdefault $c13
  ctestsubset $t9, $c4, $c1
  ctestsubset $t0, $c11, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c1
  or          $t9, $t9, $t0
  beqz        $t9, one_four
  clc         $c5, $zero, 16($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 0($c3)
one_four:
  ctestsubset $t9, $c4, $c2
  ctestsubset $t0, $c11, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c2
  or          $t9, $t9, $t0
  beqz        $t9, two_four
  clc         $c1, $zero, 24($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 8($c3)
two_four:
  ctestsubset $t9, $c4, $c5
  ctestsubset $t0, $c11, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c5
  or          $t9, $t9, $t0
  beqz        $t9, three_four
  clc         $c2, $zero, 32($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 16($c3)
three_four:
  ctestsubset $t9, $c4, $c1
  ctestsubset $t0, $c11, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c1
  or          $t9, $t9, $t0
  beqz        $t9, four_four
  clc         $c5, $zero, 40($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 24($c3)
four_four:
  ctestsubset $t9, $c4, $c2
  ctestsubset $t0, $c11, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c2
  or          $t9, $t9, $t0
  beqz        $t9, five_four
  clc         $c1, $zero, 48($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 32($c3)
five_four:
  ctestsubset $t9, $c4, $c5
  ctestsubset $t0, $c11, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c5
  or          $t9, $t9, $t0
  beqz        $t9, six_four
  clc         $c2, $zero, 56($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 40($c3)
six_four:
  ctestsubset $t9, $c4, $c1
  ctestsubset $t0, $c11, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c1
  or          $t9, $t9, $t0
  beqz        $t9, seven_four
  clc         $c5, $zero, 64($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 48($c3)
seven_four:
  ctestsubset $t9, $c4, $c2
  ctestsubset $t0, $c11, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c2
  or          $t9, $t9, $t0
  beqz        $t9, eight_four
  clc         $c1, $zero, 72($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 56($c3)
eight_four:
  ctestsubset $t9, $c4, $c5
  ctestsubset $t0, $c11, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c5
  or          $t9, $t9, $t0
  beqz        $t9, nine_four
  clc         $c2, $zero, 80($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 64($c3)
nine_four:
  ctestsubset $t9, $c4, $c1
  ctestsubset $t0, $c11, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c1
  or          $t9, $t9, $t0
  beqz        $t9, ten_four
  clc         $c5, $zero, 88($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 72($c3)
ten_four:
  ctestsubset $t9, $c4, $c2
  ctestsubset $t0, $c11, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c2
  or          $t9, $t9, $t0
  beqz        $t9, eleven_four
  clc         $c1, $zero, 96($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 80($c3)
eleven_four:
  ctestsubset $t9, $c4, $c5
  ctestsubset $t0, $c11, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c5
  or          $t9, $t9, $t0
  beqz        $t9, twelve_four
  clc         $c2, $zero, 104($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 88($c3)
twelve_four:
  ctestsubset $t9, $c4, $c1
  ctestsubset $t0, $c11, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c1
  or          $t9, $t9, $t0
  beqz        $t9, thirteen_four
  clc         $c5, $zero, 112($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 96($c3)
thirteen_four:
  ctestsubset $t9, $c4, $c2
  ctestsubset $t0, $c11, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c2
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c2
  or          $t9, $t9, $t0
  beqz        $t9, fourteen_four
  clc         $c1, $zero, 120($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 104($c3)
fourteen_four:
  ctestsubset $t9, $c4, $c5
  ctestsubset $t0, $c11, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c5
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c5
  or          $t9, $t9, $t0
  beqz        $t9, fifteen_four
  ctestsubset $t9, $c4, $c1
  ccleartag   $c5, $c5
  csc         $c5, $zero, 112($c3)
fifteen_four:
  ctestsubset $t0, $c11, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c12, $c1
  or          $t9, $t9, $t0
  ctestsubset $t0, $c13, $c1
  or          $t9, $t9, $t0
  beqz        $t9, sixteen_four
  ccleartag   $c1, $c1
  csc         $c1, $zero, 120($c3)
sixteen_four:
  jr          $ra
  nop

  .rodata
  .global __tags_bin_start
  .global __tags_bin_end
  .align 5
__tags_bin_start:
  .incbin"tags.bin"
__tags_bin_end:
