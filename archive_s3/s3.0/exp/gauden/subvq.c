/*
 * subvq.c
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
 * 12-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include "subvq.h"


/*
 * Precompute variances/(covariance-matrix-determinants) to simplify Mahalanobis distance
 * calculation (see libmain/gauden.*).  Also, calculate 1/(det) for the original codebooks,
 * based on the VQ vars.
 */
static void subvq_ivar_idet_precompute (subvq_t *vq, float64 floor)
{
    int32 s, r, c;
    float32 **idet;
    
    E_INFO("Precomputing 1/det(covar), 1/2*var\n");
    
    /* Temporary idet array for vars in subvq */
    idet = (float32 **) ckd_calloc_2d (vq->n_sv, vq->vqsize, sizeof(float32));
    
    for (s = 0; s < vq->n_sv; s++) {
	for (r = 0; r < vq->vqsize; r++) {
	    vector_nz_floor (vq->var[s][r], vq->svsize[s], floor);
	    
	    idet[s][r] = (float32) vector_maha_precomp (vq->var[s][r], vq->svsize[s]);
	}
    }
    vq->idet = idet;
}


subvq_t *subvq_init (char *file)
{
    FILE *fp;
    char line[16384];
    int32 n_sv;
    int32 s, k, n, r, c;
    char *strp;
    subvq_t *vq;
    
    E_INFO("Loading Mixture Gaussian sub-VQ file '%s'\n", file);
    
    vq = (subvq_t *) ckd_calloc (1, sizeof(subvq_t));
    
    fp = myfopen(file, "r");
    
    /* Read until "Sub-vectors" */
    for (;;) {
	if (fgets (line, sizeof(line), fp) == NULL)
	    E_FATAL("Failed to read VQParam header\n");
	if (sscanf (line, "VQParam %d %d -> %d %d",
		    &(vq->origsize.r), &(vq->origsize.c), &(vq->n_sv), &(vq->vqsize)) == 4)
	    break;
    }
    
    n_sv = vq->n_sv;
    
    vq->svsize = (int32 *) ckd_calloc (n_sv, sizeof(int32));
    vq->featdim = (int32 **) ckd_calloc (n_sv, sizeof(int32 *));
    vq->mean = (float32 ***) ckd_calloc (n_sv, sizeof(float32 **));
    vq->var  = (float32 ***) ckd_calloc (n_sv, sizeof(float32 **));
    vq->map = (int32 ***) ckd_calloc_3d (vq->origsize.r, vq->origsize.c, n_sv, sizeof(int32));
    vq->cb_invalid = bitvec_alloc (vq->origsize.r);
    
    /* Read subvector sizes and feature dimension maps */
    for (s = 0; s < n_sv; s++) {
	if ((fgets (line, sizeof(line), fp) == NULL) ||
	    (sscanf (line, "Subvector %d length %d%n", &k, &(vq->svsize[s]), &n) != 2) ||
	    (k != s))
	    E_FATAL("Error reading length(subvector %d)\n", s);
	
	vq->mean[s] = (float32 **) ckd_calloc_2d (vq->vqsize, vq->svsize[s], sizeof(float32));
	vq->var[s]  = (float32 **) ckd_calloc_2d (vq->vqsize, vq->svsize[s], sizeof(float32));
	vq->featdim[s] = (int32 *) ckd_calloc (vq->svsize[s], sizeof(int32));
	
	for (strp = line+n, c = 0; c < vq->svsize[s]; c++) {
	    if (sscanf (strp, "%d%n", &(vq->featdim[s][c]), &n) != 1)
		E_FATAL("Error reading subvector(%d).featdim(%d)\n", s, c);
	    strp += n;
	}
    }
    
    /* Echo info for sanity check */
    E_INFO("Original #codebooks(states)/codewords: %d x %d\n", vq->origsize.r, vq->origsize.c);
    E_INFO("Subvectors: %d, VQsize: %d\n", vq->n_sv, vq->vqsize);
    for (s = 0; s < n_sv; s++) {
	E_INFO("Feature dims(%d): ", s);
	for (c = 0; c < vq->svsize[s]; c++)
	    printf (" %2d", vq->featdim[s][c]);
	printf (" (%d)\n", vq->svsize[s]);
    }
    
    /* Read VQ codebooks and maps for each subvector */
    for (s = 0; s < n_sv; s++) {
	E_INFO("Reading subvq %d\n", s);
	
	E_INFO("Reading codebook\n");
	if ((fgets (line, sizeof(line), fp) == NULL) ||
	    (sscanf (line, "Codebook %d", &k) != 1) || (k != s))
	    E_FATAL("Error reading header\n", s);
	
	for (r = 0; r < vq->vqsize; r++) {
	    if (fgets (line, sizeof(line), fp) == NULL)
		E_FATAL("Error reading row(%d)\n", r);
	    
	    for (strp = line, c = 0; c < vq->svsize[s]; c++) {
		if (sscanf (strp, "%f %f%n", &(vq->mean[s][r][c]), &(vq->var[s][r][c]), &k) != 2)
		    E_FATAL("Error reading row(%d) col(%d)\n", r, c);
		strp += k;
	    }
	}
	
#if 0
	E_INFO("Sanity check: mean[0,%d]:\n", vq->vqsize-1);
	vector_print (stdout, vq->mean[s][0], vq->svsize[s]);
	vector_print (stdout, vq->mean[s][vq->vqsize-1], vq->svsize[s]);
	E_INFO("Sanity check: var[0,%d]:\n", vq->vqsize-1);
	vector_print (stdout, vq->var[s][0], vq->svsize[s]);
	vector_print (stdout, vq->var[s][vq->vqsize-1], vq->svsize[s]);
#endif

	E_INFO("Reading map\n");
	if ((fgets (line, sizeof(line), fp) == NULL) ||
	    (sscanf (line, "Map %d", &k) != 1) || (k != s))
	    E_FATAL("Error reading header\n", s);
	
	for (r = 0; r < vq->origsize.r; r++) {
	    if (fgets (line, sizeof(line), fp) == NULL)
		E_FATAL("Error reading row(%d)\n", r);
	    
	    for (strp = line, c = 0; c < vq->origsize.c; c++) {
		if (sscanf (strp, "%d%n", &(vq->map[r][c][s]), &k) != 1)
		    E_FATAL("Error reading row(%d) col(%d)\n", r, c);
		strp += k;
	    }
	}
	
#if 0
	E_INFO("Sanity check: map[0][0]:\n");
	for (c = 0; c < vq->origsize.c; c++)
	    printf (" %d", vq->map[0][c][s]);
	printf ("\n");
#endif
	fflush (stdout);
    }
    
    if ((fscanf (fp, "%s", line) != 1) || (strcmp (line, "End") != 0))
	E_FATAL("Error reading 'End' token\n");
    
    fclose (fp);

    subvq_ivar_idet_precompute (vq, 0.0001 /* varfloor */);

#if 0    
    E_INFO("Sanity check: var[*,0]:\n");
    for (s = 0; s < n_sv; s++)
	vector_print (stdout, vq->var[s][0], vq->svsize[s]);
#endif

    /* Replace invalid entries in map with duplicate of a valid entry, if possible */
    for (r = 0; r < vq->origsize.r; r++) {
	k = -1;
	for (c = 0; c < vq->origsize.c; c++) {
	    if (vq->map[r][c][0] < 0) {
		/* All ought to be < 0 */
		for (s = 1; s < vq->n_sv; s++) {
		    if (vq->map[r][c][s] >= 0)
			E_FATAL("Partially undefined map[%d][%d]\n", r, c);
		}
	    } else {
		/* All ought to be >= 0 */
		for (s = 1; s < vq->n_sv; s++) {
		    if (vq->map[r][c][s] < 0)
			E_FATAL("Partially undefined map[%d][%d]\n", r, c);
		}
		k = c;	/* A valid codeword found; remember it */
	    }
	}
	
	if (k >= 0) {
	    /* Copy k into invalid rows */
	    for (c = 0; c < vq->origsize.c; c++) {
		if (vq->map[r][c][0] < 0) {
		    for (s = 0; s < vq->n_sv; s++)
			vq->map[r][c][s] = vq->map[r][k][s];
		}
	    }
	    bitvec_clear (vq->cb_invalid, r);
	} else
	    bitvec_set (vq->cb_invalid, r);
    }
    
    return vq;
}


void subvq_free (subvq_t *s)
{
    int32 i;
    
    for (i = 0; i < s->n_sv; i++) {
	ckd_free_2d ((void **) s->mean[i]);
	ckd_free_2d ((void **) s->var[i]);
	ckd_free ((void *) s->featdim[i]);
    }
    
    ckd_free ((void *) s->svsize);
    ckd_free ((void *) s->featdim);
    ckd_free ((void *) s->mean);
    ckd_free ((void *) s->var);
    ckd_free_3d ((void ***) s->map);
    
    ckd_free ((void *) s);
}


void subvq_dist_eval (subvq_t *vq, float32 *vec, int32 **score)
{
    int32 s, i, r;
    float32 *sv;
    float64 d;
    
    for (s = 0; s < vq->n_sv; s++) {
	/* Extract subvector s from vec */
	sv = (float32 *) ckd_calloc (vq->svsize[s], sizeof(float32));
	for (i = 0; i < vq->svsize[s]; i++)
	    sv[i] = vec[vq->featdim[s][i]];
	
	/* Compare subvector to all the codewords */
	for (r = 0; r < vq->vqsize; r++) {
	    d = vector_dist_maha (sv, vq->mean[s][r], vq->var[s][r],
				  vq->idet[s][r], vq->svsize[s]);
	    score[s][r] = log_to_logs3(d);
	}
	
	ckd_free ((void *) sv);
    }
}
