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

// Query.h: The standard file-based reference query object

#ifndef INCL_GeocoderPortfolioExplorerQueryExternal_H
#define INCL_GeocoderPortfolioExplorerQueryExternal_H

#include "../geocommon/Geocoder_DllExport.h"
#include "GeoQueryItf.h"
#include "../global/RefPtr.h"
#include "Geocoder.h"

namespace PortfolioExplorer {
	///////////////////////////////////////////////////////////////////////////
	class QueryImp;

	///////////////////////////////////////////////////////////////////////////
	// Reference query interface wrapper class.  Provides a query interface to 
	// applications outside of the geocoder.
	///////////////////////////////////////////////////////////////////////////
	class Query : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	const char*	tableDir			The directory containing extra geocoder tables
		//	const char*	databaseDir			The directory containing the geocoder database.
		//	MemUse					memUse	Parameter controlling memory usage.
		///////////////////////////////////////////////////////////////////////
		Query(
			const char* tableDir_,
			const char* databaseDir_,
			Geocoder::MemUse memUse
		);

		///////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////
		virtual ~Query();

		///////////////////////////////////////////////////////////////////////
		// Opens the reference query interface using the parameters
		// given in the subclass constructor.
		// Returns true on success, false on failure.
		///////////////////////////////////////////////////////////////////////
		bool Open();

		///////////////////////////////////////////////////////////////////////
		// Is the query object open?
		///////////////////////////////////////////////////////////////////////
		bool IsOpen();

		///////////////////////////////////////////////////////////////////////
		// Close the reference query interface.
		///////////////////////////////////////////////////////////////////////
		void Close();

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting CityStatePostcodes from a postal code
		///////////////////////////////////////////////////////////////////////
		class CityStatePostcodeFromPostcodeIterator {
			friend class Query;
		public:
			bool Next(CityStatePostcode& cityStatePostcodeReturn);
			~CityStatePostcodeFromPostcodeIterator();
		private:
			CityStatePostcodeFromPostcodeIterator(void* imp_);
			// Private implementation
			void* imp;
		};

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the list of 
		// associated CityStatePostcode records.
		///////////////////////////////////////////////////////////////////////
		CityStatePostcodeFromPostcodeIterator LookupCityStatePostcodeFromPostcode(
			const char* postcode
		);

		///////////////////////////////////////////////////////////////////////
		// Get a CityStatePostcode by ID.
		///////////////////////////////////////////////////////////////////////
		bool GetCityStatePostcode(
			int ID,
			CityStatePostcode& cityStatePostcodeReturn
		);

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the centroid.
		///////////////////////////////////////////////////////////////////////
		bool LookupPostcodeCentroid(
			const char* postcode,
			PostcodeCentroid& postcodeCentroidReturn
		);

	private:
		// Pointer to implementation class
		QueryImp* imp;
	};

	typedef refcnt_ptr<Query> QueryRef;
}

#endif
