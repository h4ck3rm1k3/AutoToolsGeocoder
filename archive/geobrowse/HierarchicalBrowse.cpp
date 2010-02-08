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

// HierarchicalBrowse.cpp : implementation of the CHierarchicalBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "HierarchicalBrowse.h"
#include "GeoQuery.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHierarchicalBrowse

CHierarchicalBrowse::CHierarchicalBrowse(
	QueryImpRef refQueryFileRef_
) :
	refQueryFileRef(refQueryFileRef_),
	initialized(false),
	cityStatePostcodeBrowseGrid(this),
	zipAliasBrowseGrid(this),
	streetNameBrowseGrid(this),
	streetSegmentBrowseGrid(this)
{
}

CHierarchicalBrowse::~CHierarchicalBrowse()
{
}


BEGIN_MESSAGE_MAP(CHierarchicalBrowse,CWnd )
	//{{AFX_MSG_MAP(CHierarchicalBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CHierarchicalBrowse::OnPaint() 
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
void CHierarchicalBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// CityStatePostcode browser grid
	cityStatePostcodeBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		1
	);
	cityStatePostcodeBrowseGrid.Initialize(theApp.refQueryFile);

	// PostcodeAlias browser grid
	zipAliasBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		2
	);
	zipAliasBrowseGrid.Initialize(theApp.refQueryFile);

	// StreetName browser grid
	streetNameBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		3
	);
	streetNameBrowseGrid.Initialize(theApp.refQueryFile);

	// StreetSegment browser grid
	streetSegmentBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		4
	);
	streetSegmentBrowseGrid.Initialize(theApp.refQueryFile);

	// Coordinate browser grid
	coordinateBrowseGrid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		5
	);
	coordinateBrowseGrid.Initialize(theApp.refQueryFile);

	cityStatePostcodeBrowseLabel.Create("CityStatePostcode", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	zipAliasBrowseLabel.Create("PostcodeAlias", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	streetNameBrowseLabel.Create("StreetName", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	streetSegmentBrowseLabel.Create("StreetSegment", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	coordinateBrowseLabel.Create("Coordinate", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);


	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CHierarchicalBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CHierarchicalBrowse::HandleSize(int cx, int cy) 
{
	// PostcodeAlias
	zipAliasBrowseGrid.MoveWindow(10, 25, 150, cy / 2 - 30, TRUE);
	zipAliasBrowseLabel.MoveWindow(10, 5, 150, 15, TRUE);

	// CityStatePostcode
	cityStatePostcodeBrowseGrid.MoveWindow(170, 25, cx / 2 - 115, cy / 2 - 30, TRUE);
	cityStatePostcodeBrowseLabel.MoveWindow(170, 5, 130, 15, TRUE);

	// StreetName
	streetNameBrowseGrid.MoveWindow(cx / 2 + 80, 25, cx / 2 - 85, cy / 2 - 30, TRUE);
	streetNameBrowseLabel.MoveWindow(cx / 2 + 80, 5, 100, 15, TRUE);

	// StreetSegment
	streetSegmentBrowseGrid.MoveWindow(10, cy / 2 + 25, 2 * cx / 3 - 15, cy / 2 - 30, TRUE);
	streetSegmentBrowseLabel.MoveWindow(10, cy / 2 + 5, 100, 15, TRUE);

	// Coordinate
	coordinateBrowseGrid.MoveWindow(2 * cx / 3 + 15, cy / 2 + 25, cx / 3 - 15, cy / 2 - 30, TRUE);
	coordinateBrowseLabel.MoveWindow(2 * cx / 3 + 15, cy / 2 + 5, 100, 15, TRUE);
}


// Notifications from grids
void CHierarchicalBrowse::PostcodeAliasRowChange(int row)
{
	if (!initialized) {
		return;
	}
	// Select associated rows of CityStatePostcode
	PostcodeAlias postcodeAlias;
	if (!refQueryFileRef->GetPostcodeAliasByGroupIDCached(row - 1, postcodeAlias)) {
		return;
	}
	// Find the CityStatePostcode range for this zip.
	QueryImp::CityStatePostcodeFromPostcodeIterator iter = 
		refQueryFileRef->LookupCityStatePostcodeFromPostcode(postcodeAlias.postcode);
	int idMin, idMax;
	bool haveCityStatePostcode = false;
	CityStatePostcode cityStatePostcode;
	while (iter.Next(cityStatePostcode)) {
		idMax = cityStatePostcode.ID;
		if (!haveCityStatePostcode) {
			idMin = cityStatePostcode.ID;
			haveCityStatePostcode = true;
		}
	}

	cityStatePostcodeBrowseGrid.SetRowSelection(idMin + 1, idMax + 1);
}

void CHierarchicalBrowse::CityStatePostcodeRowChange(int row)
{
	if (!initialized) {
		return;
	}
	// Select associated rows of StreetName
	CityStatePostcode cityStatePostcode;
	if (!refQueryFileRef->GetCityStatePostcodeByIDCached(row - 1, cityStatePostcode)) {
		return;
	}
	streetNameBrowseGrid.SetRowSelection(
		cityStatePostcode.streetNameIDFirst + 1,
		cityStatePostcode.streetNameIDLast + 1
	);
}

void CHierarchicalBrowse::StreetNameRowChange(int row)
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
}

void CHierarchicalBrowse::StreetSegmentRowChange(int row)
{
	if (!initialized) {
		return;
	}
	// Select assocated rows of Coordinate
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
// Classes used to notify browser of current-cell motion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CHierarchicalBrowse::CCityStatePostcodeBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column) {
	parent->CityStatePostcodeRowChange(row);
}

void CHierarchicalBrowse::CPostcodeAliasBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column) {
	parent->PostcodeAliasRowChange(row);
}

void CHierarchicalBrowse::CStreetNameBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column) {
	parent->StreetNameRowChange(row);
}

void CHierarchicalBrowse::CStreetSegmentBrowseGridNotify::OnMovedCurrentCell(ROWCOL row, ROWCOL column) {
	parent->StreetSegmentRowChange(row);
}
