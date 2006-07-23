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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fe.h"
#include "fe_internal.h"
#include "fe_warp.h"


/*
  31 Jan 00 mseltzer - changed rounding of filter edges to -not- use 
                        rint() function. 
   3 Dec 99 mseltzer - corrected inverse DCT-2 
                        period is 1/NumFilts not 1/(2*NumFilts)
                        added "beta" factor in summation
                     - changed mel filter bank construction so that 
                        left,right,center freqs are rounded to DFT 
                        points before filter is constructed
  
*/


int32
fe_build_melfilters(melfb_t * MEL_FB)
{
    int32 i, whichfilt, start_pt;
    float32 leftfr, centerfr, rightfr, fwidth, height, *filt_edge;
    float32 melmax, melmin, dmelbw, freq, dfreq, leftslope =
        0, rightslope = 0;

    /*estimate filter coefficients */
    MEL_FB->filter_coeffs =
        (float32 **) fe_create_2d(MEL_FB->num_filters, MEL_FB->fft_size,
                                  sizeof(float32));
    MEL_FB->left_apex =
        (float32 *) calloc(MEL_FB->num_filters, sizeof(float32));
    MEL_FB->width = (int32 *) calloc(MEL_FB->num_filters, sizeof(int32));

    if (MEL_FB->doublewide == ON)
        filt_edge =
            (float32 *) calloc(MEL_FB->num_filters + 4, sizeof(float32));
    else
        filt_edge =
            (float32 *) calloc(MEL_FB->num_filters + 2, sizeof(float32));

    if (MEL_FB->filter_coeffs == NULL || MEL_FB->left_apex == NULL
        || MEL_FB->width == NULL || filt_edge == NULL) {
        fprintf(stderr,
                "memory alloc failed in fe_build_mel_filters()\n...exiting\n");
        exit(0);
    }

    dfreq = MEL_FB->sampling_rate / (float32) MEL_FB->fft_size;

    melmax = fe_mel(MEL_FB->upper_filt_freq);
    melmin = fe_mel(MEL_FB->lower_filt_freq);
    dmelbw = (melmax - melmin) / (MEL_FB->num_filters + 1);

    if (MEL_FB->doublewide == ON) {
        melmin = melmin - dmelbw;
        melmax = melmax + dmelbw;
        if ((fe_melinv(melmin) < 0) ||
            (fe_melinv(melmax) > MEL_FB->sampling_rate / 2)) {
            fprintf(stderr, "Out of Range: low  filter edge = %f (%f)\n",
                    fe_melinv(melmin), 0.0);
            fprintf(stderr, "              high filter edge = %f (%f)\n",
                    fe_melinv(melmax), MEL_FB->sampling_rate / 2);
            fprintf(stderr, "exiting...\n");
            exit(0);
        }
    }

    if (MEL_FB->doublewide == ON) {
        for (i = 0; i <= MEL_FB->num_filters + 3; ++i) {
            filt_edge[i] = fe_melinv(i * dmelbw + melmin);
        }
    }
    else {
        for (i = 0; i <= MEL_FB->num_filters + 1; ++i) {
            filt_edge[i] = fe_melinv(i * dmelbw + melmin);
        }
    }

    for (whichfilt = 0; whichfilt < MEL_FB->num_filters; ++whichfilt) {
        /*line triangle edges up with nearest dft points... */
        if (MEL_FB->doublewide == ON) {
            leftfr =
                (float32) ((int32) ((filt_edge[whichfilt] / dfreq) + 0.5))
                * dfreq;
            centerfr = (float32) ((int32)
                                  ((filt_edge[whichfilt + 2] / dfreq) +
                                   0.5)) * dfreq;
            rightfr = (float32) ((int32)
                                 ((filt_edge[whichfilt + 4] / dfreq) +
                                  0.5)) * dfreq;
        }
        else {
            leftfr =
                (float32) ((int32) ((filt_edge[whichfilt] / dfreq) + 0.5))
                * dfreq;
            centerfr = (float32) ((int32)
                                  ((filt_edge[whichfilt + 1] / dfreq) +
                                   0.5)) * dfreq;
            rightfr = (float32) ((int32)
                                 ((filt_edge[whichfilt + 2] / dfreq) +
                                  0.5)) * dfreq;
        }
        MEL_FB->left_apex[whichfilt] = leftfr;
        fwidth = rightfr - leftfr;

        /* 2/fwidth for triangles of area 1 */
        height = 2 / (float32) fwidth;
        if (centerfr != leftfr) {
            leftslope = height / (centerfr - leftfr);
        }
        if (centerfr != rightfr) {
            rightslope = height / (centerfr - rightfr);
        }

        /* Round to the nearest integer instead of truncating and adding
           one, which breaks if the divide is already an integer */
        start_pt = (int32) (leftfr / dfreq + 0.5);
        freq = (float32) start_pt *dfreq;
        i = 0;

        while (freq < centerfr) {
            MEL_FB->filter_coeffs[whichfilt][i] =
                (freq - leftfr) * leftslope;
            freq += dfreq;
            i++;
        }
        /* If the two floats are equal, the leftslope computation above
           results in Inf, so we handle the case here. */
        if (freq == centerfr) {
            MEL_FB->filter_coeffs[whichfilt][i] = height;
            freq += dfreq;
            i++;
        }
        while (freq < rightfr) {
            MEL_FB->filter_coeffs[whichfilt][i] =
                (freq - rightfr) * rightslope;
            freq += dfreq;
            i++;
        }
        MEL_FB->width[whichfilt] = i;
    }

    free(filt_edge);
    return (0);
}

int32
fe_compute_melcosine(melfb_t * MEL_FB)
{

    float32 period, freq;
    int32 i, j;

    period = (float32) 2 *MEL_FB->num_filters;

    if ((MEL_FB->mel_cosine =
         (float32 **) fe_create_2d(MEL_FB->num_cepstra,
                                   MEL_FB->num_filters,
                                   sizeof(float32))) == NULL) {
        fprintf(stderr,
                "memory alloc failed in fe_compute_melcosine()\n...exiting\n");
        exit(0);
    }


    for (i = 0; i < MEL_FB->num_cepstra; i++) {
        freq = 2 * (float32) M_PI *(float32) i / period;
        for (j = 0; j < MEL_FB->num_filters; j++)
            MEL_FB->mel_cosine[i][j] =
                (float32) cos((float64) (freq * (j + 0.5)));
    }

    return (0);

}

float32
fe_mel(float32 x)
{
    float32 warped = fe_warp_unwarped_to_warped(x);

    return (float32) (2595.0 * log10(1.0 + warped / 700.0));
}

float32
fe_melinv(float32 x)
{
    float32 warped = (float32) (700.0 * (pow(10.0, x / 2595.0) - 1.0));
    return fe_warp_warped_to_unwarped(warped);
}

/* adds 1/2-bit noise */
int32
fe_dither(int16 * buffer, int32 nsamps)
{
    int32 i;
    for (i = 0; i < nsamps; i++)
        buffer[i] += (short) ((!(s3_rand_int31() % 4)) ? 1 : 0);

    return 0;
}

void
fe_pre_emphasis(int16 const *in, float64 * out, int32 len, float32
                factor, int16 prior)
{
    int32 i;

    out[0] = (float64) in[0] - factor * (float64) prior;
    for (i = 1; i < len; i++) {
        out[i] = (float64) in[i] - factor * (float64) in[i - 1];
    }

}

void
fe_short_to_double(int16 const *in, float64 * out, int32 len)
{
    int32 i;

    for (i = 0; i < len; i++)
        out[i] = (float64) in[i];
}


void
fe_create_hamming(float64 * in, int32 in_len)
{
    int i;

    if (in_len > 1) {
        for (i = 0; i < in_len; i++)
            in[i] =
                0.54 - 0.46 * cos(2 * M_PI * i / ((float64) in_len - 1.0));
    }
    return;

}


void
fe_hamming_window(float64 * in, float64 * window, int32 in_len)
{
    int i;

    if (in_len > 1) {
        for (i = 0; i < in_len; i++)
            in[i] *= window[i];
    }
    return;

}


int32
fe_frame_to_fea(fe_t * FE, float64 * in, float64 * fea)
{
    float64 *spec, *mfspec;
    int32 returnValue = FE_SUCCESS;

    if (FE->FB_TYPE == MEL_SCALE) {
        spec = (float64 *) calloc(FE->FFT_SIZE, sizeof(float64));
        mfspec =
            (float64 *) calloc(FE->MEL_FB->num_filters, sizeof(float64));

        if (spec == NULL || mfspec == NULL) {
            fprintf(stderr,
                    "memory alloc failed in fe_frame_to_fea()\n...exiting\n");
            exit(0);
        }

        fe_spec_magnitude(in, FE->FRAME_SIZE, spec, FE->FFT_SIZE);
        fe_mel_spec(FE, spec, mfspec);
        returnValue = fe_mel_cep(FE, mfspec, fea);

        free(spec);
        free(mfspec);
    }
    else {
        fprintf(stderr,
                "MEL SCALE IS CURRENTLY THE ONLY IMPLEMENTATION!\n");
        exit(0);
    }
    return returnValue;
}



void
fe_spec_magnitude(float64 const *data, int32 data_len, float64 * spec,
                  int32 fftsize)
{
    int32 j, wrap;
    float64 *fft;

    if (NULL == (fft = (float64 *) calloc(fftsize, sizeof(float64)))) {
        E_FATAL
            ("memory alloc failed in fe_spec_magnitude()\n...exiting\n");
    }
    wrap = (data_len < fftsize) ? data_len : fftsize;
    memcpy(fft, data, wrap * sizeof(float64));
    if (data_len > fftsize) {   /*aliasing */
        E_WARN
            ("Aliasing. Consider using fft size (%d) > buffer size (%d)\n",
             fftsize, data_len);
        for (wrap = 0, j = fftsize; j < data_len; wrap++, j++)
            fft[wrap] += data[j];
    }

    fe_fft_real(fft, fftsize);

    /* zero is a special case. */
    spec[0] = fft[0] * fft[0];
    for (j = 1; j <= fftsize / 2; j++)
        spec[j] = fft[j] * fft[j] + fft[fftsize - j] * fft[fftsize - j];

    free(fft);
    return;
}

void
fe_mel_spec(fe_t * FE, float64 const *spec, float64 * mfspec)
{
    int32 whichfilt, start, i;
    float32 dfreq;

    dfreq = FE->SAMPLING_RATE / (float32) FE->FFT_SIZE;

    for (whichfilt = 0; whichfilt < FE->MEL_FB->num_filters; whichfilt++) {
        /* Round to the nearest integer instead of truncating and
           adding one, which breaks if the divide is already an
           integer */
        start = (int32) (FE->MEL_FB->left_apex[whichfilt] / dfreq + 0.5);
        mfspec[whichfilt] = 0;
        for (i = 0; i < FE->MEL_FB->width[whichfilt]; i++)
            mfspec[whichfilt] +=
                FE->MEL_FB->filter_coeffs[whichfilt][i] * spec[start + i];
    }

}




int32
fe_mel_cep(fe_t * FE, float64 * mfspec, float64 * mfcep)
{
    int32 i, j;
    int32 period;
    float32 beta;
    int32 returnValue = FE_SUCCESS;

    period = FE->MEL_FB->num_filters;

    for (i = 0; i < FE->MEL_FB->num_filters; ++i) {
        if (mfspec[i] > 0)
            mfspec[i] = log(mfspec[i]);
        else {
            mfspec[i] = -1.0e+5;
            returnValue = FE_ZERO_ENERGY_ERROR;
        }
    }
    if (FE->LOG_SPEC == OFF) {
        for (i = 0; i < FE->NUM_CEPSTRA; ++i) {
            mfcep[i] = 0;
            for (j = 0; j < FE->MEL_FB->num_filters; j++) {
                if (j == 0)
                    beta = 0.5;
                else
                    beta = 1.0;
                mfcep[i] +=
                    beta * mfspec[j] * FE->MEL_FB->mel_cosine[i][j];
            }
            mfcep[i] /= (float32) period;
        }
    }
    else {
        for (i = 0; i < FE->FEATURE_DIMENSION; i++) {
            mfcep[i] = mfspec[i];
        }
    }
    return returnValue;
}

/* This function has been replaced by fe_fft_real, and is no longer used. */
int32
fe_fft(complex const *in, complex * out, int32 N, int32 invert)
{
    int32 s, k,                 /* as above                             */
     lgN;                       /* log2(N)                              */

    complex *f1, *f2,           /* pointers into from array             */
    *t1, *t2,                   /* pointers into to array               */
    *ww;                        /* pointer into w array                 */

    complex *w, *from, *to,     /* as above                             */
     wwf2,                      /* temporary for ww*f2                  */
    *buffer,                    /* from and to flipflop btw out and buffer */
    *exch,                      /* temporary for exchanging from and to */
    *wEnd;                      /* to keep ww from going off end        */

    float64 div,                /* amount to divide result by: N or 1   */
     x;                         /* misc.                                */


    /* check N, compute lgN                                               */
    for (k = N, lgN = 0; k > 1; k /= 2, lgN++) {
        if (k % 2 != 0 || N < 0) {
            fprintf(stderr, "fft: N must be a power of 2 (is %d)\n", N);
            return (-1);
        }
    }

    /* check invert, compute div                                          */
    if (invert == 1)
        div = 1.0;
    else if (invert == -1)
        div = N;
    else {
        fprintf(stderr, "fft: invert must be either +1 or -1 (is %d)\n",
                invert);
        return (-1);
    }

    /* get the to, from buffers right, and init                           */
    buffer = (complex *) calloc(N, sizeof(complex));
    if (lgN % 2 == 0) {
        from = out;
        to = buffer;
    }
    else {
        to = out;
        from = buffer;
    }


    for (s = 0; s < N; s++) {
        from[s].r = in[s].r / div;
        from[s].i = in[s].i / div;

    }

    /* w = exp(-2*PI*i/N), w[k] = w^k                                     */
    w = (complex *) calloc(N / 2, sizeof(complex));
    for (k = 0; k < N / 2; k++) {
        x = -6.28318530717958647 * invert * k / N;
        w[k].r = cos(x);
        w[k].i = sin(x);
    }
    wEnd = &w[N / 2];

    /* go for it!                                                         */
    for (k = N / 2; k > 0; k /= 2) {
        for (s = 0; s < k; s++) {
            /* initialize pointers                                            */
            f1 = &from[s];
            f2 = &from[s + k];
            t1 = &to[s];
            t2 = &to[s + N / 2];
            ww = &w[0];
            /* compute <s,k>                                                  */
            while (ww < wEnd) {
                /* wwf2 = ww*f2                                                 */
                wwf2.r = f2->r * ww->r - f2->i * ww->i;
                wwf2.i = f2->r * ww->i + f2->i * ww->r;
                /* t1 = f1+wwf2                                                 */
                t1->r = f1->r + wwf2.r;
                t1->i = f1->i + wwf2.i;
                /* t2 = f1-wwf2                                                 */
                t2->r = f1->r - wwf2.r;
                t2->i = f1->i - wwf2.i;
                /* increment                                                    */
                f1 += 2 * k;
                f2 += 2 * k;
                t1 += k;
                t2 += k;
                ww += k;
            }
        }
        exch = from;
        from = to;
        to = exch;
    }
    free(buffer);
    free(w);
    return (0);
}

/* Translated from the FORTRAN (obviously) from "Real-Valued Fast
 * Fourier Transform Algorithms" by Henrik V. Sorensen et al., IEEE
 * Transactions on Acoustics, Speech, and Signal Processing, vol. 35,
 * no.6.  Optimized to use a static array of sine/cosines.
 */
int32
fe_fft_real(float64 * x, int n)
{
    int32 i, j, k, n1, n2, n4, i1, i2, i3, i4;
    float64 t1, t2, xt, cc, ss;
    static float64 *ccc = NULL, *sss = NULL;
    static int32 lastn = 0;
    int m;

    /* check fft size, compute fft order (log_2(n)) */
    for (k = n, m = 0; k > 1; k >>= 1, m++) {
        if (((k % 2) != 0) || (n <= 0)) {
            E_FATAL("fft: number of points must be a power of 2 (is %d)\n",
                    n);
        }
    }
    if (ccc == NULL || n != lastn) {
        if (ccc != NULL) {
            free(ccc);
        }
        if (sss != NULL) {
            free(sss);
        }
        ccc = calloc(n / 4, sizeof(*ccc));
        sss = calloc(n / 4, sizeof(*sss));
        for (i = 0; i < n / 4; ++i) {
            float64 a;

            a = 2 * M_PI * i / n;

            ccc[i] = cos(a);
            sss[i] = sin(a);
        }
        lastn = n;
    }

    j = 0;
    n1 = n - 1;
    for (i = 0; i < n1; ++i) {
        if (i < j) {
            xt = x[j];
            x[j] = x[i];
            x[i] = xt;
        }
        k = n / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    for (i = 0; i < n; i += 2) {
        xt = x[i];
        x[i] = xt + x[i + 1];
        x[i + 1] = xt - x[i + 1];
    }
    n2 = 0;
    for (k = 1; k < m; ++k) {
        n4 = n2;
        n2 = n4 + 1;
        n1 = n2 + 1;
        for (i = 0; i < n; i += (1 << n1)) {
            xt = x[i];
            x[i] = xt + x[i + (1 << n2)];
            x[i + (1 << n2)] = xt - x[i + (1 << n2)];
            x[i + (1 << n4) + (1 << n2)] = -x[i + (1 << n4) + (1 << n2)];
            for (j = 1; j < (1 << n4); ++j) {
                i1 = i + j;
                i2 = i - j + (1 << n2);
                i3 = i + j + (1 << n2);
                i4 = i - j + (1 << n1);

                /* a = 2*M_PI * j / n1; */
                /* cc = cos(a); ss = sin(a); */
                cc = ccc[j << (m - n1)];
                ss = sss[j << (m - n1)];
                t1 = x[i3] * cc + x[i4] * ss;
                t2 = x[i3] * ss - x[i4] * cc;
                x[i4] = x[i2] - t2;
                x[i3] = -x[i2] - t2;
                x[i2] = x[i1] - t1;
                x[i1] = x[i1] + t1;
            }
        }
    }
    return 0;
}


char **
fe_create_2d(int32 d1, int32 d2, int32 elem_size)
{
    char *store;
    char **out;
    int32 i, j;
    store = calloc(d1 * d2, elem_size);

    if (store == NULL) {
        fprintf(stderr, "fe_create_2d failed\n");
        return (NULL);
    }

    out = calloc(d1, sizeof(void *));

    if (out == NULL) {
        fprintf(stderr, "fe_create_2d failed\n");
        free(store);
        return (NULL);
    }

    for (i = 0, j = 0; i < d1; i++, j += d2) {
        out[i] = &((char *) store)[j * elem_size];
    }

    return out;
}

void
fe_free_2d(void **arr)
{
    if (arr != NULL) {
        free(arr[0]);
        free(arr);
    }

}

void
fe_parse_general_params(param_t const *P, fe_t * FE)
{
    if (P->SAMPLING_RATE != 0)
        FE->SAMPLING_RATE = P->SAMPLING_RATE;
    else
        FE->SAMPLING_RATE = (float32) atof(DEFAULT_SAMPLING_RATE);

    if (P->FRAME_RATE != 0)
        FE->FRAME_RATE = P->FRAME_RATE;
    else
        FE->FRAME_RATE = (int32) atoi(DEFAULT_FRAME_RATE);

    if (P->WINDOW_LENGTH != 0)
        FE->WINDOW_LENGTH = P->WINDOW_LENGTH;
    else
        FE->WINDOW_LENGTH = (float32) atof(DEFAULT_WINDOW_LENGTH);

    if (P->FB_TYPE != 0)
        FE->FB_TYPE = P->FB_TYPE;
    else
        FE->FB_TYPE = DEFAULT_FB_TYPE;

    FE->dither = P->dither;
    FE->seed = P->seed;

    if (P->PRE_EMPHASIS_ALPHA != 0)
        FE->PRE_EMPHASIS_ALPHA = P->PRE_EMPHASIS_ALPHA;
    else
        FE->PRE_EMPHASIS_ALPHA =
            (float32) atof(DEFAULT_PRE_EMPHASIS_ALPHA);

    if (P->NUM_CEPSTRA != 0)
        FE->NUM_CEPSTRA = P->NUM_CEPSTRA;
    else
        FE->NUM_CEPSTRA = atoi(DEFAULT_NUM_CEPSTRA);

    if (P->FFT_SIZE != 0)
        FE->FFT_SIZE = P->FFT_SIZE;
    else
        FE->FFT_SIZE = atoi(DEFAULT_FFT_SIZE);

    FE->LOG_SPEC = P->logspec;
    if (FE->LOG_SPEC == OFF)
        FE->FEATURE_DIMENSION = FE->NUM_CEPSTRA;
    else {
        if (P->NUM_FILTERS != 0)
            FE->FEATURE_DIMENSION = P->NUM_FILTERS;
        else {
            if (FE->SAMPLING_RATE == BB_SAMPLING_RATE)
                FE->FEATURE_DIMENSION = DEFAULT_BB_NUM_FILTERS;
            else if (FE->SAMPLING_RATE == NB_SAMPLING_RATE)
                FE->FEATURE_DIMENSION = DEFAULT_NB_NUM_FILTERS;
            else {
                E_FATAL
                    ("Please define the number of MEL filters needed\n");
            }
        }
    }
}

void
fe_parse_melfb_params(param_t const *P, melfb_t * MEL)
{
    if (P->SAMPLING_RATE != 0)
        MEL->sampling_rate = P->SAMPLING_RATE;
    else
        MEL->sampling_rate = (float32) atof(DEFAULT_SAMPLING_RATE);

    if (P->FFT_SIZE != 0)
        MEL->fft_size = P->FFT_SIZE;
    else {
        if (MEL->sampling_rate == BB_SAMPLING_RATE)
            MEL->fft_size = DEFAULT_BB_FFT_SIZE;
        if (MEL->sampling_rate == NB_SAMPLING_RATE)
            MEL->fft_size = DEFAULT_NB_FFT_SIZE;
        else
            MEL->fft_size = atoi(DEFAULT_FFT_SIZE);
    }

    if (P->NUM_CEPSTRA != 0)
        MEL->num_cepstra = P->NUM_CEPSTRA;
    else
        MEL->num_cepstra = atoi(DEFAULT_NUM_CEPSTRA);

    if (P->NUM_FILTERS != 0)
        MEL->num_filters = P->NUM_FILTERS;
    else {
        if (MEL->sampling_rate == BB_SAMPLING_RATE)
            MEL->num_filters = DEFAULT_BB_NUM_FILTERS;
        else if (MEL->sampling_rate == NB_SAMPLING_RATE)
            MEL->num_filters = DEFAULT_NB_NUM_FILTERS;
        else {
            E_FATAL
                ("Default value not defined for this sampling rate\nPlease explicitly set -nfilt\n");
        }
    }

    if (P->UPPER_FILT_FREQ != 0)
        MEL->upper_filt_freq = P->UPPER_FILT_FREQ;
    else {
        if (MEL->sampling_rate == BB_SAMPLING_RATE)
            MEL->upper_filt_freq = (float32) DEFAULT_BB_UPPER_FILT_FREQ;
        else if (MEL->sampling_rate == NB_SAMPLING_RATE)
            MEL->upper_filt_freq = (float32) DEFAULT_NB_UPPER_FILT_FREQ;
        else {
            E_FATAL
                ("Default value not defined for this sampling rate\nPlease explicitly set -upperf\n");
        }
    }

    if (P->LOWER_FILT_FREQ != 0)
        MEL->lower_filt_freq = P->LOWER_FILT_FREQ;
    else {
        if (MEL->sampling_rate == BB_SAMPLING_RATE)
            MEL->lower_filt_freq = (float32) DEFAULT_BB_LOWER_FILT_FREQ;
        else if (MEL->sampling_rate == NB_SAMPLING_RATE)
            MEL->lower_filt_freq = (float32) DEFAULT_NB_LOWER_FILT_FREQ;
        else {
            E_FATAL
                ("Default value not defined for this sampling rate\nPlease explictly set -lowerf\n");
        }
    }

    if (P->doublebw == ON)
        MEL->doublewide = ON;
    else
        MEL->doublewide = OFF;

    if (P->warp_type == NULL) {
        MEL->warp_type = DEFAULT_WARP_TYPE;
    }
    else {
        MEL->warp_type = P->warp_type;
    }
    MEL->warp_params = P->warp_params;

    if (fe_warp_set(MEL->warp_type) != FE_SUCCESS) {
        E_FATAL("Failed to initialize the warping function.\n");
    }
    fe_warp_set_parameters(MEL->warp_params, MEL->sampling_rate);
}

void
fe_print_current(fe_t const *FE)
{
    E_INFO("Current FE Parameters:\n");
    E_INFO("\tSampling Rate:             %f\n", FE->SAMPLING_RATE);
    E_INFO("\tFrame Size:                %d\n", FE->FRAME_SIZE);
    E_INFO("\tFrame Shift:               %d\n", FE->FRAME_SHIFT);
    E_INFO("\tFFT Size:                  %d\n", FE->FFT_SIZE);
    E_INFO("\tLower Frequency:           %g\n",
           FE->MEL_FB->lower_filt_freq);
    E_INFO("\tUpper Frequency:           %g\n",
           FE->MEL_FB->upper_filt_freq);
    E_INFO("\tNumber of filters:         %d\n", FE->MEL_FB->num_filters);
    E_INFO("\tNumber of Overflow Samps:  %d\n", FE->NUM_OVERFLOW_SAMPS);
    E_INFO("\tStart Utt Status:          %d\n", FE->START_FLAG);
    if (FE->dither) {
        E_INFO("Will add dither to audio\n");
        E_INFO("Dither seeded with %d\n", FE->seed);
    }
    else {
        E_INFO("Will not add dither to audio\n");
    }
    if (FE->MEL_FB->doublewide == ON) {
        E_INFO("Will use double bandwidth in mel filter\n");
    }
    else {
        E_INFO("Will not use double bandwidth in mel filter\n");
    }
}
