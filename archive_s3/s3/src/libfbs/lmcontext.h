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
