/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/*
 * fe.h
 * 
 * $Log: fe.h,v $
 * Revision 1.11  2005/02/05 02:15:02  egouvea
 * Removed fe_process(), never used
 *
 * Revision 1.10  2004/12/10 16:48:55  rkm
 * Added continuous density acoustic model handling
 *
 *
 */

#if defined(WIN32) && !defined(GNUWINCE)
#define srand48(x) srand(x)
#define lrand48() rand()
#endif

#ifndef _NEW_FE_H_
#define _NEW_FE_H_

#include "fixpoint.h"

#ifdef WORDS_BIGENDIAN
#define NATIVE_ENDIAN "big"
#else
#define NATIVE_ENDIAN "little"
#endif

#define waveform_to_cepstral_command_line_macro() \
  { "-logspec", \
    ARG_BOOLEAN, \
    "no", \
    "Write out logspectral files instead of cepstra" }, \
   \
  { "-smoothspec", \
    ARG_BOOLEAN, \
    "no", \
    "Write out cepstral-smoothed logspectral files" }, \
   \
  { "-transform", \
    ARG_STRING, \
    "legacy", \
    "Which type of transform to use to calculate cepstra (legacy, dct, or htk)" }, \
   \
  { "-spec2cep", \
    ARG_BOOLEAN, \
    "no", \
    "Input is log spectral files, output is cepstral files" }, \
   \
  { "-cep2spec", \
    ARG_BOOLEAN, \
    "no", \
    "Input is cepstral files, output is log spectral files" }, \
   \
  { "-fbtype", \
    ARG_STRING, \
    "mel_scale", \
    "FB Type of mel_scale or log_linear" }, \
   \
  { "-alpha", \
    ARG_FLOAT32, \
    ARG_STRINGIFY(DEFAULT_PRE_EMPHASIS_ALPHA), \
    "Preemphasis parameter" }, \
   \
  { "-samprate", \
    ARG_FLOAT32, \
    ARG_STRINGIFY(DEFAULT_SAMPLING_RATE), \
    "Sampling rate" }, \
   \
  { "-frate", \
    ARG_INT32, \
    ARG_STRINGIFY(DEFAULT_FRAME_RATE), \
    "Frame rate" }, \
   \
  { "-wlen", \
    ARG_FLOAT32, \
    ARG_STRINGIFY(DEFAULT_WINDOW_LENGTH), \
    "Hamming window length" }, \
   \
  { "-nfft", \
    ARG_INT32, \
    ARG_STRINGIFY(DEFAULT_FFT_SIZE), \
    "Size of FFT" }, \
   \
  { "-nfilt", \
    ARG_INT32, \
    ARG_STRINGIFY(DEFAULT_NUM_FILTERS), \
    "Number of filter banks" }, \
   \
  { "-lowerf", \
    ARG_FLOAT32, \
    ARG_STRINGIFY(DEFAULT_LOWER_FILT_FREQ), \
    "Lower edge of filters" }, \
   \
  { "-upperf", \
    ARG_FLOAT32, \
    ARG_STRINGIFY(DEFAULT_UPPER_FILT_FREQ), \
    "Upper edge of filters" }, \
   \
  { "-unit_area", \
    ARG_BOOLEAN, \
    "yes", \
    "Normalize mel filters to unit area" }, \
   \
  { "-round_filters", \
    ARG_BOOLEAN, \
    "yes", \
    "Round mel filter frequencies to DFT points" }, \
   \
  { "-ncep", \
    ARG_INT32, \
    ARG_STRINGIFY(DEFAULT_NUM_CEPSTRA), \
    "Number of cep coefficients" }, \
   \
  { "-doublebw", \
    ARG_BOOLEAN, \
    "no", \
    "Use double bandwidth filters (same center freq)" }, \
   \
  { "-lifter", \
    ARG_INT32, \
    "0", \
    "Length of sin-curve for liftering, or 0 for no liftering." }, \
   \
  { "-input_endian", \
    ARG_STRING, \
    NATIVE_ENDIAN, \
    "Endianness of input data, big or little, ignored if NIST or MS Wav" }, \
   \
  { "-warp_type", \
    ARG_STRING, \
    DEFAULT_WARP_TYPE, \
    "Warping function type (or shape)" }, \
 \
  { "-warp_params", \
    ARG_STRING, \
    NULL, \
    "Parameters defining the warping function" }, \
 \
  { "-dither", \
    ARG_BOOLEAN, \
    "no", \
    "Add 1/2-bit noise" }, \
   \
  { "-seed", \
    ARG_INT32, \
    ARG_STRINGIFY(SEED), \
    "Seed for random number generator; if less than zero, pick our own" }, \
 \
  { "-remove_dc", \
    ARG_BOOLEAN, \
    "no", \
    "Remove DC offset from each frame" }, \
   \
  { "-verbose", \
    ARG_BOOLEAN, \
    "no", \
    "Show input filenames" }
  
    

#ifdef FIXED_POINT
/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/** MFCC computation type. */
typedef fixed32 mfcc_t;
/** Internal computation type. */
typedef fixed32 window_t;

#define FLOAT2MFCC(x) FLOAT2FIX(x)
#define MFCC2FLOAT(x) FIX2FLOAT(x)
#define MFCCMUL(a,b) FIXMUL(a,b)
#define MFCCLN(x,in,out) FIXLN_ANY(x,in,out)
#else /* !FIXED_POINT */

typedef float32 mfcc_t;
typedef float64 window_t;
#define FLOAT2MFCC(x) (x)
#define MFCC2FLOAT(x) (x)
#define MFCCMUL(a,b) ((a)*(b))
#define MFCCLN(x,in,out) log(x)
#endif /* !FIXED_POINT */

/* Values for the 'logspec' field. */
enum {
	RAW_LOG_SPEC = 1,
	SMOOTH_LOG_SPEC = 2
};

/* Values for the 'FB_TYPE' field (actually MEL_SCALE is the only one
 * supported). */
enum {
	MEL_SCALE,
	LOG_LINEAR,
};

/* Values for the 'transform' field. */
enum {
	LEGACY_DCT = 0,
	DCT_II = 1,
        DCT_HTK = 2
};

/** Structure holding front-end parameters. */
typedef struct param_s param_t;
struct param_s {
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
    int32 swap;
    int32 dither;
    int32 seed;
    int32 logspec;
    int32 doublebw;
    int32 verbose;
    char *warp_type;
    char *warp_params;
    int32 transform;
    int32 lifter_val;
    int32 unit_area;
    int32 round_filters;
    int32 remove_dc;
};

/** Base Struct to hold all structure for MFCC computation. */
typedef struct melfb_s melfb_t;
struct melfb_s {
    float32 sampling_rate;
    int32 num_cepstra;
    int32 num_filters;
    int32 fft_size;
    float32 lower_filt_freq;
    float32 upper_filt_freq;
    mfcc_t **filter_coeffs;
    mfcc_t **mel_cosine;
    int32 *left_apex;
    int32 *width;
    int32 doublewide;
    char *warp_type;
    char *warp_params;
    /* Precomputed normalization constants for unitary DCT-II/DCT-III */
    mfcc_t sqrt_inv_n, sqrt_inv_2n;
    /* Value and coefficients for HTK-style liftering */
    int32 lifter_val;
    mfcc_t *lifter;
    /* Normalize filters to unit area */
    int32 unit_area;
    /* Round filter frequencies to DFT points (hurts accuracy, but is
       useful for legacy purposes) */
    int32 round_filters;
};

/* sqrt(1/2), also used for unitary DCT-II/DCT-III */
#define SQRT_HALF FLOAT2MFCC(0.707106781186548)

/** Structure for the front-end computation. */
typedef struct fe_s fe_t;
struct fe_s {
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
    int32 swap;
    int32 dither;
    int32 seed;
    float32 PRE_EMPHASIS_ALPHA;
    int16 *OVERFLOW_SAMPS;
    int32 NUM_OVERFLOW_SAMPS;    
    melfb_t *MEL_FB;
    int32 START_FLAG;
    int16 PRIOR;
    window_t *HAMMING_WINDOW;
    int32 FRAME_COUNTER;
    int32 transform;
    int32 remove_dc;
};

enum {
	FE_SUCCESS = 0,
	FE_OUTPUT_FILE_SUCCESS  = 0,
	FE_CONTROL_FILE_ERROR = -1,
	FE_START_ERROR = -2,
	FE_UNKNOWN_SINGLE_OR_BATCH = -3,
	FE_INPUT_FILE_OPEN_ERROR = -4,
	FE_INPUT_FILE_READ_ERROR = -5,
	FE_MEM_ALLOC_ERROR = -6,
	FE_OUTPUT_FILE_WRITE_ERROR = -7,
	FE_OUTPUT_FILE_OPEN_ERROR = -8,
	FE_ZERO_ENERGY_ERROR = -9,
	FE_INVALID_PARAM_ERROR =  -10
};

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


#define DEFAULT_BLOCKSIZE 200000
#define SEED  -1

/** 
 * Function that sets default values in a param_t structure
 */
void fe_init_params(param_t *P /** An allocated param_t structure */
        );

/**
 * Function that automatically initializes a front-end structure by reading
 * command-line arguments (cmd_ln.h)
 */
fe_t* fe_init_auto(void);

/**
 * Function that initialize the routine 
 * @return, an initialized fe_t structure
 */
fe_t *fe_init(param_t const *P /**< Input: A filled param_t structure */
	);

/** Function call for starting utternace */
int32 fe_start_utt(fe_t *FE /**< Input: A filled FE rountine */
	);

/**
 * Function call for the end of the utterance. It also collect the
 * rest of the sample and put in a cepstral vector: if there are
 * overflow samples remaining, it will pad with zeros to make a
 * complete frame and then process to cepstra. also deactivates
 * start flag of FE, and resets overflow buffer count.
 */
int32 fe_end_utt(fe_t *FE,  /**< Input: A filled FE structure */
		 mfcc_t *cepvector, /**< Input: The cepstral vector */
		 int32 *nframes /**< Number of frames returned */
	);

/**
 * Close the front end. 
 * Free all allocated memory within FE and destroy FE. 
 */
int32 fe_close(fe_t *FE /**< Input, a FE structure */
	);

/**
 * Process only one frame of samples
 **/
int32 fe_process_frame(fe_t *FE,  /**< A FE structure */
		       int16 *spch, /**< The speech samples */
		       int32 nsamps, /**< number of samples*/
		       mfcc_t *fr_cep /**< One frame of cepstrum*/
        );

/** 
 * Process a block of samples, returning as many frames as possible
 **/
int32 fe_process_utt(fe_t *FE,  /**< A FE structure */
		     int16 *spch, /**< The speech samples */
		     int32 nsamps, /**< number of samples*/
		     mfcc_t ***cep_block, /**< Output pointer to cepstra */
		     int32 *nframes /**< Number of frames processed */
	);

/**
 * Free the output pointer returned by fe_process_utt().
 **/
void fe_free_2d(void *arr);

/**
 * Convert a block of mfcc_t to float32 (can be done in-place)
 **/
int32 fe_mfcc_to_float(fe_t *FE,
		       mfcc_t **input,
		       float32 **output,
		       int32 nframes);

/**
 * Convert a block of float32 to mfcc_t (can be done in-place)
 **/
int32 fe_float_to_mfcc(fe_t *FE,
		       float32 **input,
		       mfcc_t **output,
		       int32 nframes);

/**
 * Process one frame of log spectra into MFCC using discrete cosine
 * transform.
 *
 * This uses a variant of the DCT-II where the first frequency bin is
 * scaled by 0.5.  Unless somebody misunderstood the DCT-III equations
 * and thought that's what they were implementing here, this is
 * ostensibly done to account for the symmetry properties of the
 * DCT-II versus the DFT - the first coefficient of the input is
 * assumed to be repeated in the negative frequencies, which is not
 * the case for the DFT.  (This begs the question, why not just use
 * the DCT-I, since it has the appropriate symmetry properties...)
 * Moreover, this is bogus since the mel-frequency bins on which we
 * are doing the DCT don't extend to the edge of the DFT anyway.
 *
 * This also means that the matrix used in computing this DCT can not
 * be made orthogonal, and thus inverting the transform is difficult.
 * Therefore if you want to do cepstral smoothing or have some other
 * reason to invert your MFCCs, use fe_logspec_dct2() and its inverse
 * fe_logspec_dct3() instead.
 *
 * Also, it normalizes by 1/nfilt rather than 2/nfilt, for some reason.
 **/
int32 fe_logspec_to_mfcc(fe_t *FE,  /**< A FE structure */
			 const mfcc_t *fr_spec, /**< One frame of spectrum */
			 mfcc_t *fr_cep /**< One frame of cepstrum */
        );

/**
 * Convert log spectra to MFCC using DCT-II.
 *
 * This uses the "unitary" form of the DCT-II, i.e. with a scaling
 * factor of sqrt(2/N) and a "beta" factor of sqrt(1/2) applied to the
 * cos(0) basis vector (i.e. the one corresponding to the DC
 * coefficient in the output).
 **/
int32 fe_logspec_dct2(fe_t *FE,  /**< A FE structure */
		      const mfcc_t *fr_spec, /**< One frame of spectrum */
		      mfcc_t *fr_cep /**< One frame of cepstrum */
        );

/**
 * Convert MFCC to log spectra using DCT-III.
 *
 * This uses the "unitary" form of the DCT-III, i.e. with a scaling
 * factor of sqrt(2/N) and a "beta" factor of sqrt(1/2) applied to the
 * cos(0) basis vector (i.e. the one corresponding to the DC
 * coefficient in the input).
 **/
int32 fe_mfcc_dct3(fe_t *FE,  /**< A FE structure */
		   const mfcc_t *fr_cep, /**< One frame of cepstrum */
		   mfcc_t *fr_spec /**< One frame of spectrum */
        );

#endif
