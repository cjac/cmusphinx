/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * vq.c -- Vector quantization to quantize <mean,var> pairs of each dimension
 * 	of all codebooks put together.
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
 * 20-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


/*
 * To compile:
 *     cc -O2 -o vq -I$S3/include -I$S3/src -L$S3/lib/alpha vq.c -lutil -lio -lm
 * where, $S3 is the s3 root directory.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <libio/libio.h>


static timing_t *tmg;


static void vq_dump (float32 **cb, int32 vqsize, int32 veclen)
{
    FILE *fp;
    int32 c, i;

    if ((fp = fopen("vq.out", "w")) == NULL) {
	E_ERROR("fopen(vq.out,w) failed\n");
	return;
    }
    
    for (c = 0; c < vqsize; c++) {
	for (i = 0; i < veclen; i++)
	    fprintf (fp, "  %12.6e", cb[c][i]);
	fprintf (fp, "\n");
    }

    fclose (fp);
}


/*
 * Create an initial quantization of pt[0..n_pt-1][0..veclen-1] into
 * cb[0..vqsize-1][0..veclen-1], by choosing vqsize distinct points from pt[][].
 */
static void vq_init (float32 **pt, float32 **cb, int32 n_pt, int32 vqsize, int32 veclen)
{
    int32 i, j, k, d, dd, **tmp;
    
    /* Choose a stride into pt[] for selecting the initial codebook points */
    dd = n_pt / vqsize;
    dd -= (dd>>3);

    /* Choose a pseudo-random initial starting point */
    tmp = (int32 **) pt;
    for (i = 0; i < n_pt; i++) {
	for (d = 0; (d < veclen) && (tmp[i][d] == 0); d++);
	if (d < veclen)
	    break;
    }
    j = tmp[i][d] % 113;
    if (j < 0)
	j = -j;

    /* Select the vqsize initial VQ points */
    for (i = 0; i < vqsize; i++) {
	for (; j < n_pt; j += dd) {
	    /* Make sure the next point considered is not alredy in the VQ list */
	    for (k = 0; k < i; k++) {
		for (d = 0; d < veclen; d++) {
		    if (cb[k][d] != pt[j][d])
			break;
		}
		if (d >= veclen)	/* This point already in VQ list */
		    break;
	    }
	    if (k >= i)			/* This point not in VQ list; can be added */
		break;
	}
	
	assert (j < n_pt);
	
	for (d = 0; d < veclen; d++)
	    cb[i][d] = pt[j][d];
	
	j += dd;
    }
    
    E_INFO("Initialization finished at %d out of %d\n", j, n_pt);
}


/*
 * Euclidean distance (squared) between two vectors.
 */
static float64 vecdist (float32 *v1, float32 *v2, int32 veclen)
{
    float64 d;
    int32 i;
    
    d = 0.0;
    for (i = 0; i < veclen; i++)
	d += (v1[i] - v2[i]) * (v1[i] - v2[i]);
    
    return d;
}


/*
 * Quantize pt[0..n_pt-1][0..veclen-1] into cb[0..vqsize-1][0..veclen-1] (where
 * vqsize < n_pt, presumably).  Do this with the following iterative procedure:
 *     1. Choose an initial VQ codebook by selecting vqsize random points from pt.
 *     2. Map each point in pt to the "nearest" codebook entry (currently based on
 * 	  Euclidean distance.
 *     3. Re-estimate each VQ entry by taking the centroid of all pt entries mapped
 * 	  to it in step 2.
 *     4. Repeat steps 2 and 3 until the "total error stabilizes".
 * In the end, replace each point in pt with the nearest VQ value.
 * Return value: final total error.
 */
static float64 vq (float32 **pt, float32 **cb, int32 n_pt, int32 vqsize, int32 veclen)
{
    int32 p, c, i, iter, bestc, *pt2cb, *n_newcb;
    float64 d, bestdist, err, prev_err;
    float32 **newcb;
    
    E_INFO("Clustering %d points into %d\n", n_pt, vqsize);

    /* Allocate some temporary variables */
    pt2cb = (int32 *) ckd_calloc (n_pt, sizeof(int32));
    newcb = (float32 **)ckd_calloc_2d (vqsize, veclen, sizeof(float32));
    n_newcb = (int32 *) ckd_calloc (vqsize, sizeof(int32));

    /* Choose an initial codebook */
    vq_init (pt, cb, n_pt, vqsize, veclen);
    
    for (iter = 0;; iter++) {
	timing_start (tmg);

	/* Map each point to closest codebook entry (using Euclidean distance metric) */
	err = 0.0;
	for (p = 0; p < n_pt; p++) {
	    bestdist = 1e+200;
	    for (c = 0; c < vqsize; c++) {
		d = vecdist (pt[p], cb[c], veclen);
		if (d < bestdist) {
		    bestdist = d;
		    bestc = c;
		}
	    }
	    
	    pt2cb[p] = bestc;
	    err += bestdist;
	}
	
	/* Update codebook entries with centroid of mapped points */
	for (c = 0; c < vqsize; c++) {
	    for (i = 0; i < veclen; i++)
		newcb[c][i] = 0.0;
	    n_newcb[c] = 0;
	}
	for (p = 0; p < n_pt; p++) {
	    c = pt2cb[p];
	    for (i = 0; i < veclen; i++)
		newcb[c][i] += pt[p][i];
	    n_newcb[c]++;
	}
	for (c = 0; c < vqsize; c++) {
	    if (n_newcb[c] == 0)
		E_ERROR("Nothing mapped to codebook entry %d; entry not updated\n", c);
	    else {
		float64 t;
		
		t = 1.0 / n_newcb[c];
		for (i = 0; i < veclen; i++)
		    cb[c][i] = newcb[c][i] * t;
	    }
	}

	timing_stop (tmg);
	
	E_INFO("%4d: Error = %e, %.2f sec CPU, %.2f sec elapsed\n",
	       iter, err, tmg->t_cpu, tmg->t_elapsed);

	timing_reset (tmg);

	/* Check if VQ codebook converged */
	if (iter > 10) {
	    if ((err == 0.0) || ((prev_err - err)/prev_err < 0.002))
		break;
	}
	prev_err = err;
    }

    /* Replace points with nearest VQ entries created */
    for (p = 0; p < n_pt; p++) {
	c = pt2cb[p];
	for (i = 0; i < veclen; i++)
	    pt[p][i] = cb[c][i];
    }
    
    ckd_free (pt2cb);
    ckd_free_2d ((void **) newcb);
    ckd_free (n_newcb);

    return err;
}


/*
 * gauden_param_read taken from s3/src/libfbs.
 * gauden_param_write modelled after gauden_param_read.
 */

#define GAUDEN_PARAM_VERSION	"0.1"

static int32 gauden_param_read(float32 *****out_param,
			       int32 *out_n_mgau,
			       int32 *out_n_feat,
			       int32 *out_n_density,
			       int32 **out_veclen,
			       const char *file_name)
{
    char version[1024], tmp;
    FILE *fp;
    int32 i, j, k, l, blk, n;
    int32 n_mgau;
    int32 n_feat;
    int32 n_density;
    int32 *veclen;
    int32 needs_reorder;
    float32 ****out;
    float32 *buf;

    E_INFO("Reading mixture gaussian parameter: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);

    if (fscanf(fp, "%s", version) != 1)
	E_FATAL("Unable to read version id\n");
    
    if (strcmp(version, GAUDEN_PARAM_VERSION) != 0)
	E_FATAL("Version mismatch: %s, expecting %s\n", version, GAUDEN_PARAM_VERSION);
    
    if (bcomment_read(fp) != S3_SUCCESS)
	E_FATAL("bcomment_read() failed\n");
    
    if ((needs_reorder = swap_check(fp)) < 0)
	E_FATAL("swap_check() failed\n");

    /* #Codebooks */
    if (fread_retry(&n_mgau, sizeof(uint32), 1, fp) != 1)
	E_FATAL("Error reading #codebooks\n");
    if (needs_reorder) {
	SWAP_INT32(&n_mgau);
    }
    *out_n_mgau = n_mgau;

    /* #Features/codebook */
    if (fread_retry(&n_feat, sizeof(uint32), 1, fp) != 1)
	E_FATAL("Error reading #features/codebook\n");
    if (needs_reorder) {
	SWAP_INT32(&n_feat);
    }
    *out_n_feat = n_feat;

    /* #Gaussian densities/feature in each codebook */
    if (fread_retry(&n_density, sizeof(uint32), 1, fp) != 1)
	E_FATAL("Error reading #densities/codebook-feature\n");
    if (needs_reorder) {
	SWAP_INT32(&n_density);
    }
    *out_n_density = n_density;
    
    /* #Dimensions in each feature stream */
    veclen = ckd_calloc(n_feat, sizeof(uint32));
    *out_veclen = veclen;
    if (fread_retry(veclen, sizeof(uint32), n_feat, fp) != n_feat)
	E_FATAL("Error reading feature vector lengths\n");
    if (needs_reorder) {
	for (i = 0; i < n_feat; i++)
	    SWAP_INT32(&veclen[i]);
    }

    /* blk = total vector length of all feature streams */
    for (i = 0, blk = 0; i < n_feat; i++)
	blk += veclen[i];

    /* #Floats to follow; for the ENTIRE SET of CODEBOOKS */
    if (fread_retry(&n, sizeof(uint32), 1, fp) != 1)
	E_FATAL("Error reading #floats\n");
    if (needs_reorder) {
	SWAP_INT32(&n);
    }
    assert(n == n_mgau * n_density * blk);
    
    /* Allocate memory for mixture gaussian densities */
    out = (float32 ****) ckd_calloc_3d (n_mgau, n_feat, n_density,
					sizeof(float32 *));
    buf = (float32 *) ckd_calloc (n, sizeof(float));
    for (i = 0, l = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		out[i][j][k] = &buf[l];

		l += veclen[j];
	    }
	}
    }

    /* Read mixture gaussian densities data */
    if (fread_retry (buf, sizeof(float32), n, fp) != n)
	E_FATAL("Error reading gaussian data\n");
    if (needs_reorder)
	for (i = 0; i < n; i++)
	    SWAP_FLOAT32(&buf[i]);
    
    E_INFO("%d codebook, %d feature, size",
	   n_mgau, n_feat);
    for (i = 0; i < n_feat; i++)
	printf (" %dx%d", n_density, veclen[i]);
    printf ("\n");

    if (fread (&tmp, 1, 1, fp) == 1)
	E_WARN("Non-empty file beyond end of data\n");

    *out_param = out;
    
    fclose(fp);

    return 0;
}


static int32 gauden_param_write(float32 ****param,
				int32 n_mgau,
				int32 n_feat,
				int32 n_density,
				int32 *veclen,
				const char *file_name)
{
    FILE *fp;
    int32 i, k, blk;

    E_INFO("Writing mixture gaussian parameter: %s\n", file_name);
    
    if ((fp = fopen(file_name, "wb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,wb) failed\n", file_name);

    fprintf (fp, "%s\n", GAUDEN_PARAM_VERSION);
    fprintf (fp, "*end_comment*\n");
    k = (int32)BYTE_ORDER_MAGIC;
    fwrite (&k, sizeof(int32), 1, fp);

    /* #Codebooks */
    k = n_mgau;
    fwrite (&k, sizeof(int32), 1, fp);

    /* #Features/codebook */
    k = n_feat;
    fwrite (&k, sizeof(int32), 1, fp);

    /* #Gaussian densities/feature in each codebook */
    k = n_density;
    fwrite (&k, sizeof(int32), 1, fp);
    
    /* #Dimensions in each feature stream */
    fwrite (veclen, sizeof(int32), n_feat, fp);

    /* blk = total vector length of all feature streams */
    for (i = 0, blk = 0; i < n_feat; i++)
	blk += veclen[i];

    /* #Floats to follow; for the ENTIRE SET of CODEBOOKS */
    k = n_mgau * n_density * blk;
    fwrite (&k, sizeof(int32), 1, fp);

    /* Read mixture gaussian densities data */
    fwrite (&(param[0][0][0][0]), sizeof(float32), k, fp);
    
    fclose(fp);

    return 0;
}


/*
 * Read mean and variance S3-format files and cluster them as follows:
 *     assume single feature vector
 *     for each dimension of feature vector {
 *         create list of <mean,var> pairs in the entire codebook set (for that dimension);
 *             // HACK!!  0 vectors omitted from the above list
 *         VQ cluster this list into the given number of vqsize points;
 *         replace codebook entries with nearest clustered values (for that dimension);
 *             // HACK!!  0 vectors remain untouched
 *     }
 *     write remapped codebooks to files "mean-vq" and "var-vq";
 */
main (int32 argc, char *argv[])
{
    float32 ****mean, ****var;
    int32 n_cb, n_feat, n_den, *featlen;
    int32 i, j, c, w, dim, p, n_pt, vqsize;
    float32 **pt, **cb;
    float64 err;
    FILE *fp;

    if ((argc < 4) || (sscanf (argv[3], "%d", &vqsize) != 1))
	E_FATAL("Usage: %s meanfile varfile vqsize\n", argv[0]);
    
    if (argc > 4) {
	if ((fp = fopen(argv[4], "w")) != NULL) {
	    *stdout = *fp;
	    *stderr = *fp;
	} else
	    E_ERROR("fopen(%s,w) failed\n", argv[4]);
    }

    gauden_param_read (&mean, &n_cb, &n_feat, &n_den, &featlen, argv[1]);
    E_INFO("%s: %d x %d x %d ", argv[1], n_cb, n_feat, n_den);
    for (i = 0; i < n_feat; i++)
	printf (" %d", featlen[i]);
    printf ("\n");
    gauden_param_read (&var, &n_cb, &n_feat, &n_den, &featlen, argv[2]);
    E_INFO("%s: %d x %d x %d ", argv[2], n_cb, n_feat, n_den);
    for (i = 0; i < n_feat; i++)
	printf (" %d", featlen[i]);
    printf ("\n");

    assert (n_feat == 1);

    n_pt = n_cb * n_den;
    pt = (float32 **) ckd_calloc_2d (n_pt, 2, sizeof(float32));
    cb = (float32 **) ckd_calloc_2d (vqsize, 2, sizeof(float32));

    tmg = timing_new ();

    /* VQ each column of mean,var pair (assuming just one feature vector) */
    for (dim = 0; dim < featlen[0]; dim++) {
	j = 0;
	for (c = 0; c < n_cb; c++) {
	    for (w = 0; w < n_den; w++) {
		if ((mean[c][0][w][dim] != 0.0) || (var[c][0][w][dim] != 0.0)) {
		    pt[j][0] = mean[c][0][w][dim];
		    pt[j][1] = var[c][0][w][dim];
		    j++;
		}
	    }
	}

	err = vq (pt, cb, j, vqsize, 2);
	vq_dump (cb, vqsize, 2);

	j = 0;
	for (c = 0; c < n_cb; c++) {
	    for (w = 0; w < n_den; w++) {
		if ((mean[c][0][w][dim] != 0.0) || (var[c][0][w][dim] != 0.0)) {
		    mean[c][0][w][dim] = pt[j][0];
		    var[c][0][w][dim] = pt[j][1];
		    j++;
		}
	    }
	}
	E_INFO("%d values quantized for dimension %d; error = %e\n", j, dim, err);
    }
    fflush (fp);

    gauden_param_write (mean, n_cb, n_feat, n_den, featlen, "mean-vq");
    gauden_param_write (var, n_cb, n_feat, n_den, featlen, "var-vq");

    fflush (fp);

    exit(0);
}
