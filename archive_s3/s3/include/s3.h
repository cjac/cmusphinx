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
 * Revision 1.1  2000/04/24  09:39:41  lenzo
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
