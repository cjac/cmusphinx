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
 * beam.h -- Various forms of pruning beam
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
 * 19-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_BEAM_H_
#define _S3_BEAM_H_

#include <s3types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structure containing various beamwidth parameters.  All logs3 values; -infinite is widest,
 * 0 is narrowest.
 */
typedef struct {
    int32 hmm;		/* For selecting active HMMs, relative to best */
    int32 ptrans;	/* For determining which HMMs transition to their successors */
    int32 word;		/* For selecting words exited, relative to best HMM score */
    int32 ptranskip;     /* Intervals at which wbeam is used for phone transitions */
    int32 wordend;      /* For selecting the number of word ends  */
     
} beam_t;

/*
 * Structure containing various histogram pruning parameters.  All in
 * integers.
 */

typedef struct {
  int32 maxwpf;         /*Max words per frame*/
  int32 maxhistpf;      /*Max histories per frame*/
  int32 maxhmmpf;        /*Max active HMMs per frame*/
} histprune_t;

typedef struct{
  int32 ds_ratio;              /* Ratio of down-sampling the frame computation */
  int32 cond_ds;            /* Whether we want to use conditional DS, 
				 cond_ds=0, don't use,
				 cond_ds=1, store previous 1 frame
			    */

  int32 dist_ds;              /* Whether we want to use distance-based DS,
				    dist_ds=0, don't use,
				    dist_ds=1, store previous 1 frame
			      */
  
  int32 skip_count;         /* Counting how many frames are skipped */

} downsampling_t;

typedef struct{
  int32 ci_pbeam;             /* The beam which prune out unnesseary parent CI phones in 
				 CI-based GMM selection*/
  int32 dyn_ci_pbeam;         /* The dynamic CI-beam computed by using both CI-pbeam and 
				 the counts*/
  int32 *ci_occu;            /* Recorded number of CD senones for a
				 particular base CI senone. */
  int32 *idx;                /* temporary indices used in absolute
				discounting of CI-based GMM selection */
  int32 max_cd;              /* Maximum CD senones computed. 
			      */
} gmm_select_t;

typedef struct{
  int32 subvqbeam;	/* For selecting active mixture components based on subvq scores */
  int32 rec_bstcid;    /* Best codeword ID for Gaussian Selection Map. */
} gau_select_t;

typedef struct{
  downsampling_t* downs;       /* All structure for down-sampling */
  gmm_select_t* gmms;          /* All structure for GMM-level of selection */
  gau_select_t* gaus;          /* All structure for Gaussian-level of selection */
  int32 gs4gs;                /* Whether the GS map is used for Gaussian Selection or not 
				 mainly for internal debugging of Conditional Down-Sampling */
  int32 svq4svq;              /* Whether SVQ scores would be used as the Gaussian Scores */
  int32 rec_bst_senscr;       /* recent best scores. */
  float32 *last_feat;         /* Last feature frame */

} fast_gmm_t;

/*
 * Create and initialize a beam_t structure, with the given
 * parameters, converting them from prob space to logs3 space.  Return
 * value: a pointer to created structure if successful, NULL otherwise.
 *
 * Note the last parameter is used in controling when the word beam is
 * applied.  
 */


beam_t *beam_init (
		   float64 hmm, 
		   float64 ptr, 
		   float64 wd, 
		   float64 wdend, 
		   int32 ptranskip);

/*
 * Create and initialize a histprune_t structure, with the given parameters. 
 */

histprune_t *histprune_init (int32 maxhmm,
			     int32 maxhist, 
			     int32 maxword);

/*
 * Create and initialize a fast_gmm_t structure, withe the given parameters
 */
fast_gmm_t *fast_gmm_init (int32 down_sampling_ratio, 
			   int32 mode_cond_ds,
			   int32 mode_dist_ds,
			   int32 isGS4GS,
			   int32 isSVQ4SVQ,
			   float32 subvqbeam,
			   float32 cibeam,  /* Input: CI phone beam */
			   int32 max_cd,    /* Input: Max CD senone to be computed */
			   int32 n_ci_sen); /* Input: no. of ci senone, use to initialize  the ci_occ array*/


#ifdef __cplusplus
}
#endif


#endif
