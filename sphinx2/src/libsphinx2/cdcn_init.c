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

/*---------------------------------------------------------------------------*
 * This routine reads the text file holding mean, variance and mode probablities
 * of the mixture density parameters computed by EM. It returns 
 * prob/sqrt(mod var) instead of var to simplify computation in CDCN
 ----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "cdcn.h"

int
cdcn_init(char const *filename, CDCN_type *cdcn_variables)
{
    int      i,j,count,count2,ndim,ncodes;

    float    *codebuff;
    float    *varbuff;
    float    *probbuff;
    float    temp;

    FILE     *codefile;

    /*
     * Initialize run_cdcn flag to 1. If an error occurs this is reset to 0
     */
    cdcn_variables->run_cdcn = TRUE;

    codefile = fopen(filename,"r");
    if (codefile==NULL)
    {
        printf("Unable to open Codebook file\n");
        printf("Unable to run CDCN. Will not process cepstra\n");
        cdcn_variables->run_cdcn = FALSE;
        return(-1);
    }

    if (fscanf(codefile,"%d %d",&(cdcn_variables->num_codes),&ndim) == 0)
    {
        printf("Error in format of cdcn statistics file\n");
        printf("Unable to run CDCN. Will not process cepstra\n");
        cdcn_variables->run_cdcn = FALSE;
        return(-1);
    }
    ncodes = cdcn_variables->num_codes;
    codebuff = (float *)malloc(ncodes*ndim*sizeof(float));
    if (codebuff == NULL)
    {
        printf("Unable to allocate space for codebook\n");
        printf("Unable to run CDCN. Will not process cepstra\n");
        cdcn_variables->run_cdcn = FALSE;
        return(-1);
    }
    varbuff = (float *)malloc(ndim*ncodes*sizeof(float));
    if (varbuff == NULL)
    {
        printf("Unable to allocate space for variances\n");
        printf("Unable to run CDCN. Will not process cepstra\n");
        cdcn_variables->run_cdcn = FALSE;
        return(-1);
    }
    probbuff = (float *)malloc(ncodes*sizeof(float));
    if (probbuff == NULL)
    {
        printf("Unable to allocate space for mode probabilites\n");
        printf("Unable to run CDCN. Will not process cepstra\n");
        cdcn_variables->run_cdcn = FALSE;
        return(-1);
    }
    count = 0;count2=0;
    for(i=0;i < ncodes ;++i)
    {
        if (fscanf(codefile,"%f",&probbuff[i]) == 0)
        {
            printf("Error in format of cdcn statistics file\n");
            printf("Unable to run CDCN. Will not process cepstra\n");
            cdcn_variables->run_cdcn = FALSE;
            return(-1);
        }
        for (j=0;j<ndim;++j)
        {
            if (fscanf(codefile,"%f",&codebuff[count]) == 0)
            {
                printf("Error in format of cdcn statistics file\n");
                printf("Unable to run CDCN. Will not process cepstra\n");
                cdcn_variables->run_cdcn = FALSE;
                return(-1);
            }
            ++count;
        }
	temp = 1;
        for (j=0;j<ndim;++j)
        {
            if (fscanf(codefile,"%f",&varbuff[count2]) == 0)
            {
                printf("Error in format of cdcn statistics file\n");
                printf("Unable to run CDCN. Will not process cepstra\n");
                cdcn_variables->run_cdcn = FALSE;
                return(-1);
            }
	    temp *= varbuff[count2];
            ++count2;
        }
	if ((temp = sqrt(temp)) == 0)
        {
            printf("Error in format of cdcn statistics file\n");
            printf("Unable to run CDCN. Will not process cepstra\n");
            cdcn_variables->run_cdcn = FALSE;
            return(-1);
        }
	probbuff[i] /= temp;
    }

    fclose(codefile);
    cdcn_variables->means = codebuff;
    cdcn_variables->variance = varbuff;
    cdcn_variables->probs = probbuff;
    cdcn_variables->firstcall = TRUE;

    cdcn_variables->corrbook = 
                (float *) malloc (ncodes *(NUM_COEFF + 1) * sizeof (float));
    if (cdcn_variables->corrbook == NULL)
    {
        printf("Unable to allocate space for correction terms\n");
        printf("Unable to run CDCN. Will not process cepstra\n");
        cdcn_variables->run_cdcn = FALSE;
        return(-1);
    }

    return(0);
}
