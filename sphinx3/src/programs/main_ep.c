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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
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
#include <stdio.h>
#include <stdlib.h>
#if !defined(WIN32)
#include <unistd.h>
#include <sys/file.h>
#if !defined(O_BINARY)
#define O_BINARY 0
#endif
#endif
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>

#if defined(WIN32)
#include <io.h>
#include <errno.h>
#define srand48(x) srand(x)
#define lrand48() rand()
#endif

#include "cont_mgau.h"
#include "classify.h"
#include "endptr.h"
#include "fe.h"

/** \file main_ep.c
    \brief Driver for end-pointing
 */

const char helpstr[] =
  "Description: \n\
Create cepstra from audio file.\n		\
									\
The main parameters that affect the final output, with typical values, are:\n \
									\
samprate, typically 8000, 11025, or 16000\n				\
lowerf, 130, 200, 130, for the respective sampling rates above\n	\
upperf, 3700, 5200, 6800, for the respective sampling rates above\n	\
nfilt, 31, 37, 40, for the respective sampling rates above\n		\
nfft, 256 or 512\n							\
format, raw or nist or mswav\n						\
\"";

const char examplestr[] =
  "Example: \n\
This example creates a cepstral file named \"output.mfc\" from an input audio file named \"input.raw\", which is a raw audio file (no header information), which was originally sampled at 16kHz. \n \
\n									\
ep -i  input.raw \n						\
        -o   output.mfc \n						\
        -raw no \n							\
        -input_endian little \n						\
        -samprate  16000 \n						\
        -lowerf    130 \n						\
        -upperf    6800 \n						\
        -nfilt     40 \n						\
        -nfft      512";

static arg_t arg[] = {

#if 0 /* ARCHAN: temporarily removed in 3.5 release */
  { "-help",
    ARG_INT32,
    "0",
    "Shows the usage of the tool"},
  { "-example",
    ARG_INT32,
    "0",
    "Shows example of how to use the tool"},
#endif

  { "-i",
    ARG_STRING,
    NULL,
    "Single audio input file" },
  { "-o",
    ARG_STRING,
    NULL,
    "Single cepstral output file" },
  { "-c",
    ARG_STRING,
    NULL,
    "Control file for batch processing" },
  { "-di",
    ARG_STRING,
    NULL,
    "Input directory, input file names are relative to this, if defined" },
  { "-ei",
    ARG_STRING,
    NULL,
    "input extension to be applied to all input files" },
  { "-do",
    ARG_STRING,
    NULL,
    "Output directory, output files are relative to this" },
  { "-eo",
    ARG_STRING,
    NULL,
    "Output extension to be applied to all output files" },
  { "-nist",
    ARG_INT32,
    "0",
    "Defines input format as NIST sphere" },
  { "-raw",
    ARG_INT32,
    "0",
    "Defines input format as raw binary data" },
  { "-mswav",
    ARG_INT32,
    "0",
    "Defines input format as Microsoft Wav (RIFF)" },
  { "-input_endian",
    ARG_STRING,
    "little",
    "Endianness of input data, big or little, ignored if NIST or MS Wav" },
  { "-nchans",
    ARG_INT32,
    "1",
    "Number of channels of data (interlaced samples assumed)" },
  { "-whichchan",
    ARG_INT32,
    "1",
    "Channel to process" },
  { "-logspec",
    ARG_INT32,
    "0",
    "Write out logspectral files instead of cepstra" },
  { "-feat",
    ARG_STRING,
    "sphinx",
    "SPHINX format - big endian" },
  { "-mach_endian",
    ARG_STRING,
#ifdef WORDS_BIGENDIAN
    "big",
#else
    "little",
#endif
    "Endianness of machine, big or little" },
  { "-alpha",
    ARG_FLOAT32,
    "0.97",
    "Preemphasis parameter" },
  { "-srate",
    ARG_FLOAT32,
    "16000.0",
    "Sampling rate" },
  { "-frate",
    ARG_INT32,
    "100",
    "Frame rate" },
  { "-wlen",
    ARG_FLOAT32,
    "0.0256",
    "Hamming window length" },
  { "-nfft",
    ARG_INT32,
    "512",
    "Size of FFT" },
  { "-nfilt",
    ARG_INT32,
    "40",
    "Number of filter banks" },
  { "-lowerf",
    ARG_FLOAT32,
    "200",
    "Lower edge of filters" },
  { "-upperf",
    ARG_FLOAT32,
    "3500",
    "Upper edge of filters" },
  { "-ncep",
    ARG_INT32,
    "13",
    "Number of cep coefficients" },
  { "-doublebw",
    ARG_INT32,
    "0",
    "Use double bandwidth filters (same center freq)" },
  { "-blocksize",
    ARG_INT32,
    "200000",
    "Block size, used to limit the number of samples used at a time when reading very large audio files" },
  { "-dither",
    ARG_INT32,
    "0",
    "Add 1/2-bit noise" },
  { "-verbose",
    ARG_INT32,
    "0",
    "Show input filenames" },
  { "-mean",
    ARG_STRING,
    NULL,
    "The mean file" },
  { "-var",
    ARG_STRING,
    NULL,
    "The var file" },
  { "-varfloor",
    ARG_FLOAT32,
    "0.0001",
    "Mixture gaussian variance floor (applied to data from -var file)" },
  { "-mixw",
    ARG_STRING,
    NULL,
    "The mixture weight file" },
  { "-mixwfloor",
    ARG_FLOAT32,
    "0.0000001",
    "Senone mixture weights floor (applied to data from -mixw file)" },
  { "-logfn",
    ARG_STRING,
    NULL,
    "Log file (default stdout/stderr)" },

  { NULL, ARG_INT32,  NULL, NULL }
};

void process_fe_class (fewrap_t *FEW,class_t *CLASSW,endpointer_t *ENDPTR,int16 *spbuffer,int32 splen);

/*       
	26-Aug-04 Archan - modified the code to make it incorporate sphinx's library.  
	24-Jun-04 Z. Al Bawab - modified the code to integrate with the Meeting 
	recorder.
	 
	9-Mar-04 Z. Al Bawab - modified the code to perform frame by frame 
   	 features extraction and classifcation.

	 7-Feb-00 M. Seltzer - wrapper created for new front end -
	 does blockstyle processing if necessary. If input stream is
	 greater than DEFAULT_BLOCKSIZE samples (currently 200000)
	 then it will read and write in DEFAULT_BLOCKSIZE chunks. 
	 
	 Had to change fe_process_utt(). Now the 2d feature array
	 is allocated internally to that function rather than
	 externally in the wrapper. 
	 
	 Added usage display with -help switch for help

	 14-Feb-00 M. Seltzer - added NIST header parsing for 
	 big endian/little endian parsing. kind of a hack.

	 changed -wav switch to -nist to avoid future confusion with
	 MS wav files
	 
	 added -mach_endian switch to specify machine's byte format
*/

void process_fe_class(fewrap_t *FEW, class_t *CLASSW, endpointer_t *ENDPTR, int16 *spbuffer, int32 splen)
{

  int32 spbuf_len, offset, nsamps,  frame_count, frame_start;
  int16 *spdata;
  int i, myclass, postclass, wincap, endutt;
  int classcount[NUMCLASSES];
  mgau_model_t *mgau ;

  /*****************************************************************/
  /********************** INITIALIZING COMPONENTS ******************/
  /*****************************************************************/
  
  endutt = 0;      
  
  wincap = 0;			// voting window current capacity

  for (i = 0; i < NUMCLASSES; i++)
    classcount[i] = 0;	
	
  mgau=CLASSW->g;

  /***************** DONE WITH INITIALIZATIONS ********************/   
  /****************************************************************/

  nsamps = splen;
  //spdata = spbuffer;
  
  E_INFO("%d %d %d\n",nsamps,FEW->FE->NUM_OVERFLOW_SAMPS, FEW->FE->FRAME_SIZE);

  /* are there enough samples to make at least 1 frame? */
  if (nsamps + FEW->FE->NUM_OVERFLOW_SAMPS >= FEW->FE->FRAME_SIZE)
    {
      /****************************************************************/
      /****** Prepend any overflow data from last call   **************/ 
      /****************************************************************/
      
      /* if there are previous samples, pre-pend them to input speech samps */
      if ((FEW->FE->NUM_OVERFLOW_SAMPS > 0)) {
	  if ((spdata = (int16 *) malloc (sizeof(int16)*(FEW->FE->NUM_OVERFLOW_SAMPS +nsamps)))==NULL){
	      E_FATAL("memory alloc failed in process_fe_class()...exiting\n");
	    }
	  /* RAH */
	  memcpy (spdata,FEW->FE->OVERFLOW_SAMPS,FEW->FE->NUM_OVERFLOW_SAMPS*(sizeof(int16))); 
	  memcpy(spdata+FEW->FE->NUM_OVERFLOW_SAMPS, spbuffer, nsamps*(sizeof(int16))); 
	  nsamps += FEW->FE->NUM_OVERFLOW_SAMPS;
	  FEW->FE->NUM_OVERFLOW_SAMPS = 0; /*reset overflow samps count */
	}
      else {
	  if ((spdata = (int16 *) malloc (sizeof(int16)*nsamps)) == NULL){
	      E_FATAL("memory alloc failed in fe_process_utt()...exiting\n");
	  }
	  /* RAH */
	  E_INFO("number of samples %d\n",nsamps);
	  memcpy(spdata, spbuffer, nsamps*(sizeof(int16)));
      }

      /****************************************************************/      
      /**************** Start the processing **************************/
      /****************************************************************/

      // initialize the frame_count and frame_start each time a new buffer of speech is available
      frame_count = 0;
      frame_start = 0;
	    
      while(frame_start+FEW->FE->FRAME_SIZE <= nsamps){
	for (i=0;i<FEW->FE->FRAME_SIZE;i++)
	  FEW->fr_data[i] = spdata[frame_start + i];
	  
	fe_process_frame(FEW->FE,FEW->fr_data,FEW->FE->FRAME_SIZE,FEW->fr_cep);	
	  
	FEW->FE->FRAME_COUNTER ++;
	/*		printf("\nFrame: %d Frame_start: %d Frame_end: %d\n",FEW->FE->FRAME_COUNTER,frame_start,frame_start+FEW->FE->FRAME_SIZE); */
	/*Do CMN HERE */

	/*for(i = 0; i < FEW->FE->NUM_CEPSTRA; i++)
	  printf ("cepblock[%d][%d]: %f\n",frame_count,i,FEW->fr_cep[i]);			
	  fr_cep[i] = cepblock[frame_count][i];			
	  cmn_prior(&fr_cep, varnorm, 1, DIMENSIONS, endutt);*/
	
	myclass = classify (mgau, FEW->fr_cep, CLASSW->priors);
	
	if (CLASSW->postprocess == 1)
	  {
	    postclass = postclassify(CLASSW->window, CLASSW->windowlen, &wincap, myclass);
	  }
	else
	  {
	    postclass = myclass;	
	  }
	endpointer_update_stat (ENDPTR, FEW->FE, CLASSW, postclass);		
	
#if 0		
	switch (postclass){
	case 0:
	  printf(" N"); 
	  classcount[0] ++;
	  break;
	case 1:
	  printf(" O"); 
	  classcount[1] ++;
	  break;
	case 2:
	  printf(" S"); 
	  classcount[2] ++;
	  break;
	case 3:
	  printf(" SIL");
	  classcount[3] ++;
	  break;
	}
	printf ("\n"); 
#endif
                
	frame_count++;				// update the frame counter
	frame_start += FEW->FE->FRAME_SHIFT;		// update the frame shift

      }
      
      /****************************************************************/	
      /**************** end the processing **************************/		
      /****************************************************************/	
      /****************************************************************/
      /********** store new overflow data within the FE  **************/
      /****************************************************************/
      
      // Calculate the number of samples processed
      spbuf_len = (frame_count-1)*FEW->FE->FRAME_SHIFT + FEW->FE->FRAME_SIZE;
      
		/* assign samples which don't fit an entire frame to FE overflow buffer for use on next pass */
      if (spbuf_len < nsamps)   {
	  //printf("spbuf_len: %d\n",spbuf_len);
	  offset = ((frame_count)*FEW->FE->FRAME_SHIFT);
	  memcpy(FEW->FE->OVERFLOW_SAMPS,spdata+offset,(nsamps-offset)*sizeof(int16));
	  FEW->FE->NUM_OVERFLOW_SAMPS = nsamps-offset;
	  FEW->FE->PRIOR = spdata[offset-1];
	  
	  /* assert FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE */
	  assert(FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE);
	}
      free(spdata); /* close if there are enough samples for 1 frame*/
    }else{ 	/* if not enough total samps for a single frame,
       		append new samps to previously stored overlap samples */
      memcpy(FEW->FE->OVERFLOW_SAMPS + FEW->FE->NUM_OVERFLOW_SAMPS,spbuffer, nsamps*(sizeof(int16)));
      FEW->FE->NUM_OVERFLOW_SAMPS += nsamps;
      //printf("saved : %d\n samples in the OVERFLOW buffer",FEW->FE->NUM_OVERFLOW_SAMPS);      		
      assert(FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE);
      frame_count=0;
    }
}

int32 main(int32 argc, char **argv)
{
  fewrap_t *FEW;
  class_t *CLASSW;
  endpointer_t *ENDPTR;	
  ptmr_t tm_class;
  int32 splen, nframes;
  int16 *spbuffer;


  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",arg);

  if(cmd_ln_int32("-logspec"))
    E_FATAL("-logspec is currently not supported\n");

  unlimit();
  ptmr_init(&tm_class);

  FEW = few_initialize();
  CLASSW = classw_initialize(cmd_ln_str("-mean"),
			     cmd_ln_str("-var") ,cmd_ln_float32("-varfloor"),
			     cmd_ln_str("-mixw"),cmd_ln_float32("-mixwfloor"),
			     TRUE,".cont.");
  ENDPTR = endpointer_initialize(FEW->FE); 
  spbuffer = fe_convert_files_to_spdata(FEW->P, FEW->FE, &splen, &nframes);
  
  ptmr_start(&tm_class);
  process_fe_class(FEW, CLASSW, ENDPTR, spbuffer, splen);
  ptmr_stop(&tm_class);
  
  E_INFO("Time used: %6.3f sec CPU, %6.3f sec Clk;  TOT: %8.3f sec CPU, %8.3f sec Clk\n\n",
	 tm_class.t_cpu, tm_class.t_elapsed, tm_class.t_tot_cpu, tm_class.t_tot_elapsed);
  
  /** free the stuff for the front-end wraper */
  few_free(FEW);
  
  /** free the stuff for the class wraper */
  classw_free(CLASSW);
  endpointer_free(ENDPTR);

  cmd_ln_appl_exit();

  return(0);
}
















