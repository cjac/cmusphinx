/*
 * kb.h -- Collection of databases/data-structures needed for Viterbi search.
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
 * 26-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _KB_H_
#define _KB_H_


#include <libmain/kbcore.h>
#include <libmain/am.h>


typedef struct {
    mdef_t *mdef;		/* Model definition; phones/triphones -> senone/tmat maps */
    dict_t *dict;		/* Pronunciation lexicon */
    lm_t *lm;			/* Backoff word-trigram LM */
    fillpen_t *fillpen;		/* Filler word probabilities */
    s3lmwid_t *dict2lmwid;	/* dict2lmwid[wid] = LM word-id mapping corresponding to
				   dictionary word-id wid; could be BAD_LMWID if word not in LM
				   or is a filler */
    acoustic_t *am;		/* Acoustic models; gaussian density codebooks/senones */
    tmat_t *tmat;		/* HMM transition matrices */
    glist_t lextree_root;	/* List of lextree root node ptrs */
    glist_t lextree_active;	/* List of lextree node ptrs active in current frame */
    glist_t *vithist;		/* vithist[f] = list of Viterbi history node ptrs created @fr f.
				   NOTE: vithist[-1] must be valid for an initial <s> entry!! */
    int32 beam;			/* Main pruning threshold for beam search */
    int32 wordbeam;		/* Auxiliary pruning threshold for determining word exits; much
				   narrower than beam. */
    int32 wordmax;		/* wordbeam might be too permissive at times (e.g., around noisy
				   speech); this provides a hard absolute limit if needed */
    int32 *wd_last_sf;		/* wd_last_sf[w] = The last start frame for word-ID w.  Used to
				   determine if a word is persistent enough. */
    /* Profiling stuff */
    ptmr_t *tm;
    ptmr_t *tm_search;
    int32 n_hmm_eval;
    int32 n_sen_eval;
} kb_t;


#endif
