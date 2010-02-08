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
# $Rev: 125 $ 
# $Date: 2009-11-11 15:26:04 +0100 (Wed, 11 Nov 2009) $ 
*/

// GeocoderImp.h: Implementation interface file for geocoder library

#ifndef INCL_GeocoderPortfolioExplorerImp_H
#define INCL_GeocoderPortfolioExplorerImp_H

#include <math.h>

#include "../global/AddressParserFirstLine.h"
#include "../global/AddressParserLastLine.h"
#include "../global/BulkAllocator.h"
#include "../global/LookupTable.h"
#include "../global/VectorNoDestruct.h"
#include "../global/Utility.h"

#include "../geocommon/Geocoder_DllExport.h"

#include "../geocommon/GeoUtil.h"
#include "Geocoder.h"
#include "GeoQueryImp.h"
//#include "GeocoderItf.h"
#include "GeoResultsInternal.h"
#include "GeoAddressTemplate.h"

namespace PortfolioExplorer {


	class Geocoder;

	// Main geocoder class
	class GeocoderImp : public GeoUtil {
	public:
		///////////////////////////////////////////////////////////////////////
		// Constructor.  
		// Inputs:
		//	Geocoder&					geocoder	The geocoder envelope class that
		//											is creating the implementation class.
		//	const TsString&			tableDir	The geocoder directory containing the
		//											lookup tables for the address parser.
		//	const TsString&			databaseDir	The geocoder directory containing the
		//											geocoder database files.
		//	MemUse						memUse		Relative amount of memory to use for caching
		///////////////////////////////////////////////////////////////////////
		GeocoderImp(
			Geocoder& geocoder,
			const TsString& tableDir,
			const TsString& databaseDir,
			Geocoder::MemUse memUse
		);

		///////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////
		~GeocoderImp();

		///////////////////////////////////////////////////////////////////////
		// Open the Geocoder instance using the given query interface.
		// Will call Open() on the query interface object.
		// Returns true on success, false on failure.
		// Override ErrorMessage() in Geocoder envelope to get message text.
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
		Geocoder::GlobalStatus CodeAddress(
			const char* line1,
			const char* line2
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
			Geocoder::GeocodeResults& resultsReturn		// result of coding
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
		void SetStreetOffset(float offset) {
			streetOffsetInFeet = offset;
		}

		///////////////////////////////////////////////////////////////////////
		// Set the minimum distance that a point may code from the end of a
		// street segment, in feet
		///////////////////////////////////////////////////////////////////////
		void SetStreetEndpointOffset(float offset)
		{
			streetEndpointOffsetInFeet = offset;
		}

		///////////////////////////////////////////////////////////////////////
		// Set how the City/State/Postcode record that "owns" a street segment
		// is reconciled with with the City/State/Postcode that best matches 
		// the entered last-line.
		///////////////////////////////////////////////////////////////////////
		void SetStreetOwnerTreatment(Geocoder::StreetOwnerTreatment streetOwnerTreatment_)
		{
			streetOwnerTreatment = streetOwnerTreatment_;
		}

		///////////////////////////////////////////////////////////////////////
		// Set the minimum interpolation value
		///////////////////////////////////////////////////////////////////////
		void SetMinInterpolation(double _minInterpolation)
		{ minInterpolation = _minInterpolation; }

		///////////////////////////////////////////////////////////////////////
		// Set the maximum interpolation value
		///////////////////////////////////////////////////////////////////////
		void SetMaxInterpolation(double _maxInterpolation)
		{ maxInterpolation = _maxInterpolation; }

		///////////////////////////////////////////////////////////////////////
		// Inputs:
		//	const CityStatePostcode&	cityStatePostcode		The current last-line record
		//	const char*					city					City to test as alias
		// Return value:
		//	bool		true if city is an alias
		///////////////////////////////////////////////////////////////////////
		bool CityIsAlias(
			const CityStatePostcode& cityStatePostcode, 
			const char* city
		);

		///////////////////////////////////////////////////////////////////////
		// Check that the data and geocoder are the same version
		// Inputs:
		//  TsString		dataDir	Directory containing geocoder database files
		// Outputs:
		// Return value:
		//	bool			true if versions match, false o/w.
		///////////////////////////////////////////////////////////////////////
		static bool CheckDataVersion(TsString dataDir);

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
		//	const char*&	stateAbbrReturn		The state abbreviation
		// Return value:
		//	bool			true if state exists, false o/w.
		///////////////////////////////////////////////////////////////////////
		bool StateCodeToAbbr(
			int stateCode,
			const char* countryCode,
			const char*& stateAbbrReturn
		);

	private:
		#ifndef NDEBUG
			void GEOTRACE(const TsString& x);
		#endif

		///////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////
		// Subclass of QueryImp to forward error messages.
		///////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////
		class QueryImpErrorMsg : public QueryImp {
		public:
			///////////////////////////////////////////////////////////////////////
			// Constructor
			// Inputs:
			//	const TsString&	tableDir	The directory containing extra geocoder tables
			//	const TsString&	databaseDir	The directory containing the geocoder database.
			//	MemUse				memUse		Parameter controlling memory usage.
			///////////////////////////////////////////////////////////////////////
			QueryImpErrorMsg(
				const TsString& tableDir_,
				const TsString& databaseDir_,
				Geocoder::MemUse memUse,
				Geocoder& geocoder_
			) :
				QueryImp(tableDir_, databaseDir_, memUse),
				geocoder(geocoder_)
			{
			}

			///////////////////////////////////////////////////////////////////////
			// Forward error messages to the geocoder's error message handler.
			///////////////////////////////////////////////////////////////////////
			virtual void ErrorMessage(const TsString& msg);
		private:
			Geocoder& geocoder;
		};



		///////////////////////////////////////////////////////////////////////
		// Given a first-line and last-line score, calculate the overall score.
		///////////////////////////////////////////////////////////////////////
		int ScoreCombined(int firstLineScore, int lastLineScore) {
			// Let either score drag down the total.
			return firstLineScore + lastLineScore - 1000;
		}

		///////////////////////////////////////////////////////////////////////
		// Given a street name score, a street range score, and a a last-line score.
		// produce the overall score.
		///////////////////////////////////////////////////////////////////////
		int ScoreCombinedFirst(
			int streetNameScore, 
			int streetSegmentScore
		) {
			return
				(
					streetNameScore * m_FirstLineStreetNameWeight + 
					streetSegmentScore * m_FirstLineStreetSegmentWeight
				) / (m_FirstLineStreetNameWeight + m_FirstLineStreetSegmentWeight);
		}

		///////////////////////////////////////////////////////////////////////
		// Given a street name score, a street range score, and a last-line score,
		// produce the overall score.
		///////////////////////////////////////////////////////////////////////
		int ScoreCombined(
			int streetNameScore, 
			int streetSegmentScore,
			int lastLineScore
		) {
			return ScoreCombined(
				ScoreCombinedFirst(streetNameScore, streetSegmentScore),
				lastLineScore
			);
		}

		///////////////////////////////////////////////////////////////////////
		// Given a last-line parse candidate and a CityStatePostcode record, 
		// calculate a score in the range 0-1000 indicating the quality
		// of match.
		// Inputs:
		//	const AddressParserLastLine::ParseCandidate&
		//				parseCandidate		The attempted last-line parse
		//	const CityStatePostcode&
		//				cityStatePostcode		The CityStatePostcode record fetched
		//									from the reference database.
		// Outputs:
		//	int&		lastLineFlags		Flags indicating modifications
		//									to last-line to make it match
		//									the reference record.
		// Return value:
		//	int		the score between 0 and 1000, with 1000 being perfect.
		///////////////////////////////////////////////////////////////////////
		int ScoreLastLine(
			const AddressParserLastLine::ParseCandidate& parseCandidate,
			const CityStatePostcode& cityStatePostcode,
			int& lastLineFlags
		);

		///////////////////////////////////////////////////////////////////////
		// Given a first-line parse candidate that is NOT an intersection,
		// and a StreetName record, calculate a score in the range 
		// 0-1000 indicating the quality of match.
		// Inputs:
		//	const AddressParserFirstLine::ParseCandidate& 
		//				candidate			The attempted first-line parse
		//	const StreetName&
		//				streetName			The StreetName record fetched
		//									from the reference database.
		//	const CityStatePostcode& 
		//				cityStatePostcode		The CityStatePostcode representing the
		//									current best last-line choice.
		// Outputs:
		//	int&		firstLineFlags		Flags indicating modifications
		//									to last-line to make it match
		//									the reference record.
		//	Geocoder::CityStatePostcode& 
		//				cityStatePostcode		A revised last-line choice based on the
		//									CityStatePostcode of the intersection.
		// Return value:
		//	int		the score between 0 and 1000, with 1000 being perfect.
		///////////////////////////////////////////////////////////////////////
		int ScoreStreetName(
			const AddressParserFirstLine::ParseCandidate& parseCandidate,
			const StreetName& streetName,
			const CityStatePostcode& cityStatePostcode,
			int& firstLineFlags,
			CityStatePostcode& chosenCityStatePostcode
		);

		///////////////////////////////////////////////////////////////////////
		// Given an address number and a StreetSegment, calculate a score in 
		// the range 0-1000 indicating the quality of match.
		// Inputs:
		//	const char*		parsedPostcodeExt	The postcode extension parsed from
		//										the input last-line, if any.
		//	const GeoUtil&	addrTemplate		The address number candidate
		//	const StreetSegment&
		//					streetSegment		The StreetSegment record fetched
		//										from the reference database.
		// Outputs:
		//	int&		streetSegmentFlags		Flags indicating mismatch of address
		//										number and range.
		//	double&		tieBreaker				If score is 1.0, then this will contain
		//										a "tiebreaker" number; higher numbers are better.
		// Return value:
		//	int		The score between 0 and 1000, with 1000 being perfect.
		///////////////////////////////////////////////////////////////////////
		int ScoreStreetSegment(
			const char* parsedPostcodeExt,
			const AddressTemplate& addrTemplate,
			const StreetSegment& streetSegment,
			int& streetSegmentFlags,
			double& tieBreaker
		);

		///////////////////////////////////////////////////////////////////////
		// Given a first-line parse candidate that is an intersection,
		// and a StreetIntersection record, calculate a score in the range 
		// 0-1000 indicating the quality of match.
		// Inputs:
		//	const AddressParserFirstLine::ParseCandidate& 
		//				candidate			The attempted first-line parse
		//	const StreetIntersection&
		//				streetIntersection	The CityStatePostcode record fetched
		//									from the reference database.
		//	const CityStatePostcode& 
		//				cityStatePostcode		The CityStatePostcode representing the
		//									current best last-line choice.
		// Outputs:
		//	int&		firstLineFlags		Flags indicating modifications
		//									to last-line to make it match
		//									the reference record.
		//	Geocoder::CityStatePostcode& 
		//				cityStatePostcode		A revised last-line choice based on the
		//									CityStatePostcode of the intersection.
		// Return value:
		//	int		the score between 0 and 1000, with 1000 being perfect.
		///////////////////////////////////////////////////////////////////////
		int ScoreStreetIntersection(
			const AddressParserFirstLine::ParseCandidate& parseCandidate,
			const StreetIntersection& streetIntersection,
			const CityStatePostcode& cityStatePostcode,
			int& firstLineFlags,
			CityStatePostcode& chosenCityStatePostcode
		);

		///////////////////////////////////////////////////////////////////////
		// Geocoding results with the extra information needed to perform the
		// just-in-time lat/lon lookup.
		///////////////////////////////////////////////////////////////////////
		struct GeocodeResultsPlus {
			GeocodeResultsInternal results;

			// Contains ID of street name record(s), used for deduping.
			int streetName1ID;
			int streetName2ID;

			// Contains information needed to find lat/lon
			StreetSegment streetSegment;
			StreetSegment streetSegment2;	// only for intersections.
		};

		///////////////////////////////////////////////////////////////////////
		// Create a geocoder results structure given a first-line parse 
		// candidate an a zip5 centroid
		// Inputs:
		//	const AddressParserFirstLine::ParseCandidate& 
		//							candidate			The first-line parse candidate
		//	const CityStatePostcode&
		//							lastLine			The reference last line
		//	const PostcodeCentroid&
		//							postcodeCentroid	The reference centroid
		//	const char*				postcodeExt			The postal code extension from the parsed last-line
		//	int						flags				Flags describing modifications
		//												that were made to achieve the match.
		//	int						score				Last line result score.
		// Outputs:
		//	GeocodeResultsPlus&		result				Results of the geocoding process,
		//												except for the lat/lon information.
		///////////////////////////////////////////////////////////////////////
		void MakeResult(
			const AddressParserFirstLine::ParseCandidate& candidate,
			const CityStatePostcode lastLine, 
			const PostcodeCentroid& postcodeCentroid,
			const char* postcodeExt,
			int flags,
			int score,
			GeocodeResultsPlus& result
		);

		///////////////////////////////////////////////////////////////////////
		// Create a geocoder results structure given a street range, 
		// last line, and associated data.  Does not fill in the lat/lon
		// data.
		// Inputs:
		//	const CityStatePostcode&
		//							lastLine			The reference last line
		//	const AddressParserFirstLine::ParseCandidate& 
		//							candidate			The first-line parse candidate
		//	const StreetName&
		//							streetName			The reference street name
		//	const StreetSegment&
		//							streetSegment			The reference street range
		//	int						flags				Flags describing modifications
		//												that were made to achieve the match.
		//	int						score				Overall result score.
		// Outputs:
		//	GeocodeResultsPlus&		result				Results of the geocoding process,
		//												except for the lat/lon information.
		///////////////////////////////////////////////////////////////////////
		void MakeResult(
			const CityStatePostcode lastLine, 
			const AddressParserFirstLine::ParseCandidate& candidate,
			const StreetName& streetName,
			const StreetSegment& streetSegment,
			int flags,
			int score,
			GeocodeResultsPlus& result
		);

		///////////////////////////////////////////////////////////////////////
		// Create a geocoder results structure given a street intersection
		// entry, last line, and associated data.  Does not fill in the lat/lon
		// data.
		// Inputs:
		//	const CityStatePostcode&
		//							lastLine			The reference last line
		//	const AddressParserFirstLine::ParseCandidate& 
		//							candidate			The first-line parse candidate
		//	const StreetIntersection&
		//							streetIntersection	The reference street intersection
		//	int						flags				Flags describing modifications
		//												that were made to achieve the match.
		//	int						score				Overall result score.
		// Outputs:
		//	GeocodeResultsPlus&		result				Results of the geocoding process,
		//												except for the lat/lon information.
		///////////////////////////////////////////////////////////////////////
		void MakeResult(
			const CityStatePostcode lastLine, 
			const AddressParserFirstLine::ParseCandidate& candidate,
			const StreetIntersection& streetIntersection,
			int flags,
			int score,
			GeocodeResultsPlus& result
		);

		///////////////////////////////////////////////////////////////////////
		// Geocode an intermediate result.
		// Inputs:
		//	GeocodeResultsPlus&		result		Contains all of the address information
		//										on input.
		// Outputs:
		//	GeocodeResultsPlus&		result		Result with lat/lon calculated.
		///////////////////////////////////////////////////////////////////////
		void CodeResult(
			GeocodeResultsPlus& result
		);

		///////////////////////////////////////////////////////////////////////
		// Get the segment points for a street range.
		// Inputs:
		//	const StreetSegment& 
		//							streetSegment		The street range to query
		// Outputs:
		//	std::vector<Geocoder::CoordinatePoint>& 
		//							points			The points that were retrieved.
		///////////////////////////////////////////////////////////////////////
		void GetSegmentPointsForStreetSegment(
			const StreetSegment& streetSegment,
			std::vector<CoordinatePoint>& points
		);

		///////////////////////////////////////////////////////////////////////
		// Get all related finance areas, accounting for postal code aliases.
		///////////////////////////////////////////////////////////////////////
		void GetRelatedFinanceAreas(
			const char* postcode,
			const char* finance,
			std::vector<const char*>& faListReturn
		);

		///////////////////////////////////////////////////////////////////////
		// Get all related finance areas, accounting for postal code's aliases.
		///////////////////////////////////////////////////////////////////////
		void GetAllFinanceAreasForCity(
			const char* city,
			int stateCode,
			const char* excludeFA,
			std::vector<const char*>& faListReturn
		);

		///////////////////////////////////////////////////////////////////////
		// Given a city name and zip code, find a CityStatePostcode matching both
		// Inputs:
		//	const char*			city		The city name to match
		//	const char*			postcode	The postal code to match
		// Outputs:
		//	Geocoder::CityStatePostcode& cityStatePostcodeReturn
		//									The found CityStatePostcode record.
		// Return value:
		//	bool				true if the CityStatePostcode was found, was o/w
		///////////////////////////////////////////////////////////////////////
		bool FindCityStatePostcodeFromCityPostcode(
			const char* city, 
			const char* postcode,
			CityStatePostcode& cityStatePostcodeReturn
		);

		///////////////////////////////////////////////////////////////////////
		// Does the result set contain multiples?
		///////////////////////////////////////////////////////////////////////
		bool ResultsAreMultiple();


		///////////////////////////////////////////////////////////////////////
		// Are the two given results aliases of each other by city and street?
		// Inputs:
		//	GeocodeResultsPlus&		result1		First result
		//	GeocodeResultsPlus&		result2		Second result
		// Return value:
		//	bool			true if they are aliases, false o/w
		///////////////////////////////////////////////////////////////////////
		bool ResultsAreAliased(
			GeocodeResultsPlus& result1,
			GeocodeResultsPlus& result2
		) {
			GeocodeResultsCmpStreetSegmentIDDedupe cmp;
			return cmp(&result1, &result2);
		}

		///////////////////////////////////////////////////////////////////////
		// Is a postal code Canadian?
		///////////////////////////////////////////////////////////////////////
		bool IsCanadaPostcode(const char* postcode) {
			return postcode[0] != 0 && isalpha(postcode[1]);
		}

		///////////////////////////////////////////////////////////////////////
		// Find a city replacement, given the state abbreviation and the city.
		// City replacements occur when cities are merged together and assume
		// a new name.
		// Inputs:
		//	const char*		stateAbbr		The state of the old city
		//	const char*		oldCity			The old city
		// Outputs:
		//	const char*&	newCityReturn	The new city
		// Return value:
		//	bool		true if a replPortfolioExplorernt was found, false o/w
		///////////////////////////////////////////////////////////////////////
		bool FindReplacementCity(
			const char* postCode,
			const char* stateAbbr,
			const char* oldCity,
			const char*& newCityReturn
		);

		///////////////////////////////////////////////////////////////////////
		// Compare two street names, giving special weighting to numerics.
		// 
		// Inputs:
		//	const char*		candidateStreet		The candidate street name
		//	const char*		referenceStreet		The reference street name
		// Return value:
		//	double		A value from 0.0 to 1.0, where 0.0 is no match and
		//				1.0 is a perfect match.
		///////////////////////////////////////////////////////////////////////
		double CompareStreetNames(
			const char* candidateStreet,
			const char* referenceStreet
		);

		///////////////////////////////////////////////////////////////////////
		// Given the set of last-line parse candidates, choose the best last line.
		// 
		// Inputs: none
		// Outputs:
		//	CityStatePostcode&	bestCityStatePostcode		Chosen CityStatePostcode
		//	int&				bestLastLineScore			Score achieved when choosing last-line
		//	int&				bestLastLineCandidateIdx	Index of best parse candidate
		//	int&				bestLastLineFlags			Flags indicating parse modifications or
		//													special cases in matching.
		// Return value:
		//	bool			true if match found, false if no match found.
		///////////////////////////////////////////////////////////////////////
		bool ChooseBestLastLine(
			CityStatePostcode& bestCityStatePostcode,
			int& bestLastLineScore,
			int& bestLastLineCandidateIdx,
			int& bestLastLineFlags
		);

		///////////////////////////////////////////////////////////////////////
		// Given the best last line, and the set of first-line parse
		// candidates, find the results set.
		// 
		// Inputs: 
		//	CityStatePostcode&	bestCityStatePostcode		The best last-line reference record
		//	int					bestLastLineScore			Score achieved when choosing last-line
		//	int					bestLastLineCandidateIdx	Index of best parse candidate
		//	int					bestLastLineFlags			Flags indicating parse modifications or
		// Inputs that are members of the class:
		//	firstLineParseCandidates
		// Outputs: none
		// Outputs that are members of the class:
		///////////////////////////////////////////////////////////////////////
		void ChooseBestFirstLineResults(
			const CityStatePostcode& bestCityStatePostcode,
			int bestLastLineScore,
			int bestLastLineCandidateIdx,
			int bestLastLineFlags
		);

		///////////////////////////////////////////////////////////////////////
		// When a last-line is changed to the owning CSP record of the matched
		// street name from the "best match" last-line, a penalty is exacted
		// depending on whether the original last-line had a postal code or not.
		// Inputs:
		//	CityStatePostcode&		bestCityStatePostcode		The "best" last-line record
		//	CityStatePostcode&		newCityStatePostcode		The new last-line record
		//	bool					haveLastLinePostcode		true if the parse candidate has a postal code
		//	int&					lastLineScore				The last-line score before penalty
		// Outputs:
		//	int&					lastLineScore				The last-line score after penalty
		///////////////////////////////////////////////////////////////////////
		void PenalizeLastLineChange(
			const CityStatePostcode& bestCityStatePostcode,
			const CityStatePostcode& newCityStatePostcode,
			bool haveLastLinePostcode,
			int currLastLineFlags,
			int& lastLineScore
		);

		///////////////////////////////////////////////////////////////////////
		// Updates the MatchStatus flags for the current candidate, based on
		// the original data (the first, unmodified parse candidate).
		// Inputs:
		//  baseline				The first, unmodified parse candidate;
		//  best					The currently used parse candidate;
		//  newCityStatePostcode	A newly chosen city-state-postcode; If NULL, "best" is used;
		//  bestLastLineIndex		The index of the "best" candidate; Used to check if "best" IS the first;
		//  lastLineFlags			Current flags, before function call;
		// Outputs:
		//  lastLineFlags			Flags, updated as necessary.
		///////////////////////////////////////////////////////////////////////
		inline void UpdateLastLineMatchStatusFlags(
			const AddressParserLastLine::ParseCandidate & baseline,
			const AddressParserLastLine::ParseCandidate & best,
			const CityStatePostcode * newCityStatePostcode,
			int bestLastLineIndex,
			int & lastLineFlags
		);

		///////////////////////////////////////////////////////////////////////
		// Catch all for updating any remaining MatchStatus flags for the
		// current first line candidate.
		//
		// Currently, this method checks the address for changes, and certain
		// parse flags that may not have been picked up during other operations.
		//
		// If there are any parse flags NOT being caught, that you feel should
		// affect first line MatchStatus flags, here is the place to check for
		// them and set the proper MatchStatus flag.
		//
		// Inputs:
		//  baseline			The first, unmodified parse candidate;
		//  candidate			The current parse candidate;
		//  candidateIndex		The index of this candidate; Used to check if this IS the first;
		//  currentScore		The current "combined" score for this candidate;
		//  firstLineFlags		Current flags, before function call;
		// Outputs:
		//  firstLineFlags		Flags, updated as necessary.
		///////////////////////////////////////////////////////////////////////
		inline void CatchRemainingFirstLineMatchStatusFlags(
			const AddressParserFirstLine::ParseCandidate & baseline,
			const AddressParserFirstLine::ParseCandidate & candidate,
			int candidateIndex,
			int currentScore,
			int & firstLineFlags
		);

		///////////////////////////////////////////////////////////////////////
		// Make sure either the ini has been read and applied,
		// or default values have been applied.
		///////////////////////////////////////////////////////////////////////
		void SetupIni();

		///////////////////////////////////////////////////////////////////////
		// Get the path and file name to the .ini file. It should be in the
		// same folder as the dll.
		///////////////////////////////////////////////////////////////////////
		void GetIniPathAndFileName(char * filePathName);

		///////////////////////////////////////////////////////////////////////
		// Read the Geocoderimp.ini file to override default values.
		///////////////////////////////////////////////////////////////////////
		template <class TMap> bool ReadIni(char *szFileName, TMap & vars);

		///////////////////////////////////////////////////////////////////////
		// Compute feet per degrees
		///////////////////////////////////////////////////////////////////////
		static inline double FeetPerDegreeLongitude(double latDeg)
		{
			assert(-90<=latDeg && latDeg<=90);

			double latRadian = latDeg * ( PI / 180 );
			double latCirc = (PI * 2 * EARTH_RADIUS_FEET) * cos(latRadian);
			return fabs(latCirc / 360);
		}

		///////////////////////////////////////////////////////////////////////
		// Compare strings using EditDistance, but convert results to
		// the range 0.0 - 1.0
		///////////////////////////////////////////////////////////////////////
		static inline float LocalCompareString(const char* str1, const char* str2)
		{
			size_t totalLength = strlen(str1) + strlen(str2);
			float score = 0.0;
			if (totalLength > 1) {
				score = float(
					1.0 - 
					EditDistance(str1, str2) / (float)(totalLength / 2));
				if (score < 0.0) {
					score = 0.0;
				}
			}
			return score;
		}

		// So we can call overridden functions in the envelope.
		Geocoder& geocoder;

		// The reference query interface for querying records.
		QueryImpRef queryItf;

		// Address parsers
		AddressParserFirstLine addressParserFirstLine;
		AddressParserLastLine addressParserLastLine;

		// Match threshold controls minumim quality of match.
		int matchThreshold;

		// Multi-match threshold sets minimum score distance between candidates
		// that must be acheived to get a match.
		int multipleMatchThreshold;

		// Directories of geocoder data:
		//		database:  Contains all built data files.
		//		tables: Contains auxilliary tables.
		TsString tableDir;
		TsString databaseDir;

		// Vector to hold list of last-line parse candidates
		// VectorNoDestruct<> is used to avoid extra element construction/destruction
		VectorNoDestruct<AddressParserLastLine::ParseCandidate> lastLineParseCandidates;

		// Vector to hold list of first-line parse candidates
		// VectorNoDestruct<> is used to avoid extra element construction/destruction
		VectorNoDestruct<AddressParserFirstLine::ParseCandidate> firstLineParseCandidates;

		// Vector to hold list of geocoding results.
		// VectorNoDestruct<> is used to avoid extra element construction/destruction
		VectorNoDestruct<GeocodeResultsPlus> geocodeResults;

		// The status returned by last CodeAddress() call
		Geocoder::GlobalStatus resultsGlobalStatus;

		// Vector to hold temporary point queries.
		std::vector<CoordinatePoint> tempPoints1;
		std::vector<CoordinatePoint> tempPoints2;
		std::vector<double> tempLengths;

		// Lookup table used to replace merged cities with their new names
		LookupTableRef cityReplacementTable;

		// Constants
		static const double PI;
		static const double EARTH_RADIUS_MILES;
		static const double EARTH_RADIUS_FEET;
		static const double MILES_PER_DEG_LATITUDE;
		static const double FEET_PER_DEG_LATITUDE;

		//Version filename
		static const TsString versionFilename;

		// Object to sort the geocode results descending by score
		struct GeocodeResultsCmpScore {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return lhs->results.matchScore > rhs->results.matchScore;
			}
		};


		// Object to sort the geocode results descending by StreetName and score
		struct GeocodeResultsCmpStreetNameScore {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return
					lhs->streetName1ID != rhs->streetName1ID ? 
					lhs->streetName1ID < rhs->streetName1ID :
					(
						lhs->streetName2ID != rhs->streetName2ID ? 
						lhs->streetName2ID < rhs->streetName2ID :
						lhs->results.matchScore > rhs->results.matchScore	// note score is descending order
					);
			}
		};
		// Object to compare the geocode results by street name
		struct GeocodeResultsCmpStreetNameDedupe {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return
					lhs->streetName1ID == rhs->streetName2ID &&
					lhs->streetName2ID == rhs->streetName1ID;
			}
		};


		// Object to sort the geocode results descending by StreetSegmentID and score
		struct GeocodeResultsCmpStreetSegmentIDScore {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return 
					lhs->streetSegment.ID == rhs->streetSegment.ID ?
					(
						lhs->streetSegment2.ID == rhs->streetSegment2.ID ?
						lhs->results.matchScore > rhs->results.matchScore :
						lhs->streetSegment2.ID < rhs->streetSegment2.ID
					) :
					lhs->streetSegment.ID < rhs->streetSegment.ID;
			}
		};
		// Object to compare the geocode results by street range
		struct GeocodeResultsCmpStreetSegmentIDDedupe {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return 
					lhs->streetSegment.ID == rhs->streetSegment.ID &&
					lhs->streetSegment2.ID == rhs->streetSegment2.ID;
			}
		};


		// Object to sort the goecode results by lat/lon and descending score
		struct GeocodeResultsCmpIntersectionLatLon {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return
					lhs->results.latitude != rhs->results.latitude ? 
					lhs->results.latitude < rhs->results.latitude :
					(
						lhs->results.longitude != rhs->results.longitude ? 
						lhs->results.longitude < rhs->results.longitude :
						lhs->results.matchScore > rhs->results.matchScore	// note score is descending order
					);
			}
		};
		// Object to compare the geocode results by lat/lon, but only consider them
		// equivalent if they are intersection.
		struct GeocodeResultsCmpIntersectionLatLonDedupe {
			bool operator()(GeocodeResultsPlus* lhs, GeocodeResultsPlus* rhs) const {
				return
					(lhs->results.geoStatus & Geocoder::GeocodeIntersection) != 0 &&
					lhs->results.latitude == rhs->results.latitude &&
					lhs->results.longitude == rhs->results.longitude;
			}
		};


		// Used while producing a unique list of FA's.
		// Strings are allocated using the bulkAllocator.
		std::vector<const char*> uniqueFAList;
		struct CmpStr {
			bool operator()(const char* lhs, const char* rhs) const {
				return strcmp(lhs, rhs) < 0;
			}
		};
		struct EqualStr {
			bool operator()(const char* lhs, const char* rhs) const {
				return strcmp(lhs, rhs) == 0;
			}
		};

		// Sorted results.  This is created after geocodeResults has been filled.
		typedef std::vector<GeocodeResultsPlus*> SortedGeocodeResults;
		SortedGeocodeResults sortedGeocodeResults;

		// Index into sortedGeocodeResults that points to the next result.
		int resultsCandidateIdx;

		// Bulk allocator used to allocate string objects
		BulkAllocatorRef bulkAllocator;

		// Offset of coded address from side and end of street.
		float streetOffsetInFeet;
		float streetEndpointOffsetInFeet;

		//Interpolation minimum/maximum
		double minInterpolation;
		double maxInterpolation;

		// How the City/State/Postcode record that "owns" a street segment
		// is reconciled with with the City/State/Postcode that best matches 
		// the entered last-line.
		Geocoder::StreetOwnerTreatment streetOwnerTreatment;

		// Weight values, setup with default values in the constructor, possibly overriden
		// with corresponding values read in from the ini file;
		int m_LastLineThresholdZipOnly;			// A zip-only query producing this score or better
												// will cause the city soundex query to be skipped.

		// Weights of last-line components; should total 1000
		int m_LastLineCityWeight;
		int m_LastLineStateWeight;
		int m_LastLinePostcodeWeight;

		// These weights exist in case we want to penalize missing components of
		// the last-line address.  However, since this tends to affect all scoring 
		// of all last-line reference records the same (and there are no ambiguities
		// in candidate last-lines), we don't currently use them.
		int m_LastLineMissingCityWeight;		// Score for supplied missing city (no penalty)
		int m_LastLineMissingStateWeight;		// Score for supplied missing state (no penalty)
		int m_LastLineMissingPostcodeWeight;	// Score for supplied missing zip (no penalty)
		int m_LastLineModWeight;				// Weight applied to modifications made
												// last-line parse candidate.

		// Weights of first-line components used when comparing
		// candidate to StreetName; should total 1000
		int m_FirstLinePredirWeight;
		int m_FirstLineStreetWeight;
		int m_FirstLinePrefixWeight;
		int m_FirstLineSuffixWeight;
		int m_FirstLinePostdirWeight;

		// Weights when comparing StreetName's Zip to chosen last-line zip.
		int m_FirstLineAddressPostcodeWeight;		// City ok but zip differs
		int m_FirstLineAddressCityPostcodeWeight;	// City and Zip differ but in same Fa

		int m_FirstLineModWeight;					// penalty applied per parse modification

		// The following weights are for the address-range
		// portion of the first line score.
		int m_FirstLineRangeAlphaWeight;	// penalty applied for address-range alpha portion mismatch
		int m_FirstLineRangeEvenOddWeight;	// penalty applied for even/odd mismatch
		int m_FirstLineRangeEvenOddUnknownWeight;	// penalty applied for even/odd unknown in range
		int m_FirstLineOutOfRangeWeight;	// penalty applied for out-of-range address

		// Relative weight of the StreetName and StreetSegment portions of
		// the match.  The implication is that if we find a matching StreetName
		// but not a matching StreetSegment, we can extrapolate from the
		// closest StreetSegment and dock the score accordingly.
		int m_FirstLineStreetNameWeight;
		int m_FirstLineStreetSegmentWeight;

		// Penalty for replacing last-line with owner of matched street
		int m_ReplaceLastLineCityWeight;
		int m_ReplaceLastLineCityNoPostcodeWeight;
		int m_ReplaceLastLinePostcodeWeight;
		int m_ReplaceLastLineFinanceWeightSameCity;
		int m_ReplaceLastLineFinanceWeightNewCity;

		bool m_bXmlInitialized;
	};

}

#endif

