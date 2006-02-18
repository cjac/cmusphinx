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
#ifndef _FE_H_
#define _FE_H_

#include "s2types.h"
#include "err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    float32 SAMPLING_RATE;
    int32 FRAME_RATE;
    float32 WINDOW_LENGTH;
    int32 FB_TYPE;
    int32 NUM_CEPSTRA;
    int32 NUM_FILTERS;
    int32 FFT_SIZE;
    float32 LOWER_FILT_FREQ;
    float32 UPPER_FILT_FREQ;
    float32 PRE_EMPHASIS_ALPHA;

    char *warp_type;
    char *warp_params;

    char *wavfile;
    char *cepfile;
    char *ctlfile;
    int32 nskip;
    int32 runlen;
    char *wavdir;
    char *cepdir;
    char *wavext;
    char *cepext;
    int32 input_format;
    int32 is_batch;
    int32 is_single;
    int32 blocksize;
    int32 verbose;
    int32 machine_endian;
    int32 input_endian;
    int32 output_endian;
    int32 dither;
    int32 logspec;
    int32 doublebw;
    int32 nchans;
    int32 whichchan;
} param_t;


typedef struct{
    float32 sampling_rate;
    int32 num_cepstra;
    int32 num_filters;
    int32 fft_size;
    float32 lower_filt_freq;
    float32 upper_filt_freq;
    float32 **filter_coeffs;
    float32 **mel_cosine;
    float32 *left_apex;
    int32 *width;
    int32 doublewide;
    char *warp_type;
    char *warp_params;
} melfb_t;


typedef struct{
    float32 SAMPLING_RATE;
    int32 FRAME_RATE;
    int32 FRAME_SHIFT;
    float32 WINDOW_LENGTH;
    int32 FRAME_SIZE;
    int32 FFT_SIZE;
    int32 FB_TYPE;
    int32 LOG_SPEC;
    int32 NUM_CEPSTRA;
    int32 FEATURE_DIMENSION;
    float32 PRE_EMPHASIS_ALPHA;
    int16 *OVERFLOW_SAMPS;
    int32 NUM_OVERFLOW_SAMPS;    
    melfb_t *MEL_FB;
    int32 START_FLAG;
    int16 PRIOR;
    float64 *HAMMING_WINDOW;
    int32 FRAME_COUNTER;
} fe_t;

/* Struct to hold the front-end parameters */
typedef struct{
        param_t *P;
        fe_t *FE;
        int16 *fr_data;
        float32 *fr_cep;
} fewrap_t;



#define MEL_SCALE 1
#define LOG_LINEAR 2

#define ON 1
#define OFF 0

/* Default values */
#define DEFAULT_SAMPLING_RATE 16000.0
#define DEFAULT_FRAME_RATE 100
#define DEFAULT_FRAME_SHIFT 160
/* The default below is set so that we have an integral number of
 * samples in a frame.
 */
#define DEFAULT_WINDOW_LENGTH 0.025625
/* Since the default sampling rate is 16000, let's make the default
 * fft size consistent with it.
 */
#define DEFAULT_FFT_SIZE 512
#define DEFAULT_FB_TYPE MEL_SCALE
#define DEFAULT_NUM_CEPSTRA 13
#define DEFAULT_NUM_FILTERS 40
#define DEFAULT_LOWER_FILT_FREQ 133.33334
#define DEFAULT_UPPER_FILT_FREQ 6855.4976
#define DEFAULT_PRE_EMPHASIS_ALPHA 0.97
#define DEFAULT_START_FLAG 0

#define DEFAULT_WARP_TYPE "inverse_linear"

#define BB_SAMPLING_RATE 16000
#define DEFAULT_BB_FFT_SIZE 512
#define DEFAULT_BB_FRAME_SHIFT 160
#define DEFAULT_BB_NUM_FILTERS 40
#define DEFAULT_BB_LOWER_FILT_FREQ 133.33334
#define DEFAULT_BB_UPPER_FILT_FREQ 6855.4976

#define NB_SAMPLING_RATE 8000
#define DEFAULT_NB_FFT_SIZE 256 /* NOTE!  In actual fact we will
				   default to 512, because this is
				   what SphinxTrain will do.  See
				   fbs_main.c:query_fe_params() */
#define DEFAULT_NB_FRAME_SHIFT 80
#define DEFAULT_NB_NUM_FILTERS 31
#define DEFAULT_NB_LOWER_FILT_FREQ 200
#define DEFAULT_NB_UPPER_FILT_FREQ 3500

#define FE_SUCCESS 0
#define FE_OUTPUT_FILE_SUCCESS 0
#define FE_CONTROL_FILE_ERROR 1
#define FE_START_ERROR 2
#define FE_UNKNOWN_SINGLE_OR_BATCH 3
#define FE_INPUT_FILE_OPEN_ERROR 4
#define FE_INPUT_FILE_READ_ERROR 5
#define FE_MEM_ALLOC_ERROR 6
#define FE_OUTPUT_FILE_WRITE_ERROR 7
#define FE_OUTPUT_FILE_OPEN_ERROR 8
#define FE_ZERO_ENERGY_ERROR 9

#define DEFAULT_BLOCKSIZE 200000
#define DITHER  OFF

/* Interface */
fe_t *fe_init(param_t const *P);

int32 fe_start_utt(fe_t *FE);

int32 fe_end_utt(fe_t *FE, float32 *cepvector, int32 *nframes);

int32 fe_close(fe_t *FE);

int32 fe_process_utt(fe_t *FE, int16 const *spch, int32 nsamps, float32 **cep, int32 *nframes);

#ifdef __cplusplus
}
#endif

#endif

/*
 * fe.h
 * 
 * $Log$
 * Revision 1.14  2006/02/18  00:11:00  egouvea
 * Closing bracket if __cplusplus defined.
 * 
 * Revision 1.13  2006/02/17 00:49:57  egouvea
 * Yet another attempt at synchronizing the front end code between
 * SphinxTrain and sphinx2.
 *
 * Added support for warping functions.
 *
 * Replaced some fprintf() followed by exit() with E_WARN and return() in
 * functions that had a non void return type.
 *
 * Set return value to FE_ZERO_ENERGY_ERROR if the energy is zero in a
 * frame, allowing the application to do something (currently, uttproc
 * and raw2cep simply print a message.
 *
 * Warning: the return value in fe_process_utt() and fe_end_utt()
 * required a change in the API (the return value has a different meaning
 * now).
 *
 * Revision 1.12  2005/10/11 13:08:40  dhdfu
 * Change the default FFT size for 8kHz to 512, as that is what Communicator models are.  Add command-line arguments to specify all FE parameters, thus removing the 8 or 16kHz only restriction.  Add default parameters for 11025Hz as well
 *
 * Revision 1.11  2005/02/05 02:15:02  egouvea
 * Removed fe_process(), never used
 *
 * Revision 1.10  2004/12/10 16:48:55  rkm
 * Added continuous density acoustic model handling
 *
 *
 */
