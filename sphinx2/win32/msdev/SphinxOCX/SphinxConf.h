// SphinxConf.h: interface for the CSphinxConf class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPHINXCONF_H__CC8C1AD3_742D_11D2_8FAA_0000F802C9A4__INCLUDED_)
#define AFX_SPHINXCONF_H__CC8C1AD3_742D_11D2_8FAA_0000F802C9A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern "C" {
#include "fbs.h"
}

class CSphinxConf  
{
public:
	CSphinxConf();
	virtual ~CSphinxConf();
	long HypConf (search_hyp_t *hyp);

};

#endif // !defined(AFX_SPHINXCONF_H__CC8C1AD3_742D_11D2_8FAA_0000F802C9A4__INCLUDED_)
