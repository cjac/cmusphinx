#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*
#include <s2types.h>
*/
#include "new_fe.h"
#include "new_fe_sp.h"

/* 
 *   HISTORY
 *
 *   12-Aug-99 Created by M Seltzer for opensource SPHINX III system
 *             Based in part on past implementations by R Singh, 
 *             M Siegler, M Ravishankar, and others
 *             

      7-Feb-00 M. Seltzer - changed fe_process_utt usage. Function now
      allocated 2d feature array internally and assigns the passed
      pointer to it. This was done to allow for varying numbers of
      frames to be written when block i/o processing
      
 */  


/*********************************************************************
   FUNCTION:   fe_init
   PARAMETERS: param_t *P
   RETURNS:    pointer to a new front end or NULL if failure.
   DESCRIPTION: builds a front end instance FE according to user
   parameters in P, and completes FE structure accordingly,
   i.e. builds appropriate filters, buffers, etc. If a param in P is
   0 then the FE parameter is set to its default value as defined
   in fe.h 
   Note: if default PRE_EMPHASIS_ALPHA is changed from 0, this will be
   problematic for init of this parameter...
**********************************************************************/

fe_t *fe_init(param_t *P)
{
    fe_t  *FE = (fe_t *) calloc(1,sizeof(fe_t));

    if (FE==NULL){
	fprintf(stderr,"memory alloc failed in fe_init()\n...exiting\n");
	return(NULL);
    }
    
    /* transfer params to front end */
    fe_parse_general_params(P,FE);

    /* compute remaining FE parameters */
    FE->FRAME_SHIFT        = (int32)(FE->SAMPLING_RATE/FE->FRAME_RATE + 0.5);/* why 0.5? */
    FE->FRAME_SIZE         = (int32)(FE->WINDOW_LENGTH*FE->SAMPLING_RATE + 0.5); /* why 0.5? */
    FE->PRIOR              = 0;

    /* establish buffers for overflow samps and hamming window */
    FE->OVERFLOW_SAMPS = (int16 *)calloc(FE->FRAME_SIZE,sizeof(int16));
    FE->HAMMING_WINDOW = (double *) calloc(FE->FRAME_SIZE,sizeof(double));
    
    if (FE->OVERFLOW_SAMPS==NULL || FE->HAMMING_WINDOW==NULL){
	fprintf(stderr,"memory alloc failed in fe_init()\n...exiting\n");
	return(NULL);
    }

    /* create hamming window */    
    fe_create_hamming(FE->HAMMING_WINDOW, FE->FRAME_SIZE);
    
    /* init and fill appropriate filter structure */
    if (FE->FB_TYPE==MEL_SCALE) {   
	if ((FE->MEL_FB = (melfb_t *) calloc(1,sizeof(melfb_t)))==NULL){
	    fprintf(stderr,"memory alloc failed in fe_init()\n...exiting\n");
	    return(NULL);
	}
	/* transfer params to mel fb */
	fe_parse_melfb_params(P, FE->MEL_FB);

	fe_build_melfilters(FE->MEL_FB);
	fe_compute_melcosine(FE->MEL_FB);
    } 
    else {
	fprintf(stderr,"MEL SCALE IS CURRENTLY THE ONLY IMPLEMENTATION!\n");
	return(NULL);
    }
    return(FE);
}


/*********************************************************************
   FUNCTION: fe_start_utt
   PARAMETERS: fe_t *FE
   RETURNS: 0 if successful
   DESCRIPTION: called at the start of an utterance. resets the
   overflow buffer and activates the start flag of the front end
**********************************************************************/
int32 fe_start_utt(fe_t *FE)
{
    FE->NUM_OVERFLOW_SAMPS = 0;
    memset(FE->OVERFLOW_SAMPS,0,FE->FRAME_SIZE*sizeof(int16));
    FE->START_FLAG=1;
    FE->PRIOR = 0;
    return 0;
}


/*********************************************************************
   FUNCTION: fe_process_utt
   PARAMETERS: fe_t *FE, int16 *spch, int32 nsamps, float **cep
   RETURNS: number of frames of cepstra computed 
   DESCRIPTION: processes the given speech data and returns
   features. will prepend overflow data from last call and store new
   overflow data within the FE
**********************************************************************/
int32 fe_process_utt(fe_t *FE, int16 *spch, int32 nsamps, float ***cep_block)
{
    int32 frame_start, frame_count=0, whichframe=0;
    int32 i, spbuf_len, offset=0;  
    double *spbuf, *fr_data, *fr_fea;
    int16 *tmp_spch = spch;
    float **cep=NULL;
    
    /* are there enough samples to make at least 1 frame? */
    if (nsamps+FE->NUM_OVERFLOW_SAMPS >= FE->FRAME_SIZE){
      
      /* if there are previous samples, pre-pend them to input speech samps */
      if ((FE->NUM_OVERFLOW_SAMPS > 0)) {
	
	if ((tmp_spch = (int16 *) malloc (sizeof(int16)*(FE->NUM_OVERFLOW_SAMPS +nsamps)))==NULL){
	    fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
	    exit(0);
	}
	/* RAH */
	memcpy (tmp_spch,FE->OVERFLOW_SAMPS,FE->NUM_OVERFLOW_SAMPS*(sizeof(int16))); /* RAH */
	memcpy(tmp_spch+FE->NUM_OVERFLOW_SAMPS, spch, nsamps*(sizeof(int16))); /* RAH */
	/*	memcpy(FE->OVERFLOW_SAMPS + FE->NUM_OVERFLOW_SAMPS, spch, nsamps*(sizeof(int16)));
		spch = FE->OVERFLOW_SAMPS;*/
	nsamps += FE->NUM_OVERFLOW_SAMPS;
	FE->NUM_OVERFLOW_SAMPS = 0; /*reset overflow samps count */
      }
      /* compute how many complete frames  can be processed and which samples correspond to those samps */
      frame_count=0;
      for (frame_start=0; frame_start+FE->FRAME_SIZE <= nsamps; frame_start += FE->FRAME_SHIFT)
	frame_count++;


      if (cep!=NULL) fe_free_2d((void**)cep);
      /* 01.14.01 RAH, added +1 */
      cep = (float **)fe_create_2d(frame_count+1,FE->NUM_CEPSTRA,sizeof(float)); /*MLS*/

      spbuf_len = (frame_count-1)*FE->FRAME_SHIFT + FE->FRAME_SIZE;    
      /* assert(spbuf_len <= nsamps);*/
      if ((spbuf=(double *)calloc(spbuf_len, sizeof(double)))==NULL){
	  fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
	  exit(0);
      }
      
      /* pre-emphasis if needed,convert from int16 to double */
      if (FE->PRE_EMPHASIS_ALPHA != 0.0){
	fe_pre_emphasis(tmp_spch, spbuf, spbuf_len, FE->PRE_EMPHASIS_ALPHA, FE->PRIOR);
      } else{
	fe_short_to_double(tmp_spch, spbuf, spbuf_len);
      }
      
      /* frame based processing - let's make some cepstra... */    
      fr_data = (double *)calloc(FE->FRAME_SIZE, sizeof(double));
      fr_fea = (double *)calloc(FE->NUM_CEPSTRA, sizeof(double));
      
      if (fr_data==NULL || fr_fea==NULL){
	  fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
	  exit(0);
      }
      for (whichframe=0;whichframe<frame_count;whichframe++){

	for (i=0;i<FE->FRAME_SIZE;i++)
	  fr_data[i] = spbuf[whichframe*FE->FRAME_SHIFT + i];
	
	fe_hamming_window(fr_data, FE->HAMMING_WINDOW, FE->FRAME_SIZE);
	
	fe_frame_to_fea(FE, fr_data, fr_fea);
	
	for (i=0;i<FE->NUM_CEPSTRA;i++)
	  cep[whichframe][i] = (float)fr_fea[i];
      }
      /* done making cepstra */
      
      
      /* assign samples which don't fill an entire frame to FE overflow buffer for use on next pass */
      if (spbuf_len < nsamps)	{
	offset = ((frame_count)*FE->FRAME_SHIFT);
	memcpy(FE->OVERFLOW_SAMPS,tmp_spch+offset,(nsamps-offset)*sizeof(int16));
	FE->NUM_OVERFLOW_SAMPS = nsamps-offset;
	FE->PRIOR = tmp_spch[offset-1];
	assert(FE->NUM_OVERFLOW_SAMPS<FE->FRAME_SIZE);
      }
      
      if (spch != tmp_spch) 
	free (tmp_spch);
      
      free(spbuf);
      free(fr_data);
      free(fr_fea);
    }
    
    /* if not enough total samps for a single frame, append new samps to
       previously stored overlap samples */
    else { 
      memcpy(FE->OVERFLOW_SAMPS+FE->NUM_OVERFLOW_SAMPS,tmp_spch, nsamps*(sizeof(int16)));
      FE->NUM_OVERFLOW_SAMPS += nsamps;
      assert(FE->NUM_OVERFLOW_SAMPS < FE->FRAME_SIZE);
      frame_count=0;
    }

    *cep_block = cep; /* MLS */
    return frame_count;
}


/*********************************************************************
   FUNCTION: fe_end_utt
   PARAMETERS: fe_t *FE, float *cepvector
   RETURNS: number of frames processed (0 or 1) 
   DESCRIPTION: if there are overflow samples remaining, it will pad
   with zeros to make a complete frame and then process to
   cepstra. also deactivates start flag of FE, and resets overflow
   buffer count. 
**********************************************************************/
int32 fe_end_utt(fe_t *FE, float *cepvector)
{
  int32 pad_len=0, frame_count=0;
  int32 i;
  double *spbuf, *fr_fea = NULL;
  
  /* if there are any samples left in overflow buffer, pad zeros to
     make a frame and then process that frame */
  
  if ((FE->NUM_OVERFLOW_SAMPS > 0)) { 
    pad_len = FE->FRAME_SIZE - FE->NUM_OVERFLOW_SAMPS;
    memset(FE->OVERFLOW_SAMPS+(FE->NUM_OVERFLOW_SAMPS),0,pad_len*sizeof(int16));
    FE->NUM_OVERFLOW_SAMPS += pad_len;
    assert(FE->NUM_OVERFLOW_SAMPS==FE->FRAME_SIZE);
    
    if ((spbuf=(double *)calloc(FE->FRAME_SIZE,sizeof(double)))==NULL){
	fprintf(stderr,"memory alloc failed in fe_end_utt()\n...exiting\n");
	exit(0);
    }
 
    if (FE->PRE_EMPHASIS_ALPHA != 0.0){
      fe_pre_emphasis(FE->OVERFLOW_SAMPS, spbuf, FE->FRAME_SIZE,FE->PRE_EMPHASIS_ALPHA, FE->PRIOR);
    } else {
      fe_short_to_double(FE->OVERFLOW_SAMPS, spbuf, FE->FRAME_SIZE);
    }
    
    /* again, who should implement cep vector? this can be implemented
       easily from outside or easily from in here */
    if ((fr_fea = (double *)calloc(FE->NUM_CEPSTRA, sizeof(double)))==NULL){
	fprintf(stderr,"memory alloc failed in fe_end_utt()\n...exiting\n");
	exit(0);
    }

    fe_hamming_window(spbuf, FE->HAMMING_WINDOW, FE->FRAME_SIZE);
    fe_frame_to_fea(FE, spbuf, fr_fea);	
    for (i=0;i<FE->NUM_CEPSTRA;i++)
      cepvector[i] = (float)fr_fea[i];
    frame_count=1;
    free(fr_fea);		/* RAH - moved up */
    free (spbuf);		/* RAH */
  } else {
    frame_count=0;
    cepvector = NULL;
  }
  
  /* reset overflow buffers... */
  FE->NUM_OVERFLOW_SAMPS = 0;
  FE->START_FLAG=0;
  
  return frame_count;
}

/*********************************************************************
   FUNCTION: fe_close
   PARAMETERS: fe_t *FE
   RETURNS: 
   DESCRIPTION: free all allocated memory within FE and destroy FE 
**********************************************************************/

int32 fe_close(fe_t *FE)
{
  /* kill FE instance - free everything... */
  if (FE->FB_TYPE==MEL_SCALE) {
    fe_free_2d((void *)FE->MEL_FB->filter_coeffs);
    fe_free_2d((void *)FE->MEL_FB->mel_cosine);
    free(FE->MEL_FB->left_apex);
    free(FE->MEL_FB->width);
    free(FE->MEL_FB);
  } else {
    fprintf(stderr,"MEL SCALE IS CURRENTLY THE ONLY IMPLEMENTATION!\n");
  }
    
  free(FE->OVERFLOW_SAMPS);
  free(FE);
  return(0);
}
