/* ====================================================================
 * Copyright (c) 1993-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

/*
 * **********************************************
 * DESCRIPTION
 *	swapLong(int32 *p)	- do a int32 byte swap at p.
 */

#include "s2types.h"

void swapLong(intp)
/*------------------------------------------------------------*
 * Swap the int32 integer at intp
 */
int32 *intp;
{
  *intp = ((*intp << 24) & 0xFF000000) |
	  ((*intp <<  8) & 0x00FF0000) |
	  ((*intp >>  8) & 0x0000FF00) |
	  ((*intp >> 24) & 0x000000FF);
}

void swapShortBuf (p, cnt)
int16 *p;
int32 cnt;
{
    while (cnt-- > 0) {
	*p = ((*p << 8) & 0x0FF00) |
	     ((*p >>  8) & 0x00FF);
	++p;	/* apollo compiler seems to break with *p++ = f(*p) */
    }
}


void swapLongBuf (p, cnt)
int32 *p;
int32 cnt;
{
    while (cnt-- > 0) {
	*p = ((*p << 24) & 0xFF000000) |
	     ((*p <<  8) & 0x00FF0000) |
	     ((*p >>  8) & 0x0000FF00) |
	     ((*p >> 24) & 0x000000FF);
	++p;	/* apollo compiler seems to break with *p++ = f(*p) */
    }
}
