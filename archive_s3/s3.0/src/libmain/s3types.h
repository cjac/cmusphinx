/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * s3types.h -- Types specific to s3 decoder.
 *
 * 
 * HISTORY
 * 
 * 23-Apr-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed MAX_CIPID from 128 to 127.
 * 
 * 12-Jul-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_S3TYPES_H_
#define _LIBMAIN_S3TYPES_H_


#include <libutil/prim_type.h>


/*
 * Size definitions for more semantially meaningful units.
 * Illegal value definitions, limits, and tests for specific types.
 */

/* CI phone id */
typedef int8		s3cipid_t;
#define BAD_CIPID	((s3cipid_t) -1)
#define NOT_CIPID(p)	((p)<0)
#define IS_CIPID(p)	((p)>=0)
#define MAX_CIPID	127

/* Phone id (triphone or ciphone) */
typedef int32		s3pid_t;
#define BAD_PID		((s3pid_t) -1)
#define NOT_PID(p)	((p)<0)
#define IS_PID(p)	((p)>=0)
#define MAX_PID		((int32)0x7fffffff)

/* Transition matrix id; perhaps as many as pids */
typedef int32		s3tmatid_t;
#define BAD_TMATID	((s3tmatid_t) -1)
#define NOT_TMATID(t)	((t)<0)
#define IS_TMATID(t)	((t)>=0)
#define MAX_TMATID	((int32)0x7fffffff)

/* Dictionary word id */
typedef int32		s3wid_t;
#define BAD_WID		((s3wid_t) -1)
#define NOT_WID(w)	((w)<0)
#define IS_WID(w)	((w)>=0)
#define MAX_WID		((int32)0x7fffffff)

/* LM word id (uint16 for conserving space) */
typedef uint16		s3lmwid_t;
#define BAD_LMWID	((s3lmwid_t) 0xffff)
#define NOT_LMWID(w)	((w)==BAD_LMWID)
#define IS_LMWID(w)	((w)!=BAD_LMWID)
#define MAX_LMWID	((uint32)0xfffe)

/* Senone id */
typedef int16   	s3senid_t;
#define BAD_SENID	((s3senid_t) -1)
#define NOT_SENID(s)	((s)<0)
#define IS_SENID(s)	((s)>=0)
#define MAX_SENID	((int32)0x7fff)

/* Mixture-gaussian codebook id */
typedef int16   	s3mgauid_t;
#define BAD_MGAUID	((s3mgauid_t) -1)
#define NOT_MGAUID(m)	((m)<0)
#define IS_MGAUID(m)	((m)>=0)
#define MAX_MGAUID	((int16)0x7fff)

/* Frame id (must be SIGNED integer) */
typedef int16   	s3frmid_t;
#define BAD_FRMID	((s3frmid_t) -1)
#define NOT_FRMID(f)	((f)<0)
#define IS_FRMID(f)	((f)>=0)
#define MAX_FRMID	((int32)0x7fff)

/* Lattice entry id */
typedef int32		s3latid_t;
#define BAD_LATID	((s3latid_t) -1)
#define NOT_LATID(l)	((l)<0)
#define IS_LATID(l)	((l)>=0)
#define MAX_LATID	((int32)0x7fffffff)


#endif
