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

#include <math.h>
#include "cdcn.h"

/************************************************************************
 *   Dummy routine to convert from suitcase to sane varibles
 ***************************************************************************/

void cdcn_norm (float z[NUM_COEFF+1], /* The input cepstrum */
		CDCN_type *cdcn_variables)
{
    /* Multidimensional arrays, yuck. */
    static void actual_cdcn_norm();
    float *variance, *prob, *tilt, *noise, *codebook, *corrbook;
    int num_codes;

    /*
     * If error, dont bother
     */
    if (!cdcn_variables->run_cdcn)
        return;

    /*
     * If the variables haven't been intialized, dont normalize
     * else results may be diastrous
     */
    if (cdcn_variables->firstcall)
        return;

    /*
     * Open suitcase
     */
    variance	= cdcn_variables->variance;
    prob	= cdcn_variables->probs;
    tilt	= cdcn_variables->tilt;
    noise	= cdcn_variables->noise;
    codebook	= cdcn_variables->means;
    corrbook	= cdcn_variables->corrbook;
    num_codes	= cdcn_variables->num_codes;

    actual_cdcn_norm(variance, prob, tilt, noise, codebook, 
                                   corrbook, num_codes, z);
    return;
}

/*************************************************************************
 *
 * cdcn_norm finds the cepstrum vector for a single noisy vector that minimizes
 * the squared error.
 * Coded by Alex Acero (acero@s),  November 1989 
 *
 *************************************************************************/

static void
actual_cdcn_norm(float variance[][NUM_COEFF+1], /* Speech cepstral variances of modes */
		 float *prob,  /* Ratio of a-prori mode probs. to mod variance */
		 float *tilt,  /* Spectral tilt cepstrum */
		 float *noise, /* Noise estimate */
		 float means[][NUM_COEFF+1], /* The cepstrum codebook */
		 float corrbook[][NUM_COEFF+1], /* The correction factor's codebook */
		 int num_codes, /* Number of codewords in codebook */
		 float z[NUM_COEFF+1]) /* The input cepstrum */
{
    float       distance,  /* distance value */
                den,       /* Denominator for reestimation */
                fk,        /* Probabilities for different codewords */
                difference;     /* stores z - x - q - r */
    int         j,              /* Index coefficients within frame */
                k;              /* Index codewords in codebook */


    float x[NUM_COEFF+1];

    /* Initialize cleaned vector x */
    for (j = 0; j <= NUM_COEFF; j++)
        x[j] = 0.0;

    difference = z[0] - means[0][0] - corrbook[0][0] - tilt[0];
    distance = difference*difference / variance[0][0];
    for (j = 1; j <= NUM_COEFF; j++)
    {
        difference = z[j] - tilt[j] - means[0][j] - corrbook[0][j];
        distance += difference*difference / variance[0][j];
    }
    fk = exp ((double) - distance / 2) * prob[0];
    den = fk;

    /* Reestimate vector x across all codewords */
    for (k = 1; k < num_codes; k++) 
    {
        /* Find estimated vector for codeword k and update x */
        difference = z[0] - means[k][0] - corrbook[k][0] - tilt[0];
        distance = difference*difference / variance[k][0];
        for (j = 1; j <= NUM_COEFF; j++)
        {
            difference = z[j] - tilt[j] - corrbook[k][j] - means[k][j];
            distance += difference*difference / variance[k][j];
        }
        fk = exp ((double) - distance / 2) * prob[k];
        for (j = 0; j <= NUM_COEFF; j++)
            x[j] += (z[j] - tilt[j] - corrbook[k][j]) * fk;
        den += fk;
    }

    /* Normalize the estimated x vector across codewords 
     * The if test is only for sanity. It almost never fails 
     */
    if (den != 0)
        for (j = 0; j <= NUM_COEFF; j++)
            z[j] = x[j]/den;
    else
       z[j] -= tilt[j];

    /* 
     * z[] itself carries the cleaned speech now
     */
}
