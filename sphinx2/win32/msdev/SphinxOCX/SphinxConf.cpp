// SphinxConf.cpp: implementation of the CSphinxConf class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sphinx.h"
#include "SphinxConf.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern "C" {
	#include "../../sphinx2/include/fbs.h"
	#include "../../sphinx2/src/fbs/include/lm.h"
	#include "../../sphinx2/src/fbs/include/lm_3g.h"
	#include "../../sphinx2/src/fbs/include/kb_exports.h"
}

long CSphinxConf::HypConf (search_hyp_t *hyp)
{
	search_hyp_t *h;
	int32 w1, w2, finishwid, startwid, type[4096];
	int32 i, k, t;

	finishwid = kb_get_word_id ("</s>");
	startwid = kb_get_word_id ("<s>");
	if ((finishwid < 0) || (startwid < 0))
		return -1;

	w1 = finishwid;
	w2 = startwid;
	type[0] = 3;	// Dummy trigram entry before utterance
	k = 1;
	for (h = hyp; h; h = h->next) {
		if (k > 4095)
			return -1;
		lm_tg_score (w1, w2, h->wid);
		type[k++] = lm3g_access_type();
		w1 = w2;
		w2 = h->wid;
	}
	lm_tg_score (w1, w2, finishwid);
	type[k++] = lm3g_access_type();
	type[k++] = 3;	// Dummy trigram entries after utterance

	for (i = 1, h = hyp; i < k-2; i++, h = h->next) {
		t = type[i-1] + type[i] + ((type[i+1] + type[i+2])<<1);
		h->conf = (float)((double)(t-6)/12.0);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSphinxConf::CSphinxConf()
{

}

CSphinxConf::~CSphinxConf()
{

}
