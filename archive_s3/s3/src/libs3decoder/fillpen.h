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

#endif
