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
 * line2wid.h -- Convert a ref or hyp line (word string sequence) to an array of
 * 		word ids.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 27-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LINE2WID_
#define _LINE2WID_


#include <main/s3types.h>
#include <main/dict.h>

/*
 * Convert a line of words terminated by an (uttid) into a corresponding array of word
 * ids.  The word ids may be BAD_WID if not found in the given pronunciation dictionary.
 * Return value: no. of words read into wid; -ve if any error occurs.
 */
int32 line2wid (dict_t *dict,		/* In: Dictionary to use for building word ids */
		char *line,		/* In: Input text string terminated by (uttid) */
		s3wid_t *wid,		/* Out: Array of word-ids for words in line;
					   wid[] must be allocated by caller */
		int32 max_n_wid,	/* In: Size of wid[] */
		int32 add_oov,		/* In: If TRUE, add OOV words to dictionary with
					   no pronunciation; otherwise leave their word
					   id as BAD_WID */
		char *uttid);		/* Out: Utterance id encountered at the end of
					   line; uttid[] must be allocated by caller */


/*
 * Check if word is of the form (uttid).  If so, copy into uttid and return 1.
 * Otherwise return 0.  uttid must be allocated by caller and long enough to fit
 * any utterance id.
 */
int32 is_uttid (char *word, char *uttid);


#endif
