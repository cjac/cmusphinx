#ifndef __PNCC_H__
#define __PNCC_H__

// sphinx libraries
#include "fe.h"

#include "online_vadmvn.h"

typedef struct pncc_s pncc_t;
struct pncc_s {
	int        m_iBegIndex;
	int        m_iEndIndex;

	float32    m_afReadSpec[40];

	int        m_iPowerBuffSize;
	int        m_iLookAheadSize;

	online_vadmvn_t* m_pOnlineVADMVN;

	float64  m_dWindowLen;
	float64  m_dFramePeriod;
	float64  m_dSampRate;
	float64* m_pdWindow;
	float64  m_dRMSMean;
	float64  m_dMinRMSPower;

	
	int32   m_iWL;
	int32   m_iFP;
};

#endif /* __PNCC_H__ */
