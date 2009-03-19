#ifndef __ONLINE_VADMVN_H__
#define __ONLINE_VADMVN_H__
/*
 * Online MPN Algorithm
 *
 * Algorithm is designed by Chanwoo Kim
 *
 * Programmed by Chanwoo Kim
 *
 * Jan 18
 *
 * Converted from the C++ and adapted to SphinxBase by David Huggins-Daines
 */

#include "libutil.h"

typedef struct online_vadmvn_s online_vadmvn_t;
struct online_vadmvn_s {
	double  dLamda;      // Normal time constant
	double  dShortLamda; // Forgetting factor with a short time constant
	double  dLongLamda;  // Forgetting factor with a large time constant
	
	int iPrevUttStat; // Option about whether to use the previous utt information or not.

	int     iFeatVectLen;
	int     iPeakNorm; // Select whether to use MVN or MVPN
	int     iLookAheadSize;

	double dTruePeak;
	int    iBufferSize;
	double dRatioTh;

	// State for VAD
	enum VAD_STATE
	{
		SIL,
		SPEECH
	} VadState;
	
	int iSilToSpeech;
	int iSpeechToSil;

	// These are actually used for.. silence skipping
	int iSilSkip; 
	int iCountSil;
	
	double* pdOnlineMean; 
	// 0.99
	double* pdOnlineShortMean;       // 0.9?
	double* pdOnlineLongMean; // Added by Chanwoo Kim... // 0.999

	double* pdOnlinePlusMean;
	double* pdOnlineMinusMean;

	// Online peak value and its smoothed one
	double* pdShortVar;
	double* pdOnlineVar;

	float32* pfSpec;

	double  dOnlineC0Peak;
	double  dOnlineScale;

	float32 ** ppfCepBuffer;

	double* pdMeanSubFeat;
	
	int     iFrameCount;
	int     iFeatVecLen;
};

online_vadmvn_t * online_vadmvn_init(double dLamda, int iFeatVectLen, int iBufferSize);
void online_vadmvn_free(online_vadmvn_t *self);
int32 online_vadmvn_run(online_vadmvn_t *self,
			float32* pOutFeat, int32 iOutFeatLen, 
			float32* pInFeat, int32 iInFeatLen);

#endif /* __ONLINE_VADMVN_H__ */
