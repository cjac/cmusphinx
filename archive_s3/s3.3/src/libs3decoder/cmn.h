/*
 * cmn.h -- Various forms of cepstral mean normalization
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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added cmn_free() and moved *mean and *var out global space and named them cmn_mean and cmn_var
 * 
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Copied from previous version.
 */


#ifndef _S3_CMN_H_
#define _S3_CMN_H_


#include <libutil/libutil.h>


/*
 * Apply Cepstral Mean Normalization (CMN) to the set of input mfc frames, by subtracting
 * the mean of the input from each frame.  C0 is also included in this process.
 * This function operates on an entire utterance at a time.  Hence, the entire utterance
 * must be available beforehand (batchmode).
 */
void cmn (float32 **mfc,	/* In/Out: mfc[f] = mfc vector in frame f */
	  int32 varnorm,	/* In: if not FALSE, variance normalize the input vectors
				   to have unit variance (along each dimension independently);
				   Irrelevant if no cmn is performed */
	  int32 n_frame,	/* In: #frames of mfc vectors */
	  int32 veclen);	/* In: mfc vector length */


#define CMN_WIN_HWM     800     /* #frames after which window shifted */
#define CMN_WIN         500

void cmn_prior(float32 **incep,  /* In/Out: mfc[f] = mfc vector in frame f*/
	      int32 varnorm,    /* This flag should always be 0 for live */
	      int32 nfr,        /* Number of incoming frames */
              int32 ceplen,     /* Length of the cepstral vector */
	      int32 endutt);    /* Flag indicating end of utterance */


/* RAH, free previously allocated memory */
void cmn_free ();

#endif
