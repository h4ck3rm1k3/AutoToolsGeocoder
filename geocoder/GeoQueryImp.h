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
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

// RefQuery.h: The standard file-based reference query object

#ifndef INCL_RefQueryFile_H
#define INCL_RefQueryFile_H

#include <stdio.h>

#include "../geocommon/Geocoder_DllExport.h"

#include "Geocoder.h"
#include "../geocommon/GeoUtil.h"
#include "../global/LookupTable.h"
#include "GeoQueryItf.h"
#include "../geocommon/GeoDataInput.h"
#include "../global/SetAssocCache.h"

namespace PortfolioExplorer {
	///////////////////////////////////////////////////////////////////////////
	// Internal query interface class.  Provides a query interface to the 
	// geocoder that can be implemented using any of several methods.
	// This implementation is built upon an indexed compressed file structure.
	///////////////////////////////////////////////////////////////////////////
	class QueryImp : public GeoUtil, public VRefCount {

	public:
		// nested forward decl and friends.
		class StreetNameFromFaStreetIterator;
		friend class StreetNameFromFaStreetIterator;

#ifndef COMPILE_GEOBROWSE
// A terrible cheat, but too many browser classes need these structs.
	private:
#endif
		// Auxilliary private data structures
		struct StreetNameSoundex {
			int ID;
			char financeNumber[7];		// USPS FA or three letters of Canada postcode
			int streetNameID;
			char streetSoundex[5];
		};

		struct CityStatePostcodeSoundex {
			int state;
			char citySoundex[5];
			int cityStatePostcodeID;
		};

		struct StreetIntersectionSoundex {
			int state;
			char streetSoundex1[5];
			int streetNameID1;
			int streetSegmentOffset1;
			char streetSoundex2[5];
			int streetNameID2;
			int streetSegmentOffset2;
		};

	public:
		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		// START GEOCODER INTERFACE
		// Only items in this section are part of the standard query interface.
		// The Geocoder should only rely upon these methods.
		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////
		virtual ~QueryImp();

		///////////////////////////////////////////////////////////////////////
		// Opens the reference query interface using the parameters
		// given in the subclass constructor.
		// Returns true on success, false on failure.
		///////////////////////////////////////////////////////////////////////
		bool Open();

		///////////////////////////////////////////////////////////////////////
		// Is the query object open?
		///////////////////////////////////////////////////////////////////////
		bool IsOpen() const { return isOpen; }

		///////////////////////////////////////////////////////////////////////
		// Close the reference query interface.
		///////////////////////////////////////////////////////////////////////
		void Close();

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting CityStatePostcodes from a postal code
		///////////////////////////////////////////////////////////////////////
		class CityStatePostcodeFromPostcodeIterator {
			friend class QueryImp;
		public:
			CityStatePostcodeFromPostcodeIterator() {}
			bool Next(CityStatePostcode& cityStatePostcodeReturn) {
				if (
					!queryImp->GetCityStatePostcodeByIDCached(current, cityStatePostcodeReturn) ||
					strcmp(cityStatePostcodeReturn.postcode, postcode) != 0
				) {
					return false;
				}
				current++;
				return true;
			}
		private:
			// Valid iterator
			CityStatePostcodeFromPostcodeIterator(
				QueryImp* queryImp_,
				const char *postcode_, 
				int current_
			) :
				queryImp(queryImp_),
				current(current_)
			{
				strncpy(postcode, postcode_, 6);
				postcode[6] = 0;
			}

			// Invalid iterator
			CityStatePostcodeFromPostcodeIterator(QueryImp* queryImp_) :
				queryImp(queryImp_),
				current(-1)
			{}

			QueryImp* queryImp;
			char postcode[7];	// the postal code being searched (null-terminated) 
			int current;		// the current CityStatePostcodeID
		};

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the list of 
		// associated CityStatePostcode records.
		///////////////////////////////////////////////////////////////////////
		CityStatePostcodeFromPostcodeIterator LookupCityStatePostcodeFromPostcode(
			const char* postcode
		);

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting all of the postal codes that are in the same
		// alias group as the given postal code (including the given postal code itself).
		///////////////////////////////////////////////////////////////////////
		class PostcodeAliasIterator {
			friend class QueryImp;
		public:
			bool Next(PostcodeAlias& postcodeAliasReturn) {
				while (true) {
					if (singlePostcodeGroup) {
						// Special case for postcode with no group.
						strcpy(postcodeAliasReturn.postcode, postcodeGroup);
						strcpy(postcodeAliasReturn.postcodeGroup, postcodeGroup);
						singlePostcodeGroup = false;
						return true;
					} else if (
						!queryImp->GetPostcodeAliasByGroupIDCached(current, postcodeAliasReturn) ||
						strcmp(postcodeAliasReturn.postcodeGroup, postcodeGroup) != 0
					) {
						return false;
					}
					current++;
					return true;
				}
				return false;
			}
		private:
			// Valid iterator
			PostcodeAliasIterator(
				QueryImp* queryImp_,
				const char *postcodeGroup_, 
				int current_
			) :
				queryImp(queryImp_),
				singlePostcodeGroup(false),
				current(current_)
			{
				strcpy(postcodeGroup, postcodeGroup_);
			}

			// Iterator for special case for postcode with no group.
			PostcodeAliasIterator(
				QueryImp* queryImp_,
				const char* postcode
			) :
				queryImp(queryImp_),
				singlePostcodeGroup(true),
				current(-1)
			{
				strcpy(postcodeGroup, postcode);
			}

			QueryImp* queryImp;
			bool singlePostcodeGroup;	// if true then use the single postal code as a group.
			char postcodeGroup[7];	// the postal code group being searched (null-terminated)
			int current;			// the current PostcodeAliasByGroup record position.
		};

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the list of all aliased postal codes, defined
		// as those postal codes that are aliases of each other.  Do not return
		// the original postal code.
		///////////////////////////////////////////////////////////////////////
		PostcodeAliasIterator LookupPostcodeAliases(
			const char* postcode
		);


		///////////////////////////////////////////////////////////////////////
		// Iterator for getting CityStatePostcodes given 
		// state FIPs code and city soundex
		///////////////////////////////////////////////////////////////////////
		class CityStatePostcodeFromStateCityIterator {
			friend class QueryImp;
		public:
			CityStatePostcodeFromStateCityIterator () {}
			bool Next(CityStatePostcode& cityStatePostcodeReturn) {
				if (
					!queryImp->GetCityStatePostcodeSoundexByIDCached(current, cityStatePostcodeSoundexTmp) ||
					cityStatePostcodeSoundexTmp.state != state ||
					strcmp(cityStatePostcodeSoundexTmp.citySoundex, soundex) != 0
				) {
					// Ran out of matching soundex entries.
					return false;
				}
				if (
					!queryImp->GetCityStatePostcodeByIDCached(cityStatePostcodeSoundexTmp.cityStatePostcodeID, cityStatePostcodeReturn)
				) {
					// Cannot read indexed entry.  This is a pretty bad problem, indicating
					// corrupt db or OS-level failure.
					return false;
				}
				current++;
				return true;
			}
		private:
			// Valid iterator
			CityStatePostcodeFromStateCityIterator (
				QueryImp* queryImp_,
				int state_,
				const char* citySoundex_,
				int current_
			) :
				queryImp(queryImp_),
				state(state_),
				current(current_)
			{
				assert(strlen(citySoundex_) == 4);
				strcpy(soundex, citySoundex_);
			}

			// Invalid iterator
			CityStatePostcodeFromStateCityIterator (
				QueryImp* queryImp_
			) :
				queryImp(queryImp_),
				current(-1)
			{
				strcpy(soundex, "");
			}

			QueryImp* queryImp;
			int state;			// state being searched
			char soundex[5];	// soundex value
			int current;		// the current CityStatePostcodeID
			CityStatePostcodeSoundex cityStatePostcodeSoundexTmp;
		};

		///////////////////////////////////////////////////////////////////////
		// Given a state FIPS code and city soundex, find
		// the associated CityStatePostcode records.
		///////////////////////////////////////////////////////////////////////
		CityStatePostcodeFromStateCityIterator LookupCityStatePostcodeFromStateCity(
			int stateCode,
			const char* citySoundex
		);

		///////////////////////////////////////////////////////////////////////
		// Get a CityStatePostcode by ID.
		///////////////////////////////////////////////////////////////////////
		bool GetCityStatePostcode(
			int ID,
			CityStatePostcode& cityStatePostcodeReturn
		) {
			return GetCityStatePostcodeByIDCached(ID, cityStatePostcodeReturn);
		}

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting StreetNames from FA and street
		///////////////////////////////////////////////////////////////////////
		class StreetNameFromFaStreetIterator {
			friend class QueryImp;
		public:
			StreetNameFromFaStreetIterator() {}
			bool Next(StreetName& streetNameReturn) {
				if (
					!queryImp->GetStreetNameSoundexByIDCached(current, streetNameSoundexTmp) ||
					strcmp(streetNameSoundexTmp.financeNumber, financeNumber) != 0 ||
					strcmp(streetNameSoundexTmp.streetSoundex, soundex) != 0
				) {
					// Ran out of matching soundex entries.
					return false;
				}
				if (
					!queryImp->GetStreetNameByIDCached(streetNameSoundexTmp.streetNameID, streetNameReturn)
				) {
					// Cannot read indexed entry.  This is a pretty bad problem.
					return false;
				}
				current++;
				return true;
			}
		private:
			// Valid iterator
			StreetNameFromFaStreetIterator(
				QueryImp* queryImp_,
				const char* financeNumber_,
				const char* streetSoundex_,
				int current_
			) :
				queryImp(queryImp_),
				current(current_)
			{
				assert(strlen(streetSoundex_) == 4);
				strcpy(soundex, streetSoundex_);
				strcpy(financeNumber, financeNumber_);
			}

			// Invalid iterator
			StreetNameFromFaStreetIterator(QueryImp* queryImp_) :
				queryImp(queryImp_),
				current(-1)
			{
				strcpy(soundex, "");
			}

			QueryImp* queryImp;
			char financeNumber[7];		// finance number being searched
			char soundex[5];			// soundex value
			int current;				// the current StreetNameID
			StreetNameSoundex streetNameSoundexTmp;
		};

		///////////////////////////////////////////////////////////////////////
		// Given Finance Number and soundex-of-street-name, find 
		// a list of StreetName records and associated CityStatePostcode
		///////////////////////////////////////////////////////////////////////
		StreetNameFromFaStreetIterator LookupStreetNameFromFaStreet(
			const char* financeNumber,
			const char* streetSoundex
		);

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting StreetSegments from StreetName
		///////////////////////////////////////////////////////////////////////
		class StreetSegmentFromStreetNameIterator {
			friend class QueryImp;
		public:
			StreetSegmentFromStreetNameIterator() {}
			bool Next(StreetSegment& streetSegmentReturn) {
				if (current >= last) {
					return false;
				}
				return queryImp->GetStreetSegmentByIDCached(current++, streetSegmentReturn);
			}
		private:
			// Valid iterator
			StreetSegmentFromStreetNameIterator(
				QueryImp* queryImp_,
				const StreetName& streetName
			) :
				queryImp(queryImp_),
				first(streetName.streetSegmentIDFirst),
				last(streetName.streetSegmentIDFirst + streetName.streetSegmentCount),
				current(streetName.streetSegmentIDFirst)
			{}

			// Invalid iterator
			StreetSegmentFromStreetNameIterator(QueryImp* queryImp_) :
				queryImp(queryImp_),
				last(0),
				current(1)
			{}

			QueryImp* queryImp;
			int first;
			int last;
			int current;
		};

		///////////////////////////////////////////////////////////////////////
		// Given a StreetName record, find the list of all 
		// associated StreetSegment records.
		// An optional address number is supplied as an optimization hint
		// to restrict the set of records read.
		///////////////////////////////////////////////////////////////////////
		StreetSegmentFromStreetNameIterator LookupStreetSegmentFromStreetName(
			const StreetName& streetName,
			const char* addrNbr = 0
		) {
			return StreetSegmentFromStreetNameIterator(this, streetName);
		}

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting CoordinatePoints from StreetSegment
		///////////////////////////////////////////////////////////////////////
		class CoordinatePointsFromStreetSegmentIterator {
			friend class QueryImp;
		public:
			CoordinatePointsFromStreetSegmentIterator() {}
			bool Next(CoordinatePoint& coordinateReturn) {
				if (current >= last) {
					return false;
				}
				return queryImp->GetCoordinateByIDCached(current++, coordinateReturn);
			}
			// Other methods implemented as necessary
		private:
			// Valid iterator
			CoordinatePointsFromStreetSegmentIterator(
				QueryImp* queryImp_,
				const StreetSegment& streetSegment
			) :
				queryImp(queryImp_),
				first(streetSegment.coordinateID),
				last(streetSegment.coordinateID + streetSegment.coordinateCount),
				current(streetSegment.coordinateID)
			{}
			
			// Invalid iterator
			CoordinatePointsFromStreetSegmentIterator(QueryImp* queryImp_) :
				queryImp(queryImp_),
				last(0),
				current(1)
			{}

			QueryImp* queryImp;
			int first;
			int last;
			int current;
		};

		///////////////////////////////////////////////////////////////////////
		// Given a StreetSegment  record, retrieve the list 
		// of latitude/longitude pairs for the street segment.
		///////////////////////////////////////////////////////////////////////
		CoordinatePointsFromStreetSegmentIterator
		LookupCoordinatePointsFromStreetSegment(
			const StreetSegment& streetSegment
		) {
			return CoordinatePointsFromStreetSegmentIterator(this, streetSegment);
		}

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting StreetIntersection from 
		// Finance Area and street soundexes
		///////////////////////////////////////////////////////////////////////
		class StreetIntersectionIterator {
			friend class QueryImp;
		public:
			StreetIntersectionIterator() {}
			bool Next(StreetIntersection& streetIntersectionReturn) {
				if (
					!queryImp->GetStreetIntersectionSoundexByIDCached(current, intersectionSoundexTmp) ||
					strcmp(intersectionSoundexTmp.streetSoundex1, soundex1) != 0 ||
					strcmp(intersectionSoundexTmp.streetSoundex2, soundex2) != 0
				) {
					// Ran out of matching soundex entries.
					return false;
				}
				if (
					!queryImp->GetStreetIntersectionFromStreetIntersectionSoundex(intersectionSoundexTmp, streetIntersectionReturn)
				) {
					// Cannot read indexed entry.  This is a pretty bad problem.
					return false;
				}
				current++;
				return true;
			}
		private:
			// Valid iterator 
			StreetIntersectionIterator(
				QueryImp* queryImp_,
				int state_,
				const char* streetSoundex1_,
				const char* streetSoundex2_,
				int current_
			) :
				queryImp(queryImp_),
				state(state_),
				current(current_)
			{
				assert(strlen(streetSoundex1_) == 4);
				assert(strlen(streetSoundex2_) == 4);
				strcpy(soundex1, streetSoundex1_);
				strcpy(soundex2, streetSoundex2_);
			}

			// Invalid iterator 
			StreetIntersectionIterator(QueryImp* queryImp_) : 
				queryImp(queryImp_),
				current(-1) 
			{}

			QueryImp* queryImp;
			int state;				// state being searched
			char soundex1[5];		// street1 soundex value
			char soundex2[5];		// street1 soundex value
			int current;			// the current StreetIntersectionID
			StreetIntersectionSoundex intersectionSoundexTmp;
		};

		///////////////////////////////////////////////////////////////////////
		// Given the soundex for two streets, find the intersections
		///////////////////////////////////////////////////////////////////////
		StreetIntersectionIterator LookupStreetIntersection(
			int stateCode,		// state FIPS code
			const char* street1Soundex,
			const char* street2Soundex
		);

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the associated postal code alias group.
		///////////////////////////////////////////////////////////////////////
		bool LookupPostcodeGroup(
			const char* postcode,
			PostcodeAlias& postcodeAliasReturn
		) {
			if (!GetPostcodeGroupFromPostcodeCached(postcode, postcodeAliasReturn)) {
				// If not found, treat the postcode like its own group.
				strcpy(postcodeAliasReturn.postcode, postcode);
				strcpy(postcodeAliasReturn.postcodeGroup, postcode);
			} 
			return true;
		}


		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the centroid.
		///////////////////////////////////////////////////////////////////////
		bool LookupPostcodeCentroid(
			const char* postcode,
			PostcodeCentroid& postcodeCentroidReturn
		) {
			return GetPostcodeCentroidFromPostcodeCached(postcode, postcodeCentroidReturn);
		}

		///////////////////////////////////////////////////////////////////////
		// Iterator for getting CityStatePostcodeFaIndex records from a postal code
		///////////////////////////////////////////////////////////////////////
		class CityStatePostcodeFaIndexFromFaIterator {
			friend class QueryImp;
		public:
			CityStatePostcodeFaIndexFromFaIterator() {}
			bool Next(CityStatePostcodeFaIndex& CityStatePostcodeFaIndexReturn) {
				if (
					!queryImp->GetCityStatePostcodeFaIndexByIDCached(current, CityStatePostcodeFaIndexReturn) ||
					strcmp(CityStatePostcodeFaIndexReturn.financeNumber, financeNumber) != 0
				) {
					return false;
				}
				current++;
				return true;
			}
		private:
			// Valid iterator
			CityStatePostcodeFaIndexFromFaIterator(
				QueryImp* queryImp_,
				const char *financeNumber_, 
				int current_
			) :
				queryImp(queryImp_),
				current(current_)
			{
				strncpy(financeNumber, financeNumber_, 6);
				financeNumber[6] = 0;
			}

			// Invalid iterator
			CityStatePostcodeFaIndexFromFaIterator(QueryImp* queryImp_) :
				queryImp(queryImp_),
				current(-1)
			{}

			QueryImp* queryImp;
			char financeNumber[7];	// the finance area being searched (null-terminated) 
			int current;		// the current CityStatePostcodeFaIndexID
		};


		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the CityStatePostcodeFaIndex
		///////////////////////////////////////////////////////////////////////
		CityStatePostcodeFaIndexFromFaIterator LookupCityStatePostcodeFaIndexFromFa(
			const char* finance
		) {
			CityStatePostcodeFaIndex cityStatePostcodeFaIndex;
			if (GetCityStatePostcodeFaIndexFromFa(finance, cityStatePostcodeFaIndex)) {
				return CityStatePostcodeFaIndexFromFaIterator(this, finance, cityStatePostcodeFaIndex.ID);
			} else {
				return CityStatePostcodeFaIndexFromFaIterator(this);
			}
		}


		///////////////////////////////////////////////////////////////////////
		// Determine the country of a postal code.  Returns the empty string
		// if no inference can be made.
		///////////////////////////////////////////////////////////////////////
		const char* CountryFromPostalCode(
			const char* postalCode
		) {
			return isdigit(*postalCode) ? "US" : (isalpha(*postalCode) ? "CA" : "");
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
		bool StateAbbrToCode(
			const char* stateAbbr,
			const char* countryCode,
			int& stateCodeReturn
		);

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
		bool StateCodeToAbbr(
			int stateCode,
			const char* countryCode,
			const char*& stateAbbrReturn
		);


		///////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////
		// END OF GEOCODER INTERFACE
		// Methods below here should not be called by the Geocoder.
		///////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	const TsString&	tableDir	The directory containing extra geocoder tables
		//	const TsString&	databaseDir	The directory containing the geocoder database.
		//	MemUse				memUse		Parameter controlling memory usage.
		///////////////////////////////////////////////////////////////////////
		QueryImp(
			const TsString& tableDir_,
			const TsString& databaseDir_,
			Geocoder::MemUse memUse
		);

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the position of the first CityStatePostcodeFaIndex
		// record, or -1 if no such record exists.
		///////////////////////////////////////////////////////////////////////
		bool GetCityStatePostcodeFaIndexFromFa(
			const char* finance,
			CityStatePostcodeFaIndex& cityStatePostcodeFaIndexReturn
		);
		bool GetCityStatePostcodeFaIndexFromFaCached(
			const char* finance,
			CityStatePostcodeFaIndex& cityStatePostcodeFaIndexReturn
		) {
			if (cityStatePostcodeFaIndexFromFaCache->Fetch(FinanceKey(finance), cityStatePostcodeFaIndexReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetCityStatePostcodeFaIndexFromFa(finance, cityStatePostcodeFaIndexReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Given the ID (postition) of a CityStatePostcodeFaIndex record, get the record.
		// Cached and non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetCityStatePostcodeFaIndexByID(
			int cityStatePostcodeFaIndexID,
			CityStatePostcodeFaIndex& cityStatePostcodeFaIndexReturn
		);
		bool GetCityStatePostcodeFaIndexByIDCached(
			int cityStatePostcodeFaIndexID,
			CityStatePostcodeFaIndex& cityStatePostcodeFaIndexReturn
		) {
			if (cityStatePostcodeFaIndexByIDCache->Fetch(IntKey(cityStatePostcodeFaIndexID), cityStatePostcodeFaIndexReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetCityStatePostcodeFaIndexByID(cityStatePostcodeFaIndexID, cityStatePostcodeFaIndexReturn);
			}
		}


		///////////////////////////////////////////////////////////////////////
		// Get the number of CityStatePostcode records.
		///////////////////////////////////////////////////////////////////////
		int GetCityStatePostcodeCount() const {
			return cityStatePostcodeCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a CityStatePostcode record by ID, cached and non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetCityStatePostcodeByID(
			int cityStatePostcodeID,
			CityStatePostcode& cityStatePostcodeReturn
		);
		bool GetCityStatePostcodeByIDCached(
			int cityStatePostcodeID,
			CityStatePostcode& cityStatePostcodeReturn
		) {
			if (cityStatePostcodeByIDCache->Fetch(IntKey(cityStatePostcodeID), cityStatePostcodeReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetCityStatePostcodeByID(cityStatePostcodeID, cityStatePostcodeReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of PostcodeAlias records
		///////////////////////////////////////////////////////////////////////
		unsigned GetPostcodeAliasCount() const { 
			return postcodeAliasCount; 
		}

		///////////////////////////////////////////////////////////////////////
		// Get a PostcodeAlias record by ID (position) from the PostcodeAlias records
		// sorted by Group, cached and non-cached versions
		///////////////////////////////////////////////////////////////////////
		bool GetPostcodeAliasByGroupID(
			int postcodeAliasID,
			PostcodeAlias& postcodeAliasReturn
		);
		bool GetPostcodeAliasByGroupIDCached(
			int postcodeAliasID,
			PostcodeAlias& postcodeAliasReturn
		) {
			if (postcodeAliasByGroupIDCache->Fetch(IntKey(postcodeAliasID), postcodeAliasReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetPostcodeAliasByGroupID(postcodeAliasID, postcodeAliasReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get a PostcodeAlias record by ID (position) from the PostcodeAlias records
		// sorted by postal code, cached and non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetPostcodeAliasByPostcodeID(
			int postcodeAliasID,
			PostcodeAlias& postcodeAliasReturn
		);
		bool GetPostcodeAliasByPostcodeIDCached(
			int postcodeAliasID,
			PostcodeAlias& postcodeAliasReturn
		) {
			if (postcodeAliasByPostcodeIDCache->Fetch(IntKey(postcodeAliasID), postcodeAliasReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetPostcodeAliasByPostcodeID(postcodeAliasID, postcodeAliasReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the associated postal code group, cached and
		// non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetPostcodeGroupFromPostcode(
			const char* postcode,
			PostcodeAlias& postcodeGroupReturn
		);
		bool GetPostcodeGroupFromPostcodeCached(
			const char* postcode,
			PostcodeAlias& postcodeGroupReturn
		) {
			if (postcodeGroupFromPostcodeCache->Fetch(PostcodeKey(postcode), postcodeGroupReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetPostcodeGroupFromPostcode(postcode, postcodeGroupReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Given a psotal code group, find the ID (position) of the first matching 
		// record in the PostcodeAliasByGroup table, cached and non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetPostcodeGroupIDFromPostcodeGroup(
			const char* postcodeGroup,
			int& postcodeGroupIDReturn
		);
		bool GetPostcodeGroupIDFromPostcodeGroupCached(
			const char* postcodeGroup,
			int& postcodeGroupIDReturn
		) {
			if (postcodeGroupIDFromPostcodeGroupCache->Fetch(PostcodeKey(postcodeGroup), postcodeGroupIDReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetPostcodeGroupIDFromPostcodeGroup(postcodeGroup, postcodeGroupIDReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Read a centroid given its positional ID.
		///////////////////////////////////////////////////////////////////////
		bool GetPostcodeCentroidByID(
			int postcodeCentroidID,
			PostcodeCentroid& postcodeCentroidReturn
		);
		bool GetPostcodeCentroidByIDCached(
			int postcodeCentroidID,
			PostcodeCentroid& postcodeCentroidReturn
		) {
			if (postcodeCentroidByIDCache->Fetch(IntKey(postcodeCentroidID), postcodeCentroidReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetPostcodeCentroidByID(postcodeCentroidID, postcodeCentroidReturn);
			}
		}


		///////////////////////////////////////////////////////////////////////
		// Given a postal code, find the associated centroid, cached and
		// non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetPostcodeCentroidFromPostcode(
			const char* postcode,
			PostcodeCentroid& postcodeCentroidReturn
		);
		bool GetPostcodeCentroidFromPostcodeCached(
			const char* postcode,
			PostcodeCentroid& postcodeCentroidReturn
		) {
			if (postcodeCentroidFromPostcodeCache->Fetch(PostcodeKey(postcode), postcodeCentroidReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetPostcodeCentroidFromPostcode(postcode, postcodeCentroidReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of CityStatePostcodeSoundex entries.
		///////////////////////////////////////////////////////////////////////
		int GetCityStatePostcodeSoundexCount() const {
			return cityStatePostcodeSoundexCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a CityStatePostcodeSoundex entry.
		///////////////////////////////////////////////////////////////////////
		bool GetCityStatePostcodeSoundexByID(
			int cityStatePostcodeSoundexID,
			CityStatePostcodeSoundex& cityStatePostcodeSoundexReturn
		);
		bool GetCityStatePostcodeSoundexByIDCached(
			int cityStatePostcodeSoundexID,
			CityStatePostcodeSoundex& cityStatePostcodeSoundexReturn
		) {
			if (cityStatePostcodeSoundexIDCache->Fetch(IntKey(cityStatePostcodeSoundexID), cityStatePostcodeSoundexReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetCityStatePostcodeSoundexByID(cityStatePostcodeSoundexID, cityStatePostcodeSoundexReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of StreetName records.
		///////////////////////////////////////////////////////////////////////
		int GetStreetNameCount() {
			return streetNameCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a StreetName record by ID, cached and uncached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetStreetNameByID(
			int streetNameID,
			StreetName& streetNameReturn
		);
		bool GetStreetNameByIDCached(
			int streetNameID,
			StreetName& streetNameReturn
		) {
			if (streetNameIDCache->Fetch(IntKey(streetNameID), streetNameReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetStreetNameByID(streetNameID, streetNameReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of StreetNameSoundex entries.
		///////////////////////////////////////////////////////////////////////
		int GetStreetNameSoundexCount() {
			return streetNameSoundexCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a StreetNameSoundex record by ID, cached and uncached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetStreetNameSoundexByID(
			int streetNameSoundexID,
			StreetNameSoundex& streetNameSoundexReturn
		);
		bool GetStreetNameSoundexByIDCached(
			int streetNameSoundexID,
			StreetNameSoundex& streetNameSoundexReturn
		) {
			if (streetNameSoundexIDCache->Fetch(IntKey(streetNameSoundexID), streetNameSoundexReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetStreetNameSoundexByID(streetNameSoundexID, streetNameSoundexReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Find the first StreetNameSoundex record by Finance Number and 
		// Street Name soundex.  Return false if none are found.
		///////////////////////////////////////////////////////////////////////
		bool FindStreetNameSoundexByFaSoundex(
			const char* financeNumber,
			const char* soundex,
			StreetNameSoundex& streetNameSoundexReturn
		);
		// Cached version
		bool FindStreetNameSoundexByFaSoundexCached(
			const char* financeNumber,
			const char* soundex,
			StreetNameSoundex& streetNameSoundexReturn
		) {
			if (streetNameSoundexFaSoundexCache->Fetch(
					StreetNameSoundexFaSoundexKey(financeNumber, soundex), 
					streetNameSoundexReturn
				)
			) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return FindStreetNameSoundexByFaSoundex(financeNumber, soundex, streetNameSoundexReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of StreetSegment records.
		///////////////////////////////////////////////////////////////////////
		int GetStreetSegmentCount() {
			return streetSegmentCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a StreetSegment record by ID, cached and uncached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetStreetSegmentByID(
			int streetSegmentID,
			StreetSegment& streetSegmentReturn
		);
		bool GetStreetSegmentByIDCached(
			int streetSegmentID,
			StreetSegment& streetSegmentReturn
		) {
			if (streetSegmentIDCache->Fetch(IntKey(streetSegmentID), streetSegmentReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader. 
				return GetStreetSegmentByID(streetSegmentID, streetSegmentReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of Coordinate records
		///////////////////////////////////////////////////////////////////////
		int GetCoordinateCount() {
			return coordinateCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a Coordinate record by ID, cached and non-cached versions.
		///////////////////////////////////////////////////////////////////////
		bool GetCoordinateByID(
			int coordinateID,
			CoordinatePoint& coordinateReturn
		);
		bool GetCoordinateByIDCached(
			int coordinateID,
			CoordinatePoint& coordinateReturn
		) {
			if (coordinateIDCache->Fetch(IntKey(coordinateID), coordinateReturn)) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetCoordinateByID(coordinateID, coordinateReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the number of StreetIntersection records
		///////////////////////////////////////////////////////////////////////
		int GetStreetIntersectionSoundexCount() {
			return streetIntersectionSoundexCount;
		}

		///////////////////////////////////////////////////////////////////////
		// Get a StreetIntersectionSoundex record by ID, cached and uncached
		// versions.
		///////////////////////////////////////////////////////////////////////
		bool GetStreetIntersectionSoundexByID(
			int streetIntersectionSoundexID,
			StreetIntersectionSoundex& streetIntersectionSoundexReturn
		);
		bool GetStreetIntersectionSoundexByIDCached(
			int streetIntersectionSoundexID,
			StreetIntersectionSoundex& streetIntersectionSoundexReturn
		) {
			if (streetIntersectionSoundexIDCache->Fetch(
					IntKey(streetIntersectionSoundexID), 
					streetIntersectionSoundexReturn
				)
			) {
				// Use cached value.
				return true;
			} else {
				// Fetch from raw reader.
				return GetStreetIntersectionSoundexByID(streetIntersectionSoundexID, streetIntersectionSoundexReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Given a StreetIntersectionSoundex record, build the associated
		// StreetIntersection record.
		///////////////////////////////////////////////////////////////////////
		bool GetStreetIntersectionFromStreetIntersectionSoundex(
			const StreetIntersectionSoundex& IntersectionSoundex,
			StreetIntersection& streetIntersectionReturn
		);

		///////////////////////////////////////////////////////////////////////
		// Override this to get error messages
		///////////////////////////////////////////////////////////////////////
		virtual void ErrorMessage(const TsString& msg) {}

	private:
		// Is the query object open?
		bool isOpen;

		// Directory containing the geocoder database.
		TsString databaseDir;
		TsString tableDir;

		///////////////////////////////////////////////////////////////////////
		// Huffman coders used for decoding various records
		///////////////////////////////////////////////////////////////////////
		// StreetName
		HuffmanCoder<int, std::less<int> > streetNameCityStatePostcodeIDCoder;
		HuffmanCoder<TsString, std::less<TsString> > streetNamePrefixCoder;
		HuffmanCoder<TsString, std::less<TsString> > streetNamePredirCoder;
		HuffmanCoder<int, std::less<int> > streetNameNameCoder;
		HuffmanCoder<TsString, std::less<TsString> > streetNameSuffixCoder;
		HuffmanCoder<TsString, std::less<TsString> > streetNamePostdirCoder;
		HuffmanCoder<int, std::less<int> > streetNameStreetSegmentIDFirstCoder;
		HuffmanCoder<int, std::less<int> > streetNameStreetSegmentCountCoder;
		// StreetSegment
		HuffmanCoder<int, std::less<int> > streetSegmentAddrLowKeyCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentAddrLowKeyCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentAddrLowNonkeyCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentAddrLowNonkeyCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentAddrHighCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentAddrHighCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentCountyKeyCoder;
		HuffmanCoder<int, std::less<int> > streetSegmentCountyNonkeyCoder;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusTractKeyCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusTractKeyCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusTractNonkeyCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusTractNonkeyCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusBlockKeyCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusBlockKeyCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusBlockNonkeyCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentCensusBlockNonkeyCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentPostcodeExtKeyCoder;
		HuffmanCoder<int, std::less<int> > streetSegmentPostcodeExtNonkeyCoder;
		HuffmanCoder<int, std::less<int> > streetSegmentCoordinateIDCoder1;
		HuffmanCoder<int, std::less<int> > streetSegmentCoordinateIDCoder2;
		HuffmanCoder<int, std::less<int> > streetSegmentCoordinateCountCoder;
		// Coordinate
		HuffmanCoder<int, std::less<int> > coordinateLatitudeCoder1;
		HuffmanCoder<int, std::less<int> > coordinateLatitudeCoder2;
		HuffmanCoder<int, std::less<int> > coordinateLongitudeCoder1;
		HuffmanCoder<int, std::less<int> > coordinateLongitudeCoder2;
		// StreetIntersection
		HuffmanCoder<int, std::less<int> > streetIntersectionStateCoder;
		HuffmanCoder<int, std::less<int> > streetIntersectionSoundex1Coder;
		HuffmanCoder<int, std::less<int> > streetIntersectionStreetNameID1Coder;
		HuffmanCoder<int, std::less<int> > streetIntersectionStreetSegmentOffset1Coder;
		HuffmanCoder<int, std::less<int> > streetIntersectionSoundex2Coder;
		HuffmanCoder<int, std::less<int> > streetIntersectionStreetNameID2Coder;
		HuffmanCoder<int, std::less<int> > streetIntersectionStreetSegmentOffset2Coder;

		// Inputs for reading reference data files.
		DataInput cityStatePostcodeInput;
		DataInput cityStatePostcodeFaIndexInput;
		DataInput citySoundexInput;
		DataInput streetNameInput;
		DataInput streetNamePositionIndexInput;
		DataInput streetNameSoundexInput;
		DataInput streetSegmentInput;
		DataInput streetSegmentPositionIndexInput;
		DataInput coordinateInput;
		DataInput coordinatePositionIndexInput;
		DataInput streetIntersectionSoundexInput;
		DataInput streetIntersectionSoundexPositionIndexInput;
		DataInput postcodeAliasByPostcodeInput;
		DataInput postcodeAliasByGroupInput;
		DataInput postcodeCentroidInput;

		// Counts of the number of records.
		unsigned cityStatePostcodeCount;
		unsigned cityStatePostcodeSoundexCount;
		unsigned cityStatePostcodeFaIndexCount;
		unsigned streetNameCount;
		unsigned streetNameSoundexCount;
		unsigned streetSegmentCount;
		unsigned coordinateCount;
		unsigned streetIntersectionSoundexCount;
		unsigned postcodeAliasCount;
		unsigned postcodeCentroidCount;

		// General-purpose temporaries.
		// Putting here avoids reconstruction.
		TsString tmpStr;

		// Table to convert from state abbreviation to FIPS code
		LookupTableRef stateAbbrToFipsTable;
		LookupTableRef stateFipsToAbbrTable;

		// Most-recently-read items.
		StreetName prevStreetName;
		int prevStreetNameID;
		StreetSegment prevStreetSegment;
		int prevStreetSegmentID;
		// Must be integers because doubles accumulate roundoff
		int prevCoordinateLat;
		int prevCoordinateLon;
		int prevCoordinateID;
		// Previous street intersection
		int prevStreetIntersectionSoundexID;
		StreetIntersectionSoundex prevStreetIntersectionSoundex;

		// Caches for various record types.  
		// This will scale with the memory use setting.
		enum {
			CityStatePostcodeFaIndexIDFromFaCacheSize = 1000,
			CityStatePostcodeFaIndexByIDCacheSize = 1000,
			CityStatePostcodeFaIndexFromFaCacheSize = 1000,
			CityStatePostcodeIDCacheSize = 1000,
			CityStatePostcodeSoundexIDCacheSize = 1000,
			StreetNameIDCacheSize = 5000,				// Not that many streets per FA
			StreetNameSoundexIDCacheSize = 1000,		// only needs to cache enough records 
														// to make binary search efficient.
			StreetNameSoundexFaSoundexCacheSize = 5000,
			StreetSegmentIDCacheSize = 100000,			// ~6MB.  Make as large as possible,
														// because the bulk of searching happens
														// with street ranges.  We want to cache
														// all street ranges in an FA if possible.
			CoordinateIDCacheSize = 10000,			// Making this larger probably won't help.
			StreetIntersectionSoundexIDCacheSize = 1000,// Intersections are rare
			PostcodeAliasCacheSize = 100,				// Don't need very many.
			PostcodeCentroidByIDCacheSize = 100,		// Don't need very many.
			PostcodeCentroidFromPostcodeCacheSize = 100	// Don't need very many.
		};

		// Scale for cache memory use
		Geocoder::MemUse memUse;

		// Generic "key" classes used by the caching mechanism
		struct IntKey {
		public:
			IntKey(int x_ = 0) : x(x_) {}
			unsigned int Hash() const { return x; }
			bool operator==(const IntKey& rhs) const { return x == rhs.x; }
			int x;
		};

		template<int I> struct StringValKey {
		public:
			StringValKey(const char* stringVal_ = "") {
				strncpy(stringVal, stringVal_, I);
				stringVal[I] = 0;
			}
			unsigned int Hash() const { 
				unsigned int hashCode = 0;
				for (const char* p = stringVal; *p != 0; p++) {
					hashCode = hashCode | (hashCode << 5) | (unsigned int)*p;
				}
				return hashCode;
			}
			bool operator==(const StringValKey& rhs) const { return strcmp(stringVal, rhs.stringVal) == 0; }
			char stringVal[I];
		};
		typedef StringValKey<6> PostcodeKey;
		typedef StringValKey<6> FinanceKey;


		// CityStatePostcode by ID
		typedef SetAssocCache<IntKey, CityStatePostcode, 4> CityStatePostcodeIDCache;
		typedef refcnt_ptr<CityStatePostcodeIDCache> CityStatePostcodeIDCacheRef;
		CityStatePostcodeIDCacheRef cityStatePostcodeByIDCache;

		// CityStatePostcodeSoundex by ID
		typedef SetAssocCache<IntKey, CityStatePostcodeSoundex, 4> CityStatePostcodeSoundexIDCache;
		typedef refcnt_ptr<CityStatePostcodeSoundexIDCache> CityStatePostcodeSoundexIDCacheRef;
		CityStatePostcodeSoundexIDCacheRef cityStatePostcodeSoundexIDCache;

		// StreetName by ID
		typedef SetAssocCache<IntKey, StreetName, 4> StreetNameIDCache;
		typedef refcnt_ptr<StreetNameIDCache> StreetNameIDCacheRef;
		StreetNameIDCacheRef streetNameIDCache;

		// StreetNameSoundex by ID
		typedef SetAssocCache<IntKey, StreetNameSoundex, 4> StreetNameSoundexIDCache;
		typedef refcnt_ptr<StreetNameSoundexIDCache> StreetNameSoundexIDCacheRef;
		StreetNameSoundexIDCacheRef streetNameSoundexIDCache;

		// StreetNameSoundex by Fa/Soundex
		struct StreetNameSoundexFaSoundexKey {
		public:
			StreetNameSoundexFaSoundexKey(
				const char* fa_ = "",
				const char* soundex_ = "Z000"
			)
			{
				soundex = GeoUtil::PackSoundex(soundex_);
				strncpy(fa, fa_, sizeof(fa));
				fa[sizeof(fa)-1] = 0;
			}
			unsigned int Hash() const { 
				int hashCode = 0;
				for (const char* p = fa; *p != 0; p++) {
					hashCode = hashCode | (hashCode << 5) | (unsigned int)*p;
				}
				return hashCode | soundex; 
			}
			bool operator==(const StreetNameSoundexFaSoundexKey& rhs) const {
				 return strcmp(fa, rhs.fa) == 0 && soundex == rhs.soundex; 
			}
			char fa[7];
			unsigned int soundex;
		};
		typedef SetAssocCache<StreetNameSoundexFaSoundexKey, StreetNameSoundex, 4> StreetNameSoundexFaSoundexCache;
		typedef refcnt_ptr<StreetNameSoundexFaSoundexCache> StreetNameSoundexFaSoundexCacheRef;
		StreetNameSoundexFaSoundexCacheRef streetNameSoundexFaSoundexCache;

		// StreetSegment by ID
		typedef SetAssocCache<IntKey, StreetSegment, 4> StreetSegmentIDCache;
		typedef refcnt_ptr<StreetSegmentIDCache> StreetSegmentIDCacheRef;
		StreetSegmentIDCacheRef streetSegmentIDCache;

		// Coordinate by ID
		typedef SetAssocCache<IntKey, CoordinatePoint, 4> CoordinateIDCache;
		typedef refcnt_ptr<CoordinateIDCache> CoordinateIDCacheRef;
		CoordinateIDCacheRef coordinateIDCache;

		// StreetIntersectionSoundex by ID
		typedef SetAssocCache<IntKey, StreetIntersectionSoundex, 4> StreetIntersectionSoundexIDCache;
		typedef refcnt_ptr<StreetIntersectionSoundexIDCache> StreetIntersectionSoundexIDCacheRef;
		StreetIntersectionSoundexIDCacheRef streetIntersectionSoundexIDCache;

		// Postal code Alias, sorted by Postal code, index by ID (record position)
		typedef SetAssocCache<IntKey, PostcodeAlias, 4> PostcodeAliasByPostcodeIDCache;
		typedef refcnt_ptr<PostcodeAliasByPostcodeIDCache> PostcodeAliasByPostcodeIDCacheRef;
		PostcodeAliasByPostcodeIDCacheRef postcodeAliasByPostcodeIDCache;

		// Postal code Alias, sorted by Group, index by ID (record position)
		typedef SetAssocCache<IntKey, PostcodeAlias, 4> PostcodeAliasByGroupIDCache;
		typedef refcnt_ptr<PostcodeAliasByGroupIDCache> PostcodeAliasByGroupIDCacheRef;
		PostcodeAliasByGroupIDCacheRef postcodeAliasByGroupIDCache;

		// Postal code group from Postal code 
		typedef SetAssocCache<PostcodeKey, PostcodeAlias, 4> PostcodeGroupFromPostcodeCache;
		typedef refcnt_ptr<PostcodeGroupFromPostcodeCache> PostcodeGroupFromPostcodeCacheRef;
		PostcodeGroupFromPostcodeCacheRef postcodeGroupFromPostcodeCache;

		// Postal code group ID from Postal code group
		typedef SetAssocCache<PostcodeKey, int, 4> PostcodeGroupIDFromPostcodeGroupCache;
		typedef refcnt_ptr<PostcodeGroupIDFromPostcodeGroupCache> PostcodeGroupIDFromPostcodeGroupCacheRef;
		PostcodeGroupIDFromPostcodeGroupCacheRef postcodeGroupIDFromPostcodeGroupCache;

		// Postal code centroid by ID
		typedef SetAssocCache<IntKey, PostcodeCentroid, 4> PostcodeCentroidByIDCache;
		typedef refcnt_ptr<PostcodeCentroidByIDCache> PostcodeCentroidByIDCacheRef;
		PostcodeCentroidByIDCacheRef postcodeCentroidByIDCache;

		// Postal code centroid from postal code
		typedef SetAssocCache<PostcodeKey, PostcodeCentroid, 4> PostcodeCentroidFromPostcodeCache;
		typedef refcnt_ptr<PostcodeCentroidFromPostcodeCache> PostcodeCentroidFromPostcodeCacheRef;
		PostcodeCentroidFromPostcodeCacheRef postcodeCentroidFromPostcodeCache;

		// Postcode/state/FA ID (position) from postcode.  Caches the position of the first
		// record matching the postcode, or -1 if none match.
		typedef SetAssocCache<FinanceKey, CityStatePostcodeFaIndex, 4> CityStatePostcodeFaIndexFromFaCache;
		typedef refcnt_ptr<CityStatePostcodeFaIndexFromFaCache> CityStatePostcodeFaIndexFromFaCacheRef;
		CityStatePostcodeFaIndexFromFaCacheRef cityStatePostcodeFaIndexFromFaCache;

		// Postcode/state/FA by ID (position)
		typedef SetAssocCache<IntKey, CityStatePostcodeFaIndex, 4> CityStatePostcodeFaIndexByIDCache;
		typedef refcnt_ptr<CityStatePostcodeFaIndexByIDCache> CityStatePostcodeFaIndexByIDCacheRef;
		CityStatePostcodeFaIndexByIDCacheRef cityStatePostcodeFaIndexByIDCache;
	};

	typedef refcnt_ptr<QueryImp> QueryImpRef;
}

#endif
