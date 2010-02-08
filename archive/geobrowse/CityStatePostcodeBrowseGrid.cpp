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
// CityStatePostcodeBrowseGrid.cpp: Display records from a RefQueryFile object.
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CityStatePostcodeBrowseGrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// constructor
/////////////////////////////////////////////////////////////////////////////
CCityStatePostcodeBrowseGrid::CCityStatePostcodeBrowseGrid() :
	selStart(0),
	selEnd(0)
{
}

/////////////////////////////////////////////////////////////////////////////
// destructor
/////////////////////////////////////////////////////////////////////////////
CCityStatePostcodeBrowseGrid::~CCityStatePostcodeBrowseGrid()
{
}

/////////////////////////////////////////////////////////////////////////////
// Wizard-generated message map
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CCityStatePostcodeBrowseGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CCityStatePostcodeBrowseGrid)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////
// Initialize: Initialize the grid
// Inputs:
//	GeoCoder::QueryImpRef	refQueryFile	The reference query object
//													from which records will be read.
///////////////////////////////////////////////////////////////////////////
void CCityStatePostcodeBrowseGrid::Initialize(
	QueryImpRef refQueryFile_
) {
	refQueryFile = refQueryFile_;

	CGXGridWnd::Initialize();

	// Column headings:
	// State, Postcode, FinanceNumber, City, StreetNameIDFirst, StreetNameIDLast
	SetColCount(6);

	// View grids disallow UNDO.  Don't turn this on, or warnings will
	// be issued when you try to re-run a job with a big grid.
	GetParam()->EnableUndo(FALSE);

}

///////////////////////////////////////////////////////////////////////////
// GetStyleRowCol: Called by grid core to get the values and styles for
// grid cells.  This allows us to implement a "virtual" grid.
// Inputs:
//	ROWCOL			nRow		Row of cell to fetch.  Zero means column header.
//	ROWCOL			nCol		Column of cell to fetch.  Zero means row header.
//	GXModifyType	mt			Modification type (see OG docs).
//	int				nType		-1 for complete row, column, or table
// Outputs:
//	CGXStyle&		styleReturn	The style object that will be filled in 
//								by this method.
// Return value:
//	BOOL		TRUE if this method filled in the style/value, FALSE if
//				the grid base class should do it.
///////////////////////////////////////////////////////////////////////////
BOOL CCityStatePostcodeBrowseGrid::GetStyleRowCol(
	ROWCOL nRow, 
	ROWCOL nCol, 
	CGXStyle& style, 
	GXModifyType mt, 
	int nType
) {
	// Set basic style
	style.SetReadOnly(TRUE);

	if (mt == gxRemove) {
		// not supported
		return FALSE;
	} else if (nType == -1) {      
		// here you can return the style for a complete row, column or table
		return FALSE;   
	} else if (nRow == 0 && nCol == 0) {
		// style for the top-left button
		return FALSE;
	} else if (nRow == 0 && nCol > 0) {
		// Column headers
		const char* label = "";
		switch (nCol) {
		case 1: label = "State"; break;
		case 2: label = "Postcode"; break;
		case 3: label = "Finance"; break;
		case 4: label = "City"; break;
		case 5: label = "StreetNameIDFirst"; break;
		case 6: label = "StreetNameIDLast"; break;
		}
		style.
			SetValue(label).
			SetReadOnly(TRUE);
		return TRUE;
	} else if (nCol == 0) {
		// Row headers
		return FALSE;
	}

	if (refQueryFile == 0) {
		return FALSE;
	}

	int recordID = nRow - 1;
	CityStatePostcode cityStatePostcode;
	if (!refQueryFile->GetCityStatePostcodeByIDCached(recordID, cityStatePostcode)) {
		style.
			SetValue("***ERROR***");
		return TRUE;
	}

	char buf[100];
	switch (nCol) {
	case 1: 
		sprintf(buf, "%d", cityStatePostcode.state);
		style.SetValue(buf);
		break;
	case 2: 
		style.SetValue(cityStatePostcode.postcode);
		break;
	case 3: 
		style.SetValue(cityStatePostcode.financeNumber);
		break;
	case 4: 
		style.SetValue(cityStatePostcode.city);
		break;
	case 5: 
		sprintf(buf, "%d", cityStatePostcode.streetNameIDFirst);
		style.SetValue(buf);
		break;
	case 6:
		sprintf(buf, "%d", cityStatePostcode.streetNameIDLast);
		style.SetValue(buf);
		break;
	default:
		style.
			SetValue("***ERROR***");
		return TRUE;
		break;
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Override of the CGXGridCore base class method.
// Called to get the row count dynamically and avoid allocating style objects.
// Return value:
//	ROWCOL		The number of rows in the grid
///////////////////////////////////////////////////////////////////////////
ROWCOL CCityStatePostcodeBrowseGrid::GetRowCount()
{
	if (refQueryFile == 0) {
		return 0;
	} else {
		return refQueryFile->GetCityStatePostcodeCount();
	}
}


///////////////////////////////////////////////////////////////////////////
// Set the highlighted selection
///////////////////////////////////////////////////////////////////////////
void CCityStatePostcodeBrowseGrid::SetRowSelection(int selStart_, int selEnd_)
{
	int oldSelStart = selStart;
	int oldSelEnd = selEnd;

	selStart = selStart_;
	selEnd = selEnd_;

	if (oldSelStart <= oldSelEnd) {
		// Unhilight previous selection
		RedrawRowCol(oldSelStart, 1, oldSelEnd, GetColCount());
	}

	if (selStart <= selEnd) {
		// Hilight new selection
		RedrawRowCol(selStart, 1, selEnd, GetColCount());
		// Put top of select at top row
		SetTopRow(selStart);
	}
}


