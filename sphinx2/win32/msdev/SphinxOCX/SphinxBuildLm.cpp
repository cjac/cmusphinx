// SphinxBuildLm.cpp : implementation file
//

#include "stdafx.h"
#include "Sphinx.h"
#include "SphinxBuildLm.h"

#include <fstream.h>
#include <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSphinxBuildLm dialog

IMPLEMENT_DYNCREATE(CSphinxBuildLm, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSphinxBuildLm, COlePropertyPage)
	//{{AFX_MSG_MAP(CSphinxBuildLm)
	ON_EN_CHANGE(IDC_DICTIONARY_EDIT, OnChangeDictionaryEdit)
	ON_EN_CHANGE(IDC_EDIT, OnChangeEdit)
	ON_EN_CHANGE(IDC_NGRAM_EDIT, OnChangeNgramEdit)
	ON_BN_CLICKED(IDC_SAVE_DICT_BUTTON, OnSaveDictButton)
	ON_BN_CLICKED(IDC_SAVE_NGRAMS_BUTTON, OnSaveNgramsButton)
	ON_BN_CLICKED(IDC_USE_BUTTON, OnUseButton)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

// {1C28CED8-4BDB-11D1-B901-006008165B1E}
IMPLEMENT_OLECREATE_EX(CSphinxBuildLm, "Sphinx.CSphinxBuildLm",
	0x1c28ced8, 0x4bdb, 0x11d1, 0xb9, 0x1, 0x0, 0x60, 0x8, 0x16, 0x5b, 0x1e)


/////////////////////////////////////////////////////////////////////////////
// CSphinxBuildLm::CSphinxBuildLmFactory::UpdateRegistry -
// Adds or removes system registry entries for CSphinxBuildLm

BOOL CSphinxBuildLm::CSphinxBuildLmFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Define string resource for page type; replace '0' below with ID.

	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, 0);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxBuildLm::CSphinxBuildLm - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CSphinxBuildLm::CSphinxBuildLm() :
	COlePropertyPage(IDD, IDS_SPHINX_BUILDLM_CAPTION)
{
	//{{AFX_DATA_INIT(CSphinxBuildLm)
	m_csArgFile = _T("");
	m_csDictionaryEdit = _T("");
	m_csEdit = _T("");
	m_csNGramEdit = _T("");
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxBuildLm::DoDataExchange - Moves data between page and properties

void CSphinxBuildLm::DoDataExchange(CDataExchange* pDX)
{
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//{{AFX_DATA_MAP(CSphinxBuildLm)
	DDP_Text(pDX, IDC_ARG_FILE, m_csArgFile, _T("ArgFile") );
	DDX_Text(pDX, IDC_ARG_FILE, m_csArgFile);
	DDX_Text(pDX, IDC_DICTIONARY_EDIT, m_csDictionaryEdit);
	DDX_Text(pDX, IDC_EDIT, m_csEdit);
	DDX_Text(pDX, IDC_NGRAM_EDIT, m_csNGramEdit);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


// adds <s> and </s>, also strips out most punctuation,
// turns into upper case

inline _TCHAR* MungeString( LPCTSTR szString ) 
{
	int   lLen = _tcslen( szString );
	_TCHAR* szNew = new _TCHAR[lLen + 32];
	int	  i;
	int   j;

	_tcscpy( szNew, _T("<s> ") );
	j = 4;

	for (i = 0; i < lLen; i++) {
		if (isalpha( szString[i] )) 
			szNew[j++] = _toupper( szString[i] );
		else if (szString[i] == ' ')
			szNew[j++] = ' ';
		else if (szString[i] == '_')
			szNew[j++] = '_';
	}
	szNew[j] = '\0';
	
	_tcscat( szNew, _T(" </s> \n ") );
	
	return szNew;
}

CString CGIify( CString csIn, LPCTSTR szSpaceReplace )
{
	int     lLen = csIn.GetLength();
	int     i;
	CString csOut;
	_TCHAR	szTemp[64];

	for (i = 0; i < lLen; i++) {
		if (_istalpha( csIn[i] )) 
			csOut += csIn[i];
		else if (csIn[i] == _T(' '))
			csOut += szSpaceReplace;
		else {
			if (csIn[i] < 0x10) 
				_stprintf( szTemp, _T("%%0%X"), csIn[i] );
			else
				_stprintf( szTemp, _T("%%%X"), csIn[i] );
			csOut += szTemp;
		}
	}

	return csOut;
}

// this also gets rid of <s> and </s>
CString RemoveDuplicates( CString csIn )
{
	int		 lWords = 0;
	int		 lLen = csIn.GetLength();
	int		 i;
	int		 j;
	int		 k;
	CString* pcsWordList;
	CString	 csOut;
	CString	 csLast;
	bool	 fGood;

	for (i = 0; i < lLen; i++) {
		if ((csIn[i] == ' ') || (csIn[i] == '\n') || (csIn[i] == '\t'))
			lWords++;
	}

	pcsWordList = new CString[lWords];

	for (i = 0, j = 0, k = 0; i < lLen; i++) {
		if ((csIn[i] == ' ') || (csIn[i] == '\n') || (csIn[i] == '\t')) {
			pcsWordList[k] = csIn.Mid( j, (i - j) );			
			j = i;
			pcsWordList[k].TrimRight();
			pcsWordList[k].TrimLeft();
			k++;
		}
	}

	csLast.Empty();
	for (i = 0; i < lWords; i++) {
		if ((pcsWordList[i] == "<s>") || (pcsWordList[i] == "</s>"))
			continue;
		else {
			fGood = true;
			for (j = i + 1; j < lWords; j++) {
				if (pcsWordList[i] == pcsWordList[j]) {
					fGood = false;
					break;
				}
			}
			if (fGood) {
				csOut += pcsWordList[i];
				csOut += ' ';
				csLast = pcsWordList[i];
			}
		}
	}
	csOut.TrimLeft();
	csOut.TrimRight();

	return csOut;
}

bool CSphinxBuildLm::BuildLM( CString csCorpus, CString csURL, int lType )
{
	int				 lLineCount = 0;
	int				 lLen = csCorpus.GetLength();
	int				 i;
	ofstream		 ofOut;
	ifstream		 ifIn;
	_TCHAR**		 pszList;
	_TCHAR			 szTemp[8192];
	CString			 csResult;
	CString			 csList;
	CString			 csRequest;
	CString			 csHeader = "Content-Type: application/x-www-form-urlencoded\r\nUser-Agent: BuildLM\r\n";
	CString			 csServer;
	CString			 csObject;
	DWORD			 lServiceType;
	DWORD			 lRet;
	INTERNET_PORT	 nPort;
	CInternetSession isSession;
	CHttpConnection* phtConnection;
	CHttpFile*		 phfPost;
	CWaitCursor		 wait;

	for (i = 0; i < lLen; i++) {
		if (csCorpus[i] == '\n') 
			lLineCount++;
	}
	if (csCorpus[lLen - 1] != '\n')
		lLineCount++;

	pszList = new _TCHAR*[lLineCount];
	for (i = 0; i < lLineCount; i++) {
		_stscanf( csCorpus, _T("%[^\n]"), szTemp );
		csCorpus = csCorpus.Mid( csCorpus.Find( _T('\n') ) + 1);
		csList += MungeString( szTemp );
	}
	
	if (lType == BUILDLM_TYPE_NGRAMS) {
		csRequest = CGIify( csList, _T("+") );
		csRequest = _T("CORPUS=") + csRequest;
		csRequest += _T("+%0A&DISCOUNT=0.5\r\n");
	}
	else if (lType == BUILDLM_TYPE_DICTIONARY) {
		csRequest = RemoveDuplicates( csList );
		csRequest = CGIify( csRequest, _T("%0A") );
		csRequest = _T("WORDS=") + csRequest;
		csRequest += _T("%0A");
	}
	else {
		AfxMessageBox( _T("Invalid lType in BuildLM") );
		return false;
	}

	if (!AfxParseURL( csURL, lServiceType, csServer, csObject, nPort )) {
		_stprintf( szTemp, _T("Could not understand URL: %s"), csURL );
		AfxMessageBox( szTemp );
		return false;
	}
	if (lServiceType != AFX_INET_SERVICE_HTTP) {
		csURL = _T("http://") + csURL;
		if (!AfxParseURL( csURL, lServiceType, csServer, csObject, nPort ) || (lServiceType != AFX_INET_SERVICE_HTTP)) {
			AfxMessageBox( _T("Sorry, HTTP is the only supported protocol at this time.") );
			return false;
		}
	}
	phtConnection = isSession.GetHttpConnection( csServer, nPort );
	phfPost = phtConnection->OpenRequest( CHttpConnection::HTTP_VERB_POST, csObject );
	phfPost->SendRequest( csHeader, (void*) (LPCTSTR) csRequest, csRequest.GetLength() );

	phfPost->QueryInfoStatusCode( lRet );
	if (lRet == HTTP_STATUS_DENIED)
	{
		DWORD dwPrompt;
		dwPrompt = phfPost->ErrorDlg( NULL, ERROR_INTERNET_INCORRECT_PASSWORD,
									  FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
									  NULL );

		// if the user cancelled the dialog, bail out

		if (dwPrompt != ERROR_INTERNET_FORCE_RETRY)
		{
			AfxMessageBox( _T("Access denied: Invalid password") );
			return false;
		}

		phfPost->SendRequest( csHeader, (void*) (LPCTSTR) csRequest, csRequest.GetLength() );
		phfPost->QueryInfoStatusCode( lRet );
	}

	CString strNewLocation;
	phfPost->QueryInfo( HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation );

	// were we redirected?
	// these response status codes come from WININET.H

	if (lRet == HTTP_STATUS_MOVED ||
		lRet == HTTP_STATUS_REDIRECT ||
		lRet == HTTP_STATUS_REDIRECT_METHOD) 
	{
		CString strNewLocation;
		phfPost->QueryInfo( HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation );

		int nPlace = strNewLocation.Find( _T("Location: ") );
		if (nPlace == -1) {
			AfxMessageBox( _T("The site has moved and left no forwarding address.") );
			return false;
		}

		strNewLocation = strNewLocation.Mid( nPlace + 10 );
		nPlace = strNewLocation.Find( _T('\n') );
		if (nPlace > 0)
			strNewLocation = strNewLocation.Left( nPlace );

		// close up the redirected site

		//pFile->Close();
		//delete pFile;
		//pServer->Close();
		//delete pServer;

		// figure out what the old place was
		if (!AfxParseURL(strNewLocation, lServiceType, csServer, csObject, nPort))
		{
			_stprintf( szTemp, _T("Error: redirected to invalid URL: %s"), strNewLocation );
			AfxMessageBox( szTemp );
			return false;
		}

		_stprintf( szTemp, _T("Caution: redirected to %s"), strNewLocation );
		AfxMessageBox( szTemp );

		if (lServiceType != INTERNET_SERVICE_HTTP)
		{
			_stprintf( szTemp, _T("Error: redirected to non-HTTP URL: %s"), strNewLocation );
			AfxMessageBox( szTemp );
			return false;
		}

		// try again at the new location
		phtConnection = isSession.GetHttpConnection( csServer, nPort);
		phfPost = phtConnection->OpenRequest( CHttpConnection::HTTP_VERB_POST, csObject );
		phfPost->SendRequest( csHeader, (void*) (LPCTSTR) csRequest, csRequest.GetLength() );

		phfPost->QueryInfoStatusCode( lRet );
		if ( lRet != HTTP_STATUS_OK )
		{
			_stprintf( szTemp, _T("HTTP Error -- got status code %d"), lRet );
			return false;
		}
	}

	csResult.Empty();
	while (phfPost->ReadString( szTemp, 8191 )) {
		csResult += szTemp;
		if (szTemp[_tcslen( szTemp ) - 1] != _T('\n'))
			csResult += _T('\n');
	}

	if (lType == BUILDLM_TYPE_NGRAMS) {
		m_fNGramsBuilt = true;
		m_csNGrams = csResult;
	}
	else if (lType == BUILDLM_TYPE_DICTIONARY) {
		m_fDictionaryBuilt = true;
		m_csDictionary = csResult;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CSphinxBuildLm message handlers

void CSphinxBuildLm::OnChangeDictionaryEdit() 
{
	// TODO: Add your control notification handler code here
	
	m_fDictionaryBuilt = false;
	m_fNGramsBuilt = false;
	m_fDictionarySaved = false;
	m_fNGramsSaved = false;
}

void CSphinxBuildLm::OnChangeEdit() 
{
	// TODO: Add your control notification handler code here
	
	m_fDictionaryBuilt = false;
	m_fNGramsBuilt = false;
	m_fDictionarySaved = false;
	m_fNGramsSaved = false;
}

void CSphinxBuildLm::OnChangeNgramEdit() 
{
	// TODO: Add your control notification handler code here
	
	m_fDictionaryBuilt = false;
	m_fNGramsBuilt = false;
	m_fDictionarySaved = false;
	m_fNGramsSaved = false;
}

void CSphinxBuildLm::OnSaveDictButton() 
{
	// TODO: Add your control notification handler code here
	
	static CString csFileName;
	CFileDialog	   cmnFile( FALSE, _T(".dic"), csFileName, NULL, 
						    _T("Dictionary Files (*.dic; *.dict)|*.dic; *.dict|All Files (*.*)|*.*||"),
						    this );
//	ofstream	   ofOut;
	FILE*		   fileOut;
	_TCHAR		   szTemp[4096];

	UpdateData( TRUE );
	if (!m_fDictionaryBuilt) {
		if (!BuildLM( m_csEdit, m_csDictionaryEdit, BUILDLM_TYPE_DICTIONARY ))
			return;
	}
	
	if (cmnFile.DoModal() == IDOK) {
		csFileName = cmnFile.GetPathName();
		
		//ofOut.open( UniToAscii( csFileName ) );
		//ofOut << m_csDictionary;
		//ofOut.close();

		fileOut = _tfopen( csFileName, _T("w") );
		_ftprintf( fileOut, _T("%s"), m_csDictionary );
		fclose( fileOut );

		GetShortPathName( csFileName, szTemp, 4095 );
		m_csDictFile = szTemp;

		m_fDictionarySaved = true;
	}
}

void CSphinxBuildLm::OnSaveNgramsButton() 
{
	// TODO: Add your control notification handler code here

	static CString	csFileName;
	CFileDialog		cmnFile( FALSE, _T(".lm"), csFileName, NULL, 
						     _T("N-Gram Files (*.tg; *.bg; *.lm)|*.tg; *.bg; *.lm|All Files (*.*)|*.*||"),
						     this );
	FILE*			fileOut;
	_TCHAR			szTemp[4096];

	UpdateData( TRUE );
	if (!m_fNGramsBuilt) {
		if (!BuildLM( m_csEdit, m_csNGramEdit, BUILDLM_TYPE_NGRAMS ))
			return;
	}
	
	if (cmnFile.DoModal() == IDOK) {
		csFileName = cmnFile.GetPathName();
		
//		ofOut.open( UniToAscii( csFileName ) );
//		ofOut << m_csNGrams;
//		ofOut.close();

		fileOut = _tfopen( csFileName, _T("w") );
		_ftprintf( fileOut, _T("%s"), m_csNGrams );
		fclose( fileOut );

		GetShortPathName( csFileName, szTemp, 4095 );
		m_csNGramFile = szTemp;

		m_fNGramsSaved = true;
	}
}

void CSphinxBuildLm::OnUseButton() 
{
	// TODO: Add your control notification handler code here
	
	CString  csMessage;
	CString	 csBackup;
	CString	 csArgFile;
	_TCHAR	 szTemp[8192];	 
	   
	//ifstream ifArgFile;
	//ofstream ofArgFile;
	FILE*	fileArg;

	if (!m_fDictionarySaved) {
		OnSaveDictButton();
		if (!m_fDictionarySaved)
			return;
	}

	if (!m_fNGramsSaved) {
		OnSaveNgramsButton();
		if (!m_fNGramsSaved)
			return;
	}

	UpdateData( TRUE );

	csBackup = m_csArgFile + _T(".bak");
	
	csMessage = _T("Modifying ");
	csMessage += m_csArgFile;
	csMessage += _T(". A backup will be saved as ");
	csMessage += csBackup;

	if (AfxMessageBox( csMessage, MB_OKCANCEL ) == IDOK) {
		CopyFile( m_csArgFile, csBackup, FALSE );

		//ifArgFile.open( UniToAscii( m_csArgFile ) );
		fileArg = _tfopen( m_csArgFile, _T("r") );

		if (!fileArg) {
			csMessage = _T("Couldn't open ");
			csMessage += m_csArgFile;
			AfxMessageBox( csMessage );
			return;
		}

		while (!feof( fileArg )) {
			//getline( szTemp, 8191 );			
			_fgetts( szTemp, 8191, fileArg );
			if (_tcsstr( szTemp, _T("-dictfn") )) 
				csArgFile += _T(" -dictfn ") + m_csDictFile;
			else if (_tcsstr( szTemp, _T("-lmfn") ))
				csArgFile += _T(" -lmfn ") + m_csNGramFile;
			else
				csArgFile += szTemp;
				csArgFile += _T("\n");
		}
		//ifArgFile.close();
		fclose( fileArg );
		
//		ofArgFile.open( m_csArgFile );
//		ofArgFile.write( csArgFile, csArgFile.GetLength() );
//		ofArgFile.close();

		fileArg = _tfopen( m_csArgFile, _T("w") );
		_ftprintf( fileArg, _T("%s"), csArgFile );
	}
}

int CSphinxBuildLm::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COlePropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_fDictionaryBuilt = false;
	m_fNGramsBuilt = false;
	m_fDictionarySaved = false;
	m_fNGramsSaved = false;

	m_csDictionaryEdit = _T("http://alf9.speech.cs.cmu.edu:8044/cgi-bin/cgi-pronounce");
	m_csNGramEdit = _T("http://alf9.speech.cs.cmu.edu:8044/cgi-bin/make_lm");
	
	return 0;
}
