/*
 * libmisc.h -- Misc. routines specific to S3.
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
 * 26-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBMISC_LIBMISC_H_
#define _LIBMISC_LIBMISC_H_


#define START_WORD		"<s>"
#define FINISH_WORD		"</s>"
#define SILENCE_WORD		"<sil>"
#define SILENCE_CIPHONE		"SIL"
#define BEGIN_SILENCE_CIPHONE	"SILb"
#define END_SILENCE_CIPHONE	"SILe"

#define LOGPROB_ZERO	((int32) 0xc8000000)		/* Approx -infinity */
#define RENORM_THRESH	((int32) ((LOGPROB_ZERO)>>1))	/* Bestscore getting close to 0 */

#define S3_MAX_FRAMES	30000	/* Max utterance length: 60sec long (@10ms/frame) */


#include "logs3.h"
#include "corpus.h"
#include "dictmisc.h"
#include "vector.h"


#endif
