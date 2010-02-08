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

// CodeAddressBrowse.cpp : implementation of the CCodeAddressBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "CodeAddressBrowse.h"
#include "Soundex.h"
#include "Registry.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodeAddressBrowse

CCodeAddressBrowse::CCodeAddressBrowse(
	QueryImpRef refQueryFileRef_
) :
	initialized(false),
	refQueryFileRef(refQueryFileRef_)
{
}

CCodeAddressBrowse::~CCodeAddressBrowse()
{
	delete geocoder;
}


BEGIN_MESSAGE_MAP(CCodeAddressBrowse,CWnd )
	//{{AFX_MSG_MAP(CCodeAddressBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CCodeAddressBrowse::OnPaint() 
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
void CCodeAddressBrowse::Initialize()
{
	TsString tableDir;
	TsString databaseDir;
	Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\DataLever\\geobrowse\\TableDirectory", tableDir);
	Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\DataLever\\geobrowse\\DatabaseDirectory", databaseDir);
	geocoder = new Geocoder_Notify(tableDir, databaseDir);
	if (!geocoder->Open()) {
		AfxMessageBox("Cannot open geocoder");
	}

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
	codingResultsGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		1
	);
	codingResultsGrid.Initialize();
	codingResultsGrid.SetColCount(ColumnCount);
	// Column headings
	codingResultsGrid.SetStyleRange(
		CGXRange(0, CityColumn),
		CGXStyle().SetValue("City")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, StateColumn),
		CGXStyle().SetValue("State")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, PostcodeColumn),
		CGXStyle().SetValue("Postcode")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, PostcodeExtColumn),
		CGXStyle().SetValue("PcodeExt")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, NbrColumn),
		CGXStyle().SetValue("Nbr")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, PredirColumn),
		CGXStyle().SetValue("Predir")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, PrefixColumn),
		CGXStyle().SetValue("Prefix")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, StreetColumn),
		CGXStyle().SetValue("Street")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, SuffixColumn),
		CGXStyle().SetValue("Suffix")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, PostdirColumn),
		CGXStyle().SetValue("Postdir")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, IntersectColumn),
		CGXStyle().SetValue("Intersect?")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, Street2Column),
		CGXStyle().SetValue("Street2")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, LatColumn),
		CGXStyle().SetValue("Lat")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, LonColumn),
		CGXStyle().SetValue("Lon")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, ScoreColumn),
		CGXStyle().SetValue("Score")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, MatchFlagsColumn),
		CGXStyle().SetValue("Match Flags")
	);
	codingResultsGrid.SetStyleRange(
		CGXRange(0, GeoFlagsColumn),
		CGXStyle().SetValue("Geo Flags")
	);
	codingResultsGridLabel.Create("Coding results", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CCodeAddressBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CCodeAddressBrowse::HandleSize(int cx, int cy) 
{
	// First Line
	firstLineLabel.MoveWindow(10, 10, 90, 20);
	firstLineEdit.MoveWindow(110, 10, 250, 20);

	// Last Line
	lastLineLabel.MoveWindow(10, 40, 90, 20);
	lastLineEdit.MoveWindow(110, 40, 250, 20);

	// Parse button
	parseButton.MoveWindow(375, 10, 100, 50);

	// Coding results grid
	codingResultsGridLabel.MoveWindow(10, 80, 200, 20, TRUE);
	codingResultsGrid.MoveWindow(10, 110, cx - 20, (cy - 200), TRUE);
}


BOOL CCodeAddressBrowse::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nID == ParseButtonID) {
		UpdateViews();
	}
	return 0;
	// return CWnd ::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CCodeAddressBrowse::UpdateViews()
{
	if (!initialized) {
		return;
	}

	// Clear results
	codingResultsGrid.SetRowCount(0);

	// Code the address
	CString firstLineStr;
	firstLineEdit.GetWindowText(firstLineStr);
	CString lastLineStr;
	lastLineEdit.GetWindowText(lastLineStr);

	geocoder->CodeAddress(firstLineStr, lastLineStr);

	char tmp[40];
	Geocoder::GeocodeResults result;
	while (geocoder->GetNextCandidate(result)) {
		// Set parse results to row
		codingResultsGrid.SetRowCount(codingResultsGrid.GetRowCount() + 1);
		int row = codingResultsGrid.GetRowCount();

		codingResultsGrid.SetStyleRange(
			CGXRange(row, CityColumn),
			CGXStyle().SetValue(result.GetCity())
		);
		sprintf(tmp, "%d", result.GetState());
		codingResultsGrid.SetStyleRange(
			CGXRange(row, StateColumn),
			CGXStyle().SetValue(tmp)
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, PostcodeColumn),
			CGXStyle().SetValue(result.GetPostcode())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, PostcodeExtColumn),
			CGXStyle().SetValue(result.GetPostcodeExt())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, NbrColumn),
			CGXStyle().SetValue(result.GetAddrNbr())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, PredirColumn),
			CGXStyle().SetValue(result.GetPredir())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, PrefixColumn),
			CGXStyle().SetValue(result.GetPrefix())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, StreetColumn),
			CGXStyle().SetValue(result.GetStreet())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, SuffixColumn),
			CGXStyle().SetValue(result.GetSuffix())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, PostdirColumn),
			CGXStyle().SetValue(result.GetPostdir())
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, IntersectColumn),
			CGXStyle().SetValue(
				(result.GetGeoStatus() & Geocoder::GeocodeIntersection) != 0 ? "Yes" : "No"
			)
		);
		codingResultsGrid.SetStyleRange(
			CGXRange(row, Street2Column),
			CGXStyle().SetValue(result.GetStreet2())
		);
		sprintf(tmp, "%11.6f", result.GetLatitude());
		codingResultsGrid.SetStyleRange(
			CGXRange(row, LatColumn),
			CGXStyle().SetValue(tmp)
		);
		sprintf(tmp, "%11.6f", result.GetLongitude());
		codingResultsGrid.SetStyleRange(
			CGXRange(row, LonColumn),
			CGXStyle().SetValue(tmp)
		);
		sprintf(tmp, "%d", result.GetMatchScore());
		codingResultsGrid.SetStyleRange(
			CGXRange(row, ScoreColumn),
			CGXStyle().SetValue(tmp)
		);
		sprintf(tmp, "0x%08x", result.GetMatchStatus());
		codingResultsGrid.SetStyleRange(
			CGXRange(row, MatchFlagsColumn),
			CGXStyle().SetValue(tmp)
		);
		sprintf(tmp, "0x%08x", result.GetGeoStatus());
		codingResultsGrid.SetStyleRange(
			CGXRange(row, GeoFlagsColumn),
			CGXStyle().SetValue(tmp)
		);
	}


}

