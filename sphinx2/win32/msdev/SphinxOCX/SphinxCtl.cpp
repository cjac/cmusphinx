// SphinxCtl.cpp : Implementation of the CSphinxCtrl ActiveX Control class.

/*
 * *************************************************************************
 * CMU ARPA Speech Project
 * 
 * Copyright (C) 1998 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * *************************************************************************
 * 
 * HISTORY
 * 
 * 23-Sep-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *		Changed the format of recognition results to be a string containing
 *		attribute value pairs.  This is to accommodate results that include
 *		not just the recognition hypothesis string, but also other values such
 *		as the peak power, confidence values, etc. for the utterance.
 *		Fixed permissions from 02 to 04 in _access call during init().
 */


#include "stdafx.h"

#include <assert.h>

#include "Sphinx.h"
#include "SphinxCtl.h"
#include "objbase.h"
// #include "SphinxPpg.h"
//#include "SphinxConf.h"
//#include "SphinxBuildLm.h"
//#include "SphinxArgfileUpdate.h"

extern "C" {
  /* #include "latnode.h"		// for latnode_t */
#include "kb.h"
#include "lm_3g.h"
#include "lmclass.h"
#include "dict.h"
#include "search.h"
#include "fbs.h"
  unsigned __int16 **search_get_uttpscr();
  int searchFrame();
#include <io.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR gszResultBuffer[8192];

IMPLEMENT_DYNCREATE(CSphinxCtrl, COleControl)

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSphinxCtrl, COleControl)
	//{{AFX_MSG_MAP(CSphinxCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_MESSAGE(WM_UTTINFO, OnUtteranceMessage)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CSphinxCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CSphinxCtrl)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "ArgFile", m_csArgFile, OnArgFileChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "LogFile", m_csLogFile, OnLogFileChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "IgnoreEmptyUtterance", m_fIgnoreEmptyUtterance, OnIgnoreEmptyUtteranceChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "LogDirectory", m_csLogDirectory, OnLogDirectoryChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "SamplesPerSecond", m_samplesPerSecond, OnSamplesPerSecondChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "RawLogDir", m_rawLogDir, OnRawLogDirChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "PartialResultInterval", m_partialResultInterval, OnPartialResultIntervalChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "RequirePartialResult", m_requirePartialResult, OnRequirePartialResultChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "UttIdPrefix", m_uttIdPrefix, OnUttIdPrefixChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSphinxCtrl, "ConfThreshPercentage", m_confThreshPercentage, OnConfThreshPercentageChanged, VT_I4)
	DISP_FUNCTION(CSphinxCtrl, "StartListening", StartListening, VT_I4, VTS_NONE)
	DISP_FUNCTION(CSphinxCtrl, "StopListening", StopListening, VT_I4, VTS_NONE)
	DISP_FUNCTION(CSphinxCtrl, "ToggleListening", ToggleListening, VT_I4, VTS_NONE)
	DISP_FUNCTION(CSphinxCtrl, "SaveLattice", SaveLattice, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CSphinxCtrl, "Init", Init, VT_I4, VTS_NONE)
	DISP_FUNCTION(CSphinxCtrl, "GetDictWord", GetDictWord, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CSphinxCtrl, "GetDictWordID", GetDictWordID, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CSphinxCtrl, "GetUttPscr", GetUttPscr, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CSphinxCtrl, "SetLM", SetLM, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CSphinxCtrl, "GetUttSeqCount", GetUttSeqCount, VT_I4, VTS_NONE)
	DISP_FUNCTION(CSphinxCtrl, "ReadLM", ReadLM, VT_I4, VTS_BSTR VTS_BSTR VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(CSphinxCtrl, "UpdateLM", UpdateLM, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CSphinxCtrl, "DeleteLM", DeleteLM, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CSphinxCtrl, "AddNewWord", AddNewWord, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_STOCKPROP_FONT()
	//}}AFX_DISPATCH_MAP
DISP_FUNCTION_ID(CSphinxCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()
//	DISP_FUNCTION(CSphinxCtrl, "AddWord", AddWord, VT_I4, VTS_BSTR)



/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CSphinxCtrl, COleControl)
	//{{AFX_EVENT_MAP(CSphinxCtrl)
	EVENT_CUSTOM("UtteranceResult", FireUtteranceResult, VTS_BSTR)
	EVENT_CUSTOM("NewUtterance", FireNewUtterance, VTS_NONE)
	EVENT_CUSTOM("EndUtterance", FireEndUtterance, VTS_NONE)
	EVENT_CUSTOM("UtteranceError", FireUtteranceError, VTS_I4)
	EVENT_CUSTOM("StartListening", FireStartListening, VTS_NONE)
	EVENT_CUSTOM("StopListening", FireStopListening, VTS_NONE)
	EVENT_CUSTOM("UtterancePartialResult", FireUtterancePartialResult, VTS_BSTR)
	EVENT_CUSTOM("UtterancePower", FireUtterancePower, VTS_I4)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS( CSphinxCtrl, 2 )
	// PROPPAGEID( CSphinxPropPage::guid )
	//PROPPAGEID( CSphinxArgfileUpdate::guid )
//	PROPPAGEID( CLSID_CFontPropPage )
///	PROPPAGEID( CSphinxBuildLm::guid )
END_PROPPAGEIDS( CSphinxCtrl )


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CSphinxCtrl, "SPHINX.SphinxCtrl.1",
	0x9cd6ab6f, 0x7302, 0x11d0, 0x95, 0x53, 0, 0, 0xf8, 0x2, 0xbd, 0x8e)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CSphinxCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DSphinx =
		{ 0xc8bfb24e, 0x466b, 0x11d1, { 0xb8, 0xfc, 0, 0x60, 0x8, 0x16, 0x5b, 0x1e } };
const IID BASED_CODE IID_DSphinxEvents =
		{ 0xc8bfb24f, 0x466b, 0x11d1, { 0xb8, 0xfc, 0, 0x60, 0x8, 0x16, 0x5b, 0x1e } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwSphinxOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CSphinxCtrl, IDS_SPHINX, _dwSphinxOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::CSphinxCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CSphinxCtrl

BOOL CSphinxCtrl::CSphinxCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_SPHINX,
			IDB_SPHINX,
			afxRegApartmentThreading,
			_dwSphinxOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::CSphinxCtrl - Constructor

CSphinxCtrl::CSphinxCtrl()
{
	InitializeIIDs(&IID_DSphinx, &IID_DSphinxEvents);

	m_thDecode = NULL;

	// TODO: Initialize your control's instance data here.
	m_lState = ST_IDLE;
	m_fThreadCreated = FALSE;
	m_fADStarted = FALSE;
	m_fADRestart = FALSE;
	m_uttSeqCount = 0;

	m_lThreadId = GetCurrentThreadId();

//	SetInitialSize( 40, 40 );
//	m_fSetSize = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::~CSphinxCtrl - Destructor

CSphinxCtrl::~CSphinxCtrl()
{
	// TODO: Cleanup your control's instance data here.

	if (m_thDecode)
		m_thDecode->SuspendThread();

	if (m_lState == ST_LISTENING)
		StopListening();

	m_lState = ST_EXITING;
/*
	if (m_fThreadCreated) {
		do {
			Sleep(100);
			if (! GetExitCodeThread (m_thDecode->m_hThread, &dwCode))
				break;
		} while (dwCode == STILL_ACTIVE);
	}
  */
	if ( m_fADStarted ) {	 
		fbs_end();
		cont_ad_close( m_cont );
		ad_close( m_ad );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::OnDraw - Drawing function

void CSphinxCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	DoSuperclassPaint(pdc, rcBounds);
	if (!m_fSetSize) { // Prevent infinite recursion!
		m_fSetSize = true;
		SetControlSize( 40, 40 );
		Refresh();
		m_fSetSize = false;
	}
	else {
		CPoint cpCenter = rcBounds.CenterPoint();	 
		CBrush brush( GetSysColor( COLOR_3DFACE ) );
		
		pdc->FillRect( rcBounds, &brush );
		pdc->DrawIcon( cpCenter.x - 16, cpCenter.y - 16, LoadIcon( AfxGetInstanceHandle(), 
									MAKEINTRESOURCE( IDI_ABOUTDLL ) ) );
			}
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::DoPropExchange - Persistence support

void CSphinxCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.
	PX_String( pPX, _T( "ArgFile" ), m_csArgFile, _T( ARG_ARGFILE ) );
	PX_String( pPX, _T( "LogDirectory" ), m_csLogDirectory, _T( "" ) );
	PX_String( pPX, _T( "RawLogDir" ), m_rawLogDir, _T( "" ) );
	PX_String( pPX, _T( "UttIdPrefix" ), m_uttIdPrefix, _T( DEFAULT_UTT_ID_PREFIX ) );
	PX_Bool( pPX, _T( "IgnoreEmptyUtterance" ), m_fIgnoreEmptyUtterance, TRUE );
	PX_String( pPX, _T( "LogFile" ), m_csLogFile, _T( ARG_LOGFILE ) );
	PX_Long( pPX, _T( "SamplesPerSecond" ), m_samplesPerSecond, DEFAULT_SAMPLES_PER_SECOND );
	PX_Bool( pPX, _T( "RequirePartialResult" ), m_requirePartialResult, FALSE );
	PX_Long( pPX, _T( "PartialResultInterval" ), m_partialResultInterval, DEFAULT_PARTIAL_RESULT_INTERVAL );
	PX_Long( pPX, _T( "ConfThreshPercentage" ), m_confThreshPercentage, DEFAULT_CONF_THRESH );
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::OnResetState - Reset control to default state

void CSphinxCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::AboutBox - Display an "About" box to the user

void CSphinxCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_SPHINX);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::PreCreateWindow - Modify parameters for CreateWindowEx

BOOL CSphinxCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = _T("STATIC");
	return COleControl::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::IsSubclassedControl - This is a subclassed control

BOOL CSphinxCtrl::IsSubclassedControl()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl::OnOcmCommand - Handle command messages

LRESULT CSphinxCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
	WORD wNotifyCode = HIWORD(wParam);
#else
	WORD wNotifyCode = HIWORD(lParam);
#endif

	// TODO: Switch on wNotifyCode here.

	return 0;
}


////////////////////////////////////////////////////////////////////////////
// decode_thread. From demo2Dlg.cpp
//
//  The decoding thread.  Created by the main thread when the user first clicks the START
//	LISTENING button.  Subsequent stops and starts just cause this thread to sleep and
//	resume, by monitoring the state changes effected by the main thread.  Once started,
//	the thread constantly calls the cont_ad_read function to obtain non-silence A/D data,
//	breaks them up into utterances based on long intervening silences and decodes each
//	one.
// 

extern "C" {
	int32 searchCurrentFrame(void);
}

void
strip_class(char *inout)
{
	char *out;
	char *in;
	char *e;

	if (inout == NULL) {
		return;
	}

	in = out = inout;
	while (*in) {
		if (*in == '_') {
			*in = ' ';
		}
		else if (*in == ':') {
			/* find the first space after the underscore */
			e = strchr(in, ' ');
			if (e == NULL) {
				/* No space after underscore.  Position e to
					terminal nul char */
				e = in + strlen(in);
			}
			in = e;
		}

		*out = *in;

		if (*in) {
			/* if not at terminal nul, advance one char */
			out++;
			in++;
		}
	}
	*out = *in;
}

static UINT DecodeThread (LPVOID p)
{
	int32 k, rem, ts, utt_maxpow;
	int16 buf[2048];
	char* szResult;
	char* szPartResult;
	char* szLastPartResult = NULL;
	int32 n_proc_frm;			// # of frames that where processed to produce a partial result
	int32 update_frm;		// The frame of the last partial result update
	CString csResult, csConfResult, formatResult;
	CSphinxCtrl* cscParent;
	_TCHAR* szTemp;
	search_hyp_t *hypseg, *h;
	char confhypstr[4096];

	FILE *cont_ad_fp;
	cont_ad_fp = fopen ("d:\\sphinx\\arg\\demo.cont","wt");
	cont_ad_set_logfp (cont_ad_fp);

	// parameter p is a pointer to the CSphinxCtrl object that invoked the thread
	cscParent = (CSphinxCtrl*) p;

//	assert( cscParent->PostMessage( WM_VSCROLL, 0, 0 ) );

	while (cscParent->m_lState != ST_EXITING) {
		// Wait for application to get out of non-IDLE (not-listening) state
		while (cscParent->m_lState == ST_IDLE)
			Sleep(200);

		if (cscParent->m_lState != ST_LISTENING)	// EXITING or ERROR state
			break;

		// User clicked START LISTENING; start A/D recording
		ad_start_rec (cscParent->m_ad);

		// Wait until start of utterance (non-zero amount of data from cont_ad_read)
		while ((cscParent->m_lState == ST_LISTENING) && ((k = cont_ad_read (cscParent->m_cont, buf, 2048)) == 0))
		  Sleep(100);
		
		// Perhaps user clicked STOP LISTENING without saying anything at all
		if (cscParent->m_lState != ST_LISTENING) {
		  ad_stop_rec (cscParent->m_ad);
		  continue;
		}
		//		if (cscParent->m_cont->siglvl > 0) 
		  E_INFO ("singal level: %d\n",cscParent->m_cont->siglvl);
		if (k < 0) {
			E_ERROR("cont_ad_read failed\n");
			cscParent->PostMessage( WM_UTTINFO, UTT_ERROR, (LPARAM) UTTERR_CONT_AD_READ_FAIL );
			//cscParent->FireUtteranceError(UTTERR_CONT_AD_READ_FAIL);
			return 1;
		}

		// Start of utterance; start decoding
		cscParent->PostMessage( WM_UTTINFO, UTT_NEWUTT, 0 );
		//cscParent->FireNewUtterance();
		uttproc_begin_utt (NULL);
		uttproc_rawdata (buf, k, 0);
		utt_maxpow = cscParent->m_cont->siglvl;
		cscParent->PostMessage (WM_UTTINFO,UTT_POWER,(LPARAM) utt_maxpow); // RAH 1.22.2000
		update_frm = 0;

		// Note current timestamp from continuous listening module
		ts = cscParent->m_cont->read_ts;

		// Decode until end of utterance
		for (;;) {
			if ((k = cont_ad_read (cscParent->m_cont, buf, 2048)) < 0) {
				E_ERROR("cont_ad_read failed\n");
				cscParent->PostMessage( WM_UTTINFO, UTT_ERROR, (LPARAM) UTTERR_CONT_AD_READ_FAIL );
				//cscParent->FireUtteranceError(UTTERR_CONT_AD_READ_FAIL);
				return 1;
			}
			cscParent->PostMessage (WM_UTTINFO,UTT_POWER,(LPARAM) cscParent->m_cont->siglvl); // RAH 1.22.2000
			if (k == 0) {
				// No data read; check timestamp difference compared to most recent
				// non-silence data.  If difference > 1sec worth of data, end of utt
				if ((cscParent->m_cont->read_ts - ts) > (cscParent->m_samplesPerSecond >> 1))
					break;
			} else {
				// Data available; update most recent non-silence timestamp
				ts = cscParent->m_cont->read_ts;
				if (cscParent->m_cont->siglvl > utt_maxpow)
					utt_maxpow = cscParent->m_cont->siglvl;
			}

			// Decode new data (non-blocking mode; see DEMO1 for more info)
			rem = uttproc_rawdata (buf, k, 0);

			// Sleep a bit if no work remaining to be done
			if ((rem == 0) && (k == 0))
				Sleep(20);

			if (cscParent->m_requirePartialResult) {
				// The application wants partial results

				if (uttproc_partial_result(&n_proc_frm, &szPartResult) < 0) {
					cscParent->PostMessage( WM_UTTINFO, UTT_ERROR, (LPARAM) UTTERR_UTTPROC_RESULT_FAIL);
					return 1;
				}

				if ((n_proc_frm - update_frm) > cscParent->m_partialResultInterval) {
					// We've processed more frames than the update interval since
					// the last update.  Check if a different partial result is available

					update_frm = n_proc_frm;

					if ((szLastPartResult == NULL) ||
						(strcmp(szLastPartResult, szPartResult) != 0)) {
						// If we have never sent a partial result back or
						// the last partial result is different from this one
						// report the new partial result to the main thread

						strip_class(szPartResult);

						csResult = szPartResult;
						csResult.TrimLeft();

						if (!(cscParent->m_fIgnoreEmptyUtterance && csResult.IsEmpty())) {
							formatResult.Format("hyp: %s\nnpow: %d\npow: %d\nid: %s\n",
								csResult,
								cscParent->m_cont->noise_level,
								utt_maxpow,
								uttproc_get_uttid());
							szTemp = new _TCHAR[formatResult.GetLength() + 1];
							_tcscpy( szTemp, formatResult );
							cscParent->PostMessage( WM_UTTINFO, UTT_PARTIAL, (LPARAM) (szTemp) );
						}

						if (szLastPartResult) {
							free(szLastPartResult);
						}

						szLastPartResult = strdup(szPartResult);
					}
				}
			}
		}

		// End of utterance after "long" silence
		uttproc_end_utt ();		// Bookkeeping
		cscParent->PostMessage( WM_UTTINFO, UTT_ENDUTT, 0 );
		cscParent->PostMessage (WM_UTTINFO,UTT_POWER,(LPARAM) -1); // RAH 1.22.2000


		//cscParent->FireEndUtterance();

		cont_ad_powhist_dump(stdout, cscParent->m_cont);

		if (cscParent->m_lState != ST_LISTENING) {	// User clicked STOP LISTENING in the meantime
			ad_stop_rec (cscParent->m_ad);
//			AfxMessageBox( "Called ad_stop_rec" );
		}

		// Obtain result for utterance, BLOCKING mode (see DEMO1 for non-blocking mode)
		if (uttproc_result_seg (&(cscParent->m_lFrm), &hypseg, 1) < 0) {
			E_ERROR("uttproc_result failed\n");
			cscParent->PostMessage( WM_UTTINFO, UTT_ERROR, (LPARAM) UTTERR_UTTPROC_RESULT_FAIL );
			//cscParent->FireUtteranceError(UTTERR_UTTPROC_RESULT_FAIL);
			return 1;
		}

		/* Fill in confidence values for words in result and build filtered hypothesis */
		{
			int32 len;
			double conf_thresh;

			cscParent->conf->HypConf(hypseg);
			conf_thresh = (double)(cscParent->m_confThreshPercentage) * 0.01;

			len = 0;
			for (h = hypseg; h; h = h->next) {
				if (h->conf >= conf_thresh) {
					strcpy (confhypstr+len, h->word);
					len += strlen(h->word);
				} else {
					strcpy (confhypstr+len, ".?");
					len += 2;
					strcpy (confhypstr+len, h->word);
					len += strlen(h->word);
					strcpy (confhypstr+len, "?.");
					len += 2;
				}
				confhypstr[len++] = ' ';
				confhypstr[len] = '\0';
			}
			strip_class (confhypstr);
			csConfResult = confhypstr;
		}

		/* Raw result string */
		search_result (&(cscParent->m_lFrm), &szResult);
		strip_class(szResult);
		csResult = szResult;
		csResult.TrimLeft();

		if (!(cscParent->m_fIgnoreEmptyUtterance && csResult.IsEmpty())) {
		//if (!csResult.IsEmpty()) {
			(cscParent->m_uttSeqCount)++;
#ifdef _SIMPLERESULT
			formatResult.Format("%s", csResult);
#else
			formatResult.Format("hyp: %s\nconfhyp: %s\nnpow: %d\npow: %d\nseq: %d\nid: %s\n",
				csResult,
				csConfResult,
				cscParent->m_cont->noise_level,
				utt_maxpow,
				cscParent->m_uttSeqCount,
				uttproc_get_uttid());
#endif
			szTemp = new _TCHAR[formatResult.GetLength() + 1];
			_tcscpy( szTemp, formatResult );
			cscParent->PostMessage( WM_UTTINFO, UTT_RESULT, (LPARAM) (szTemp) );
			//cscParent->FireUtteranceResult(csResult);
		}
		E_INFO("%s: Noise pow: %d; Max pow: %d; Seq: %d\n", uttproc_get_uttid(),
			cscParent->m_cont->noise_level, utt_maxpow, cscParent->m_uttSeqCount);
	}

	// printf ("Thread exiting...\n"); fflush (stdout);
	return (0);
}


// not sure if this will work, but hey...
_inline char* UniToAscii( CString csUni ) 
{
	_TCHAR* szTemp = _tcsdup( csUni );

#ifdef _UNICODE
	char* szAscii;
	int i;
	int lMax;
	int lAdd;

	lMax = csUni.GetLength();

	szAscii = (char*) malloc( sizeof( char ) * (lMax + 1) );

	// this oughta make it work for both endian-ness-es
	if (((char*) szTemp)[(2 * i)] == '\0')
		lAdd = 1;
	else
		lAdd = 0;

	for (i = 0; i < lMax; i++) {
		szAscii[i] = ((char*) szTemp)[(2 * i) + lAdd];
	}
	szAscii[i] = '\0';

	free( szTemp );

	return szAscii;
#else // it's identical to strdup
	return szTemp;
#endif
}

// converts the internal arguments into an artificial argv for fbs_init() to use
char** CSphinxCtrl::MakeArgList( int* plArgCount )
{
	char** ppszArgList = new char* [ARG_COUNT + 1];

	*plArgCount = ARG_COUNT;

	if (_access(m_csArgFile, 04) < 0) {
		AfxMessageBox("Argfile is not readable\n");
		exit(-1);
	}

	ppszArgList[0] = strdup( ARG_EXENAME );
	ppszArgList[1] = strdup( "-argfile" );
	ppszArgList[2] = UniToAscii( m_csArgFile );// strdup( (const char*) m_csArgFile );
	ppszArgList[3] = strdup( "-logfn" );
	ppszArgList[4] = UniToAscii( m_csLogFile );//strdup( (const char*) m_csLogFile );
	if (!m_csLogDirectory.IsEmpty()) {
		ppszArgList[5] = strdup( "-rawlogdir" );
		ppszArgList[6] = UniToAscii( m_csLogDirectory );
	}
	else
		*plArgCount -= 2;

	ppszArgList[*plArgCount] = NULL;
  
	return ppszArgList;
//	return NULL;
}
	

long CSphinxCtrl::InitAD()
{
	CString csTemp;
	int lArgCount;
	char** ppszArgList = MakeArgList( &lArgCount );
	csTemp = "Argument list:\n";
	const char *ttt;
	
	for (int i = 0; i < lArgCount; i++) {
		csTemp += ppszArgList[i];
		csTemp += '\n';
	}
	//AfxMessageBox( csTemp );

	//AfxMessageBox( "Starting fbs_init" );
	fbs_init( lArgCount, ppszArgList );
	//AfxMessageBox( "Finished with fbs_init" );

	ttt = (const char *)m_uttIdPrefix;

	uttproc_set_auto_uttid_prefix((char *)ttt);

	delete ppszArgList;

	m_lState = ST_IDLE;

	/* Open and start A/D recording; clear A/D buffer */
	E_INFO("Opening A/D, sps= %d\n", m_samplesPerSecond);
	if ((m_ad = ad_open_sps (m_samplesPerSecond)) == NULL) {
		E_ERROR("ad_open failed\n");
		//EndDialog(IDOK);
	}

	/* Open the continuous listening/silence filtering module and calibrate it */
	if ((m_cont = cont_ad_init (m_ad, ad_read)) == NULL) {
		ad_close (m_ad);
		E_ERROR("cont_ad_init failed\n");
		//EndDialog(IDOK);
	}
	if (ad_start_rec (m_ad) < 0) {	// Need to start recording for calibration
		ad_close (m_ad);
		cont_ad_close (m_cont);
		E_ERROR("ad_start_rec failed\n");
		//EndDialog(IDOK);
	}
	cont_ad_calib (m_cont);
	ad_stop_rec (m_ad);				// Stop recording after calibration

	conf = new CSphinxConf();

	//AfxMessageBox( "Sphinx ready.", MB_ICONINFORMATION ); 

	//PostMessage( WM_VSCROLL, 0, 0 );

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl message handlers

long CSphinxCtrl::StartListening() 
{
	// TODO: Add your dispatch handler code here

	// initialize, if it hasn't happened already
	if (!(m_fADStarted && m_fThreadCreated)) 
		Init();

	m_lState = ST_LISTENING;	// Signals decoder thread to resume

	FireStartListening();

	return 0;
}

long CSphinxCtrl::StopListening() 
{
	// TODO: Add your dispatch handler code here
	if (m_lState == ST_LISTENING) {
		m_lState = ST_IDLE;		// Signals decoder thread to pause
		//ad_stop_rec( m_ad );

		FireStopListening();
	}

	return 0;
}

long CSphinxCtrl::ToggleListening() 
{
	// TODO: Add your dispatch handler code here
	if (m_lState == ST_LISTENING)
		StopListening();
	else if (m_lState == ST_IDLE)
		StartListening();

	return m_lState;
}

void CSphinxCtrl::OnArgFileChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSphinxCtrl::OnLogFileChanged() 
{
	// TODO: Add notification handler code
	const char *tmp;

	tmp = m_csLogFile;

	uttproc_set_logfile((char *)tmp);

	SetModifiedFlag();
}

void CSphinxCtrl::OnIgnoreEmptyUtteranceChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

afx_msg LRESULT CSphinxCtrl::OnUtteranceMessage( WPARAM lMessage, LPARAM lExtra )
{
	_TCHAR* szTemp = (_TCHAR*) lExtra;

	switch (lMessage) {
	case UTT_NEWUTT:
		FireNewUtterance();
		break;
	case UTT_ERROR:
		FireUtteranceError( (int) lExtra );
		break;
	case UTT_ENDUTT:
		FireEndUtterance();
		break;
	case UTT_RESULT:
//		AfxMessageBox( szTemp );
		FireUtteranceResult( szTemp );
		delete [] szTemp;
		break;
	case UTT_PARTIAL:
//		AfxMessageBox( szTemp );
		FireUtterancePartialResult( szTemp );
		delete [] szTemp;
		break;
	case UTT_POWER:
	  FireUtterancePower( (int) lExtra );
	  break;
	}

	return TRUE;
}

long CSphinxCtrl::SaveLattice( LPCTSTR szFilename ) 
{
	// TODO: Add your dispatch handler code here
	latnode_t*	pLat;
	FILE*		fOut;

    search_save_lattice ();
    sort_lattice ();
    pLat = search_get_lattice ();

	if (!(fOut = _tfopen( szFilename, _T("w") )))
		return FALSE;
	
	while (pLat) {
		fprintf( fOut, "%d %d %d %d %d %d\n", pLat->wid, pLat->fef, 
				 pLat->lef, pLat->sf, pLat->reachable, pLat->info.fanin );
		pLat = pLat->next;
	}

	fclose( fOut );

	return TRUE;
}


long CSphinxCtrl::Init() 
{
	// TODO: Add your dispatch handler code here

	// fire up the A/D stuff
	if (!m_fADStarted) {
		InitAD();
		m_fADStarted = TRUE;
	}

	// If decoding thread not already started, start one
	if (!m_fThreadCreated) {
		m_thDecode = AfxBeginThread( DecodeThread, this );
		m_fThreadCreated = TRUE;
	}

	return TRUE;
}

BSTR CSphinxCtrl::GetDictWord( long lID ) 
{
	CString csResult;
//    dictT* pdict = kb_get_word_dict();

//	csResult = dictid_to_str( pdict, lID );

	AfxMessageBox("GetDictWord is unimplemented at present");

	return csResult.AllocSysString();
}

long CSphinxCtrl::GetDictWordID( LPCTSTR szWord ) 
{
    char* szTemp = UniToAscii( szWord );
	int   lReturn;

	lReturn = kb_get_word_id( szTemp );
	free( szTemp );

	return lReturn;
}

VARIANT CSphinxCtrl::GetUttPscr() 
{
	VARIANT vaResult;
/*	VARIANT vaTemp;
	SAFEARRAY FAR* psaResult;
	unsigned __int16** ppnUttPscr;
	SAFEARRAYBOUND sabound[2];
	int i, j;
	long lIndices[2];

	VariantInit( &vaResult );
	ppnUttPscr = search_get_uttpscr();

	if (!ppnUttPscr) {
		vaResult.vt = VT_EMPTY;
		return vaResult;
	}

	sabound[0] = searchFrame();
	sabound[1] = phoneCiCount();

	psaResult = (SAFEARRAY FAR*) SafeArrayCreate( VT_I2, 2, sabound );

	vaTemp.vt = VT_I2;

	for (lIndices[0] = 0; lIndices[0] < rgsaBound[0]; lIndices[0]++) {
		for (lIndices[1] = 0; lIndices[1] < rgsaBound[1]; lIndices[1]++) {
			vaTemp.iVal = (short) ppnUttPscr[lIndices[0]][lIndices[1]];
			if (SafeArrayPutElement( psaResult, lIndices, &vaTemp ) != S_OK) {
				AfxMessageBox( "SafeArrayPutElement error." );
				vaResult.vt = VT_EMPTY;
				return vaResult;
			}
		}
	}

	vaResult.vt = VT_ARRAY | VT_I2;
	vaResult.parray = psaResult;*/

	return vaResult;
}

void CSphinxCtrl::OnLogDirectoryChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSphinxCtrl::OnSamplesPerSecondChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSphinxCtrl::OnRawLogDirChanged() 
{
	const char *tmp;

	tmp = m_rawLogDir;

	uttproc_set_rawlogdir((char *)tmp);

	SetModifiedFlag();
}

void CSphinxCtrl::OnPartialResultIntervalChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSphinxCtrl::OnRequirePartialResultChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSphinxCtrl::OnUttIdPrefixChanged() 
{
	// TODO: Add notification handler code
	const char *ttt;

	ttt = (const char *)m_uttIdPrefix;
	uttproc_set_auto_uttid_prefix((char *)ttt);

	SetModifiedFlag();
}

long CSphinxCtrl::GetUttSeqCount() 
{
	// TODO: Add your dispatch handler code here

	return m_uttSeqCount;
}

void CSphinxCtrl::OnConfThreshPercentageChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}


long CSphinxCtrl::SetLM(LPCTSTR lmName) 
{
	/*
	 * Set the currently active LM to the given named LM.  Multiple LMs can be loaded initially
	 * (during fbs_init) or at run time using lm_read (see below).
	 * Return value: 0 if successful, else -1.
	 */
	return uttproc_set_lm ((char *)lmName);
}


long CSphinxCtrl::ReadLM(LPCTSTR szLmFile, LPCTSTR szLmName, double lw=7.5, double uw=0.5, double wip=0.65) 
{
	/*
	 * Read in a new LM file (lmfile), and associate it with name lmname.  If there is
	 * already an LM with the same name, it is automatically deleted.  The current LM is
	 * undefined at this point; use uttproc_set_lm(lmname) immediately afterwards.
	 * Return value: 0 if successful, else -1.
	 */
	return lm_read ((char *)szLmFile,	/* In: LM file name */
					(char *)szLmName,	/* In: LM name associated with this model */
					lw,			/* In: Language weight; typically 6.5-9.5 */
					uw,			/* In: Unigram weight; typically 0.5 */
					wip);		/* In: Word insertion penalty; typically 0.65 */
}


long CSphinxCtrl::UpdateLM(LPCTSTR szLmName) 
{
	/*
	 * Indicate to the decoder that the named LM has been updated (e.g., with the addition of
	 * a new unigram).
	 * Return value: 0 if successful, else -1.
	 */
	return uttproc_lmupdate ((char *)szLmName);
}


long CSphinxCtrl::DeleteLM(LPCTSTR szLmName) 
{
	/*
	* Delete the named LM from the LM collection.  The current LM is undefined at this
	* point.  Use uttproc_set_lm(...) immediately afterwards.
	* Return value: 0 if successful, else -1.
	*/
	return lm_delete ((char *)szLmName);
}





void CSphinxCtrl::AddNewWord(LPCTSTR lmName, LPCTSTR className, LPCTSTR newWord, LPCTSTR pron) 
{
  // TODO: Add your dispatch handler code here

  /*  

      RAH 11.22.199 - This is currently not supported. It relies on
      some function from the libraries that are not being carried
      forward with the this version. 

      We should seriously look at what is missing


  lmclass_set_t lmclass_set;
  lmclass_t lmclass;
  dictT *dict;
  int32 n, wid;
  lm_t *lm;
  
  // Updating the [name] class 
  lmclass_set = kb_get_lmclass_set ();
  lmclass = lmclass_get_lmclass (lmclass_set, (char *)className);
  if (! lmclass) {
    E_ERROR("Couldn't find the specified lmclass \n");
    return;
  }
  
  // Add new word to dictionary 
  dict = kb_get_word_dict ();
  wid = dict_add_word (dict, (char *)newWord, (char *)pron);
  if (wid < 0) {
    E_ERROR("dict_add_word(%s,%s) failed\n", newWord, pron);
    return;
  }
  
  // Finally, add word to class LM and reconfigure search lextree 
  lm = lm_get_current();	// Assuming there is only one
  if (lm_add_classword (lm, wid, lmclass) < 0)
    E_ERROR("lm_add_classword(%s) failed\n", newWord);
  else {
    uttproc_lmupdate((char *)lmName);
    E_INFO("lm_add_classword(%s) done\n", newWord);
  } */
}
