/*
 * cmn.h -- Various forms of cepstral mean normalization
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
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBMAIN_CMN_H_
#define _LIBMAIN_CMN_H_

#include <libutil/prim_type.h>

/*
 * Apply Cepstral Mean Normalization (CMN) to the set of input mfc frames, by subtracting
 * the mean of the input from each.
 */
void cmn (float **mfc,		/* In/Out: mfc[f] = mfc vector in frame f */
	  int32 n_frame,	/* In: #frames of mfc vectors */
	  int32 veclen);	/* In: mfc vector length, including C0 */

#endif
