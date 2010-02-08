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
// ParseAddressBrowse.h: A browse window to retrieve intersecting
// streets within a state
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_ParseAddressBrowse_H
#define INCL_ParseAddressBrowse_H

class CParseAddressBrowse;

#include "AddressParserFirstLine.h"
#include "AddressParserLastLine.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CParseAddressBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CParseAddressBrowse : public CWnd
{
public:
	// Construction
	CParseAddressBrowse();
	virtual ~CParseAddressBrowse();

	void Initialize();

private:
	/////////////////////////////////////////////////////////////////////////////
	// Handle resizing of window
	/////////////////////////////////////////////////////////////////////////////
	void HandleSize(int cx, int cy);

	/////////////////////////////////////////////////////////////////////////////
	// Update the data views.
	/////////////////////////////////////////////////////////////////////////////
	void UpdateViews();

	// The parser objects
	AddressParserFirstLineRef firstLineParser;
	AddressParserLastLineRef lastLineParser;
	bool initialized;

	// Grids containing parse results
	CGXGridWnd firstLineParseGrid;
	CGXGridWnd lastLineParseGrid;

	// Edit fields
	CEdit firstLineEdit;
	CEdit lastLineEdit;

	// Button to process address
	CButton parseButton;

	// Labels 
	CStatic firstLineLabel;
	CStatic lastLineLabel;
	CStatic firstLineParseGridLabel;
	CStatic lastLineParseGridLabel;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParseAddressBrowse)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	enum {
		FirstLineEditID = 1,
		LastLineEditID = 2,
		ParseButtonID = 3
	};

	enum {
		NbrColumn = 1,
		PredirColumn,
		PrefixColumn,
		StreetColumn,
		SuffixColumn,
		PostdirColumn,
		UnitDesColumn,
		UnitColumn,
		IntersectColumn,
		Street2Column,
		ModsColumn,
		NbrColumns = ModsColumn
	};

protected:
	// Generated message map functions
	//{{AFX_MSG(CParseAddressBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
