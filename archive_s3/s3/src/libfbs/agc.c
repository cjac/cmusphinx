/*
 * agc.c -- Various forms of automatic gain control (AGC)
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


#include <assert.h>

#include <libutil/libutil.h>
#include "agc.h"


/*
 * Normalize c0 for all frames such that max(c0) = 0.
 */
void agc_max (float **mfc, int32 n_frame)
{
    float32 maxc0;
    int32 i;

    assert (n_frame > 0);

    maxc0 = mfc[0][0];
    for (i = 1; i < n_frame; i++) {
	if (mfc[i][0] > maxc0)
	    maxc0 = mfc[i][0];
    }

    for (i = 0; i < n_frame; i++)
	mfc[i][0] -= maxc0;
}
