/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
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
 * 19-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed to compute CMN over ALL dimensions of cep instead of 1..12.
 * 
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <assert.h>

#include <libutil/libutil.h>
#include "cmn.h"
#include <math.h>
#include <string.h>


/*
 * Normalize all cepstral vectors (except C0) by subtracting the mean for the
 * entire utterance.
 */
void norm_mean (float32 **mfc, int32 n_frame, int32 veclen)
{
    static float32 *mean = 0, *var = 0;
    float *mfcp;
    int32 i, f, normvar;
    char *varnorm;

    varnorm = (char *) cmd_ln_access ("-varnorm");
    if (strcmp(varnorm,"yes")==0) normvar = 1;
    else normvar = 0;

    if (normvar) E_INFO("Normalizing utterance variance\n");

    assert ((n_frame > 0) && (veclen > 0));
    
    if (mean == 0)
	mean = (float32 *) ckd_calloc (veclen, sizeof (float32));
    if (var == 0)
	var = (float32 *) ckd_calloc (veclen, sizeof (float32));

    for (i = 0; i < veclen; i++)
	mean[i] = var[i] = 0.0;

    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    mean[i] += mfcp[i];
    }

    for (i = 0; i < veclen; i++)
	mean[i] /= n_frame;

    if (normvar) {
        for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
	        var[i] += (mfcp[i]-mean[i])*(mfcp[i]-mean[i]);
        }

        for (i = 0; i < veclen; i++)
	    var[i] = (float)sqrt((double)var[i]/(double)n_frame); /* std dev */
    }

    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    mfcp[i] -= mean[i];
    }

    if (normvar) {
        for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
	        mfcp[i] /= var[i];
        }
    }
}
