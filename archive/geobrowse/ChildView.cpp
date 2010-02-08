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

// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView() :
	hierarchicalBrowser(0),
	zipStreetBrowser(0),
	zipCityBrowser(0),
	stateCitySoundexBrowser(0),
	stateCityBrowser(0),
	streetIntersectionBrowser(0),
	streetIntersectionSoundexBrowser(0),
	parseAddressBrowser(0),
	codeAddressBrowser(0)
{
}

CChildView::~CChildView()
{
	delete hierarchicalBrowser;
	delete zipStreetBrowser;
	delete zipCityBrowser;
	delete stateCityBrowser;
	delete stateCitySoundexBrowser;
	delete streetIntersectionBrowser;
	delete streetIntersectionSoundexBrowser;
	delete parseAddressBrowser;
	delete codeAddressBrowser;
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers
/////////////////////////////////////////////////////////////////////////////
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}


/////////////////////////////////////////////////////////////////////////////
// Crate the window and all child windows.
/////////////////////////////////////////////////////////////////////////////
BOOL CChildView::Create(
	LPCTSTR lpszClassName, 
	LPCTSTR lpszWindowName, 
	DWORD dwStyle, 
	const RECT& rect, 
	CWnd* pParentWnd, 
	UINT nID, 
	CCreateContext* pContext
) {
	if (!CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext)) {
		return FALSE;
	}

	// Top-level splitter
	splitter.Create(this);

	// Tab control in top for browsing windows.
	tabWindow.Create(
		&splitter, 
		WS_CHILD | WS_VISIBLE | TWS_LEFTRIGHTSCROLL| TWS_TABS_ON_BOTTOM,
		1
	);
	splitter.AddPane(0, 0, &tabWindow);

	// Edit control in the bottom for messages.
	errorLog.Create(
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		CRect(0,0,0,0), 
		&splitter, 
		2
	);
	splitter.AddPane(1, 0, &errorLog);

	splitter.SetSplitterPositions(0, 200);

	SetTimer(LogTimerID, 1000, 0);
	SetTimer(SampleTimerID, 1000, 0);

	return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (splitter.m_hWnd != 0) {
		splitter.MoveWindow(0, 0, cx, cy, TRUE);

		int colPos, rowPos;
		splitter.GetSplitterPositions(colPos, rowPos);
		if (rowPos < 10) {
			splitter.SetSplitterPositions(0, 3 * cy / 4);
		}
	}
}

void CChildView::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == LogTimerID) {
		while (!theApp.notifier->messages.empty()) {
			TsString msg = TsString("\r\n") + theApp.notifier->messages.front();
			theApp.notifier->messages.pop_front();
			errorLog.SetSel(errorLog.GetWindowTextLength(), errorLog.GetWindowTextLength());
			errorLog.ReplaceSel(msg.c_str());
		}
	} else if (nIDEvent == SampleTimerID) {
		if (theApp.refQueryFile != 0 && theApp.refQueryFile->IsOpen()) {
			KillTimer(SampleTimerID);
			CreateBrowserTabs();
		}
	}
	CWnd ::OnTimer(nIDEvent);
}


/////////////////////////////////////////////////////////////////////////////
// Create all of the browser windows.
/////////////////////////////////////////////////////////////////////////////
void CChildView::CreateBrowserTabs()
{
	// Adjust the tab fonts
	LOGFONT logFont;
	CFont newFont;
	CFont* tabFont = tabWindow.GetFontUnselectedTab();
	tabFont->GetLogFont(&logFont);
	int size;
	if (logFont.lfHeight > 0) {
		size = logFont.lfHeight + 5;
	} else if (logFont.lfHeight < 0) {
		size = logFont.lfHeight - 5;
	} else {
		size = 15;
	}
	logFont.lfHeight = size;
	newFont.CreateFontIndirect(&logFont);
	tabWindow.SetFontUnselectedTab(&newFont, FALSE);
	tabWindow.SetFontSelectedTab(&newFont, FALSE);
	tabWindow.SetFontActiveTab(&newFont, FALSE);

	// Default hierarchical browser.
	hierarchicalBrowser = new CHierarchicalBrowse(theApp.refQueryFile);
	hierarchicalBrowser->Create(
		0,
		"Browser",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		1
	);
	tabWindow.AddTab(
		hierarchicalBrowser,
		"Browser"
	);
	hierarchicalBrowser->Initialize();

	// Street browser
	zipStreetBrowser = new CPostcodeStreetBrowse(theApp.refQueryFile);
	zipStreetBrowser->Create(
		0,
		"Fa/Street",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		2
	);
	tabWindow.AddTab(
		zipStreetBrowser,
		"Fa/Street"
	);
	zipStreetBrowser->Initialize();

	// City by Postcode browser
	zipCityBrowser = new CPostcodeCityBrowse(theApp.refQueryFile);
	zipCityBrowser->Create(
		0,
		"City by Postcode",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		3
	);
	tabWindow.AddTab(
		zipCityBrowser,
		"City by Postcode"
	);
	zipCityBrowser->Initialize();

	// City by state/city browser
	stateCityBrowser = new CStateCityBrowse(theApp.refQueryFile);
	stateCityBrowser->Create(
		0,
		"City by State/City",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		4
	);
	tabWindow.AddTab(
		stateCityBrowser,
		"City by State/City"
	);
	stateCityBrowser->Initialize();

	// State/City Soundex browser
	stateCitySoundexBrowser = new CStateCitySoundexBrowse(theApp.refQueryFile);
	stateCitySoundexBrowser->Create(
		0,
		"State/City Soundex",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		5
	);
	tabWindow.AddTab(
		stateCitySoundexBrowser,
		"State/City Soundex"
	);
	stateCitySoundexBrowser->Initialize();



	// Street Intersection browser
	streetIntersectionBrowser = new CStreetIntersectionBrowse(theApp.refQueryFile);
	streetIntersectionBrowser->Create(
		0,
		"Street Intersection Query",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		6
	);
	tabWindow.AddTab(
		streetIntersectionBrowser,
		"Street Intersection Query"
	);
	streetIntersectionBrowser->Initialize();

	// Street IntersectionSoundex browser
	streetIntersectionSoundexBrowser = new CStreetIntersectionSoundexBrowse(theApp.refQueryFile);
	streetIntersectionSoundexBrowser->Create(
		0,
		"Street Intersection Soundex",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		7
	);
	tabWindow.AddTab(
		streetIntersectionSoundexBrowser,
		"Street Intersection Soundex"
	);
	streetIntersectionSoundexBrowser->Initialize();

	// Parse address browser
	parseAddressBrowser = new CParseAddressBrowse();
	parseAddressBrowser->Create(
		0,
		"Address Parser",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		8
	);
	tabWindow.AddTab(
		parseAddressBrowser,
		"Address Parser"
	);
	parseAddressBrowser->Initialize();


	// Code address browser
	codeAddressBrowser = new CCodeAddressBrowse(theApp.refQueryFile);
	codeAddressBrowser->Create(
		0,
		"Address Coder",
		WS_VISIBLE | WS_CHILD,
		CRect(0,0,0,0),
		&tabWindow,
		9
	);
	tabWindow.AddTab(
		codeAddressBrowser,
		"Address Coder"
	);
	codeAddressBrowser->Initialize();

	// Show hierarchical browser
	tabWindow.ActivateTab(0);
}


