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
 * cb.c -- Printing and looking at S3-format codebooks in various ways.
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
 * 21-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 * 
 */


/*
 * To compile:
 *     cc -O2 -o cb -I$S3/include -I$S3/src -L$S3/lib/alpha cb.c -lutil -lio
 * where, $S3 is the s3 root directory.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <libio/libio.h>


/*
 * gauden_param_read taken from s3/src/libfbs.
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

    fprintf (stderr, "Reading mixture gaussian parameter: %s\n", file_name);
    
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

    if (fread (&tmp, 1, 1, fp) == 1)
	E_WARN("Non-empty file beyond end of data\n");

    *out_param = out;
    
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
    FILE *fp;
    int32 i, c, w, dim;
    
    if ((argc < 4) || (sscanf (argv[3], "%d", &dim) != 1))
	E_FATAL("Usage: %s meanfile varfile dimension\n", argv[0]);

    gauden_param_read (&mean, &n_cb, &n_feat, &n_den, &featlen, argv[1]);
    fprintf (stderr, "%s: %d x %d x %d x %d\n", argv[2], n_cb, n_feat, n_den, featlen[0]);

    gauden_param_read (&var, &n_cb, &n_feat, &n_den, &featlen, argv[2]);
    fprintf (stderr, "%s: %d x %d x %d x %d\n", argv[2], n_cb, n_feat, n_den, featlen[0]);

    assert (n_feat == 1);
    
    for (c = 0; c < n_cb; c++) {
	for (w = 0; w < n_den; w++) {
	    /* printf ("%5d %3d %12.5e %12.5e\n",
		    c, w, mean[c][0][w][dim], var[c][0][w][dim]); */
	    printf ("%12.5e %12.5e\n",
		    mean[c][0][w][dim], var[c][0][w][dim]);
	}
    }

    exit(0);
}
