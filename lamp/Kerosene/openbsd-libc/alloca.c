/* $OpenBSD: alloca.S,v 1.4 2005/08/07 16:40:14 espie Exp $ */
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* like alloc, but automatic free in return */

#ifdef __MC68K__

#ifdef OPENBSD

#include "DEFS.h"

ENTRY(alloca)
	movl	sp@,a0		/* save return addr */
	movl	sp,d0		/* get current SP value */
	subl	sp@(4),d0	/* allocate requested space */
	andb	#~3,d0		/* longword align for efficiency */
	addql	#8,d0		/* reuse space of call frame */
	movl	d0,sp		/* set new SP value */
	lea	sp@(-4),sp	/* account for argument pop in caller */
	jmp 	a0@		/* funny return */

#endif

#ifdef __MWERKS__

// Translation to Motorola syntax by Joshua Juran

#ifdef __cplusplus
extern "C" {
#endif

extern void* alloca();

asm void* alloca()
{
	MOVEA.L	(SP),A0   // save return addr
	MOVE.L	SP,D0     // get current SP value
	SUB.L	4(SP),D0  // allocate requested space
	AND.B	#~3,D0    // longword align for efficiency
	ADDQ.L	#8,D0     // reuse space of call frame
	MOVEA.L	D0,SP     // set new SP value
	LEA		-4(SP),SP // account for argument pop in caller
	JMP		(A0)      // funny return
}

#ifdef __cplusplus
}
#endif

#endif  // __MWERKS__

#endif  // __MC68K__

