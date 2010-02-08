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
// PostcodeCityBrowse.h: A browse window to retrieve city records by Postcode
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_PostcodeCityBrowse_H
#define INCL_PostcodeCityBrowse_H

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CPostcodeCityBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CPostcodeCityBrowse : public CWnd
{
public:
	// Construction
	CPostcodeCityBrowse(QueryImpRef refQueryFileRef);
	virtual ~CPostcodeCityBrowse();

	void Initialize();

	/////////////////////////////////////////////////////////////////////////////
	// Notifications from grids
	/////////////////////////////////////////////////////////////////////////////
	void StreetSoundexRowChange(int row);

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
	CGXGridWnd cityGrid;

	// Edit fields
	CEdit zipEdit;

	// Labels 
	CStatic zipLabel;
	CStatic cityGridLabel;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPostcodeCityBrowse)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	enum {
		PostcodeEditID = 1
	};



protected:
	// Generated message map functions
	//{{AFX_MSG(CPostcodeCityBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
