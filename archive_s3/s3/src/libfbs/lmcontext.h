/*
 * lmcontext.h -- Surrounding LM context for each utterance
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


#ifndef _LMCONTEXT_H_
#define _LMCONTEXT_H_


#include <libmisc/corpus.h>
#include <s3.h>
#include "dict.h"


/*
 * Read LM context words for the given uttid from the given corpus.  Two words of
 * preceding context, and 1 word of succeeding context.  The preceding context can be:
 * 	- -		(i.e., no context, use unigram transition to first word)
 * 	- <word>	(single word context, can be START_WORD)
 * 	<word1> <word2>	(two word context)
 * The succeeding context can be:
 * 	-		(i.e., no context, utt can end in any word)
 * 	FINISH_WORD	(utt must end in FINISH_WORD).
 * (FATAL_ERROR if any error, such as unknown word, or missing context spec for uttid.)
 */
void lmcontext_load (corpus_t *corp,	/* In: Corpus to look up */
		     char *uttid,	/* In: Uttid to look for in corpus */
		     s3wid_t *pred,	/* Out: pred[0],pred[1] = two word history;
					   pred[0] is earlier than pred[1] */
		     s3wid_t *succ);	/* Out: *succ = FINISH_WORD id or BAD_WID */

#endif
