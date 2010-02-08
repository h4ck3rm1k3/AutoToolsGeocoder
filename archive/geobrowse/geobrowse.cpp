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

// geobrowse.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "geobrowse.h"

#include "MainFrm.h"
#include "Registry.h"
#include <Shlwapi.h>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_USE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <time.h>

/////////////////////////////////////////////////////////////////////////////
// Special function to free memory allocated by SHBrowseForFolder.
// Typical Microsoft busy-work...
/////////////////////////////////////////////////////////////////////////////
static void ILFree(LPITEMIDLIST pidl)
{
	LPMALLOC pMalloc;
	if (pidl) { 
		SHGetMalloc(&pMalloc); 
		pMalloc->Free(pidl); 
		pMalloc->Release();      
	} 
}  

static void ChooseDirectory(
	const TsString& prompt,
	TsString& returnDir
) {
	// Locate geocoder data directory manually
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.lpszTitle = prompt.c_str();
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	ITEMIDLIST* idList;
	idList = SHBrowseForFolder(&bi);

	if (bi.pidlRoot) 
	{ 
		ILFree((LPITEMIDLIST)bi.pidlRoot); 
	} 
	if (idList != 0) {
		// User did not cancel
		// Copy to the edit box
		char path[MAX_PATH + 2];
		BOOL ok = SHGetPathFromIDList(idList, path);
		ILFree(idList);

		// Strip trailing \
		// Don't use PathRemoveBackslash, it doesn't link
		returnDir = path;
		while (!returnDir.empty() && *(returnDir.end() - 1) == '\\') {
			returnDir.resize(returnDir.size() - 1);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CGeobrowseApp

BEGIN_MESSAGE_MAP(CGeobrowseApp, CWinApp)
	//{{AFX_MSG_MAP(CGeobrowseApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TEST_READ_STREET_SEGMENTS, OnTestReadStreetSegments)
	ON_COMMAND(ID_FILE_RESET, OnFileReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeobrowseApp construction

CGeobrowseApp::CGeobrowseApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGeobrowseApp object

CGeobrowseApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGeobrowseApp initialization

BOOL CGeobrowseApp::InitInstance()
{
	// Initialize the XML4C system
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& )
	{
		AfxMessageBox("Xerces XML initialization failed");
		return false;
	}


	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Initialize the Objective Grid library
	GXInit();

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("PortfolioExplorer Geocoder Browser"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	// Make error-queue for application
	notifier = new NotifierQueue;

	OpenDatabase();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGeobrowseApp message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CGeobrowseApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGeobrowseApp message handlers


void CGeobrowseApp::OnTestReadStreetSegments() 
{
	time_t t1 = clock();
	for (int i = 0; i < 100000; i++) {
//	for (int i = 0; i < refQueryFile->GetStreetSegmentCount(); i++) {
		StreetSegment streetSegment;
		if (!refQueryFile->GetStreetSegmentByIDCached(i, streetSegment)) {
			AfxMessageBox("ERROR!");
			break;
		}
	}
	time_t t2 = clock();
	char buf[1000];
	sprintf(buf, "Time to read %d StreetSegment records is %g", refQueryFile->GetStreetSegmentCount(), (double)(t2-t1)/CLOCKS_PER_SEC);
	AfxMessageBox(buf);
}

void CGeobrowseApp::OnFileReset() 
{
	ChooseDirectories();
	OpenDatabase();
}

void CGeobrowseApp::ChooseDirectories() 
{
	ChooseDirectory("Select the address parsing table directory", tableDir);
	ChooseDirectory("Select the geocoder database directory", databaseDir);

	// Remember the directories in the registry
	Registry::SetString("HKEY_LOCAL_MACHINE\\Software\\Datalever\\geobrowse\\TableDirectory", tableDir);
	Registry::SetString("HKEY_LOCAL_MACHINE\\Software\\Datalever\\geobrowse\\DatabaseDirectory", databaseDir);
}

void CGeobrowseApp::OpenDatabase() 
{
	bool opened = false;
	// Retrieve stored data directory.
	if (
		Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\Datalever\\geobrowse\\TableDirectory", tableDir) &&
		Registry::GetString("HKEY_LOCAL_MACHINE\\Software\\Datalever\\geobrowse\\DatabaseDirectory", databaseDir)
	) {
		refQueryFile = new RefQueryFileWithErrorHook(tableDir, databaseDir, notifier.get());
		if (refQueryFile->Open()) {
			opened = true;
		}
	}

	if (!opened) {
		ChooseDirectories();
		// Try opening again
		refQueryFile = new RefQueryFileWithErrorHook(tableDir, databaseDir, notifier.get());
	}

	if (!refQueryFile->Open()) {
		AfxMessageBox("Error opening geocoder data.  See message log");
	}
}

