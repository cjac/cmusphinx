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
#define O_BINARY 0
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

#include "main_ep.h"
#include "cont_mgau.h"
#include "ep_cmd_ln_defn.h"

void fe_validate_parameters(param_t *P) ;

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

int32 main(int32 argc, char **argv)
{

	fewrap_t *FEW;
	class_t *CLASSW;
	endpointer_t *ENDPTR;	
	ptmr_t tm_class;
	
	int32 splen, nframes;
        int16 *spbuffer;

	unlimit();

	ptmr_init(&tm_class);
	FEW = few_initialize(argc,argv);
        CLASSW = classw_initialize(cmd_ln_str("-mean"),
				   cmd_ln_str("-var") ,cmd_ln_float32("-varfloor"),
				   cmd_ln_str("-mixw"),cmd_ln_float32("-mixwfloor"),
				   TRUE,".cont.");
	ENDPTR = endpointer_initialize(FEW->FE); 
	spbuffer = fe_convert_files(FEW->P, FEW->FE, &splen, &nframes);
	/*E_INFO("splen: %d, nframes: %d\n",splen,nframes);*/

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
	if(spbuffer) free(spbuffer);


        return(0);
}

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

	/* are there enough samples to make at least 1 frame? */
    	if (nsamps + FEW->FE->NUM_OVERFLOW_SAMPS >= FEW->FE->FRAME_SIZE)
	{
		/****************************************************************/
 	       	/****** Prepend any overflow data from last call   **************/ 
		/****************************************************************/
		
		/* if there are previous samples, pre-pend them to input speech samps */
      		if ((FEW->FE->NUM_OVERFLOW_SAMPS > 0)) 
		{
        		if ((spdata = (int16 *) malloc (sizeof(int16)*(FEW->FE->NUM_OVERFLOW_SAMPS +nsamps)))==NULL)		
			{
            			fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
            			exit(0);
        		}
        		/* RAH */
        		memcpy (spdata,FEW->FE->OVERFLOW_SAMPS,FEW->FE->NUM_OVERFLOW_SAMPS*(sizeof(int16))); 
        		memcpy(spdata+FEW->FE->NUM_OVERFLOW_SAMPS, spbuffer, nsamps*(sizeof(int16))); 
        		nsamps += FEW->FE->NUM_OVERFLOW_SAMPS;
        		FEW->FE->NUM_OVERFLOW_SAMPS = 0; /*reset overflow samps count */
      		}
		else
		{
			if ((spdata = (int16 *) malloc (sizeof(int16)*nsamps)) == NULL)
                        {
                                fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
                                exit(0);
                        }
                        /* RAH */
                        memcpy(spdata, spbuffer, nsamps*(sizeof(int16)));
		}

	    /****************************************************************/	
	    /**************** Start the processing **************************/		
	    /****************************************************************/	

	    // initialize the frame_count and frame_start each time a new buffer of speech is available
	    frame_count = 0;
	    frame_start = 0;

     	    while(frame_start+FEW->FE->FRAME_SIZE <= nsamps)
	    {
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
		if (spbuf_len < nsamps)   
		{
			//printf("spbuf_len: %d\n",spbuf_len);
        		offset = ((frame_count)*FEW->FE->FRAME_SHIFT);
        		memcpy(FEW->FE->OVERFLOW_SAMPS,spdata+offset,(nsamps-offset)*sizeof(int16));
        		FEW->FE->NUM_OVERFLOW_SAMPS = nsamps-offset;
        		FEW->FE->PRIOR = spdata[offset-1];

			/* assert FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE */
        		assert(FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE);
      		}

		free(spdata);

	} 	// close if there are enough samples for 1 frame



	/* if not enough total samps for a single frame, append new samps to
       		previously stored overlap samples */
	else
	{
		
      		memcpy(FEW->FE->OVERFLOW_SAMPS + FEW->FE->NUM_OVERFLOW_SAMPS,spbuffer, nsamps*(sizeof(int16)));
		FEW->FE->NUM_OVERFLOW_SAMPS += nsamps;
		//printf("saved : %d\n samples in the OVERFLOW buffer",FEW->FE->NUM_OVERFLOW_SAMPS);      		
      		assert(FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE);
      		frame_count=0;
    	}


}



/*** Function to free the front-end wrapper ***/
void few_free(fewrap_t *FEW)
{

	free(FEW->fr_data);
        free(FEW->fr_cep);
 
        fe_close(FEW->FE);
        fe_free_param(FEW->P);
        free(FEW);   

}





/*** Function to initialize the front-end wrapper ***/
fewrap_t * few_initialize(int argc, char **argv)
{
	fewrap_t *FEW = (fewrap_t *) calloc(1,sizeof(fewrap_t));

        /********************** INITIALIZING COMPONENTS ******************/

        // initialize parameters
        FEW->P = fe_parse_options(argc, argv);

        // initialize the front-end parameters
        if (( FEW->FE = fe_init(FEW->P))==NULL){
                fprintf(stderr,"memory alloc failed in fe_convert_files()\n...exiting\n");
                exit(0);
        }

        /*************** Allocate memory for each frame of speech *******************/

        FEW->fr_data = (int16 *)calloc(FEW->FE->FRAME_SIZE, sizeof(int16));
        FEW->fr_cep = (float32 *)calloc(FEW->FE->NUM_CEPSTRA, sizeof(float32));

	return(FEW);

}



int16 * fe_convert_files(param_t *P, fe_t *FE, int32 *splenp, int32 *nframesp)
{

    char *infile, fileroot[MAXCHARS];
    int32 splen,total_samps,frames_proc,nframes,nblocks;
    int32 fp_in, last_blocksize=0,curr_block,total_frames;
    int16 *spdata;

    spdata = NULL;	 
    
    /* 20040917: ARCHAN The fe_copy_str and free pair is very fishy
       here. If the number of utterance being processed is larger than
       1M, there may be chance, we will hit out of segment problem. */


	if (P->is_single){
		
	fe_build_filenames(P,fileroot,&infile);
	if (P->verbose) printf("%s\n",infile);

	if (fe_openfiles(P,FE,infile,&fp_in,&total_samps,&nframes,&nblocks) != FE_SUCCESS){	  printf("fe_openfiles exited!\n");	
	  exit(0);
	}
		
	//printf("nframes: %d, nblocks: %d\n",nframes,nblocks);
		
	if (nblocks*P->blocksize>=total_samps) 
	    last_blocksize = total_samps - (nblocks-1)*P->blocksize;
	
	if (!fe_start_utt(FE)){
	    curr_block=1;
	    total_frames=frames_proc=0;
		//printf("Total frames %d, last_blocksize: %d\n",total_frames, last_blocksize);
	
		/* process last (or only) block */
	    if (spdata!=NULL) free(spdata);
	    splen =last_blocksize;
	    if ((spdata = (int16 *)calloc(splen,sizeof(int16)))==NULL){
		fprintf(stderr,"Unable to allocate memory block of %d shorts for input speech\n",splen);
		exit(0);
	    } 
	    if (fe_readblock_spch(P,fp_in,splen,spdata)!=splen){
		fprintf(stderr,"Error reading speech data\n");
		exit(0);
	    }
	
	    fe_closefiles(fp_in);
	    
	}
	else{
	    	fprintf(stderr,"fe_start_utt() failed\n");
		exit(-1);
	}
	
    }
    else{
		fprintf(stderr,"Unknown mode - single or batch?\n");
		exit(-1);
    }
       
    ckd_free(infile);


	*splenp = splen; 
	*nframesp = nframes;   
	
	return(spdata);
}

param_t *fe_parse_options(int32 argc, char **argv) 
{
    param_t *P;
    int32 format;
    char *endian;
    
    if ((P=(param_t *)malloc(sizeof(param_t)))==NULL){
        fprintf(stderr,"memory alloc failed in fe_parse_options()\n...exiting\n");
        exit(0);
    }
    cmd_ln_parse(arg,argc,argv);
    
    /*    if (argc <3 ){
        fprintf(stderr,"No Command Line Arguments given\n");
        fe_usage(argv);
	}
            
	fe_init_params(P);*/

    fe_init_params(P);
   

    P->wavfile = (char *)cmd_ln_access("-i");
    if (P->wavfile != NULL) {
        P->is_single = ON;
    }
    
    
    P->ctlfile = (char *)cmd_ln_access("-c");
    if (P->ctlfile != NULL) {
        P->is_batch = ON;
    }
    
    P->wavdir = (char *)cmd_ln_access("-di");
    P->cepdir = (char *)cmd_ln_access("-do");
    P->wavext = (char *)cmd_ln_access("-ei");
    P->cepext = (char *)cmd_ln_access("-eo");
    format = *(int32 *)cmd_ln_access("-raw");
    if (format) {
        P->input_format = RAW;
    }
    format = *(int32 *)cmd_ln_access("-nist");
    if (format) {
        P->input_format = NIST;
    } 
    format = *(int32 *)cmd_ln_access("-mswav");
    if (format) {
        P->input_format = MSWAV;
    }
    
    P->nchans = *(int32 *)cmd_ln_access("-nchans");
    P->whichchan = *(int32 *)cmd_ln_access("-whichchan");
    P->PRE_EMPHASIS_ALPHA = *(float32 *)cmd_ln_access("-alpha");
    P->SAMPLING_RATE = *(float32 *)cmd_ln_access("-srate");
    P->WINDOW_LENGTH = *(float32 *)cmd_ln_access("-wlen");
    P->FRAME_RATE = *(int32 *)cmd_ln_access("-frate");
    if (!strcmp((const char *)cmd_ln_access("-feat"), "sphinx")) 
    {
        P->FB_TYPE = MEL_SCALE;
        P->output_endian = BIG;
    } 
    else 
    {
        E_ERROR("MEL_SCALE IS CURRENTLY THE ONLY IMPLEMENTATION\n\n");
        E_FATAL("Make sure you specify '-feat sphinx'\n");
    }	
    P->NUM_FILTERS = *(int32 *)cmd_ln_access("-nfilt");
    P->NUM_CEPSTRA = *(int32 *)cmd_ln_access("-ncep");
    P->LOWER_FILT_FREQ = *(float32 *)cmd_ln_access("-lowerf");
    P->UPPER_FILT_FREQ = *(float32 *)cmd_ln_access("-upperf");
    P->FFT_SIZE = *(int32 *)cmd_ln_access("-nfft");
    if (*(int32 *)cmd_ln_access("-doublebw")) {
        P->doublebw = ON; 
    } else {
        P->doublebw = OFF;
    }
    P->blocksize = *(int32 *)cmd_ln_access("-blocksize");
    P->verbose = *(int32 *)cmd_ln_access("-verbose");
    endian = (char *)cmd_ln_access("-mach_endian");
    if (!strcmp("big", endian)) {
        P->machine_endian = BIG;
    } else {
        if (!strcmp("little", endian)) {
            P->machine_endian = LITTLE;
        } else {
            E_FATAL("Machine must be big or little Endian\n");
        }	
    }
    endian = (char *)cmd_ln_access("-input_endian");
    if (!strcmp("big", endian)) {
        P->input_endian = BIG;
    } else {
        if (!strcmp("little", endian)) {
            P->input_endian = LITTLE;
        } else {
            E_FATAL("Input must be big or little Endian\n");
        }	
    }
    P->dither = *(int32 *)cmd_ln_access("-dither");
    P->logspec = *(int32 *)cmd_ln_access("-logspec");
    
    fe_validate_parameters(P);
   
    /*        if (!strcmp("-i",argv[1]))
        {
                P->wavfile = argv[2]; 
                P->is_single = ON;
        }
        else
	fe_usage(argv);*/

    return (P);
}

void fe_validate_parameters(param_t *P) 
{
    
    if ((P->is_batch) && (P->is_single)) {
        E_FATAL("You cannot define an input file and a control file\n");
    }
    
    if (P->wavfile == NULL && P->wavdir == NULL){
        E_FATAL("No input file or file directory given\n");
    }
    
    /*    if (P->cepfile == NULL && P->cepdir == NULL){
        E_FATAL("No cepstra file or file directory given\n");
	}*/
    
    if (P->ctlfile==NULL && P->cepfile==NULL && P->wavfile==NULL){
        E_FATAL("No control file given\n");
    }
    
    if (P->nchans>1){
        E_INFO("Files have %d channels of data\n", P->nchans);
        E_INFO("Will extract features for channel %d\n", P->whichchan);
    }
    
    if (P->whichchan > P->nchans) {
        E_FATAL("You cannot select channel %d out of %d\n", P->whichchan, P->nchans);
    }
    
    if ((P->UPPER_FILT_FREQ * 2) > P->SAMPLING_RATE) {
        E_WARN("Upper frequency higher than Nyquist frequency");
    }
    
    if (P->doublebw==ON) {
        E_INFO("Will use double bandwidth filters\n");
    }
}


void fe_init_params(param_t *P)
{
  P->SAMPLING_RATE = DEFAULT_SAMPLING_RATE;
    P->FRAME_RATE = DEFAULT_FRAME_RATE;
    P->WINDOW_LENGTH = DEFAULT_WINDOW_LENGTH;
    P->FB_TYPE = DEFAULT_FB_TYPE;
    P->PRE_EMPHASIS_ALPHA = DEFAULT_PRE_EMPHASIS_ALPHA;
    P->NUM_CEPSTRA = DEFAULT_NUM_CEPSTRA;
    P->FFT_SIZE = DEFAULT_FFT_SIZE;
    P->NUM_FILTERS = DEFAULT_NUM_FILTERS;
    P->UPPER_FILT_FREQ = DEFAULT_UPPER_FILT_FREQ;
    P->LOWER_FILT_FREQ = DEFAULT_LOWER_FILT_FREQ;
    P->blocksize = DEFAULT_BLOCKSIZE;


    P->is_batch = OFF;
    P->is_single = OFF;

    /*  P->SAMPLING_RATE = 11025.0;
  P->FRAME_RATE = 105;
  P->WINDOW_LENGTH = 0.024;
  P->FB_TYPE = MEL_SCALE;
  P->PRE_EMPHASIS_ALPHA = 0.97;
  P->NUM_CEPSTRA =  13;
  P->FFT_SIZE = 512;
  P->NUM_FILTERS = 36;
  P->UPPER_FILT_FREQ = 5400;
  P->LOWER_FILT_FREQ = 130;
  P->blocksize = 262500;*/

    P->verbose = OFF;
    P->input_format= RAW;
    P->input_endian = LITTLE;
#ifdef __BIG_ENDIAN__
    P->machine_endian = BIG;
#else
    P->machine_endian = LITTLE;
#endif
    P->output_endian = BIG;
    P->dither = DITHER;
    P->wavfile = NULL;
    P->cepfile = NULL;
    P->ctlfile = NULL;
    P->wavdir = NULL;
    P->cepdir = NULL;
    P->wavext = NULL;
    P->cepext = NULL;
    P->nchans = ONE_CHAN;
    P->whichchan = ONE_CHAN;
    P->doublebw = OFF;
    
}


int32 fe_build_filenames(param_t *P, char *fileroot, char **infilename)
{
    char cbuf[MAXCHARS];
    char chanlabel[MAXCHARS];

    if (P->nchans>1)
	sprintf(chanlabel, ".ch%d", P->whichchan);
	   

    if (P->is_single){
	sprintf(cbuf,"%s","");
	strcat(cbuf,P->wavfile);


	*infilename = fe_copystr(*infilename,cbuf);
	}	
    else{
	fprintf(stderr,"Unspecified Batch or Single Mode\n");
    }

    return 0;
}

char *fe_copystr(char *dest_str, char *src_str)
{
    int i,src_len, len;
    char *s;
    
    src_len = strlen(src_str);
    len = src_len;
    s = (char *)malloc((char)(len+1));
    for (i=0;i<src_len;i++)
	*(s+i) = *(src_str+i);
    *(s+src_len) = NULL_CHAR;
    
    return(s);
}

int32 fe_count_frames(fe_t *FE, int32 nsamps, int32 count_partial_frames)
{
    int32 frame_start,frame_count = 0;
    
    for (frame_start=0;frame_start+FE->FRAME_SIZE<=nsamps;
	 frame_start+=FE->FRAME_SHIFT)
	frame_count++;
   
    if (count_partial_frames){
	if ((frame_count-1)*FE->FRAME_SHIFT+FE->FRAME_SIZE < nsamps)
	    frame_count++;
    }
    
    return(frame_count);
}
	    
int32 fe_openfiles(param_t *P, fe_t *FE, char *infile, int32 *fp_in, int32 *nsamps, 
		   int32 *nframes, int32 *nblocks)
{


    struct stat filestats;
    int fp=0, len=0, numframes, numblocks;
    FILE *fp2;
    char line[MAXCHARS];
    int got_it=0;


    /* Note: this is kind of a hack to read the byte format from the
       NIST header */
    if (P->input_format == NIST){
        if ((fp2 = fopen(infile,"rb")) == NULL){
	    fprintf(stderr,"Cannot read %s\n",infile);
	    return(FE_INPUT_FILE_READ_ERROR);
	}
	*line=0;
	got_it = 0;
	while(strcmp(line,"end_head") && !got_it){
	    fscanf(fp2,"%s",line);
	    if (!strcmp(line,"sample_byte_format")){
		fscanf(fp2,"%s",line);
		if (!strcmp(line,"-s2")){
		    fscanf(fp2,"%s",line);
		    if (!strcmp(line,"01")){
			P->input_endian=LITTLE;
			got_it=1;
		    }
		    else if(!strcmp(line,"10")){
			P->input_endian=BIG;
			got_it = 1;
		    }
		    else
			fprintf(stderr,"Unknown/unsupported byte order\n");	
		}
		else 
		    fprintf(stderr,"Error determining byte format\n");
	    }
	}
	if (!got_it){
	    fprintf(stderr,"Can't find byte format in header, setting to machine's endian\n");
	    P->input_endian = P->machine_endian;
	}	    
	fclose(fp2);
    }
    else if (P->input_format == RAW){
      /*
	P->input_endian = P->machine_endian;
      */
    }
    else if (P->input_format == MSWAV){
	P->input_endian = LITTLE; // Default for MS WAV riff files
    }
    
    
    if ((fp = open(infile, O_RDONLY | O_BINARY, 0644))<0){
        fprintf(stderr,"Cannot open %s\n",infile);
  	return (FE_INPUT_FILE_OPEN_ERROR);
    }
    else{
        if (fstat(fp,&filestats)!=0) printf("fstat failed\n");
	
	if (P->input_format == NIST){
	    short *hdr_buf;

	    len = (filestats.st_size-HEADER_BYTES)/sizeof(short);
	    /* eat header */
	    hdr_buf = (short *)calloc(HEADER_BYTES/sizeof(short),sizeof(short));
	    if (read(fp,hdr_buf,HEADER_BYTES)!=HEADER_BYTES){
		fprintf(stderr,"Cannot read %s\n",infile);
		return (FE_INPUT_FILE_READ_ERROR);
	    }
	    free(hdr_buf);    
	}
	else if (P->input_format == RAW){
	    len = filestats.st_size/sizeof(int16);
	}
	else if (P->input_format == MSWAV){
            /* Read the header */
            MSWAV_hdr *hdr_buf;
	    if ((hdr_buf = (MSWAV_hdr*) calloc(1,sizeof(MSWAV_hdr))) == NULL){
	        fprintf(stderr,"Cannot allocate for input file header\n");
	        return (FE_INPUT_FILE_READ_ERROR);
            }
            if (read(fp,hdr_buf,sizeof(MSWAV_hdr)) != sizeof(MSWAV_hdr)){
	        fprintf(stderr,"Cannot allocate for input file header\n");
	        return (FE_INPUT_FILE_READ_ERROR);
            }
	    /* Check header */
	    if (strncmp(hdr_buf->rifftag,"RIFF",4)!=0 ||
		strncmp(hdr_buf->wavefmttag,"WAVEfmt",7)!=0) {
	        fprintf(stderr,"Error in mswav file header\n");
	        return (FE_INPUT_FILE_READ_ERROR);
            }
	    if (strncmp(hdr_buf->datatag,"data",4)!=0) {
	      /* In this case, there are other "chunks" before the
	       * data chunk, which we can ignore. We have to find the
	       * start of the data chunk, which begins with the string
	       * "data".
	       */
	      int16 found=OFF;
	      char readChar;
	      char *dataString = "data";
	      int16 charPointer = 0;
	      printf("LENGTH: %d\n", strlen(dataString));
	      while (found != ON) {
		if (read(fp,&readChar,sizeof(char)) != sizeof(char)){
		  fprintf(stderr,"Failed reading wav file.\n");
		  return (FE_INPUT_FILE_READ_ERROR);
		}
		if (readChar == dataString[charPointer]) {
		  charPointer++;
		}
		if (charPointer == strlen(dataString)) {
		  found = ON;
		  strcpy(hdr_buf->datatag, dataString);
		  if (read(fp,&(hdr_buf->datalength),sizeof(int32)) != sizeof(int32)){
		    fprintf(stderr,"Failed reading wav file.\n");
		    return (FE_INPUT_FILE_READ_ERROR);
		  }
		}
	      }
	    }
            if (P->input_endian!=P->machine_endian) { // If machine is Big Endian
                hdr_buf->datalength = SWAPL(&(hdr_buf->datalength));
	        hdr_buf->data_format = SWAPW(&(hdr_buf->data_format));
                hdr_buf->numchannels = SWAPW(&(hdr_buf->numchannels));
                hdr_buf->BitsPerSample = SWAPW(&(hdr_buf->BitsPerSample));
                hdr_buf->SamplingFreq = SWAPL(&(hdr_buf->SamplingFreq));
	        hdr_buf->BytesPerSec = SWAPL(&(hdr_buf->BytesPerSec));
	    }
	    /* Check Format */
	    if (hdr_buf->data_format != 1 || hdr_buf->BitsPerSample != 16){
                fprintf(stderr,"MS WAV file not in 16-bit PCM format\n");
		return (FE_INPUT_FILE_READ_ERROR);
	    }
	    len = hdr_buf->datalength / sizeof(short);
	    P->nchans = hdr_buf->numchannels;
	    /* DEBUG: Dump Info */
	    fprintf(stderr,"Reading MS Wav file %s:\n", infile);
	    fprintf(stderr,"\t16 bit PCM data, %d channels %d samples\n",P->nchans,len);
	    fprintf(stderr,"\tSampled at %d\n",hdr_buf->SamplingFreq);
	    free(hdr_buf);
	}
	else {
	    fprintf(stderr,"Unknown input file format\n");
	    return(FE_INPUT_FILE_OPEN_ERROR);
	}
    }


    len = len/P->nchans;
    *nsamps = len;
    *fp_in = fp;

    numblocks = (int)((float)len/(float)P->blocksize);
    if (numblocks*P->blocksize<len)
	numblocks++;

	numframes = fe_count_frames(FE,len,COUNT_PARTIAL);

    *nblocks = numblocks;  
    *nframes = numframes;  

    return 0;
}

int32 fe_readblock_spch(param_t *P, int32 fp, int32 nsamps, int16 *buf)
{
    int32 bytes_read, cum_bytes_read, nreadbytes, actsamps, offset, i, j, k;
    int16 *tmpbuf;
    int32 nchans, whichchan;
    
    nchans = P->nchans;
    whichchan = P->whichchan;
    
    if (nchans==1){
        if (P->input_format==RAW || P->input_format==NIST || P->input_format==MSWAV){    
	    nreadbytes = nsamps*sizeof(int16);
	    if ((bytes_read = read(fp,buf,nreadbytes))!=nreadbytes){
  	        fprintf(stderr,"error reading block\n");
		return(0);
	    }	
	}
	else{
  	    fprintf(stderr,"unknown input file format\n");
	    return(0);
	}
	cum_bytes_read = bytes_read;
    }
    else if (nchans>1){
	
	if (nsamps<P->blocksize){
	    actsamps = nsamps*nchans;
	    tmpbuf = (int16 *)calloc(nsamps*nchans,sizeof(int16));
	    cum_bytes_read = 0;
	    if (P->input_format==RAW || P->input_format==NIST){    
		
		k=0;
		nreadbytes = actsamps*sizeof(int16);
		
		if ((bytes_read = read(fp,tmpbuf,nreadbytes))!=nreadbytes){
		    fprintf(stderr,"error reading block (got %d not %d)\n",bytes_read,nreadbytes);
		    return(0);
		}
		
		for (j=whichchan-1;j<actsamps;j=j+nchans){
		    buf[k] = tmpbuf[j];
		    k++;
		}
		cum_bytes_read += bytes_read/nchans;
	    }
	    else{
		fprintf(stderr,"unknown input file format\n");
		return(0);
	    }
	    free(tmpbuf);
	}
	else{
	    tmpbuf = (int16 *)calloc(nsamps,sizeof(int16));
	    actsamps = nsamps/nchans;	    
	    cum_bytes_read = 0;
	    
	    if (actsamps*nchans != nsamps){
		fprintf(stderr,"Warning: Blocksize %d is not an integer multiple of Number of channels %d\n",nsamps, nchans);
	    }
	    
	    if (P->input_format==RAW || P->input_format==NIST){    
		for (i=0;i<nchans;i++){
		    
		    offset = i*actsamps;
		    k=0;
		    nreadbytes = nsamps*sizeof(int16);
		    
		    if ((bytes_read = read(fp,tmpbuf,nreadbytes))!=nreadbytes){
			fprintf(stderr,"error reading block (got %d not %d)\n",bytes_read,nreadbytes);
			return(0);
		    }
		    
		    for (j=whichchan-1;j<nsamps;j=j+nchans){
			buf[offset+k] = tmpbuf[j];
			k++;
		    }
		    cum_bytes_read += bytes_read/nchans;
		}
	    }
	    else{
		fprintf(stderr,"unknown input file format\n");
		return(0);
	    }
	    free(tmpbuf);
	}
    }
    
    else{
	fprintf(stderr,"unknown number of channels!\n");
	return(0);
    }
  
    if (P->input_endian!=P->machine_endian){
         for(i=0;i<nsamps;i++)
	    SWAPW(&buf[i]);
    }

    if (P->dither==ON)
        fe_dither(buf,nsamps);

    return(cum_bytes_read/sizeof(int16));

}

int32 fe_writeblock_feat(param_t *P, fe_t *FE, int32 fp, int32 nframes, float32 **feat)
{   

    int32 i, length, nwritebytes;

    if (P->logspec == ON)
        length = nframes*FE->MEL_FB->num_filters;
    else
        length = nframes*FE->NUM_CEPSTRA;
    
    if (P->output_endian != P->machine_endian){
	for (i=0;i<length;++i) SWAPF(feat[0]+i);
    }
    
    nwritebytes = length*sizeof(float32);
    if  (write(fp, feat[0], nwritebytes) != nwritebytes) {
        fprintf(stderr,"Error writing block of features\n");
        close(fp);
        return(FE_OUTPUT_FILE_WRITE_ERROR);
    }

    
    if (P->output_endian != P->machine_endian){
	for (i=0;i<length;++i) SWAPF(feat[0]+i);
    }
//   printf("Length: %d\n",length); 
    return(length);
}


int32 fe_closefiles(int32 fp_in)
{
    close(fp_in);
    return 0;
}


/* adds 1/2-bit noise */
int32 fe_dither(int16 *buffer,int32 nsamps)
{
  int32 i;
  srand48((long)time(0));
  for (i=0;i<nsamps;i++)
    buffer[i] += (short)((!(lrand48()%4))?1:0);
  
  return 0;
}

int32 fe_free_param(param_t *P)
{
    free(P);
    /* but no one calls it (29/09/00) */
    return 0;
}

int32 fe_usage(char **argv)
{

    printf("wave2class - Converts audio stream into Mel-Frequency Cepstrum Coefficients and classifies each frame\n Usage: %s -i <raw audio file at 11.025kHz>\n", argv[0]); 
    exit(0);
    return 0;
}

