/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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

#ifdef WIN32			/* RAH, needed for memcpy */
#include <memory.h>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "s3types.h"

#include "libs3decoder/fe.h"
#include "libs3decoder/fe_internal.h"
#include "fe_internal_dump.h"
#include "fe_dump.h"
#include "metrics.h"


/*********************************************************************

   FUNCTION: fe_dump_process_utt
   PARAMETERS: fe_t *FE, int16 *spch, int32 nsamps, float **cep
   RETURNS: number of frames of cepstra computed 
   DESCRIPTION: processes the given speech data and returns
   features. will prepend overflow data from last call and store new
   overflow data within the FE

**********************************************************************/

int32 fe_dump_process_utt(fe_t *FE, int16 *spch, int32 nsamps, float32 ***cep_block)	/* RAH, upgraded cep_block to float32 */
{
    int32 frame_start, frame_count=0, whichframe=0;
    int32 i, spbuf_len, offset=0;  
    double *spbuf, *fr_data, *fr_fea;
    int16 *tmp_spch = spch;
    float32 **cep=NULL;
    
    /* are there enough samples to make at least 1 frame? */
    if (nsamps+FE->NUM_OVERFLOW_SAMPS >= FE->FRAME_SIZE) {
      
        /* if there are previous samples, pre-pend them to input */
        /* speech samps */
        if ((FE->NUM_OVERFLOW_SAMPS > 0)) {
            if ((tmp_spch = (int16 *) malloc 
                 (sizeof(int16)*(FE->NUM_OVERFLOW_SAMPS +nsamps)))==NULL) {
                fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
                exit(0);
            }
            /* RAH */
            memcpy(tmp_spch,FE->OVERFLOW_SAMPS,FE->NUM_OVERFLOW_SAMPS*(sizeof(int16)));
            /* RAH */
            memcpy(tmp_spch+FE->NUM_OVERFLOW_SAMPS, spch, nsamps*(sizeof(int16))); /* RAH */
            nsamps += FE->NUM_OVERFLOW_SAMPS;
            FE->NUM_OVERFLOW_SAMPS = 0; /*reset overflow samps count */
        }
        
        /* compute how many complete frames can be processed and
           which samples correspond to those samps */
        frame_count=0;
        for (frame_start=0; frame_start+FE->FRAME_SIZE <= nsamps;
             frame_start += FE->FRAME_SHIFT)
            frame_count++;
        
        /* if (cep!=NULL) fe_free_2d((void**)cep); */ /* It should never 
           not be NULL */
        if ((cep = (float32 **)fe_create_2d
             (frame_count+1,FE->NUM_CEPSTRA,sizeof(float32))) == NULL) {
            fprintf(stderr,"memory alloc for cep failed in fe_process_utt()\n\tfe_create_2d(%ld,%d,%d)\n...exiting\n",(long int) (frame_count+1),FE->NUM_CEPSTRA,sizeof(float32));  /* typecast to make the compiler happy - EBG */
            exit(0);
        }
        
        
        spbuf_len = (frame_count-1)*FE->FRAME_SHIFT + FE->FRAME_SIZE;    

        if ((spbuf=(double *)calloc(spbuf_len, sizeof(double)))==NULL){
            fprintf(stderr,"memory alloc failed in fe_process_utt()\n...exiting\n");
            exit(0);
        }

        if (fe_dump) {
            fe_dump_short_frame(fe_dumpfile, tmp_spch, spbuf_len);
        }

        /* pre-emphasis if needed,convert from int16 to double */
        metricsStart("preemphasis");

        if (FE->PRE_EMPHASIS_ALPHA != 0.0) {
            fe_pre_emphasis(tmp_spch, spbuf, spbuf_len,
                            FE->PRE_EMPHASIS_ALPHA, FE->PRIOR);
        } else {
            fe_short_to_double(tmp_spch, spbuf, spbuf_len);
        }

        metricsStop("preemphasis");

        if (fe_dump) {
            fe_dump_double_frame(fe_dumpfile, spbuf, spbuf_len, "PREEMPHASIS");
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
	
            metricsStart("HammingWindow");

            fe_hamming_window(fr_data, FE->HAMMING_WINDOW, FE->FRAME_SIZE);

            metricsStop("HammingWindow");

            if (fe_dump) {
                fe_dump_double_frame
                    (fe_dumpfile, fr_data, FE->FRAME_SIZE, "HAMMING_WINDOW");
            }

            fe_frame_to_fea_dump(FE, fr_data, fr_fea);

            for (i=0;i<FE->NUM_CEPSTRA;i++)
                cep[whichframe][i] = (float32)fr_fea[i];
        }

        /* done making cepstra */

        if (fe_dump) {
            fe_dump2d_float_frame(fe_dumpfile, cep, frame_count, 
                                  FE->NUM_CEPSTRA,
                                  "CEPSTRUM_PRODUCER", "CEPSTRUM");
        }

        /* assign samples which don't fill an entire frame to FE */
        /* overflow buffer for use on next pass */
        if (spbuf_len < nsamps)	{
            offset = ((frame_count)*FE->FRAME_SHIFT);
            memcpy(FE->OVERFLOW_SAMPS,tmp_spch+offset,
                   (nsamps-offset)*sizeof(int16));
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
 *
 * FUNCTION: fe_end_utt
 * PARAMETERS: fe_t *FE, float *cepvector
 * RETURNS: number of frames processed (0 or 1) 
 * DESCRIPTION: if there are overflow samples remaining, it will pad
 * with zeros to make a complete frame and then process to
 * cepstra. also deactivates start flag of FE, and resets overflow
 * buffer count.
 * 
 *********************************************************************/

int32 fe_dump_end_utt(fe_t *FE, float32 *cepvector)
{
    int32 pad_len=0, frame_count=0;
    int32 i;
    double *spbuf, *fr_fea = NULL;
    
    /* if there are any samples left in overflow buffer, pad zeros to
       make a frame and then process that frame */
    
    if ((FE->NUM_OVERFLOW_SAMPS > 0)) { 
        pad_len = FE->FRAME_SIZE - FE->NUM_OVERFLOW_SAMPS;
        memset(FE->OVERFLOW_SAMPS+(FE->NUM_OVERFLOW_SAMPS), 0,
               pad_len*sizeof(int16));
        FE->NUM_OVERFLOW_SAMPS += pad_len;
        assert(FE->NUM_OVERFLOW_SAMPS==FE->FRAME_SIZE);
        
        if ((spbuf=(double *)calloc(FE->FRAME_SIZE,sizeof(double)))==NULL){
            fprintf(stderr,"memory alloc failed in fe_end_utt()\n...exiting\n");
            exit(0);
        }
        
        if (fe_dump) {
            fe_dump_short_frame(fe_dumpfile, FE->OVERFLOW_SAMPS,
                                FE->NUM_OVERFLOW_SAMPS);
        }

        /* pre-emphasis if needed,convert from int16 to double */
        metricsStart("preemphasis");

        if (FE->PRE_EMPHASIS_ALPHA != 0.0){
            fe_pre_emphasis(FE->OVERFLOW_SAMPS, spbuf,
                            FE->FRAME_SIZE,FE->PRE_EMPHASIS_ALPHA, FE->PRIOR);
        } else {
            fe_short_to_double(FE->OVERFLOW_SAMPS, spbuf, FE->FRAME_SIZE);
        }

        metricsStop("preemphasis");
        
        if (fe_dump) {
            fe_dump_double_frame(fe_dumpfile, spbuf, FE->FRAME_SIZE,
                                 "PREEMPHASIS");
        }

        /* again, who should implement cep vector? this can be implemented
           easily from outside or easily from in here */
        if ((fr_fea = (double *)calloc(FE->NUM_CEPSTRA, sizeof(double)))
            == NULL){
            fprintf(stderr,"memory alloc failed in fe_end_utt()\n...exiting\n");
            exit(0);
        }
        
        metricsStart("HammingWindow");
        fe_hamming_window(spbuf, FE->HAMMING_WINDOW, FE->FRAME_SIZE);
        metricsStop("HammingWindow");

        if (fe_dump) {
            fe_dump_double_frame
                (fe_dumpfile, spbuf, FE->FRAME_SIZE, "HAMMING_WINDOW");
        }

        fe_frame_to_fea_dump(FE, spbuf, fr_fea);
 
        for (i=0;i<FE->NUM_CEPSTRA;i++)
            cepvector[i] = (float32)fr_fea[i];
        
        if (fe_dump) {
            fe_dump_float_frame(fe_dumpfile, cepvector, 
                                  FE->NUM_CEPSTRA,
                                  "CEPSTRUM_PRODUCER", "CEPSTRUM");
        }

        frame_count=1;
        free(fr_fea);               /* RAH - moved up */
        free (spbuf);               /* RAH */
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
 *
 * FUNCTION: fe_dump_preemphasis
 * DESCRIPTION: dumps the results of the preemphasis step in a single
 *              line in the given FILE
 *	
 *********************************************************************/

void fe_dump_short_frame(FILE *stream, short *shortframe,
			 int bufferSize)
{
    int i;
    i = 0;
    fprintf(stream, "FRAME_SOURCE %d", bufferSize);
    for (i = 0; i < bufferSize; i++) {
	fprintf(stream, " %d", shortframe[i]);
    }
    fprintf(stream, "\n");
}


/*********************************************************************
 *
 * FUNCTION: fe_dump_preemphasis
 * DESCRIPTION: dumps the results of the preemphasis step in a single
 *              line in the given FILE
 *	
 *********************************************************************/

void fe_dump_double_frame(FILE *stream, double *preemphasizedAudio,
			  int bufferSize, char *processorName)
{
    int j;
    union data64 x;

    fprintf(stream, "%s %d", processorName, bufferSize);

    for (j = 0; j < bufferSize; j++) {
        x.f = preemphasizedAudio[j];
        
        if (fe_dump_type == HEXADECIMAL) {
            fprintf(stream, " 0x%08x%08x", (unsigned int) x.i.half1, 
		    (unsigned int) x.i.half2);
        } else if (fe_dump_type == DECIMAL) { 
            fprintf(stream, " %15.5f", x.f);
        } else if (fe_dump_type == SCIENTIFIC) {
            fprintf(stream, " %.8E", x.f);
        }
    }
    fprintf(stream, "\n");
}


/*********************************************************************
 *
 * FUNCTION: fe_dump_float_frame
 * DESCRIPTION: dumps the given float32 vector to the given FILE
 *    in the following format:
 *     PROCESSOR_NAME 0 SIZE_0 data0 data1 data2 ...
 *
 *********************************************************************/

void fe_dump_float_frame(FILE *stream, float32 *data, int eachArraySize,
			 char *processorName, char *individualName)
{
    int j;
    union data32 x;

    fprintf(stream, "%s\n", processorName);

    fprintf(stream, "%s %d ", individualName, eachArraySize);

    for (j = 0; j < eachArraySize; j++) {

      x.f = data[j];

      if (fe_dump_type == HEXADECIMAL) {
	fprintf(stream, " 0x%08x%08x", x.i.half1, x.i.half2);
      } else if (fe_dump_type == DECIMAL) {
	fprintf(stream, " %9.5f", x.f);
      } else if (fe_dump_type == SCIENTIFIC) {
	fprintf(stream, " %.8E", x.f);
      }
    }

    fprintf(stream, "\n");
}


/*********************************************************************
 *
 * FUNCTION: fe_dump_2d_float_frame
 * DESCRIPTION: dumps the given 2D float32 array to the given FILE
 *    in the following format:
 *     PROCESSOR_NAME 0 SIZE_0 data0 data1 data2 ...
 *     PROCESSOR_NAME 1 SIZE_1 data0 data1 data2 ...
 *
 *********************************************************************/

void fe_dump2d_float_frame(FILE *stream, float32 **data, int array2DSize,
			   int eachArraySize, char *processorName,
			   char *individualName)
{
    int i, j;
    union data32 x;

    fprintf(stream, "%s %d\n", processorName, array2DSize);

    for (i = 0; i < array2DSize; i++) {

        fprintf(stream, "%s %d", individualName, eachArraySize);

	for (j = 0; j < eachArraySize; j++) {

            x.f = data[i][j];

            if (fe_dump_type == HEXADECIMAL) {
                fprintf(stream, " 0x%08x%08x", x.i.half1, x.i.half2);
            } else if (fe_dump_type == DECIMAL) {
                fprintf(stream, " %9.5f", x.f);
            } else if (fe_dump_type == SCIENTIFIC) {
                fprintf(stream, " %.8E", x.f);
            }
	}

	fprintf(stream, "\n");
    }
}
