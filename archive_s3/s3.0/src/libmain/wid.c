/*
 * wid.c -- Mapping word-IDs between LM and dictionary.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
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
