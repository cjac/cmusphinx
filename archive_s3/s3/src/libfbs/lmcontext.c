/*
 * lmcontext.c -- Surrounding LM context for each utterance.
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


/*
 * This module deals with files that specify the utterance boundary context for
 * each utterance.  An utterance need not begin with <s> and end with </s>.
 * Instead, one can use the context specified in a given file.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if (_SUN4)
#include <unistd.h>
#endif
#include <math.h>

#include <libutil/libutil.h>
#include <libmisc/libmisc.h>

#include "lmcontext.h"
#include "dict.h"
#include "lm.h"


void lmcontext_load (corpus_t *corp, char *uttid, s3wid_t *pred, s3wid_t *succ)
{
    char *str, wd[4096], *strp;
    s3wid_t w[3];
    int32 i, n;
    dict_t *dict;
    s3lmwid_t lwid;
    
    if ((str = corpus_lookup (corp, uttid)) == NULL)
	E_FATAL("Couldn't find LM context for %s\n", uttid);
    dict = dict_getdict ();
    
    strp = str;
    for (i = 0; i < 4; i++) {
	if (sscanf (strp, "%s%n", wd, &n) != 1) {
	    if (i < 3)
		E_FATAL("Bad LM context spec for %s: %s\n", uttid, str);
	    else
		break;
	}
	strp += n;
	
	if (strcmp (wd, "-") == 0)
	    w[i] = BAD_WID;
	else {
	    w[i] = dict_wordid (wd);
	    if (NOT_WID(w[i]))
		E_FATAL("LM context word (%s) for %s not in dictionary\n", wd, uttid);
	    w[i] = dict_basewid(w[i]);
	    
	    switch (i) {
	    case 0: 
		if ((n = dict->word[w[0]].n_comp) > 0)
		    w[0] = dict->word[w[0]].comp[n-1].wid;
		break;
		
	    case 1:
		if ((n = dict->word[w[1]].n_comp) > 0) {
		    w[0] = dict->word[w[1]].comp[n-2].wid;
		    w[1] = dict->word[w[1]].comp[n-1].wid;
		}
		break;
		
	    case 2:
		if (w[2] != dict_wordid(FINISH_WORD))
		    E_FATAL("Illegal successor LM context for %s: %s\n", uttid, str);
		break;
		
	    default:
		assert (0);	/* Should never get here */
		break;
	    }
	}
    }
    
    if (IS_WID(w[0]) && NOT_WID(w[1]))
	E_FATAL("Bad LM context spec for %s: %s\n", uttid, str);

    for (i = 0; i < 3; i++) {
	if (IS_WID(w[i])) {
	    lwid = lm_lmwid (w[i]);
	    if (NOT_LMWID(lwid))
		E_FATAL("LM context word (%s) for %s not in LM\n", wd, uttid);
	}
    }
    
    pred[0] = w[0];
    pred[1] = w[1];
    *succ = w[2];
}
