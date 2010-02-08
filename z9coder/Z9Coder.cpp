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

// JH_Z9Coder.cpp: Interface to Zip9 coding system

#include "GeoCoder_Headers.h"
#include "Z9Coder.h"
#include "Z9CoderImp.h"

using namespace PortfolioExplorer;
namespace PinpointExplorer {

	class Zip9CoderImp__ {
	public:
		Zip9CoderImp__() {
			results.Reset();
		}
		DeCompressZip4Ref coder;
		DeCompressZip4::Results results;
	};
	
	// Constructor
	Z9Coder::Z9Coder() 
	{
		imp = new Zip9CoderImp__;
		imp->coder = new DeCompressZip4;
	}

	// Destructor
	Z9Coder::~Z9Coder()
	{
		delete imp;
		imp = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// Open the Z9 coder
	// Inputs:
	//	const TsString&		directory		directory where data files are located
	// Outputs:
	//	TsString&			errorMsg		Error message produced on failure.
	// Return value:
	//	bool			true on success, false on failure.
	///////////////////////////////////////////////////////////////////////////
	bool Z9Coder::Open(
		const char * pFilename,
		char * errorMsg
	) {
		try {
			imp->coder->Open(pFilename);
			return true;
		}
		catch (const ErrorException & ex) 
		{
			strncpy(errorMsg, ex.message.c_str(), 256);
			errorMsg[255] = 0;
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Geocode a ZIP
	// Inputs:
	//	int			zip			The 5-digit ZIP code
	//	int			plus4		The 4-digit ZIP extension
	// Outputs:
	//	TsString&			errorMsg		Error message produced on failure.
	// Return value:
	//	bool		true if coded successfully, false o/w
	///////////////////////////////////////////////////////////////////////////
	bool Z9Coder::CodeZip(
		int zip, 
		int plus4,
		char * errorMsg
	) {
		try 
		{
			bool bRet = imp->coder->Find(zip, plus4, imp->results, true);
			if (bRet)
                strcpy(errorMsg, "Zip not found");
			return bRet;
		}
		catch (const ErrorException & ex) 
		{
			strncpy(errorMsg, ex.message.c_str(), 256);
			errorMsg[255] = 0;
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Methods to get census codes.  All return null-terminated strings.
	// Valid immediately after a successful call to CodeZip.
	///////////////////////////////////////////////////////////////////////////
	const char* Z9Coder::GetCensusID() {
		return imp->results.censusIDResultBuffer;
	}

	const char* Z9Coder::GetStateFips() {
		return imp->results.stateFipsResultBuffer;
	}

	const char* Z9Coder::GetCountyFips() {
		return imp->results.countyFipsResultBuffer;
	}

	const char* Z9Coder::GetTract() {
		return imp->results.tractResultBuffer;
	}

	const char* Z9Coder::GetBlock() {
		return imp->results.blockResultBuffer;
	}


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
	const char* Z9Coder::GetRecordType()
	{
		return imp->results.recTypeResultBuffer;
	}

	///////////////////////////////////////////////////////////////////////////
	// Get centroid type
	// Valid immediately after a successful call to CodeZip.
	// Return Value:
	//	9 - 9-digit ZIP
	//  7 - 7-digit ZIP
	//  5 - 5-digit ZIP
	//  0 - failed to code
	///////////////////////////////////////////////////////////////////////////
	const char* Z9Coder::GetCentroidType()
	{
		return imp->results.cenTypeResultBuffer;
	}

	///////////////////////////////////////////////////////////////////////////
	// Methods to get result latitude/longitude
	// Valid immediately after a successful call to CodeZip.
	///////////////////////////////////////////////////////////////////////////
	double Z9Coder::GetLatitude()
	{
		return imp->results.m_dLatResult;
	}
	double Z9Coder::GetLongitude()
	{
		return imp->results.m_dLongResult;
	}

}



