/*
 * kb.h -- Knowledge bases, search parameters, and auxiliary structures for decoding
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
 * 07-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added kb_t.ci_active.
 * 
 * 02-Jun-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_KB_H_
#define _S3_KB_H_


#include <libutil/libutil.h>
#include "kbcore.h"
#include "lextree.h"
#include "vithist.h"
#include "ascr.h"
#include "beam.h"


/*
 * There can be several unigram lextrees.  If we're at the end of frame f, we can only
 * transition into the roots of lextree[(f+1) % n_lextree]; same for fillertree[].  This
 * alleviates the problem of excessive Viterbi pruning in lextrees.
 */

typedef struct {
    kbcore_t *kbcore;		/* Core model structures */
    
    int32 n_lextree;		/* See above comment about n_lextree */
    lextree_t **ugtree;
    lextree_t **fillertree;
    int32 n_lextrans;		/* #Transitions to lextree (root) made so far */
    
    vithist_t *vithist;		/* Viterbi history, built during search */
    
    float32 ***feat;		/* Feature frames */
    int32 nfr;			/* #Frames in feat in current utterance */
    
    int32 *ssid_active;		/* For determining the active senones in any frame */
    int32 *comssid_active;
    int32 *sen_active;
    
    int32 bestscore;		/* Best HMM state score in current frame */
    int32 bestwordscore;	/* Best wordexit HMM state score in current frame */
    
    ascr_t *ascr;		/* Senone and composite senone scores for one frame */
    beam_t *beam;		/* Beamwidth parameters */
    
    char *uttid;
    
    int32 utt_hmm_eval;
    int32 utt_sen_eval;
    int32 utt_gau_eval;
    int32 *hmm_hist;		/* Histogram: #frames in which a given no. of HMMs are active */
    int32 hmm_hist_bins;	/* #Bins in above histogram */
    int32 hmm_hist_binsize;	/* Binsize in above histogram (#HMMs/bin) */
    
    ptmr_t tm_sen;
    ptmr_t tm_srch;
    int32 tot_fr;
    float64 tot_sen_eval;	/* Senones evaluated over the entire session */
    float64 tot_gau_eval;	/* Gaussian densities evaluated over the entire session */
    float64 tot_hmm_eval;	/* HMMs evaluated over the entire session */
    float64 tot_wd_exit;	/* Words hypothesized over the entire session */
    
    FILE *matchsegfp;
} kb_t;

void kb_init (kb_t *kb);
void kb_lextree_active_swap (kb_t *kb);
void kb_free (kb_t *kb);	/* RAH 4.16.01 */

#endif
