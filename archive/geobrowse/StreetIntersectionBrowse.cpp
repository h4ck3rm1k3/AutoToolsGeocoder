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

// StreetIntersectionBrowse.cpp : implementation of the CStreetIntersectionBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "StreetIntersectionBrowse.h"
#include "Soundex.h"
#include "Registry.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStreetIntersectionBrowse

CStreetIntersectionBrowse::CStreetIntersectionBrowse(
	QueryImpRef refQueryFileRef_
) :
	refQueryFileRef(refQueryFileRef_),
	initialized(false),
	grid(this)
{
	stateToFipsTable = new LookupTable;

	TsString tableDir;
	if (Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\DataLever\\geobrowse\\TableDirectory", tableDir)) {
		TsString errorMsg;
		stateToFipsTable->LoadFromFile(tableDir + "/state_abbr_to_fips.csv", errorMsg);
	}
}

CStreetIntersectionBrowse::~CStreetIntersectionBrowse()
{
}


BEGIN_MESSAGE_MAP(CStreetIntersectionBrowse,CWnd )
	//{{AFX_MSG_MAP(CStreetIntersectionBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CStreetIntersectionBrowse::OnPaint() 
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
void CStreetIntersectionBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// State input
	stateLabel.Create("State:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	stateEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, StateEditID);

	// Street1
	street1Label.Create("Street1:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	street1Edit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, Street1EditID);
	street1Edit.SetLimitText(60);
	street1SoundexLabel.Create("Soundex = ", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Street2
	street2Label.Create("Street2:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	street2Edit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, Street2EditID);
	street2Edit.SetLimitText(60);
	street2SoundexLabel.Create("Soundex = ", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Full street info
	fullStreet1Edit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY, CRect(0,0,0,0), this, -1);
	fullStreet1Label.Create("Full street1 info:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	fullStreet2Edit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY, CRect(0,0,0,0), this, -1);
	fullStreet2Label.Create("Full street2 info:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Intersection grid
	grid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		10
	);
	gridLabel.Create("Street Intersections", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	grid.Initialize();
	grid.SetColCount(9);
	// Column headings
	grid.SetStyleRange(
		CGXRange(0, 1),
		CGXStyle().SetValue("State")
	);
	grid.SetStyleRange(
		CGXRange(0, 2),
		CGXStyle().SetValue("City1")
	);
	grid.SetStyleRange(
		CGXRange(0, 3),
		CGXStyle().SetValue("Postcode1")
	);
	grid.SetStyleRange(
		CGXRange(0, 4),
		CGXStyle().SetValue("Street1")
	);
	grid.SetStyleRange(
		CGXRange(0, 5),
		CGXStyle().SetValue("StreetSegmentID1")
	);
	grid.SetStyleRange(
		CGXRange(0, 6),
		CGXStyle().SetValue("City2")
	);
	grid.SetStyleRange(
		CGXRange(0, 7),
		CGXStyle().SetValue("Postcode2")
	);
	grid.SetStyleRange(
		CGXRange(0, 8),
		CGXStyle().SetValue("Street2")
	);
	grid.SetStyleRange(
		CGXRange(0, 9),
		CGXStyle().SetValue("StreetSegmentID2")
	);

	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CStreetIntersectionBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CStreetIntersectionBrowse::HandleSize(int cx, int cy) 
{
	// State
	stateLabel.MoveWindow(10, 10, 100, 20);
	stateEdit.MoveWindow(120, 10, 100, 20);

	// Street1
	street1Label.MoveWindow(10, 40, 100, 20);
	street1Edit.MoveWindow(120, 40, 200, 20);
	street1SoundexLabel.MoveWindow(330, 40, 150, 20);

	// Street2
	street2Label.MoveWindow(10, 70, 100, 20);
	street2Edit.MoveWindow(120, 70, 200, 20);
	street2SoundexLabel.MoveWindow(330, 70, 150, 20);

	// Intersection browser
	gridLabel.MoveWindow(10, 100, 140, 20, TRUE);
	grid.MoveWindow(10, 125, cx - 20, cy - 275, TRUE);

	// Full street information
	fullStreet1Label.MoveWindow(10, cy - 150, 200, 20, TRUE);
	fullStreet1Edit.MoveWindow(10, cy - 130, cx / 2 - 20, 120, TRUE);
	fullStreet2Label.MoveWindow(cx / 2 + 10, cy - 150, 200, 20, TRUE);
	fullStreet2Edit.MoveWindow(cx / 2 + 10, cy - 130, cx / 2 - 20, 120, TRUE);
}


BOOL CStreetIntersectionBrowse::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (
		(nID == StateEditID || nID == Street1EditID || nID == Street2EditID ) &&
		nCode == EN_CHANGE
	) {
		UpdateViews();
	}
	return 0;
	// return CWnd ::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CStreetIntersectionBrowse::UpdateViews()
{
	if (!initialized) {
		return;
	}
	CString tmpStr;

	// Clear results
	grid.SetRowCount(0);
	fullStreet1Edit.SetWindowText("");
	fullStreet2Edit.SetWindowText("");

	// Process State
	stateEdit.GetWindowText(tmpStr);
	if (tmpStr.GetLength() == 0) {
		return;
	}
	int state = atoi(tmpStr);
	if (state <= 0 || state > 99) {
		// Check for abbreviation
		tmpStr.MakeUpper();
		const char* tmpStr2;
		if (stateToFipsTable->Find((const char*)tmpStr, tmpStr2)) {
			state = atoi(tmpStr2);
		}
	}

	char buf[100];

	// Process Street1
	char soundex1[5];
	street1Edit.GetWindowText(tmpStr);
	Soundex3((const char*)tmpStr, buf, soundex1);
	street1SoundexLabel.SetWindowText(soundex1);

	// Process Street2
	char soundex2[5];
	street2Edit.GetWindowText(tmpStr);
	Soundex3((const char*)tmpStr, buf, soundex2);
	street2SoundexLabel.SetWindowText(soundex2);

	streetIntersectionList.clear();

	if (state <= 0 || state > 99) {
		return;
	}

	// Query twice with the first/second soundex swapped.
	for (int loop = 0; loop < 2; loop++) {
		QueryImp::StreetIntersectionIterator iter;
		if (loop == 0) {
			// Query intersections
			iter = refQueryFileRef->LookupStreetIntersection(state, soundex1, soundex2);
		} else {
			// Query intersections
			iter = refQueryFileRef->LookupStreetIntersection(state, soundex2, soundex1);
		}

		StreetIntersection streetIntersection;
		while (iter.Next(streetIntersection)) {
			streetIntersectionList.push_back(streetIntersection);
			int row = grid.GetRowCount() + 1;
			grid.SetRowCount(row);

			char tmp[20];

			// Column values
			sprintf(tmp, "%d", streetIntersection.cityStatePostcode1.state);
			grid.SetStyleRange(
				CGXRange(row, 1),
				CGXStyle().SetValue(tmp)
			);

			grid.SetStyleRange(
				CGXRange(row, 2),
				CGXStyle().SetValue(streetIntersection.cityStatePostcode1.city)
			);

			grid.SetStyleRange(
				CGXRange(row, 3),
				CGXStyle().SetValue(streetIntersection.cityStatePostcode1.postcode)
			);

			grid.SetStyleRange(
				CGXRange(row, 4),
				CGXStyle().SetValue(streetIntersection.streetName1.street)
			);

			sprintf(tmp, "%d", streetIntersection.streetSegment1.ID);
			grid.SetStyleRange(
				CGXRange(row, 5),
				CGXStyle().SetValue(tmp)
			);

			grid.SetStyleRange(
				CGXRange(row, 6),
				CGXStyle().SetValue(streetIntersection.cityStatePostcode2.city)
			);

			grid.SetStyleRange(
				CGXRange(row, 7),
				CGXStyle().SetValue(streetIntersection.cityStatePostcode2.postcode)
			);

			grid.SetStyleRange(
				CGXRange(row, 8),
				CGXStyle().SetValue(streetIntersection.streetName2.street)
			);

			sprintf(tmp, "%d", streetIntersection.streetSegment2.ID);
			grid.SetStyleRange(
				CGXRange(row, 9),
				CGXStyle().SetValue(tmp)
			);


		}
	}
}

void CStreetIntersectionBrowse::RowChanged(int row)
{
	if (row == 0) {
		return;
	}

	char tmp[100];

	// Reset the outputs
	fullStreet1Edit.SetWindowText("");
	fullStreet2Edit.SetWindowText("");

	row--;
	if (row < 0 || row > streetIntersectionList.size()) {
		assert(0);
		return;
	}

	// Construct full street information
	TsString tmpStr;
	const StreetIntersection& streetIntersection = streetIntersectionList[row];

	// Query first street coordinates
	QueryImp::CoordinatePointsFromStreetSegmentIterator iter;
	iter = refQueryFileRef->LookupCoordinatePointsFromStreetSegment(streetIntersection.streetSegment1);
	CoordinatePoint point;
	std::vector<CoordinatePoint> points;
	while (iter.Next(point)) {
		points.push_back(point);
	}

	tmpStr = "City1=";
	tmpStr += streetIntersection.cityStatePostcode1.city;
	tmpStr += ", Postcode1=";
	tmpStr += streetIntersection.cityStatePostcode1.postcode;
	tmpStr += "\r\n";
	tmpStr += streetIntersection.streetName1.predir;
	tmpStr += " ";
	tmpStr += streetIntersection.streetName1.street;
	tmpStr += " ";
	tmpStr += streetIntersection.streetName1.suffix;
	tmpStr += " ";
	tmpStr += streetIntersection.streetName1.postdir;
	tmpStr += "\r\n";
	tmpStr += streetIntersection.streetSegment1.isRightSide ? "Right side, " : "Left side, ";
	tmpStr += "AddrLow=";
	tmpStr += streetIntersection.streetSegment1.addrLow;
	tmpStr += ", AddrHigh=";
	tmpStr += streetIntersection.streetSegment1.addrHigh;

	tmpStr += "\r\n";
	tmpStr += "Lat1= ";
	sprintf(tmp, "%g", points[0].latitude);
	tmpStr += tmp;
	tmpStr += ", Long1= ";
	sprintf(tmp, "%g", points[0].longitude);
	tmpStr += tmp;
	tmpStr += "\r\n";
	tmpStr += "Lat1= ";
	sprintf(tmp, "%g", points[points.size()-1].latitude);
	tmpStr += tmp;
	tmpStr += ", Long2= ";
	sprintf(tmp, "%g", points[points.size()-1].longitude);
	tmpStr += tmp;
	fullStreet1Edit.SetWindowText(tmpStr.c_str());

	// Query second street coordinates
	iter = refQueryFileRef->LookupCoordinatePointsFromStreetSegment(streetIntersection.streetSegment2);
	points.clear();
	while (iter.Next(point)) {
		points.push_back(point);
	}

	tmpStr = "City2=";
	tmpStr += streetIntersection.cityStatePostcode2.city;
	tmpStr += ", Postcode2=";
	tmpStr += streetIntersection.cityStatePostcode2.postcode;
	tmpStr += "\r\n";
	tmpStr += streetIntersection.streetName2.predir;
	tmpStr += " ";
	tmpStr += streetIntersection.streetName2.street;
	tmpStr += " ";
	tmpStr += streetIntersection.streetName2.suffix;
	tmpStr += " ";
	tmpStr += streetIntersection.streetName2.postdir;
	tmpStr += "\r\n";
	tmpStr += streetIntersection.streetSegment2.isRightSide ? "Right side, " : "Left side, ";
	tmpStr += "AddrLow=";
	tmpStr += streetIntersection.streetSegment2.addrLow;
	tmpStr += ", AddrHigh=";
	tmpStr += streetIntersection.streetSegment2.addrHigh;

	tmpStr += "\r\n";
	tmpStr += "Lat1= ";
	sprintf(tmp, "%g", points[0].latitude);
	tmpStr += tmp;
	tmpStr += ", Long1= ";
	sprintf(tmp, "%g", points[0].longitude);
	tmpStr += tmp;
	tmpStr += "\r\n";
	tmpStr += "Lat1= ";
	sprintf(tmp, "%g", points[points.size()-1].latitude);
	tmpStr += tmp;
	tmpStr += ", Long2= ";
	sprintf(tmp, "%g", points[points.size()-1].longitude);
	tmpStr += tmp;
	fullStreet2Edit.SetWindowText(tmpStr.c_str());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Class to catch grid cell movement.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CStreetIntersectionGridNotify::OnMovedCurrentCell(
	ROWCOL row, ROWCOL column
) {
	parent->RowChanged(row);
}

