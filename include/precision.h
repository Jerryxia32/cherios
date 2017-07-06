/*-
 * Copyright (c) 2017 Hongyan Xia
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef _PRECISION_H_
#define	_PRECISION_H_

#include"mips.h"
#include"cheric.h"

//count trailing zeroes in unsigned ints
static inline size_t ctz(size_t input)
{
    size_t result=0, count=0;
    while((input&1) == 0 && count < 8*sizeof(size_t)) {   
        ++count;
        result++;
        input >>= 1;
    }   
    return result;
}

//determine whether this start address and chunk size can be precisely
//represented by a cap with manWidth mantissa bits.
static inline int can_represent(size_t size, size_t startAddr, size_t tbWidth)
{
    size_t endAddr = startAddr + size;
    size_t startTz = ctz(startAddr);
    size_t endTz = ctz(endAddr);
    startTz = (startTz<endTz)? startTz : endTz;
    startAddr >>= startTz;
    endAddr >>= startTz;
    if(endAddr-startAddr>((1<<tbWidth)-1)) return 0;
    else return 1;
}

//this function takes the requested malloc size as input
//and outputs the alignment requirement for caps with manWidth
//bits of mantissa
static inline size_t align_chunk(size_t reqSize, size_t tbWidth)
{
    if(!reqSize) return 0;
    size_t result = 1;
    while(reqSize!=0) {
        result <<= 1;
        reqSize >>= 1;
    }

    return result>>tbWidth;
}

//this function rounds up the size to the next alignment boundary
static inline size_t round_size(size_t reqSize, size_t tbWidth)
{
    size_t align = align_chunk(reqSize, tbWidth);
    if(align==0) align+=1;
    size_t result;
    result = reqSize & ~(align-1);
    if((reqSize & (align-1)) != 0)
        result += align;
    //if( result - reqSize >= 8 ) result -= 8;
    return result;
}

#endif
