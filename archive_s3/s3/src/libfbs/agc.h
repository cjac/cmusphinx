/*
 * agc.h -- Various forms of automatic gain control (AGC)
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
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFBS_AGC_H_
#define _LIBFBS_AGC_H_

#include <libutil/prim_type.h>

/*
 * Apply AGC to the given mfc vectors (normalize all C0 mfc coefficients in the given
 * input such that the max C0 value is 0, by subtracting the input max C0 from all).
 * This function operates on an entire utterance at a time.  Hence, the entire utterance
 * must be available beforehand.
 */
void agc_max (float **mfc,	/* In/Out: mfc[f] = mfc vector in frame f */
	      int32 n_frame);	/* In: #frames of mfc vectors */

#endif
