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
# $Rev: 52 $ 
# $Date: 2006-10-06 05:33:29 +0200 (Fri, 06 Oct 2006) $ 
*/

// Query.cpp: External geocoder reference query interface

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "../geocommon/Geocoder_Headers.h"
#include "GeoQuery.h"
#include "GeoQueryImp.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////
	// Constructor
	// Inputs:
	//	const char*	tableDir			The directory containing extra geocoder tables
	//	const char*	databaseDir			The directory containing the geocoder database.
	//	MemUse					memUse	Parameter controlling memory usage.
	///////////////////////////////////////////////////////////////////////
	Query::Query(
		const char* tableDir_,
		const char* databaseDir_,
		Geocoder::MemUse memUse
	) {
		imp = new QueryImp(tableDir_, databaseDir_, memUse);
	}

	///////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////
	Query::~Query()
	{
		delete imp;
		imp = 0;
	}

	///////////////////////////////////////////////////////////////////////
	// Opens the reference query interface using the parameters
	// given in the subclass constructor.
	// Returns true on success, false on failure.
	///////////////////////////////////////////////////////////////////////
	bool Query::Open()
	{
		return imp->Open();
	}

	///////////////////////////////////////////////////////////////////////
	// Is the query object open?
	///////////////////////////////////////////////////////////////////////
	bool Query::IsOpen()
	{
		return imp->IsOpen();
	}

	///////////////////////////////////////////////////////////////////////
	// Close the reference query interface.
	///////////////////////////////////////////////////////////////////////
	void Query::Close()
	{
		imp->Close();
	}

	///////////////////////////////////////////////////////////////////////
	// Iterator for getting CityStatePostcodes from a postal code
	///////////////////////////////////////////////////////////////////////
	bool Query::CityStatePostcodeFromPostcodeIterator::Next(
		CityStatePostcode& cityStatePostcodeReturn
	) {
		return reinterpret_cast<QueryImp::CityStatePostcodeFromPostcodeIterator*>(imp)->Next(cityStatePostcodeReturn);
	}

	Query::CityStatePostcodeFromPostcodeIterator::~CityStatePostcodeFromPostcodeIterator() {
		delete reinterpret_cast<QueryImp::CityStatePostcodeFromPostcodeIterator*>(imp);
		imp = 0;
	}

	Query::CityStatePostcodeFromPostcodeIterator::CityStatePostcodeFromPostcodeIterator(void* imp_) : imp(imp_) {}

	///////////////////////////////////////////////////////////////////////
	// Given a postal code, find the list of 
	// associated CityStatePostcode records.
	///////////////////////////////////////////////////////////////////////
	Query::CityStatePostcodeFromPostcodeIterator 
	Query::LookupCityStatePostcodeFromPostcode(
		const char* postcode
	) {
		QueryImp::CityStatePostcodeFromPostcodeIterator iter = 
			imp->LookupCityStatePostcodeFromPostcode(postcode);
		return CityStatePostcodeFromPostcodeIterator(new QueryImp::CityStatePostcodeFromPostcodeIterator(iter));
	}

	///////////////////////////////////////////////////////////////////////
	// Get a CityStatePostcode by ID.
	///////////////////////////////////////////////////////////////////////
	bool Query::GetCityStatePostcode(
		int ID,
		CityStatePostcode& cityStatePostcodeReturn
	) {
		return imp->GetCityStatePostcode(ID, cityStatePostcodeReturn);
	}

	///////////////////////////////////////////////////////////////////////
	// Given a postal code, find the centroid.
	///////////////////////////////////////////////////////////////////////
	bool Query::LookupPostcodeCentroid(
		const char* postcode,
		PostcodeCentroid& postcodeCentroidReturn
	) {
		return imp->LookupPostcodeCentroid(postcode, postcodeCentroidReturn);
	}
}

