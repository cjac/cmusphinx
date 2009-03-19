/* -*- c-basic-offset: 4 -*- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SQ(X)          ((X) * (X))
#define ABS(X)         (((X) > 0) ?        (X) : -(X))
#define MIN2(X, Y)     (((X) < (Y))?       (X) : (Y))
#define MIN3(X, Y, Z)  ((MIN2(X,Y) < (Z))? MIN2(X,Y) : (Z))

extern double const gammatone_1024FFT_40Ch_130L_6800H[40][512];

typedef struct COMPLEX_s {
    double r;
    double i;
} COMPLEX;

typedef struct phase_itd_s phase_itd_t;
struct phase_itd_s {
    short *psBufferL;
    short *psBufferR;
    double *pdBufferL;
    double *pdBufferR;
    double *pdOutBuffer;
    double *pdPrevOutBuffer;
    short*  psOutBuffer;
    short* asPrevOutBuffer;
    double* pdHamming;
    COMPLEX *w, *buffer;
    COMPLEX* pdIn_L;
    COMPLEX* pdIn_R;

    COMPLEX* pdOut_L;
    COMPLEX* pdOut_R;
    COMPLEX* pdOut;
    COMPLEX* pdOutTime;
	
    double*  pdSpec;

    double*  pd_angleL;
    double*  pd_angleR;
    double*  pd_phaseDiff;
    double*  pd_switch;
    double*  pd_tempSwitch;
    double*  pd_Gswitch;
    double*  pd_specReshape;
    double*  pd_gammaDistortion;
};

static const double  dSampRate      = 16000.0;
static const double  dFrameLen      = 0.05;
static const int     iFFTSize       = 1024;
static const int     iNumFilts      = 40;
static const double  dMaskingWeight = 0.001;
static const double  dTh            = 0.3;

int32
complex_fft(COMPLEX const *w, COMPLEX *buffer,
	    COMPLEX const *in, COMPLEX *out,
	    int32 N, int32 invert)
{
    COMPLEX
	*from, *to,                 /* as above                             */
	wwf2,                       /* temporary for ww*f2                  */
	*buffer,                    /* from and to flipflop btw out and buffer */
	*exch;                      /* temporary for exchanging from and to */

    float64
	div,                        /* amount to divide result by: N or 1   */
	x;                          /* misc.                                */

    int32
	s, k,                       /* as above                             */
	lgN;                        /* log2(N)                              */

    COMPLEX
	*f1, *f2,                   /* pointers into from array             */
	*t1, *t2;                   /* pointers into to array               */

    COMPLEX const
	*ww,                        /* pointer into w array                 */
	*wEnd;                      /* to keep ww from going off end        */

    /* check invert, compute div                                          */
    if (invert == 1)
	div = 1.0;
    else if (invert == -1)
	div = N;
    else
	E_FATAL("invert must be 1 or -1\n");

    /* get the to, from buffers right, and init                           */
    if (lgN%2 == 0)
    {
	from = out;
	to = buffer;
    }
    else
    {
	to = out;
	from = buffer;
    }

    for (s = 0; s<N; s++)
    {
	from[s].r = in[s].r/div;
	from[s].i = in[s].i/div;
    }

    wEnd = &w[N/2];

    /* go for it!                                                         */
    for (k = N/2; k > 0; k /= 2)
    {
	for (s = 0; s < k; s++)
	{
	    /* initialize pointers                                            */
	    f1 = &from[s]; f2 = &from[s+k];
	    t1 = &to[s]; t2 = &to[s+N/2];
	    ww = &w[0];
	    /* compute <s,k>                                                  */
	    while (ww < wEnd)
	    {
		/* wwf2 = ww*f2                                                 */
		wwf2.r = f2->r*ww->r - f2->i*ww->i*invert;
		wwf2.i = f2->r*ww->i*invert + f2->i*ww->r;
		/* t1 = f1+wwf2                                                 */
		t1->r = f1->r + wwf2.r;
		t1->i = f1->i + wwf2.i;
		/* t2 = f1-wwf2                                                 */
		t2->r = f1->r - wwf2.r;
		t2->i = f1->i - wwf2.i;
		/* increment                                                    */
		f1 += 2*k; f2 += 2*k;
		t1 += k; t2 += k;
		ww += k;
	    }
	}
	exch = from; from = to; to = exch;
    }

    return 0;
}

phase_itd_t *
phase_itd_init(double dSampRate, double dFrameLen)
{
    phase_itd_t *self;
    int iFL = (int)(dSampRate * dFrameLen + 0.5);
    int iFP = (int)((dSampRate * dFrameLen + 0.5) / 2);

    self->psBufferL = ckd_calloc(iFL, sizeof(short));
    self->psBufferR = ckd_calloc(iFL, sizeof(short));
    self->pdBufferL = ckd_calloc(iFL, sizeof(double));
    self->pdBufferR = ckd_calloc(iFL, sizeof(double));
    self->pdOutBuffer = ckd_calloc(iFL, sizeof(double));
    self->pdPrevOutBuffer = ckd_calloc(iFL, sizeof(double));
    self->psOutBuffer = ckd_calloc(iFL, sizeof(short));
    self->pdHamming = ckd_calloc(iFL, sizeof(double));

    /* Create Hamming window. */
    for (i = 0; i < iFL; i++)
	self->pdHamming[i] = 0.54 - 0.46 * cos(2 * _M_PI * i / (iFL - 1));

    /* Create FFT twiddle factors. */
    /* w = exp(-2*PI*i/N), w[k] = w^k                                     */
    self->w = ckd_calloc(nfft/2, sizeof(COMPLEX));
    for (k = 0; k < nfft/2; k++)
    {
	double x = -6.28318530717958647*k/nfft;
	self->w[k].r = cos(x);
	self->w[k].i = sin(x);
    }
    self->buffer = ckd_calloc(nfft, sizeof(COMPLEX));

    double   dNom;
    int      iFI = 0;

    pdIn_L   = (COMPLEX*) malloc (sizeof(COMPLEX) * iFFTSize);
    pdIn_R   = (COMPLEX*) malloc (sizeof(COMPLEX) * iFFTSize);

    pdOut_L  = (COMPLEX*) malloc (sizeof(COMPLEX) * iFFTSize);
    pdOut_R  = (COMPLEX*) malloc (sizeof(COMPLEX) * iFFTSize);
    pdOut    = (COMPLEX*) malloc (sizeof(COMPLEX) * iFFTSize);
    pdOutTime = (COMPLEX*) malloc (sizeof(COMPLEX) * iFFTSize);

    pd_angleL    = (double*) malloc(sizeof(double) * iFFTSize);
    pd_angleR    = (double*) malloc(sizeof(double) * iFFTSize);
    pd_phaseDiff = (double*) malloc(sizeof(double) * iFFTSize);
    pd_switch    = (double*) malloc(sizeof(double) * iFFTSize);
    pd_tempSwitch    = (double*) malloc(sizeof(double) * iFFTSize);
    pd_Gswitch   = (double*) malloc(sizeof(double) * iNumFilts);
    pd_gammaDistortion = (double*) malloc(sizeof(double) * iFFTSize);

    pd_specReshape = (double*) malloc(sizeof(double) * iFFTSize);
    pdSpec   = (double*) malloc(sizeof(double) * iFFTSize);
	
    //////////////////////////////////////////////////////////////////////////////
    //
    // Gamma distortion calcuation
    //
    memset(pd_gammaDistortion, 0, sizeof(double) * iFFTSize);
    for (i = 0; i < iFFTSize / 2; i++)
    {
	pd_gammaDistortion[i] = 0;

	for (j = 0; j < iNumFilts; j++)
	{
	    pd_gammaDistortion[i] += SQ(m_aad_H_1024FFT_40Ch_130L_6800H[j][i]);
	}

	pd_gammaDistortion[i] = MIN2(1 / pd_gammaDistortion[i], 5.0);
    }

    return self;
}

void
phase_itd_free(phase_itd_t *self)
{
    ckd_free(self->w);
    ckd_free(self->buffer);
    ckd_free(self->pdBufferL);
    ckd_free(self->pdBufferR);
    ckd_free(self->psBufferL);
    ckd_free(self->psBufferR);
    ckd_free(self->pdOutBuffer);
    ckd_free(self->pdPrevOutBuffer);
    ckd_free(self->psOutBuffer);
    ckd_free(self->pdHamming);
    ckd_free(self);
}

void
phase_itd_run(short *stereo_inbuf, int nsamp, short *mono_outbuf)
{
    // short to double conversion
    for (i = 0; i < iFL; i++)
    {
	pdBufferL[i] = ((double)psBufferL[i] * pdHamming[i]);
	pdBufferR[i] = ((double)psBufferR[i] * pdHamming[i]);
    }

    // Pack complex FFTs
    for (i = 0; i < iFL; i++)
    {
	pdIn_L[i].r = pdBufferL[i];
	pdIn_L[i].i = 0.0;

	pdOut_L[i].r = 0.0;
	pdOut_L[i].i = 0.0;

	pdIn_R[i].r = pdBufferR[i];
	pdIn_R[i].i = 0.0;

	pdOut_R[i].r = 0.0;
	pdOut_R[i].i = 0.0;
    }

    for (i = iFL; i < iFFTSize; i++)
    {
	pdIn_L[i].r = 0.0;
	pdIn_L[i].i = 0.0;

	pdOut_L[i].r = 0.0;
	pdOut_L[i].i = 0.0;

	pdIn_R[i].r = 0.0;
	pdIn_R[i].i = 0.0;

	pdOut_R[i].r = 0.0;
	pdOut_R[i].i = 0.0;
    }

    // Do FFTs
    fe_fft(pdIn_L, pdOut_L, iFFTSize, 1);
    fe_fft(pdIn_R, pdOut_R, iFFTSize, 1);

    for (i = 0; i < iFFTSize; i++)
    {
	pdOut[i].r = (pdOut_L[i].r + pdOut_L[i].r) / 2;
	pdOut[i].i = (pdOut_L[i].i + pdOut_L[i].i) / 2;

	pdSpec[i]   = SQ(pdOut[i].r) + SQ(pdOut[i].i);

	pd_angleL[i] = atan(pdOut_L[i].i / pdOut_L[i].r);
	pd_angleR[i] = atan(pdOut_R[i].i / pdOut_R[i].r);

	double dtemp2 = ABS(pd_angleR[i] - pd_angleL[i]);

	double temp = MIN2(ABS(pd_angleR[i] - pd_angleL[i]), ABS(pd_angleR[i] - pd_angleL[i] - 2 * _M_PI));

	pd_phaseDiff[i] = MIN3(ABS(pd_angleR[i] - pd_angleL[i]), 
			       ABS(pd_angleR[i] - pd_angleL[i] - 2 * _M_PI), 
			       ABS(pd_angleR[i] - pd_angleL[i] + 2 * _M_PI));
    }

    pd_phaseDiff[0] =   0;
    for (i = 1; i < iFFTSize / 2; i++)
    {
	pd_phaseDiff[i] /= (2 * _M_PI * i / (iFFTSize));
    }

    memset(pd_switch, 0, sizeof(double) * iFFTSize);

    for (i = 0; i < iFFTSize / 2; i++)
    {
	if (pd_phaseDiff[i] < dTh)
	{
	    pd_switch[i]     = 1.0;
	    pd_tempSwitch[i] = 1.0;

	}
	else
	{
	    pd_switch[i] = dMaskingWeight;
	    pd_tempSwitch[i] = dMaskingWeight;
	}

    }

    for (i = 2; i < iFFTSize / 2; i++)
    {
	if (   pd_tempSwitch[i - 2] == 1 && pd_tempSwitch[i - 1] == 1 && pd_tempSwitch[i] == 1
	       && pd_tempSwitch[i+ 1] == 1 && pd_tempSwitch[i+ 2] == 1)
	{
	    pd_switch[i - 2]  = 1;
	    pd_switch[i - 1]  = 1;
	    pd_switch[i]      = 1;
	    pd_switch[i + 1]  = 1;
	    pd_switch[i + 2]  = 1;
	}
	else
	{
	    pd_switch[i]      = dMaskingWeight;
	}
    }

    // Gamma switch weighting
    for (i = 0; i < iNumFilts; i++)
    {
	pd_Gswitch[i] = 0;
	dNom          = 0;

	for (j = 0; j < iFFTSize / 2; j++)
	{
	    pd_Gswitch[i] += pd_switch[j] * pdSpec[j] *  SQ(m_aad_H_1024FFT_40Ch_130L_6800H[i][j]);
	    dNom          += pdSpec[j] *  SQ(m_aad_H_1024FFT_40Ch_130L_6800H[i][j]);
	}

	pd_Gswitch[i] /= dNom;
    }


    // Spectral Reshaping
    memset(pd_specReshape, 0, sizeof(double) * iFFTSize);

    for (i = 0; i < iNumFilts; i++)
    {
	for (j = 0; j < iFFTSize / 2; j++)
	{
	    pd_specReshape[j] += pd_Gswitch[i] * SQ(m_aad_H_1024FFT_40Ch_130L_6800H[i][j]);
	}
    }

    for (i = 0; i < iFFTSize / 2; i++)
    {
	pdOut[i].r *= (pd_specReshape[i] * pd_gammaDistortion[i]);
	pdOut[i].i *= (pd_specReshape[i] * pd_gammaDistortion[i]);

	pdOut[iFFTSize - 1 - i].r *= (pd_specReshape[i] * pd_gammaDistortion[i]);
	pdOut[iFFTSize - 1 - i].i *= (pd_specReshape[i] * pd_gammaDistortion[i]); 
    }

    // Inverse fft
    memset(pdOutTime, 0, sizeof(COMPLEX) * iFFTSize);
    fe_fft(pdOut, pdOutTime, iFFTSize, -1);
    memset(pdOutBuffer, 0, sizeof(double) * iFL);

    for (i = 0; i < iFL; i++)
    {
	pdOutBuffer[i] = pdOutTime[i].r;
    }
		
    for (i = 0; i < iFL / 2; i++)
    {
	psOutBuffer[i] = (short)(pdOutBuffer[i] + pdPrevOutBuffer[i + iFL / 2]) / (1.08); // 1 + 0.08
    }

    for (i = 0; i < iFL; i++)
    {
	pdPrevOutBuffer[i] = pdOutBuffer[i];
    }

    return 0;
}
