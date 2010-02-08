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

// PostcodeStreetBrowse.cpp : implementation of the CPostcodeStreetBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "PostcodeStreetBrowse.h"
#include "Soundex.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPostcodeStreetBrowse

CPostcodeStreetBrowse::CPostcodeStreetBrowse(
	QueryImpRef refQueryFileRef_
) :
	refQueryFileRef(refQueryFileRef_),
	initialized(false),
	streetNameSoundexBrowseGrid(this),
	streetNameBrowseGrid(this),
	streetSegmentBrowseGrid(this)
{
}

CPostcodeStreetBrowse::~CPostcodeStreetBrowse()
{
}


BEGIN_MESSAGE_MAP(CPostcodeStreetBrowse,CWnd )
	//{{AFX_MSG_MAP(CPostcodeStreetBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CPostcodeStreetBrowse::OnPaint() 
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
void CPostcodeStreetBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// Postcode code input
	zipLabel.Create("Postcode:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	zipEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, PostcodeEditID);

	// Finance area
	financeAreaLabel.Create("Finance Area = ", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Street name input
	streetNameLabel.Create("Street:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	streetNameEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, StreetNameEditID);
	streetNameEdit.SetLimitText(60);

	// Soundex
	streetSoundexLabel.Create("Soundex = ", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Street name soundex browser grid
	streetNameSoundexBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		3
	);
	streetNameSoundexBrowseGrid.Initialize(theApp.refQueryFile);
	streetNameSoundexBrowseLabel.Create("Street soundex", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Street name browser grid
	streetNameBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		3
	);
	streetNameBrowseGrid.Initialize(theApp.refQueryFile);
	streetNameBrowseLabel.Create("Street names", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Street range browser grid
	streetSegmentBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		4
	);
	streetSegmentBrowseGrid.Initialize(theApp.refQueryFile);
	streetSegmentBrowseLabel.Create("Street Ranges", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	// Coordinate browser grid
	coordinateBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		5
	);
	coordinateBrowseGrid.Initialize(theApp.refQueryFile);
	coordinateBrowseLabel.Create("Coordinate", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);

	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CPostcodeStreetBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CPostcodeStreetBrowse::HandleSize(int cx, int cy) 
{
	// Postcode
	zipLabel.MoveWindow(10, 10, 100, 20);
	zipEdit.MoveWindow(120, 10, 100, 20);

	// Finance number
	financeAreaLabel.MoveWindow(230, 10, 200, 20);

	// Street
	streetNameLabel.MoveWindow(10, 40, 100, 20);
	streetNameEdit.MoveWindow(120, 40, 200, 20);

	// Street soundex
	streetSoundexLabel.MoveWindow(330, 40, 200, 20);

	// StreetNameSoundex browser
	streetNameSoundexBrowseLabel.MoveWindow(10, 70, 100, 20, TRUE);
	streetNameSoundexBrowseGrid.MoveWindow(10, 95, cx / 3 - 20, cy / 2 - 100, TRUE);

	// StreetName browser
	streetNameBrowseLabel.MoveWindow(cx / 3, 70, 100, 20, TRUE);
	streetNameBrowseGrid.MoveWindow(cx / 3, 95, 2 * cx / 3 - 20, cy / 2 - 100, TRUE);

	// StreetSegment browser
	streetSegmentBrowseLabel.MoveWindow(10, cy / 2 + 30, 100, 20, TRUE);
	streetSegmentBrowseGrid.MoveWindow(10, cy / 2 + 50, cx - 290, cy / 2 - 50, TRUE);

	// Coordinate
	coordinateBrowseLabel.MoveWindow(cx - 260, cy / 2 + 30, 100, 20, TRUE);
	coordinateBrowseGrid.MoveWindow(cx - 260, cy / 2 + 50, 250, cy / 2 - 50, TRUE);
}


BOOL CPostcodeStreetBrowse::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (
		(nID == PostcodeEditID || nID == StreetNameEditID) &&
		nCode == EN_CHANGE
	) {
		UpdateViews();
	}
	return 0;
	// return CWnd ::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CPostcodeStreetBrowse::UpdateViews()
{
	if (!initialized) {
		return;
	}

	CString postcode, streetStr;
	zipEdit.GetWindowText(postcode);
	streetNameEdit.GetWindowText(streetStr);


	// Get Fa from postal code
	financeAreaLabel.SetWindowText("Unknown");
	CityStatePostcode cityStatePostcode;
	QueryImp::CityStatePostcodeFromPostcodeIterator iter = 
		refQueryFileRef->LookupCityStatePostcodeFromPostcode(postcode);
	if (iter.Next(cityStatePostcode)) {
		financeAreaLabel.SetWindowText(cityStatePostcode.financeNumber);
	}

	// Process street name soundex
	char soundex[5];
	char buf[100];
	Soundex3((const char*)streetStr, buf, soundex);
	streetSoundexLabel.SetWindowText(soundex);

	// Select Street Soundex entries
	QueryImp::StreetNameSoundex streetNameSoundex;
	if (refQueryFileRef->FindStreetNameSoundexByFaSoundexCached(cityStatePostcode.financeNumber, soundex, streetNameSoundex)) {
		// Read the remainder.
		int idFirst = streetNameSoundex.ID;
		int idLast = idFirst + 1;
		while (
			refQueryFileRef->GetStreetNameSoundexByID(idLast, streetNameSoundex) &&
			strcmp(streetNameSoundex.financeNumber, cityStatePostcode.financeNumber) == 0 &&
			strcmp(streetNameSoundex.streetSoundex, soundex) == 0
		) {
			idLast++;
		}
		streetNameSoundexBrowseGrid.SetRowSelection(idFirst + 1, idLast);
	} else {
		streetNameSoundexBrowseGrid.SetRowSelection(0,-1);
	}
}


/////////////////////////////////////////////////////////////////////////////
// Notifications from grids
/////////////////////////////////////////////////////////////////////////////
void CPostcodeStreetBrowse::StreetSoundexRowChange(int row)
{
	if (!initialized) {
		return;
	}
	// Select associated rows of StreetName
	QueryImp::StreetNameSoundex streetNameSoundex;
	if (!refQueryFileRef->GetStreetNameSoundexByID(row - 1, streetNameSoundex)) {
		return;
	}
	streetNameBrowseGrid.SetRowSelection(
		streetNameSoundex.streetNameID + 1,
		streetNameSoundex.streetNameID + 1
	);

	StreetNameRowChange(streetNameSoundex.streetNameID + 1);
}


void CPostcodeStreetBrowse::StreetNameRowChange(int row)
{
	if (!initialized) {
		return;
	}
	// Select assocated rows of StreetSegment
	StreetName streetName;
	if (!refQueryFileRef->GetStreetNameByIDCached(row - 1, streetName)) {
		return;
	}
	streetSegmentBrowseGrid.SetRowSelection(
		streetName.streetSegmentIDFirst + 1,
		streetName.streetSegmentIDFirst + streetName.streetSegmentCount
	);
	StreetSegmentRowChange(streetName.streetSegmentIDFirst + 1);
}

void CPostcodeStreetBrowse::StreetSegmentRowChange(int row)
{
	if (!initialized) {
		return;
	}
	// Select assocated rows of StreetSegment
	StreetSegment streetSegment;
	if (!refQueryFileRef->GetStreetSegmentByIDCached(row - 1, streetSegment)) {
		return;
	}
	coordinateBrowseGrid.SetRowSelection(
		streetSegment.coordinateID + 1,
		streetSegment.coordinateID + streetSegment.coordinateCount
	);

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Class to catch grid cell movement.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CPostcodeStreetBrowse::CStreetSoundexBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column)
{
	parent->StreetSoundexRowChange(row);
}

void CPostcodeStreetBrowse::CStreetNameBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column) {
	parent->StreetNameRowChange(row);
}


void CPostcodeStreetBrowse::CStreetSegmentBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column) {
	parent->StreetSegmentRowChange(row);
}
