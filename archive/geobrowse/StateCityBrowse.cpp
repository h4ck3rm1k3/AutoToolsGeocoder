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

// StateCityBrowse.cpp : implementation of the CStateCityBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "StateCityBrowse.h"
#include "Soundex.h"
#include "Registry.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStateCityBrowse
/////////////////////////////////////////////////////////////////////////////
CStateCityBrowse::CStateCityBrowse(
	QueryImpRef refQueryFileRef_
) :
	refQueryFileRef(refQueryFileRef_),
	initialized(false)
{
	stateToFipsTable = new LookupTable;

	TsString tableDir;
	if (Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\DataLever\\geobrowse\\TableDirectory", tableDir)) {
		TsString errorMsg;
		stateToFipsTable->LoadFromFile(tableDir + "/state_abbr_to_fips.csv", errorMsg);
	}
}

CStateCityBrowse::~CStateCityBrowse()
{
}


BEGIN_MESSAGE_MAP(CStateCityBrowse,CWnd )
	//{{AFX_MSG_MAP(CStateCityBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CStateCityBrowse::OnPaint() 
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
void CStateCityBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// State input
	stateLabel.Create("State code:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	stateEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, StateEditID);
	stateCodeLabel.Create("", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// City input
	cityLabel.Create("City:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	cityEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, CityEditID);
	cityEdit.SetLimitText(60);
	citySoundexLabel.Create("Soundex = ", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// City grid
	cityGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		2
	);

	cityGrid.Initialize();
	cityGrid.SetColCount(4);
	// Column headings
	cityGrid.SetStyleRange(
		CGXRange(0, 1),
		CGXStyle().SetValue("State")
	);
	cityGrid.SetStyleRange(
		CGXRange(0, 2),
		CGXStyle().SetValue("Postcode")
	);
	cityGrid.SetStyleRange(
		CGXRange(0, 3),
		CGXStyle().SetValue("Finance")
	);
	cityGrid.SetStyleRange(
		CGXRange(0, 4),
		CGXStyle().SetValue("City")
	);
	cityGrid.GetParam()->EnableUndo(FALSE);


	cityGridLabel.Create("CityStatePostcode", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CStateCityBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CStateCityBrowse::HandleSize(int cx, int cy) 
{
	// State
	stateLabel.MoveWindow(10, 10, 100, 20);
	stateEdit.MoveWindow(120, 10, 100, 20);
	stateCodeLabel.MoveWindow(230, 10, 180, 20);

	// City
	cityLabel.MoveWindow(10, 40, 100, 20);
	cityEdit.MoveWindow(120, 40, 100, 20);
	citySoundexLabel.MoveWindow(230, 40, 180, 20);

	// City browser
	cityGridLabel.MoveWindow(10, 70, 100, 20, TRUE);
	cityGrid.MoveWindow(10, 95, cx - 30, cy - 110, TRUE);
}


BOOL CStateCityBrowse::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (
		(nID == StateEditID || nID == CityEditID) && 
		nCode == EN_CHANGE
	) {
		UpdateViews();
	}
	return 0;
	// return CWnd ::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CStateCityBrowse::UpdateViews()
{
	if (!initialized) {
		return;
	}

	// Clear grid
	cityGrid.SetRowCount(0);

	// Process State
	CString tmpStr;
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

	// Set code label
	if (state <= 0) {
		stateCodeLabel.SetWindowText("Unknown");
	} else {
		char buf[25];
		sprintf(buf, "Code = %d", state);
		stateCodeLabel.SetWindowText(buf);
	}

	// Process City
	cityEdit.GetWindowText(tmpStr);
	if (tmpStr.GetLength() == 0 || tmpStr.GetLength() >= 100) {
		return;
	}
	char soundex[5];
	char workBuf[101];
	Soundex2((const char*)tmpStr, workBuf, soundex);

	// Set soundex label
	TsString tmp("Soundex = ");
	tmp += soundex;
	citySoundexLabel.SetWindowText(tmp.c_str());

	if (state <= 0 || state > 99) {
		return;
	}

	// Query cities by state/soundex.
	QueryImp::CityStatePostcodeFromStateCityIterator iter = 
		refQueryFileRef->LookupCityStatePostcodeFromStateCity(state, soundex);

	CityStatePostcode cityStatePostcode;
	while (iter.Next(cityStatePostcode)) {
		int row = cityGrid.GetRowCount() + 1;
		cityGrid.SetRowCount(row);

		char tmp[20];

		// Column values
		sprintf(tmp, "%d", cityStatePostcode.state);
		cityGrid.SetStyleRange(
			CGXRange(row, 1),
			CGXStyle().SetValue(tmp)
		);

		cityGrid.SetStyleRange(
			CGXRange(row, 2),
			CGXStyle().SetValue(cityStatePostcode.postcode)
		);

		cityGrid.SetStyleRange(
			CGXRange(row, 3),
			CGXStyle().SetValue(cityStatePostcode.financeNumber)
		);

		cityGrid.SetStyleRange(
			CGXRange(row, 4),
			CGXStyle().SetValue(cityStatePostcode.city)
		);
	}
}


