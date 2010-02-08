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
# $Rev: 58 $ 
# $Date: 2006-11-28 21:56:33 +0100 (Tue, 28 Nov 2006) $ 
*/

// geocoder.cpp : Defines the initialization routines for the DLL.
//

#include "../geocommon/Geocoder_Headers.h"
#if defined(WIN32)
#ifdef AFX_EXT_DLL
#include <afxdllx.h>
#endif // AFX_EXT_DLL
#endif // WIN32;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(WIN32)
#ifdef AFX_EXT_DLL
static AFX_EXTENSION_MODULE GeocoderDLL = { NULL, NULL };
#endif // AFX_EXT_DLL

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		PortfolioExplorer::GeoUtil::dllModuleInstance = hInstance;
#ifdef AFX_EXT_DLL
		TRACE0("Geocoder.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(GeocoderDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(GeocoderDLL);
#endif // AFX_EXT_DLL
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifdef AFX_EXT_DLL
		TRACE0("Geocoder.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(GeocoderDLL);
#endif // AFX_EXT_DLL
	}
	return 1;   // ok
}
#endif // WIN32;
