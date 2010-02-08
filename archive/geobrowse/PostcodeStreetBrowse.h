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
// PostcodeStreetBrowse.h: A browse window to retrieve street records by
// Postcode code (or FA) and street name (which is converted to Soundex)
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_PostcodeStreetBrowse_H
#define INCL_PostcodeStreetBrowse_H

#include "CityStatePostcodeBrowseGrid.h"
#include "StreetNameSoundexBrowseGrid.h"
#include "StreetNameBrowseGrid.h"
#include "StreetSegmentBrowseGrid.h"
#include "CoordinateBrowseGrid.h"

class CPostcodeStreetBrowse;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CPostcodeStreetBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CPostcodeStreetBrowse : public CWnd
{
public:
	// Construction
	CPostcodeStreetBrowse(QueryImpRef refQueryFileRef);
	virtual ~CPostcodeStreetBrowse();

	void Initialize();

	/////////////////////////////////////////////////////////////////////////////
	// Notifications from grids
	/////////////////////////////////////////////////////////////////////////////
	void StreetSoundexRowChange(int row);
	void StreetNameRowChange(int row);
	void StreetSegmentRowChange(int row);

private:
	/////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////
	// Classes to catch grid cell movement.
	/////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////
	class CStreetSoundexBrowseGridNotify : public CStreetNameSoundexBrowseGrid {
	public:
		CStreetSoundexBrowseGridNotify(CPostcodeStreetBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CPostcodeStreetBrowse* parent;
	};

	class CStreetNameBrowseGridNotify : public CStreetNameBrowseGrid {
	public:
		CStreetNameBrowseGridNotify(CPostcodeStreetBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CPostcodeStreetBrowse* parent;
	};

	class CStreetSegmentBrowseGridNotify : public CStreetSegmentBrowseGrid {
	public:
		CStreetSegmentBrowseGridNotify(CPostcodeStreetBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CPostcodeStreetBrowse* parent;
	};


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
	CStreetSoundexBrowseGridNotify streetNameSoundexBrowseGrid;
	CStreetNameBrowseGridNotify streetNameBrowseGrid;
	CStreetSegmentBrowseGridNotify streetSegmentBrowseGrid;
	CCoordinateBrowseGrid coordinateBrowseGrid;

	// Edit fields
	CEdit zipEdit;
	CEdit streetNameEdit;

	// Labels 
	CStatic zipLabel;
	CStatic financeAreaLabel;
	CStatic streetNameLabel;
	CStatic streetSoundexLabel;
	CStatic streetNameSoundexBrowseLabel;
	CStatic streetNameBrowseLabel;
	CStatic streetSegmentBrowseLabel;
	CStatic coordinateBrowseLabel;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPostcodeStreetBrowse)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	enum {
		PostcodeEditID = 1,
		StreetNameEditID = 2,
	};



protected:
	// Generated message map functions
	//{{AFX_MSG(CPostcodeStreetBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
