/*
 * am.c -- Acoustic model evaluation
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
 * 22-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include "am.h"


acoustic_t *acoustic_init (feat_t *f, gauden_t *g, senone_t *s,
			   float64 beam,
			   int32 maxfr)
{
    acoustic_t *am;
    int32 i;
    
    if (senone_n_mgau(s) != gauden_n_mgau(g)) {
	E_ERROR("#Parent mixture Gaussians mismatch: senone(%d), gauden(%d)\n",
		senone_n_mgau(s), gauden_n_mgau(g));
    }
    
    if (feat_n_stream(f) != senone_n_stream(s)) {
	E_ERROR("#Feature-streams mismatch: feat(%d), senone(%d)\n",
		feat_n_stream(f), senone_n_stream(s));
    }
    
    if (feat_n_stream(f) != gauden_n_stream(g)) {
	E_ERROR("#Feature-streams mismatch: feat(%d), gauden(%d)\n",
		feat_n_stream(f), gauden_n_stream(g));
	return NULL;
    }
    
    for (i = 0; i < feat_n_stream(f); i++) {
	if (feat_stream_len(f, i) != gauden_stream_len(g, i)) {
	    E_ERROR("Feature stream(%d) length mismatch: feat(%d), gauden(%d)\n",
		    feat_stream_len(f, i), gauden_stream_len(g, i));
	    return NULL;
	}
    }
    
    if (beam > 1.0) {
	E_ERROR("mgaubeam > 1.0 (%e)\n", beam);
	return NULL;
    }
    
    am = (acoustic_t *) ckd_calloc (1, sizeof(acoustic_t));

    am->fcb = f;
    am->gau = g;
    am->sen = s;
    
    am->mgaubeam = (beam == 0.0) ? LOGPROB_ZERO : logs3(beam);
    if (am->mgaubeam > 0)
	am->mgaubeam = 0;
    am->tot_mgau_eval = 0;
    am->tot_dist_valid = 0.0;
    
    am->dist_valid = (am->mgaubeam <= LOGPROB_ZERO) ? NULL :
	(int32 *) ckd_calloc (g->max_n_mean, sizeof(int32));
    
    if (f->compute_feat) {
	/* Input is MFC cepstra; feature vectors computed from that */
	am->mfc = (float32 **) ckd_calloc_2d (maxfr, feat_cepsize(am->fcb),
					      sizeof(float32));
	am->feat = feat_array_alloc (f, 1);
    } else {
	/* Input is directly feature vectors */
	am->mfc = NULL;
	am->feat = feat_array_alloc (f, maxfr);
    }
    
    am->dist = (int32 *) ckd_calloc (g->max_n_mean, sizeof(int32));
    am->gauden_active = bitvec_alloc (g->n_mgau);

    am->senscr = (int32 *) ckd_calloc (s->n_sen, sizeof(int32));
    am->senscale = (int32 *) ckd_calloc (maxfr, sizeof(int32));
    am->sen_active = bitvec_alloc (s->n_sen);
    
    return am;
}


static int32 senactive_to_mgauactive (acoustic_t *am)
{
    int32 n, s;
    gauden_t *gau;
    senone_t *sen;
    
    sen = am->sen;
    gau = am->gau;
    
    bitvec_clear_all(am->gauden_active, gau->n_mgau);
    n = 0;
    for (s = 0; s < sen->n_sen; s++) {
	if (bitvec_is_set (am->sen_active, s)) {
	    bitvec_set (am->gauden_active, sen->sen2mgau[s]);
	    n++;
	}
    }
    
    return n;
}


int32 acoustic_eval (acoustic_t *am, int32 frm)
{
    senone_t *sen;
    gauden_t *gau;
    int32 m, f, s, best, bestgau;
    int32 i, j, k;
    float32 **fv;
    float32 **mfc;
    
    sen = am->sen;
    gau = am->gau;

    if (am->mfc) {
	mfc = am->mfc + frm;
	am->fcb->compute_feat(am->fcb, mfc, am->feat[0]);
	fv = am->feat[0];
    } else {
	fv = am->feat[frm];
    }
    
    /* Identify the active mixture Gaussians */
    if (senactive_to_mgauactive(am) == 0)
	E_FATAL("No states active\n");
    
    /* Since we're going to accumulate into senscr for each feature stream */
    memset (am->senscr, 0, sen->n_sen * sizeof(int32));
    
    /* Evaluate all senones; FOR NOW NOT YET OPTIMIZED TO JUST THE ACTIVE ONES */
    best = MAX_NEG_INT32;
    for (m = 0; m < gau->n_mgau; m++) {
	if (bitvec_is_set (am->gauden_active, m)) {
	    for (f = 0; f < gau->n_feat; f++) {
		k = gauden_dist (gau, m, f, fv[f], am->dist);
		
		if (am->dist_valid) {
		    /* Determine set of active mgau components based on pruning beam */
		    bestgau = MAX_NEG_INT32;
		    for (i = 0; i < k; i++)
			if (am->dist[i] > bestgau)
			    bestgau = am->dist[i];
		    j = 0;
		    for (i = 0; i < k; i++) {
			if (am->dist[i] >= bestgau + am->mgaubeam) {
			    am->dist_valid[j++] = i;
			}
		    }
		    am->n_dist_valid = j;
		    k = j;
		    
		    am->tot_dist_valid += j;
		    am->tot_mgau_eval += 1;
		}
#if 1
		for (i = 0; i < sen->mgau2sen[m].n_sen; i++) {
		    s = sen->mgau2sen[m].sen[i];
		    if (bitvec_is_set (am->sen_active, s))
			am->senscr[s] += senone_eval (sen, s, f, am->dist,
						      am->dist_valid, k);
		}
#else
		senone_eval_all (sen, m, f, am->dist, am->dist_valid, k, am->senscr);
#endif
	    }
	    
	    /* Find the best senone score so far */
	    for (i = 0; i < sen->mgau2sen[m].n_sen; i++) {
		s = sen->mgau2sen[m].sen[i];
		if (bitvec_is_set (am->sen_active, s) && (am->senscr[s] > best))
		    best = am->senscr[s];
	    }
	}
    }

    /* CD-CI likelihood-interpolation (later) */
    
    /* Normalize senone score by subtracting the best */
    for (s = 0; s < sen->n_sen; s++)
	if (bitvec_is_set (am->sen_active, s))
	    am->senscr[s] -= best;
    
    return best;
}
