#if !defined(AFX_SPHINXARGFILEUPDATE_H__8CE8812D_1449_11D3_B693_00105A0F4D95__INCLUDED_)
#define AFX_SPHINXARGFILEUPDATE_H__8CE8812D_1449_11D3_B693_00105A0F4D95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SphinxArgfileUpdate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSphinxArgfileUpdate : Property page dialog

class CSphinxArgfileUpdate : public COlePropertyPage
{
	DECLARE_DYNCREATE(CSphinxArgfileUpdate)
	DECLARE_OLECREATE_EX(CSphinxArgfileUpdate)

// Constructors
public:
	CSphinxArgfileUpdate();

// Dialog Data
	//{{AFX_DATA(CSphinxArgfileUpdate)
	enum { IDD = IDD_ARGFILEUPDATE_SPHINX };
	CEdit	m_cedArgFile;
	CEdit	m_cedPhoneFile;
	CEdit	m_cedMapFile;
	CEdit	m_cedHmmDir;
	CEdit	m_cedClusterFile;
	CEdit	m_cedLmFile;
	CEdit	m_cedDictFile;
	CString	m_csArgFile;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

	void	UpdateArgfile();
	void	OnBrowse(CString , CString , CEdit *);
	
// Message maps
protected:
	//{{AFX_MSG(CSphinxArgfileUpdate)
	afx_msg void OnDictBrowse();
	afx_msg void OnLmBrowse();
	afx_msg void OnPhoneBrowse();
	afx_msg void OnMapBrowse();
	afx_msg void OnHmmBrowse();
	afx_msg void OnClusterBrowse();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHINXARGFILEUPDATE_H__8CE8812D_1449_11D3_B693_00105A0F4D95__INCLUDED_)
