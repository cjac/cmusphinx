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
#define int32 int

#ifndef	M_PI
#define M_PI	(3.14159265358979323846)
#endif	M_PI

#define FORWARD_FFT 1
#define INVERSE_FFT -1

typedef struct { double r, i; } complex;

/* functions */
int32 fe_build_melfilters(melfb_t *MEL_FB);
int32 fe_compute_melcosine(melfb_t *MEL_FB);
float fe_mel(float x);
float fe_melinv(float x);
void fe_pre_emphasis(int16 *in, double *out, int32 len, float factor, int16 prior);
void fe_hamming_window(double *in, double *window, int32 in_len);
void fe_init_hamming(double *win, int32 len);
void fe_spec_magnitude(double *data, int32 data_len, double *spec, int32 fftsize);
void fe_frame_to_fea(fe_t *FE, double *in, double *fea);
void fe_mel_spec(fe_t *FE, double *spec, double *mfspec);
void fe_mel_cep(fe_t *FE, double *mfspec, double *mfcep);
int32 fe_fft(complex *in, complex *out, int32 N, int32 invert);
void fe_short_to_double(int16 *in, double *out, int32 len);
char **fe_create_2d(int32 d1, int32 d2, int32 elem_size);
void fe_free_2d(void **arr);
void fe_print_current(fe_t *FE);
void fe_parse_general_params(param_t *P, fe_t *FE);
void fe_parse_melfb_params(param_t *P, melfb_t *MEL);
void fe_create_hamming (double *in, int32 in_len);



