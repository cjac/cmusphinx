#if !defined(AFX_SPHINXBUILDLM_H__1C28CED9_4BDB_11D1_B901_006008165B1E__INCLUDED_)
#define AFX_SPHINXBUILDLM_H__1C28CED9_4BDB_11D1_B901_006008165B1E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SphinxBuildLm.h : header file
//

#define	BUILDLM_TYPE_DICTIONARY 1
#define	BUILDLM_TYPE_NGRAMS 2

/////////////////////////////////////////////////////////////////////////////
// CSphinxBuildLm : Property page dialog

class CSphinxBuildLm : public COlePropertyPage
{
	DECLARE_DYNCREATE(CSphinxBuildLm)
	DECLARE_OLECREATE_EX(CSphinxBuildLm)

// Constructors
public:
	CSphinxBuildLm();

// Dialog Data
	//{{AFX_DATA(CSphinxBuildLm)
	enum { IDD = IDD_BUILDLM_SPHINX };
	CString	m_csArgFile;
	CString	m_csDictionaryEdit;
	CString	m_csEdit;
	CString	m_csNGramEdit;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CSphinxBuildLm)
	afx_msg void OnChangeDictionaryEdit();
	afx_msg void OnChangeEdit();
	afx_msg void OnChangeNgramEdit();
	afx_msg void OnSaveDictButton();
	afx_msg void OnSaveNgramsButton();
	afx_msg void OnUseButton();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// SDH added
protected:
	bool BuildLM( CString csCorpus, CString csURL, int lType );

	bool	m_fDictionaryBuilt;
	bool	m_fNGramsBuilt;
	bool	m_fDictionarySaved;
	bool	m_fNGramsSaved;

	CString m_csNGrams;
	CString	m_csDictionary;
	CString m_csNGramFile;
	CString m_csDictFile;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHINXBUILDLM_H__1C28CED9_4BDB_11D1_B901_006008165B1E__INCLUDED_)
