/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 12-Jul-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBFBS_S3TYPES_H_
#define _LIBFBS_S3TYPES_H_


#include <libutil/prim_type.h>


/*
 * Size definitions for more semantially meaningful units.
 */
typedef int16	s3cipid_t;	/* Ci phone id */
typedef int32	s3pid_t;	/* Phone id (triphone or ciphone) */
typedef int32	s3wid_t;	/* Dictionary word id */
typedef uint16	s3lmwid_t;	/* LM word id (uint16 for conserving space) */
typedef int32	s3latid_t;	/* Lattice entry id */
typedef int16   s3frmid_t;	/* Frame id (must be SIGNED integer) */
typedef uint16  s3senid_t;	/* Senone id */
typedef int32   s3mgauid_t;	/* Mixture-gaussian codebook id */
typedef int32	s3tmatid_t;	/* Transition matrix id; there can be as many as pids */

/*
 * Illegal value definitions and tests for specific types.
 */
#define BAD_CIPID	((s3cipid_t) -1)
#define NOT_CIPID(p)	((p)<0)
#define IS_CIPID(p)	((p)>=0)

#define BAD_PID		((s3pid_t) -1)
#define NOT_PID(p)	((p)<0)
#define IS_PID(p)	((p)>=0)

#define BAD_TMATID	((s3tmatid_t) -1)
#define NOT_TMATID(t)	((t)<0)
#define IS_TMATID(t)	((t)>=0)

#define BAD_WID		((s3wid_t) -1)
#define NOT_WID(w)	((w)<0)
#define IS_WID(w)	((w)>=0)

#define BAD_LMWID	((s3lmwid_t) 0xffff)
#define NOT_LMWID(w)	((w)==BAD_LMWID)
#define IS_LMWID(w)	((w)!=BAD_LMWID)

#define BAD_LATID	((s3latid_t) -1)
#define NOT_LATID(l)	((l)<0)
#define IS_LATID(l)	((l)>=0)

#define BAD_FRMID	((s3frmid_t) -1)
#define NOT_FRMID(f)	((f)<0)
#define IS_FRMID(f)	((f)>=0)

#define BAD_SENID	((s3senid_t) 0xffff)
#define NOT_SENID(s)	((s)==BAD_SENID)
#define IS_SENID(s)	((s)!=BAD_SENID)

#define BAD_MGAUID	((s3mgauid_t) -1)
#define NOT_MGAUID(m)	((m)<0)
#define IS_MGAUID(m)	((m)>=0)


#endif
