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
# $Rev: 63 $ 
# $Date: 2006-12-11 23:53:36 +0100 (Mon, 11 Dec 2006) $ 
*/

// Geocoder.h: Public interface file for geocoder library

#ifndef INCL_GeocoderPortfolioExplorer_H
#define INCL_GeocoderPortfolioExplorer_H

//#include "GeocoderItf.h"

namespace PortfolioExplorer {

	// Forward declaration of implementation classes
	class GeocoderImp;

	// Forward declaration of "internal" structure, not to be used for cross-DLL calls.
	// Actual structure is declared in GeocodeResultsInternal.h
	struct GeocodeResultsInternal;
#ifdef WIN32
#pragma warning(disable : 4275)
#endif
	// Main geocoder class
#if defined(UNIX)
	class Geocoder
#else
	class __declspec(dllexport) Geocoder
#endif
	{
	public:

		// Interface struct definitions
		// Memory usage control
		enum MemUse {
			MemUseSmall,		// 30% of normal ~= 3MB
			MemUseNormal,		// ~= 9MB
			MemUseLarge			// 3x normal ~= 27MB
		};

		// Possible global status return values from CodeAddress().
		enum GlobalStatus {
			GlobalSingle,		// There is a distinguished "best" result
			GlobalMultiple,		// There are multiple close results 
			GlobalFailure		// There are no results that exceed the threshold.
		};

		// Possible address-matching status flags
		enum MatchStatus {	
			// Possible failure codes
			MatchFailed =                0x80000000,// Match score did not exceed threshold

			// Possible success codes
			MatchPerfect =               0x04000000,	// everything matched perfectly
			MatchIsValid =               0x02000000,	// exceeded threshold but not perfect

			// Modifications of success status -- first-line address
			MatchChangedPredir =         0x00000001,
			MatchChangedStName =         0x00000002,
			MatchChangedStPrefix =       0x00000004,
			MatchChangedStSuffix =       0x00000008,
			MatchChangedPostdir =        0x00000010,
			MatchChangedUnitDes =        0x00000020,
			MatchAddressNbrOutOfRange =  0x00000040,
			MatchUnitNbrOutOfRange =     0x00000080,	// THIS FLAG IS CURRENTLY NOT USED;
			MatchMovedAddrLetterToUnit = 0x00000100,	// e.g. "123A MAIN ST" to 
														//		"123 MAIN ST #A"
			MatchEvenOddDiffer =         0x00000200,	// even segment and odd address or vice-versa

			// Modifications of success status -- last-line address
			MatchSuppliedCity =          0x00001000,	// City was added
			MatchSuppliedState =         0x00002000,	// State was added
			MatchSuppliedPostcode =      0x00004000,	// Postal code was added
			MatchChangedCity =           0x00010000,	// City was changed
			MatchChangedState =          0x00020000,	// State was changed
			MatchChangedPostcode =       0x00040000,	// Postal code was changed
			MatchChangedPostcodeExt =    0x00080000,	// Postal code extension was changed
													// (4-digit for US, 3-letter for CA) THIS FLAG IS CURRENTLY NOT USED;
			MatchCityAliasUsed =         0x00100000	// City was matched to alias or replacement
		};

		// Note that for intersections, the MatchChanged* flags will be set
		// if either street name is altered.


		// Possible geocoding status flags
		enum GeocodeStatus {
			// Possible failure codes
			GeocodeFailed =              0x80000000,	// No street match or no lat/lon segment.

			// Possible success codes
			GeocodeAddress =             0x08000000,	// Correctly coded from street address
			GeocodeExtrapolate =         0x04000000,	// Extrapolate addr number out of range
			GeocodeIntersection =        0x02000000,	// Street intersection
			GeocodeZip9Centroid =        0x01000000,	// 9-digit zip centroid (not yet implemented)
			GeocodeZip7Centroid =        0x00800000,	// 7-digit zip centroid (not yet implemented)
			GeocodeZip5Centroid =        0x00400000,	// 5-digit zip centroid 
			GeocodePostcode6Centroid =   0x00200000,	// 6-letter Canadian postcode centroid
			GeocodePostcode3Centroid =   0x00100000,	// 3-letter Canadian postcode centroid
			GeocodePostcodeAnyCentriod = 
				GeocodeZip9Centroid | GeocodeZip7Centroid | 
				GeocodeZip5Centroid | GeocodePostcode6Centroid | 
				GeocodePostcode3Centroid			// Any centroid

		};

		// Structure to hold the results of geocoding
#if defined(UNIX)
		struct GeocodeResults
#else
		struct __declspec(dllexport) GeocodeResults
#endif
		{
			GeocodeResults();
			GeocodeResults(const GeocodeResults& rhs);
			GeocodeResults& operator=(const GeocodeResults& rhs);
			~GeocodeResults();
			void Clear();

			// Accessors for components of GeocodeResults.  This is safe across DLL boundaries.
			const char* GetAddrNbr();		// address number
			const char* GetPrefix();		// prefix (Canada only)
			const char* GetPredir();		// predirectional
			const char* GetStreet();		// street name
			const char* GetSuffix();		// street suffix
			const char* GetPostdir();		// postdirectional
			const char* GetUnitDes();		// unit designator
			const char* GetUnit();			// unit
			const char* GetCity();			// city name
			int GetState();					// numeric state/province code
			const char* GetStateAbbr();		// two- or three-letter state abbreviation
			const char* GetCountryCode();	// ISO two-letter country code
			int GetCountyCode();			// County FIPS code (USA only)
			const char* GetCensusTract();	// six-char census tract (USA only)
			const char* GetCensusBlock();	// four-char census block (USA only)
			const char* GetPostcode();		// five-digit ZIP code or three-letter leading Canadian postal code
			const char* GetPostcodeExt();	// four-digit ZIP+4 or three-letter trailing Canadian  postal code
			double GetLatitude();			// latitude
			double GetLongitude();			// longitude
			int GetMatchScore();			// match score, 0-1000
			int GetMatchStatus()	;		// MatchStatus flag set
			int GetGeoStatus();				// GeocodeStatus flag set
			// When a street intersection is coded, the geoStatus flags will
			// contain GeocodeIntersection.  When that is true, the following
			// fields will contain the intersecting street.
			const char* GetPrefix2();		// intersecting street prefix
			const char* GetPredir2();		// intersecting street predirectional
			const char* GetStreet2();		// intersecting street name
			const char* GetSuffix2();		// intersecting street suffix
			const char* GetPostdir2();		// intersecting street postdirectional

			// Get the pointer to the internal results.
			// Do not use this function call across DLL boundaries.
			GeocodeResultsInternal& GetResultsInternal() { return *resultsInternal; }
		private:
			// Pointer to private results data.
			GeocodeResultsInternal* resultsInternal;
		};


		// Default threshold values
		enum {
			DefaultMatchThreshold = 800,
			DefaultMultipleMatchThreshold = 25
		};


		// Possible treatment of how the City/State/Postcode record that "owns" a street segments
		// is reconciled with with the City/State/Postcode that best matches the entered last-line.
		enum StreetOwnerTreatment {
			ChooseStreetOwnerOverLastLine,		// Always choose the City/State/Postcode that "owns" the street segment
			ChooseLastLineOverStreetOwner,		// Always choose the City/State/Postcode that best matches the last-line
			ChooseStreetOwnerCountrySpecific	// DEFAULT
												// Use country-specific logic, because for USA we want the best match to
												// last-line (for vanity city names), whereas for CA we want the street-segment
												// owner (because three-letter postcode granularity holds several cities).
		};

		///////////////////////////////////////////////////////////////////////
		// Constructor.  
		// Inputs:
		//	const char*			tableDir	The geocoder directory containing the
		//									lookup tables for the address parser.
		//	const char*			databaseDir	The geocoder directory containing the
		//									geocoder database files.
		//	MemUse				memUse		Relative amount of memory to use for caching
		///////////////////////////////////////////////////////////////////////
		Geocoder(
			const char* tableDir,
			const char* databaseDir,
			MemUse memUse = MemUseNormal
		);

		///////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////
		~Geocoder();

		///////////////////////////////////////////////////////////////////////
		// Error-message-receiving method.
		// Override this to intercept human-readable error messages.
		///////////////////////////////////////////////////////////////////////
		virtual void ErrorMessage(const char* message);

#ifndef NDEBUG
		///////////////////////////////////////////////////////////////////////
		// Trace method for debugging.
		// Override this to intercept human-readable debug trace messages.
		///////////////////////////////////////////////////////////////////////
		virtual void TraceMessage(const char* message);
#endif

		///////////////////////////////////////////////////////////////////////
		// Open the Geocoder instance using the given query interface.
		// Will call Open() on the query interface object.
		// Returns true on success, false on failure.
		// Override ErrorMessage() to get message text.
		///////////////////////////////////////////////////////////////////////
		bool Open();

		///////////////////////////////////////////////////////////////////////
		// Close the Geocoder instance.
		// This will be called by the destructor.
		///////////////////////////////////////////////////////////////////////
		void Close();

		///////////////////////////////////////////////////////////////////////
		// Code an address.  Call GetNextCandidate() to check results.
		// Inputs:
		//	const char*			line1			street address
		//	const char*			line2			city, state, zip
		// Return value:
		//	GlobalStatus		A status code indicating the overall result of
		//						the geocoding process
		///////////////////////////////////////////////////////////////////////
		GlobalStatus CodeAddress(
			const char* line1,					// street address
			const char* line2					// city, state, zip
		);

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
		bool GetNextCandidate(
			GeocodeResults& resultsReturn		// result of coding
		);

		///////////////////////////////////////////////////////////////////////
		// Set the threshold score that determines a match (0-1000)
		///////////////////////////////////////////////////////////////////////
		void SetMatchThreshold(int threshold);

		///////////////////////////////////////////////////////////////////////
		// Set the score delta that determines a multiple (0-1000)
		///////////////////////////////////////////////////////////////////////
		void SetMultipleThreshold(int threshold);

		///////////////////////////////////////////////////////////////////////
		// Set the offset of a coded address from the side of the street, feet.
		///////////////////////////////////////////////////////////////////////
		void SetStreetOffset(float offset);

		///////////////////////////////////////////////////////////////////////
		// Set the minimum distance that a point may code from the end of a
		// street segment, in feet
		///////////////////////////////////////////////////////////////////////
		void SetStreetEndpointOffset(float offset);

		///////////////////////////////////////////////////////////////////////
		// Set how the City/State/Postcode record that "owns" a street segment
		// is reconciled with with the City/State/Postcode that best matches 
		// the entered last-line.
		///////////////////////////////////////////////////////////////////////
		void SetStreetOwnerTreatment(StreetOwnerTreatment streetOwnerTreatment);

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
		// Set the minimum or maximum interpolation value.
		//  The min and max values define the interpolation on a street segment 
		//  where 0.0 is the beginning of the segment and 1.0 is the end.  The
		//  values are set to -1.0 and 2.0 respectively by default, allowing for
		//  extrapolation beyond the end of the segment by 100% in each direction.
		//  It is reccomended that min < max and the range between min and max is
		//  not much smaller than (0.0, 1.0)
		///////////////////////////////////////////////////////////////////////
		void SetMinInterpolation(double _minInterpolation);
		void SetMaxInterpolation(double _maxInterpolation);

		///////////////////////////////////////////////////////////////////////
		// Check that the data and geocoder are the same version
		// Inputs:
		//  TsString		dataDir	Directory containing geocoder database files
		// Outputs:
		// Return value:
		//	bool			true if versions match, false o/w.
		///////////////////////////////////////////////////////////////////////
		static bool CheckDataVersion(const char * pDataDir);

	private:
		// The implementation class that really does all the work.
		// It contains methods that are an exact reflection of the 
		// external interface methods.
		// This helps hides implementation details.
		GeocoderImp* imp;
	};

}

#endif

