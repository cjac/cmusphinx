/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * gauden.h -- gaussian density module.
 *
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
