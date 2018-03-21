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
  ctestsubset $t0, $c4, $c1
  beqz        $t0, one
  clc         $c5, $zero, 16($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 0($c3)
one:
  ctestsubset $t0, $c4, $c2
  beqz        $t0, two
  clc         $c1, $zero, 24($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 8($c3)
two:
  ctestsubset $t0, $c4, $c5
  beqz        $t0, three
  clc         $c2, $zero, 32($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 16($c3)
three:
  ctestsubset $t0, $c4, $c1
  beqz        $t0, four
  clc         $c5, $zero, 40($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 24($c3)
four:
  ctestsubset $t0, $c4, $c2
  beqz        $t0, five
  clc         $c1, $zero, 48($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 32($c3)
five:
  ctestsubset $t0, $c4, $c5
  beqz        $t0, six
  clc         $c2, $zero, 56($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 40($c3)
six:
  ctestsubset $t0, $c4, $c1
  beqz        $t0, seven
  clc         $c5, $zero, 64($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 48($c3)
seven:
  ctestsubset $t0, $c4, $c2
  beqz        $t0, eight
  clc         $c1, $zero, 72($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 56($c3)
eight:
  ctestsubset $t0, $c4, $c5
  beqz        $t0, nine
  clc         $c2, $zero, 80($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 64($c3)
nine:
  ctestsubset $t0, $c4, $c1
  beqz        $t0, ten
  clc         $c5, $zero, 88($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 72($c3)
ten:
  ctestsubset $t0, $c4, $c2
  beqz        $t0, eleven
  clc         $c1, $zero, 96($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 80($c3)
eleven:
  ctestsubset $t0, $c4, $c5
  beqz        $t0, twelve
  clc         $c2, $zero, 104($c3)
  ccleartag   $c5, $c5
  csc         $c5, $zero, 88($c3)
twelve:
  ctestsubset $t0, $c4, $c1
  beqz        $t0, thirteen
  clc         $c5, $zero, 112($c3)
  ccleartag   $c1, $c1
  csc         $c1, $zero, 96($c3)
thirteen:
  ctestsubset $t0, $c4, $c2
  beqz        $t0, fourteen
  clc         $c1, $zero, 120($c3)
  ccleartag   $c2, $c2
  csc         $c2, $zero, 104($c3)
fourteen:
  ctestsubset $t0, $c4, $c5
  beqz        $t0, fifteen
  ctestsubset $t0, $c4, $c1
  ccleartag   $c5, $c5
  csc         $c5, $zero, 112($c3)
fifteen:
  beqz        $t0, sixteen
  ccleartag   $c1, $c1
  csc         $c1, $zero, 120($c3)
sixteen:
  jr          $ra
