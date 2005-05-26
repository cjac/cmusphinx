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
#include "mdef.h"

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

        printf("\nClass: %s, Number of frames: %d - N: %d, O: %d, S: %d, SIL: %d\n", CLASSW->classname[myclass], frame_count, classcount[CLASS_N], classcount[CLASS_O], classcount[CLASS_S], classcount[CLASS_SIL]);

}

/********************************************************************/
// This function frees the class wrapper
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

class_t * classw_initialize(char * mdeffile, char* meanfile, 
                            char *varfile, float64 varfloor,
                            char* mixwfile, float64 mixwfloor, 
                            int32 precomp, char *senmgau)
{
        class_t *CLASSW = (class_t*) calloc(1, sizeof(class_t));
        mdef_t *mdef;

        /************ Read the means, variances, and mixture weights ****************/

        /*Initialize g */
        CLASSW->g= mgau_init(meanfile,
                             varfile , varfloor,
                             mixwfile, mixwfloor, 
                             precomp,
                             senmgau,
                             FULL_FLOAT_COMP);

        mdef = mdef_init(mdeffile);

        /* Make sure we have only one emitting state */
        if (mdef_n_emit_state(mdef) != 1) {
                E_FATAL("Models required to have a single emitting state, currently have %d", 
                        mdef_n_emit_state(mdef));
        }

        /* Map classes to CI models, using the model definition */
        CLASSW->classmap[CLASS_N] = mdef_ciphone_id(mdef, "N");
        if (CLASSW->classmap[CLASS_N] == BAD_S3CIPID) {
                E_WARN("Phone N not defined in current model set\n");
        }
        CLASSW->classmap[CLASS_S] = mdef_ciphone_id(mdef, "S");
        if (CLASSW->classmap[CLASS_S] == BAD_S3CIPID) {
                E_WARN("Phone S not defined in current model set\n");
        }
        CLASSW->classmap[CLASS_SIL] = mdef_ciphone_id(mdef, "SIL");
        if (CLASSW->classmap[CLASS_SIL] == BAD_S3CIPID) {
                E_WARN("Phone SIL not defined in current model set\n");
        }
        if ((CLASSW->classmap[CLASS_N] == BAD_S3CIPID) &&
            (CLASSW->classmap[CLASS_S] == BAD_S3CIPID) &&
            (CLASSW->classmap[CLASS_SIL] == BAD_S3CIPID)) {
                E_FATAL("Model set has to have at least one of N, S, or SIL\n");
        }
        CLASSW->classmap[CLASS_O] = mdef_ciphone_id(mdef, "O");
        if (CLASSW->classmap[CLASS_O] == BAD_S3CIPID) {
                E_FATAL("Phone O not defined in current model set\n");
        }

        if (mdef != NULL) {
                mdef_free(mdef);
        }

        mgau_precomp_hack_log_to_float(CLASSW->g);

        E_INFO("Classification\n");
        E_INFO("Will use %d cepstral components\n", DIMENSIONS);

        /*  mgau_dump(CLASSW->g,MGAU_MEAN);*/
  
        /******** Set the priors of the classes *************/

        CLASSW->priors[CLASS_N] = (float32)PRIOR_N;    // N
        CLASSW->priors[CLASS_O] = (float32)PRIOR_O;    // O
        CLASSW->priors[CLASS_S] = (float32)PRIOR_S;    // S
        CLASSW->priors[CLASS_SIL] = (float32)PRIOR_SIL;  // SIL

        /********* Initialize voting window parameters **********/

        CLASSW->windowlen       = VOTEWINDOWLEN;        // voting window length

        CLASSW->postprocess     = POSTPROCESS;          // post-processing?

        if ( CLASSW->postprocess == 1)
                CLASSW->classlatency = CLASSLATENCY;            // Number of latency frames      
        else
                CLASSW->classlatency = 0;
  
        CLASSW->classname[CLASS_N] = (char*) malloc (10 * sizeof(char));
        strcpy(CLASSW->classname[CLASS_N], "Noise");
        CLASSW->classname[CLASS_O] = (char*) malloc (10 * sizeof(char));
        strcpy(CLASSW->classname[CLASS_O], "Owner");
        CLASSW->classname[CLASS_S] = (char*) malloc (10 * sizeof(char));
        strcpy(CLASSW->classname[CLASS_S], "Secondary");
        CLASSW->classname[CLASS_SIL] = (char*) malloc (10 * sizeof(char));
        strcpy(CLASSW->classname[CLASS_SIL], "Silence");

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
        
        if (windowcap < 2)
        {
                window[windowcap] = myclass;
                windowcap ++;
                *wincap = windowcap;
                return CLASS_SIL;
        }
        else if ((2 <= windowcap) && (windowcap < 4))
        {
                window[windowcap] = myclass;
                windowcap ++;
                *wincap = windowcap;
                return CLASS_SIL;
        }
        else if (windowcap == 4)
        {
                window[windowcap] = myclass;    
                windowcap ++;
                postclass = vote(window, windowcap);
                *wincap = windowcap;
                return postclass;
        }
        else
        {
                for (i = 1; i < windowlen; i++)
                        window[i - 1] = window[i];

                window[windowlen - 1] = myclass;        
                postclass = vote(window, windowcap);
                *wincap = windowcap;
                return postclass;
        }
}

int vote (int *window, int windowlen)
{
        int i, myclass, max;
        int count[NUMCLASSES];

        for (i = 0; i < NUMCLASSES; i ++)
                count[i] = 0;

        for (i = 0; i < windowlen; i ++) {
                assert (window[i] < NUMCLASSES);
                count[window[i]] ++;
        }
                
        /* Initialize the max with the silence class. */
        max = count[CLASS_SIL];
        myclass = CLASS_SIL;
        /* Now, find the most popular class. */
        for (i = 0; i < NUMCLASSES; i++) {
                if (count[i] > max)
                {
                        max = count[i];
                        myclass = i;
                }
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

void calclikeli (float *frame,    /*Input: the frame */
                 mgau_model_t *g, /*Input: multiptle mixture models */
                 float likeli[NUMCLASSES], /* Output the, likelihood for each class */
                 s3cipid_t *map) /* map between ci models and classes */
{
        int i,j,k;

        float32 a, b, pmix;     
        float32 m, v, f;
        /* a is the product(k) [1/(2 * pi * varijk )^(1/2)]*/
        /* b is the sum (k) [ -0.5 * pow ( (x[k] - meansijk), 2 )  */
        /* p is the likelihood given class and gaussian mixture */
  
        /* assert (g->n_mgau==NUMCLASSES); */

        for (i = 0; i < NUMCLASSES; i++){       /* for each class*/
                if (map[i] == BAD_S3CIPID) {
                        continue;
                }
                likeli[i] = 0;  
                for (j = 0; j < g->mgau->n_comp; j++)   {/* for each mixture in the class */
                        a = g->mgau[map[i]].lrd[j];
                        b = 0;  
                        /*NOTE: CHANGE HERE: ignore C_8 to C_12 */
                        for (k = 0; k  < DIMENSIONS; k++) {
                                f=frame[k];
                                m=g->mgau[map[i]].mean[j][k];
                                v= g->mgau[map[i]].var[j][k];

                                b += (f - m) * (f - m) * v; 
                        }       
                        if (b > 1000) /* HACK to prevent underflows on alpha
                                         (this number is arbitrary!) */
                                pmix = 0.0f;
                        else
                                pmix = (float)(a * exp (-0.5 * b));
                        likeli[i] += g->mgau[map[i]].mixw_f[j] * pmix; 
                }       
        }
}


/********* Function to classify the frame *******************/ 

int classify (float *frame, mgau_model_t *g, float priors[NUMCLASSES], 
              s3cipid_t *classmap)
{
        int i, myclass;
        float maxpost, post;
        float likeli[NUMCLASSES];

        calclikeli(frame, g, likeli, classmap);
        
        /* Second calculate the posterior       */
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
