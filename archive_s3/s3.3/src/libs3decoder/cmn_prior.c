/*************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 * Created
 */


#include "cmn_prior.h"

void cmn_prior(float32 **incep, int32 varnorm, int32 nfr, int32 ceplen, 
							   int32 endutt)
{
  static float32 *cur_mean = NULL; /* the mean subtracted from input frames */
  static float32 *sum = NULL;	     /* the sum over input frames */
  static int32   nframe;	     /* the total number of input frames */
  static int32   initialize=1;
  float32 sf;
  int32   i, j;
  
  if (varnorm)
    E_FATAL("Variance normalization not implemented in live mode decode]n");
  
  if (initialize){
    cur_mean = (float32 *) ckd_calloc(ceplen, sizeof(float32));
    
    /* A front-end dependent magic number */
    cur_mean[0] = 12.0;
    
    sum      = (float32 *) ckd_calloc(ceplen, sizeof(float32));
    nframe   = 0;
    initialize = 0;
    E_INFO("mean[0]= %.2f, mean[1..%d]= 0.0\n", cur_mean[0], ceplen-1);
  }
  
  if (nfr <= 0)
    return;
  
  for (i = 0; i < nfr; i++){
    for (j = 0; j < ceplen; j++){
      sum[j] += incep[i][j];
      incep[i][j] -= cur_mean[j];
    }
    ++nframe;
  }
  
  /* Shift buffer down if we have more than CMN_WIN_HWM frames */
  if (nframe > CMN_WIN_HWM) {
    sf = (float32) (1.0/nframe);
    for (i = 0; i < ceplen; i++)
      cur_mean[i] = sum[i] * sf;
    
    /* Make the accumulation decay exponentially */
    if (nframe >= CMN_WIN_HWM) {
      sf = CMN_WIN * sf;
      for (i = 0; i < ceplen; i++)
	sum[i] *= sf;
      nframe = CMN_WIN;
    }
  }
  
  if (endutt) {
    /* Update mean buffer */
    
    /* 01.15.01 RAH - removing this printf, it is damn annoying
       printf("Mean norm update: from <"); 
       for (i = 0; i < ceplen; i++) 
       printf("%5.2f ", cur_mean[i]); 
       printf(">\n");
    */
    
    sf = (float32) (1.0/nframe);
    for (i = 0; i < ceplen; i++)
      cur_mean[i] = sum[i] * sf;
    
    /* Make the accumulation decay exponentially */
    if (nframe > CMN_WIN_HWM) {
      sf = CMN_WIN * sf;
      for (i = 0; i < ceplen; i++)
	sum[i] *= sf;
      nframe = CMN_WIN;
    }
    
    /* 01.15.01 RAH - removing this printf, it is damn annoying
       printf("Mean norm update: to   < ");
       for (i = 0; i < ceplen; i++)
       printf("%5.2f ", cur_mean[i]);
       printf(">\n");
    */
  }
}
