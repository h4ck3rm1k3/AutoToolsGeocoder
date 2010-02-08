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

// ParseAddressBrowse.cpp : implementation of the CParseAddressBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "ParseAddressBrowse.h"
#include "Soundex.h"
#include "Registry.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParseAddressBrowse

CParseAddressBrowse::CParseAddressBrowse() :
	initialized(false)
{
	firstLineParser = new AddressParserFirstLine;
	lastLineParser = new AddressParserLastLine;

	TsString tableDir;
	if (Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\DataLever\\geobrowse\\TableDirectory", tableDir)) {
		const char* errorMsg;
		firstLineParser->Open(tableDir.c_str(), errorMsg);
		lastLineParser->Open(tableDir.c_str(), errorMsg);
	}
}

CParseAddressBrowse::~CParseAddressBrowse()
{
}


BEGIN_MESSAGE_MAP(CParseAddressBrowse,CWnd )
	//{{AFX_MSG_MAP(CParseAddressBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CParseAddressBrowse::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(&rect, RGB(192, 192, 192));
	
	// Do not call CWnd::OnPaint() for painting messages
}


/////////////////////////////////////////////////////////////////////////////
// Crate the window and all child windows.
/////////////////////////////////////////////////////////////////////////////
void CParseAddressBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// First line
	firstLineLabel.Create("First Line:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	firstLineEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, FirstLineEditID);

	// Last line
	lastLineLabel.Create("Last Line:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	lastLineEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, LastLineEditID);

	// Parse Button
	parseButton.Create("Parse", WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, ParseButtonID);

	// first line parse grid
	firstLineParseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		10
	);
	firstLineParseGrid.Initialize();
	firstLineParseGrid.SetColCount(NbrColumns);
	// Column headings
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, NbrColumn),
		CGXStyle().SetValue("Nbr")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, PredirColumn),
		CGXStyle().SetValue("Predir")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, PrefixColumn),
		CGXStyle().SetValue("Prefix")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, StreetColumn),
		CGXStyle().SetValue("Street")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, SuffixColumn),
		CGXStyle().SetValue("Suffix")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, PostdirColumn),
		CGXStyle().SetValue("Postdir")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, UnitDesColumn),
		CGXStyle().SetValue("UnitDes")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, UnitColumn),
		CGXStyle().SetValue("Unit")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, IntersectColumn),
		CGXStyle().SetValue("Intersect?")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, Street2Column),
		CGXStyle().SetValue("Street2")
	);
	firstLineParseGrid.SetStyleRange(
		CGXRange(0, ModsColumn),
		CGXStyle().SetValue("Mods")
	);
	firstLineParseGridLabel.Create("First Line Parse Candidates", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);


	// last line parse grid
	lastLineParseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		10
	);
	lastLineParseGrid.Initialize();
	lastLineParseGrid.SetColCount(5);
	// Column headings
	lastLineParseGrid.SetStyleRange(
		CGXRange(0, 1),
		CGXStyle().SetValue("City")
	);
	lastLineParseGrid.SetStyleRange(
		CGXRange(0, 2),
		CGXStyle().SetValue("State")
	);
	lastLineParseGrid.SetStyleRange(
		CGXRange(0, 3),
		CGXStyle().SetValue("Postcode")
	);
	lastLineParseGrid.SetStyleRange(
		CGXRange(0, 4),
		CGXStyle().SetValue("Pcode Ext")
	);
	lastLineParseGrid.SetStyleRange(
		CGXRange(0, 5),
		CGXStyle().SetValue("Mods")
	);
	lastLineParseGridLabel.Create("Last Line Parse Candidates", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CParseAddressBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CParseAddressBrowse::HandleSize(int cx, int cy) 
{
	// First Line
	firstLineLabel.MoveWindow(10, 10, 90, 20);
	firstLineEdit.MoveWindow(110, 10, 250, 20);

	// Last Line
	lastLineLabel.MoveWindow(10, 40, 90, 20);
	lastLineEdit.MoveWindow(110, 40, 250, 20);

	// Parse button
	parseButton.MoveWindow(375, 10, 100, 50);

	// First Line Grid
	firstLineParseGridLabel.MoveWindow(10, 80, 200, 20, TRUE);
	firstLineParseGrid.MoveWindow(10, 110, cx - 20, (cy - 200) / 2, TRUE);

	// Last Line Grid
	lastLineParseGridLabel.MoveWindow(10, 130 + (cy - 200) / 2, 200, 20, TRUE);
	lastLineParseGrid.MoveWindow(10, 160 + (cy - 200) / 2, cx - 20, (cy - 200) / 2, TRUE);
}


BOOL CParseAddressBrowse::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nID == ParseButtonID) {
		UpdateViews();
	}
	return 0;
	// return CWnd ::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CParseAddressBrowse::UpdateViews()
{
	if (!initialized) {
		return;
	}

	// Clear results
	firstLineParseGrid.SetRowCount(0);
	lastLineParseGrid.SetRowCount(0);

	CString firstLineStr;
	firstLineEdit.GetWindowText(firstLineStr);

	// Parse first line
	AddressParserFirstLine::ParseCandidate firstLineParseCandidate;
	bool ok = firstLineParser->Parse((const char*)firstLineStr, firstLineParseCandidate, true);
	firstLineParser->PermuteAddress(~0);
	for (; ok; ok = firstLineParser->NextAddressPermutation(firstLineParseCandidate, true)) {
		// Set parse results to row
		firstLineParseGrid.SetRowCount(firstLineParseGrid.GetRowCount() + 1);
		int row = firstLineParseGrid.GetRowCount();

		firstLineParseGrid.SetStyleRange(
			CGXRange(row, NbrColumn),
			CGXStyle().SetValue(firstLineParseCandidate.number)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, PredirColumn),
			CGXStyle().SetValue(firstLineParseCandidate.predir)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, PrefixColumn),
			CGXStyle().SetValue(firstLineParseCandidate.prefix)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, StreetColumn),
			CGXStyle().SetValue(firstLineParseCandidate.street)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, SuffixColumn),
			CGXStyle().SetValue(firstLineParseCandidate.suffix)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, PostdirColumn),
			CGXStyle().SetValue(firstLineParseCandidate.postdir)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, UnitDesColumn),
			CGXStyle().SetValue(firstLineParseCandidate.unitDesignator)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, UnitColumn),
			CGXStyle().SetValue(firstLineParseCandidate.unitNumber)
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, IntersectColumn),
			CGXStyle().SetValue(firstLineParseCandidate.isIntersection ? "Yes" : "No")
		);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, Street2Column),
			CGXStyle().SetValue(firstLineParseCandidate.street2)
		);
		char buf[20];
		sprintf(buf, "%d", firstLineParseCandidate.numberOfMods);
		firstLineParseGrid.SetStyleRange(
			CGXRange(row, ModsColumn),
			CGXStyle().SetValue(buf)
		);
	}


	CString lastLineStr;
	lastLineEdit.GetWindowText(lastLineStr);

	// Parse last line
	AddressParserLastLine::ParseCandidate lastLineParseCandidate;

	ok = lastLineParser->Parse((const char*)lastLineStr, lastLineParseCandidate, true);
	lastLineParser->PermuteAddress(~0);
	for (;ok; ok = lastLineParser->NextAddressPermutation(lastLineParseCandidate, true)) {
		// Set parse results to row
		lastLineParseGrid.SetRowCount(lastLineParseGrid.GetRowCount() + 1);
		int row = lastLineParseGrid.GetRowCount();

		lastLineParseGrid.SetStyleRange(
			CGXRange(row, 1),
			CGXStyle().SetValue(lastLineParseCandidate.city)
		);
		lastLineParseGrid.SetStyleRange(
			CGXRange(row, 2),
			CGXStyle().SetValue(lastLineParseCandidate.state)
		);
		lastLineParseGrid.SetStyleRange(
			CGXRange(row, 3),
			CGXStyle().SetValue(lastLineParseCandidate.postcode)
		);
		lastLineParseGrid.SetStyleRange(
			CGXRange(row, 4),
			CGXStyle().SetValue(lastLineParseCandidate.postcodeExt)
		);

		char buf[20];
		sprintf(buf, "%d", lastLineParseCandidate.numberOfMods);
		lastLineParseGrid.SetStyleRange(
			CGXRange(row, 5),
			CGXStyle().SetValue(buf)
		);
	}

}

