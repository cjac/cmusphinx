/* ====================================================================
 * Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
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

#include "fe.h"

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
#include <fcntl.h>
#include <assert.h>

#if defined(WIN32)
#include <io.h>
#include <errno.h>

/*Now using customized random generator. */
/*#define srand48(x) srand(x)
  #define lrand48() rand()*/
#endif

#if (WIN32) 
/* RAH #include <random.h> */
#include <time.h>		/* RAH */
#endif

int32 ep_fe_openfiles(param_t *P, fe_t *FE, char *infile, int32 *fp_in, int32 *nsamps, int32 *nframes, int32 *nblocks);
void fe_init_dither(int32 seed);

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
fewrap_t * few_initialize()
{
  fewrap_t *FEW = (fewrap_t *) ckd_calloc(1,sizeof(fewrap_t));
  
  /********************** INITIALIZING COMPONENTS ******************/
  
  // initialize parameters
  FEW->P = fe_parse_options();
  
  // initialize the front-end parameters
  if (( FEW->FE = fe_init(FEW->P))==NULL){
    fprintf(stderr,"memory alloc failed in fe_convert_files()\n...exiting\n");
    exit(0);
  }
  
  /*************** Allocate memory for each frame of speech *******************/
  
  FEW->fr_data = (int16 *)ckd_calloc(FEW->FE->FRAME_SIZE, sizeof(int16));
  FEW->fr_cep = (float32 *)ckd_calloc(FEW->FE->NUM_CEPSTRA, sizeof(float32));
  
  return(FEW);

}

int32 fe_convert_files(param_t *P)
{

    fe_t *FE;
    char *infile,*outfile, fileroot[MAXCHARS];
    FILE *ctlfile;
    int16 *spdata=NULL;
    int32 splen,total_samps,frames_proc,nframes,nblocks,last_frame;
    int32 fp_in,fp_out, last_blocksize=0,curr_block,total_frames;
    float32 **cep = NULL, **last_frame_cep;
    int32 return_value;
    int32 warn_zero_energy = OFF;
    int32 process_utt_return_value;
    
    splen=0;
    if ((FE = fe_init(P))==NULL){
	E_ERROR("memory alloc failed...exiting\n");
	return(FE_MEM_ALLOC_ERROR);
    }

    if (P->is_batch){
	if ((ctlfile = fopen(P->ctlfile,"r")) == NULL){
	    E_ERROR("Unable to open control file %s\n",P->ctlfile);
	    return(FE_CONTROL_FILE_ERROR);
	}
	while (fscanf(ctlfile,"%s",fileroot)!=EOF){
	    fe_build_filenames(P,fileroot,&infile,&outfile);

	    if (P->verbose) E_INFO("%s\n",infile);

	    return_value = fe_openfiles(P,FE,infile,&fp_in,&total_samps,&nframes,&nblocks,outfile,&fp_out);
	    if (return_value != FE_SUCCESS){
	      return(return_value);
	    }

	    warn_zero_energy = OFF;

	    if (nblocks*P->blocksize>=total_samps) 
		last_blocksize = total_samps - (nblocks-1)*P->blocksize;
	    
	    if (!fe_start_utt(FE)){
		curr_block=1;
		total_frames=frames_proc=0;
		/*execute this loop only if there is more than 1 block to
		  be processed */
		while(curr_block < nblocks){
		    splen = P->blocksize;
		    if ((spdata = (int16 *)calloc(splen,sizeof(int16)))==NULL){
			E_ERROR("Unable to allocate memory block of %d shorts for input speech\n",splen);
			return(FE_MEM_ALLOC_ERROR);
		    } 
		    if (fe_readblock_spch(P,fp_in,splen,spdata)!=splen){
			E_ERROR("error reading speech data\n");
			return(FE_INPUT_FILE_READ_ERROR);
		    }
		    process_utt_return_value = 
		      fe_process_utt(FE,spdata,splen,&cep, &frames_proc);
		    if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
		      warn_zero_energy = ON;
		    } else {
		      assert(process_utt_return_value == FE_SUCCESS);
		    }
		    if (frames_proc>0)
			fe_writeblock_feat(P,FE,fp_out,frames_proc,cep);
		    ckd_free_2d((void **)cep);
		    curr_block++;
		    total_frames += frames_proc;
		    if (spdata!=NULL) { 
		      free(spdata); 
		      spdata = NULL; 
		    }
		}
		/* process last (or only) block */
		if (spdata!=NULL) {
		  free(spdata);
		}
		splen=last_blocksize;
		
		if ((spdata = (int16 *)calloc(splen,sizeof(int16)))==NULL){
		    E_ERROR("Unable to allocate memory block of %d shorts for input speech\n",splen);
		    return(FE_MEM_ALLOC_ERROR);
		} 

		if (fe_readblock_spch(P,fp_in,splen,spdata)!=splen){
		    E_ERROR("error reading speech data\n");
		    return(FE_INPUT_FILE_READ_ERROR);
		}
		
		process_utt_return_value = 
		  fe_process_utt(FE,spdata,splen,&cep, &frames_proc);
		if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
		  warn_zero_energy = ON;
		} else {
		  assert(process_utt_return_value == FE_SUCCESS);
		}
		if (frames_proc>0)
		    fe_writeblock_feat(P,FE,fp_out,frames_proc,cep);
		ckd_free_2d((void **)cep);
		curr_block++;
		if (P->logspec != ON)
		    last_frame_cep = (float32 **)ckd_calloc_2d(1,FE->NUM_CEPSTRA,sizeof(float32));
		else
		    last_frame_cep = (float32 **)ckd_calloc_2d(1,FE->MEL_FB->num_filters,sizeof(float32));
		process_utt_return_value = 
		  fe_end_utt(FE, last_frame_cep[0], &last_frame);
		if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
		  warn_zero_energy = ON;
		} else {
		  assert(process_utt_return_value == FE_SUCCESS);
		}
		if (last_frame>0){
		    fe_writeblock_feat(P,FE,fp_out,last_frame,last_frame_cep);
		    frames_proc++;
		}
		total_frames += frames_proc;
		
		fe_closefiles(fp_in,fp_out);		
		free(spdata); spdata = 0;
		ckd_free_2d((void **)last_frame_cep);
		
	    }
	    else{
		E_ERROR("fe_start_utt() failed\n");
		return(FE_START_ERROR);
	    }
	}
	fe_close(FE);
	if (ON == warn_zero_energy) {
	  E_WARN("File %s has some frames with zero energy. Consider using dither\n", infile);
	}
    }
    
    else if (P->is_single){
	
	fe_build_filenames(P,fileroot,&infile,&outfile);
	if (P->verbose) printf("%s\n",infile);
	return_value = fe_openfiles(P,FE,infile,&fp_in,&total_samps,&nframes,&nblocks,outfile,&fp_out);
	if (return_value != FE_SUCCESS){
	  return(return_value);
	}

	warn_zero_energy = OFF;
	
	if (nblocks*P->blocksize>=total_samps) 
	    last_blocksize = total_samps - (nblocks-1)*P->blocksize;
	
	if (!fe_start_utt(FE)){
	    curr_block=1;
	    total_frames=frames_proc=0;
	    /*execute this loop only if there are more than 1 block to
	      be processed */
	    while(curr_block < nblocks){
		splen = P->blocksize;
		if ((spdata = (int16 *)calloc(splen,sizeof(int16)))==NULL){
		    E_ERROR("Unable to allocate memory block of %d shorts for input speech\n",splen);
		    return(FE_MEM_ALLOC_ERROR);
		} 
		if (fe_readblock_spch(P,fp_in,splen,spdata)!=splen){
		    E_ERROR("Error reading speech data\n");
		    return(FE_INPUT_FILE_READ_ERROR);
		}
		process_utt_return_value = 
		  fe_process_utt(FE,spdata,splen,&cep, &frames_proc);
		if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
		  warn_zero_energy = ON;
		} else {
		  assert(process_utt_return_value == FE_SUCCESS);
		}
		if (frames_proc>0)
		    fe_writeblock_feat(P,FE,fp_out,frames_proc,cep);
		ckd_free_2d((void **)cep);
		curr_block++;
		total_frames += frames_proc;
		if (spdata!=NULL) { 
		  free(spdata); 
		  spdata = NULL; 
		}		
	    }
	    /* process last (or only) block */
	    if (spdata!=NULL) {free(spdata);}
	    splen =last_blocksize;
	    if ((spdata = (int16 *)calloc(splen,sizeof(int16)))==NULL){
		E_ERROR("Unable to allocate memory block of %d shorts for input speech\n",splen);
		return(FE_MEM_ALLOC_ERROR);
	    } 
	    if (fe_readblock_spch(P,fp_in,splen,spdata)!=splen){
		E_ERROR("Error reading speech data\n");
		return(FE_INPUT_FILE_READ_ERROR);
	    }
	    process_utt_return_value = 
	      fe_process_utt(FE,spdata,splen,&cep, &frames_proc);
	    if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
	      warn_zero_energy = ON;
	    } else {
	      assert(process_utt_return_value == FE_SUCCESS);
	    }
	    if (frames_proc>0)
		fe_writeblock_feat(P,FE,fp_out,frames_proc,cep);
	    ckd_free_2d((void **)cep);

	    curr_block++;
	    if (P->logspec != ON)
	        last_frame_cep = (float32 **)ckd_calloc_2d(1,FE->NUM_CEPSTRA,sizeof(float32));
	    else
	        last_frame_cep = (float32 **)ckd_calloc_2d(1,FE->MEL_FB->num_filters,sizeof(float32));
	    process_utt_return_value = 
	      fe_end_utt(FE, last_frame_cep[0], &last_frame);
	    if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
	      warn_zero_energy = ON;
	    } else {
	      assert(process_utt_return_value == FE_SUCCESS);
	    }
	    if (last_frame>0){
	      fe_writeblock_feat(P,FE,fp_out,last_frame,last_frame_cep);
	      frames_proc++;
	    }
	    total_frames += frames_proc;
	    	    
	    fe_closefiles(fp_in,fp_out);
	    
	    free(spdata);
	    ckd_free_2d((void **)last_frame_cep);
	}
	else{
	    E_ERROR("fe_start_utt() failed\n");
	    return(FE_START_ERROR);
	}
	
	fe_close(FE);
	if (ON == warn_zero_energy) {
	  E_WARN("File %s has some frames with zero energy. Consider using dither\n", infile);
	}
    }
    else{
	E_ERROR("Unknown mode - single or batch?\n");
	return(FE_UNKNOWN_SINGLE_OR_BATCH);
	
    }

    P->splen=splen;
    P->nframes=nframes;
    P->spdata=spdata;
    return(FE_SUCCESS);
    
}

int16 * fe_convert_files_to_spdata(param_t *P, fe_t *FE, int32 *splenp, int32 *nframesp)
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
                
        fe_build_filenames(P,fileroot,&infile,NULL);
        if (P->verbose) printf("%s\n",infile);

        if (ep_fe_openfiles(P,FE,infile,&fp_in,&total_samps,&nframes,&nblocks) != FE_SUCCESS){       printf("fe_openfiles exited!\n");        
          exit(0);
        }
                

                
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
        
            close(fp_in);
            
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


void fe_validate_parameters(param_t *P) 
{
    
    if ((P->is_batch) && (P->is_single)) {
        E_FATAL("You cannot define an input file and a control file\n");
    }
    
    if (P->wavfile == NULL && P->wavdir == NULL){
        E_FATAL("No input file or file directory given\n");
    }
    
    if (P->cepfile == NULL && P->cepdir == NULL){
        E_FATAL("No cepstra file or file directory given\n");
    }
    
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


param_t *fe_parse_options() 
{
    param_t *P;
    int32 format;
    char *endian;
    
    if ((P=(param_t *)malloc(sizeof(param_t)))==NULL){
        E_FATAL("memory alloc failed in fe_parse_options()\n...exiting\n");
    }
    
    fe_init_params(P);
    
    P->wavfile = cmd_ln_str("-i");
    if (P->wavfile != NULL) {
        P->is_single = ON;
    }
    
    P->cepfile = cmd_ln_str("-o");
    
    P->ctlfile = cmd_ln_str("-c");
    if (P->ctlfile != NULL) {
        P->is_batch = ON;
    }
    
    P->wavdir = cmd_ln_str("-di");
    P->cepdir = cmd_ln_str("-do");
    P->wavext = cmd_ln_str("-ei");
    P->cepext = cmd_ln_str("-eo");
    format = cmd_ln_int32("-raw");
    if (format) {
        P->input_format = RAW;
    }
    format = cmd_ln_int32("-nist");
    if (format) {
        P->input_format = NIST;
    } 
    format = cmd_ln_int32("-mswav");
    if (format) {
        P->input_format = MSWAV;
    }
    
    P->nchans = cmd_ln_int32("-nchans");
    P->whichchan = cmd_ln_int32("-whichchan");
    P->PRE_EMPHASIS_ALPHA = cmd_ln_float32("-alpha");
    P->SAMPLING_RATE = cmd_ln_float32("-srate");
    P->WINDOW_LENGTH = cmd_ln_float32("-wlen");
    P->FRAME_RATE = cmd_ln_int32("-frate");
    if (!strcmp(cmd_ln_str("-feat"), "sphinx")) 
    {
        P->FB_TYPE = MEL_SCALE;
        P->output_endian = BIG;
    } 
    else 
    {
        E_ERROR("MEL_SCALE IS CURRENTLY THE ONLY IMPLEMENTATION\n\n");
        E_FATAL("Make sure you specify '-feat sphinx'\n");
    }	
    P->NUM_FILTERS = cmd_ln_int32("-nfilt");
    P->NUM_CEPSTRA = cmd_ln_int32("-ncep");
    P->LOWER_FILT_FREQ = cmd_ln_float32("-lowerf");
    P->UPPER_FILT_FREQ = cmd_ln_float32("-upperf");
    P->FFT_SIZE = cmd_ln_int32("-nfft");
    if (cmd_ln_int32("-doublebw")) {
        P->doublebw = ON; 
    } else {
        P->doublebw = OFF;
    }
    P->blocksize = cmd_ln_int32("-blocksize");
    P->verbose = cmd_ln_int32("-verbose");
    endian = cmd_ln_str("-mach_endian");
    if (!strcmp("big", endian)) {
        P->machine_endian = BIG;
    } else {
        if (!strcmp("little", endian)) {
            P->machine_endian = LITTLE;
        } else {
            E_FATAL("Machine must be big or little Endian\n");
        }	
    }
    endian = cmd_ln_str("-input_endian");
    if (!strcmp("big", endian)) {
        P->input_endian = BIG;
    } else {
        if (!strcmp("little", endian)) {
            P->input_endian = LITTLE;
        } else {
            E_FATAL("Input must be big or little Endian\n");
        }	
    }
    P->dither = cmd_ln_int32("-dither");
    P->logspec = cmd_ln_int32("-logspec");
    
    if(P->dither){
      fe_init_dither(*(int32 *)cmd_ln_access("-seed"));
    }

    fe_validate_parameters(P);
    
    return (P);
    
}


void fe_init_params(param_t *P)
{
    P->FB_TYPE = DEFAULT_FB_TYPE;
    P->is_batch = OFF;
    P->is_single = OFF;
    P->wavfile = NULL;
    P->cepfile = NULL;
    P->ctlfile = NULL;
    P->wavdir = NULL;
    P->cepdir = NULL;
    P->wavext = NULL;
    P->cepext = NULL;
    
    
}


int32 fe_build_filenames(param_t *P, char *fileroot, char **infilename, char **outfilename)
{
    char cbuf[MAXCHARS];
    char chanlabel[MAXCHARS];
    
    if (P->nchans>1)
        sprintf(chanlabel, ".ch%d", P->whichchan);
    
    if (P->is_batch){
        sprintf(cbuf,"%s","");
        strcat(cbuf,P->wavdir);
        strcat(cbuf,"/");
        strcat(cbuf,fileroot);
        strcat(cbuf,".");
        strcat(cbuf,P->wavext);
	if (infilename != NULL) {
	  *infilename = fe_copystr(*infilename,cbuf);
	}
        
        sprintf(cbuf,"%s","");
        strcat(cbuf,P->cepdir);
        strcat(cbuf,"/");
        strcat(cbuf,fileroot);
        if (P->nchans>1)
            strcat(cbuf, chanlabel);
        strcat(cbuf,".");
        strcat(cbuf,P->cepext);
	if (outfilename != NULL) {
	  *outfilename = fe_copystr(*outfilename,cbuf);	
	}
    }
    else if (P->is_single){
        sprintf(cbuf,"%s","");
        strcat(cbuf,P->wavfile);
	if (infilename != NULL) {
	  *infilename = fe_copystr(*infilename,cbuf);
	}
        
        sprintf(cbuf,"%s","");
        strcat(cbuf,P->cepfile);
	if (outfilename != NULL) {
	  *outfilename = fe_copystr(*outfilename,cbuf);
	}
    }
    else{
        E_FATAL("Unspecified Batch or Single Mode\n");
    }
    
    return 0;
}


char *fe_copystr(char *dest_str, char *src_str)
{
    int i,src_len, len;
    char *s;
    
    src_len = strlen(src_str);
    len = src_len;
    s = (char *)malloc(len+1);
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


/* Temporary hack. This will duplicate some code, but allow wave2feat and main_ep.c coexists in the same codebase. */	
int32 ep_fe_openfiles(param_t *P, fe_t *FE, char *infile, int32 *fp_in, int32 *nsamps, 
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
                if (charPointer == (int16)strlen(dataString)) {
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

int32 fe_openfiles(param_t *P, fe_t *FE, char *infile, int32 *fp_in, int32 *nsamps, 
		   int32 *nframes, int32 *nblocks, char *outfile, int32 *fp_out)
{
    struct stat filestats;
    int fp=0, len=0, outlen, numframes, numblocks;
    FILE *fp2;
    char line[MAXCHARS];
    int got_it=0;
    
    
    /* Note: this is kind of a hack to read the byte format from the
    NIST header */
    if (P->input_format == NIST){
        if ((fp2 = fopen(infile,"rb")) == NULL){
            E_ERROR("Cannot read %s\n",infile);
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
                        E_ERROR("Unknown/unsupported byte order\n");
                }
                else 
                    E_ERROR("Error determining byte format\n");
            }
        }
        if (!got_it){
            E_WARN("Can't find byte format in header, setting to machine's endian\n");
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
                E_ERROR("Cannot read %s\n",infile);
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
                E_ERROR("Cannot allocate for input file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            if (read(fp,hdr_buf,sizeof(MSWAV_hdr)) != sizeof(MSWAV_hdr)){
                E_ERROR("Cannot allocate for input file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            /* Check header */
            if (strncmp(hdr_buf->rifftag,"RIFF",4)!=0 ||
                strncmp(hdr_buf->wavefmttag,"WAVEfmt",7)!=0) {
                E_ERROR("Error in mswav file header\n");
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
                        E_ERROR("Failed reading wav file.\n");
                        return (FE_INPUT_FILE_READ_ERROR);
                    }
                    if (readChar == dataString[charPointer]) {
                        charPointer++;
                    }
                    if (charPointer == (int)strlen(dataString)) {
                        found = ON;
                        strcpy(hdr_buf->datatag, dataString);
                        if (read(fp,&(hdr_buf->datalength),sizeof(int32)) != sizeof(int32)){
                            E_ERROR("Failed reading wav file.\n");
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
                E_ERROR("MS WAV file not in 16-bit PCM format\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            len = hdr_buf->datalength / sizeof(short);
            P->nchans = hdr_buf->numchannels;
            /* DEBUG: Dump Info */
            E_INFO("Reading MS Wav file %s:\n", infile);
            E_INFO("\t16 bit PCM data, %d channels %d samples\n",P->nchans,len);
            E_INFO("\tSampled at %d\n",hdr_buf->SamplingFreq);
            free(hdr_buf);
        }
        else {
            E_ERROR("Unknown input file format\n");
            return(FE_INPUT_FILE_OPEN_ERROR);
        }
    }
    
    
    len = len/P->nchans;
    *nsamps = len;
    *fp_in = fp;
    
    numblocks = (int)((float)len/(float)P->blocksize);
    if (numblocks*P->blocksize<len)
        numblocks++;
    
    *nblocks = numblocks;  
    
    if ((fp = open(outfile, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0644)) < 0) {
        E_ERROR("Unable to open %s for writing features\n",outfile);
        return(FE_OUTPUT_FILE_OPEN_ERROR);
    }
    else{
        /* compute number of frames and write cepfile header */
        numframes = fe_count_frames(FE,len,COUNT_PARTIAL);
        if (P->logspec != ON)
            outlen = numframes*FE->NUM_CEPSTRA;
        else
            outlen = numframes*FE->MEL_FB->num_filters;
        if (P->output_endian != P->machine_endian)
            SWAPL(&outlen);
        if  (write(fp, &outlen, 4) != 4) {
            E_ERROR("Data write error on %s\n",outfile);
            close(fp);
            return(FE_OUTPUT_FILE_WRITE_ERROR);
        }
        if (P->output_endian != P->machine_endian)
            SWAPL(&outlen);
    }
    
    *nframes = numframes;  
    *fp_out = fp;
    
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
  	        E_ERROR("error reading block\n");
		return(0);
	    }	
	}
	else{
  	    E_ERROR("unknown input file format\n");
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
		    E_ERROR("error reading block (got %d not %d)\n",bytes_read,nreadbytes);
		    return(0);
		}
		
		for (j=whichchan-1;j<actsamps;j=j+nchans){
		    buf[k] = tmpbuf[j];
		    k++;
		}
		cum_bytes_read += bytes_read/nchans;
	    }
	    else{
		E_ERROR("unknown input file format\n");
		return(0);
	    }
	    free(tmpbuf);
	}
	else{
	    tmpbuf = (int16 *)calloc(nsamps,sizeof(int16));
	    actsamps = nsamps/nchans;	    
	    cum_bytes_read = 0;
	    
	    if (actsamps*nchans != nsamps){
		E_WARN("Blocksize %d is not an integer multiple of Number of channels %d\n",nsamps, nchans);
	    }
	    
	    if (P->input_format==RAW || P->input_format==NIST){    
		for (i=0;i<nchans;i++){
		    
		    offset = i*actsamps;
		    k=0;
		    nreadbytes = nsamps*sizeof(int16);
		    
		    if ((bytes_read = read(fp,tmpbuf,nreadbytes))!=nreadbytes){
			E_ERROR("error reading block (got %d not %d)\n",bytes_read,nreadbytes);
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
		E_ERROR("unknown input file format\n");
		return(0);
	    }
	    free(tmpbuf);
	}
    }
    
    else{
	E_ERROR("unknown number of channels!\n");
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
        close(fp);
        E_FATAL("Error writing block of features\n");
    }

    if (P->output_endian != P->machine_endian){
	for (i=0;i<length;++i) SWAPF(feat[0]+i);
    }
    
    return(length);
}


int32 fe_closefiles(int32 fp_in, int32 fp_out)
{
    close(fp_in);
    close(fp_out);
    return 0;
}

void fe_init_dither(int32 seed)
{
  if(seed<0){
    E_INFO("You are using the internal mechanism to generate the seed.");
    s3_rand_seed((long)time(0));
  }else{
    E_INFO("You are using %d as the seed.",seed);
    s3_rand_seed(seed);
  }
}

/* adds 1/2-bit noise */
int32 fe_dither(int16 *buffer,int32 nsamps)
{
  int32 i;
  for (i=0;i<nsamps;i++)
    buffer[i] += (short)((!(s3_rand_int31()%4))?1:0);
  
  return 0;
}

int32 fe_free_param(param_t *P)
{

    /* but no one calls it (29/09/00) */
    return 0;
}

