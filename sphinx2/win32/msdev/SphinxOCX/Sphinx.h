#if !defined(AFX_SPHINX_H__C8BFB255_466B_11D1_B8FC_006008165B1E__INCLUDED_)
#define AFX_SPHINX_H__C8BFB255_466B_11D1_B8FC_006008165B1E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Sphinx.h : main header file for SPHINX.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSphinxApp : See Sphinx.cpp for implementation.

class CSphinxApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHINX_H__C8BFB255_466B_11D1_B8FC_006008165B1E__INCLUDED)
