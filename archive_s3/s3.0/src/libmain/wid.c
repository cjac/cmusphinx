/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * wid.c -- Mapping word-IDs between LM and dictionary.
 * 
 * 
 * HISTORY
 * 
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include "dict.h"
#include "lm.h"


s3lmwid_t *wid_dict_lm_map (dict_t *dict, lm_t *lm)
{
    int32 u, n;
    s3wid_t w;
    s3lmwid_t *map;
    
    assert (dict_size(dict) > 0);
    map = (s3lmwid_t *) ckd_calloc (dict_size(dict), sizeof(s3lmwid_t));
    for (n = 0; n < dict_size(dict); n++)
	map[n] = BAD_LMWID;
    
    n = 0;
    for (u = 0; u < lm->n_ug; u++) {
	w = dict_wordid (dict, lm->wordstr[u]);
	lm->ug[u].dictwid = w;
	
	if (NOT_WID(w)) {
	    n++;
	} else {
	    if (dict_filler_word (dict, w))
		E_ERROR("Filler dictionary word '%s' found in LM\n", lm->wordstr[u]);
	    
	    if (w != dict_basewid (dict, w)) {
		E_ERROR("LM word '%s' is an alternative pronunciation in dictionary\n",
			lm->wordstr[u]);
		
		w = dict_basewid (dict, w);
		lm->ug[u].dictwid = w;
	    }
	    
	    for (; IS_WID(w); w = dict_nextalt(dict, w))
		map[w] = (s3lmwid_t) u;
	}
    }
    
    if (n > 0)
	E_INFO("%d LM words not in dictionary; ignored\n", n);
    
    return map;
}
