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
# $Rev: 122 $ 
# $Date: 2009-10-09 20:06:27 +0200 (Fri, 09 Oct 2009) $ 
*/

// Z9Coder.h: Interface to Zip9 coding system

#ifndef INCL_Z9CODER_H
#define INCL_Z9CODER_H


#ifdef GEOCODER_EXPORTS  
	#define ZIP9_EXPORT __declspec(dllexport)
#else
	#define ZIP9_EXPORT __declspec(dllimport)
#endif

namespace PinpointExplorer {
#ifdef _WIN64
	const wchar_t * const REG_FOLDER = L"SOFTWARE\\Wow6432Node\\SRC\\ZIP9Coder\\3.0";
#else 
	const wchar_t * const REG_FOLDER = L"SOFTWARE\\SRC\\ZIP9Coder\\3.0";
#endif

	class Zip9CoderImp__;
	
	class ZIP9_EXPORT Z9Coder {
	public:
		// Constructor
		Z9Coder();

		// Destructor
		virtual ~Z9Coder();

		///////////////////////////////////////////////////////////////////////////
		// Open the Z9 coder
		// Inputs:
		//	const TsString&		directory		directory where data files are located
		// Outputs:
		//	char * errorMsg Must be 256 characters to receive error message
		// Return value:
		//	bool			true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////
		bool Open(
			const char * pFilename,
			char * errorMsg
		);

		///////////////////////////////////////////////////////////////////////////
		// Geocode a ZIP
		// Inputs:
		//	int			zip			The 5-digit ZIP code
		//	int			plus4		The 4-digit ZIP extension
		// Outputs:
		//	char * errorMsg Must be 256 characters to receive error message
		// Return value:
		//	bool		true if coded successfully, false o/w
		///////////////////////////////////////////////////////////////////////////
		bool CodeZip(
			int zip, 
			int plus4,
			char * errorMsg
		);

		///////////////////////////////////////////////////////////////////////////
		// Methods to get census codes.  All return null-terminated strings.
		// Valid immediately after a successful call to CodeZip.
		///////////////////////////////////////////////////////////////////////////
		const char* GetCensusID();
		const char* GetStateFips();
		const char* GetCountyFips();
		const char* GetTract();
		const char* GetBlock();

		///////////////////////////////////////////////////////////////////////////
		// Get USPS record type as null-terminated string.
		// Valid immediately after a successful call to CodeZip.
		// Return Value:
		// H - high-rise
		// F - firm
		// S - street
		// R - rural route/highway contract
		// P - post office box
		// G - general delivery
		// Z - GDT 5-digit ZIP
		///////////////////////////////////////////////////////////////////////////
		const char* GetRecordType();

		///////////////////////////////////////////////////////////////////////////
		// Get centroid type
		// Valid immediately after a successful call to CodeZip.
		// Return Value:
		//	9 - 9-digit ZIP
		//  7 - 7-digit ZIP
		//  5 - 5-digit ZIP
		//  0 - failed to code
		///////////////////////////////////////////////////////////////////////////
		const char* GetCentroidType();

		///////////////////////////////////////////////////////////////////////////
		// Methods to get result latitude/longitude
		// Valid immediately after a successful call to CodeZip.
		///////////////////////////////////////////////////////////////////////////
		double GetLatitude();
		double GetLongitude();

	private:
		// Pointer to internal implementation
		Zip9CoderImp__* imp;
	};

	int ZIP9_EXPORT _stdcall Z9_Load_Create(const char *pFileName, char *pError);
	int ZIP9_EXPORT _stdcall Z9_Load_Add(int handle, int *aFields);
	void ZIP9_EXPORT _stdcall Z9_Load_FinalizeStatistics(int handle);
	int ZIP9_EXPORT _stdcall Z9_Load_Close(int handle, char *pError);
	
}


#endif
