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
 * line2wid.c -- Convert a ref or hyp line (word string sequence) to an array of
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


#include <libutil/libutil.h>
#include <main/dict.h>


int32 is_uttid (char *word, char *uttid)
{
    int32 k;
    
    k = strlen(word);
    if ((word[0] == '(') && (word[k-1] == ')')) {
	word[k-1] = '\0';
	strcpy (uttid, word+1);
	word[k-1] = ')';
	return 1;
    }
    return 0;
}


int32 line2wid (dict_t *dict, char *line, s3wid_t *wid, int32 max_n_wid, int32 add_oov,
		char *uttid)
{
    char *lp, word[1024];
    int32 n, k;
    s3wid_t w;
    s3cipid_t ci[1];
    
    uttid[0] = '\0';
    ci[0] = (s3cipid_t) 0;
    
    lp = line;
    n = 0;
    while (sscanf (lp, "%s%n", word, &k) == 1) {
	lp += k;

	if (n >= max_n_wid)
	    return -n;
	
	if (is_uttid (word, uttid))
	    break;
	
	wid[n] = dict_wordid (dict, word);	/* Up to caller to handle BAD_WIDs */
	if (NOT_WID(wid[n])) {
	    /* OOV word */
	    if (add_oov) {
		E_INFO("Adding %s to dictionary\n", word);
		wid[n] = dict_add_word (dict, word, NULL, 0);
		if (NOT_WID(wid[n]))
		    E_FATAL("dict_add_word(%s) failed for line: %s\n", word, line);
	    } else
		E_FATAL("Unknown word (%s) in line: %s\n", word, line);
	}
	
	n++;
    }
    
    if (sscanf (lp, "%s", word) == 1)	/* Check that line really ended */
	E_WARN("Nonempty data ignored after uttid(%s) in line: %s\n", uttid, line);
    
    return n;
}
