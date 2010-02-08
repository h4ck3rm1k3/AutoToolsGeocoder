/********************************************************************
Copyright (C) 1998-2006 SRC, LLC

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License version 2.1 as published by the Free Software Foundation

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*********************************************************************/

/*
# $Rev: 40 $ 
# $Date: 2006-08-02 18:40:51 +0200 (Wed, 02 Aug 2006) $ 
*/

/////////////////////////////////////////////////////////////////////////////
// ChildView.h : interface of the CChildView class
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__4961EABF_713B_11D4_B1E7_00105A0FCDF8__INCLUDED_)
#define AFX_CHILDVIEW_H__4961EABF_713B_11D4_B1E7_00105A0FCDF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ot_tabwnd.h"			// stingray tab window
#include "layout\SplitWnd.h"	// stingray splitter window

#include "HierarchicalBrowse.h"
#include "PostcodeStreetBrowse.h"
#include "PostcodeCityBrowse.h"
#include "StateCityBrowse.h"
#include "StreetIntersectionBrowse.h"
#include "StreetIntersectionSoundexBrowse.h"
#include "StateCitySoundexBrowse.h"
#include "ParseAddressBrowse.h"
#include "CodeAddressBrowse.h"

/////////////////////////////////////////////////////////////////////////////
// CChildView window

class CChildView : public CWnd
{
public:
	// Construction
	CChildView();
	virtual ~CChildView();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	/////////////////////////////////////////////////////////////////////////////
	// Create all of the browser windows.
	/////////////////////////////////////////////////////////////////////////////
	void CreateBrowserTabs();

	SECSplitterWnd splitter;	// top-level splitter
	CEdit errorLog;				// error-log window
	SECTabWnd tabWindow;		// Tab window to hold browse panes

	enum { 
		LogTimerID = 100,
		SampleTimerID = 101
	};


	CHierarchicalBrowse* hierarchicalBrowser;
	CPostcodeStreetBrowse* zipStreetBrowser;
	CStateCitySoundexBrowse* stateCitySoundexBrowser;
	CPostcodeCityBrowse* zipCityBrowser;
	CStateCityBrowse* stateCityBrowser;
	CStreetIntersectionBrowse* streetIntersectionBrowser;
	CStreetIntersectionSoundexBrowse* streetIntersectionSoundexBrowser;
	CParseAddressBrowse* parseAddressBrowser;
	CCodeAddressBrowse* codeAddressBrowser;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif 

