/*
 * cmn.c -- Various forms of cepstral mean normalization
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
 * 19-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed to compute CMN over ALL dimensions of cep instead of 1..12.
 * 
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <libutil/libutil.h>
#include "cmn.h"


void cmn (float32 **mfc, int32 n_frame, int32 veclen)
{
    static float32 *mean = 0;
    float *mfcp;
    int32 i, f;

    assert ((n_frame > 0) && (veclen > 0));
    
    if (mean == 0)
	mean = (float32 *) ckd_calloc (veclen, sizeof (float32));

    for (i = 0; i < veclen; i++)
	mean[i] = 0.0;

    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    mean[i] += mfcp[i];
    }

    for (i = 0; i < veclen; i++)
	mean[i] /= n_frame;

    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    mfcp[i] -= mean[i];
    }
}
