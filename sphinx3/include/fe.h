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
 * fe.h -- Feature vector description and cepstra->feature computation.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.25  2006/02/23 03:53:02  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added fe_convert_one_file
 * 2, Use fe_convert_one_file twice in fe_convert_files instead of repeating 200 lines of code twice.
 * 3, Replaced -srate by -samprate
 * 4, Replaced -mach_endian by -machine_endian
 * 5, eliminate ep_fe_openfiles.
 * 6, Fixed dox-doc.
 *
 * Revision 1.24.4.4  2005/09/25 18:58:18  arthchan2003
 * Remove things like SWAPbla from fe.h, now put all to bio.h
 *
 * Revision 1.24.4.3  2005/07/18 19:07:42  arthchan2003
 * 1, Added keyword , 2, Remove unnecessry E_INFO, 3, resolved conflicts in command-line names between wave2feat/ep and decode,  because both ep and wave2feat are relatively new, both follow decode's convention, now call -mach_endian to be -machine_endian, -srate to be -samprate. 4, assert, FRAME_SIZE not equal to 0, in fe_count_frame, if not that could cause infinite loop.
 *
 *
 */

#ifndef _FE_H_
#define _FE_H_


#include <s3types.h>
#include <bio.h>

/** \file fe.h
    \brief High level function for converting waveforms to cepstral
 */
#ifdef __cplusplus
extern "C" {
#endif

  /**
   *  \struct param_t
   *  \brief  Base Struct to hold all front-end parameters for computation.
   */

typedef struct{
    float32 SAMPLING_RATE;
    int32 FRAME_RATE;
    float32 WINDOW_LENGTH;
    int32 FB_TYPE;
    int32 NUM_CEPSTRA;
    int32 NUM_FILTERS;
    int32 FFT_SIZE;
    int32 LOG_SPEC;
    int32 FEATURE_DIMENSION;
    float32 LOWER_FILT_FREQ;
    float32 UPPER_FILT_FREQ;
    float32 PRE_EMPHASIS_ALPHA;

    char *wavfile;
    char *cepfile;
    char *ctlfile;
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
  
  int32 splen;
  int32 nframes;
  int16* spdata;
} param_t;


  /**
   * \struct melfb_t
   * \brief Base Struct to hold all structure for MFCC computation. 
   */

typedef struct{
  float32 sampling_rate;  /**< Sampling rate  */
  int32 num_cepstra;      /**< Number of cepstra */
  int32 num_filters;      /**< Number of filters */
  int32 fft_size;         /**< Size of FFT */
    float32 lower_filt_freq;
    float32 upper_filt_freq;
    float32 **filter_coeffs;
    float32 **mel_cosine;
    float32 *left_apex;
    int32 *width;
    int32 doublewide;
}melfb_t;

  /**
   * \struct fe_t
   * \brief Structure that hold information and variable for the front end. 
   */
typedef struct{
    float32 SAMPLING_RATE;
    int32 FRAME_RATE;
    int32 FRAME_SHIFT;
    float32 WINDOW_LENGTH;
    int32 FRAME_SIZE;
    int32 FFT_SIZE;
    int32 FB_TYPE;
    int32 NUM_CEPSTRA;
    float32 PRE_EMPHASIS_ALPHA;
    int16 *OVERFLOW_SAMPS;
    int32 NUM_OVERFLOW_SAMPS;    
    melfb_t *MEL_FB;
    int32 START_FLAG;
    int16 PRIOR;
    float64 *HAMMING_WINDOW;
    int32 FRAME_COUNTER;
  int32 dither;
} fe_t;

  /**
   * \struct fewrap_t
   * \brief Wrapper structure to hold the front-end parameters  
   */
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
#define DEFAULT_SAMPLING_RATE "16000.0" /**Default sampling rate */
#define DEFAULT_FRAME_RATE "100"  /**Default frame rate */
#define DEFAULT_FRAME_SHIFT "160" /**Default frame shift */
#define DEFAULT_WINDOW_LENGTH "0.025625" /** Default window length */
#define DEFAULT_FFT_SIZE "512" /** Default FFT size */
#define DEFAULT_FB_TYPE MEL_SCALE  /** Default filter bank type */
#define DEFAULT_NUM_CEPSTRA "13"  /** Default number of ceptral */
#define DEFAULT_NUM_FILTERS "40" /** Default number of filter banks */
#define DEFAULT_LOWER_FILT_FREQ "133.33334" /** Default lower filter frequency */
#define DEFAULT_UPPER_FILT_FREQ "6855.4976" /** Default upper filter frequency */
#define DEFAULT_PRE_EMPHASIS_ALPHA "0.97" /** Default emphasis alpha */
#define DEFAULT_START_FLAG 0

#define BB_SAMPLING_RATE 16000 /**(For 16K sampling frequency) Broad band sampling rate */
#define DEFAULT_BB_FFT_SIZE 512 /**(For 16K sampling frequency)Broad band FFT sampling rate */
#define DEFAULT_BB_FRAME_SHIFT 160 /** (For 16K sampling frequency) Broad band frame shift  */
#define DEFAULT_BB_NUM_FILTERS 40  /** (For 16K sampling frequency) Broad band number of filters   */
#define DEFAULT_BB_LOWER_FILT_FREQ 133.33334  /** (For 16K sampling frequency) Broad lower filter frequency   */
#define DEFAULT_BB_UPPER_FILT_FREQ 6855.4976 /** (For 16K sampling frequency) Broad upper filter frequency   */

#define NB_SAMPLING_RATE 8000 /**(For 8k sampling frequency) Narrow band sampling rate */
#define DEFAULT_NB_FFT_SIZE 256 /*512*/ /**(For 8k sampling frequency) Narrow band FFT sampling rate */
#define DEFAULT_NB_FRAME_SHIFT 80  /** (For 8k sampling frequency) Narrow band frame shift  */
#define DEFAULT_NB_NUM_FILTERS 31 /** (For 8k sampling frequency) Narrow band number filters  */
#define DEFAULT_NB_LOWER_FILT_FREQ 200  /** (For 8k sampling frequency) Narrow lower filter frequency   */
#define DEFAULT_NB_UPPER_FILT_FREQ 3500   /** (For 8k sampling frequency) Narrow upper filter frequency   */

#define DEFAULT_BLOCKSIZE "200000" /** Default block size*/
#define DITHER  OFF /** By default, dithering is off */


/* The following only use in the application level */

#define NULL_CHAR '\0'
#define MAXCHARS 2048

#define WAV 1
#define RAW 2
#define NIST 3
#define MSWAV 4

#define ONE_CHAN "1"

#define LITTLE 1
#define BIG 2

#define FE_SUCCESS 0 /** A general ID for successful operation. */
#define FE_OUTPUT_FILE_SUCCESS 0 /** Synonym of FE_SUCCESS ?*/
#define FE_CONTROL_FILE_ERROR 1 /** Unable to open a control file */
#define FE_START_ERROR 2 /** An error that indicated the front end cannot start correctly */
#define FE_UNKNOWN_SINGLE_OR_BATCH 3 /** Cannot determine whether the frontend is in batch mode or live mode */
#define FE_INPUT_FILE_OPEN_ERROR 4 /** Cannot open an input file */
#define FE_INPUT_FILE_READ_ERROR 5  /** Cannot read an input file */
#define FE_MEM_ALLOC_ERROR 6 /** Cannot allocate memory */
#define FE_OUTPUT_FILE_WRITE_ERROR 7 /** Cannot write on an output file  */
#define FE_OUTPUT_FILE_OPEN_ERROR 8 /** Cannot open an output file */
#define FE_ZERO_ENERGY_ERROR 9  /** The frame consists of no frames that has positive energy*/

#define COUNT_PARTIAL 1
#define COUNT_WHOLE 0
#define HEADER_BYTES 1024
/*
  #if defined(ALPHA) || defined(ALPHA_OSF1) || defined(alpha_osf1) || defined(__alpha) || defined(mips) 
*/
/*#define SWAPBYTES*/


  /** 
      \struct MSWAV_hdr
      \brief A MS Wavefile header. 
      Some defines for MS Wav Files 
      The MS Wav file is a RIFF file, and has the following 44 byte header 
  */
typedef struct RIFFHeader{
    char rifftag[4];      /**< "RIFF" string */
    int32 TotalLength;      /**< Total length */
    char wavefmttag[8];   /**< "WAVEfmt " string (note space after 't') */
    int32 RemainingLength;  /**< Remaining length */
    int16 data_format;    /**< data format tag, 1 = PCM */
    int16 numchannels;    /**< Number of channels in file */
    int32 SamplingFreq;     /**< Sampling frequency */
    int32 BytesPerSec;      /**< Average bytes/sec */
    int16 BlockAlign;     /**< Block align */
    int16 BitsPerSample;  /**< 8 or 16 bit */
    char datatag[4];      /**< "data" string */
    int32 datalength;       /**< Raw data length */
} MSWAV_hdr;



/* Functions */

  /**
     Function that initialize the routine 
     @return, an initialized fe_t structure
   */
  fe_t *fe_init(param_t const *P /**< Input: A filled param_t structure */
	      );

  /**
     Function call for starting utternace 
   */
  int32 fe_start_utt(fe_t *FE /**< Input: A filled FE rountine */
		   );

  /**
     Function call for the end of the utterance. It also collect the
     rest of the sample and put in a cepstral vector: if there are
     overflow samples remaining, it will pad with zeros to make a
     complete frame and then process to cepstra. also deactivates
     start flag of FE, and resets overflow buffer count.
   */
  int32 fe_end_utt(fe_t *FE,  /**< Input: A filled FE structure */
		   float32 *cepvector, /**< Input: The cepstral vector */
		   int32 *nframes /**Output: number of frames */
		   );

  /**
     Close the front end. 
     Free all allocated memory within FE and destroy FE. 
   */
  int32 fe_close(fe_t *FE /**< Input, a FE structure */
		 );
  
  /**
     Process frames. 
     A wrapper function to batch process a block of cepstrum
   */
		 
  int32 fe_process(fe_t *FE,  /**< A FE structure */
		   int16 *spch, /**< */
		   int32 nsamps,  /**< Number of samples */
		   float32 ***cep_block /**< A block of cepstrum*/
		   );

  /**
     Process only one frame of samples
   */
  int32 fe_process_frame(fe_t *FE,  /**< A FE structure */
			 int16 *spch, /**< The speech samples */
			 int32 nsamps, /**< number of samples*/
			 float32 *fr_cep /**< One frame of cepstrum*/
			 );

  int32 fe_process_utt(fe_t *FE,  /**< A FE structure */
		       int16 *spch, /**< The speech samples */
		       int32 nsamps, /**< number of samples*/
		       float32 ***cep_block, 
		       int32 *nframes);

  /** 
      Functions that wrap up the front-end operations on the front-end
      wrapper operations.  
  */
  fewrap_t * few_initialize();

  /**
   *  Free the FEW structure
   */
  void few_free(fewrap_t *FEW /**< the FEW structure one wants to free*/
		);

  /**
     Function that parse the command line and put it into the param_t structure. 
   */
  param_t *fe_parse_options();
  
  /** 
      Function that initialize give a simple param_t structure 
   */
  void fe_init_params(param_t *P /** An allocated param_t structure */
		      );
  
  /**
   * The master function that control conversion of batch of waveforms or a waveform to cepstrals. 
     
   */
  int32 fe_convert_files(param_t *P /**An allocated param_t structure */
			 );
int16 * fe_convert_files_to_spdata(param_t *P, fe_t *FE, int32 *splenp, int32 *nframesp);

  /**
     When P->is_batch :  
     When P->is_single :
   */
  int32 fe_build_filenames(param_t *P,  /**<In: a parameter structure */
			   char *fileroot,  /**< In: the file root */
			   char **infilename, /**< In?: Infilename */
			   char **outfilename /**< In?: Outfilename */
			   );

  /** 
      Copy string from src_str to dest_str 
      Notice that both strings need to be pre-allocated by users. 
   */
  char *fe_copystr(char *dest_str,  /**< Out: a destination string */
		   char *src_str    /**< In: a source string */
		   );

  /**
     Count number of frames for ?
   */
  int32 fe_count_frames(fe_t *FE,  /**<In: a front end structure */
			int32 nsamps, 
			int32 count_partial_frames
			);
int32 fe_readspch(param_t *P, char *infile, int16 **spdata, int32 *splen);
int32 fe_writefeat(fe_t *FE, char *outfile, int32 nframes, float32 **feat);
int32 fe_free_param(param_t *P);

  int32 fe_openfiles(param_t *P,       /**<In: a parameter structure */
		     fe_t *FE,         /**<In: a front-end parameter structure */
		     char *infile,     /**<In: The input file string */
		     int32 *fp_in,     /**<Out: Input file pointer.  */
		     int32 *nsamps,    /**<Out: Number of samples.  */
		     int32 *nframes,   /**<Out: Number of frames.  */
		     int32 *nblocks,   /**<Out: Number of blocks. */
		     char *outfile,    /**<In: The input file string */
		     int32 *fp_out     /**<Out: Output file pointer */
		     );

  int32 fe_readblock_spch(param_t *P, /**< In: a parameter structure */
			  int32 fp,  /**< In: The input file pointer*/
			  int32 nsamps, /**< In: Number of samples */
			  int16 *buf
			  );
int32 fe_writeblock_feat(param_t *P, fe_t *FE, int32 fp, int32 nframes, float32 **feat);
int32 fe_closefiles(int32 fp_in, int32 fp_out);


  /** Initialize dither with seed */
  void fe_init_dither(int32 seed /**< In: seed to initialize dither */
		      );

  /**
     Apply dither on the samples of frames. 
   */
  int32 fe_dither(int16 *buffer, /**<In/Out: a buffer of samples */
		  int32 nsamps   /**<In: number of samples */
		  );
  

#ifdef __cplusplus
}
#endif

#endif
