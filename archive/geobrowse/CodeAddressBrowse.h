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
// CodeAddressBrowse.h: A browse window to retrieve intersecting
// streets within a state
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_CodeAddressBrowse_H
#define INCL_CodeAddressBrowse_H

class CCodeAddressBrowse;

#include "Geocoder.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCodeAddressBrowse window
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CCodeAddressBrowse : public CWnd
{
public:
	// Construction
	CCodeAddressBrowse(QueryImpRef refQueryFileRef);
	virtual ~CCodeAddressBrowse();

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

	// Reference query object
	QueryImpRef refQueryFileRef;

	bool initialized;

	// Grids containing coding results
	CGXGridWnd codingResultsGrid;

	// Edit fields
	CEdit firstLineEdit;
	CEdit lastLineEdit;

	// Button to process address
	CButton parseButton;

	// Labels 
	CStatic firstLineLabel;
	CStatic lastLineLabel;
	CStatic codingResultsGridLabel;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodeAddressBrowse)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	enum {
		FirstLineEditID = 1,
		LastLineEditID = 2,
		ParseButtonID = 3
	};

	enum {
		CityColumn = 1,
		StateColumn,
		PostcodeColumn,
		PostcodeExtColumn,
		NbrColumn,
		PredirColumn,
		PrefixColumn,
		StreetColumn,
		SuffixColumn,
		PostdirColumn,
		IntersectColumn,
		Street2Column,
		LatColumn,
		LonColumn,
		ScoreColumn,
		MatchFlagsColumn,
		GeoFlagsColumn,
		ColumnCount = GeoFlagsColumn
	};

	// Geocoder subclass
	class Geocoder_Notify : public Geocoder {
	public:
		///////////////////////////////////////////////////////////////////////
		// Constructor.  Will use the given reference-query interface object.
		// Inputs:
		//	const TsString&			tableDir	The geocoder directory containing the
		//											lookup tables for the address parser.
		//	const TsString&			databaseDir	The geocoder directory containing the
		//											geocoder database files.
		///////////////////////////////////////////////////////////////////////
		Geocoder_Notify(
			const TsString& tableDir,
			const TsString& databaseDir
		) :
			Geocoder(tableDir.c_str(), databaseDir.c_str())
		{
		}

		///////////////////////////////////////////////////////////////////////
		// Error-message-receiving method.
		// Override this to intercept human-readable error messages.
		///////////////////////////////////////////////////////////////////////
		virtual void ErrorMessage(const char* message) {
			TsString msg("Geocoder Error: ");
			msg += message;
			AfxMessageBox(msg.c_str());
		}

#ifndef NDEBUG
		///////////////////////////////////////////////////////////////////////
		// Trace method for debugging.
		// Override this to intercept human-readable debug trace messages.
		///////////////////////////////////////////////////////////////////////
		virtual void TraceMessage(const char* message) {
			TRACE0(message);
			TRACE0("\n");
		}
#endif

	};

	// The geocoder
	Geocoder_Notify *geocoder;

protected:
	// Generated message map functions
	//{{AFX_MSG(CCodeAddressBrowse)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}


#endif
