/*
 * cmn.c -- Various forms of cepstral mean normalization
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
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the name norm_mean() to cmn().
 * 
 * 19-Jun-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed to compute CMN over ALL dimensions of cep instead of 1..12.
 * 
 * 04-Nov-1995	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include "cmn.h"


void cmn (float32 **mfc, int32 varnorm, int32 n_frame, int32 veclen)
{
    static float32 *mean = NULL;
    static float32 *var = NULL;
    float32 *mfcp;
    float32 t;
    int32 i, f;
    
    assert ((n_frame > 0) && (veclen > 0));
    
    if (mean == NULL)
	mean = (float32 *) ckd_calloc (veclen, sizeof (float32));

    /* Find mean cep vector for this utterance */
    for (i = 0; i < veclen; i++)
	mean[i] = 0.0;
    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    mean[i] += mfcp[i];
    }
    for (i = 0; i < veclen; i++)
	mean[i] /= n_frame;
    
    if (! varnorm) {
	/* Subtract mean from each cep vector */
	for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
		mfcp[i] -= mean[i];
	}
    } else {
	/* Scale cep vectors to have unit variance along each dimension, and subtract means */
        if (var == NULL)
    	    var = (float32 *) ckd_calloc (veclen, sizeof (float32));
	
        for (i = 0; i < veclen; i++)
	    var[i] = 0.0;
	
        for (f = 0; f < n_frame; f++) {
    	    mfcp = mfc[f];
	    
	    for (i = 0; i < veclen; i++) {
                t = mfcp[i] - mean[i];
                var[i] += t * t;
            }
        }
        for (i = 0; i < veclen; i++) 
            var[i] = (float32) sqrt ((float64) n_frame / var[i]);	/* Inverse Std. Dev */

        for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
	        mfcp[i] = (mfcp[i] - mean[i]) * var[i];
        }
    }
}
