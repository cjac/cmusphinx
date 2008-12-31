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
 *
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2006/04/06  14:03:02  dhdfu
 * Prevent confusion among future generations by calling this s2_semi_mgau instead of sc_vq
 * 
 * Revision 1.1  2006/04/05 20:14:26  dhdfu
 * Add cut-down support for Sphinx-2 fast GMM computation (from
 * PocketSphinx).  This does *not* support Sphinx2 format models, but
 * rather semi-continuous Sphinx3 models.  I'll try to write a model
 * converter at some point soon.
 *
 * Unfortunately the smallest models I have for testing don't do so well
 * on the AN4 test sentence (should use AN4 models, maybe...) so it comes
 * with a "don't panic" warning.
 *
 * Revision 1.4  2006/04/04 15:31:31  dhuggins
 * Remove redundant acoustic score scaling in senone computation.
 *
 * Revision 1.3  2006/04/04 15:24:29  dhuggins
 * Get the meaning of LOG_BASE right (oops!).  Seems to work fine now, at
 * least at logbase=1.0001.
 *
 * Revision 1.2  2006/04/04 14:54:40  dhuggins
 * Add support for s2_semi_mgau - it doesn't crash, but it doesn't work either :)
 *
 * Revision 1.1  2006/04/04 04:25:17  dhuggins
 * Add a cut-down version of sphinx2 fast GMM computation (SCVQ) from
 * PocketSphinx.  Not enabled or tested yet.  Doesn't support Sphinx2
 * models (write an external conversion tool instead, please).  Hopefully
 * this will put an end to me complaining about Sphinx3 being too slow :-)
 *
 * Revision 1.12  2004/12/10 16:48:56  rkm
 * Added continuous density acoustic model handling
 *
 * 
 * 22-Nov-2004  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 		Moved best senone score and best senone within phone
 * 		computation out of here and into senscr module, for
 * 		integrating continuous  models into sphinx2.
 * 
 * 19-Nov-97  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added ability to read power variance file if it exists.
 * 
 * 19-Jun-95  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added scvq_set_psen() and scvq_set_bestpscr().  Modified SCVQScores_all to
 * 	also compute best senone score/phone.
 * 
 * 19-May-95  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added check for bad VQ scores in SCVQScores and SCVQScores_all.
 * 
 * 01-Jul-94  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	In SCVQScores, returned result from SCVQComputeScores_opt().
 * 
 * 01-Nov-93  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added compressed, 16-bit senone probs option.
 * 
 *  6-Apr-92  Fil Alleva (faa) at Carnegie-Mellon University
 *	- added SCVQAgcSet() and agcType.
 *
 * 08-Oct-91  Eric Thayer (eht) at Carnegie-Mellon University
 *	Created from system by Xuedong Huang
 * 22-Oct-91  Eric Thayer (eht) at Carnegie-Mellon University
 *	Installed some efficiency improvements to acoustic scoring
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

#include "s3types.h"
#include "logs3.h"
#include "s2_semi_mgau.h"
#include "err.h"
#include "kdtree.h"
#include "ckd_alloc.h"
#include "bio.h"
#include "vector.h"

#define MGAU_MIXW_VERSION	"1.0"   /* Sphinx-3 file format version for mixw */
#define MGAU_PARAM_VERSION	"1.0"   /* Sphinx-3 file format version for mean/var */

/* centered 5 frame difference of c[0]...c[12] with centered 9 frame
 * of c[1]...c[12].  Don't ask me why it was designed this way! */
#define DCEP_VECLEN	25
#define DCEP_LONGWEIGHT	0.5
#define POW_VECLEN	3       /* pow, diff pow, diff diff pow */

#define CEP_VECLEN	13
#define POW_VECLEN	3
#define CEP_SIZE	CEP_VECLEN
#define POW_SIZE	POW_VECLEN

#define NONE		-1
#define WORST_DIST	(int32)(0x80000000)
#define BTR		>
#define NEARER		>
#define WORSE		<
#define FARTHER		<

enum { CEP_FEAT = 0, DCEP_FEAT = 1, POW_FEAT = 2, DDCEP_FEAT = 3 };
static int32 fLenMap[S2_NUM_FEATURES] = {
    CEP_VECLEN, DCEP_VECLEN, POW_VECLEN, CEP_VECLEN
};

/*
 * In terms of already shifted and negated quantities (i.e. dealing with
 * 8-bit quantized values):
 */
#define LOG_ADD(p1,p2)	(logadd_tbl[(p1<<8)+(p2)])

extern unsigned char logadd_tbl[];

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/*
 * Compute senone scores.
 */
static int32 SCVQComputeScores(s2_semi_mgau_t * s, ascr_t * ascr);

/*
 * Optimization for various topN cases, PDF-size(#bits) cases of
 * SCVQCoomputeScores() and SCVQCoomputeScores_all().
 */
static int32 get_scores4_8b(s2_semi_mgau_t * s, ascr_t * ascr);
static int32 get_scores2_8b(s2_semi_mgau_t * s, ascr_t * ascr);
static int32 get_scores1_8b(s2_semi_mgau_t * s, ascr_t * ascr);
static int32 get_scores_8b(s2_semi_mgau_t * s, ascr_t * ascr);

static void
cepDist0(s2_semi_mgau_t * s, fast_gmm_t * fgmm, int32 frame, mfcc_t * z)
{
    register int32 i, j, k, cw;
    vqFeature_t *worst, *best, *topn, *cur;     /*, *src; */
    mean_t diff, sqdiff, compl; /* diff, diff^2, component likelihood */
    mfcc_t *obs;
    mean_t *mean;
    var_t *var = s->vars[(int32) CEP_FEAT];
    int32 *det = s->dets[(int32) CEP_FEAT], *detP;
    int32 *detE = det + S2_NUM_ALPHABET;
    var_t d;
    kd_tree_node_t *node;
    vqFeature_t vtmp;

    best = topn = s->f[CEP_FEAT];
    assert(z != NULL);
    assert(topn != NULL);
    memcpy(topn, s->lcfrm, sizeof(vqFeature_t) * s->topN);
    worst = topn + (s->topN - 1);
    if (s->kdtrees)
        node =
            eval_kd_tree(s->kdtrees[(int32) CEP_FEAT], z, s->kd_maxdepth);
    else
        node = NULL;
    /* initialize topn codewords to topn codewords from previous frame */
    for (i = 0; i < s->topN; i++) {
        cw = topn[i].codeword;
        mean = s->means[(int32) CEP_FEAT] + cw * CEP_VECLEN + 1;
        var = s->vars[(int32) CEP_FEAT] + cw * CEP_VECLEN + 1;
        d = det[cw];
        obs = z;
        for (j = 1; j < CEP_VECLEN; j++) {
            diff = *obs++ - *mean++;
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMSUB(d, compl);
            ++var;
        }
        topn[i].val.dist = (int32) d;
        if (i == 0)
            continue;
        vtmp = topn[i];
        for (j = i - 1; j >= 0 && (int32) d > topn[j].val.dist; j--) {
            topn[j + 1] = topn[j];
        }
        topn[j + 1] = vtmp;
    }
    if (frame % fgmm->downs->ds_ratio)
        return;
    if (node) {
        uint32 maxbbi = s->kd_maxbbi == -1 ? node->n_bbi : MIN(node->n_bbi,
                                                               s->
                                                               kd_maxbbi);
        for (i = 0; i < maxbbi; ++i) {
            cw = node->bbi[i];
            mean = s->means[(int32) CEP_FEAT] + cw * CEP_VECLEN + 1;
            var = s->vars[(int32) CEP_FEAT] + cw * CEP_VECLEN + 1;
            d = det[cw];
            obs = z;
            for (j = 1; (j < CEP_VECLEN) && (d >= worst->val.dist); j++) {
                diff = *obs++ - *mean++;
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, *var);
                d = GMMSUB(d, compl);
                ++var;
            }
            if (j < CEP_VECLEN)
                continue;
            if (d < worst->val.dist)
                continue;
            for (k = 0; k < s->topN; k++) {
                /* already there, so don't need to insert */
                if (topn[k].codeword == cw)
                    break;
            }
            if (k < s->topN)
                continue;       /* already there.  Don't insert */
            /* remaining code inserts codeword and dist in correct spot */
            for (cur = worst - 1; cur >= best && d >= cur->val.dist; --cur)
                memcpy(cur + 1, cur, sizeof(vqFeature_t));
            ++cur;
            cur->codeword = cw;
            cur->val.dist = (int32) d;
        }
    }
    else {
        mean = s->means[(int32) CEP_FEAT];
        var = s->vars[(int32) CEP_FEAT];
        for (detP = det, ++mean, ++var; detP < detE; detP++, ++mean, ++var) {
            d = *detP;
            obs = z;
            cw = detP - det;
            for (j = 1; (j < CEP_VECLEN) && (d >= worst->val.dist); j++) {
                diff = *obs++ - *mean++;
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, *var);
                d = GMMSUB(d, compl);
                ++var;
            }
            if (j < CEP_VECLEN) {
                /* terminated early, so not in topn */
                mean += (CEP_VECLEN - j);
                var += (CEP_VECLEN - j);
                continue;
            }
            if (d < worst->val.dist)
                continue;
            for (i = 0; i < s->topN; i++) {
                /* already there, so don't need to insert */
                if (topn[i].codeword == cw)
                    break;
            }
            if (i < s->topN)
                continue;       /* already there.  Don't insert */
            /* remaining code inserts codeword and dist in correct spot */
            for (cur = worst - 1; cur >= best && d >= cur->val.dist; --cur)
                memcpy(cur + 1, cur, sizeof(vqFeature_t));
            ++cur;
            cur->codeword = cw;
            cur->val.dist = (int32) d;
        }
    }

    memcpy(s->lcfrm, topn, sizeof(vqFeature_t) * s->topN);
}


static void
dcepDist0(s2_semi_mgau_t * s, fast_gmm_t * fgmm, int32 frame, mfcc_t * dzs,
          mfcc_t * dzl)
{
    register int32 i, j, k, cw;
    vqFeature_t *worst, *best, *topn, *cur;     /* , *src; */
    mean_t diff, sqdiff, compl;
    mfcc_t *obs1, *obs2;
    mean_t *mean;
    var_t *var;
    int32 *det = s->dets[(int32) DCEP_FEAT], *detP;
    int32 *detE = det + S2_NUM_ALPHABET;        /*, tmp; */
    var_t d;
    kd_tree_node_t *node;
    vqFeature_t vtmp;

    best = topn = s->f[DCEP_FEAT];
    assert(dzs != NULL);
    assert(dzl != NULL);
    assert(topn != NULL);

    if (s->kdtrees) {
        mfcc_t dceps[(CEP_VECLEN - 1) * 2];
        memcpy(dceps, dzs, (CEP_VECLEN - 1) * sizeof(*dzs));
        memcpy(dceps + CEP_VECLEN - 1, dzl,
               (CEP_VECLEN - 1) * sizeof(*dzl));
        node =
            eval_kd_tree(s->kdtrees[(int32) DCEP_FEAT], dceps,
                         s->kd_maxdepth);
    }
    else
        node = NULL;

    memcpy(topn, s->ldfrm, sizeof(vqFeature_t) * s->topN);
    worst = topn + (s->topN - 1);
    /* initialize topn codewords to topn codewords from previous frame */
    for (i = 0; i < s->topN; i++) {
        cw = topn[i].codeword;
        mean = s->means[(int32) DCEP_FEAT] + cw * DCEP_VECLEN + 1;
        var = s->vars[(int32) DCEP_FEAT] + cw * DCEP_VECLEN + 1;
        d = det[cw];
        obs1 = dzs;
        obs2 = dzl;
        for (j = 1; j < CEP_VECLEN; j++, obs1++, obs2++, mean++, var++) {
            diff = *obs1 - *mean;
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMSUB(d, compl);
            diff =
                MFCCMUL((*obs2 - mean[CEP_VECLEN - 1]), s->dcep80msWeight);
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, var[CEP_VECLEN - 1]);
            d = GMMSUB(d, compl);
        }
        topn[i].val.dist = (int32) d;
        if (i == 0)
            continue;
        vtmp = topn[i];
        for (j = i - 1; j >= 0 && (int32) d > topn[j].val.dist; j--) {
            topn[j + 1] = topn[j];
        }
        topn[j + 1] = vtmp;
    }
    if (frame % fgmm->downs->ds_ratio)
        return;
    if (node) {
        uint32 maxbbi = s->kd_maxbbi == -1 ? node->n_bbi : MIN(node->n_bbi,
                                                               s->
                                                               kd_maxbbi);
        for (i = 0; i < maxbbi; ++i) {
            cw = node->bbi[i];
            mean = s->means[(int32) DCEP_FEAT] + cw * DCEP_VECLEN + 1;
            var = s->vars[(int32) DCEP_FEAT] + cw * DCEP_VECLEN + 1;
            d = det[cw];
            obs1 = dzs;
            obs2 = dzl;
            for (j = 1; j < CEP_VECLEN && (d NEARER worst->val.dist);
                 j++, obs1++, obs2++, mean++, var++) {
                diff = *obs1 - *mean;
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, *var);
                d = GMMSUB(d, compl);
                diff =
                    MFCCMUL((*obs2 - mean[CEP_VECLEN - 1]),
                            s->dcep80msWeight);
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, var[CEP_VECLEN - 1]);
                d = GMMSUB(d, compl);
            }
            if (j < CEP_VECLEN)
                continue;
            if (d < worst->val.dist)
                continue;
            for (k = 0; k < s->topN; k++) {
                /* already there, so don't need to insert */
                if (topn[k].codeword == cw)
                    break;
            }
            if (k < s->topN)
                continue;       /* already there.  Don't insert */
            /* remaining code inserts codeword and dist in correct spot */
            for (cur = worst - 1;
                 cur >= best && (int32) d >= cur->val.dist; --cur)
                memcpy(cur + 1, cur, sizeof(vqFeature_t));
            ++cur;
            cur->codeword = cw;
            cur->val.dist = (int32) d;
        }
    }
    else {
        mean = s->means[(int32) DCEP_FEAT];
        var = s->vars[(int32) DCEP_FEAT];
        /* one distance value for each codeword */
        for (detP = det; detP < detE; detP++) {
            d = *detP;
            obs1 = dzs;         /* reset observed */
            obs2 = dzl;         /* reset observed */
            cw = detP - det;
            for (j = 1, ++mean, ++var;
                 (j < CEP_VECLEN) && (d NEARER worst->val.dist);
                 j++, obs1++, obs2++, mean++, var++) {
                diff = *obs1 - *mean;
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, *var);
                d = GMMSUB(d, compl);
                diff =
                    MFCCMUL((*obs2 - mean[CEP_VECLEN - 1]),
                            s->dcep80msWeight);
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, var[CEP_VECLEN - 1]);
                d = GMMSUB(d, compl);
            }
            mean += CEP_VECLEN - 1;
            var += CEP_VECLEN - 1;
            if (j < CEP_VECLEN) {
                mean += (CEP_VECLEN - j);
                var += (CEP_VECLEN - j);
                continue;
            }
            if (d < worst->val.dist)
                continue;
            for (i = 0; i < s->topN; i++) {
                /* already there, so don't need to insert */
                if (topn[i].codeword == cw)
                    break;
            }
            if (i < s->topN)
                continue;       /* already there.  Don't insert */
            /* remaining code inserts codeword and dist in correct spot */
            for (cur = worst - 1;
                 cur >= best && (int32) d >= cur->val.dist; --cur)
                memcpy(cur + 1, cur, sizeof(vqFeature_t));
            ++cur;
            cur->codeword = cw;
            cur->val.dist = (int32) d;
        }
    }
    memcpy(s->ldfrm, topn, sizeof(vqFeature_t) * s->topN);
}

static void
ddcepDist0(s2_semi_mgau_t * s, fast_gmm_t * fgmm, int32 frame, mfcc_t * z)
{
    register int32 i, j, k, cw;
    vqFeature_t *worst, *best, *topn, *cur;     /*, *src; */
    mean_t diff, sqdiff, compl;
    mfcc_t *obs;
    mean_t *mean;
    var_t *var;
    int32 *det = s->dets[(int32) DDCEP_FEAT];
    int32 *detE = det + S2_NUM_ALPHABET;
    int32 *detP;                /*, tmp; */
    var_t d;
    kd_tree_node_t *node;
    vqFeature_t vtmp;

    best = topn = s->f[DDCEP_FEAT];
    assert(z != NULL);
    assert(topn != NULL);

    if (s->kdtrees)
        node =
            eval_kd_tree(s->kdtrees[(int32) DDCEP_FEAT], z,
                         s->kd_maxdepth);
    else
        node = NULL;

    memcpy(topn, s->lxfrm, sizeof(vqFeature_t) * s->topN);
    worst = topn + (s->topN - 1);
    /* initialize topn codewords to topn codewords from previous frame */
    for (i = 0; i < s->topN; i++) {
        cw = topn[i].codeword;
        mean = s->means[(int32) DDCEP_FEAT] + cw * CEP_VECLEN + 1;
        var = s->vars[(int32) DDCEP_FEAT] + cw * CEP_VECLEN + 1;
        d = det[cw];
        obs = z;
        for (j = 1; j < CEP_VECLEN; j++) {
            diff = *obs++ - *mean++;
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMSUB(d, compl);
            ++var;
        }
        topn[i].val.dist = (int32) d;
        if (i == 0)
            continue;
        vtmp = topn[i];
        for (j = i - 1; j >= 0 && (int32) d > topn[j].val.dist; j--) {
            topn[j + 1] = topn[j];
        }
        topn[j + 1] = vtmp;
    }
    if (frame % fgmm->downs->ds_ratio)
        return;
    if (node) {
        uint32 maxbbi = s->kd_maxbbi == -1 ? node->n_bbi : MIN(node->n_bbi,
                                                               s->
                                                               kd_maxbbi);
        for (i = 0; i < maxbbi; ++i) {
            cw = node->bbi[i];
            mean = s->means[(int32) DDCEP_FEAT] + cw * CEP_VECLEN + 1;
            var = s->vars[(int32) DDCEP_FEAT] + cw * CEP_VECLEN + 1;
            d = det[cw];
            obs = z;
            for (j = 1; (j < CEP_VECLEN) && (d >= worst->val.dist); j++) {
                diff = *obs++ - *mean++;
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, *var);
                d = GMMSUB(d, compl);
                ++var;
            }
            if (j < CEP_VECLEN)
                continue;
            if (d < worst->val.dist)
                continue;
            for (k = 0; k < s->topN; k++) {
                /* already there, so don't need to insert */
                if (topn[k].codeword == cw)
                    break;
            }
            if (k < s->topN)
                continue;       /* already there.  Don't insert */
            /* remaining code inserts codeword and dist in correct spot */
            for (cur = worst - 1;
                 cur >= best && (int32) d >= cur->val.dist; --cur)
                memcpy(cur + 1, cur, sizeof(vqFeature_t));
            ++cur;
            cur->codeword = cw;
            cur->val.dist = (int32) d;
        }
    }
    else {
        mean = s->means[(int32) DDCEP_FEAT];
        var = s->vars[(int32) DDCEP_FEAT];
        for (detP = det, ++mean, ++var; detP < detE; detP++, ++mean, ++var) {
            d = *detP;
            obs = z;
            cw = detP - det;
            for (j = 1; (j < CEP_VECLEN) && (d >= worst->val.dist); j++) {
                diff = *obs++ - *mean++;
                sqdiff = MFCCMUL(diff, diff);
                compl = MFCCMUL(sqdiff, *var);
                d = GMMSUB(d, compl);
                ++var;
            }
            if (j < CEP_VECLEN) {
                /* terminated early, so not in topn */
                mean += (CEP_VECLEN - j);
                var += (CEP_VECLEN - j);
                continue;
            }
            if (d < worst->val.dist)
                continue;

            for (i = 0; i < s->topN; i++) {
                /* already there, so don't need to insert */
                if (topn[i].codeword == cw)
                    break;
            }
            if (i < s->topN)
                continue;       /* already there.  Don't insert */
            /* remaining code inserts codeword and dist in correct spot */
            for (cur = worst - 1;
                 cur >= best && (int32) d >= cur->val.dist; --cur)
                memcpy(cur + 1, cur, sizeof(vqFeature_t));
            ++cur;
            cur->codeword = cw;
            cur->val.dist = (int32) d;
        }
    }

    memcpy(s->lxfrm, topn, sizeof(vqFeature_t) * s->topN);
}

static void
powDist(s2_semi_mgau_t * s, fast_gmm_t * fgmm, int32 frame, mfcc_t * pz)
{
    register int32 i, j, cw;
    vqFeature_t *topn;
    var_t nextBest;
    var_t dist[S2_NUM_ALPHABET];
    mean_t diff, sqdiff, compl;
    var_t *dP, *dE = dist + S2_NUM_ALPHABET;
    mfcc_t *obs;
    mean_t *mean = s->means[(int32) POW_FEAT];
    var_t *var = s->vars[(int32) POW_FEAT];
    int32 *det = s->dets[(int32) POW_FEAT];
    var_t d;

    if (frame % fgmm->downs->ds_ratio)
        return;

    topn = s->f[POW_FEAT];
    assert(pz != NULL);
    assert(topn != NULL);

    /* one distance value for each codeword */
    for (dP = dist; dP < dE; dP++) {
        cw = dP - dist;
        d = 0;
        obs = pz;
        for (j = 0; j < POW_VECLEN; j++, obs++, mean++, var++) {
            diff = *obs - *mean;
            sqdiff = MFCCMUL(diff, diff);
            compl = MFCCMUL(sqdiff, *var);
            d = GMMADD(d, compl);
        }
        *dP = *det++ - d;
    }
    /* compute top N codewords */
    for (i = 0; i < s->topN; i++) {
        dP = dist;
        nextBest = *dP++;
        cw = 0;
        for (; dP < dE; dP++) {
            if (*dP NEARER nextBest) {
                nextBest = *dP;
                cw = dP - dist;
            }
        }
        topn[i].codeword = cw;
        topn[i].val.dist = (int32) nextBest;

        dist[cw] = WORST_DIST;
    }
}

/*
 * Compute senone scores for the active senones.
 */
int32
s2_semi_mgau_frame_eval(s2_semi_mgau_t * s,
                        ascr_t * ascr,
                        fast_gmm_t * fgmm, mfcc_t ** feat, int32 frame)
{
    int i, j;
    int32 tmp[S2_NUM_FEATURES];

    cepDist0(s, fgmm, frame, feat[CEP_FEAT]);

    dcepDist0(s, fgmm, frame, feat[DCEP_FEAT],
              feat[DCEP_FEAT] + CEP_VECLEN - 1);

    powDist(s, fgmm, frame, feat[POW_FEAT]);

    ddcepDist0(s, fgmm, frame, feat[DDCEP_FEAT]);

    /* normalize the topN feature scores */
    for (j = 0; j < S2_NUM_FEATURES; j++) {
        tmp[j] = s->f[j][0].val.score;
    }
    for (i = 1; i < s->topN; i++)
        for (j = 0; j < S2_NUM_FEATURES; j++) {
            tmp[j] = logmath_add(s->logmath, tmp[j], s->f[j][i].val.score);
        }
    for (i = 0; i < s->topN; i++)
        for (j = 0; j < S2_NUM_FEATURES; j++) {
            s->f[j][i].val.score -= tmp[j];
            if (s->f[j][i].val.score > 0)
                s->f[j][i].val.score = INT_MIN; /* tkharris++ */
            /* E_FATAL("**ERROR** VQ score= %d\n", f[j][i].val.score); */
        }


    SCVQComputeScores(s, ascr);
    return 0;
}


static int32
SCVQComputeScores(s2_semi_mgau_t * s, ascr_t * ascr)
{
    switch (s->topN) {
    case 4:
        return get_scores4_8b(s, ascr);
        break;
    case 2:
        return get_scores2_8b(s, ascr);
        break;
    case 1:
        return get_scores1_8b(s, ascr);
        break;
    default:
        return get_scores_8b(s, ascr);
        break;
    }
}

static int32
get_scores_8b(s2_semi_mgau_t * s, ascr_t * ascr)
{
    E_FATAL("get_scores_8b() not implemented\n");
    return 0;
}

/*
 * Like get_scores4, but uses OPDF_8B with FAST8B:
 *     LogProb(feature f, codeword c, senone s) =
 *         OPDF_8B[f]->prob[c][s]
 * Also, uses true 8-bit probs, so addition in logspace is an easy lookup.
 */
static int32
get_scores4_8b(s2_semi_mgau_t * s, ascr_t * ascr)
{
    register int32 j, n;
    int32 tmp1, tmp2;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
    int32 w0, w1, w2, w3;       /* weights */

    memset(ascr->senscr, 0, s->CdWdPDFMod * sizeof(*ascr->senscr));
    for (j = 0; j < S2_NUM_FEATURES; j++) {
        /* ptrs to senone prob ids */
        pid_cw0 = s->OPDF_8B[j][s->f[j][0].codeword];
        pid_cw1 = s->OPDF_8B[j][s->f[j][1].codeword];
        pid_cw2 = s->OPDF_8B[j][s->f[j][2].codeword];
        pid_cw3 = s->OPDF_8B[j][s->f[j][3].codeword];

        w0 = s->f[j][0].val.score;
        w1 = s->f[j][1].val.score;
        w2 = s->f[j][2].val.score;
        w3 = s->f[j][3].val.score;

        /* Floor w0..w3 to 256<<10 - 162k */
        if (w3 < -99000)
            w3 = -99000;
        if (w2 < -99000)
            w2 = -99000;
        if (w1 < -99000)
            w1 = -99000;
        if (w0 < -99000)
            w0 = -99000;        /* Condition should never be TRUE */

        /* Quantize */
        w3 = (511 - w3) >> 10;
        w2 = (511 - w2) >> 10;
        w1 = (511 - w1) >> 10;
        w0 = (511 - w0) >> 10;

        /* NOTE: Iterating over all senones will probably make this
         * slower than Sphinx2. */
        for (n = 0; n < ascr->n_sen; n++) {
            if (!ascr->sen_active[n])
                continue;

            tmp1 = pid_cw0[n] + w0;
            tmp2 = pid_cw1[n] + w1;
            tmp1 = LOG_ADD(tmp1, tmp2);
            tmp2 = pid_cw2[n] + w2;
            tmp1 = LOG_ADD(tmp1, tmp2);
            tmp2 = pid_cw3[n] + w3;
            tmp1 = LOG_ADD(tmp1, tmp2);

            ascr->senscr[n] -= tmp1 << 10;
        }
    }
    return 0;
}

static int32
get_scores2_8b(s2_semi_mgau_t * s, ascr_t * ascr)
{
    register int32 n;
    int32 tmp1, tmp2;
    unsigned char *pid_cw00, *pid_cw10, *pid_cw01, *pid_cw11,
        *pid_cw02, *pid_cw12, *pid_cw03, *pid_cw13;
    int32 w00, w10, w01, w11, w02, w12, w03, w13;

    memset(ascr->senscr, 0, s->CdWdPDFMod * sizeof(*ascr->senscr));
    /* ptrs to senone prob ids */
    pid_cw00 = s->OPDF_8B[0][s->f[0][0].codeword];
    pid_cw10 = s->OPDF_8B[0][s->f[0][1].codeword];
    pid_cw01 = s->OPDF_8B[1][s->f[1][0].codeword];
    pid_cw11 = s->OPDF_8B[1][s->f[1][1].codeword];
    pid_cw02 = s->OPDF_8B[2][s->f[2][0].codeword];
    pid_cw12 = s->OPDF_8B[2][s->f[2][1].codeword];
    pid_cw03 = s->OPDF_8B[3][s->f[3][0].codeword];
    pid_cw13 = s->OPDF_8B[3][s->f[3][1].codeword];

    w00 = s->f[0][0].val.score;
    w10 = s->f[0][1].val.score;
    w01 = s->f[1][0].val.score;
    w11 = s->f[1][1].val.score;
    w02 = s->f[2][0].val.score;
    w12 = s->f[2][1].val.score;
    w03 = s->f[3][0].val.score;
    w13 = s->f[3][1].val.score;

    /* Floor w0..w3 to 256<<10 - 162k */
    /* Condition should never be TRUE */
    if (w10 < -99000)
        w10 = -99000;
    if (w00 < -99000)
        w00 = -99000;
    if (w11 < -99000)
        w11 = -99000;
    if (w01 < -99000)
        w01 = -99000;
    if (w12 < -99000)
        w12 = -99000;
    if (w02 < -99000)
        w02 = -99000;
    if (w13 < -99000)
        w13 = -99000;
    if (w03 < -99000)
        w03 = -99000;

    /* Quantize */
    w10 = (511 - w10) >> 10;
    w00 = (511 - w00) >> 10;
    w11 = (511 - w11) >> 10;
    w01 = (511 - w01) >> 10;
    w12 = (511 - w12) >> 10;
    w02 = (511 - w02) >> 10;
    w13 = (511 - w13) >> 10;
    w03 = (511 - w03) >> 10;

    for (n = 0; n < ascr->n_sen; n++) {
        if (!ascr->sen_active[n])
            continue;
        tmp1 = pid_cw00[n] + w00;
        tmp2 = pid_cw10[n] + w10;
        tmp1 = LOG_ADD(tmp1, tmp2);
        ascr->senscr[n] -= tmp1 << 10;
        tmp1 = pid_cw01[n] + w01;
        tmp2 = pid_cw11[n] + w11;
        tmp1 = LOG_ADD(tmp1, tmp2);
        ascr->senscr[n] -= tmp1 << 10;
        tmp1 = pid_cw02[n] + w02;
        tmp2 = pid_cw12[n] + w12;
        tmp1 = LOG_ADD(tmp1, tmp2);
        ascr->senscr[n] -= tmp1 << 10;
        tmp1 = pid_cw03[n] + w03;
        tmp2 = pid_cw13[n] + w13;
        tmp1 = LOG_ADD(tmp1, tmp2);
        ascr->senscr[n] -= tmp1 << 10;
    }
    return 0;
}

static int32
get_scores1_8b(s2_semi_mgau_t * s, ascr_t * ascr)
{
    int32 j;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;

    /* Ptrs to senone prob values for the top codeword of all codebooks */
    pid_cw0 = s->OPDF_8B[0][s->f[0][0].codeword];
    pid_cw1 = s->OPDF_8B[1][s->f[1][0].codeword];
    pid_cw2 = s->OPDF_8B[2][s->f[2][0].codeword];
    pid_cw3 = s->OPDF_8B[3][s->f[3][0].codeword];

    for (j = 0; j < ascr->n_sen; j++) {
        if (!ascr->sen_active[j])
            continue;

        /* ** HACK!! ** <<10 hardwired!! */
        ascr->senscr[j] =
            -((pid_cw0[j] + pid_cw1[j] + pid_cw2[j] + pid_cw3[j]) << 10);
    }
    return 0;
}

int32
s2_semi_mgau_load_kdtree(s2_semi_mgau_t * s, const char *kdtree_path,
                         uint32 maxdepth, int32 maxbbi)
{
    if (read_kd_trees(kdtree_path, &s->kdtrees, &s->n_kdtrees,
                      maxdepth, maxbbi) == -1)
        E_FATAL("Failed to read kd-trees from %s\n", kdtree_path);
    if (s->n_kdtrees != S2_NUM_FEATURES)
        E_FATAL("Number of kd-trees != %d\n", S2_NUM_FEATURES);

    s->kd_maxdepth = maxdepth;
    s->kd_maxbbi = maxbbi;
    return 0;
}

static int32
read_dists_s3(s2_semi_mgau_t * s, char const *file_name, double SmoothMin)
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
    if (n_feat != 4)
        E_FATAL("#Features streams(%d) != 4\n", n_feat);
    if (n != n_sen * n_feat * n_comp) {
        E_FATAL
            ("%s: #float32s(%d) doesn't match header dimensions: %d x %d x %d\n",
             file_name, i, n_sen, n_feat, n_comp);
    }

    /* Quantized mixture weight arrays. */
    s->OPDF_8B[0] =
        (unsigned char **) ckd_calloc_2d(S2_NUM_ALPHABET, n_sen,
                                         sizeof(unsigned char));
    s->OPDF_8B[1] =
        (unsigned char **) ckd_calloc_2d(S2_NUM_ALPHABET, n_sen,
                                         sizeof(unsigned char));
    s->OPDF_8B[2] =
        (unsigned char **) ckd_calloc_2d(S2_NUM_ALPHABET, n_sen,
                                         sizeof(unsigned char));
    s->OPDF_8B[3] =
        (unsigned char **) ckd_calloc_2d(S2_NUM_ALPHABET, n_sen,
                                         sizeof(unsigned char));

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

            /* Convert to logs3, quantize, and transpose */
            for (c = 0; c < n_comp; c++) {
                int32 qscr;

                qscr = logs3(s->logmath, pdf[c]);
                /* ** HACK!! ** hardwired threshold!!! */
                if (qscr < -161900)
                    E_FATAL("**ERROR** Too low senone PDF value: %d\n",
                            qscr);
                qscr = (511 - qscr) >> 10;
                if ((qscr > 255) || (qscr < 0))
                    E_FATAL("scr(%d,%d,%d) = %d\n", f, c, i, qscr);
                s->OPDF_8B[f][c][i] = qscr;
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
int32
s3_read_mgau(const char *file_name, float32 ** cb)
{
    char tmp;
    FILE *fp;
    int32 i, blk, n;
    int32 n_mgau;
    int32 n_feat;
    int32 n_density;
    int32 veclen[4];
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
    if (n_mgau != 1)
        E_FATAL("%s: #codebooks (%d) != 1\n", file_name, n_mgau);

    /* #Features/codebook */
    if (bio_fread(&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
        E_FATAL("fread(%s) (#features) failed\n", file_name);
    if (n_feat != 4)
        E_FATAL("#Features streams(%d) != 4\n", n_feat);

    /* #Gaussian densities/feature in each codebook */
    if (bio_fread(&n_density, sizeof(int32), 1, fp, byteswap, &chksum) !=
        1)
        E_FATAL("fread(%s) (#density/codebook) failed\n", file_name);
    if (n_density != S2_NUM_ALPHABET)
        E_FATAL("%s: Number of densities per feature(%d) != %d\n",
                file_name, n_mgau, S2_NUM_ALPHABET);

    /* Vector length of feature stream */
    if (bio_fread(&veclen, sizeof(int32), 4, fp, byteswap, &chksum) != 4)
        E_FATAL("fread(%s) (feature vector-length) failed\n", file_name);
    for (i = 0, blk = 0; i < 4; ++i)
        blk += veclen[i];

    /* #Floats to follow; for the ENTIRE SET of CODEBOOKS */
    if (bio_fread(&n, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
        E_FATAL("fread(%s) (total #floats) failed\n", file_name);
    if (n != n_mgau * n_density * blk)
        E_FATAL
            ("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
             file_name, n, n_mgau, n_density, blk);

    /* This gets a bit tricky, as SCVQ expects C0 to exist in
     * the three cepstra streams even though it isn't used (this can
     * be fixed but isn't yet). */
    for (i = 0; i < 4; ++i) {
        int j;

        cb[i] =
            (float32 *) ckd_calloc(S2_NUM_ALPHABET * fLenMap[i],
                                   sizeof(float32));

        if (veclen[i] == fLenMap[i]) {  /* Not true except for POW_FEAT */
            if (bio_fread
                (cb[i], sizeof(float32), S2_NUM_ALPHABET * fLenMap[i], fp,
                 byteswap, &chksum) != S2_NUM_ALPHABET * fLenMap[i])
                E_FATAL("fread(%s, %d) of feat %d failed\n", file_name,
                        S2_NUM_ALPHABET * fLenMap[i], i);
        }
        else if (veclen[i] < fLenMap[i]) {
            for (j = 0; j < S2_NUM_ALPHABET; ++j) {
                if (bio_fread
                    ((cb[i] + j * fLenMap[i]) + (fLenMap[i] - veclen[i]),
                     sizeof(float32), veclen[i], fp, byteswap,
                     &chksum) != veclen[i])
                    E_FATAL("fread(%s, %d) in feat %d failed\n", file_name,
                            veclen[i], i);
            }
        }
        else
            E_FATAL("%s: feature %d length %d is not <= expected %d\n",
                    file_name, i, veclen[i], fLenMap[i]);
    }

    if (chksum_present)
        bio_verify_chksum(fp, byteswap, chksum);

    if (fread(&tmp, 1, 1, fp) == 1)
        E_FATAL("%s: More data than expected\n", file_name);

    fclose(fp);

    E_INFO("%d mixture Gaussians, %d components, veclen %d\n", n_mgau,
           n_density, blk);

    return n;
}

static int32
s3_precomp(mean_t ** means, var_t ** vars, int32 ** dets, float32 vFloor, logmath_t * logmath)
{
    int feat;
    float64 log_base = log(logmath_get_base(logmath));

    for (feat = 0; feat < 4; ++feat) {
        float32 *fmp;
        mean_t *mp;
        var_t *vp;
        int32 *dp, vecLen, i;

        vecLen = fLenMap[(int32) feat];
        fmp = (float32 *) means[feat];
        mp = means[feat];
        vp = vars[feat];
        dp = dets[feat];

        for (i = 0; i < S2_NUM_ALPHABET; ++i) {
            int32 d, j;

            d = 0;
            for (j = 0; j < vecLen; ++j, ++vp, ++mp, ++fmp) {
                float64 fvar;

#ifdef FIXED_POINT
                *mp = FLOAT2FIX(*fmp);
#endif
                /* Omit C0 for cepstral features (but not pow_feat!) */
                if (j == 0 && feat != POW_FEAT) {
                    *vp = 0;
                    continue;
                }

                /* Always do these pre-calculations in floating point */
                fvar = *(float32 *) vp;
                if (fvar < vFloor)
                    fvar = vFloor;
                d += logs3(logmath, 1 / sqrt(fvar * 2.0 * M_PI));
                *vp = (var_t) (1.0 / (2.0 * fvar * log_base));
            }
            *dp++ = d;
        }
    }
    return 0;
}

s2_semi_mgau_t *
s2_semi_mgau_init(const char *mean_path, const char *var_path,
                  float64 varfloor, const char *mixw_path,
                  float64 mixwfloor, int32 topn, logmath_t *logmath)
{
    s2_semi_mgau_t *s;
    int i;

    s = ckd_calloc(1, sizeof(*s));

    s->use20ms_diff_pow = FALSE;
    s->logmath = logmath;

    for (i = 0; i < S2_MAX_TOPN; i++) {
        s->lxfrm[i].val.dist = s->ldfrm[i].val.dist =
            s->lcfrm[i].val.dist = WORST_DIST;
        s->lxfrm[i].codeword = s->ldfrm[i].codeword =
            s->lcfrm[i].codeword = i;
    }

    /* Read means and variances. */
    if (s3_read_mgau(mean_path, s->means) < 0) {
        ckd_free(s);
        return NULL;
    }
    if (s3_read_mgau(var_path, s->vars) < 0) {
        ckd_free(s);
        return NULL;
    }
    /* Precompute means, variances, and determinants. */
    for (i = 0; i < 4; ++i)
        s->dets[i] = s->detArr + i * S2_NUM_ALPHABET;
    s3_precomp(s->means, s->vars, s->dets, varfloor, logmath);

    /* Read mixture weights (gives us CdWdPDFMod = number of
     * mixture weights per codeword, which is fixed at the number
     * of senones since we have only one codebook) */
    s->CdWdPDFMod = read_dists_s3(s, mixw_path, mixwfloor);
    s->topN = topn;

    return s;
}

void
s2_semi_mgau_free(s2_semi_mgau_t * s)
{
    /* FIXME: Need to free stuff. */
    ckd_free_2d(s->OPDF_8B[0]);
    ckd_free_2d(s->OPDF_8B[1]);
    ckd_free_2d(s->OPDF_8B[2]);
    ckd_free_2d(s->OPDF_8B[3]);

    ckd_free(s->means[0]);
    ckd_free(s->means[1]);
    ckd_free(s->means[2]);
    ckd_free(s->means[3]);

    ckd_free(s->vars[0]);
    ckd_free(s->vars[1]);
    ckd_free(s->vars[2]);
    ckd_free(s->vars[3]);

    ckd_free(s);
}
