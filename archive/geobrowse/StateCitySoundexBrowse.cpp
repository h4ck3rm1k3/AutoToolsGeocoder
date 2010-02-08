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

// StateCitySoundexBrowse.cpp

#include "stdafx.h"
#include "geobrowse.h"
#include "StateCitySoundexBrowse.h"

#pragma warning(disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStateCitySoundexBrowse
/////////////////////////////////////////////////////////////////////////////
CStateCitySoundexBrowse::CStateCitySoundexBrowse(
	QueryImpRef refQueryFileRef_
) :
	refQueryFileRef(refQueryFileRef_),
	initialized(false)
{
}

CStateCitySoundexBrowse::~CStateCitySoundexBrowse()
{
}


BEGIN_MESSAGE_MAP(CStateCitySoundexBrowse,CWnd )
	//{{AFX_MSG_MAP(CStateCitySoundexBrowse)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CStateCitySoundexBrowse::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(&rect, RGB(192, 192, 192));
	
	// Do not call CWnd::OnPaint() for painting messages
}


/////////////////////////////////////////////////////////////////////////////
// Crate the window and all child windows.
/////////////////////////////////////////////////////////////////////////////
void CStateCitySoundexBrowse::Initialize()
{
	assert(theApp.refQueryFile != 0);

	// grid
	grid.Create(
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
		CRect(0, 0, 400, 100),
		this,
		2
	);
	grid.Initialize(refQueryFileRef);
	grid.GetParam()->EnableUndo(FALSE);

	initialized = true;

	// Cause initial placement to happen
	CRect rect;
	GetClientRect(rect);
	HandleSize(rect.right, rect.bottom);
}

void CStateCitySoundexBrowse::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if (initialized) {
		HandleSize(cx, cy);
	}
}

void CStateCitySoundexBrowse::HandleSize(int cx, int cy) 
{
	// grid
	grid.MoveWindow(10, 10, cx - 20, cy - 20, TRUE);
}


