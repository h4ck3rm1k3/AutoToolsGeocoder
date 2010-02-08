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

// Geocoder.cpp: Public interface file for geocoder library

#include "../geocommon/Geocoder_Headers.h"
#include "Geocoder.h"
#include "GeocoderImp.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////
	// Constructor.  
	// Inputs:
	//	const char*			tableDir	The geocoder directory containing the
	//									lookup tables for the address parser.
	//	const char*			databaseDir	The geocoder directory containing the
	//									geocoder database files.
	//	MemUse				memUse		Relative amount of memory to use for caching
	///////////////////////////////////////////////////////////////////////
	Geocoder::Geocoder(
		const char* tableDir,
		const char* databaseDir,
		MemUse memUse
	) {
		imp = new GeocoderImp(*this, tableDir, databaseDir, memUse);
	}

	// Destructor
	Geocoder::~Geocoder()
	{
		Close();
		delete imp;
		imp = 0;
	}

	// Error-message-receiving method.
	// Override this to intercept human-readable error messages.
	//virtual 
	void Geocoder::ErrorMessage(const char* message)
	{
		// Default implemntation does nothing
	}

#ifndef NDEBUG
	///////////////////////////////////////////////////////////////////////
	// Trace method for debugging.
	// Override this to redirect human-readable debug trace messages
	///////////////////////////////////////////////////////////////////////
	void Geocoder::TraceMessage(const char* message)
	{
	}
#endif

	// Open the Geocoder instance using the given query interface.
	// Will call Open() on the query interface object.
	// Returns true on success, false on failure.
	// Override ErrorMessage() to get message text.
	bool Geocoder::Open()
	{
		return imp->Open();
	}

	// Close the Geocoder instance.
	// This will be called by the destructor.
	void Geocoder::Close()
	{
		imp->Close();
	}

	///////////////////////////////////////////////////////////////////////
	// Code an address.  Call GetNextCandidate() to check results.
	// Inputs:
	//	const char*			line1			street address
	//	const char*			line2			city, state, zip
	// Return value:
	//	GlobalStatus		A status code indicating the overall result of
	//						the geocoding process
	///////////////////////////////////////////////////////////////////////
	Geocoder::GlobalStatus Geocoder::CodeAddress(
		const char* line1,
		const char* line2
	) {
		return imp->CodeAddress(line1, line2);
	}

	///////////////////////////////////////////////////////////////////////
	// Fetch candidate address interpretations for the last
	// call to CodeAddress().  The candidates will be returned in 
	// order of decreasing score.  This method returns false when
	// there are no more candidates.
	// Inputs:
	//	GeocodeResults&		resultsReturn	result of coding
	// Return value:
	//	bool		true if there is another result false o/w
	///////////////////////////////////////////////////////////////////////
	bool Geocoder::GetNextCandidate(
		GeocodeResults& resultsReturn		// result of coding
	) {
		return imp->GetNextCandidate(resultsReturn);
	}

	///////////////////////////////////////////////////////////////////////
	// Set the threshold score that determines a match (0-1000)
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetMatchThreshold(int threshold)
	{
		imp->SetMatchThreshold(threshold);
	}

	///////////////////////////////////////////////////////////////////////
	// Set the score delta that determines a multiple (0-1000)
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetMultipleThreshold(int threshold)
	{
		imp->SetMultipleThreshold(threshold);
	}

	///////////////////////////////////////////////////////////////////////
	// Set the offset of a coded address from the side of the street, feet.
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetStreetOffset(float offset)
	{
		imp->SetStreetOffset(offset);
	}

	///////////////////////////////////////////////////////////////////////
	// Set the minimum distance that a point may code from the end of a
	// street segment, in feet
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetStreetEndpointOffset(float offset)
	{
		imp->SetStreetEndpointOffset(offset);
	}

	///////////////////////////////////////////////////////////////////////
	// Set how the City/State/Postcode record that "owns" a street segment
	// is reconciled with with the City/State/Postcode that best matches 
	// the entered last-line.
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetStreetOwnerTreatment(StreetOwnerTreatment streetOwnerTreatment)
	{
		imp->SetStreetOwnerTreatment(streetOwnerTreatment);
	}

	///////////////////////////////////////////////////////////////////////
	// Convert a state abbreviation to a state FIPS code
	// Inputs:
	//	const char*		stateAbbr			The state abbreviation
	//	const char*		countryCode			The country code (US or CA)
	// Outputs:
	//	int&			stateCodeReturn		The resulting state code
	// Return value:
	//	bool			true if state exists, false o/w.
	///////////////////////////////////////////////////////////////////////
	bool Geocoder::StateAbbrToCode(
		const char* stateAbbr,
		const char* countryCode,
		int& stateCodeReturn
	) {
		return imp->StateAbbrToCode(stateAbbr, countryCode, stateCodeReturn);
	}

	///////////////////////////////////////////////////////////////////////
	// Convert a state FIPS code to a state abbeviation.
	// Inputs:
	//	int				stateCode			The state FIPS code
	//	const char*		countryCode			The country code (US or CA)
	// Outputs:
	//	 char*		stateAbbrReturn		The state abbreviation
	// Return value:
	//	bool			true if state exists, false o/w.
	///////////////////////////////////////////////////////////////////////
	bool Geocoder::StateCodeToAbbr(
		int stateCode,
		const char* countryCode,
		const char*& stateAbbrReturn
	) {
		return imp->StateCodeToAbbr(stateCode, countryCode, stateAbbrReturn);
	}

	///////////////////////////////////////////////////////////////////////
	// Set the minimum interpolation value
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetMinInterpolation(double _minInterpolation)
	{
		imp->SetMinInterpolation(_minInterpolation);
	}

	///////////////////////////////////////////////////////////////////////
	// Set the maximum interpolation value
	///////////////////////////////////////////////////////////////////////
	void Geocoder::SetMaxInterpolation(double _maxInterpolation)
	{
		imp->SetMaxInterpolation(_maxInterpolation);
	}

	///////////////////////////////////////////////////////////////////////
	// Check that the data and geocoder are the same version
	// Inputs:
	//  TsString		dataDir	Directory containing geocoder database files
	// Outputs:
	// Return value:
	//	bool			true if versions match, false o/w.
	///////////////////////////////////////////////////////////////////////
	bool Geocoder::CheckDataVersion(const char * pDataDir)
	{
		return GeocoderImp::CheckDataVersion(pDataDir);
	}
}

