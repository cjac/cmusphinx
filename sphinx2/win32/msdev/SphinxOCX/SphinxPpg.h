#if !defined(AFX_SPHINXPPG_H__C8BFB25F_466B_11D1_B8FC_006008165B1E__INCLUDED_)
#define AFX_SPHINXPPG_H__C8BFB25F_466B_11D1_B8FC_006008165B1E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SphinxPpg.h : Declaration of the CSphinxPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CSphinxPropPage : See SphinxPpg.cpp.cpp for implementation.

class CSphinxPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CSphinxPropPage)
	DECLARE_OLECREATE_EX(CSphinxPropPage)

// Constructor
public:
	CSphinxPropPage();

// Dialog Data
	//{{AFX_DATA(CSphinxPropPage)
	enum { IDD = IDD_PROPPAGE_SPHINX };
	CString	m_csArgFile;
	CString	m_csLogFile;
	BOOL	m_fIgnoreEmpty;
	UINT	m_sps;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CSphinxPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHINXPPG_H__C8BFB25F_466B_11D1_B8FC_006008165B1E__INCLUDED)
