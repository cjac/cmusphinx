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
 * s3.h -- Some basic Sphinx-3 related definitions.
 * 
 * HISTORY
 * 
 * 30-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Eric Thayer's S3 trainer version.
 */


/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: s3.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef S3_H
#define S3_H


#define	S3_SUCCESS	0
#define S3_ERROR	-1
#define S3_WARNING	-2

/* These belong in libutil/prim_type.h */
#define MAX_IEEE_NORM_POS_FLOAT32	3.4e+38
#define MIN_IEEE_NORM_POS_FLOAT32	1.2e-38
#define MAX_IEEE_NORM_POS_FLOAT64	1.8e+307
#define MIN_IEEE_NORM_POS_FLOAT64	2.2e-308

#define START_WORD		"<s>"
#define FINISH_WORD		"</s>"
#define SILENCE_WORD		"<sil>"
#define SILENCE_CIPHONE		"SIL"
#define BEGIN_SILENCE_CIPHONE	"SILb"
#define END_SILENCE_CIPHONE	"SILe"

#define LOGPROB_ZERO	((int32) 0xc8000000)		/* Approx -infinity */
#define RENORM_THRESH	((int32) ((LOGPROB_ZERO)>>1))	/* Bestscore getting close to 0 */

#define S3_MAX_FRAMES	30000	/* Utterance size: 60sec long (@10ms/frame) */

/* The maximum # of states for any given acoustic model */
#define MAX_N_STATE	20

/* The maximum # of attributes associated with any given acoustic model */
#define MAX_N_ATTRIB	5

#ifndef TRUE
#define TRUE  1
#define FALSE 0	/* assume that true is never defined w/o false */
#endif

#define TYING_NON_EMITTING	(0xffffffff)
#define TYING_NO_ID		(0xffffffff)

#endif /* S3_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2002/12/03  23:02:31  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 * 
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * Revision 1.2  1995/10/10  12:25:04  eht
 * Add TYING_NO_ID to the set of symbolic constants defined.
 *
 * Revision 1.1  1995/10/09  21:17:24  eht
 * Initial revision
 *
 *
 */
