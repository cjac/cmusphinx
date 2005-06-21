/*
 * gauden.h -- gaussian density module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * $Log$
 * Revision 1.6  2005/06/21  18:55:09  arthchan2003
 * 1, Add comments to describe this modules, 2, Fixed doxygen documentation. 3, Added $ keyword.
 * 
 * Revision 1.4  2005/06/13 04:02:55  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added gauden_mean_reload() for application of MLLR.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added active argument to gauden_dist_norm and gauden_dist_norm_global,
 * 		and made the latter a static function.
 * 
 * 06-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Initial version created.
 * 		Very liberally borrowed/adapted from Eric's S3 trainer implementation.
 */


#ifndef _LIBFBS_GAUDEN_H_
#define _LIBFBS_GAUDEN_H_

/** \file ms_gauden.h
 * \brief (Sphinx 3.0 specific) Gaussian density module.
 *
 * Gaussian density distribution implementation. There are two major
 * difference bettwen ms_gauden and cont_mgau. One is the fact that
 * ms_gauden only take cares of the Gaussian computation part where
 * cont_mgau actually take care of senone computation as well. The
 * other is the fact that ms_gauden is a multi-stream implementation
 * of GMM computation.
 *
 */

#include <s3types.h>
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**
 * Multivariate gaussian mixture density parameters
 */
typedef struct {
    vector_t ***mean;	/** mean[codebook][feature][codeword] vector */
    vector_t ***var;	/** like mean; diagonal covariance vector only */
    float32 ***det;	/** log(determinant) for each variance vector;
			   actually, log(sqrt(2*pi*det)) */
    int32 n_mgau;	/** #codebooks */
    int32 n_feat;	/** #feature streams in each codebook */
    int32 n_density;	/** #gaussian densities in each codebook-feature stream */
    int32 *featlen;	/** feature length for each feature */
} gauden_t;

  /**
 * Structure to store distance (density) values for a given input observation
 * wrt density values in some given codebook.
 */
typedef struct {
    int32 id;		/** Index of codeword (gaussian density) */
    int32 dist;		/** Density value for input observation wrt above codeword;
			   NOTE: result in logs3 domain; hence int32 */

} gauden_dist_t;


  /**
 * Read mixture gaussian codebooks from the given files.  Allocate memory space needed
 * for them.  Apply the specified variance floor value.
 * Return value: ptr to the model created; NULL if error.
 * (See Sphinx3 model file-format documentation.)
 */
gauden_t *
gauden_init (char *meanfile,	/**< Input: File containing means of mixture gaussians */
	     char *varfile,	/**< Input: File containing variances of mixture gaussians */
	     float32 varfloor	/**< Input: Floor value to be applied to variances */
	     );

  /**
 * Reload mixture Gaussian means from the given file.  The means must have already
 * been loaded at least once (using gauden_init).
 * @return 0 if successful, -1 otherwise.
 */
int32 gauden_mean_reload (gauden_t *g,		/**< In/Out: g->mean to be reloaded */
			  char *meanfile	/**< In: File to reload means from */
			  );

  /**
 * Compute gaussian density values for the given input observation vector wrt the
 * specified mixture gaussian codebook (which may consist of several feature streams).
 * Density values are left UNnormalized.
 * @return 0 if successful, -1 otherwise.
 */
int32
gauden_dist (gauden_t *g,	/**< In: handle to entire ensemble of codebooks */
	     s3mgauid_t mgau,	/**< In: codebook for which density values to be evaluated
				   (g->{mean,var}[mgau]) */
	     int32 n_top,	/**< In: #top densities to be evaluated */
	     vector_t *obs,	/**< In: Observation vector; obs[f] = for feature f */
	     gauden_dist_t **out_dist
				/**< Out: n_top best codewords and density values,
				   in worsening order, for each feature stream.
				   out_dist[f][i] = i-th best density for feature f.
				   Caller must allocate memory for this output */
	    );


  /**
 * Normalize density values (previously computed by gauden_dist).
 * Two cases:  If (g->n_mgau == 1), normalize such that the sum of the n_top codeword
 * scores for each feature in dist sums to 1 (in prob domain).
 * Otherwise, normalize by dividing the density value (subtracting, in logprob domain) for
 * each codeword by the best one.
 * @return scaling applied to every senone score as a result of the normalization.
 */
int32
gauden_dist_norm (gauden_t *g,		/**< In: handle to all collection of codebooks */
		  int32 n_top,		/**< In: #density values computed per feature */
		  gauden_dist_t ***dist,/**< In/Out: n_top density indices and values for
					   each feature.  On return, density values are
					   normalized. */
		  int8 *active	/**< In: active[gid] is non-0 iff codebook gid is
					   active.  If NULL, all codebooks active */
		  );

#ifdef __cplusplus
}
#endif

#endif /* GAUDEN_H */ 
