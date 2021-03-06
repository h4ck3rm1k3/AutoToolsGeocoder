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
// StreetIntersectionSoundexBrowse.h: Browse street intersection soundex records.
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_StreetIntersectionSoundexBrowse_H
#define INCL_StreetIntersectionSoundexBrowse_H

#include "StreetIntersectionSoundexBrowseGrid.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CStreetIntersectionSoundexBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CStreetIntersectionSoundexBrowse : public CWnd
{
public:
	// Construction
	CStreetIntersectionSoundexBrowse(QueryImpRef refQueryFileRef);
	virtual ~CStreetIntersectionSoundexBrowse();

	void Initialize();

private:
	/////////////////////////////////////////////////////////////////////////////
	// Handle resizing of window
	/////////////////////////////////////////////////////////////////////////////
	void HandleSize(int cx, int cy);

	// The query object
	QueryImpRef refQueryFileRef;
	bool initialized;

	// Browsing grids.
	CStreetIntersectionSoundexBrowseGrid grid;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreetIntersectionSoundexBrowse)
	public:
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CStreetIntersectionSoundexBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
