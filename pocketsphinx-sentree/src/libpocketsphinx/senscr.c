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
 * senscr.c -- 	Senone score computation module.
 * 		Hides details of s2 (semi-continuous) and s3 (continuous)
 * 		models, and computes generic "senone scores".
 *
 * HISTORY
 * 
 * 02-Dec-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added acoustic score weight (applied only to S3 continuous
 * 		acoustic models).
 * 
 * 01-Dec-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added senone active flag related functions.
 * 
 * 20-Nov-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */

/* System headers. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

/* SphinxBase headers. */
#include <sphinx_types.h>
#include <err.h>
#include <ckd_alloc.h>

/* Local headers. */
#include "kb.h"
#include "s2_semi_mgau.h"
#include "ms_mgau.h"
#include "phone.h"
#include "search.h"
#include "search_const.h"
#include "senscr.h"

/* Global variables shared by search and GMM computations. */
int16 *senone_scores;
int32 *senone_active;
int32 n_senone_active;
bitvec_t *senone_active_vec;

int32
senscr_all(mfcc_t **feat, int32 frame_idx)
{
    int32 best, bestidx;

    if (g_ms_mgau)
        best = ms_cont_mgau_frame_eval(g_ms_mgau, senone_scores,
                                       senone_active, n_senone_active,
                                       feat, frame_idx, TRUE, &bestidx);
    else
        best = s2_semi_mgau_frame_eval(g_semi_mgau, senone_scores,
                                       senone_active, n_senone_active,
                                       feat, frame_idx, TRUE, &bestidx);
    return best;
}


int32
senscr_active(mfcc_t **feat, int32 frame_idx)
{
    int32 best, bestidx;

    if (g_ms_mgau) {
        sen_active_flags2list();
        best = ms_cont_mgau_frame_eval(g_ms_mgau, senone_scores,
                                       senone_active, n_senone_active,
                                       feat, frame_idx, FALSE, &bestidx);
    }
    else {
        best = s2_semi_mgau_frame_eval(g_semi_mgau, senone_scores,
                                       senone_active, n_senone_active,
                                       feat, frame_idx, FALSE, &bestidx);
    }
    return best;
}

void
sen_active_clear(void)
{
    bitvec_clear_all(senone_active_vec, bin_mdef_n_sen(g_mdef));
    n_senone_active = 0;
}

#define MPX_BITVEC_SET(h,i)                                                     \
            if ((h)->s.mpx_ssid[i] != -1)                                       \
                bitvec_set(senone_active_vec,                                   \
                           bin_mdef_sseq2sen(g_mdef, (h)->s.mpx_ssid[i], (i)));
#define NONMPX_BITVEC_SET(h,i)                                          \
                bitvec_set(senone_active_vec,                           \
                           bin_mdef_sseq2sen(g_mdef, (h)->s.ssid, (i)));

void
hmm_sen_active(hmm_t * hmm)
{
    int i;

    if (hmm_is_mpx(hmm)) {
        switch (hmm_n_emit_state(hmm)) {
        case 5:
            MPX_BITVEC_SET(hmm, 4);
            MPX_BITVEC_SET(hmm, 3);
        case 3:
            MPX_BITVEC_SET(hmm, 2);
            MPX_BITVEC_SET(hmm, 1);
            MPX_BITVEC_SET(hmm, 0);
            break;
        default:
            for (i = 0; i < hmm_n_emit_state(hmm); ++i) {
                MPX_BITVEC_SET(hmm, i);
            }
        }
    }
    else {
        switch (hmm_n_emit_state(hmm)) {
        case 5:
            NONMPX_BITVEC_SET(hmm, 4);
            NONMPX_BITVEC_SET(hmm, 3);
        case 3:
            NONMPX_BITVEC_SET(hmm, 2);
            NONMPX_BITVEC_SET(hmm, 1);
            NONMPX_BITVEC_SET(hmm, 0);
            break;
        default:
            for (i = 0; i < hmm_n_emit_state(hmm); ++i) {
                NONMPX_BITVEC_SET(hmm, i);
            }
        }
    }
}

int32
sen_active_flags2list(void)
{
    int32 w, n, b, total_dists, total_words, extra_bits;
    bitvec_t *flagptr;

    total_dists = bin_mdef_n_sen(g_mdef);
    total_words = total_dists / BITVEC_BITS;
    extra_bits = total_dists % BITVEC_BITS;
    w = n = 0;
    for (flagptr = senone_active_vec; w < total_words; ++w, ++flagptr) {
        if (*flagptr == 0)
            continue;
        for (b = 0; b < BITVEC_BITS; ++b)
            if (*flagptr & (1UL << b))
                senone_active[n++] = w * BITVEC_BITS + b;
    }

    for (b = 0; b < extra_bits; ++b)
        if (*flagptr & (1UL << b))
            senone_active[n++] = w * BITVEC_BITS + b;

    n_senone_active = n;
    return n;
}
