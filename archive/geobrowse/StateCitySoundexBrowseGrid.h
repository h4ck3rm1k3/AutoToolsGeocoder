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

// StateCitySoundexBrowseGrid.h: A "virtual" grid that supports direct browsing of
// the StateCitySoundex records of the geocoder database.

#ifndef INCL_StateCitySoundexBrowseGrid_H
#define INCL_StateCitySoundexBrowseGrid_H

/////////////////////////////////////////////////////////////////////////////
// CStateCitySoundexBrowseGrid window.  Grid subclass that uses a FileFollower to browse
// the contents of a file that is being written.
/////////////////////////////////////////////////////////////////////////////

class CStateCitySoundexBrowseGrid : public CGXGridWnd
{
public:
	/////////////////////////////////////////////////////////////////////////////
	// constructor
	/////////////////////////////////////////////////////////////////////////////
	CStateCitySoundexBrowseGrid();

	/////////////////////////////////////////////////////////////////////////////
	// destructor
	/////////////////////////////////////////////////////////////////////////////
	virtual ~CStateCitySoundexBrowseGrid();

	///////////////////////////////////////////////////////////////////////////
	// Initialize: Initialize the grid
	// Inputs:
	//	GeoCoder::QueryImpRef	refQueryFile	The reference query object
	//													from which records will be read.
	///////////////////////////////////////////////////////////////////////////
	void Initialize(
		QueryImpRef refQueryFile
	);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStateCitySoundexBrowseGrid)
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CStateCitySoundexBrowseGrid)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

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
	virtual BOOL GetStyleRowCol(
		ROWCOL nRow, 
		ROWCOL nCol, 
		CGXStyle& style, 
		GXModifyType mt, 
		int nType
	);

	///////////////////////////////////////////////////////////////////////////
	// Override of the CGXGridCore base class method.
	// Called to get the row count dynamically and avoid allocating style objects.
	// Return value:
	//	ROWCOL		The number of rows in the grid
	///////////////////////////////////////////////////////////////////////////
	virtual ROWCOL GetRowCount();

private:
	// Object from which records are read.
	QueryImpRef refQueryFile;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif

