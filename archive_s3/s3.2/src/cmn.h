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

#endif
