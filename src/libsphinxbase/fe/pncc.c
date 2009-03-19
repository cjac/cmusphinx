/////////////////////////////////////////////////////////////////////////////
//
// FileName:
//
//
// History
// 
//  Programmed by Chanwoo Kim
//


//
//	Programmed by Chanwoo Kim 	(Mar 28, 2007)
//
// PCC Program Programmed by Chanwoo Kim (Oct, 20, 2008


#include "pncc.h"
#include "fft.h"

static const int PCC_DEFAULT_FFT_SIZE = 1024;
static const int PCC_DEFAULT_NUM_CHAN = 40;
extern double const 
gammatone_1024FFT_40Ch_130L_6800H[PCC_DEFAULT_NUM_CHAN][PCC_DEFAULT_FFT_SIZE / 2];

pncc_t *
pncc_init(cmd_ln_t *config)
{
	pncc_t *self = ckd_calloc(1, sizeof(*self));


	self->pOnlineVADMVN = online_vadmvn_init(DEFAULT_DLAMBDA,
						 13, 10);
	return (0);
}

void
pncc_free(pncc_t *self)
{
	online_vadmvn_free(self->pOnlineVADMVN);
}

int32 pncc_run()
{
	for (j = 0; j < 40; j++)
	{
		adGamVal[j] = 0.0;
			
		for (k = 0; k < self->pConfig->m_iFFTSize / 2 ; k ++)
		{
			adGamVal[j] += pdSpec[k] * m_aad_H_1024FFT_40Ch_130L_6800H[j][k] * m_aad_H_1024FFT_40Ch_130L_6800H[j][k];
		}
				
		adGamVal[j] = pow(adGamVal[j], 0.1);

	}

	for (j = 0; j < 13; j++)
	{
		afCeps[j] = (float32)adCeps[j];
	}

	bFlag = OutputNorm(afNormCeps, 13, afCeps, 13);

	return (0);
}

int32 pncc_output_norm(pncc_t *self,
		       float* pf_y, 
		       int i_y_Len, 
		       float* pf_x,
		       int i_x_Len)
{
	int32 bFlag;
	int32 i;


	if (i_x_Len != 13 || i_y_Len != 13)
	{
		E_FATAL("Sorry, but currently, feature length is hardwired to 13");
	}

	self->pOnlineVADMVN->Run(pf_y, i_y_Len, pf_x, i_x_Len);

	///////////////////////////////////////////////////////////////////////
	//
	// Check whether this frame should be skipped or not..
	//
	bFlag = 0;
	for (i = 0; i < i_y_Len; i++)
	{
		if (pf_y[i] != 0)
		{
			bFlag = 1;
			break;
		}
	}

	if (bFlag == 0)
	{
		return (0);
	}
	else
	{
		return (1);
	}
}
