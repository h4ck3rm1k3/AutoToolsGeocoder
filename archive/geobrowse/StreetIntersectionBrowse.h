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
// StreetIntersectionBrowse.h: A browse window to retrieve intersecting
// streets within a state
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_StreetIntersectionBrowse_H
#define INCL_StreetIntersectionBrowse_H

class CStreetIntersectionBrowse;

#include "LookupTable.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Class to catch grid cell movement.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CStreetIntersectionGridNotify : public CGXGridWnd {
public:
	CStreetIntersectionGridNotify(CStreetIntersectionBrowse* parent_) : parent(parent_) {}

	/////////////////////////////////////////////////////////////////////////////
	// Called by OG base class when the cell focus moves.
	/////////////////////////////////////////////////////////////////////////////
	virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
private:
	CStreetIntersectionBrowse* parent;
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CStreetIntersectionBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CStreetIntersectionBrowse : public CWnd
{
public:
	// Construction
	CStreetIntersectionBrowse(QueryImpRef refQueryFileRef);
	virtual ~CStreetIntersectionBrowse();

	void Initialize();

	/////////////////////////////////////////////////////////////////////////////
	// Notify of grid row change
	/////////////////////////////////////////////////////////////////////////////
	void RowChanged(int row);

private:
	/////////////////////////////////////////////////////////////////////////////
	// Handle resizing of window
	/////////////////////////////////////////////////////////////////////////////
	void HandleSize(int cx, int cy);

	/////////////////////////////////////////////////////////////////////////////
	// Update the data views.
	/////////////////////////////////////////////////////////////////////////////
	void UpdateViews();

	// The query object
	QueryImpRef refQueryFileRef;
	bool initialized;

	// Browsing grids.
	CStreetIntersectionGridNotify grid;

	// Edit fields
	CEdit stateEdit;
	CEdit street1Edit;
	CEdit street2Edit;

	// Labels 
	CStatic stateLabel;
	CStatic street1Label;
	CStatic street2Label;
	CStatic street1SoundexLabel;
	CStatic street2SoundexLabel;
	CStatic streetIntersectionBrowseLabel;
	CStatic gridLabel;
	CStatic fullStreet1Label;
	CStatic fullStreet2Label;

	// Output information
	CEdit fullStreet1Edit;
	CEdit fullStreet2Edit;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreetIntersectionBrowse)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	enum {
		StateEditID = 1,
		Street1EditID = 2,
		Street2EditID = 3
	};

	// Street intersection records corresponding to current grid contents
	std::vector<StreetIntersection> streetIntersectionList;


protected:
	// Generated message map functions
	//{{AFX_MSG(CStreetIntersectionBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LookupTableRef stateToFipsTable;
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
