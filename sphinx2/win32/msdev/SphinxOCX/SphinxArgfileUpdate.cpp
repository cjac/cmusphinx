// SphinxArgfileUpdate.cpp : implementation file
//

#include "stdafx.h"
#include "sphinx.h"
#include "SphinxArgfileUpdate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <io.h>

#include "assert.h"

/////////////////////////////////////////////////////////////////////////////
// CSphinxArgfileUpdate dialog

IMPLEMENT_DYNCREATE(CSphinxArgfileUpdate, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSphinxArgfileUpdate, COlePropertyPage)
	//{{AFX_MSG_MAP(CSphinxArgfileUpdate)
	ON_BN_CLICKED(IDC_DICTBROWSE, OnDictBrowse)
	ON_BN_CLICKED(IDC_LMBROWSE, OnLmBrowse)
	ON_BN_CLICKED(IDC_PHONEBROWSE, OnPhoneBrowse)
	ON_BN_CLICKED(IDC_MAPBROWSE, OnMapBrowse)
	ON_BN_CLICKED(IDC_HMMBROWSE, OnHmmBrowse)
	ON_BN_CLICKED(IDC_CLUSTERBROWSE, OnClusterBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

// {8CE8812C-1449-11D3-B693-00105A0F4D95}
IMPLEMENT_OLECREATE_EX(CSphinxArgfileUpdate, "Sphinx.CSphinxArgfileUpdate",
	0x8ce8812c, 0x1449, 0x11d3, 0xb6, 0x93, 0x0, 0x10, 0x5a, 0xf, 0x4d, 0x95)


/////////////////////////////////////////////////////////////////////////////
// CSphinxArgfileUpdate::CSphinxArgfileUpdateFactory::UpdateRegistry -
// Adds or removes system registry entries for CSphinxArgfileUpdate

BOOL CSphinxArgfileUpdate::CSphinxArgfileUpdateFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Define string resource for page type; replace '0' below with ID.

	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, 0);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxArgfileUpdate::CSphinxArgfileUpdate - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CSphinxArgfileUpdate::CSphinxArgfileUpdate() :
	COlePropertyPage(IDD, IDS_SPHINX_PPG_ARGFILE_UPDATE)
{
	//{{AFX_DATA_INIT(CSphinxArgfileUpdate)
	m_csArgFile = _T("");
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxArgfileUpdate::DoDataExchange - Moves data between page and properties

void CSphinxArgfileUpdate::DoDataExchange(CDataExchange* pDX)
{
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//{{AFX_DATA_MAP(CSphinxArgfileUpdate)
	DDX_Control(pDX, IDC_ARGFILEED, m_cedArgFile);
	DDX_Control(pDX, IDC_PHONEFILE, m_cedPhoneFile);
	DDX_Control(pDX, IDC_MAPFILE, m_cedMapFile);
	DDX_Control(pDX, IDC_HMMDIR, m_cedHmmDir);
	DDX_Control(pDX, IDC_CLUSTERFILE, m_cedClusterFile);
	DDX_Control(pDX, IDC_LMFILE, m_cedLmFile);
	DDX_Control(pDX, IDC_DICTFILE, m_cedDictFile);
	DDP_Text(pDX, IDC_ARGFILEED, m_csArgFile, _T("ArgFile") );
	DDX_Text(pDX, IDC_ARGFILEED, m_csArgFile);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);

	if ( IsModified() )
		UpdateArgfile();

}


/////////////////////////////////////////////////////////////////////////////
// CSphinxArgfileUpdate message handlers

///////////////////////////////////////////
// Util function to read the arg from a line
inline static CString GetArg( CString csLine ) 
{
	int loc;

	csLine.TrimLeft();
	csLine.TrimRight();

	loc = csLine.FindOneOf( " \t" );
	return csLine.Right( csLine.GetLength() - loc - 1 );
}



///////////////////////////////////////////
// On Init, load the params from the argfile
BOOL CSphinxArgfileUpdate::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();
		
	// we don't want apply to be the execution button here, 
	// so let's ignore changes to everything
/*
	IgnoreApply(  IDC_DICTFILE );
	IgnoreApply(  IDC_LMFILE );
	IgnoreApply(  IDC_PHONEFILE );
	IgnoreApply(  IDC_MAPFILE );
	IgnoreApply(  IDC_HMMDIR );
	IgnoreApply(  IDC_CLUSTERFILE );
*/	
	IgnoreApply(  IDC_DICTBROWSE );
	IgnoreApply(  IDC_LMBROWSE );
	IgnoreApply(  IDC_PHONEBROWSE );
	IgnoreApply(  IDC_MAPBROWSE );
	IgnoreApply(  IDC_HMMBROWSE );
	IgnoreApply(  IDC_CLUSTERBROWSE );
	

	//	load the parameters from the current arg file,
	//	and put them in the edit boxes
	CString	 csArgFile;
	CString  csMessage;
	CString	 csTemp;
	_TCHAR	 szTemp[8192];	 
	   
	FILE*	fileArg;

	m_cedArgFile.GetWindowText( csArgFile );

	fileArg = _tfopen( csArgFile, _T("r") );

	if (!fileArg) {
		csMessage = _T("Couldn't open ");
		csMessage += csArgFile;
		csMessage += " to get or change settings.";
		AfxMessageBox( csMessage );
		return 0;
	}

	// read in the argfile
	while (!feof( fileArg )) {
		_fgetts( szTemp, 8191, fileArg );
		if (_tcsstr( szTemp, _T("-dictfn") )) {
			m_cedDictFile.SetWindowText( GetArg( szTemp ) );

		} else if (_tcsstr( szTemp, _T("-lmfn") )) {
			m_cedLmFile.SetWindowText( GetArg( szTemp ) );

		} else if (_tcsstr( szTemp, _T("-phnfn") )) {
			m_cedPhoneFile.SetWindowText( GetArg( szTemp ) );

		} else if (_tcsstr( szTemp, _T("-mapfn") )) {
			m_cedMapFile.SetWindowText( GetArg( szTemp ) );

		} else if (_tcsstr( szTemp, _T("-hmmdir") )) {
			m_cedHmmDir.SetWindowText(  GetArg( szTemp ) );

		} else if (_tcsstr( szTemp, _T("-sendumpfn") )) {
			m_cedClusterFile.SetWindowText(  GetArg( szTemp ) );

		}
	}
	fclose( fileArg );


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


///////////////////////////////////////////
// Browse Buttons

// Generic function to fill in an edit box with a path
void CSphinxArgfileUpdate::OnBrowse(CString csFilter, CString csDescr, CEdit *cedBox) 
{
	int nTemp;
	CString csFile;
	char *szTemp;

	CFileDialog	   cmnFile( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						    csFilter, this );

	if ( cmnFile.DoModal() == IDOK ) {
		// Update the value of the dictionary name field with what was specified
		csFile = cmnFile.GetPathName();
		if ( _access( csFile, 04 ) < 0 ) {
			csDescr += " is not readable\n";
			AfxMessageBox( csDescr );
			OnBrowse( csFilter, csDescr, cedBox );
			return;
		}
		nTemp = GetShortPathName( csFile, NULL, 0 ) + 1;
		szTemp = (char*) malloc( nTemp );
		GetShortPathName( csFile, szTemp, nTemp );

		cedBox->SetWindowText( szTemp );
	}
}

void CSphinxArgfileUpdate::OnDictBrowse() 
{
	OnBrowse( _T("Dictionary Files (*.dic; *.dict)|*.dic; *.dict|All Files (*.*)|*.*||"),
			  _T("Dictionary file"),
			  &m_cedDictFile );
}

void CSphinxArgfileUpdate::OnLmBrowse() 
{
	OnBrowse( _T("Language Model Files (*.lm; *.arpabo)|*.lm; *.arpabo|All Files (*.*)|*.*||"),
			  _T("Language model file"),
			  &m_cedLmFile );
}

void CSphinxArgfileUpdate::OnPhoneBrowse() 
{
	OnBrowse( _T("Phone Files (*.phone)|*.phone|All Files (*.*)|*.*||"),
			  _T("Phone file"),
			  &m_cedPhoneFile );	
}

void CSphinxArgfileUpdate::OnMapBrowse() 
{
	OnBrowse( _T("Map Files (*.map)|*.map|All Files (*.*)|*.*||"),
			  _T("Map file"),
			  &m_cedMapFile );

}

void CSphinxArgfileUpdate::OnHmmBrowse() 
{

/* This should work only if we're Windows 95 capable, 
 * but who knows.	Check Visual C++/MFC FAQ for more 
 * info on this routine.
 */

	LPMALLOC pMalloc;
	/* Gets the Shell's default allocator */
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		BROWSEINFO bi;
		char pszBuffer[MAX_PATH];
		LPITEMIDLIST pidl;
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		bi.hwndOwner = GetSafeHwnd();
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = _T("Select a Starting Directory");
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
		bi.lpfn = NULL;
		bi.lParam = 0;
		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder( &bi )) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
			{ 
				// At this point pszBuffer contains the selected path */.
				m_cedHmmDir.SetWindowText( pszBuffer );
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		pMalloc->Release();
	}
}

void CSphinxArgfileUpdate::OnClusterBrowse() 
{
	OnBrowse( _T("Cluster Files (*.sen)|*.sen|All Files (*.*)|*.*||"),
			  _T("Cluster file"),
			  &m_cedClusterFile );

}

///////////////////////////////////////////
// Update

void CSphinxArgfileUpdate::UpdateArgfile() 
{	
	CString  csArgFile;
	CString  csMessage;
	CString	 csBackup;
	CString	 csArgData;
	CString	 csTemp;
	_TCHAR	 szTemp[8192];	 
	   
	FILE*	fileArg;

	m_cedArgFile.GetWindowText( csArgFile );

	// Alert the user to a a backup
	csBackup = csArgFile + _T(".bak");
	
	csMessage = _T("Modifying ");
	csMessage += csArgFile;
	csMessage += _T(". A backup will be saved as ");
	csMessage += csBackup;

	if (AfxMessageBox( csMessage, MB_OKCANCEL ) == IDOK) {
	
		// Do the backup
		CopyFile( csArgFile, csBackup, FALSE );
		fileArg = _tfopen( csArgFile, _T("r") );

		if (!fileArg) {
			csMessage = _T("Couldn't open ");
			csMessage += csArgFile;
			AfxMessageBox( csMessage );
			return;
		}

		// read in the argfile
		while (!feof( fileArg )) {
			_fgetts( szTemp, 8191, fileArg );
			if (_tcsstr( szTemp, _T("-dictfn") )) {
				m_cedDictFile.GetWindowText( csTemp );
				csArgData += _T(" -dictfn ") + csTemp + _T("\n");

			} else if (_tcsstr( szTemp, _T("-lmfn") )) {
				m_cedLmFile.GetWindowText( csTemp );
				csArgData += _T(" -lmfn ") + csTemp + _T("\n");

			} else if (_tcsstr( szTemp, _T("-phnfn") )) {
				m_cedPhoneFile.GetWindowText( csTemp );
				csArgData += _T(" -phnfn ") + csTemp + _T("\n");

			} else if (_tcsstr( szTemp, _T("-mapfn") )) {
				m_cedMapFile.GetWindowText( csTemp );
				csArgData += _T(" -mapfn ") + csTemp + _T("\n");

			} else if (_tcsstr( szTemp, _T("-hmmdirlist") )) {
				m_cedHmmDir.GetWindowText( csTemp );
				csArgData += _T(" -hmmdirlist ") + csTemp + _T("\n");
			
			} else if (_tcsstr( szTemp, _T("-hmmdir") )) {
				m_cedHmmDir.GetWindowText( csTemp );
				csArgData += _T(" -hmmdir ") + csTemp + _T("\n");
				
			} else if (_tcsstr( szTemp, _T("-cbdir") )) {
				m_cedHmmDir.GetWindowText( csTemp );
				csArgData += _T(" -cbdir ") + csTemp + _T("\n");


			} else if (_tcsstr( szTemp, _T("-sendumpfn") )) {
				m_cedClusterFile.GetWindowText( csTemp );
				csArgData += _T(" -sendumpfn ") + csTemp + _T("\n");

/*	
// forget about these two for now			
			} else if (_tcsstr( szTemp, _T("-matchfn") )) {


			} else if (_tcsstr( szTemp, _T("-kbdumpdir") )) {
				m_cedMapFile.GetWindowText( csTemp );
				csArgData += _T(" -kbdumpdir ") + csTemp + _T("\n");
*/			

			} else {
				csArgData += szTemp;
			}
		}
		fclose( fileArg );

		// print it out to the requested file
		fileArg = _tfopen( csArgFile, _T("w") );
		if (!fileArg) {
			csMessage = _T("Couldn't open ");
			csMessage += csArgFile;
			csMessage += _T(" for output.");
			AfxMessageBox( csMessage );
			return;
		}
		_ftprintf( fileArg, _T("%s"), csArgData );
		fclose( fileArg );  
	}
}
