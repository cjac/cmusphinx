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

/* System headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#if defined(__ADSPBLACKFIN__)
#elif !defined(_WIN32_WCE)
#include <sys/types.h>
#endif

#ifndef M_PI 
#define M_PI 3.14159265358979323846 
#endif

/* SphinxBase headers */
#include <sphinx_config.h>
#include <cmd_ln.h>
#include <fixpoint.h>
#include <ckd_alloc.h>
#include <bio.h>
#include <err.h>
#include <prim_type.h>

/* Local headers */
#include "s2_semi_mgau.h"
#include "kdtree.h"
#include "posixwin32.h"

#define MGAU_MIXW_VERSION	"1.0"   /* Sphinx-3 file format version for mixw */
#define MGAU_PARAM_VERSION	"1.0"   /* Sphinx-3 file format version for mean/var */
#define NONE		-1
#define WORST_DIST	(int32)(0x80000000)

struct vqFeature_s {
    int32 score; /* score or distance */
    int32 codeword; /* codeword (vector index) */
};

/** Subtract GMM component b (assumed to be positive) and saturate */
#ifdef FIXED_POINT
#define GMMSUB(a,b) \
	(((a)-(b) > a) ? (INT_MIN) : ((a)-(b)))
/** Add GMM component b (assumed to be positive) and saturate */
#define GMMADD(a,b) \
	(((a)+(b) < a) ? (INT_MAX) : ((a)+(b)))
#else
#define GMMSUB(a,b) ((a)-(b))
#define GMMADD(a,b) ((a)+(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 199901L)
#define LOGMATH_INLINE inline
#elif defined(__GNUC__)
#define LOGMATH_INLINE static inline
#elif defined(_MSC_VER)
#define LOGMATH_INLINE __inline
#else
#define LOGMATH_INLINE static
#endif

/* Allocate 0..159 for negated quantized mixture weights and 0..96 for
 * negated normalized acoustic scores, so that the combination of the
 * two (for a single mixture) can never exceed 255. */
#define MAX_NEG_MIXW 159 /**< Maximum negated mixture weight value. */
#define MAX_NEG_ASCR 96  /**< Maximum negated acoustic score value. */

/**
 * Quickly log-add two negated log probabilities.
 *
 * @param lmath The log-math object
 * @param mlx A negative log probability (0 < mlx < 255)
 * @param mly A negative log probability (0 < mly < 255)
 * @return -log(exp(-mlx)+exp(-mly))
 *
 * We can do some extra-fast log addition since we know that
 * mixw+ascr is always less than 256 and hence x-y is also always less
 * than 256.  This relies on some cooperation from logmath_t which
 * will never produce a logmath table smaller than 256 entries.
 *
 * Note that the parameters are *negated* log probabilities (and
 * hence, are positive numbers), as is the return value.  This is the
 * key to the "fastness" of this function.
 */
LOGMATH_INLINE int
fast_logmath_add(logmath_t *lmath, int mlx, int mly)
{
    logadd_t *t = LOGMATH_TABLE(lmath);
    int d, r;

    /* d must be positive, obviously. */
    if (mlx > mly) {
        d = (mlx - mly);
        r = mly;
    }
    else {
        d = (mly - mlx);
        r = mlx;
    }

    return r - (((uint8 *)t->table)[d]);
}

static int32 get_scores4_4b(s2_semi_mgau_t * s, int16 *senone_scores,
                            int32 *senone_active, int32 n_senone_active,
                            int32 *out_bestidx);
static int32 get_scores_rle(s2_semi_mgau_t * s, int16 *senone_scores,
                            int32 *senone_active, int32 n_senone_active,
                            int32 *out_bestidx);
static int32 get_scores_rle_all(s2_semi_mgau_t * s, int16 *senone_scores,
                                int32 *out_bestidx);

static int32 get_scores4_8b(s2_semi_mgau_t * s, int16 *senone_scores,
                            int32 *senone_active, int32 n_senone_active,
                            int32 *out_bestidx);
static int32 get_scores2_8b(s2_semi_mgau_t * s, int16 *senone_scores,
                            int32 *senone_active, int32 n_senone_active,
                            int32 *out_bestidx);
static int32 get_scores1_8b(s2_semi_mgau_t * s, int16 *senone_scores,
                            int32 *senone_active, int32 n_senone_active,
                            int32 *out_bestidx);
static int32 get_scores_8b(s2_semi_mgau_t * s, int16 *senone_scores,
                           int32 *senone_active, int32 n_senone_active,
                           int32 *out_bestidx);
static int32 get_scores4_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                                int32 *out_bestidx);
static int32 get_scores2_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                                int32 *out_bestidx);
static int32 get_scores1_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                                int32 *out_bestidx);
static int32 get_scores_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                               int32 *out_bestidx);

static void
eval_topn(s2_semi_mgau_t *s, int32 feat, mfcc_t *z)
{
    int32 i, ceplen;
    vqFeature_t *topn;

    topn = s->f[feat];
    ceplen = s->veclen[feat];
    
    for (i = 0; i < s->topn; i++) {
        mean_t *mean, diff, sqdiff, compl; /* diff, diff^2, component likelihood */
        vqFeature_t vtmp;
        var_t *var, d;
        mfcc_t *obs;
        int32 cw, j;

        cw = topn[i].codeword;
        mean = s->means[feat] + cw * ceplen;
        var = s->vars[feat] + cw * ceplen;
        d = s->dets[feat][cw];
        obs = z;
        for (j = 0; j < ceplen; j++) {
            diff = *obs++ - *mean++;
            /* FIXME: Use standard deviations, to avoid squaring, as per Bhiksha. */
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMSUB(d, compl);
            ++var;
        }
        topn[i].score = (int32)d;
        if (i == 0)
            continue;
        vtmp = topn[i];
        for (j = i - 1; j >= 0 && (int32)d > topn[j].score; j--) {
            topn[j + 1] = topn[j];
        }
        topn[j + 1] = vtmp;
    }
}

static void
eval_cb_kdtree(s2_semi_mgau_t *s, int32 feat, mfcc_t *z,
               kd_tree_node_t *node, uint32 maxbbi)
{
    vqFeature_t *worst, *best, *topn;
    int32 i, ceplen;

    best = topn = s->f[feat];
    worst = topn + (s->topn - 1);
    ceplen = s->veclen[feat];

    for (i = 0; i < maxbbi; ++i) {
        mean_t *mean, diff, sqdiff, compl; /* diff, diff^2, component likelihood */
        var_t *var, d;
        mfcc_t *obs;
        vqFeature_t *cur;
        int32 cw, j, k;

        cw = node->bbi[i];
        mean = s->means[feat] + cw * ceplen;
        var = s->vars[feat] + cw * ceplen;
        d = s->dets[feat][cw];
        obs = z;
        for (j = 0; (j < ceplen) && (d >= worst->score); j++) {
            diff = *obs++ - *mean++;
            /* FIXME: Use standard deviations, to avoid squaring, as per Bhiksha. */
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMSUB(d, compl);
            ++var;
        }
        if (j < ceplen)
            continue;
        if ((int32)d < worst->score)
            continue;
        for (k = 0; k < s->topn; k++) {
            /* already there, so don't need to insert */
            if (topn[k].codeword == cw)
                break;
        }
        if (k < s->topn)
            continue;       /* already there.  Don't insert */
        /* remaining code inserts codeword and dist in correct spot */
        for (cur = worst - 1; cur >= best && (int32)d >= cur->score; --cur)
            memcpy(cur + 1, cur, sizeof(vqFeature_t));
        ++cur;
        cur->codeword = cw;
        cur->score = (int32)d;
    }
}

static void
eval_cb(s2_semi_mgau_t *s, int32 feat, mfcc_t *z)
{
    vqFeature_t *worst, *best, *topn;
    mean_t *mean;
    var_t *var, *det, *detP, *detE;
    int32 i, ceplen;

    best = topn = s->f[feat];
    worst = topn + (s->topn - 1);
    mean = s->means[feat];
    var = s->vars[feat];
    det = s->dets[feat];
    detE = det + s->n_density;
    ceplen = s->veclen[feat];

    for (detP = det; detP < detE; ++detP) {
        mean_t diff, sqdiff, compl; /* diff, diff^2, component likelihood */
        var_t d;
        mfcc_t *obs;
        vqFeature_t *cur;
        int32 cw, j;

        d = *detP;
        obs = z;
        cw = detP - det;
        for (j = 0; (j < ceplen) && (d >= worst->score); ++j) {
            diff = *obs++ - *mean++;
            /* FIXME: Use standard deviations, to avoid squaring, as per Bhiksha. */
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMSUB(d, compl);
            ++var;
        }
        if (j < ceplen) {
            /* terminated early, so not in topn */
            mean += (ceplen - j);
            var += (ceplen - j);
            continue;
        }
        if ((int32)d < worst->score)
            continue;
        for (i = 0; i < s->topn; i++) {
            /* already there, so don't need to insert */
            if (topn[i].codeword == cw)
                break;
        }
        if (i < s->topn)
            continue;       /* already there.  Don't insert */
        /* remaining code inserts codeword and dist in correct spot */
        for (cur = worst - 1; cur >= best && (int32)d >= cur->score; --cur)
            memcpy(cur + 1, cur, sizeof(vqFeature_t));
        ++cur;
        cur->codeword = cw;
        cur->score = (int32)d;
    }
}

static void
mgau_dist(s2_semi_mgau_t * s, int32 frame, int32 feat, mfcc_t * z)
{
    /* Initialize topn codewords to topn codewords from previous
     * frame, and calculate their densities. */
    memcpy(s->f[feat], s->lastf[feat], sizeof(vqFeature_t) * s->topn);
    eval_topn(s, feat, z);

    /* If this frame is skipped, do nothing else. */
    if (frame % s->ds_ratio)
        return;

    /* Evaluate the rest of the codebook (or subset thereof). */
    if (s->kdtrees) {
        kd_tree_node_t *node;
        uint32 maxbbi;

        node =
            eval_kd_tree(s->kdtrees[feat], z, s->kd_maxdepth);
        maxbbi = s->kd_maxbbi == -1 ? node->n_bbi : MIN(node->n_bbi,
                                                        s->
                                                        kd_maxbbi);
        eval_cb_kdtree(s, feat, z, node, maxbbi);
    }
    else {
        eval_cb(s, feat, z);
    }

    /* Make a copy of current topn. */
    memcpy(s->lastf[feat], s->f[feat], sizeof(vqFeature_t) * s->topn);
}

static void
mgau_norm(s2_semi_mgau_t *s, int feat)
{
    int32 norm;
    int j;

    /* Compute quantized normalizing constant. */
    norm = s->f[feat][0].score >> SENSCR_SHIFT;
    for (j = 1; j < s->topn; ++j) {
        norm = logmath_add(s->lmath_8b, norm,
                           s->f[feat][j].score >> SENSCR_SHIFT);
    }

    /* Normalize the scores, negate them, and clamp their dynamic range. */
    for (j = 0; j < s->topn; ++j) {
        s->f[feat][j].score = -((s->f[feat][j].score >> SENSCR_SHIFT) - norm);
        if (s->f[feat][j].score < 0 || s->f[feat][j].score > MAX_NEG_ASCR)
            s->f[feat][j].score = MAX_NEG_ASCR;
    }
}

/*
 * Compute senone scores for the active senones.
 */
int32
s2_semi_mgau_frame_eval(s2_semi_mgau_t * s,
                        int16 *senone_scores,
                        int32 *senone_active,
                        int32 n_senone_active,
			mfcc_t ** featbuf, int32 frame,
			int32 compallsen,
                        int32 *out_bestidx)
{
    int i;

    for (i = 0; i < s->n_feat; ++i) {
        mgau_dist(s, frame, i, featbuf[i]);
        mgau_norm(s, i);
    }

    if (compallsen) {
        if (s->rle_bits) {
            return get_scores_rle_all(s, senone_scores, out_bestidx);
        }
	switch (s->topn) {
	case 4:
	    return get_scores4_8b_all(s, senone_scores, out_bestidx);
	case 2:
	    return get_scores2_8b_all(s, senone_scores, out_bestidx);
	case 1:
	    return get_scores1_8b_all(s, senone_scores, out_bestidx);
	default:
	    return get_scores_8b_all(s, senone_scores, out_bestidx);
	}
    }
    else {
        if (s->rle_bits) {
            return get_scores_rle(s, senone_scores, senone_active,
                                  n_senone_active, out_bestidx);
        }
        switch (s->topn) {
	case 4:
            if (s->mixw_cb)
                return get_scores4_4b(s, senone_scores,
                                      senone_active, n_senone_active,
                                      out_bestidx);
            else
                return get_scores4_8b(s, senone_scores,
                                      senone_active, n_senone_active,
                                      out_bestidx);
	case 2:
	    return get_scores2_8b(s, senone_scores,
                                  senone_active, n_senone_active,
                                  out_bestidx);
	case 1:
	    return get_scores1_8b(s, senone_scores,
                                  senone_active, n_senone_active,
                                  out_bestidx);
	default:
	    return get_scores_8b(s, senone_scores,
                                  senone_active, n_senone_active,
                                  out_bestidx);
	}
    }
}

static int32
get_scores_rle(s2_semi_mgau_t * s, int16 *senone_scores,
               int32 *senone_active, int32 n_senone_active,
               int32 *out_bestidx)
{
    int32 j;
    uint8 w_den[4][16];
    int32 best = (int32)0x7fffffff;
    uint8 tmp[6000];

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (j = 0; j < s->n_feat; j++) {
        int32 i, k;

        /* Precompute scaled densities. */
        for (k = 0; k < 16; ++k) {
            w_den[0][k] = s->mixw_cb[k] + s->f[j][0].score;
            w_den[1][k] = s->mixw_cb[k] + s->f[j][1].score;
            w_den[2][k] = s->mixw_cb[k] + s->f[j][2].score;
            w_den[3][k] = s->mixw_cb[k] + s->f[j][3].score;
        }

        /* Compute partial scores for each active component. */
        for (i = 0; i < s->topn; ++i) {
            uint8 *x;
            int32 *n, *en;
            int l, cw, endrun;

            /* Set up initial senone and codeword pointers. */
            n = senone_active;
            en = senone_active + n_senone_active;
            x = s->mixw[j][s->f[j][i].codeword];
            cw = endrun = 0;

            /* Run through all senones. */
            while (n < en) {
                /* Scan forward to encompass current active senone. */
                while (endrun <= *n) {
                    l = (*x >> 4) + 1;
                    cw = *x & 0xf;
                    endrun += l;
                    ++x;
                }
                /* Scan forward to encompass all senones in this run. */
                if (i == 0) {
                    while (n < en && *n < endrun) {
                        tmp[*n] = w_den[i][cw];
                        ++n;
                    }
                }
                else {
                    while (n < en && *n < endrun) {
                        tmp[*n] = fast_logmath_add(s->lmath_8b, tmp[*n], w_den[i][cw]);
                        ++n;
                    }
                }

                assert(endrun <= s->n_sen);
                if (endrun == s->n_sen)
                    break;
            }
        }

        /* And now, add together the partial scores. */
        for (k = 0; k < n_senone_active; k++) {
	    int n = senone_active[k];
            senone_scores[n] += tmp[n];
            if (j == s->n_feat - 1 && senone_scores[n] < best) {
                best = senone_scores[n];
                *out_bestidx = n;
            }
        }
    }

    return best;
}

static int32
get_scores_rle_all(s2_semi_mgau_t * s, int16 *senone_scores, int32 *out_bestidx)
{
    int32 j;
    uint8 w_den[4][16];
    int32 best = (int32)0x7fffffff;
    uint8 tmp[6000];

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (j = 0; j < s->n_feat; j++) {
        int32 i, k, n;

        /* Precompute scaled densities. */
        for (k = 0; k < 16; ++k) {
            w_den[0][k] = s->mixw_cb[k] + s->f[j][0].score;
            w_den[1][k] = s->mixw_cb[k] + s->f[j][1].score;
            w_den[2][k] = s->mixw_cb[k] + s->f[j][2].score;
            w_den[3][k] = s->mixw_cb[k] + s->f[j][3].score;
        }

        /* Compute partial scores for each active component. */
        for (i = 0; i < s->topn; ++i) {
            uint8 *x;
            int l, cw;

            x = s->mixw[j][s->f[j][i].codeword];
            /* Run through all senones. */
            n = 0;
            while (n < s->n_sen) {
                l = (*x >> 4) + 1;
                cw = *x & 0xf;
                ++x;
                /* Do all senones in this run. */
                if (i == 0) {
                    memset(tmp + n, w_den[i][cw], l);
                }
                else {
                    int r;
                    for (r = 0; r < l; ++r) {
                        tmp[n + r] = fast_logmath_add(s->lmath_8b,
                                                      tmp[n + r], w_den[i][cw]);
                    }
                }
                n += l;
            }
        }

        /* And now, add together the partial scores. */
        for (n = 0; n < s->n_sen; ++n) {
            senone_scores[n] += tmp[n];
            if (j == s->n_feat - 1 && senone_scores[n] < best) {
                best = senone_scores[n];
                *out_bestidx = n;
            }
        }
    }

    return best;
}

static int32
get_scores4_4b(s2_semi_mgau_t * s, int16 *senone_scores,
               int32 *senone_active, int32 n_senone_active,
               int32 *out_bestidx)
{
    int32 j;
    uint8 w_den[4][16];
    int32 best = (int32)0x7fffffff;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (j = 0; j < s->n_feat; j++) {
        uint8 *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
        int32 k;

        /* Precompute scaled densities. */
        for (k = 0; k < 16; ++k) {
            w_den[0][k] = s->mixw_cb[k] + s->f[j][0].score;
            w_den[1][k] = s->mixw_cb[k] + s->f[j][1].score;
            w_den[2][k] = s->mixw_cb[k] + s->f[j][2].score;
            w_den[3][k] = s->mixw_cb[k] + s->f[j][3].score;
        }

        /* ptrs to senone prob ids */
        pid_cw0 = s->mixw[j][s->f[j][0].codeword];
        pid_cw1 = s->mixw[j][s->f[j][1].codeword];
        pid_cw2 = s->mixw[j][s->f[j][2].codeword];
        pid_cw3 = s->mixw[j][s->f[j][3].codeword];

        for (k = 0; k < n_senone_active; k++) {
	    int n = senone_active[k];
            int tmp, cw;

            if (n & 1) {
                cw = pid_cw0[n/2] >> 4;
                tmp = w_den[0][cw];
                cw = pid_cw1[n/2] >> 4;
                tmp = fast_logmath_add(s->lmath_8b, tmp, w_den[1][cw]);
                cw = pid_cw2[n/2] >> 4;
                tmp = fast_logmath_add(s->lmath_8b, tmp, w_den[2][cw]);
                cw = pid_cw3[n/2] >> 4;
                tmp = fast_logmath_add(s->lmath_8b, tmp, w_den[3][cw]);
            }
            else {
                cw = pid_cw0[n/2] & 0x0f;
                tmp = w_den[0][cw];
                cw = pid_cw1[n/2] & 0x0f;
                tmp = fast_logmath_add(s->lmath_8b, tmp, w_den[1][cw]);
                cw = pid_cw2[n/2] & 0x0f;
                tmp = fast_logmath_add(s->lmath_8b, tmp, w_den[2][cw]);
                cw = pid_cw3[n/2] & 0x0f;
                tmp = fast_logmath_add(s->lmath_8b, tmp, w_den[3][cw]);
            }
            senone_scores[n] += tmp;
            if (j == s->n_feat - 1 && senone_scores[n] < best) {
                best = senone_scores[n];
                *out_bestidx = n;
            }
        }
    }

    return best;
}

static int32
get_scores_8b(s2_semi_mgau_t * s, int16 *senone_scores,
              int32 *senone_active, int32 n_senone_active,
              int32 *out_bestidx)
{
    int32 i, j, k;
    int32 best = (int32)0x7fffffff;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (i = 0; i < s->n_feat; ++i) {
        for (j = 0; j < n_senone_active; j++) {
            int sen = senone_active[j];
            uint8 *pid_cw;
            int32 tmp;
            pid_cw = s->mixw[i][s->f[i][0].codeword];
            tmp = pid_cw[sen] + s->f[i][0].score;
            for (k = 1; k < s->topn; ++k) {
                pid_cw = s->mixw[i][s->f[i][k].codeword];
                tmp = fast_logmath_add(s->lmath_8b, tmp,
                                       pid_cw[sen] + s->f[i][k].score);
            }
            senone_scores[sen] += tmp;
            if (i == s->n_feat - 1 && senone_scores[sen] < best) {
                best = senone_scores[sen];
                *out_bestidx = sen;
            }
        }
    }
    return best;
}

static int32
get_scores_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                  int32 *out_bestidx)
{
    int32 i, j, k;
    int32 best = (int32)0x7fffffff;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (i = 0; i < s->n_feat; ++i) {
        for (j = 0; j < s->n_sen; j++) {
            uint8 *pid_cw;
            int32 tmp;
            pid_cw = s->mixw[i][s->f[i][0].codeword];
            tmp = pid_cw[j] + s->f[i][0].score;
            for (k = 1; k < s->topn; ++k) {
                pid_cw = s->mixw[i][s->f[i][k].codeword];
                tmp = fast_logmath_add(s->lmath_8b, tmp,
                                       pid_cw[j] + s->f[i][k].score);
            }
            senone_scores[j] += tmp;
            if (i == s->n_feat - 1 && senone_scores[j] < best) {
                best = senone_scores[j];
                *out_bestidx = j;
            }
        }
    }
    return best;
}

static int32
get_scores4_8b(s2_semi_mgau_t * s, int16 *senone_scores,
               int32 *senone_active, int32 n_senone_active,
               int32 *out_bestidx)
{
    int32 j;
    int32 best = (int32)0x7fffffff;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (j = 0; j < s->n_feat; j++) {
        uint8 *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
        int32 k;

        /* ptrs to senone prob ids */
        pid_cw0 = s->mixw[j][s->f[j][0].codeword];
        pid_cw1 = s->mixw[j][s->f[j][1].codeword];
        pid_cw2 = s->mixw[j][s->f[j][2].codeword];
        pid_cw3 = s->mixw[j][s->f[j][3].codeword];

        for (k = 0; k < n_senone_active; k++) {
            int32 tmp1, tmp2;
	    int32 n = senone_active[k];

            tmp1 = pid_cw0[n] + s->f[j][0].score;
            tmp2 = pid_cw1[n] + s->f[j][1].score;
            tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
            tmp2 = pid_cw2[n] + s->f[j][2].score;
            tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
            tmp2 = pid_cw3[n] + s->f[j][3].score;
            tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);

            senone_scores[n] += tmp1;
            if (j == s->n_feat - 1 && senone_scores[n] < best) {
                best = senone_scores[n];
                *out_bestidx = n;
            }
        }
    }
    return best;
}

static int32
get_scores4_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                   int32 *out_bestidx)
{
    int32 j;
    int32 best = (int32)0x7fffffff;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    for (j = 0; j < s->n_feat; j++) {
        uint8 *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
        int32 n;

        /* ptrs to senone prob ids */
        pid_cw0 = s->mixw[j][s->f[j][0].codeword];
        pid_cw1 = s->mixw[j][s->f[j][1].codeword];
        pid_cw2 = s->mixw[j][s->f[j][2].codeword];
        pid_cw3 = s->mixw[j][s->f[j][3].codeword];

        for (n = 0; n < s->n_sen; n++) {
            int32 tmp1, tmp2;
            tmp1 = pid_cw0[n] + s->f[j][0].score;
            tmp2 = pid_cw1[n] + s->f[j][1].score;
            tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
            tmp2 = pid_cw2[n] + s->f[j][2].score;
            tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
            tmp2 = pid_cw3[n] + s->f[j][3].score;
            tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);

            senone_scores[n] += tmp1;
            if (j == s->n_feat - 1 && senone_scores[n] < best) {
                best = senone_scores[n];
                *out_bestidx = n;
            }
        }
    }
    return best;
}

static int32
get_scores2_8b(s2_semi_mgau_t * s, int16 *senone_scores,
               int32 *senone_active, int32 n_senone_active,
               int32 *out_bestidx)
{
    int32 k;
    int32 best = (int32)0x7fffffff;
    uint8 *pid_cw00, *pid_cw10, *pid_cw01, *pid_cw11,
        *pid_cw02, *pid_cw12, *pid_cw03, *pid_cw13;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    /* ptrs to senone prob ids */
    pid_cw00 = s->mixw[0][s->f[0][0].codeword];
    pid_cw10 = s->mixw[0][s->f[0][1].codeword];
    pid_cw01 = s->mixw[1][s->f[1][0].codeword];
    pid_cw11 = s->mixw[1][s->f[1][1].codeword];
    pid_cw02 = s->mixw[2][s->f[2][0].codeword];
    pid_cw12 = s->mixw[2][s->f[2][1].codeword];
    pid_cw03 = s->mixw[3][s->f[3][0].codeword];
    pid_cw13 = s->mixw[3][s->f[3][1].codeword];

    for (k = 0; k < n_senone_active; k++) {
        int32 tmp1, tmp2, n;
	n = senone_active[k];

        tmp1 = pid_cw00[n] + s->f[0][0].score;
        tmp2 = pid_cw10[n] + s->f[0][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        tmp1 = pid_cw01[n] + s->f[1][0].score;
        tmp2 = pid_cw11[n] + s->f[1][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        tmp1 = pid_cw02[n] + s->f[2][0].score;
        tmp2 = pid_cw12[n] + s->f[2][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        tmp1 = pid_cw03[n] + s->f[3][0].score;
        tmp2 = pid_cw13[n] + s->f[3][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        if (senone_scores[n] < best) {
            best = senone_scores[n];
            *out_bestidx = n;
        }
    }
    return best;
}

static int32
get_scores2_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                   int32 *out_bestidx)
{
    uint8 *pid_cw00, *pid_cw10, *pid_cw01, *pid_cw11,
        *pid_cw02, *pid_cw12, *pid_cw03, *pid_cw13;
    int32 best = (int32)0x7fffffff;
    int32 n;

    memset(senone_scores, 0, s->n_sen * sizeof(*senone_scores));
    /* ptrs to senone prob ids */
    pid_cw00 = s->mixw[0][s->f[0][0].codeword];
    pid_cw10 = s->mixw[0][s->f[0][1].codeword];
    pid_cw01 = s->mixw[1][s->f[1][0].codeword];
    pid_cw11 = s->mixw[1][s->f[1][1].codeword];
    pid_cw02 = s->mixw[2][s->f[2][0].codeword];
    pid_cw12 = s->mixw[2][s->f[2][1].codeword];
    pid_cw03 = s->mixw[3][s->f[3][0].codeword];
    pid_cw13 = s->mixw[3][s->f[3][1].codeword];

    for (n = 0; n < s->n_sen; n++) {
        int32 tmp1, tmp2;

        tmp1 = pid_cw00[n] + s->f[0][0].score;
        tmp2 = pid_cw10[n] + s->f[0][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        tmp1 = pid_cw01[n] + s->f[1][0].score;
        tmp2 = pid_cw11[n] + s->f[1][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        tmp1 = pid_cw02[n] + s->f[2][0].score;
        tmp2 = pid_cw12[n] + s->f[2][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        tmp1 = pid_cw03[n] + s->f[3][0].score;
        tmp2 = pid_cw13[n] + s->f[3][1].score;
        tmp1 = fast_logmath_add(s->lmath_8b, tmp1, tmp2);
        senone_scores[n] += tmp1;
        if (senone_scores[n] < best) {
            best = senone_scores[n];
            *out_bestidx = n;
        }
    }
    return best;
}

static int32
get_scores1_8b(s2_semi_mgau_t * s, int16 *senone_scores,
               int32 *senone_active, int32 n_senone_active,
               int32 *out_bestidx)
{
    int32 j, k;
    int32 best = (int32)0x7fffffff;
    uint8 *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;

    /* Ptrs to senone prob values for the top codeword of all codebooks */
    pid_cw0 = s->mixw[0][s->f[0][0].codeword];
    pid_cw1 = s->mixw[1][s->f[1][0].codeword];
    pid_cw2 = s->mixw[2][s->f[2][0].codeword];
    pid_cw3 = s->mixw[3][s->f[3][0].codeword];

    for (k = 0; k < n_senone_active; k++) {
        j = senone_active[k];
        senone_scores[j] =
            (pid_cw0[j] + pid_cw1[j] + pid_cw2[j] + pid_cw3[j]);
        if (senone_scores[j] < best) {
            best = senone_scores[j];
            *out_bestidx = j;
        }
    }
    return best;
}

static int32
get_scores1_8b_all(s2_semi_mgau_t * s, int16 *senone_scores,
                   int32 *out_bestidx)
{
    int32 j;
    int32 best = (int32)0x7fffffff;
    uint8 *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;

    /* Ptrs to senone prob values for the top codeword of all codebooks */
    pid_cw0 = s->mixw[0][s->f[0][0].codeword];
    pid_cw1 = s->mixw[1][s->f[1][0].codeword];
    pid_cw2 = s->mixw[2][s->f[2][0].codeword];
    pid_cw3 = s->mixw[3][s->f[3][0].codeword];

    for (j = 0; j < s->n_sen; j++) {
        senone_scores[j] =
            (pid_cw0[j] + pid_cw1[j] + pid_cw2[j] + pid_cw3[j]);
        if (senone_scores[j] < best) {
            best = senone_scores[j];
            *out_bestidx = j;
        }
    }
    return best;
}

int32
s2_semi_mgau_load_kdtree(s2_semi_mgau_t * s, const char *kdtree_path,
                         uint32 maxdepth, int32 maxbbi)
{
    if (read_kd_trees(kdtree_path, &s->kdtrees, &s->n_kdtrees,
                      maxdepth, maxbbi) == -1)
        E_FATAL("Failed to read kd-trees from %s\n", kdtree_path);
    if (s->n_kdtrees != s->n_feat)
        E_FATAL("Number of kd-trees != %d\n", s->n_feat);

    s->kd_maxdepth = maxdepth;
    s->kd_maxbbi = maxbbi;
    return 0;
}

static int32
read_sendump_rle(s2_semi_mgau_t *s, FILE *file,
                 size_t offset, size_t filesize,
                 int32 n_comp, int32 n_sen)
{
    uint8 *mwdata, *x, *e;
    int32 f, c, i;

    /* Read in the data. */
    if (s->sendump_mmap) {
        mwdata = (uint8 *)mmio_file_ptr(s->sendump_mmap) + offset;
    }
    else {
        mwdata = ckd_malloc(filesize - offset);
        if (fread(mwdata, 1, filesize - offset, file) != filesize - offset) {
            E_ERROR("Failed to read %d bytes from sendump\n", filesize - offset);
            return -1;
        }
    }

    /* Now decode it to find the endpoints for each (feature,component) */
    s->mixw = ckd_calloc_2d(s->n_feat, n_comp, sizeof(**s->mixw));
    x = mwdata;
    e = mwdata + filesize - offset;
    for (f = 0; f < s->n_feat; ++f) {
        for (c = 0; c < n_comp; ++c) {
            s->mixw[f][c] = x;
            i = 0;
            while (i < n_sen) {
                int cw, l;
                if (x >= e) {
                    E_ERROR("Inconsistent RLE in sendump: final n_sen = %d\n", i);
                    return -1;
                }
                l = (*x >> 4) + 1;
                cw = *x & 0xf;
                i += l;
                ++x;
            }
        }
    }

    fclose(file);
    return 0;
}

static int32
read_sendump(s2_semi_mgau_t *s, bin_mdef_t *mdef, char const *file)
{
    FILE *fp;
    char line[1000];
    int32 i, n, r, c;
    int32 do_swap, do_mmap;
    size_t filesize, offset;
    int n_clust = 0;
    int n_feat = s->n_feat;
    int n_density = s->n_density;
    int n_sen = bin_mdef_n_sen(mdef);
    int n_bits = 8;

    s->n_sen = n_sen; /* FIXME: Should have been done earlier */
    do_mmap = cmd_ln_boolean_r(s->config, "-mmap");

    if ((fp = fopen(file, "rb")) == NULL)
        return -1;

    E_INFO("Loading senones from dump file %s\n", file);
    /* Read title size, title */
    fread(&n, sizeof(int32), 1, fp);
    /* This is extremely bogus */
    do_swap = 0;
    if (n < 1 || n > 999) {
        SWAP_INT32(&n);
        if (n < 1 || n > 999) {
            E_FATAL("Title length %x in dump file %s out of range\n", n, file);
        }
        do_swap = 1;
    }
    if (fread(line, sizeof(char), n, fp) != n)
        E_FATAL("Cannot read title\n");
    if (line[n - 1] != '\0')
        E_FATAL("Bad title in dump file\n");
    E_INFO("%s\n", line);

    /* Read header size, header */
    fread(&n, 1, sizeof(n), fp);
    if (do_swap) SWAP_INT32(&n);
    if (fread(line, sizeof(char), n, fp) != n)
        E_FATAL("Cannot read header\n");
    if (line[n - 1] != '\0')
        E_FATAL("Bad header in dump file\n");

    /* Read other header strings until string length = 0 */
    for (;;) {
        fread(&n, 1, sizeof(n), fp);
        if (do_swap) SWAP_INT32(&n);
        if (n == 0)
            break;
        if (fread(line, sizeof(char), n, fp) != n)
            E_FATAL("Cannot read header\n");
        /* Look for a cluster count, if present */
        if (!strncmp(line, "feature_count ", strlen("feature_count "))) {
            n_feat = atoi(line + strlen("feature_count "));
        }
        if (!strncmp(line, "mixture_count ", strlen("mixture_count "))) {
            n_density = atoi(line + strlen("mixture_count "));
        }
        if (!strncmp(line, "model_count ", strlen("model_count "))) {
            n_sen = atoi(line + strlen("model_count "));
        }
        if (!strncmp(line, "cluster_count ", strlen("cluster_count "))) {
            n_clust = atoi(line + strlen("cluster_count "));
        }
        if (!strncmp(line, "cluster_bits ", strlen("cluster_bits "))) {
            n_bits = atoi(line + strlen("cluster_bits "));
        }
        if (!strncmp(line, "rle_bits ", strlen("rle_bits "))) {
            s->rle_bits = atoi(line + strlen("rle_bits "));
        }
    }

    /* Read #codewords, #pdfs, but only if there is no cluster_count */
    if (n_clust == 0) {
        fread(&r, 1, sizeof(r), fp);
        if (do_swap) SWAP_INT32(&r);
        fread(&c, 1, sizeof(c), fp);
        if (do_swap) SWAP_INT32(&c);
        E_INFO("Rows: %d, Columns: %d\n", r, c);
    }

    if (n_feat != s->n_feat) {
        E_ERROR("Number of feature streams mismatch: %d != %d\n",
                n_feat, s->n_feat);
        fclose(fp);
        return -1;
    }
    if (n_density != s->n_density) {
        E_ERROR("Number of densities mismatch: %d != %d\n",
                n_density, s->n_density);
        fclose(fp);
        return -1;
    }
    if (n_sen != s->n_sen) {
        E_ERROR("Number of senones mismatch: %d != %d\n",
                n_sen, s->n_sen);
        fclose(fp);
        return -1;
    }

    if (!((n_clust == 0) || (n_clust == 15) || (n_clust == 16))) {
        E_ERROR("Cluster count must be 0, 15, or 16\n");
        fclose(fp);
        return -1;
    }
    if (n_clust == 15)
        ++n_clust;

    if (!((n_bits == 8) || (n_bits == 4))) {
        E_ERROR("Cluster count must be 4 or 8\n");
        fclose(fp);
        return -1;
    }

    if (do_mmap) {
            E_INFO("Using memory-mapped I/O for senones\n");
    }
    offset = ftell(fp);
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, offset, SEEK_SET);

    /* Allocate memory for pdfs (or memory map them) */
    if (do_mmap) {
        s->sendump_mmap = mmio_file_read(file);
        /* Get cluster codebook if any. */
        if (n_clust) {
            s->mixw_cb = ((uint8 *) mmio_file_ptr(s->sendump_mmap)) + offset;
            offset += n_clust;
        }
    }
    else {
        /* Get cluster codebook if any. */
        if (n_clust) {
            s->mixw_cb = ckd_calloc(1, n_clust);
            if (fread(s->mixw_cb, 1, n_clust, fp) != (size_t) n_clust) {
                E_ERROR("Failed to read %d bytes from sendump\n", n_clust);
                return -1;
            }
        }
    }

    /* If RLE is enabled, we need to scan the file to get start points. */
    if (s->rle_bits) {
        return read_sendump_rle(s, fp, offset, filesize, n_density, n_sen);
    }

    /* Set up pointers, or read, or whatever */
    if (s->sendump_mmap) {
        s->mixw = ckd_calloc_2d(s->n_feat, n_density, sizeof(*s->mixw));
        for (n = 0; n < n_feat; n++) {
            int step = n_sen;
            if (n_bits == 4)
                step = (step + 1) / 2;
            for (i = 0; i < n_density; i++) {
                s->mixw[n][i] = ((uint8 *) mmio_file_ptr(s->sendump_mmap)) + offset;
                offset += step;
            }
        }
    }
    else {
        s->mixw = ckd_calloc_3d(n_feat, n_density, n_sen, sizeof(***s->mixw));
        /* Read pdf values and ids */
        for (n = 0; n < n_feat; n++) {
            int step = n_sen;
            if (n_bits == 4)
                step = (step + 1) / 2;
            for (i = 0; i < n_density; i++) {
                if (fread(s->mixw[n][i], sizeof(***s->mixw), step, fp)
                    != (size_t) step) {
                    E_ERROR("Failed to read %d bytes from sendump\n", step);
                    return -1;
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

static int32
read_mixw(s2_semi_mgau_t * s, char const *file_name, double SmoothMin)
{
    char **argname, **argval;
    char eofchk;
    FILE *fp;
    int32 byteswap, chksum_present;
    uint32 chksum;
    float32 *pdf;
    int32 i, f, c, n;
    int32 n_sen;
    int32 n_feat;
    int32 n_comp;
    int32 n_err;

    E_INFO("Reading mixture weights file '%s'\n", file_name);

    if ((fp = fopen(file_name, "rb")) == NULL)
        E_FATAL("fopen(%s,rb) failed\n", file_name);

    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr(fp, &argname, &argval, &byteswap) < 0)
        E_FATAL("bio_readhdr(%s) failed\n", file_name);

    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
        if (strcmp(argname[i], "version") == 0) {
            if (strcmp(argval[i], MGAU_MIXW_VERSION) != 0)
                E_WARN("Version mismatch(%s): %s, expecting %s\n",
                       file_name, argval[i], MGAU_MIXW_VERSION);
        }
        else if (strcmp(argname[i], "chksum0") == 0) {
            chksum_present = 1; /* Ignore the associated value */
        }
    }
    bio_hdrarg_free(argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #senones, #features, #codewords, arraysize */
    if ((bio_fread(&n_sen, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
        || (bio_fread(&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) !=
            1)
        || (bio_fread(&n_comp, sizeof(int32), 1, fp, byteswap, &chksum) !=
            1)
        || (bio_fread(&n, sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
        E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if (n_feat != s->n_feat)
        E_FATAL("#Features streams(%d) != %d\n", n_feat, s->n_feat);
    if (n != n_sen * n_feat * n_comp) {
        E_FATAL
            ("%s: #float32s(%d) doesn't match header dimensions: %d x %d x %d\n",
             file_name, i, n_sen, n_feat, n_comp);
    }

    /* n_sen = number of mixture weights per codeword, which is
     * fixed at the number of senones since we have only one codebook.
     */
    s->n_sen = n_sen;

    /* Quantized mixture weight arrays. */
    s->mixw = ckd_calloc_3d(s->n_feat, s->n_density, n_sen, sizeof(***s->mixw));

    /* Temporary structure to read in floats before conversion to (int32) logs3 */
    pdf = (float32 *) ckd_calloc(n_comp, sizeof(float32));

    /* Read senone probs data, normalize, floor, convert to logs3, truncate to 8 bits */
    n_err = 0;
    for (i = 0; i < n_sen; i++) {
        for (f = 0; f < n_feat; f++) {
            if (bio_fread((void *) pdf, sizeof(float32),
                          n_comp, fp, byteswap, &chksum) != n_comp) {
                E_FATAL("bio_fread(%s) (arraydata) failed\n", file_name);
            }

            /* Normalize and floor */
            if (vector_sum_norm(pdf, n_comp) <= 0.0)
                n_err++;
            vector_floor(pdf, n_comp, SmoothMin);
            vector_sum_norm(pdf, n_comp);

            /* Convert to LOG, quantize, and transpose */
            for (c = 0; c < n_comp; c++) {
                int32 qscr;

                qscr = -logmath_log(s->lmath_8b, pdf[c]);
                if ((qscr > MAX_NEG_MIXW) || (qscr < 0))
                    qscr = MAX_NEG_MIXW;
                s->mixw[f][c][i] = qscr;
            }
        }
    }
    if (n_err > 0)
        E_ERROR("Weight normalization failed for %d senones\n", n_err);

    ckd_free(pdf);

    if (chksum_present)
        bio_verify_chksum(fp, byteswap, chksum);

    if (fread(&eofchk, 1, 1, fp) == 1)
        E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read %d x %d x %d mixture weights\n", n_sen, n_feat, n_comp);
    return n_sen;
}


/* Read a Sphinx3 mean or variance file. */
static int32
s3_read_mgau(s2_semi_mgau_t *s, const char *file_name, float32 ***out_cb)
{
    char tmp;
    FILE *fp;
    int32 i, blk, n;
    int32 n_mgau;
    int32 n_feat;
    int32 n_density;
    int32 *veclen;
    int32 byteswap, chksum_present;
    char **argname, **argval;
    uint32 chksum;

    E_INFO("Reading S3 mixture gaussian file '%s'\n", file_name);

    if ((fp = fopen(file_name, "rb")) == NULL)
        E_FATAL("fopen(%s,rb) failed\n", file_name);

    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr(fp, &argname, &argval, &byteswap) < 0)
        E_FATAL("bio_readhdr(%s) failed\n", file_name);

    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
        if (strcmp(argname[i], "version") == 0) {
            if (strcmp(argval[i], MGAU_PARAM_VERSION) != 0)
                E_WARN("Version mismatch(%s): %s, expecting %s\n",
                       file_name, argval[i], MGAU_PARAM_VERSION);
        }
        else if (strcmp(argname[i], "chksum0") == 0) {
            chksum_present = 1; /* Ignore the associated value */
        }
    }
    bio_hdrarg_free(argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* #Codebooks */
    if (bio_fread(&n_mgau, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
        E_FATAL("fread(%s) (#codebooks) failed\n", file_name);
    if (n_mgau != 1) {
        E_ERROR("%s: #codebooks (%d) != 1\n", file_name, n_mgau);
        fclose(fp);
        return -1;
    }

    /* #Features/codebook */
    if (bio_fread(&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
        E_FATAL("fread(%s) (#features) failed\n", file_name);
    if (s->n_feat == 0)
        s->n_feat = n_feat;
    else if (n_feat != s->n_feat)
        E_FATAL("#Features streams(%d) != %d\n", n_feat, s->n_feat);

    /* #Gaussian densities/feature in each codebook */
    if (bio_fread(&n_density, sizeof(int32), 1, fp,
                  byteswap, &chksum) != 1)
        E_FATAL("fread(%s) (#density/codebook) failed\n", file_name);
    if (s->n_density == 0)
        s->n_density = n_density;
    else if (n_density != s->n_density)
        E_FATAL("%s: Number of densities per feature(%d) != %d\n",
                file_name, n_mgau, s->n_density);

    /* Vector length of feature stream */
    if (s->veclen == NULL)
        s->veclen = ckd_calloc(s->n_feat, sizeof(int32));
    veclen = ckd_calloc(s->n_feat, sizeof(int32));
    if (bio_fread(veclen, sizeof(int32), s->n_feat,
                  fp, byteswap, &chksum) != s->n_feat)
        E_FATAL("fread(%s) (feature vector-length) failed\n", file_name);
    for (i = 0, blk = 0; i < s->n_feat; ++i) {
        if (s->veclen[i] == 0)
            s->veclen[i] = veclen[i];
        else if (veclen[i] != s->veclen[i])
            E_FATAL("feature stream length %d is inconsistent (%d != %d)\n",
                    i, veclen[i], s->veclen[i]);
        blk += veclen[i];
    }

    /* #Floats to follow; for the ENTIRE SET of CODEBOOKS */
    if (bio_fread(&n, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
        E_FATAL("fread(%s) (total #floats) failed\n", file_name);
    if (n != n_mgau * n_density * blk)
        E_FATAL
            ("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
             file_name, n, n_mgau, n_density, blk);

    *out_cb = ckd_calloc(s->n_feat, sizeof(float32 *));
    for (i = 0; i < s->n_feat; ++i) {
        (*out_cb)[i] =
            (float32 *) ckd_calloc(n_density * veclen[i],
                                   sizeof(float32));
        if (bio_fread
            ((*out_cb)[i], sizeof(float32),
             n_density * veclen[i], fp,
             byteswap, &chksum) != n_density * veclen[i])
            E_FATAL("fread(%s, %d) of feat %d failed\n", file_name,
                    n_density * veclen[i], i);
    }
    ckd_free(veclen);

    if (chksum_present)
        bio_verify_chksum(fp, byteswap, chksum);

    if (fread(&tmp, 1, 1, fp) == 1)
        E_FATAL("%s: More data than expected\n", file_name);

    fclose(fp);

    E_INFO("%d mixture Gaussians, %d components, %d feature streams, veclen %d\n", n_mgau,
           n_density, n_feat, blk);

    return n;
}

static int32
s3_precomp(s2_semi_mgau_t *s, logmath_t *lmath, float32 vFloor)
{
    int feat;

    for (feat = 0; feat < s->n_feat; ++feat) {
        float32 *fmp;
        mean_t *mp;
        var_t *vp, *dp;
        int32 vecLen, i;

        vecLen = s->veclen[feat];
        fmp = (float32 *) s->means[feat];
        mp = s->means[feat];
        vp = s->vars[feat];
        dp = s->dets[feat];

        for (i = 0; i < s->n_density; ++i) {
            var_t d;
            int32 j;

            d = 0;
            for (j = 0; j < vecLen; ++j, ++vp, ++mp, ++fmp) {
                float64 fvar;

#ifdef FIXED_POINT
                *mp = FLOAT2FIX(*fmp);
#endif
                /* Always do these pre-calculations in floating point */
                fvar = *(float32 *) vp;
                if (fvar < vFloor)
                    fvar = vFloor;
                d += (var_t)logmath_log(lmath, 1 / sqrt(fvar * 2.0 * M_PI));
                *vp = (var_t)logmath_ln_to_log(lmath, 1.0 / (2.0 * fvar));
            }
            *dp++ = d;
        }
    }
    return 0;
}

s2_semi_mgau_t *
s2_semi_mgau_init(cmd_ln_t *config, logmath_t *lmath, bin_mdef_t *mdef)
{
    s2_semi_mgau_t *s;
    char const *sendump_path;
    float32 **fgau;
    int i;

    s = ckd_calloc(1, sizeof(*s));
    s->config = config;

    /* Log-add table. */
    s->lmath_8b = logmath_init(logmath_get_base(lmath), SENSCR_SHIFT, TRUE);
    if (s->lmath_8b == NULL) {
        s2_semi_mgau_free(s);
        return NULL;
    }
    /* Ensure that it is only 8 bits wide so that fast_logmath_add() works. */
    if (logmath_get_width(s->lmath_8b) != 1) {
        E_ERROR("Log base %f is too small to represent add table in 8 bits\n",
                logmath_get_base(s->lmath_8b));
        s2_semi_mgau_free(s);
        return NULL;
    }

    /* Read means and variances. */
    if (s3_read_mgau(s, cmd_ln_str_r(config, "-mean"), &fgau) < 0) {
        s2_semi_mgau_free(s);
        return NULL;
    }
    s->means = (mean_t **)fgau;
    if (s3_read_mgau(s, cmd_ln_str_r(config, "-var"), &fgau) < 0) {
        s2_semi_mgau_free(s);
        return NULL;
    }
    s->vars = (var_t **)fgau;

    /* Precompute (and fixed-point-ize) means, variances, and determinants. */
    s->dets = (var_t **)ckd_calloc_2d(s->n_feat, s->n_density, sizeof(**s->dets));
    s3_precomp(s, lmath, cmd_ln_float32_r(config, "-varfloor"));

    s->topn = cmd_ln_int32_r(config, "-topn");
    s->ds_ratio = cmd_ln_int32_r(config, "-ds");

    /* Read mixture weights */
    if ((sendump_path = cmd_ln_str_r(config, "-sendump")))
        read_sendump(s, mdef, sendump_path);
    else
        read_mixw(s, cmd_ln_str_r(config, "-mixw"),
                  cmd_ln_float32_r(config, "-mixwfloor"));

    /* Top-N scores from current, previous frame */
    s->f = (vqFeature_t **) ckd_calloc_2d(s->n_feat, s->topn,
                                          sizeof(vqFeature_t));
    s->lastf = (vqFeature_t **) ckd_calloc_2d(s->n_feat, s->topn,
                                              sizeof(vqFeature_t));
    for (i = 0; i < s->n_feat; ++i) {
        int32 j;
        for (j = 0; j < s->topn; ++j) {
            s->lastf[i][j].score = WORST_DIST;
            s->lastf[i][j].codeword = j;
        }
    }

    return s;
}

void
s2_semi_mgau_free(s2_semi_mgau_t * s)
{
    uint32 i;

    logmath_free(s->lmath_8b);
    if (s->sendump_mmap) {
        ckd_free_2d(s->mixw); 
        mmio_file_unmap(s->sendump_mmap);
    }
    else {
        ckd_free_3d(s->mixw);
    }
    for (i = 0; i < s->n_feat; ++i) {
        ckd_free(s->means[i]);
        ckd_free(s->vars[i]);
    }
    for (i = 0; i < s->n_kdtrees; ++i)
        free_kd_tree(s->kdtrees[i]);
    ckd_free(s->kdtrees);
    ckd_free(s->veclen);
    ckd_free(s->means);
    ckd_free(s->vars);
    ckd_free_2d((void **)s->f);
    ckd_free_2d((void **)s->lastf);
    ckd_free_2d((void **)s->dets);
    ckd_free(s);
}
