/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * fillpen.c -- Filler penalties (penalties for words that do not show up in
 * the main LM.
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
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _S3_FILLPEN_H_
#define _S3_FILLPEN_H_

#include <libutil/libutil.h>
#include "dict.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dict_t *dict;	/* Reference dictionary for which the filler word probabilities
			   are maintained in this structure */
    int32 *prob;	/* Filler word probability (in logs3 space, after
			   langwt and inspen application) */
    float64 lw;		/* Language weight */
    float64 wip;	/* Word insertion penalty */
} fillpen_t;


/*
 * Initialize filler probabilities (penalties, whatever) module and return a pointer to the
 * structure created.  Filler word probabilities are simple unigram probabilities.  Here is an
 * example of such a file (one entry per line; a word and a probability):
 *   <sil>  0.10792
 *   <uh>   0.00866
 *   <um>   0.00147
 * If the first character in a line is a '#', the line is treated as a comment and ignored.
 * If no filler probabilities file is provided, the silence word gets silprob, and all other
 * filler words get fillprob.  As with the trigram LM, the resulting log-probabilities are
 * multiplied by a language weight and finally a word insertion penalty is tacked on.
 */
fillpen_t *fillpen_init (dict_t *dict,		/* In: Dictionary containing filler words */
			 char *file,		/* In: Filler word probabilities file, if any */
			 float64 silprob,	/* In: Default probability for silence word */
			 float64 fillprob,	/* In: Default probability for non-silence filler
						   words */
			 float64 lw,		/* In: Language weight (see lm.h) */
			 float64 wip);		/* In: Word insertion penalty (see lm.h) */

/*
 * Return the filler word probability for the given dictionary word-ID.
 */
int32 fillpen (fillpen_t *f,		/* In: Filler word probabilities structure */
	       s3wid_t w);		/* In: Dictionary word-ID of filler word */

/* RAH 
   free memory allocated by fillpen_init
 */
void fillpen_free (fillpen_t *f);

#ifdef __cplusplus
}
#endif

#endif
