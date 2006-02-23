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

#ifndef _FE_INTERNAL_H_
#define _FE_INTERNAL_H_

/** \file fe_internal.h
    \brief All signal processing functions are opned 
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef	M_PI
#define M_PI	(3.14159265358979323846)
#endif	/* M_PI */

#define FORWARD_FFT 1
#define INVERSE_FFT -1

  /** \struct complex
      \brief A structure to represent complex numbers
   */
typedef struct { float64 r, i; } complex;


  /** Build mel filters */

  int32 fe_build_melfilters(melfb_t *MEL_FB /**< A mel-frequency banks data structure */
			  );

  /** Compute mel cosine. Effectively, applied discrete cosine transform on the filter banks
      output. 
   */
  int32 fe_compute_melcosine(melfb_t *MEL_FB /**< A mel-frequency banks data structure */
			   );

  /** Convert a number to mel scale */
  float32 fe_mel(float32 x /**< A number in real-domain*/
	       );

  /** Convert a number from mel scale to real domain */
  float32 fe_melinv(float32 x /**< A number in mel-scale */
		  );

  /** Perform pre-emphasis 
      The output will be produced based on the following formula:
      
      o[0]=in[0]-factor * prior
      o[i]=in[i]-factor * in[i-1]
   */
  void fe_pre_emphasis(int16 const *in, /**<Input: The input vector */
		       float64 *out,  /**<Output: The output vector*/
		       int32 len,     /**<Input: The length of the input vector in*/
		       float32 factor,  /**<Input: The preemphasis factor */
		       int16 prior      /**< Input: The value that is assumed to be before the first sample */
		       );

  /** Create a hamming window */
void fe_create_hamming(float64 *in, int32 in_len);

  /** Apply a hamming window */
  void fe_hamming_window(float64 *in, /**< Input: The input vector */
			 float64 *window,  /**< Input: The type of window */
			 int32 in_len /**<Input: the length of the input evector in */
		       );

  /** Initialize hamming window 
      The hamming window is initialized using the following formula.
      in[i] = 0.54  - 0.46 * cos (2 * PI * i / (length -1))
   */
  void fe_init_hamming(float64 *win, /**< Input/Output: The input/output vector */
		       int32 len       /**< Input: The length of the vector */
		     );

  /** Compute the magnitude of a spectrum 
   */
  void fe_spec_magnitude(float64 const *data, /**< Input : The input data */
			 int32 data_len,  /**< Input : The data length*/
			 float64 *spec, /**<Input : The output spectrum*/
			 int32 fftsize /**<Input: The size of FFT */
			 );
  
  /** Compute the feature from frame 
   */
  int32 fe_frame_to_fea(fe_t *FE,  /**< Input: A FE structure */
			float64 *in,  /**< Input: The input data */
			float64 *fea  /**< Output: The output feature vector */
			); 

  /** Mel spectrum */
  void fe_mel_spec(fe_t *FE,  /**< Input: A FE structure */
		   float64 const *spec,  /**< Input: The spectrum vector */
		   float64 *mfspec  /**< Output: A mel spectral vector after passing through the filter banks*/
		   );

  /** Mel cepstrum */

  int32 fe_mel_cep(fe_t *FE,  /**< Input: A FE structure */
		   float64 *mfspec, /**< Input:  mel spectral vector after passing through the filter banks */
		   float64 *mfcep   /**< Output: mel cepstral vector after */
		   );

  /** FFT Implementation of both fast Fourier transform (FFT) and
      inverse fast Fourier transform (IFFT)
  */

  int32 fe_fft(complex const *in, /**< Input: The input vector in complex numbers */
	       complex *out,      /**< Output: The output vector in complex numbers */
	       int32 N,           /**< The size of the FFT */
	       int32 invert       /**< invert=-1, IFFT, invert=1, FFT*/
	       );

  /** FFT Implementation of fast Fourier transform (FFT) only,
      optimized for real-valued input
  */
  int32 fe_fft_real(float64 *x, /**< Input/Output: The input vector in real numbers */
		    int n,           /**< The size of the FFT */
		    int m           /**< The order (log2(size)) of the FFT */
	  );

  /** Convert short to double
   */
  void fe_short_to_double(int16 const *in, /**< Input: vector in short */
			  float64 *out,    /**< Output: vector in double */
			  int32 len        /**< Input: Length of the vector */
			  );

  /** DUPLICATION! Front end specific memory allocation routine 
      Create a 2D array. 
      @return: a d1 x d2 array will be returned
   */
  char **fe_create_2d(int32 d1, /** Input: first dimension*/
		      int32 d2,  /** Input: second dimension*/
		      int32 elem_size /** Input : the size of element */
		      );

  /** DUPLICATION! Front end specific memory delallocation routine */
  void fe_free_2d(void **arr /**Input: a 2d vector */
		);

  /** Print the fe structure.  */
  void fe_print_current(fe_t *FE /** Input: an FE structure */
		      );
  
  /**Parse general parameters from a param_t */
  void fe_parse_general_params(param_t const *P, /**Input: a param_t structure*/
			       fe_t *FE /**Output: a FE structure */
			       );

  /**Parse mel frequency parameters from a param_t */
  void fe_parse_melfb_params(param_t const *P, /**Input: a param_t structure*/
			     melfb_t *MEL /**Output: a FE structure */
			     );

#ifdef __cplusplus
}
#endif

#endif
