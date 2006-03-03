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
 * HISTORY
 * $Log$
 * Revision 1.21  2006/03/03  20:04:06  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 * 
 * Revision 1.20  2006/02/24 04:20:06  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH, Used some macros. Merged with Dave and Evandro's changes.
 *
 *
 * Revision 1.19  2006/02/02 22:56:26  dhdfu
 * add a missing parameter
 *
 * Revision 1.18  2005/07/05 13:12:36  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.17  2005/07/04 20:57:52  dhdfu
 * Finally remove the "temporary hack" for the endpointer, and do
 * everything in logs3 domain.  Should make it faster and less likely to
 * crash on Alphas.
 *
 * Actually it kind of duplicates the existing GMM computation functions,
 * but it is slightly different (see the comment in classify.c).  I don't
 * know the rationale for this.
 *
 * Revision 1.16  2005/07/02 04:24:46  egouvea
 * Changed some hardwired constants to user defined parameters in the end pointer. Tested with make test-ep.
 *
 * Revision 1.15.4.5  2005/09/08 02:24:53  arthchan2003
 * Fix the mistake in last check-in.
 *
 * Revision 1.15.4.4  2005/09/07 23:48:58  arthchan2003
 * Remove .
 *
 * Revision 1.15.4.3  2005/07/18 23:21:24  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.15.4.2  2005/07/05 21:35:00  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.18  2005/07/05 13:12:36  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.15.4.1  2005/07/05 06:49:38  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.17  2005/07/04 20:57:52  dhdfu
 * Finally remove the "temporary hack" for the endpointer, and do
 * everything in logs3 domain.  Should make it faster and less likely to
 * crash on Alphas.
 *
 * Actually it kind of duplicates the existing GMM computation functions,
 * but it is slightly different (see the comment in classify.c).  I don't
 * know the rationale for this.
 *
 * Revision 1.16  2005/07/02 04:24:46  egouvea
 * Changed some hardwired constants to user defined parameters in the end pointer. Tested with make test-ep.
 *
 * Revision 1.15  2005/06/22 05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.2  2005/03/30 00:43:41  archan
 *
 * Add $Log$
 * Revision 1.21  2006/03/03  20:04:06  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 * 
 * Add Revision 1.20  2006/02/24 04:20:06  arthchan2003
 * Add Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH, Used some macros. Merged with Dave and Evandro's changes.
 * Add
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
#include "logs3.h"
#include "cmdln_macro.h"

/* Define a null device that depends on the platform */
#if defined(WIN32)
#define NULL_DEVICE "NUL"
#else
#define NULL_DEVICE "/dev/null"
#endif

/** \file main_ep.c
    \brief Driver for end-pointing
 */

const char helpstr[] =
  "Description: \n\
Print speech detection timing information from audio file.\n		\
									\
The main parameters that affect the final output, with typical values, are:\n \
									\
samprate, typically 8000, 11025, or 16000\n				\
lowerf, 200, 130, 130, for the respective sampling rates above\n	\
upperf, 3700, 5400, 6800, for the respective sampling rates above\n	\
nfilt, 31, 36, 40, for the respective sampling rates above\n		\
nfft, 256 or 512\n							\
format, raw or nist or mswav\n						\
\"";

const char examplestr[] =
  "Example: \n\
This example displays the timing information reporting beginning/end of speech from an input audio file named \"input.raw\", which is a raw audio file (no header information), which was originally sampled at 16kHz. \n \
\n									\
ep -i  input.raw \n						\
        -raw 1 \n							\
        -input_endian little \n						\
        -samprate  16000 \n						\
        -lowerf    130 \n						\
        -upperf    6800 \n						\
        -nfilt     40 \n						\
        -nfft      512";
static arg_t arg[] = {

  waveform_to_cepstral_command_line_macro()
  log_table_command_line_macro()
  common_application_properties_command_line_macro()
  gmm_command_line_macro() 

  { "-machine_endian",
    ARG_STRING,
#ifdef WORDS_BIGENDIAN
    "big",
#else
    "little",
#endif
    "Endianness of machine, big or little" },

  { "-i",
    ARG_STRING,
    NULL,
    "Single audio input file" },
  { "-o",
    ARG_STRING,
    NULL_DEVICE,
    "Single cepstral output file. Not used by ep." },
  { "-c",
    ARG_STRING,
    NULL,
    "Control file for batch processing. Not used by ep." },
  { "-di",
    ARG_STRING,
    NULL,
    "Input directory, input file names are relative to this, if defined. Not used by ep." },
  { "-ei",
    ARG_STRING,
    NULL,
    "Input extension to be applied to all input files. Not used by ep." },
  { "-do",
    ARG_STRING,
    NULL,
    "Output directory, output files are relative to this. Not used by ep." },
  { "-eo",
    ARG_STRING,
    NULL,
    "Output extension to be applied to all output files. Not used by ep." },
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
  { "-nchans",
    ARG_INT32,
    ONE_CHAN,
    "Number of channels of data (interlaced samples assumed)" },
  { "-whichchan",
    ARG_INT32,
    ONE_CHAN,
    "Channel to process" },
  { "-logspec",
    ARG_INT32,
    "0",
    "Write out logspectral files instead of cepstra" },
  { "-feat",
    ARG_STRING,
    "sphinx",
    "SPHINX format - big endian" },
  { "-verbose",
    ARG_INT32,
    "0",
    "Show input filenames" },
  { "-mdef",
    ARG_STRING,
    NULL,
    "The model definition file" },
  { "-pad_before",
    ARG_FLOAT32,
    PAD_T_BEFORE,
    "Pad these many seconds before speech begin" },
  { "-pad_after",
    ARG_FLOAT32,
    PAD_T_AFTER,
    "Pad these many seconds after speech end" },
  { "-speech_start",
    ARG_FLOAT32,
    UTT_T_START,
    "Declare speech after these many seconds of speech (pad not accounted)" },
  { "-speech_end",
    ARG_FLOAT32,
    UTT_T_END,
    "Declare end of speech after these many seconds of non-speech (pad not accounted)" },
  { "-speech_cancel",
    ARG_FLOAT32,
    UTT_T_CANCEL,
    "Cancel a start of speech  after these many seconds of non-speech" },
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
  
  wincap = 0;			/* voting window current capacity*/

  for (i = 0; i < NUMCLASSES; i++)
    classcount[i] = 0;	
	
  mgau=CLASSW->g;

  /***************** DONE WITH INITIALIZATIONS ********************/   
  /****************************************************************/

  nsamps = splen;
  /*spdata = spbuffer;*/
  
  E_INFO("%d samples, %d overflow, %d frame size\n", nsamps, FEW->FE->NUM_OVERFLOW_SAMPS, FEW->FE->FRAME_SIZE);

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

      /* initialize the frame_count and frame_start each time a new buffer of speech is available*/
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
	
	myclass = classify (FEW->fr_cep, mgau, CLASSW->priors, CLASSW->classmap);
	
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
                
	frame_count++;				/* update the frame counter*/
	frame_start += FEW->FE->FRAME_SHIFT;		/* update the frame shift*/

      }
      
      /****************************************************************/	
      /**************** end the processing **************************/		
      /****************************************************************/	
      /****************************************************************/
      /********** store new overflow data within the FE  **************/
      /****************************************************************/
      
      /* Calculate the number of samples processed */
      spbuf_len = (frame_count-1)*FEW->FE->FRAME_SHIFT + FEW->FE->FRAME_SIZE;
      
		/* assign samples which don't fit an entire frame to FE overflow buffer for use on next pass */
      if (spbuf_len < nsamps)   {
	/*printf("spbuf_len: %d\n",spbuf_len);*/
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
      /*printf("saved : %d\n samples in the OVERFLOW buffer",FEW->FE->NUM_OVERFLOW_SAMPS);      		 */
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
  cmd_ln_appl_enter(argc, argv, "default.arg", arg);

  if(cmd_ln_int32("-logspec"))
    E_FATAL("-logspec is currently not supported\n");

  if (strcmp(cmd_ln_str("-o"), NULL_DEVICE) != 0) {
    E_FATAL("Output file not used by ep: code outputs only timing info.\n");
  }

  if (cmd_ln_str("-c") ||
      cmd_ln_str("-di") ||
      cmd_ln_str("-do") ||
      cmd_ln_str("-ei") ||
      cmd_ln_str("-eo")) {
    E_FATAL("Only one file processed at a time, control file not allowed\n");
  }

  unlimit();
  ptmr_init(&tm_class);

  logs3_init(cmd_ln_float32("-logbase"), 1, cmd_ln_int32("-log3table"));

  FEW = few_initialize();
  CLASSW = classw_initialize(cmd_ln_str("-mdef"), cmd_ln_str("-mean"),
			     cmd_ln_str("-var"), cmd_ln_float32("-varfloor"),
			     cmd_ln_str("-mixw"), cmd_ln_float32("-mixwfloor"),
			     TRUE, ".cont.");
  ENDPTR = endpointer_initialize(FEW->FE,
				 cmd_ln_float32("-pad_before"),
				 cmd_ln_float32("-pad_after"),
				 cmd_ln_float32("-speech_start"),
				 cmd_ln_float32("-speech_end"),
				 cmd_ln_float32("-speech_cancel"));
 
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
