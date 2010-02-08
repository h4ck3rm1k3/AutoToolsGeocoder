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
// HierarchicalBrowse.h: Hierarchical browse window.
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_HierarchicalBrowse_H
#define INCL_HierarchicalBrowse_H

#include "CityStatePostcodeBrowseGrid.h"
#include "PostcodeAliasBrowseGrid.h"
#include "StreetNameBrowseGrid.h"
#include "StreetSegmentBrowseGrid.h"
#include "CoordinateBrowseGrid.h"


class CHierarchicalBrowse;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CHierarchicalBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CHierarchicalBrowse : public CWnd
{
public:
	// Construction
	CHierarchicalBrowse(QueryImpRef refQueryFileRef);
	virtual ~CHierarchicalBrowse();

	void Initialize();

	// Notifications from grids
	void CityStatePostcodeRowChange(int row);
	void PostcodeAliasRowChange(int row);
	void StreetNameRowChange(int row);
	void StreetSegmentRowChange(int row);

private:

	/////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////
	// Classes used to notify browser of current-cell motion
	/////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////
	class CCityStatePostcodeBrowseGridNotify : public CCityStatePostcodeBrowseGrid {
	public:
		CCityStatePostcodeBrowseGridNotify(CHierarchicalBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CHierarchicalBrowse* parent;
	};

	class CPostcodeAliasBrowseGridNotify : public CPostcodeAliasBrowseGrid {
	public:
		CPostcodeAliasBrowseGridNotify(CHierarchicalBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CHierarchicalBrowse* parent;
	};

	class CStreetNameBrowseGridNotify : public CStreetNameBrowseGrid {
	public:
		CStreetNameBrowseGridNotify(CHierarchicalBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CHierarchicalBrowse* parent;
	};

	class CStreetSegmentBrowseGridNotify : public CStreetSegmentBrowseGrid {
	public:
		CStreetSegmentBrowseGridNotify(CHierarchicalBrowse* parent_) : parent(parent_) {}

		/////////////////////////////////////////////////////////////////////////////
		// Called by OG base class when the cell focus moves.
		/////////////////////////////////////////////////////////////////////////////
		virtual void OnMovedCurrentCell(ROWCOL row, ROWCOL column);
	private:
		CHierarchicalBrowse* parent;
	};


	void HandleSize(int cx, int cy);

	// The query object
	QueryImpRef refQueryFileRef;
	bool initialized;

	// Browsing grids.
	CCityStatePostcodeBrowseGridNotify cityStatePostcodeBrowseGrid;
	CPostcodeAliasBrowseGridNotify zipAliasBrowseGrid;
	CStreetNameBrowseGridNotify streetNameBrowseGrid;
	CStreetSegmentBrowseGridNotify streetSegmentBrowseGrid;
	CCoordinateBrowseGrid coordinateBrowseGrid;

	// Labels for grids
	CStatic cityStatePostcodeBrowseLabel;
	CStatic zipAliasBrowseLabel;
	CStatic streetNameBrowseLabel;
	CStatic streetSegmentBrowseLabel;
	CStatic coordinateBrowseLabel;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHierarchicalBrowse)
	public:
	protected:
	//}}AFX_VIRTUAL


protected:
	// Generated message map functions
	//{{AFX_MSG(CHierarchicalBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
