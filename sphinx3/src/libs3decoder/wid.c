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


#include "wid.h"


s3lmwid_t *wid_dict_lm_map (dict_t *dict, lm_t *lm)
{
    int32 u, n;
    s3wid_t w;
    s3lmwid_t *map;
    
    assert (dict_size(dict) > 0);
    map = (s3lmwid_t *) ckd_calloc (dict_size(dict), sizeof(s3lmwid_t));
    for (n = 0; n < dict_size(dict); n++)
	map[n] = BAD_S3LMWID;
    
    n = 0;
    for (u = 0; u < lm_n_ug(lm); u++) {
	w = dict_wordid (dict, lm_wordstr(lm, u));
	lm_lmwid2dictwid(lm, u) = w;
	
	if (NOT_S3WID(w)) {
	    n++;
	} else {
	    if (dict_filler_word (dict, w))
		E_ERROR("Filler dictionary word '%s' found in LM\n", lm_wordstr(lm, u));
	    
	    if (w != dict_basewid (dict, w)) {
		E_ERROR("LM word '%s' is an alternative pronunciation in dictionary\n",
			lm_wordstr(lm, u));
		
		w = dict_basewid (dict, w);
		lm_lmwid2dictwid(lm, u) = w;
	    }
	    
	    for (; IS_S3WID(w); w = dict_nextalt(dict, w))
		map[w] = (s3lmwid_t) u;
	}
    }
    
    if (n > 0)
	E_INFO("%d LM words not in dictionary; ignored\n", n);
    
    return map;
}


int32 wid_wordprob2alt (dict_t *dict, wordprob_t *wp, int32 n)
{
    int32 i, j;
    s3wid_t w;
    
    for (i = 0, j = n; i < n; i++) {
	w = wp[i].wid;
	
	for (w = dict_nextalt (dict, w); IS_S3WID(w); w = dict_nextalt (dict, w)) {
	    wp[j].wid = w;
	    wp[j].prob = wp[i].prob;
	    j++;
	}
    }
    
    return j;
}
