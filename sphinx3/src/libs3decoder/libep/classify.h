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

#include "libutil/libutil.h"
#include "cont_mgau.h"

#ifdef __cplusplus
 extern "C" {
 #endif 

#ifndef __FRAME_CLASSIFIER__
#define __FRAME_CLASSIFIER__

/******** Set the parameters of the classes *************/

#define NUMCLASSES      4	// Number of classes

/*
#define NUMMIXTURES     32 	// Number of gaussian mixtures used in classification
*/
#define DIMENSIONS      13	// Length of the feature vector
#define MAXFRAMES       10000	

/******** Set the names of the classes *************/
// this is how the mdef file arranged the models

#define CLASS_N         0 	// Noise
#define CLASS_O         1 	// Owner speech
#define CLASS_S         2 	// Secondary speech
#define CLASS_SIL       3 	// Silence

/******** Set the priors of the classes *************/

#define PRIOR_N  	0.1 	// N
#define PRIOR_O  	0.4 	// O
#define PRIOR_S  	0.1 	// S
#define PRIOR_SIL  	0.4 	// SIL

/****************************************************/


/******** Set the width of the voting Window *************/

#define VOTEWINDOWLEN 	5	// Don't change this number as the code expects 5, or you have to 
				// change the code

#define CLASSLATENCY  	2	// Number of latency frames caused by the post processing (voting window)

#define POSTPROCESS	1	// Enabling/disabling post-processing

/**************************************************/

/*** class to store the classifier parameters ***/ 
typedef struct{
  char *classname[NUMCLASSES];
  int32 windowlen;
  mgau_model_t *g ; 

  float32 priors[NUMCLASSES];

  int32 window[VOTEWINDOWLEN];              // the voting window contains class numbers
  int32 postprocess;
  int32 classlatency;
}class_t;





/* Macro to byteswap an int variable.  x = ptr to variable */
#define SWAP_INT(x)   *(x) = ((0x000000ff & (*(x))>>24) | \
                                (0x0000ff00 & (*(x))>>8) | \
                                (0x00ff0000 & (*(x))<<8) | \
                                (0xff000000 & (*(x))<<24))
/* Macro to byteswap a float variable.  x = ptr to variable */
#define SWAP_FLOAT(x) SWAP_INT((int *) x)

void majority_class(class_t* CLASSW, int *classcount, int frame_count);

class_t * classw_initialize(char* meanfile,   /* The mean file */
			    char *varfile,       /* The variance file */
			    float64 varfloor,    /* variance floor */
			    char* mixwfile,         /* The mixture weight */
			    float64 mixwfloor,       /* mixture weight floor */
			    int32 precomp,       /* pre-computation of values, 0, not to pre-compute 1, 
						    to precompute */
			    char *senmgau);       /* whether it is SCHMM, ".semi." or FCHMM ".cont." */

void classw_free(class_t *CLASSW);

/*Not open this function for now */
/*void readfeatures (char *filename, float *features[MAXFRAMES], int *numofframes);*/

/*void calclikeli (float *means[NUMCLASSES][NUMMIXTURES], 
		 float *vars[NUMCLASSES][NUMMIXTURES], 
		 float mixwghts[NUMCLASSES][NUMMIXTURES], 
		 float *frame, 
		 float likeli[NUMCLASSES]);*/

/*void calclikeli (mgau_model_t *g, Input: multiptle mixture models 
		 float *frame,    Input: the frame 
		 float likeli[NUMCLASSES]);*/

/*int classify (float *means[NUMCLASSES][NUMMIXTURES], 
	      float *vars[NUMCLASSES][NUMMIXTURES], 
	      float mixwghts[NUMCLASSES][NUMMIXTURES], 
	      float *frame, 
	      float priors[NUMCLASSES]);*/

int classify (mgau_model_t *g,  /* multiple mixture models */
	      float *frame,     /* the frame */
	      float priors[NUMCLASSES]); /* The prior of each classes */

int postclassify (int *window, int windowlen, int *wincap, int myclass);

int vote (int *window, int windowlen);

#endif /*__FRAME_CLASSIFIER__*/

#ifdef __cplusplus
 }
#endif 

