/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "libs3decoder/new_fe.h"
#include "libs3decoder/new_fe_sp.h"
#include "new_fe_sp_dump.h"
#include "fe_dump.h"


void fe_frame_to_fea_dump(fe_t *FE, double *in, double *fea)
{
    double *spec, *mfspec;

    /* RAH, typo */
    if (FE->FB_TYPE == MEL_SCALE){

	spec = (double *)calloc(FE->FFT_SIZE, sizeof(double));
	mfspec = (double *)calloc(FE->MEL_FB->num_filters, sizeof(double));

	if (spec==NULL || mfspec==NULL){
	    fprintf(stderr,"memory alloc failed in fe_frame_to_fea()\n...exiting\n");
	    exit(0);
	}

        metricsStart("SpectrumMagnitude");
        
        fe_spec_magnitude(in, FE->FRAME_SIZE, spec, FE->FFT_SIZE);
        
        metricsStop("SpectrumMagnitude");


        fe_dump_double_frame(fe_dumpfile, spec, FE->FFT_SIZE/2,
                             "SPEC_MAGNITUDE");

        metricsStart("MelSpectrum");

	fe_mel_spec(FE, spec, mfspec);

        metricsStop("MelSpectrum");

        fe_dump_double_frame(fe_dumpfile, mfspec, FE->MEL_FB->num_filters,
                             "MEL_SPECTRUM   ");

        metricsStart("MelCepstrum");

	fe_mel_cep(FE, mfspec, fea);

        metricsStop("MelCepstrum");

        fe_dump_double_frame(fe_dumpfile, fea, FE->NUM_CEPSTRA,
                             "MEL_CEPSTRUM   ");

	free(spec);
	free(mfspec);
    }
    else {
	fprintf(stderr,"MEL SCALE IS CURRENTLY THE ONLY IMPLEMENTATION!\n");
	exit(0);
    }
    
}
