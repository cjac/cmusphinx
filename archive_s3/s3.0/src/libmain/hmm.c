/*
 * hmm.c -- HMM Viterbi search.
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
 * 24-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added hmm_vit_trans_comp().
 * 
 * 16-Jul-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>

#include "hmm.h"


void hmm_clear (mdef_t *m, hmm_t *h)
{
    int32 i;

    h->in.score = LOGPROB_ZERO;
    h->in.hist = NULL;
    for (i = 0; i < m->n_emit_state; i++) {
	h->state[i].score = LOGPROB_ZERO;
	h->state[i].hist = NULL;
    }
    h->out.score = LOGPROB_ZERO;
    h->out.hist = NULL;

    h->bestscore = LOGPROB_ZERO;
    
    h->active = -1;
}


void hmm_enter (hmm_t *h, int32 score, int32 data, vithist_t *hist, int16 f)
{
    h->in.score = score;
    h->in.data = data;
    h->in.hist = hist;
    h->active = f;
}


int32 hmm_vit_trans (hmm_t *src, hmm_t *dst, int32 frm)
{
    if (dst->in.score < src->out.score) {
	dst->in.score = src->out.score;
	dst->in.data = src->out.data;
	dst->in.hist = src->out.hist;

	if (dst->active < frm) {
	    dst->active = frm;
	    return 1;
	} else {
	    assert (dst->active == frm);
	    return 0;
	}
    } else
	return 0;
}


int32 hmm_vit_trans_comp (int32 score, vithist_t *hist, int32 data, hmm_t *dst, int32 frm)
{
    if (dst->in.score < score) {
	dst->in.score = score;
	dst->in.data = data;
	dst->in.hist = hist;

	if (dst->active < frm) {
	    dst->active = frm;
	    return 1;
	} else {
	    assert (dst->active == frm);
	    return 0;
	}
    } else
	return 0;
}


/*
 * State score updated by taking the best incoming score and then accumulating the
 * senone score for that state.  (I.e., the same as in the S3 trainer.)  The HMM topology
 * is assumed to be left-to-right, so that the states can be updated right-to-left.
 * NOTE: The exit state is updated separately.  In the Viterbi DP diagram, transitions to
 * the exit state occur from the current time (are vertical).  Hence they should be made
 * only after the history has been logged for the emitting states.
 */
int32 hmm_vit_eval (mdef_t *m, tmat_t *tmat, hmm_t *h, int32 *senscore)
{
    int32 d, s, src, score, hmmscore;
    hmm_state_t *st;
    s3senid_t *sen;
    int32 **tp;
    
    sen = m->phone[h->pid].state;
    tp = tmat->tp[m->phone[h->pid].tmat];
    
    /* Update states [m->n_emit_state-1 .. 1], right-to-left.  State 0 handled below */
    st = h->state;
    hmmscore = MAX_NEG_INT32;
    for (d = m->n_emit_state-1; d > 0; d--) {	/* For each destination state */
	score = LOGPROB_ZERO;
	src = -1;
	for (s = 0; s <= d; s++) {		/* From each source state */
	    if ((tp[s][d] > LOGPROB_ZERO) && ((st[s].score + tp[s][d]) >= score)) {
		score = st[s].score + tp[s][d];	/* P[i,t-1] * a[i,j] (i=s, j=d) */
		src = s;
	    }
	}
	
	if (src >= 0) {
	    st[d].score = score;
	    st[d].data = st[src].data;
	    st[d].hist = st[src].hist;
	} else
	    st[d].score = LOGPROB_ZERO;
	st[d].score += senscore[sen[d]];	/* Include b[j] */
	
	if (hmmscore < st[d].score)
	    hmmscore = st[d].score;
    }

    /* State 0, handled separately to include the initial or entry state */
    if ((tp[0][0] > LOGPROB_ZERO) && ((st[0].score + tp[0][0]) >= h->in.score)) {
	st[0].score += tp[0][0] + senscore[sen[0]];
    } else {
	st[0].score = h->in.score + senscore[sen[0]];
	st[0].data = h->in.data;
	st[0].hist = h->in.hist;
    }
    
    if (hmmscore < st[0].score)
	hmmscore = st[0].score;

    /* "Consume" the incoming state score, so it isn't used again in the next frame */
    h->in.score = LOGPROB_ZERO;
    
    h->bestscore = hmmscore;
    
    return hmmscore;
}


/*
 * Update the exit state score for the given HMM.
 */
void hmm_vit_eval_exit (mdef_t *m, tmat_t *tmat, hmm_t *h)
{
    int32 score, src, s, d;
    int32 **tp;
    hmm_state_t *st;
    
    tp = tmat->tp[m->phone[h->pid].tmat];
    
    st = h->state;
    src = -1;
    score = LOGPROB_ZERO;
    d = m->n_emit_state;
    for (s = 0; s < d; s++) {
	if ((tp[s][d] > LOGPROB_ZERO) && ((st[s].score + tp[s][d]) >= score)) {
	    score = st[s].score + tp[s][d];
	    src = s;
	}
    }
    if (src >= 0) {
	h->out.score = score;
	h->out.data = st[src].data;
	h->out.hist = st[src].hist;
    } else
	h->out.score = LOGPROB_ZERO;
}


void hmm_dump (FILE *fp, mdef_t *m, hmm_t *h)
{
    int32 i;
    
    fprintf (fp, "\t%5d\t", h->active);

    fprintf (fp, "% 12d", h->in.score);
    for (i = 0; i < m->n_emit_state; i++)
	fprintf (fp, " %12d", h->state[i].score);
    fprintf (fp, "% 12d", h->out.score);
    fprintf (fp, "\n");

    fprintf (fp, "\t\t");
    if (h->in.hist)
	fprintf (fp, " %12d", h->in.hist->id);
    else
	fprintf (fp, " %12d", -1);
    for (i = 0; i < m->n_emit_state; i++) {
	if (h->state[i].hist)
	    fprintf (fp, " %12d", h->state[i].hist->id);
	else
	    fprintf (fp, " %12d", -1);
    }
    if (h->out.hist)
	fprintf (fp, " %12d", h->out.hist->id);
    else
	fprintf (fp, " %12d", -1);
    fprintf (fp, "\n");
    
    fflush (fp);
}
