/*
 * agc.h -- Various forms of automatic gain control (AGC)
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


#ifndef _S3_AGC_H_
#define _S3_AGC_H_


#include <libutil/libutil.h>


/*
 * Apply AGC to the given mfc vectors (normalize all C0 mfc coefficients in the given
 * input such that the max C0 value is 0, by subtracting the input max C0 from all).
 * This function operates on an entire utterance at a time.  Hence, the entire utterance
 * must be available beforehand (batchmode).
 */
void agc_max (float32 **mfc,	/* In/Out: mfc[f] = cepstrum vector in frame f */
	      int32 n_frame);	/* In: #frames of cepstrum vectors supplied */

#endif
