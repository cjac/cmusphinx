/* ====================================================================
 * Copyright (c) 1989-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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

#include <stdlib.h>
#include <math.h>
#include "cdcn.h"

/*************************************************************************
 *
 * cdcn_update finds the vectors x, noise
 * and tilt that maximize the a posteriori probability.
 * only one iteration is performed.  this routine can be recalled to 
 * perform multiple iterations if cycles permit.
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified by Uday Jain, June 95
 *
 *************************************************************************/

float
cdcn_update (float *z,		/* The observed cepstrum vectors */
	     int num_frames,	/* Number of frames in utterance */
	     CDCN_type *cdcn_variables)
{	
    float       distortion;
    float	*noise, *tilt, *codebook, *prob, *variance, *corrbook;
    int 	num_codes;
    /* Multidimensional arrays, gar gar gar */
    static float initialize (float *, int, float *, float *, float,
			     float *, float *, float *, int);
    static void correction(float *, float *, float *, float *, int);
    static float max_q (float *, float *, float *, float *, float *,
			float *, int, float *, int);

    /*
     * If error, dont bother
     */
    if (!cdcn_variables->run_cdcn)
        return(-1e+30);
        
    /*
     * Open suitcase
     */

    noise	= cdcn_variables->noise;
    tilt	= cdcn_variables->tilt;
    codebook 	= cdcn_variables->means;
    prob	= cdcn_variables->probs;
    variance	= cdcn_variables->variance;
    corrbook	= cdcn_variables->corrbook;
    num_codes	= cdcn_variables->num_codes;

    /*
     * Initialize if this is the first time the routine is being called
     */
    if (cdcn_variables->firstcall)
    {
        /* Get initial estimates for noise, tilt, x, y */
        initialize (z,num_frames,noise,tilt,SPEECH_THRESHOLD,codebook,
						     prob,variance,num_codes);
        correction (tilt, noise, codebook, corrbook, num_codes);
        cdcn_variables->firstcall = FALSE;
    }

    /*
     * Compute the correction terms for the means 
     * Perform one iteration of the estimation of n and q
     */ 
    distortion = max_q (variance, prob, noise, tilt, codebook, corrbook, 
			num_codes, z, num_frames);

    correction (tilt, noise, codebook, corrbook, num_codes);  
    return (distortion);
}


/*************************************************************************
 *
 * initialize finds an estimate of the noise vector as the average of all
 * frames whose power is below a threshold. It also computes the average
 * log-energy of the frames whose log-energy is above that threshold
 * (supposedly speech frames).
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified by Uday Jain, June 95
 *
 *************************************************************************/

static float
initialize (float data[][NUM_COEFF+1],	/* The observation cepstrum vectors */
	    int	num_frames,		/* Number of frames in utterance */
	    float *noise,		/* Cepstrum vector for the noise */
	    float tilt[],
	    float speech_threshold,	/* Threshold for speech and noise */
	    float codebook[][NUM_COEFF+1],
	    float *prob,
	    float var[][NUM_COEFF+1],
	    int ncodes)
{
    float	noise_ceiling,	/* Threshold to separate speech and noise */
		min,		/* Minimum log-energy in utterance */
		speech_power,	/* Average speech power */
		codemean[NUM_COEFF+1],
		localprob[256];
    int		i,		/* Index all frames in utterance */
		j,		/* Index all coefficients within frame */
		noise_frames,	/* Number of noise frames */
		speech_frames;	/* Number of speech frames */

    for (j = 1; j <= NUM_COEFF; j++)
	tilt[j] = 0.0;
    /* De-normalize prob w.r.t variance */
    for (i=0;i<ncodes;++i)
    {
	localprob[i] = 1.0;
	for (j=0;j<=NUM_COEFF;++j)
	    localprob[i] *= var[i][j];
        localprob[i] = prob[i]*sqrt(localprob[i]);
    }
    /* Initialize tilt */
    for (j=0;j<=NUM_COEFF;++j)
    {
	tilt[j] = 0;
	codemean[j] = 0;
	for (i=0;i<ncodes;++i)
	    codemean[j] += localprob[i]*codebook[i][j];
    }
    for (i=0;i<num_frames;++i)
	for (j=0;j<=NUM_COEFF;++j)
	    tilt[j] += data[i][j];

    for (j=0;j<=NUM_COEFF;++j)
	tilt[j] = tilt[j]/(float)num_frames - codemean[j];

    /* Search for the extrema c[0] in the file */
    min = data[0][0];
    for (i = 0; i < num_frames; i++)
	if (data[i][0] < min) min = data[i][0];

    /* Compute thresholds for noise */
    noise_ceiling = speech_threshold + min;

    /* Every frame whose power is below noise_ceiling is considered noise.
    and every frame above is considered speech */
    noise_frames = 0;
    speech_frames = 0;
    speech_power = 0;
    for (j = 0; j <= NUM_COEFF; j++)
	noise[j] = 0.0;
    for (i = 0; i < num_frames; i++) {
	if (data[i][0] < noise_ceiling) {
	    noise_frames++;
	    for (j = 0; j <= NUM_COEFF; j++)
		noise[j] += data[i][j];
	}
	else {
	    speech_frames++;
	    speech_power += data[i][0];
	}
    }
    for (j = 0; j <= NUM_COEFF; j++)
	noise[j] /= noise_frames;
    speech_power /= speech_frames;
    return (speech_power);
}


/*************************************************************************
 *
 * Subroutine correction computes the correction cepstrum vectors for a 
 * codebook given the spectral tilt and the noise cepstrum vectors.
 * For every codeword it finds the corresponding correction vector. That
 * is, every cepstrum vector within a cluster is transformed differently.
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified by Uday Jain, June 95
 *
 *************************************************************************/

static
void correction(float *tilt, 	/* The spectral tilt cepstrum vector */
		float *noise, 	/* The noise cepstrum vector */
		float *codebook,/* The codebook */
		float *corrbook,/* The correction cepstrum vectors */
		int num_codes)  /* The number of codewords in the codebook */
{
    float aux[N + 1];	/* auxiliary vector for FFTs */
    double exp();
    int	i, 		/* Index for all codewords in codebook */
	j, 		/* Index for all coefficients within a frame */
	offset;		/* Points to current codeword */

    void  resfft();
 
    for (i = 0, offset = 0; i < num_codes; i++, offset += (NUM_COEFF+1)) {

	/* Take direct FFT of noise - tilt - codeword */
	for (j = 0; j <= NUM_COEFF; j++)
	    aux[j] = noise[j] - tilt[j] - codebook[offset + j];
	for (j = NUM_COEFF + 1; j <= N; j++)
	    aux[j] = 0.0;
	resfft (aux, N, M);

	/* Process every frequency through the non-linear function */
	for (j = 0; j <= N; j++)
	    aux[j] = log (exp((double)aux[j]) + 1.0);

	/* Take inverse FFT and write result back */
	resfft (aux, N, M);
	for (j = 0; j <= NUM_COEFF; j++)
	    corrbook[offset + j] = aux[j] / N2;
    }
} 


/*************************************************************************
 *
 * max_q reestimates the tilt cepstrum vector that maximizes the likelihood.
 * It also labels the cleaned speech.
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified by Uday Jain, June 95
 *
 *************************************************************************/

static
float max_q (float *variance,   /* Speech cepstral variances of the modes */
	     float *prob,       /* Ratio of a-priori probabilities of the codes 
				   and the mod of their variances*/
	     float *noise,      /* Cepstrum vector for the noise */
	     float *tilt,       /* Spectral tilt cepstrum */
	     float *codebook,   /* The cepstrum codebook */
	     float *corrbook,   /* The correction factor's codebook */
	     int  num_codes,    /* Number of codewords in codebook */
	     float *z,          /* The input cepstrum */
	     int num_frames)    /* Number of frames in utterance */
{
    float    newtilt[NUM_COEFF + 1],  /* The new tilt vector */
        newnoise[NUM_COEFF + 1], /* The new noise vector */
        *zword,        /* pointer to frame i of z */
        distance,    /* distance value */
        difference,   /* stores z - x- q - r */
        *codeword,    /* pointer to current codeword */
        *corrword,    /* pointer to current correction word */
        *modevar,   /* pointer to the variance of the current word */
        loglikelihood,    /* The log-likelihood */
        probz,         /* Prob. of z given tilt/noise */
        dennoise,    /* Denominator of noise estimation */
        dentilt,    /* Denominator of tilt estimation */
        pnoise,        /* Probability that frame is noise */
        qi[NUM_COEFF+1],  /* Contribution to tilt of frame i */
        ni[NUM_COEFF+1],  /* Contribution to noise of frame i */
        fk;        /* Probabilities for different codewords */
    int i,        /* Index frames in utterance */
        j,        /* Index coefficients within frame */
        k,    /* Index codewords in codebook */
        offset;  /* Holds offset of current code with base address */

    /* Initialize newtilt and newnoise */
    for (j = 0; j <= NUM_COEFF; j++) {
        newtilt[j] = 0.0;
        newnoise[j] = 0.0;
    }
    loglikelihood = 0.0;
    dennoise = 0.0;
    dentilt = 0.0;

    /* Reestimate tilt vector for all frames in utterance */
    for (i = 0; i < num_frames; i++) {
        /* Pointers to current frame */
        zword = z + i * (NUM_COEFF + 1);

        /* Reestimate noise vector for codeword 0 */
        difference = zword[0] - corrbook[0] - codebook[0] - tilt[0];
        distance = difference*difference / variance[0];
        for (j = 1; j <= NUM_COEFF; j++)
        {
            difference = zword[j] - corrbook[j] - codebook[j] - tilt[j];
            distance += difference*difference / variance[j];
        }
        fk = exp ((double) - distance / 2)*prob[0];
        probz =  fk;
        pnoise = fk;
        for (j = 0; j <= NUM_COEFF; j++) {
            ni[j] = zword[j] * fk;
            qi[j] = 0.0;
        }

        offset = 0;
    /* Reestimate tilt vector across all codewords */
        for (k = 1; k < num_codes; k++) {
        /* Get pointers to current codeword and correction vector */
            offset += (NUM_COEFF+1);
            codeword = codebook + offset;
            corrword = corrbook + offset;
            modevar  = variance + offset; 

        /* Restimate tilt vector for codeword k */
            difference = zword[0] - codeword[0] - corrword[0] - tilt[0];
            distance += difference*difference / modevar[0];
            for (j = 1; j <= NUM_COEFF; j++)
            {
                difference = zword[j] - codeword[j] - corrword[j] - tilt[j];
                distance += difference*difference / modevar[j];
            }
            fk = exp ((double) - distance / 2) * prob[k];
            probz += fk;
            for (j = 0; j <= NUM_COEFF; j++)
                qi[j] += (zword[j] -codeword[j] - corrword[j]) * fk;
        }
	if (probz != 0.0)
	{
	    /*
	     * before the sign in the loglikelihood used to be negative 
	     */
            loglikelihood += log ((double) probz);
            pnoise /= probz;
            dennoise += pnoise;
            dentilt += (1 - pnoise);
            for (j = 0; j <= NUM_COEFF; j++) {
                newnoise[j] += ni[j] / probz;
                newtilt[j] += qi[j] / probz;
            }
        }
    }

    /* Normalize the estimated tilt vector across codewords */
    for (j = 0; j <= NUM_COEFF; j++) {
	if (dennoise != 0)
            noise[j] = newnoise[j] / dennoise;
        tilt[j] = newtilt[j] / dentilt;
    }
    loglikelihood /= num_frames;
    /* 
     * we deactivate this part of the code since it is not needed
     *
     * loglikelihood += OFFSET_LIKELIHOOD  ; 
     *
     */
    return (loglikelihood);
}


