/*
 * gauden.h -- gaussian density module.
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
 * 11-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added det_varinv argument to gauden_init().
 * 
 * 09-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added mgau_t.cb0.
 * 
 * 19-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started, based on original S3 version.
 */


#ifndef _LIBMAIN_GAUDEN_H_
#define _LIBMAIN_GAUDEN_H_


#include "s3types.h"
#include <libmisc/libmisc.h>


/* A single mixture-Gaussian; each has its own size (#means) and variance sharing */
typedef struct {
    int32 n_mean;	/* #mean vectors (#component Gaussians) */
    int32 n_var;	/* #var vectors; must be == n_mean, 1, or 0, meaning:
			   unshared, shared (within this codebook), or none (euclidean) */
    vector_t *mean;	/* Mean vectors (centroids) for this mixture density codebook */
    vector_t *var;	/* Variance vectors */
    float32 *det;	/* log(sqrt(2*pi*det)) for each variance vector */
    int32 cb0;		/* TRUE iff all mean vectors in this codebook are 0.0; meaning a
			   potentially uninitialized codebook */
} mgau_t;


/*
 * The set of mixture-Gaussians in an acoustic model.  The model consists of a collection
 * of (possibly shared) CODEBOOKs.  Each codebook has one mgau_t for each feature stream.
 */
typedef struct {
    int32 n_mgau;	/* #codebooks (perhaps n_mgau is a misnomer) */
    int32 n_feat;	/* #input feature streams */
    int32 *featlen;	/* Vector length of each feature stream */
    mgau_t **mgau;	/* mgau[n_mgau][n_feat]; each one can have a different #means */
    int32 max_n_mean;	/* Max(mgau[][].n_mean); useful for clients to allocate memory */
    float64 min_density; /* Gaussian density values in (int32)logs3 domain can underflow;
			    Use this floor value if they underflow */
} gauden_t;


/* Some access macros */
#define gauden_n_mgau(g)	((g)->n_mgau)
#define gauden_max_n_mean(g)	((g)->max_n_mean)
#define gauden_n_stream(g)	((g)->n_feat)
#define gauden_stream_len(g,i)	((g)->featlen[i])
#define gauden_n_mean(g,m,f)	((g)->mgau[m][f].n_mean)
#define gauden_n_var(g,m,f)	((g)->mgau[m][f].n_var)
#define gauden_cb0(g,m,f)	((g)->mgau[m][f].cb0)


/*
 * Read mixture gaussian codebooks from the given files.  Allocate memory space needed
 * for them.  Apply the specified variance floor value.  Optionally, precompute the
 * log(determinants) for the (diagonal) variances and transform variances to 1/(2*var)
 * (these are optimizations for Mahalanobis distance computation).
 * Return value: ptr to the model created; NULL if error.
 * (See Sphinx3 model file-format documentation.)
 */
gauden_t *
gauden_init (char *meanfile,	/* In: File containing means of mixture gaussians */
	     char *varfile,	/* In: File containing variances of mixture gaussians */
	     float64 varfloor,	/* In: Floor value to be applied to variances;
				   usually 0.0001 */
	     int32 det_varinv);	/* In: Boolean, whether to precompute determinants and
				   transform var to 1/(2*var) for distance computation */

/*
 * Reload mixture Gaussian means from the given file.  The means must have already
 * been loaded at least once (using gauden_init).
 * Return value: 0 if successful, -1 otherwise.
 */
int32 gauden_mean_reload (gauden_t *g,		/* In/Out: g->mean to be reloaded */
			  char *meanfile);	/* In: File to reload means from */

/*
 * Compute gaussian density values for the given input vector wrt the specified mixture
 * gaussian codebook.  Density values are unnormalized and in logs3 domain.
 * Return value: #densities computed and returned in dist.
 */
int32
gauden_dist (gauden_t *g,	/* In: handle to entire ensemble of codebooks */
	     int32 m,		/* In: <codebook,feature> specifying the mgau */
	     int32 f,
	     vector_t obs,	/* In: Input observation vector */
	     int32 *dist	/* Out: "Distance" of obs wrt each density in
				   g->mgau[m][f].  Caller must allocate sufficient
				   space for all densities (e.g. g->max_n_mean) */
	    );

/* Floor the values of non-zero variance vectors */
void gauden_var_nzvec_floor (gauden_t *g,	/* In: Gaussian density to be operated on */
			     float64 floor);	/* In: Floor value */

#endif
