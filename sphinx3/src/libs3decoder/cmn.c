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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added cmn_free() and moved *mean and *var out global space and named them cmn_mean and cmn_var
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

static float32 *cmn_mean = NULL;
static float32 *cmn_var = NULL;


void cmn (float32 **mfc, int32 varnorm, int32 n_frame, int32 veclen)
{
  /*    static float32 *mean = NULL; */ /*  */
  /*    static float32 *var = NULL;	*/ /*  */
    float32 *mfcp;
    float32 t;
    int32 i, f;
    
    assert ((n_frame > 0) && (veclen > 0));
    
    if (cmn_mean == NULL)
	cmn_mean = (float32 *) ckd_calloc (veclen, sizeof (float32));

    /* Find mean cep vector for this utterance */
    for (i = 0; i < veclen; i++)
      cmn_mean[i] = 0.0;
    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    cmn_mean[i] += mfcp[i];
    }
    for (i = 0; i < veclen; i++)
	cmn_mean[i] /= n_frame;
    
    if (! varnorm) {
	/* Subtract mean from each cep vector */
	for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
		mfcp[i] -= cmn_mean[i];
	}
    } else {
	/* Scale cep vectors to have unit variance along each dimension, and subtract means */
        if (cmn_var == NULL)
    	    cmn_var = (float32 *) ckd_calloc (veclen, sizeof (float32));
	
        for (i = 0; i < veclen; i++)
	    cmn_var[i] = 0.0;
	
        for (f = 0; f < n_frame; f++) {
    	    mfcp = mfc[f];
	    
	    for (i = 0; i < veclen; i++) {
                t = mfcp[i] - cmn_mean[i];
                cmn_var[i] += t * t;
            }
        }
        for (i = 0; i < veclen; i++) 
	  cmn_var[i] = (float32) sqrt ((float64) n_frame / cmn_var[i]); /* Inverse Std. Dev, RAH added type case from sqrt */

        for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
	        mfcp[i] = (mfcp[i] - cmn_mean[i]) * cmn_var[i];
        }
    }
}

/* 
 * RAH, free previously allocated memory
 */
void cmn_free ()
{
  ckd_free ((void *) cmn_var);
  ckd_free ((void *) cmn_mean);
}
