#if !defined(AFX_SPHINXCTL_H__C8BFB25D_466B_11D1_B8FC_006008165B1E__INCLUDED_)
#define AFX_SPHINXCTL_H__C8BFB25D_466B_11D1_B8FC_006008165B1E__INCLUDED_

#define DEFAULT_SAMPLES_PER_SECOND	16000
#define DEFAULT_UTT_ID_PREFIX		""

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SphinxConf.h"

// sphinx includes
extern "C"
{
#include <fbs.h>
#include "ad.h"
#include "cont_ad.h"
#include "err.h"
}


// SphinxCtl.h : Declaration of the CSphinxCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CSphinxCtrl : See SphinxCtl.cpp for implementation.

class CSphinxCtrl : public COleControl
{
	DECLARE_DYNCREATE(CSphinxCtrl)

// Constructor
public:
	CSphinxCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSphinxCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CSphinxCtrl();

	DECLARE_OLECREATE_EX(CSphinxCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CSphinxCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CSphinxCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CSphinxCtrl)		// Type name and misc status

	// Subclassed control support
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL IsSubclassedControl();
	LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

// Message maps
	//{{AFX_MSG(CSphinxCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CSphinxCtrl)
	CString m_csArgFile;
	afx_msg void OnArgFileChanged();
	CString m_csLogFile;
	afx_msg void OnLogFileChanged();
	BOOL m_fIgnoreEmptyUtterance;
	afx_msg void OnIgnoreEmptyUtteranceChanged();
	CString m_csLogDirectory;
	afx_msg void OnLogDirectoryChanged();
	long m_samplesPerSecond;
	afx_msg void OnSamplesPerSecondChanged();
	CString m_rawLogDir;
	afx_msg void OnRawLogDirChanged();
	long m_partialResultInterval;
	afx_msg void OnPartialResultIntervalChanged();
	BOOL m_requirePartialResult;
	afx_msg void OnRequirePartialResultChanged();
	CString m_uttIdPrefix;
	afx_msg void OnUttIdPrefixChanged();
	long m_confThreshPercentage;
	afx_msg void OnConfThreshPercentageChanged();
	afx_msg long StartListening();
	afx_msg long StopListening();
	afx_msg long ToggleListening();
	afx_msg long SaveLattice(LPCTSTR szFilename);
	afx_msg long Init();
	afx_msg BSTR GetDictWord(long lID);
	afx_msg long GetDictWordID(LPCTSTR szWord);
	afx_msg VARIANT GetUttPscr();
	afx_msg long SetLM(LPCTSTR lmName);
	afx_msg long GetUttSeqCount();
	afx_msg long ReadLM(LPCTSTR szLmFile, LPCTSTR szLmName, double lw, double uw, double wip);
	afx_msg long UpdateLM(LPCTSTR szLmName);
	afx_msg long DeleteLM(LPCTSTR szLmName);
	afx_msg void AddNewWord (LPCTSTR lmName, LPCTSTR className, LPCTSTR newWord, LPCTSTR pron);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CSphinxCtrl)
	void FireUtteranceResult(LPCTSTR Result)
		{FireEvent(eventidUtteranceResult,EVENT_PARAM(VTS_BSTR), Result);}
	void FireNewUtterance()
		{FireEvent(eventidNewUtterance,EVENT_PARAM(VTS_NONE));}
	void FireEndUtterance()
		{FireEvent(eventidEndUtterance,EVENT_PARAM(VTS_NONE));}
	void FireUtteranceError(long Error)
		{FireEvent(eventidUtteranceError,EVENT_PARAM(VTS_I4), Error);}
	void FireStartListening()
		{FireEvent(eventidStartListening,EVENT_PARAM(VTS_NONE));}
	void FireStopListening()
		{FireEvent(eventidStopListening,EVENT_PARAM(VTS_NONE));}
	void FireUtterancePartialResult(LPCTSTR Result)
		{FireEvent(eventidUtterancePartialResult,EVENT_PARAM(VTS_BSTR), Result);}
	void FireUtterancePower(long Power)
		{FireEvent(eventidUtterancePower,EVENT_PARAM(VTS_I4), Power);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CSphinxCtrl)
	dispidArgFile = 1L,
	dispidLogFile = 2L,
	dispidIgnoreEmptyUtterance = 3L,
	dispidLogDirectory = 4L,
	dispidSamplesPerSecond = 5L,
	dispidRawLogDir = 6L,
	dispidPartialResultInterval = 7L,
	dispidRequirePartialResult = 8L,
	dispidUttIdPrefix = 9L,
	dispidConfThreshPercentage = 10L,
	dispidStartListening = 11L,
	dispidStopListening = 12L,
	dispidToggleListening = 13L,
	dispidSaveLattice = 14L,
	dispidInit = 15L,
	dispidGetDictWord = 16L,
	dispidGetDictWordID = 17L,
	dispidGetUttPscr = 18L,
	dispidSetLM = 19L,
	dispidGetUttSeqCount = 20L,
	dispidReadLM = 21L,
	dispidUpdateLM = 22L,
	dispidDeleteLM = 23L,
	dispidAddNewWord = 24L,
	eventidUtteranceResult = 1L,
	eventidNewUtterance = 2L,
	eventidEndUtterance = 3L,
	eventidUtteranceError = 4L,
	eventidStartListening = 5L,
	eventidStopListening = 6L,
	eventidUtterancePartialResult = 7L,
	eventidUtterancePower = 8L,
	//}}AFX_DISP_ID
	};

//////////////// SDH added
friend UINT DecodeThread (LPVOID p);

private:
	int					m_lThreadId;		// So that the decoder can pass us messages
	int					m_lFrm;				// For retrieving the final result for each utterance 
	int					m_lArgumentCount;	// The surrogate __argc
	char**				m_pszArgumentList;	// The surrogate __argv, converted from a m_csArgumentList
	int					m_lState;			// The decoder's state
	BOOL				m_fThreadCreated;	// Is the thread created?
	BOOL				m_fADStarted;		// Is the A/D stuff started?
	BOOL				m_fADRestart;		// Should the A/D be restarted whenever possible?
	CWinThread*			m_thDecode;			// Decoder thread object
	cont_ad_t*			m_cont;				// The continuous listening module object
	ad_rec_t*			m_ad;				// The raw A/D source object
	bool				m_fSetSize;			// Used in OnDraw to prevent resizing
//    dictT*				m_pDict;			// The dictionary; for 
	int					m_uttSeqCount;			// Global count of final results posted so far
												// maintained, actually, by decode thread.
	long InitAD();
	char** MakeArgList( int* );
	CSphinxConf			*conf;

	afx_msg LRESULT OnUtteranceMessage( WPARAM lMessage, LPARAM lExtra );

//////////////// end added

};

///////////////////////////////////////////////////////////////////////
// SDH added
//
// adapted from fbs demo2

// Different types of information passed to main thread via WM_UTTINFO message 
#define UTT_NEWUTT		0	// Beginning of a new utterance
#define UTT_ENDUTT		1	// End of utterance input data
#define UTT_RESULT		2	// Result of most recent utterance now available
#define UTT_ERROR		3	// Some error during decoding; should never occur
#define UTT_PARTIAL		4	// Partial decoding result
#define UTT_POWER               5       // For power information

#define WM_UTTINFO		(WM_USER + 1)

// Different states the application can be in
#define ST_IDLE			0	// Not listening; user must click START to start listening
#define ST_LISTENING	1	// Listening; decoding automatically segmented utterances
#define ST_ERROR		2	// Some error occurred; should never happen
#define ST_EXITING		3	// Exiting, waiting for decoder thread to catch this and exit

// Default values for the surrogate __argv to pass to fbs_init
#define ARG_EXENAME		"sphinx.exe"	// argv[0] -- I don't think it matters...
#define ARG_ARGFILE		".\\lm\\lm.arg"
#define ARG_LOGFILE		".\\sphinxocx.log"
#define ARG_COUNT		7				// not an argument -- # items in arg list
										// should generally be (2p+1), where p is the
										// number of parameters

#define DEFAULT_PARTIAL_RESULT_INTERVAL	10	// # of frames between partial results
#define DEFAULT_CONF_THRESH		45

// Error codes passed to FireUtteranceError 
#define UTTERR_OTHER				0
#define UTTERR_CONT_AD_READ_FAIL	1	 
#define UTTERR_UTTPROC_RESULT_FAIL	2

#define MAX_RESULT		32768	// max length of result string

static UINT decode_thread (LPVOID p);

// end added
///////////////////////////////////////////////////////////////////////


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHINXCTL_H__C8BFB25D_466B_11D1_B8FC_006008165B1E__INCLUDED)
