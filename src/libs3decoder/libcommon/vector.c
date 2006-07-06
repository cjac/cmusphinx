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
 * vector.c
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
 * $Log$
 * Revision 1.19  2006/04/19  18:59:38  arthchan2003
 * Check whether nan appear explicitly in the acoustic models and vector.c, this will make gausubvq works. This shouldn't break the VQ generated in the past.  Because if nan exists in any of the past models, gausubvq will give abnormal results and some of us should have observed.
 * 
 * Revision 1.18  2006/02/22 20:35:17  arthchan2003
 * Merge from branch SPHINX3_5_2_RCI_IRII_BRANCH:
 * 1, Not allocated lrd in vector.c because its only consumer subvq.c has done it. 2, Fixed dox-doc.
 *
 *
 * Revision 1.16.4.2  2005/10/17 04:47:53  arthchan2003
 * Not allocate lrd in vector.c because subvq(its only consumer) will do it separately.
 *
 * Revision 1.16.4.1  2005/07/05 06:47:08  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.17  2005/07/02 03:48:46  egouvea
 * Removed condition compilation on WIN32 for included time.h. Not regression tested since this affects only Windows
 *
 * Revision 1.16  2005/06/21 21:02:05  arthchan2003
 * Please ignore the last log. vector.c has now fix doxygen-style documents and fixed $ keyword.
 *
 * Revision 1.15  2005/06/21 21:00:44  arthchan2003
 * Add a statistics inventory structure, it takes care of 1, counters, 2, timers. Interfaces are provided to allow convenient clearing and updating of structures
 *
 * Revision 1.2  2005/03/30 01:08:38  archan
 * codebase-wide update. Performed an age-old trick: Adding $Log into all .c and .h files. This will make sure the commit message be preprended into a file.
 *
 * 10-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added vector_accum(), vector_vqlabel(), and vector_vqgen().
 * 
 * 09-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added vector_is_zero(), vector_cmp(), and vector_dist_eucl().
 * 		Changed the name vector_dist_eval to vector_dist_maha.
 * 
 * 07-Oct-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added distance computation related functions.
 * 
 * 12-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Copied from Eric Thayer.
 */


#include "vector.h"
#include "logs3.h"
#include "s3types.h"
#include <math.h>
#include <float.h>


#include <time.h>               /* RAH */

float64
vector_sum_norm(float32 * vec, int32 len)
{
    float64 sum, f;
    int32 i;

    sum = 0.0;
    for (i = 0; i < len; i++)
        sum += vec[i];

    if (sum != 0.0) {
        f = 1.0 / sum;
        for (i = 0; i < len; i++)
            /*      vec[i] *=  f;         *//*  */
            /* The compiler was complaining about the above line, to make sure we don't lose accuracy, use this  */
            vec[i] = (float32) ((float64) vec[i] * (float64) f);
    }

    return sum;
}


void
vector_floor(float32 * vec, int32 len, float64 flr)
{
    int32 i;

    for (i = 0; i < len; i++)
        if (vec[i] < flr)
            vec[i] = (float32) flr;
}


void
vector_nz_floor(float32 * vec, int32 len, float64 flr)
{
    int32 i;

    for (i = 0; i < len; i++)
        if ((vec[i] != 0.0) && (vec[i] < flr))
            vec[i] = (float32) flr;
}


void
vector_print(FILE * fp, vector_t v, int32 dim)
{
    int32 i;

    for (i = 0; i < dim; i++)
        fprintf(fp, " %11.4e", v[i]);
    fprintf(fp, "\n");
    fflush(fp);
}

int
vector_normalize(vector_t v, uint32 dim)
{
    uint32 i;
    float64 s;
    int ret_val = S3_SUCCESS;

    for (i = 0, s = 0; i < dim; i++)
        s += v[i];

    if (s != 0) {
        for (i = 0; i < dim; i++)
            v[i] /= (float) s;
    }
    else {
        ret_val = S3_WARNING;
    }

    return ret_val;
}

int32
vector_is_zero(float32 * vec, int32 len)
{
    int32 i;

    for (i = 0; (i < len) && (vec[i] == 0.0); i++);
    return (i == len);          /* TRUE iff all mean values are 0.0 */
}

int32
vector_is_nan(float32 * vec, int32 len)
{
    int32 i;
    for (i = 0; i < len; i++) {
#if defined(WIN32)
        return _isnan(vec[i]);
#else
        return isnan(vec[i]);
#endif
    }
}



int32
vector_maxcomp_int32(int32 * val, int32 len)
{
    int32 i, bi;

    bi = 0;
    for (i = 1; i < len; i++) {
        if (val[i] > val[bi])
            bi = i;
    }
    return bi;
}


int32
vector_mincomp_int32(int32 * val, int32 len)
{
    int32 i, bi;

    bi = 0;
    for (i = 1; i < len; i++) {
        if (val[i] < val[bi])
            bi = i;
    }
    return bi;
}


int32
vector_maxcomp_float32(float32 * val, int32 len)
{
    int32 i, bi;

    bi = 0;
    for (i = 1; i < len; i++) {
        if (val[i] > val[bi])
            bi = i;
    }
    return bi;
}


int32
vector_mincomp_float32(float32 * val, int32 len)
{
    int32 i, bi;

    bi = 0;
    for (i = 1; i < len; i++) {
        if (val[i] < val[bi])
            bi = i;
    }
    return bi;
}


void
vector_accum(float32 * dst, float32 * src, int32 len)
{
    int32 i;

    for (i = 0; i < len; i++)
        dst[i] += src[i];
}


int32
vector_cmp(float32 * v1, float32 * v2, int32 len)
{
    int32 i;

    for (i = 0; i < len; i++) {
        if (v1[i] < v2[i])
            return -1;
        if (v1[i] > v2[i])
            return 1;
    }

    return 0;
}


int32
vector_mean(float32 * mean, float32 ** data, int32 n_vec, int32 n_dim)
{
    int32 i, j;
    float64 f;

    assert((n_vec > 0) && (n_dim > 0));

    for (i = 0; i < n_dim; i++)
        mean[i] = 0.0;

    for (i = 0; i < n_vec; i++) {
        for (j = 0; j < n_dim; j++)
            mean[j] += data[i][j];
    }

    f = 1.0 / (float64) n_vec;
    for (i = 0; i < n_dim; i++)
        mean[i] *= (float32) f;

    return 0;
}


float64
vector_dist_eucl(float32 * v1, float32 * v2, int32 len)
{
    float64 d;
    int32 i;

    d = 0.0;
    for (i = 0; i < len; i++)
        d += (v1[i] - v2[i]) * (v1[i] - v2[i]);

    return d;
}


float64
vector_maha_precomp(float32 * var, int32 len)
{
    float64 det;
    int32 i;

    for (det = (float64) 0.0, i = 0; i < len; i++) {    /* log(1.0/prod(var[i])) */
        det -= (float64) (log(var[i]));
        var[i] = (float32) (1.0 / (var[i] * 2.0));
    }
    det -= log(2.0 * PI) * len;

    return (det * 0.5);         /* sqrt */
}


float64
vector_dist_maha(float32 * vec, float32 * mean, float32 * varinv,
                 float64 loginvdet, int32 len)
{
    float64 dist, diff;
    int32 i;

    dist = loginvdet;
    for (i = 0; i < len; i++) {
        diff = (vec[i] - mean[i]);
        dist -= diff * diff * varinv[i];
    }

    return dist;
}


int32
vector_vqlabel(float32 * vec, float32 ** mean, int32 rows, int32 cols,
               float64 * sqerr)
{
    int32 i, besti;
    float64 d, bestd;

    assert((rows > 0) && (cols > 0));

    bestd = vector_dist_eucl(mean[0], vec, cols);
    besti = 0;

    for (i = 1; i < rows; i++) {
        d = vector_dist_eucl(mean[i], vec, cols);
        if (bestd > d) {
            bestd = d;
            besti = i;
        }
    }

    if (sqerr)
        *sqerr = bestd;

    return besti;
}


float64
vector_vqgen(float32 ** data, int32 rows, int32 cols, int32 vqrows,
             float64 epsilon, int32 maxiter,
             float32 ** mean, int32 * map, int32 seed)
{
    int32 i, j, r, it;
    float64 sqerr, prev_sqerr = 0, t;
    bitvec_t sel;
    int32 *count;
    float32 *gmean;
    ptmr_t tm;

    assert((rows >= vqrows) && (maxiter >= 0) && (epsilon > 0.0));

    sel = bitvec_alloc(rows);

    ptmr_init(&tm);
    ptmr_start(&tm);


    /* Pick a random initial set of centroids */

    if (seed < 0) {
        E_INFO
            ("You are using the internal mechanism of vector_vqgen to decide the seed.  \n");
        s3_rand_seed((unsigned) time(NULL));

    }
    else {

        /*ARCHAN: DON'T delete this line! This will always give the
           experimenter the seed they were using in their experiments. This
           make the result be repeatable */

        E_INFO("You are using %d as the seed \n", seed);
        s3_rand_seed(seed);
    }


    for (i = 0; i < vqrows; i++) {
        /* Find r = a random, previously unselected row from the input */

        r = (s3_rand_int31() & (int32) 0x7fffffff) % rows;

        /*        r = (rand() & (int32)0x7fffffff) % rows; */

        while (bitvec_is_set(sel, r)) { /* BUG: possible infinite loop!! */
            if (++r >= rows)
                r = 0;
        }
        bitvec_set(sel, r);

        memcpy((void *) (mean[i]), (void *) (data[r]),
               cols * sizeof(float32));
        /* BUG: What if two randomly selected rows are identical in content?? */
    }
    bitvec_free(sel);

    count = (int32 *) ckd_calloc(vqrows, sizeof(int32));

    /* In k-means, unmapped means in any iteration are a problem.  Replace them with gmean */
    gmean = (float32 *) ckd_calloc(cols, sizeof(float32));
    vector_mean(gmean, mean, vqrows, cols);

    for (it = 0;; it++) {       /* Iterations of k-means algorithm */
        /* Find the current data->mean mappings (labels) */
        sqerr = 0.0;
        for (i = 0; i < rows; i++) {
            map[i] = vector_vqlabel(data[i], mean, vqrows, cols, &t);
            sqerr += t;
        }
        ptmr_stop(&tm);

        if (it == 0)
            E_INFO("Iter %4d: %.1fs CPU; sqerr= %e\n", it, tm.t_cpu,
                   sqerr);
        else
            E_INFO("Iter %4d: %.1fs CPU; sqerr= %e; delta= %e\n",
                   it, tm.t_cpu, sqerr, (prev_sqerr - sqerr) / prev_sqerr);

        /* Check if exit condition satisfied */
        if ((sqerr == 0.0) || (it >= maxiter - 1) ||
            ((it > 0) && (((prev_sqerr - sqerr) / prev_sqerr) < epsilon)))
            break;
        prev_sqerr = sqerr;

        ptmr_start(&tm);

        /* Update (reestimate) means */
        for (i = 0; i < vqrows; i++) {
            for (j = 0; j < cols; j++)
                mean[i][j] = 0.0;
            count[i] = 0;
        }
        for (i = 0; i < rows; i++) {
            vector_accum(mean[map[i]], data[i], cols);
            count[map[i]]++;
        }
        for (i = 0; i < vqrows; i++) {
            if (count[i] > 1) {
                t = 1.0 / (float64) (count[i]);
                for (j = 0; j < cols; j++)
                    /*              mean[i][j] *= t; *//* RAH, compiler was complaining about this,  */
                    mean[i][j] = (float32) ((float64) mean[i][j] * (float64) t);        /*  */
            }
            else if (count[i] == 0) {
                E_ERROR("Iter %d: mean[%d] unmapped\n", it, i);
                memcpy(mean[i], gmean, cols * sizeof(float32));
            }
        }
    }

    ckd_free(count);
    ckd_free(gmean);

    return sqerr;
}


float64
vector_pdf_entropy(float32 * p, int32 len)
{
    float64 sum;
    int32 i;

    sum = 0.0;
    for (i = 0; i < len; i++) {
        if (p[i] > 0.0)
            sum -= p[i] * log(p[i]);
    }
    sum /= log(2.0);

    return sum;
}


float64
vector_pdf_cross_entropy(float32 * p1, float32 * p2, int32 len)
{
    float64 sum;
    int32 i;

    sum = 0.0;
    for (i = 0; i < len; i++) {
        if (p2[i] > 0.0)
            sum -= p1[i] * log(p2[i]);
    }
    sum /= log(2.0);

    return sum;
}


void
vector_gautbl_alloc(vector_gautbl_t * gautbl, int32 n_gau, int32 veclen)
{
    gautbl->n_gau = n_gau;
    gautbl->veclen = veclen;
    gautbl->mean =
        (float32 **) ckd_calloc_2d(n_gau, veclen, sizeof(float32));
    gautbl->var =
        (float32 **) ckd_calloc_2d(n_gau, veclen, sizeof(float32));
    /*    gautbl->lrd = (float32 *) ckd_calloc (n_gau, sizeof(float32)); */
    gautbl->distfloor = logs3_to_log(S3_LOGPROB_ZERO);
}


void
vector_gautbl_free(vector_gautbl_t * gautbl)
{
    if (gautbl->mean != NULL) {
        ckd_free_2d((void **) gautbl->mean);
    }
    if (gautbl->var != NULL) {
        ckd_free_2d((void **) gautbl->var);
    }
    if (gautbl->lrd != NULL) {
        ckd_free((void *) gautbl->lrd);
    }

}


void
vector_gautbl_var_floor(vector_gautbl_t * gautbl, float64 floor)
{
    int32 g;

    for (g = 0; g < gautbl->n_gau; g++)
        vector_floor(gautbl->var[g], gautbl->veclen, floor);
}


void
vector_gautbl_maha_precomp(vector_gautbl_t * gautbl)
{
    int32 g;

    for (g = 0; g < gautbl->n_gau; g++)
        gautbl->lrd[g] =
            (float32) vector_maha_precomp(gautbl->var[g], gautbl->veclen);
}


void
vector_gautbl_eval_logs3(vector_gautbl_t * gautbl,
                         int32 offset,
                         int32 count, float32 * x, int32 * score)
{
    int32 i, r;
    float64 f;
    int32 end, veclen;
    float32 *m1, *m2, *v1, *v2;
    float64 dval1, dval2, diff1, diff2;

    f = log_to_logs3_factor();

    /* Interleave evaluation of two vectors at a time for speed on pipelined machines */
    end = offset + count;
    veclen = gautbl->veclen;

    for (r = offset; r < end - 1; r += 2) {
        m1 = gautbl->mean[r];
        m2 = gautbl->mean[r + 1];
        v1 = gautbl->var[r];
        v2 = gautbl->var[r + 1];
        dval1 = gautbl->lrd[r];
        dval2 = gautbl->lrd[r + 1];

        for (i = 0; i < veclen; i++) {
            diff1 = x[i] - m1[i];
            dval1 -= diff1 * diff1 * v1[i];
            diff2 = x[i] - m2[i];
            dval2 -= diff2 * diff2 * v2[i];
        }

        if (dval1 < gautbl->distfloor)
            dval1 = gautbl->distfloor;
        if (dval2 < gautbl->distfloor)
            dval2 = gautbl->distfloor;

        score[r] = (int32) (f * dval1);
        score[r + 1] = (int32) (f * dval2);
    }

    if (r < end) {
        m1 = gautbl->mean[r];
        v1 = gautbl->var[r];
        dval1 = gautbl->lrd[r];

        for (i = 0; i < veclen; i++) {
            diff1 = x[i] - m1[i];
            dval1 -= diff1 * diff1 * v1[i];
        }

        if (dval1 < gautbl->distfloor)
            dval1 = gautbl->distfloor;

        score[r] = (int32) (f * dval1);
    }
}
