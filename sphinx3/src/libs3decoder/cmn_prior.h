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


#ifndef _S3_CMN_PRIOR_H_
#define _S3_CMN_PRIOR_H_


#include <libutil/libutil.h>

/*
 * Apply Cepstral Mean Normalization (CMN) to the set of input mfc frames, 
 * by subtracting the mean of the input from each frame.  C0 is also included 
 * in this process.
 */

#define CMN_WIN_HWM     800     /* #frames after which window shifted */
#define CMN_WIN         500

void cmn_prior(float32 **incep, /* In/Out: mfc[f] = mfc vector in frame f*/
	      int32 varnorm,    /* This flag should always be 0 for live */
	      int32 nfr,        /* Number of incoming frames */
              int32 ceplen,     /* Length of the cepstral vector */
	      int32 endutt);    /* Flag indicating end of utterance */

#endif
