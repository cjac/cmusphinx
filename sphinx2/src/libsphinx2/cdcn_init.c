/*---------------------------------------------------------------------------*
 * This routine reads the text file holding mean, variance and mode 
 * probablities of the mixture density parameters computed by EM. It returns 
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
