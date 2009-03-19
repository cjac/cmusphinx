/* -*- c-basic-offset: 4 -*- */

#include "online_vadmvn.h"
//
// Algorithm design: by Chanwoo Kim
// 
// Jan 18, 2009
//

#define DEFAULT_LAMDA 0.994
#define DEFAULT_FEAT_VEC_LEN 13;

#define SQ(X) ((X) * (X))

static const int iLimit = 1000.0;

// Just from one utterance
static const double DEFAULT_MVN_STAT[] = {
    -10.734615,
    -2.795471,
    -0.154431,
    0.179448,
    0.854412,
    0.036483,
    0.279851,
    -0.134282,
    0.252706,
    0.103600,
    -0.035344,
    0.145824,
    0.342790,
    25.216553,
    0.018874,
    0.173904,
    -0.477790,
    -0.887197,
    -0.167616,
    -0.390107,
    0.150655,
    -0.272305,
    -0.036676,
    -0.088977,
    -0.208385,
    -0.252665,
    -5.607917,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    3.755589,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    0.000000,
    -0.340038,
    -0.178657,
    0.362116,
    0.059204,
    -0.025431,
    -0.020536,
    0.129906,
    -0.064746,
    0.046427,
    0.117093,
    0.020796,
    0.010235,
    0.058674,
    30.730174,
    9.953625,
    5.973311,
    2.027958,
    3.682064,
    1.409649,
    0.538161,
    0.470809,
    0.283592,
    0.414406,
    0.392151,
    0.214588,
    0.313877,
};

///////////////////////////////////////////////////////////////////////
//
// These values are obtained from 1,600 utterances in the RM using original version of PCC program and sphinx_fe in sphinxbase 0.4
// 
// Not statistical.. data.. just from one training utterance. will obtain statistical data later on... no time to do this right away
//
static const double ms_adStdPCC[] = {       
    9.55497484487207,
    4.29651828357334,
    2.01502614482376,
    1.92384781604820,
    1.37754659937030,
    1.28274437344835,
    1.66303527436052,
    1.45362241844379,
    0.87605674913279,
    0.73495480282991,
    0.71644292457834,
    0.80861050650092,
    0.59224043546249
   
};

static const double ms_adStdMFCC[] = {
   1.00000000000000,
   0.35280392502267,
   0.15330789654680,
   0.14482829089055,
   0.11447612567787,
   0.08435111033731,
   0.08228406277289,
   0.07750260236165,
   0.06323215482779,
   0.06470358834759,
   0.05357954353969,
   0.05103171947216,
   0.05020093811808
};

online_vadmvn_t *
online_vadmvn_init(double dLamda,
		   int iFeatVectLen,
		   int iBufferSize)
{
    online_vadmvn_t *self = ckd_calloc(1, sizeof(*self));

    self->dLamda = dLamda;
    self->dLongLamda    = 0.999;
    self->dShortLamda   = 0.9;
    self->pdMeanSubFeat = NULL;
    self->dOnlineScale  = 10 / self->dOnlineC0Peak;
    self->iSilToSpeech = 0;
    self->iSpeechToSil = 0;
    self->VadState = SIL;
    self->iSilSkip  = 1; 
    self->iCountSil = 0;
    self->iFeatVectLen  = iFeatVectLen;
    self->iBufferSize   = iBufferSize;
    self->ppfCepBuffer = ckd_calloc_2d(iBufferSize, self->iFeatVectLen,
					   sizeof(*self->ppfCepBuffer));

#define ALLOC(member, count)				\
    member = ckd_calloc((count), sizeof(*member))
    ALLOC(self->pfSpec, self->iFeatVectLen);
    ALLOC(self->pInFeat, self->iFeatVectLen);
    ALLOC(self->pdOnlineMean, iFeatVectLen);
    ALLOC(self->pdOnlineShortMean, iFeatVectLen);
    ALLOC(self->pdOnlinePlusMean, iFeatVectLen);
    ALLOC(self->pdOnlineMinusMean, iFeatVectLen);
    ALLOC(self->pdShortVar, iFeatVectLen);
    ALLOC(self->pdOnlineLongMean, iFeatVectLen);
    ALLOC(self->pdOnlineVar, iFeatVectLen);

    return self;
}

void
online_vadmvn_free(online_vadmvn_t *self)
{
    if (self == NULL)
	return;

    ckd_free(self->pInFeat);
    ckd_free(self->pdOnlineMean);
    ckd_free(self->pdOnlineLongMean);
    ckd_free(self->pdOnlineShortMean);
    ckd_free(self->pdOnlineVar);
    ckd_free(self->pfSpec);
    ckd_free(self->pdShortVar);
    ckd_free(self->pdOnlineMinusMean);
    ckd_free(self->pdOnlinePlusMean);
    ckd_free_2d(self->ppfCepBuffer);
}

// Programmed by Chanwoo Kim
int32 online_vadmvn_run(online_vadmvn_t *self, float32* inout_feat)
{
    int i, j, nfr;

    memcpy(self->pInFeat, inout_feat, self->iFeatVectLen * sizeof(*self->pInFeat));

    // If this the first frame..
    if (self->iFrameCount == 0)
    {
	printf("First frame is being processed.\n");
	// Just start with the default values
	for (i = 0; i < self->iFeatVectLen; i++)
	{
	    self->pdOnlineMean[i]        = self->pInFeat[i];//DEFAULT_MEAN[i];
	    self->pdOnlineVar[i]         = SQ(ms_adStdPCC[i]);
	}		
	printf("%f %f %f\n", self->pdOnlineMean[0], self->pdOnlineShortMean[0], self->pdOnlineLongMean[0]);
    }

    // Look-ahead buffer
    // Storing the values in the buffer...
    if (self->iFrameCount < self->iBufferSize)
    {
	// Let's just put in the buffer
	memcpy(self->ppfCepBuffer[self->iFrameCount], self->pInFeat,
	       sizeof(float32) * self->iFeatVectLen);
    }
    else
    {
	// Let's update the buffer..
	for (i = 0; i < self->iBufferSize - 1; i++)
	{
	    memcpy(self->ppfCepBuffer[i], self->ppfCepBuffer[i + 1],
		   sizeof(float32) * self->iFeatVectLen);
	}

	memcpy(self->ppfCepBuffer[self->iBufferSize - 1], self->pInFeat,
	       sizeof(float32) * self->iFeatVectLen);
    }

    // If we do not use the previous stat information
    if (self->iFrameCount == self->iBufferSize)
    {
	// Online mean is just mean of the previous data.
	for (i = 0; i < self->iFeatVectLen; i++)
	{
	    self->pdOnlineMinusMean[i] = 0;
	    for (j = 0; j < self->iBufferSize; j++)
	    { 
				
		self->pdOnlineMean[i] += self->ppfCepBuffer[j][i];
	    }
	    self->pdOnlineMean[i]     /= self->iBufferSize;
	    for (j = 0; j < self->iBufferSize; j++)
	    {
		if (self->ppfCepBuffer[j][i] < self->pdOnlineMean[i])
		{
		    self->pdOnlineMinusMean[i] += self->ppfCepBuffer[j][i];
		}
	    }
	    self->pdOnlineMinusMean[i] =  0; //self->pdOnlineMinusMean[i] / (self->iBufferSize) - self->pdOnlineMean[i];
	    self->pdOnlineShortMean[i] = 0; ///2 * self->pdOnlineMinusMean[i];
	}
	// Let's initialize...the minus mean and the 
    }
    if (self->iFrameCount >= self->iBufferSize)
    {
	//	printf("%f %f, %f\n " , self->pdOnlineMean[0], self->pdOnlineShortMean[0], self->pdOnlineShortMean[0]); //,  self->pdOnlineShortMean[i], self->pdOnlineLongMean[i]);
	//  printf("%f %f %f\n", self->pdOnlineMean[i],  self->pdOnlineShortMean[i], self->pdOnlineLongMean[i]);
	// First let's do mean subtraction..
	for (i = 0; i < self->iFeatVectLen; i++)
	{
	    self->pdOnlineMean[i] = self->dLongLamda * self->pdOnlineMean[i] + (1 - self->dLongLamda) * self->pInFeat[i];
	    inout_feat[i]       = self->pInFeat[i] - self->pdOnlineMean[i];
	}
	// Needs to be modified later
	if (self->iFrameCount == 0)
	{
		
	    //	self->pdOnlineMinusMean[0]   = inout_feat[0];
	    self->pdOnlinePlusMean[0]    = inout_feat[0];
	    self->pdOnlineShortMean[0]   = inout_feat[0];//DEFAULT_MEAN[i];
	    self->pdOnlineLongMean[0]    = inout_feat[0];//DEFAULT_MEAN[i];

	    printf("%f %f\n", self->pdOnlineLongMean[0], self->pdOnlineShortMean[0]);
	}

	// Long and short means are calculated
	for (i = 0; i < self->iFeatVectLen; i++)
	{
	    self->pdOnlineShortMean[i] = self->dShortLamda* self->pdOnlineShortMean[i] + (1 - self->dShortLamda) *  inout_feat[i];
	    self->pdOnlineLongMean[i]  = self->dLongLamda * self->pdOnlineLongMean[i]  + (1 - self->dLongLamda)  *  inout_feat[i];
	}

	// Let's update the V/Silecne information
	//
	// +++ VAD ++++
	//
	// Due to the characteristic of the exponential window, we need to do subtraction by 10
	// Determined to be silence
	// Plus and minus means are updated in this way:
	if (self->pdOnlineShortMean[0] < 0)
	{
	    self->pdOnlineMinusMean[0] = self->dLongLamda * self->pdOnlineMinusMean[0] 
		+ (1 - self->dLongLamda) * inout_feat[0];
	}
	else
	{
	    self->pdOnlinePlusMean[0] = self->dLongLamda * self->pdOnlinePlusMean[0] 
		+ (1 - self->dLongLamda) * inout_feat[0];
	}

	if (self->pdOnlineMinusMean[0] > -0.5)
	{
	    self->VadState = SIL;
	    self->iSilToSpeech = 0;
	    self->iSpeechToSil = 0;
	}
	else
	{
	    if (inout_feat[0] < self->pdOnlineMinusMean[0])
	    {
		// Very simple state machine
		if (self->VadState == SIL)
		{
		    self->iSilToSpeech = 0;
		    self->iSpeechToSil = 0;
		}
		else if (self->VadState == SPEECH)
		{
		    self->iSilToSpeech = 0;
		    self->iSpeechToSil++;

		    if (self->iSpeechToSil == 5)
		    {
			self->VadState = SIL;
			self->iSilToSpeech = 0;
			self->iSpeechToSil = 0;
		    }
		}
	    }
	    else 
	    {
		// Very simple state machine
		if (self->VadState == SIL)
		{
		    self->iSilToSpeech++;
		    self->iSpeechToSil = 0;

		    if (self->iSilToSpeech == 5)
		    {
			self->VadState = SPEECH;
			self->iSilToSpeech = 0;
			self->iSpeechToSil = 0;
		    }
		}
		else if (self->VadState == SPEECH)
		{
		    self->iSilToSpeech = 0;
		    self->iSpeechToSil = 0;
		}
	    }
	}
	if (self->VadState == SPEECH)
	{
	    self->iSilSkip  = 0; 
	    self->iCountSil = 0;
	}
	else
	{
	    self->iCountSil++;

	    if (self->iCountSil >= 20)
	    {
		self->iSilSkip = 1;
		self->iCountSil = 0;
	    }
	}
	for (i = 0; i < self->iFeatVectLen; i++)
	{
	    if (self->VadState == SPEECH)
	    {
		//	self->pdShortVar[i]      = (1 - self->dLongLamda) * self->pdOnlineVar[i] + (1 - self->dLongLamda) * SQ(inout_feat[i]);
		self->pdOnlineVar[i]     = (self->dLamda) * self->pdOnlineVar[i] + (1 - self->dLamda) * SQ(inout_feat[i]);
	    }

	    if (self->iFrameCount >= self->iBufferSize)
	    {
		/* FIXME: Chanwoo's error here, should be adStdPCC? */
		inout_feat[i] =  (float32)( (self->ppfCepBuffer[0][i] - self->pdOnlineMean[i]) / sqrt(self->pdOnlineVar[i]) * ms_adStdMFCC[i] * 2 );
	    }
			
	} // for
	nfr = 1;
    }
    else
    {
	nfr = 0;
    }

    if (self->iSilSkip == 1)
    {
	nfr = 0;
    }

    self->iFrameCount++;
    return nfr;
}
