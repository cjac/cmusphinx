/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * $Log$
 * Revision 1.6  2006/02/22  16:46:38  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Added function hmm_vit_eval, a wrapper of computing the hmm level scores. 2, Fixed issues in , 3, Fixed issues of dox-doc
 * 
 * Revision 1.5.4.1  2005/09/25 18:53:36  arthchan2003
 * Added hmm_vit_eval, in lextree.c, hmm_dump and hmm_vit_eval is now separated.
 *
 * Revision 1.5  2005/06/21 18:34:41  arthchan2003
 * Log. 1, Fixed doxygen documentation for all functions. 2, Add $Log$
 * Revision 1.6  2006/02/22  16:46:38  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Added function hmm_vit_eval, a wrapper of computing the hmm level scores. 2, Fixed issues in , 3, Fixed issues of dox-doc
 * 
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Modified hmm_vit_eval_3st() to include explicit checks for
 *		tr[0][2] and tr[1][3]. Included compiler directive activated
 *		checks for int32 underflow
 *
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified hmm_t.state to be a run-time array instead of a compile-time
 * 		one.  Modified compile-time 3 and 5-state versions of hmm_vit_eval
 * 		into hmm_vit_eval_3st and hmm_vit_eval_5st, to allow run-time selection.
 * 		Removed hmm_init().
 * 
 * 11-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Bugfix in computing HMM exit state score.
 * 
 * 08-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added HMM_SKIPARCS compile-time option and hmm_init().
 * 
 * 20-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Bugfix in hmm_eval: If state1->state2 transition took place,
 * 		state1 history didn't get propagated to state2.
 * 		Also, included tp[][] in HMM evaluation.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started, based on an earlier version.
 */

#include <assert.h>
#include <stdlib.h>

#include "hmm.h"
#include "ckd_alloc.h"

#define HMM_BLOCK_SIZE 1000

static int32 NO_UFLOW_ADD(int32 a, int32 b);

hmm_context_t *
hmm_context_init(int32 n_emit_state, int32 mpx,
		 int32 ***tp,
		 int32 *senscore,
		 s3senid_t **sseq)
{
    hmm_context_t *ctx;

    assert(n_emit_state > 0);
    assert(tp != NULL);
    /* Multiplex HMMs all have their own senone sequences */
    assert(mpx || (sseq != NULL));

    ctx = ckd_calloc(1, sizeof(*ctx));
    ctx->n_emit_state = n_emit_state;
    ctx->mpx = mpx;
    ctx->tp = (const int32 ***)tp;
    ctx->senscore = senscore;
    ctx->sseq = (const s3senid_t **)sseq;

    return ctx;
}

void
hmm_context_free(hmm_context_t *ctx)
{
    ckd_free(ctx);
}

void
hmm_init(hmm_context_t *ctx, hmm_t *hmm)
{
    hmm->state = ckd_calloc(hmm_n_emit_state(ctx), sizeof(hmm_state_t));
    if (ctx->mpx)
        hmm->s.mpx_ssid = ckd_calloc(hmm_n_emit_state(ctx), sizeof(int32));
    hmm_clear(ctx, hmm);
}

void
hmm_deinit(hmm_context_t *ctx, hmm_t *hmm)
{
    ckd_free(hmm->state);
    if (ctx->mpx)
        ckd_free(hmm->s.mpx_ssid);
}

void
hmm_dump(const hmm_context_t *ctx,
	 hmm_t * hmm,
         FILE * fp)
{
    int32 i;

    fprintf(fp, " %11d    ", hmm_in_score(ctx, hmm));
    for (i = 1; i < hmm_n_state(ctx); i++)
        fprintf(fp, " %11d", hmm_score(ctx, hmm, i));
    fprintf(fp, "\n");

    fprintf(fp, " %11d    ", hmm_history(ctx, hmm, 0));
    for (i = 1; i < hmm_n_state(ctx); i++)
        fprintf(fp, " %11d", hmm_history(ctx, hmm, i));
    fprintf(fp, "\n");

    fprintf(fp, " %-11s    ", "senid");
    for (i = 0; i < hmm_n_emit_state(ctx); i++)
        fprintf(fp, " %11d", hmm_senid(ctx, hmm, i));
    fprintf(fp, "\n");

    if (ctx->senscore) {
        fprintf(fp, " %-11s    ", "senscr");
        for (i = 0; i < hmm_n_emit_state(ctx); i++)
            fprintf(fp, " %11d", hmm_senscr(ctx, hmm, i));
        fprintf(fp, "\n");
    }


    if (hmm_in_score(ctx, hmm) > 0)
        fprintf(fp,
                "ALERT!! The input score %d is large than 0. Probably wrap around.\n",
                hmm_in_score(ctx, hmm));
    if (hmm_out_score(ctx, hmm) > 0)
        fprintf(fp,
                "ALERT!! The output score %d is large than 0. Probably wrap around\n.",
                hmm_out_score(ctx, hmm));

    fflush(fp);
}


void
hmm_clear(const hmm_context_t *ctx, hmm_t * h)
{
    int32 i;

    for (i = 0; i < hmm_n_emit_state(ctx); i++) {
        hmm_score(ctx, h, i) = S3_LOGPROB_ZERO;
        hmm_history(ctx, h, i) = -1;
    }
    hmm_in_score(ctx, h) = S3_LOGPROB_ZERO;
    hmm_in_history(ctx, h) = -1;
    hmm_out_score(ctx, h) = S3_LOGPROB_ZERO;
    hmm_out_history(ctx, h) = -1;

    h->bestscore = S3_LOGPROB_ZERO;
}

void
hmm_enter(const hmm_context_t *ctx, hmm_t *h, int32 score, int32 histid)
{
    hmm_in_score(ctx, h) = score;
    hmm_in_history(ctx, h) = histid;
}

/* Added by ARCHAN at 20040127 , always check for underflow, it's proved to be crucial. */
#define _CHECKUNDERFLOW_ 1

/* The compiler will inline this for us, probably. */
static int32
NO_UFLOW_ADD(int32 a, int32 b)
{
    int c;
#ifdef _CHECKUNDERFLOW_
    c = a + b;
    c = (c > 0 && a < 0 && b < 0) ? MAX_NEG_INT32 : c;
#else
    c = a + b;
#endif
    return c;

}

#define hmm_tprob_5st(tp, i, j) (tp[(i)*6+(j)])
#define nonmpx_senscr(ctx, sseq, i) ((ctx)->senscore[(sseq)[i]])

static int32
hmm_vit_eval_5st(const hmm_context_t *ctx, hmm_t * hmm)
{
    int32 s0, s1, s2, s3, s4, best;
    const int32 *tp;
    const s3senid_t *sseq;

    tp = ctx->tp[hmm->tmatid][0];
    sseq = ctx->sseq[hmm_ssid(ctx, hmm, 0)];

    /* 4 = max(2,3,4); */
    s4 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 4), hmm_tprob_5st(tp, 4, 4));
    s3 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 3), hmm_tprob_5st(tp, 3, 4));
    s2 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 2), hmm_tprob_5st(tp, 2, 4));

    if (s4 < s3) {
        if (s3 >= s2) {
            s4 = s3;
            hmm_history(ctx, hmm, 4) = hmm_history(ctx, hmm, 3);
        }
        else {
            s4 = s2;
            hmm_history(ctx, hmm, 4) = hmm_history(ctx, hmm, 2);
        }
    }
    else if (s4 < s2) {
        s4 = s2;
        hmm_history(ctx, hmm, 4) = hmm_history(ctx, hmm, 2);
    }
    s4 = NO_UFLOW_ADD(s4, nonmpx_senscr(ctx, sseq, 4));
    hmm_score(ctx, hmm, 4) = s4;

    /* 3 = max(1,2,3); */
    s3 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 3), hmm_tprob_5st(tp, 3, 3));
    s2 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 2), hmm_tprob_5st(tp, 2, 3));
    s1 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 1), hmm_tprob_5st(tp, 1, 3));
    if (s3 < s2) {
        if (s2 >= s1) {
            s3 = s2;
            hmm_history(ctx, hmm, 3) = hmm_history(ctx, hmm, 2);
        }
        else {
            s3 = s1;
            hmm_history(ctx, hmm, 3) = hmm_history(ctx, hmm, 1);
        }
    }
    else if (s3 < s1) {
        s3 = s1;
        hmm_history(ctx, hmm, 3) = hmm_history(ctx, hmm, 1);
    }

    s3 = NO_UFLOW_ADD(s3, nonmpx_senscr(ctx, sseq, 3));
    hmm_score(ctx, hmm, 3) = s3;

    best = (s4 > s3) ? s4 : s3;

    /* Exit state score */
    s4 = NO_UFLOW_ADD(s4, hmm_tprob_5st(tp, 4, 5));
    s3 = NO_UFLOW_ADD(s3, hmm_tprob_5st(tp, 3, 5));

    if (s4 < s3) {
        hmm_out_score(ctx, hmm) = s3;
        hmm_out_history(ctx, hmm) = hmm_history(ctx, hmm, 3);
    }
    else {
        hmm_out_score(ctx, hmm) = s4;
        hmm_out_history(ctx, hmm) = hmm_history(ctx, hmm, 4);
    }

    /* 2 = max(0,1,2); */
    s2 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 2), hmm_tprob_5st(tp, 2, 2));
    s1 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 1), hmm_tprob_5st(tp, 1, 2));
    s0 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 0), hmm_tprob_5st(tp, 0, 2));
    if (s2 < s1) {
        if (s1 >= s0) {
            s2 = s1;
            hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 1);
        }
        else {
            s2 = s0;
            hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 0);
        }
    }
    else if (s2 < s0) {
        s2 = s0;
        hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 0);
    }

    s2 = NO_UFLOW_ADD(s2, nonmpx_senscr(ctx, sseq, 2));

    hmm_score(ctx, hmm, 2) = s2;
    if (best < s2)
        best = s2;

    /* 1 = max(0,1); */
    s1 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 1), hmm_tprob_5st(tp, 1, 1));
    s0 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 0), hmm_tprob_5st(tp, 0, 1));

    if (s1 < s0) {
        s1 = s0;
        hmm_history(ctx, hmm, 1) = hmm_history(ctx, hmm, 0);
    }
    s1 = NO_UFLOW_ADD(s1, nonmpx_senscr(ctx, sseq, 1));
    hmm_score(ctx, hmm, 1) = s1;
    if (best < s1)
        best = s1;

    /* 0 = max(0,in); */
    s0 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 0), hmm_tprob_5st(tp, 0, 0));

    if (s0 < hmm_in_score(ctx, hmm)) {
        s0 = hmm_in_score(ctx, hmm);
        hmm_history(ctx, hmm, 0) = hmm_in_history(ctx, hmm);
    }
    s0 = NO_UFLOW_ADD(s0, nonmpx_senscr(ctx, sseq, 0));

    hmm_score(ctx, hmm, 0) = s0;
    if (best < s0)
        best = s0;

    hmm_in_score(ctx, hmm) = S3_LOGPROB_ZERO;    /* Consumed */
    hmm_bestscore(ctx, hmm) = best;

    return best;
}

#define hmm_tprob_3st(tp, i, j) (tp[(i)*4+(j)])

static int32
hmm_vit_eval_3st(const hmm_context_t *ctx, hmm_t * hmm)
{
    int32 s0, s1, s2, best;
    const int32 *tp;
    const s3senid_t *sseq;

    tp = ctx->tp[hmm->tmatid][0];
    sseq = ctx->sseq[hmm_ssid(ctx, hmm, 0)];

    /* 2 = max(0,1,2); */
    s2 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 2), hmm_tprob_3st(tp, 2, 2));
    s1 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 1), hmm_tprob_3st(tp, 1, 2));

    if (hmm_tprob_3st(tp, 0, 2) > S3_LOGPROB_ZERO) {      /* Only if skip(0->2) is allowed */
        s0 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 0), hmm_tprob_3st(tp, 0, 2));

        if (s2 < s1) {
            if (s1 >= s0) {
                s2 = s1;
                hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 1);
            }
            else {
                s2 = s0;
                hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 0);
            }
        }
        else if (s2 < s0) {
            s2 = s0;
            hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 0);
        }
    }
    else {
        if (s2 < s1) {
            s2 = s1;
            hmm_history(ctx, hmm, 2) = hmm_history(ctx, hmm, 1);
        }
    }

    s2 = NO_UFLOW_ADD(s2, nonmpx_senscr(ctx, sseq, 2));
    hmm_score(ctx, hmm, 2) = s2;

    /* 1 = max(0,1); */
    s1 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 1), hmm_tprob_3st(tp, 1, 1));
    s0 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 0), hmm_tprob_3st(tp, 0, 1));

    if (s1 < s0) {
        s1 = s0;
        hmm_history(ctx, hmm, 1) = hmm_history(ctx, hmm, 0);
    }

    s1 = NO_UFLOW_ADD(s1, nonmpx_senscr(ctx, sseq, 1));
    hmm_score(ctx, hmm, 1) = s1;

    best = (s2 > s1) ? s2 : s1;

    /* Exit state score */
    s2 = NO_UFLOW_ADD(s2, hmm_tprob_3st(tp, 2, 3));

    if (hmm_tprob_3st(tp, 1, 3) > S3_LOGPROB_ZERO) {      /* Only if skip(1->3) is allowed */
        s1 = NO_UFLOW_ADD(s1, hmm_tprob_3st(tp, 1, 3));

        if (s2 < s1) {
            hmm_out_score(ctx, hmm) = s1;
            hmm_out_history(ctx, hmm) = hmm_history(ctx, hmm, 1);
        }
        else {
            hmm_out_score(ctx, hmm) = s2;
            hmm_out_history(ctx, hmm) = hmm_history(ctx, hmm, 2);
        }
    }
    else {
        hmm_out_score(ctx, hmm) = s2;
        hmm_out_history(ctx, hmm) = hmm_history(ctx, hmm, 2);
    }
    /* 0 = max(0,in); */
    s0 = NO_UFLOW_ADD(hmm_score(ctx, hmm, 0), hmm_tprob_3st(tp, 0, 0));

    if (s0 < hmm_in_score(ctx, hmm)) {
        s0 = hmm_in_score(ctx, hmm);
        hmm_history(ctx, hmm, 0) = hmm_in_history(ctx, hmm);
    }

    s0 = NO_UFLOW_ADD(s0, nonmpx_senscr(ctx, sseq, 0));
    hmm_score(ctx, hmm, 0) = s0;

    if (best < s0)
        best = s0;

    hmm_in_score(ctx, hmm) = S3_LOGPROB_ZERO;    /* Consumed */
    hmm_bestscore(ctx, hmm) = best;

    return best;
}

static int32
hmm_vit_eval_anytopo(const hmm_context_t *ctx, hmm_t * h)
{
    int32 to, from, bestfrom;
    int32 newscr, scr, bestscr;
    int final_state;

    /* Compute previous state-score + observation output prob for each emitting state */
    for (from = 0; from < hmm_n_emit_state(ctx); ++from) {
        if ((ctx->st_sen_scr[from] =
             hmm_score(ctx, h, from) + hmm_senscr(ctx, h, from)) < S3_LOGPROB_ZERO)
            ctx->st_sen_scr[from] = S3_LOGPROB_ZERO;
    }

    /* FIXME/TODO: Use the BLAS for all this. */
    /* Evaluate final-state first, which does not have a self-transition */
    final_state = hmm_n_emit_state(ctx);
    to = final_state;
    scr = S3_LOGPROB_ZERO;
    bestfrom = -1;
    for (from = to - 1; from >= 0; --from) {
        if ((hmm_tprob(ctx, h, from, to) > S3_LOGPROB_ZERO) &&
            ((newscr = ctx->st_sen_scr[from]
              + hmm_tprob(ctx, h, from, to)) > scr)) {
            scr = newscr;
            bestfrom = from;
        }
    }
    hmm_out_score(ctx, h) = scr;
    if (bestfrom >= 0)
        hmm_out_history(ctx, h) = hmm_history(ctx, h, bestfrom);
    bestscr = scr;

    /* Evaluate all other states, which might have self-transitions */
    for (to = final_state - 1; to >= 0; --to) {
        /* Score from self-transition, if any */
        scr =
            (hmm_tprob(ctx, h, to, to) > S3_LOGPROB_ZERO)
            ? ctx->st_sen_scr[to] + hmm_tprob(ctx, h, to, to)
            : S3_LOGPROB_ZERO;

        /* Scores from transitions from other states */
        bestfrom = -1;
        for (from = to - 1; from >= 0; --from) {
            if ((hmm_tprob(ctx, h, from, to) > S3_LOGPROB_ZERO) &&
                ((newscr = ctx->st_sen_scr[from]
                  + hmm_tprob(ctx, h, from, to)) > scr)) {
                scr = newscr;
                bestfrom = from;
            }
        }

        /* Update new result for state to */
        hmm_score(ctx, h, to) = scr;
        if (bestfrom >= 0) {
            hmm_history(ctx, h, to) = hmm_history(ctx, h, bestfrom);
            if (ctx->mpx)
                h->s.mpx_ssid[to] = h->s.mpx_ssid[bestfrom];
        }

        if (bestscr < scr)
            bestscr = scr;
    }

    h->bestscore = bestscr;
    return bestscr;
}

int32
hmm_vit_eval(const hmm_context_t *ctx, hmm_t * hmm)
{
    int32 bs = 0;
    if (ctx->mpx) {
        bs = hmm_vit_eval_anytopo(ctx, hmm);
    }
    else {
        if (hmm_n_emit_state(ctx) == 5)
            bs = hmm_vit_eval_5st(ctx, hmm);
        else if (hmm_n_emit_state(ctx) == 3)
            bs = hmm_vit_eval_3st(ctx, hmm);
        else
            bs = hmm_vit_eval_anytopo(ctx, hmm);
    }
    return bs;
}

int32
hmm_dump_vit_eval(const hmm_context_t *ctx, 
                  hmm_t * hmm, FILE * fp)
{
    int32 bs = 0;

    if (fp)
        hmm_dump(ctx, hmm, fp);
    bs = hmm_vit_eval(ctx, hmm);

    return bs;
}
