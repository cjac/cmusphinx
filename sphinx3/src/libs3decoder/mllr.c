/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * mllr.c -- Application of MLLR regression matrices to codebook means
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
 * 24-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon University
 *              First checked in from sphinx 3.0 to sphinx 3.5
 * 
 * 02-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added reading of MLLR classes in transformation file.  Currently must
 * 		be 1.
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started (copied from Vipul Parikh's implementation).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mllr.h"

void mllr_dump(float32 **A, float32 *B,int32 veclen)
{
  int32 i,j;
  char* tmpstr;
  assert(A!=NULL);
  assert(B!=NULL);



  tmpstr=ckd_calloc((veclen * 20),sizeof(char));

  for(i=0;i<veclen;i++){
    sprintf(tmpstr,"A %d ",i);
    for(j=0;j<veclen;j++){
      sprintf(tmpstr,"%s %f ",tmpstr, A[i][j]);
    }
    sprintf(tmpstr,"%s\n",tmpstr);

    E_INFO("%s\n",tmpstr);
  }
  
  sprintf(tmpstr,"B\n");
  for(i=0;i<veclen;i++){
    sprintf(tmpstr,"%s %f ",tmpstr, B[i]);
  }
  sprintf(tmpstr,"%s \n",tmpstr);
  E_INFO("%s\n",tmpstr);


  ckd_free(tmpstr);
}

int32 mllr_read_regmat (const char *regmatfile,
			float32 ***A,
			float32 **B,
			int32 ceplen
			)
{
    int32 j, k, n;
    FILE  *fp;
    float32 **lA, *lB;

    if ((fp = fopen(regmatfile, "r")) == NULL) {
	E_ERROR ("fopen(%s,r) failed\n", regmatfile);
	return -1;
    } else
	E_INFO ("Reading MLLR transformation file %s\n", regmatfile);

    lA = (float32 **) ckd_calloc_2d (ceplen, ceplen, sizeof(float32));    
    lB = (float32 *) ckd_calloc (ceplen, sizeof (float32 ));

    /* Read #MLLR-classes; must be 1 for now (rkm@cs.cmu.edu, 12-Dec-1996) */
    if ((fscanf (fp, "%d", &n) != 1) || (n != 1))
	goto readerror;

    /* The number of stream must be 1 for now (archan@cs.cmu.edu 24-Jul-2004) */
    if ((fscanf (fp, "%d", &n) != 1) || (n != 1))
	goto readerror;
    
    if ((fscanf(fp, "%d", &n) != 1) || (ceplen != n))
      goto readerror;
    
    for (j = 0; j < ceplen; j++) {
      for (k = 0; k < ceplen; ++k) {
	if (fscanf(fp, "%f ", &lA[j][k]) != 1)
	  goto readerror;
      }
    }
    for (j = 0; j < ceplen; j++) {
      if (fscanf(fp, "%f ", &lB[j]) != 1)
	goto readerror;
    }

    *A = lA;
    *B = lB;

    fclose(fp);

    return 0;

readerror:
    E_ERROR("Error reading MLLR file %s\n", regmatfile);
    ckd_free_2d ((void **)lA);
    ckd_free (lB);

    fclose (fp);
    
    *A = NULL;
    *B = NULL;
    
    return -1;
}


int32 mllr_free_regmat (float32 **A,
			float32 *B
			)
{
    ckd_free_2d ((void **) A);
    ckd_free (B);
    return 0;
}



int32 mllr_norm_mgau (mgau_model_t *mgauset,
		      float32 **A,
		      float32 *B,
		      mdef_t *mdef
		      )
{
    int32 d, c, l, m;
    float32 *temp;

    int32 n_density=mgauset->n_mgau;
    int32 n_mix=mgauset->max_comp;
    int32 ceplen=mgauset->veclen;
    mgau_t *mgau;

    /* Transform codebook for each stream s */
    temp = (float32 *) ckd_calloc (ceplen, sizeof(float32));
    
    /* Transform each density d in selected codebook */

    for (d = 0; d < n_density; d++) {
      mgau=mgauset->mgau;

      if(mdef->cd2cisen[d]!=d){ /* If d is a CD senone */
	for(c = 0 ; c < n_mix ; c++){
	  for (l = 0; l < ceplen; l++){
	    temp[l] = 0.0;
	    for (m = 0; m < ceplen; m++) {
	      temp[l] += A[l][m] * mgau[d].mean[c][m];
	    }
	    temp[l] += B[l];
	  }

	  for(l=0 ; l < ceplen ;l++){
	    mgau[d].mean[c][l] = temp[l];
	  }
	}
      }
    }

    ckd_free (temp);

    return 0;
} 



