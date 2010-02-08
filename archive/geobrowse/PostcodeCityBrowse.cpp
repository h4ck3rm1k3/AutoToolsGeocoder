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

// PostcodeCityBrowse.cpp : implementation of the CPostcodeCityBrowse class
//

#include "stdafx.h"
#include "geobrowse.h"
#include "PostcodeCityBrowse.h"
#include "Soundex.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPostcodeCityBrowse
/////////////////////////////////////////////////////////////////////////////
CPostcodeCityBrowse::CPostcodeCityBrowse(
	QueryImpRef refQueryFileRef_
) :
	refQueryFileRef(refQueryFileRef_),
	initialized(false)
{
}

CPostcodeCityBrowse::~CPostcodeCityBrowse()
{
}


BEGIN_MESSAGE_MAP(CPostcodeCityBrowse,CWnd )
	//{{AFX_MSG_MAP(CPostcodeCityBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CPostcodeCityBrowse::OnPaint() 
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
void CPostcodeCityBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// Postcode code input
	zipLabel.Create("Postcode:", WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this);
	zipEdit.Create(WS_VISIBLE | WS_CHILD| WS_BORDER, CRect(0,0,0,0), this, PostcodeEditID);

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

void CPostcodeCityBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CPostcodeCityBrowse::HandleSize(int cx, int cy) 
{
	// Postcode
	zipLabel.MoveWindow(10, 10, 100, 20);
	zipEdit.MoveWindow(120, 10, 100, 20);

	// City browser
	cityGridLabel.MoveWindow(10, 70, 100, 20, TRUE);
	cityGrid.MoveWindow(10, 95, cx - 30, cy - 110, TRUE);
}


BOOL CPostcodeCityBrowse::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nID == PostcodeEditID && nCode == EN_CHANGE) {
		UpdateViews();
	}
	return 0;
	// return CWnd ::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CPostcodeCityBrowse::UpdateViews()
{
	if (!initialized) {
		return;
	}

	// Clear grid
	cityGrid.SetRowCount(0);

	// Process postal code
	CString postcode;
	zipEdit.GetWindowText(postcode);
	if (postcode.GetLength() == 0) {
		return;
	}

	// Query cities by Postal code
	QueryImp::CityStatePostcodeFromPostcodeIterator iter = 
		refQueryFileRef->LookupCityStatePostcodeFromPostcode(postcode);

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


