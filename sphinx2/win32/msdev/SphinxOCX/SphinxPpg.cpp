// SphinxPpg.cpp : Implementation of the CSphinxPropPage property page class.

#include "stdafx.h"
#include "Sphinx.h"
#include "SphinxPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CSphinxPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSphinxPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CSphinxPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CSphinxPropPage, "SPHINX.SphinxPropPage.1",
	0xc8bfb250, 0x466b, 0x11d1, 0xb8, 0xfc, 0, 0x60, 0x8, 0x16, 0x5b, 0x1e)


/////////////////////////////////////////////////////////////////////////////
// CSphinxPropPage::CSphinxPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CSphinxPropPage

BOOL CSphinxPropPage::CSphinxPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_SPHINX_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxPropPage::CSphinxPropPage - Constructor

CSphinxPropPage::CSphinxPropPage() :
	COlePropertyPage(IDD, IDS_SPHINX_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CSphinxPropPage)
	m_csArgFile = _T("");
	m_csLogFile = _T("");
	m_fIgnoreEmpty = FALSE;
	m_sps = 0;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxPropPage::DoDataExchange - Moves data between page and properties

void CSphinxPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSphinxPropPage)
	DDP_Text(pDX, IDC_ARGFILE_EDIT, m_csArgFile, _T("ArgFile") );
	DDX_Text(pDX, IDC_ARGFILE_EDIT, m_csArgFile);
	DDP_Text(pDX, IDC_LOGFILE_EDIT, m_csLogFile, _T("LogFile") );
	DDX_Text(pDX, IDC_LOGFILE_EDIT, m_csLogFile);
	DDP_Check(pDX, IDC_IGNORE_CHECK, m_fIgnoreEmpty, _T("IgnoreEmptyUtterance") );
	DDX_Check(pDX, IDC_IGNORE_CHECK, m_fIgnoreEmpty);
	DDP_Text(pDX, IDC_SPS_EDIT, m_sps, _T("SamplesPerSecond") );
	DDX_Text(pDX, IDC_SPS_EDIT, m_sps);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxPropPage message handlers
