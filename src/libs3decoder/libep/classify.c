 /* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights
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

/*************************************************
 * CMU CALO Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * 17-Jun-2004  Ziad Al Bawab (ziada@cs.cmu.edu) at Carnegie Mellon University
 * Created
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 

#include "classify.h"


/********************************************************************/
// This function reports the majority class 
/*********************************************************************/
void majority_class(class_t *CLASSW, int *classcount, int frame_count)
{
  int i, max, myclass;

  max = 0;
  myclass = 0;

  for( i = 0; i <NUMCLASSES; i++){
    if(classcount[i] >= max){
      max = classcount[i];
      myclass = i;
    }
  }

  printf("\nClass: %s, Number of frames: %d - N: %d, O: %d, S: %d, SIL: %d\n",CLASSW->classname[myclass],frame_count, classcount[0], classcount[1], classcount[2], classcount[3]);

}

/********************************************************************/
// This function initializes the class wrapper
/*********************************************************************/
void classw_free(class_t *CLASSW)
{
  int i;
  if(CLASSW->g)
    mgau_free(CLASSW->g);

  for(i=0;i<NUMCLASSES;i++){
    if(CLASSW->classname[i])
      ckd_free(CLASSW->classname[i]);
  }

  ckd_free(CLASSW);
}



/*********************************************************************/
// This function initializes the class wrapper
/*********************************************************************/

class_t * classw_initialize(char* meanfile, char *varfile, float64 varfloor,
			    char* mixwfile, float64 mixwfloor, int32 precomp, char *senmgau)
{
  class_t *CLASSW = (class_t*) calloc(1,sizeof(class_t));

  /************ Read the means, variances, and mixture weights ****************/

  /*Initialize g */
  CLASSW->g= mgau_init(meanfile,
		       varfile , varfloor,
		       mixwfile, mixwfloor, 
		       precomp,
		       senmgau,
		       FULL_FLOAT_COMP);

  mgau_precomp_hack_log_to_float(CLASSW->g);
  E_INFO("Classification\n");
  /*  mgau_dump(CLASSW->g,MGAU_MEAN);*/
  
  /******** Set the priors of the classes *************/

  CLASSW->priors[0] = PRIOR_N;    // N
  CLASSW->priors[1] = PRIOR_O;    // O
  CLASSW->priors[2] = PRIOR_S;    // S
  CLASSW->priors[3] = PRIOR_SIL;  // SIL

  /********* Initialize voting window parameters **********/

  CLASSW->windowlen 	= VOTEWINDOWLEN;      	// voting window length

  CLASSW->postprocess 	= POSTPROCESS;      	// post-processing?

  if ( CLASSW->postprocess == 1)
    CLASSW->classlatency = CLASSLATENCY;     	// Number of latency frames	 
  else
    CLASSW->classlatency = 0;
  
  CLASSW->classname[0] = (char*) malloc ( 10 * sizeof(char));
  strcpy(CLASSW->classname[0], "Noise");
  CLASSW->classname[1] = (char*) malloc ( 10 * sizeof(char));
  strcpy(CLASSW->classname[1], "Owner");
  CLASSW->classname[2] = (char*) malloc ( 10 * sizeof(char));
  strcpy(CLASSW->classname[2], "Secondary");
  CLASSW->classname[3] = (char*) malloc ( 10 * sizeof(char));
  strcpy(CLASSW->classname[3] ,"Silence");

  return(CLASSW);
}




/*********************************************************************/
// This function post-processes the classification results with a
// voting window of 5 frames 					      
/*********************************************************************/

int postclassify (int *window, int windowlen, int *wincap, int myclass)
{
	int i, postclass, windowcap;

	windowcap = *wincap;	
	
	if(windowcap < 2)
	{
		window[windowcap] = myclass;
		windowcap ++;
		*wincap = windowcap;
		return 4;
	}
	else if( (2 <= windowcap) && (windowcap < 4) )
 	{
		window[windowcap] = myclass;
                windowcap ++;
		*wincap = windowcap;
                return 3;
	}
	else if ( windowcap == 4)
	{
		window[windowcap] = myclass;	
                windowcap ++;
		postclass = vote(window,windowlen);
		*wincap = windowcap;
		return postclass;
	}
	else
	{
		for (i = 1; i < windowlen; i++)
			window[i - 1] = window[i];

		window[windowlen - 1] = myclass;	
		postclass = vote(window,windowlen);
		*wincap = windowcap;
		return postclass;
	}
}

int vote (int *window, int windowlen)
{
	int i, myclass, max;
	int count[VOTEWINDOWLEN];

	for (i = 0; i < windowlen; i ++)
		count[i] = 0;

	for (i = 0; i < windowlen; i ++)
		count[window[i]] ++;
		
	max = count[3];
	myclass = 3;

	for (i = 0; i < windowlen; i++)
		if (count[i] > max)
		{
			max = count[i];
			myclass = i;
		}

	return myclass;
}

/******************* Function to read the features values ******************/

void readfeatures (char *filename, float *array[MAXFRAMES], int *numofframes)
{
  int i,j;
  int totalfeatures, totalframes;
  FILE *binfile;

  binfile = fopen (filename, "rb");
  
  if ( binfile == NULL)
    E_INFO ("Binary File %s couldn't be opened!\n",filename);
  
  fread(&totalfeatures, sizeof(int),1,binfile);
  
  SWAP_INT(&totalfeatures);	
  
  totalframes = totalfeatures/DIMENSIONS;
  *numofframes = totalframes;
  
  for (i = 0; i < totalframes; i++){
    array[i] = (float *) malloc (sizeof(float) * DIMENSIONS);
    fread(array[i], sizeof(float),DIMENSIONS, binfile);
    for (j =0; j < DIMENSIONS; j++) {
      SWAP_FLOAT(&array[i][j]);
    }
  }

        fclose(binfile);

}




/******** Function to calculate the likelihood for each class given the frame ****************/
/* Only work when mgau_precomp_hack_log_to_float is run before it */

void calclikeli (mgau_model_t *g, /*Input: multiptle mixture models */
		 float *frame,    /*Input: the frame */
		 float likeli[NUMCLASSES]) /* Output the, likelihood for each class */
{
  int i,j,k;
  
  float a, b, pmix;	
  float m, v, f;
  /* a is the product(k) [1/(2 * pi * varijk )^(1/2)]*/
  /* b is the sum (k) [ -0.5 * pow ( (x[k] - meansijk), 2 )  */
  /* p is the likelihood given class and gaussian mixture */
  
  assert (g->n_mgau==NUMCLASSES);

  for (i = 0; i < g->n_mgau; i++){	/* for each class*/
      likeli[i] = 0;	
      for (j = 0; j < g->mgau->n_comp; j++)	{/* for each mixture in the class */
	a = g->mgau[i].lrd[j];
	b = 0;	
	/*NOTE: CHANGE HERE: ignore C_8 to C_12 */
	for (k = 0; k  < DIMENSIONS; k++) {
	  f=frame[k];
	  m=g->mgau[i].mean[j][k];
	  v= g->mgau[i].var[j][k];

	  b += (f - m) * (f - m) * v; 
	}	
	pmix = a * exp (-0.5 * b);	
	likeli[i] += g->mgau[i].mixw_f[j] * pmix; 
      }	
  }
}


/********* Function to classify the frame *******************/ 

int classify (mgau_model_t *g,float *frame, float priors[NUMCLASSES])
{
  int i, myclass;
  float maxpost, post;
  float likeli[NUMCLASSES];

  calclikeli(g, frame, likeli);
	
  /* Second calculate the posterior	*/
  maxpost = 0;
  myclass = 0;
  
  for (i = 0; i < NUMCLASSES; i++){
      post = likeli[i] * priors[i];
#if 0
      E_INFO("Posterior Probabilities %f, likelihood %f, priors %f\n",post,likeli[i],priors[i]);
#endif
      if (maxpost < post) {
	   myclass = i;
	  maxpost = post;
      }
  }
  return(myclass);

}



