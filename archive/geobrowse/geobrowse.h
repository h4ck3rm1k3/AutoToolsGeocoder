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

// geobrowse.h : main header file for the GEOBROWSE application
//

#if !defined(AFX_GEOBROWSE_H__4961EAB9_713B_11D4_B1E7_00105A0FCDF8__INCLUDED_)
#define AFX_GEOBROWSE_H__4961EAB9_713B_11D4_B1E7_00105A0FCDF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "TsString.h"
#include <list>

#include "GeoQueryImp.h"
#include "Geocoder.h"

/////////////////////////////////////////////////////////////////////////////
// Class that adds motifier-based error message hook to QueryImp.
/////////////////////////////////////////////////////////////////////////////
class RefQueryFileWithErrorHook : public QueryImp {
public:
	class Notifier : public VRefCount {
	public:
		virtual void Message(const TsString& msg) = 0;
	};
	typedef refcnt_ptr<Notifier> NotifierRef;

	RefQueryFileWithErrorHook(
		TsString dataDirectory_,
		TsString tableDirectory_,
		NotifierRef notifier_
	) :
		QueryImp(dataDirectory_, tableDirectory_, Geocoder::MemUseNormal),
		notifier(notifier_)
	{}

	///////////////////////////////////////////////////////////////////////
	// Override this to get error messages
	///////////////////////////////////////////////////////////////////////
	virtual void ErrorMessage(const TsString& msg) {
		notifier->Message(msg);
	}
private:
	NotifierRef notifier;
};


/////////////////////////////////////////////////////////////////////////////
// Class used to store error messages for later display.
/////////////////////////////////////////////////////////////////////////////
class NotifierQueue : public RefQueryFileWithErrorHook::Notifier {
public:
	virtual void Message(const TsString& msg) {
		messages.push_back(msg);
	}
	std::list<TsString> messages;
};
typedef refcnt_ptr<NotifierQueue> NotifierQueueRef;



/////////////////////////////////////////////////////////////////////////////
// CGeobrowseApp:
// See geobrowse.cpp for the implementation of this class
//

class CGeobrowseApp : public CWinApp
{
public:
	CGeobrowseApp();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeobrowseApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGeobrowseApp)
	afx_msg void OnAppAbout();
	afx_msg void OnTestReadStreetSegments();
	afx_msg void OnFileReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	QueryImpRef refQueryFile;
	TsString databaseDir;
	TsString tableDir;
	NotifierQueueRef notifier;

private:
	void ChooseDirectories();
	void OpenDatabase();

};

// Global application object
extern CGeobrowseApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEOBROWSE_H__4961EAB9_713B_11D4_B1E7_00105A0FCDF8__INCLUDED_)
