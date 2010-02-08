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

// GeocoderImp.cpp: Public interface file for geocoder library

#include "../geocommon/Geocoder_Headers.h"

#if defined(WIN32)
#include "Shlwapi.h"
#include <io.h>
#endif
#include <map>
#if defined(UNIX)
#include <unistd.h>
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#endif

#include <xercesc/util/PlatformUtils.hpp>

#include "../global/Soundex.h"
#include "GeocoderImp.h"
#include "../geocommon/GeoUtil.h"
#include "GeoAddressTemplate.h"
#include "GeoDataVersion.h"
#include "../global/Filesys.h"

XERCES_CPP_NAMESPACE_USE

// Debug tracing
#ifdef NDEBUG
	#define GEOTRACE(x)
#endif

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Utilty methods
	//////////////////////////////////////////////////////////////////////

	const double GeocoderImp::PI = 3.14159265359;
	const double GeocoderImp::EARTH_RADIUS_MILES = (7913.125 / 2);
	const double GeocoderImp::EARTH_RADIUS_FEET = (5280 * 7913.125 / 2);
	const double GeocoderImp::MILES_PER_DEG_LATITUDE = (PI * EARTH_RADIUS_MILES * 2 / 360);
	const double GeocoderImp::FEET_PER_DEG_LATITUDE = (PI * EARTH_RADIUS_FEET * 2 / 360);

	const TsString GeocoderImp::versionFilename = "Version.txt";

	///////////////////////////////////////////////////////////////////////
	// Constructor.  Will use the given reference-query interface object.
	// Inputs:
	//	Geocoder&					geocoder	The geocoder envelope class that
	//											is creating the implementation class.
	//	const TsString&			tableDir	The geocoder directory containing the
	//											lookup tables for the address parser.
	//	const TsString&			databaseDir	The geocoder directory containing the
	//											geocoder database files.
	//	MemUse						memUse		Relative amount of memory to use for caching
	///////////////////////////////////////////////////////////////////////
	GeocoderImp::GeocoderImp(
		Geocoder& geocoder_,
		const TsString& tableDir_,
		const TsString& databaseDir_,
		Geocoder::MemUse memUse
	) :
		geocoder(geocoder_),
		matchThreshold(Geocoder::DefaultMatchThreshold),
		multipleMatchThreshold(Geocoder::DefaultMultipleMatchThreshold),
		tableDir(tableDir_),
		databaseDir(databaseDir_),
		resultsCandidateIdx(0),
		streetOffsetInFeet(50.0),
		streetEndpointOffsetInFeet(0.0),
		minInterpolation(-1.0),
		maxInterpolation(2.0),
		streetOwnerTreatment(Geocoder::ChooseStreetOwnerCountrySpecific),
		m_LastLineThresholdZipOnly(900),
		m_LastLineCityWeight(334),
		m_LastLineStateWeight(333),
		m_LastLinePostcodeWeight(333),
		m_LastLineMissingCityWeight(334),
		m_LastLineMissingStateWeight(333),
		m_LastLineMissingPostcodeWeight(333),
		m_LastLineModWeight(20),
		m_FirstLinePredirWeight(90),
		m_FirstLineStreetWeight(700),
		m_FirstLinePrefixWeight(30),
		m_FirstLineSuffixWeight(90),
		m_FirstLinePostdirWeight(90),
		m_FirstLineModWeight(33),
		m_FirstLineStreetNameWeight(700),
		m_FirstLineStreetSegmentWeight(300),
		m_ReplaceLastLineCityWeight(30),
		m_ReplaceLastLineCityNoPostcodeWeight(60),
		m_ReplaceLastLinePostcodeWeight(4),
		m_ReplaceLastLineFinanceWeightSameCity(50),
		m_ReplaceLastLineFinanceWeightNewCity(200),
		m_FirstLineAddressPostcodeWeight(50),
		m_FirstLineAddressCityPostcodeWeight(150),
		m_FirstLineRangeAlphaWeight(100),
		m_FirstLineRangeEvenOddWeight(300),
		m_FirstLineRangeEvenOddUnknownWeight(30),
		m_FirstLineOutOfRangeWeight(400),
		m_bXmlInitialized(false)
	{
		bulkAllocator = new BulkAllocator;
		queryItf = new QueryImpErrorMsg(tableDir, databaseDir, memUse, geocoder_);
	}

	///////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////
	GeocoderImp::~GeocoderImp()
	{
	}

	///////////////////////////////////////////////////////////////////////
	// Open the Geocoder instance using the given query interface.
	// Will call Open() on the query interface object.
	// Returns true on success, false on failure.
	// Override ErrorMessage() to get message text.
	///////////////////////////////////////////////////////////////////////
	bool GeocoderImp::Open()
	{
		// Initialize the XML4C system
		// We put this here to prevent callers from needing to deal with XML stuff.
		try
		{
			XMLPlatformUtils::Initialize();
			m_bXmlInitialized = true;
		}
		catch (const XMLException& )
		{
			geocoder.ErrorMessage("Xerces XML initialization failed");
			return false;
		}

		try {
			// Before opening any interfaces, test for file existence and produce
			// a better human-readable error message than sucomponents will.
			TsString testFilename = databaseDir + FileSys::GetPathSeparator() +
				"CityStatePostcode.dat";
#if defined(UNIX)
			if (access(testFilename.c_str(),0 ) != 0) {
#else
			if (_access(testFilename.c_str(),0 ) != 0) {
#endif
				geocoder.ErrorMessage(("Cannot access " + testFilename).c_str());
				geocoder.ErrorMessage(("Geocoder data files not found at \"" + databaseDir + "\"").c_str());
				throw 1;
			}

			//Don't allow the geocoder to open unless the data version is correct
			if( !CheckDataVersion(databaseDir) ) {
				geocoder.ErrorMessage("Invalid version of Geocoder data");
				throw 1;
			}

			// Open the query interface.
			if (!queryItf->Open()) {
				// RefQueryInterface reports errors via its own interface.
				throw 1;
			}
			const char* errorPtr;
			TsString errorMsg;
			if (
				!addressParserFirstLine.Open(tableDir.c_str(), errorPtr) ||
				!addressParserLastLine.Open(tableDir.c_str(), errorPtr)
			) {
				errorMsg = errorPtr;
				geocoder.ErrorMessage(errorMsg.c_str());
				throw 1;
			}

			// Load lookup table
			cityReplacementTable = new LookupTable;
			if (
				!cityReplacementTable->LoadFromFile(tableDir + "/city_replacement.csv", errorMsg)
			) {
				geocoder.ErrorMessage(errorMsg.c_str());
				throw 1;
			}
		} catch (int) {
			// Cleanup XML parsing
			XMLPlatformUtils::Terminate();
			m_bXmlInitialized = false;
			return false;
		}
		// Read the ini file last.
		SetupIni();
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// SetupIni
	// This method does not return anything, as it doesn't stop operations if
	// there is no ini file to read. The defaults set up in the constructor will
	// simply be used.
	///////////////////////////////////////////////////////////////////////////////
	void GeocoderImp::SetupIni()
	{
		bool bSuccess;
		std::map<TsString, TsString> iniVars;
		char iniFilePathName[260];

		GetIniPathAndFileName(iniFilePathName);
		bSuccess = ReadIni(iniFilePathName, iniVars);
		if (bSuccess)
		{	// Get [Threshold_Values] values from map;
			if (ScanInteger(iniVars["MatchThreshold"].c_str(),matchThreshold))
				iniVars.erase("MatchThreshold");
			if (ScanInteger(iniVars["MultipleMatchThreshold"].c_str(),multipleMatchThreshold))
				iniVars.erase("MultipleMatchThreshold");
			// Get [Street_Offsets] values from map;
			if (ScanFloat(iniVars["StreetOffsetInFeet"].c_str(),streetOffsetInFeet))
				iniVars.erase("StreetOffsetInFeet");
			if (ScanFloat(iniVars["StreetEndpointOffsetInFeet"].c_str(),streetEndpointOffsetInFeet))
				iniVars.erase("StreetEndpointOffsetInFeet");
			// Get [Weight_Values] values from map;
			if (ScanInteger(iniVars["LastLineThresholdZipOnly"].c_str(),m_LastLineThresholdZipOnly))
				iniVars.erase("LastLineThresholdZipOnly");
			if (ScanInteger(iniVars["LastLineCityWeight"].c_str(),m_LastLineCityWeight))
				iniVars.erase("LastLineCityWeight");
			if (ScanInteger(iniVars["LastLineStateWeight"].c_str(),m_LastLineStateWeight))
				iniVars.erase("LastLineStateWeight");
			if (ScanInteger(iniVars["LastLinePostcodeWeight"].c_str(),m_LastLinePostcodeWeight))
				iniVars.erase("LastLinePostcodeWeight");
			if (ScanInteger(iniVars["LastLineMissingCityWeight"].c_str(),m_LastLineMissingCityWeight))
				iniVars.erase("LastLineMissingCityWeight");
			if (ScanInteger(iniVars["LastLineMissingStateWeight"].c_str(),m_LastLineMissingStateWeight))
				iniVars.erase("LastLineMissingStateWeight");
			if (ScanInteger(iniVars["LastLineMissingPostcodeWeight"].c_str(),m_LastLineMissingPostcodeWeight))
				iniVars.erase("LastLineMissingPostcodeWeight");
			if (ScanInteger(iniVars["LastLineModWeight"].c_str(),m_LastLineModWeight))
				iniVars.erase("LastLineModWeight");
			if (ScanInteger(iniVars["FirstLinePredirWeight"].c_str(),m_FirstLinePredirWeight))
				iniVars.erase("FirstLinePredirWeight");
			if (ScanInteger(iniVars["FirstLineStreetWeight"].c_str(),m_FirstLineStreetWeight))
				iniVars.erase("FirstLineStreetWeight");
			if (ScanInteger(iniVars["FirstLinePrefixWeight"].c_str(),m_FirstLinePrefixWeight))
				iniVars.erase("FirstLinePrefixWeight");
			if (ScanInteger(iniVars["FirstLineSuffixWeight"].c_str(),m_FirstLineSuffixWeight))
				iniVars.erase("FirstLineSuffixWeight");
			if (ScanInteger(iniVars["FirstLinePostdirWeight"].c_str(),m_FirstLinePostdirWeight))
				iniVars.erase("FirstLinePostdirWeight");
			if (ScanInteger(iniVars["FirstLineModWeight"].c_str(),m_FirstLineModWeight))
				iniVars.erase("FirstLineModWeight");
			if (ScanInteger(iniVars["FirstLineStreetNameWeight"].c_str(),m_FirstLineStreetNameWeight))
				iniVars.erase("FirstLineStreetNameWeight");
			if (ScanInteger(iniVars["FirstLineStreetSegmentWeight"].c_str(),m_FirstLineStreetSegmentWeight))
				iniVars.erase("FirstLineStreetSegmentWeight");
			if (ScanInteger(iniVars["ReplaceLastLineCityWeight"].c_str(),m_ReplaceLastLineCityWeight))
				iniVars.erase("ReplaceLastLineCityWeight");
			if (ScanInteger(iniVars["ReplaceLastLineCityNoPostcodeWeight"].c_str(),m_ReplaceLastLineCityNoPostcodeWeight))
				iniVars.erase("ReplaceLastLineCityNoPostcodeWeight");
			if (ScanInteger(iniVars["ReplaceLastLinePostcodeWeight"].c_str(),m_ReplaceLastLinePostcodeWeight))
				iniVars.erase("ReplaceLastLinePostcodeWeight");
			if (ScanInteger(iniVars["ReplaceLastLineFinanceWeightSameCity"].c_str(),m_ReplaceLastLineFinanceWeightSameCity))
				iniVars.erase("ReplaceLastLineFinanceWeightSameCity");
			if (ScanInteger(iniVars["ReplaceLastLineFinanceWeightNewCity"].c_str(),m_ReplaceLastLineFinanceWeightNewCity))
				iniVars.erase("ReplaceLastLineFinanceWeightNewCity");
			// Set 'currently unused' values too...
			if (ScanInteger(iniVars["FirstLineAddressPostcodeWeight"].c_str(),m_FirstLineAddressPostcodeWeight))
				iniVars.erase("FirstLineAddressPostcodeWeight");
			if (ScanInteger(iniVars["FirstLineAddressCityPostcodeWeight"].c_str(),m_FirstLineAddressCityPostcodeWeight))
				iniVars.erase("FirstLineAddressCityPostcodeWeight");
			if (ScanInteger(iniVars["FirstLineRangeAlphaWeight"].c_str(),m_FirstLineRangeAlphaWeight))
				iniVars.erase("FirstLineRangeAlphaWeight");
			if (ScanInteger(iniVars["FirstLineRangeEvenOddWeight"].c_str(),m_FirstLineRangeEvenOddWeight))
				iniVars.erase("FirstLineRangeEvenOddWeight");
			if (ScanInteger(iniVars["FirstLineRangeEvenOddUnknownWeight"].c_str(),m_FirstLineRangeEvenOddUnknownWeight))
				iniVars.erase("FirstLineRangeEvenOddUnknownWeight");
			if (ScanInteger(iniVars["FirstLineOutOfRangeWeight"].c_str(),m_FirstLineOutOfRangeWeight))
				iniVars.erase("FirstLineOutOfRangeWeight");

			if (!iniVars.empty())
			{
				// TODO 5	Something in the ini file that we did not use; Perhaps something was misspelled;
				//			Should we tell the user?
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetIniPathAndFileName
	///////////////////////////////////////////////////////////////////////////////
	void GeocoderImp::GetIniPathAndFileName(char * filePathName)
	{
		filePathName[0] = '\0';
#if defined(WIN32)
		DWORD strLen = 260; //MAX_PATH

		strLen = GetModuleFileName(dllModuleInstance,filePathName,strLen);
		if (strLen > 0)
		{
			PathRemoveFileSpec(filePathName);
			strcat(filePathName,"\\");
		}
#endif
#if defined(UNIX)
		// TODO will need an implementation for getting the path on Unix/Linux;
#endif
		strcat(filePathName,"GeocoderImp.ini");
	}

	///////////////////////////////////////////////////////////////////////////////
	// ReadIni
	///////////////////////////////////////////////////////////////////////////////
	template <class TMap> bool GeocoderImp::ReadIni(char *szFileName, TMap & vars)
	{
		FILE *pFile = fopen(szFileName, "r");
		if (pFile)
		{
			char buffer[2048];
			while (fgets(buffer, sizeof(buffer), pFile))
			{
				if (*buffer && *buffer!=';' && *buffer!='[') // for right now, skip [Label] lines;
				{
					char * pItem = strtok(buffer, "=");
					if (!pItem || !*pItem)
						continue;
					else
					{ // trim whitespace;
						char *endptr;

						while (*pItem && *pItem <= ' ') pItem++;
						endptr = pItem;
						while (*endptr > ' ') endptr++;
						*endptr = '\0';
						if (!*pItem) // empty string for the item;
							continue;
					}

					char * pValue = strtok(NULL, "\n\0");
					if (!pValue || !*pValue)
						continue;
					else
					{ // trim whitespace;
						char *endptr;

						while (*pValue && *pValue <= ' ') pValue++;
						endptr = pValue;
						while (*endptr > ' ') endptr++;
						*endptr = '\0';
						if (!*pValue) // empty string for the value;
							continue;
					}

					vars[pItem] = pValue;
				}
			}
			fclose(pFile);
			return true;
		}
		return false;
	} // ReadIni

	///////////////////////////////////////////////////////////////////////
	// Close the Geocoder instance.
	// This will be called by the destructor.
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::Close()
	{
		if (queryItf.get())
			queryItf->Close();
		addressParserFirstLine.Close();
		addressParserLastLine.Close();
		cityReplacementTable = 0;
		// Cleanup XML parsing
		if (m_bXmlInitialized)
			XMLPlatformUtils::Terminate();
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
	Geocoder::GlobalStatus GeocoderImp::CodeAddress(
		const char* line1,
		const char* line2
	) {
		// Clear out information from last coding
		bulkAllocator->Reset();
		lastLineParseCandidates.clear();
		firstLineParseCandidates.clear();
		geocodeResults.clear();
		sortedGeocodeResults.clear();
		resultsCandidateIdx = 0;
		resultsGlobalStatus = Geocoder::GlobalFailure;

		// Parse the last-line address.
		if (!addressParserLastLine.Parse(line2, lastLineParseCandidates.UseExtraOnEnd(), true)) {
			// No last line candidate was found
			resultsGlobalStatus = Geocoder::GlobalFailure;
			return resultsGlobalStatus; // Without a last line, we can't do anything (not even centroids). So, just get out;
		}
		addressParserLastLine.PermuteAddress(~0);

		// Retrieve last-line parse permutations.
		while (true) {
			AddressParserLastLine::ParseCandidate& candidate = lastLineParseCandidates.UseExtraOnEnd();
			if (!addressParserLastLine.NextAddressPermutation(candidate, true)) {
				lastLineParseCandidates.pop_back();
				break;
			}
			GEOTRACE(TsString("Parsed last-line candidate: (") + candidate.city + ") (" + candidate.state + ") (" + candidate.postcode + ")");
		}

		// Parse the first-line address and get the first candidate.
		// Now parsing this BEFORE choosing a last line candidate, should that fail.
		// This is so the user will know that the failure wasn't because we didn't look at
		// their first line data (even though failing last line data aborts the process anyway).
		AddressParserFirstLine::ParseCandidate& candidate = firstLineParseCandidates.UseExtraOnEnd();
		if (!addressParserFirstLine.Parse(line1, candidate, true)) {
			firstLineParseCandidates.pop_back();
		} else {
			GEOTRACE(TsString("Parsed first-line candidate: (") + candidate.number + ") (" + candidate.predir + ") (" + candidate.street + ") (" + candidate.suffix +") (" + candidate.postdir +") (" + candidate.unitDesignator +") (" + candidate.unitNumber + ") (" + candidate.street2 + ")");
		}

		// Retrieve permutations of first-line parse candidates, using all available permutations.
		addressParserFirstLine.PermuteAddress(~0);
		while (true) {
			AddressParserFirstLine::ParseCandidate& candidate = firstLineParseCandidates.UseExtraOnEnd();
			if (!addressParserFirstLine.NextAddressPermutation(candidate, true)) {
				firstLineParseCandidates.pop_back();
				break;
			}
			GEOTRACE(TsString("Parsed first-line candidate: (") + candidate.number + ") (" + candidate.predir + ") (" + candidate.street + ") (" + candidate.suffix +") (" + candidate.postdir +") (" + candidate.unitDesignator +") (" + candidate.unitNumber + ") (" + candidate.street2 + ")");
		}

		// Best last-line chosen and associated information.
		CityStatePostcode bestCityStatePostcode;
		int bestLastLineScore = 0;
		int bestLastLineCandidateIdx = -1;
		int bestLastLineFlags = 0;

		if (ChooseBestLastLine(
				bestCityStatePostcode,
				bestLastLineScore,
				bestLastLineCandidateIdx,
				bestLastLineFlags))
		{	// Successfully chosen a last line candidate, look at already parsed first line candidates; 
			// Create a list of Finance Areas to search
			GetRelatedFinanceAreas(
				bestCityStatePostcode.postcode,
				bestCityStatePostcode.financeNumber,
				uniqueFAList
			);

			// Given the best last-line and the uniqueFAList, walk the first-line parse candidates
			// and compare them agains the database.
			ChooseBestFirstLineResults(
				bestCityStatePostcode,
				bestLastLineScore,
				bestLastLineCandidateIdx,
				bestLastLineFlags
			);

			if (geocodeResults.empty() && !uniqueFAList.empty()) {
				// No geocode results were found.
				// Sometimes this happens because the specified city has multiple FAs, and we are
				// searching in the wrong FA.  This is unusual, but occurs because of bad source data.
				// To work around hte problem, search the CSP database for records with the correct city
				// but a different FA than the one in the chosen bestLastLine.  Then extact a new 
				// uniqueFAList from those records and try again.
				const char* excludeFA = uniqueFAList[0];
				uniqueFAList.clear();

				GetAllFinanceAreasForCity(
					bestCityStatePostcode.city,
					bestCityStatePostcode.state,
					excludeFA,
					uniqueFAList
				);

				// Try again using the new FA list.
				ChooseBestFirstLineResults(
					bestCityStatePostcode,
					bestLastLineScore,
					bestLastLineCandidateIdx,
					bestLastLineFlags
				);
			}


			// Dedupe results by street name
			{
				// Get pointers to results
				for (unsigned i = 0; i < geocodeResults.size(); i++) {
					sortedGeocodeResults.push_back(&geocodeResults[i]);
				}
				// Sort by street name(s) and score
				GeocodeResultsCmpStreetNameScore cmp;
				std::sort(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp);
				// Unique by street name(s)
				GeocodeResultsCmpStreetNameDedupe cmp2;
				SortedGeocodeResults::iterator iter = 
					std::unique(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp2);
				sortedGeocodeResults.resize(iter - sortedGeocodeResults.begin());
			}

			// Dedupe results by street alias.
			{
				// Sort by StreetSegmentID and score.
				GeocodeResultsCmpStreetSegmentIDScore cmp;
				std::sort(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp);
				// Unique by StreetSegmentID
				GeocodeResultsCmpStreetSegmentIDDedupe cmp2;
				SortedGeocodeResults::iterator iter = 
					std::unique(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp2);
				sortedGeocodeResults.resize(iter - sortedGeocodeResults.begin());
			}

			// Dedupe intersection results by position
			if (
				!sortedGeocodeResults.empty() && 
				(sortedGeocodeResults[0]->results.geoStatus & Geocoder::GeocodeIntersection) != 0
			) {
				// We have to actually go get the coding results to do this.
				// This will cause some redundant processing, but intersections are
				// less common than street addresses.
				for (unsigned i = 0; i < sortedGeocodeResults.size(); i++) {
					CodeResult(*sortedGeocodeResults[i]);
				}
				// Sort by position and score.
				GeocodeResultsCmpIntersectionLatLon cmp;
				std::sort(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp);
				// Unique by position
				GeocodeResultsCmpIntersectionLatLonDedupe cmp2;
				SortedGeocodeResults::iterator iter = 
					std::unique(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp2);
				sortedGeocodeResults.resize(iter - sortedGeocodeResults.begin());
			}


			// Sort the results by descending score.
			{
				GeocodeResultsCmpScore cmp;
				std::sort(sortedGeocodeResults.begin(), sortedGeocodeResults.end(), cmp);
			}
		} // end if-- ChooseBestLastLine;
		else // ChooseBestLastLine failed; setup results that it would normally return to be the first last line candidate;
		{
			// Since we passed the last line parser okay, there should be at least ONE item in the last line parse candidate list;
			assert(!lastLineParseCandidates.empty());
			memset(&bestCityStatePostcode, 0, sizeof(bestCityStatePostcode));
			bestLastLineCandidateIdx = 0;
			StateAbbrToCode(lastLineParseCandidates[bestLastLineCandidateIdx].state, bestCityStatePostcode.country, bestCityStatePostcode.state);
		}
		if (sortedGeocodeResults.empty()) {
			// No results... try to find a centroid

			if (firstLineParseCandidates.empty()) {
				// Add a first-line parse candidate so we have something to add to the result.
				AddressParserFirstLine::ParseCandidate parseCandidate;
				memset(&parseCandidate, 0, sizeof(parseCandidate));
				firstLineParseCandidates.push_back(parseCandidate);
			}

			if (bestCityStatePostcode.postcode[0] == 0) {
				// Fill this in with candidate to make the results happy.
				strcpy(bestCityStatePostcode.city, lastLineParseCandidates[bestLastLineCandidateIdx].city);
				strcpy(bestCityStatePostcode.postcode, lastLineParseCandidates[bestLastLineCandidateIdx].postcode);
				strcpy(bestCityStatePostcode.stateAbbr, lastLineParseCandidates[bestLastLineCandidateIdx].state);
				// re-score last line (if things are "missing");
				bestLastLineScore = ScoreLastLine(lastLineParseCandidates[bestLastLineCandidateIdx],bestCityStatePostcode,bestLastLineFlags);
				UpdateLastLineMatchStatusFlags(
					lastLineParseCandidates[0],
					lastLineParseCandidates[bestLastLineCandidateIdx],
					NULL,
					bestLastLineCandidateIdx,
					bestLastLineFlags);
			}
			else
			{
				// Since we are going to use the original ZIP to find a centroid, give the user their
				// original data back as a result, NOT what MIGHT be in bestCityStatePostcode from an
				// unsuccessful attempt to find an address (for example, from a changed bestCityStatePostcode
				// because the original ZIP is a point ZIP).
				if (_stricmp(lastLineParseCandidates[bestLastLineCandidateIdx].postcode, bestCityStatePostcode.postcode) != 0)
				{
					strcpy(bestCityStatePostcode.postcode, lastLineParseCandidates[bestLastLineCandidateIdx].postcode);
					if (bestLastLineFlags&Geocoder::MatchChangedPostcode) // since we're changing it back, re-score the last line;
						bestLastLineScore = ScoreLastLine(lastLineParseCandidates[bestLastLineCandidateIdx],bestCityStatePostcode,bestLastLineFlags);
					UpdateLastLineMatchStatusFlags(
						lastLineParseCandidates[0],
						lastLineParseCandidates[bestLastLineCandidateIdx],
						NULL,
						bestLastLineCandidateIdx,
						bestLastLineFlags);
				}
			}
			PostcodeCentroid postcodeCentroid;

			// Look for ZIP+extension first
			char buf[32];
			strcpy(buf, lastLineParseCandidates[bestLastLineCandidateIdx].postcode);
			strcat(buf, lastLineParseCandidates[bestLastLineCandidateIdx].postcodeExt);
			if (queryItf->GetPostcodeCentroidFromPostcodeCached(buf, postcodeCentroid)) {
				GeocodeResultsPlus& result = geocodeResults.UseExtraOnEnd();
				MakeResult(
					firstLineParseCandidates[0],
					bestCityStatePostcode,
					postcodeCentroid,
					lastLineParseCandidates[bestLastLineCandidateIdx].postcodeExt,
					bestLastLineFlags,
					bestLastLineScore,
					result
				);
				// Add the new result to the sorted list too.
				assert(!geocodeResults.empty());
				sortedGeocodeResults.push_back(&geocodeResults[0]);
			} else {
				// Look for ZIP without extension
				strcpy(buf, lastLineParseCandidates[bestLastLineCandidateIdx].postcode);
				if (queryItf->GetPostcodeCentroidFromPostcodeCached(buf, postcodeCentroid)) {
					GeocodeResultsPlus& result = geocodeResults.UseExtraOnEnd();
					MakeResult(
						firstLineParseCandidates[0],
						bestCityStatePostcode,
						postcodeCentroid,
						"",					// no extension
						bestLastLineFlags,
						bestLastLineScore,
						result
					);
					// Add the new result to the sorted list too.
					assert(!geocodeResults.empty());
					sortedGeocodeResults.push_back(&geocodeResults[0]);
				}
			}
		} // end if-- Empty results, trying to find a Zip Centroid;

		if (sortedGeocodeResults.empty()) {
			resultsGlobalStatus =  Geocoder::GlobalFailure;
		} else if (ResultsAreMultiple()) {
			resultsGlobalStatus =  Geocoder::GlobalMultiple;
		} else {
			resultsGlobalStatus =  Geocoder::GlobalSingle;
		}
		return resultsGlobalStatus;
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
	bool GeocoderImp::GetNextCandidate(
		Geocoder::GeocodeResults& resultsReturn
	) {
		if (unsigned(resultsCandidateIdx) < sortedGeocodeResults.size()) {
			CodeResult(*sortedGeocodeResults[resultsCandidateIdx]);
			resultsReturn.GetResultsInternal() = sortedGeocodeResults[resultsCandidateIdx]->results;
			resultsCandidateIdx++;
			return true;
		} else {
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Set the threshold score that determines a match (0-1000)
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::SetMatchThreshold(int threshold)
	{
		matchThreshold = threshold;
	}

	///////////////////////////////////////////////////////////////////////
	// Set the score delta that determines a multiple (0-1000)
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::SetMultipleThreshold(int threshold)
	{
		multipleMatchThreshold = threshold;
	}

	///////////////////////////////////////////////////////////////////////
	// Given a last-line parse candidate and a CityStatePostcode record, 
	// calculdate a score in the rance 0-1000 indicating the quality
	// of match.
	// Inputs:
	//	const AddressParserLastLine::ParseCandidate&	parseCandidate
	//									The attempted last-line parse
	//	const CityStatePostcode& cityStatePostcode
	//									The CityStatePostcode record fetched
	//									from the reference database.
	// Outputs:
	//	int&		lastLineFlags		Flags indicating modifications
	//									to last-line to make it match
	//									the reference record.
	// Return value:
	//	int		the score between 0 and 1000, with 1000 being perfect.
	///////////////////////////////////////////////////////////////////////
	int GeocoderImp::ScoreLastLine(
		const AddressParserLastLine::ParseCandidate& parseCandidate,
		const CityStatePostcode& cityStatePostcode,
		int& lastLineFlags
	) {
		int score = 0;
		lastLineFlags = 0;

		bool havePostcode = parseCandidate.postcode[0] != 0;
		bool haveState = parseCandidate.state[0] != 0;
		bool haveCity = parseCandidate.city[0] != 0;

		// Match Zip code and finance area.
		double zipScore = 0.0;
		if (havePostcode) {
			if (strcmp(parseCandidate.postcode, cityStatePostcode.postcode) == 0) {
				zipScore = 1.0;
			} else {
				if (isalpha(parseCandidate.postcode[0])) {
					zipScore = 1.0 - EditDistance(parseCandidate.postcode, cityStatePostcode.postcode) / 3.0;
				} else {
					zipScore = 1.0 - EditDistance(parseCandidate.postcode, cityStatePostcode.postcode) / 5.0;
				}
				if (zipScore < 0.9) {
					// Don't penalize as much if FA is the same
					QueryImp::CityStatePostcodeFromPostcodeIterator iter2 = 
						queryItf->LookupCityStatePostcodeFromPostcode(parseCandidate.postcode);
					CityStatePostcode tmpCityStatePostcode;
					if (
						iter2.Next(tmpCityStatePostcode) &&
						strcmp(tmpCityStatePostcode.financeNumber, cityStatePostcode.financeNumber) == 0
					) {
							zipScore = 0.9;
					}
				}
				if (zipScore < 0.0) {
					zipScore = 0.0;
				}
			}
		}

		// Match state
		int candidateStateCode;
		const char* refStateAbbr;
		bool matchState = 
			haveState && 
			(
				StateAbbrToCode(
					parseCandidate.state, 
					queryItf->CountryFromPostalCode(parseCandidate.postcode), 
					candidateStateCode
				) &&
				candidateStateCode == cityStatePostcode.state
			) ||
			(
				StateCodeToAbbr(
					cityStatePostcode.state, 
					queryItf->CountryFromPostalCode(cityStatePostcode.postcode), 
					refStateAbbr
				) &&
				strcmp(parseCandidate.state, refStateAbbr) == 0
			);
		float stateScore = 0.0;
		if (!matchState && haveState)
		{	// let's try getting "close" on the state, in case there was an entry error
			// such as "OR" (Oregon) for "OK" (Oklahoma).
			if (refStateAbbr && *refStateAbbr)
			{	// Should be true, as StateCodeToAbbr should have been run if we get to here.
				size_t stateLength = strlen(parseCandidate.state) + strlen(refStateAbbr);
				stateScore = float(1.0 - EditDistance(parseCandidate.state, refStateAbbr) / (float) (stateLength / 2));
				if (stateScore < 0)
					stateScore = 0.0;
			}
		}

		// Match city
		size_t totalCityLength = strlen(parseCandidate.city) + strlen(cityStatePostcode.city);
		float cityScore;
		if (haveCity && totalCityLength > 1) {
			cityScore = float(
				1.0 - 
				EditDistance(parseCandidate.city, cityStatePostcode.city) / (float)(totalCityLength / 2));
			if (cityScore < 0) {
				cityScore = 0.0;
			}
			if (cityScore < 0.75) {
				// Try testing a city replacement alias.
				const char* newCity;
				if (FindReplacementCity(cityStatePostcode.postcode, cityStatePostcode.stateAbbr, parseCandidate.city, newCity)) {
					float newCityScore = float(
						1.0 - 
						EditDistance(newCity, cityStatePostcode.city) / 
						(float)((strlen(newCity) + strlen(cityStatePostcode.city)) / 2));
					newCityScore -= (float)(m_ReplaceLastLineCityWeight / 100.0);
					if (newCityScore > cityScore) {
						cityScore = newCityScore;
						lastLineFlags |= Geocoder::MatchCityAliasUsed;
					}
				}
			}
			if ((parseCandidate.flags & AddressParserLastLine::ParseCandidate::CityAlias) != 0) {
				lastLineFlags |= Geocoder::MatchCityAliasUsed;
			}
		} else {
			cityScore = 0.0;
		}

		// Match the state and Zip.  It is possible to be missing parts of
		// the parse-candidate address.  The following combinations of
		// missing information are allowed:
		//    Missing Zip
		//    Missing State
		//    Missing City
		//    Missing City and State
		if (!havePostcode) {
			// Missing Zip 
			if (!haveCity || !haveState) {
				// Nothing may be missing
				return 0;
			}
			lastLineFlags |= Geocoder::MatchSuppliedPostcode;
			score = 
				m_LastLineMissingPostcodeWeight + 
				(matchState ? m_LastLineStateWeight : (int)(stateScore * m_LastLineStateWeight)) +
				(int)(cityScore * m_LastLineCityWeight);
		} else if (!haveCity) {
			if (haveState) {
				// Missing city only
				lastLineFlags |= Geocoder::MatchSuppliedCity;
				score = 
					(int)(zipScore * m_LastLinePostcodeWeight) +
					(matchState ? m_LastLineStateWeight : (int)(stateScore * m_LastLineStateWeight)) +
					m_LastLineMissingCityWeight;
			} else {
				// Missing city and state.  Require exact zip match
				lastLineFlags |= Geocoder::MatchSuppliedCity | Geocoder::MatchSuppliedState;
				score = 
					(strcmp(parseCandidate.postcode, cityStatePostcode.postcode) == 0 ? m_LastLinePostcodeWeight : 0) +
					m_LastLineMissingStateWeight +
					m_LastLineMissingCityWeight;

			}
		} else if (!haveState) {
			// Missing state only
			lastLineFlags |= Geocoder::MatchSuppliedState;
			score = 
				(int)(zipScore * m_LastLinePostcodeWeight) +
				m_LastLineMissingStateWeight +
				(int)(cityScore * m_LastLineCityWeight);
		} else {
			// Have all components.

			// If the Zip is perfect, don't penalize the city as badly.  This is used by the
			// logic that digs for related cities in the same Zip code, especially for Canada.
			if (zipScore == 1.0) {
				score = 
					m_LastLinePostcodeWeight +
					(int)((cityScore + 1.0) * 0.5 * m_LastLineCityWeight) + 
					(matchState ? m_LastLineStateWeight : (int)(stateScore * m_LastLineStateWeight));
			} else {
				score = 
					(int)(zipScore * m_LastLinePostcodeWeight) +
					(matchState ? m_LastLineStateWeight : (int)(stateScore * m_LastLineStateWeight)) +
					(int)(cityScore * m_LastLineCityWeight);
			}
		}

		// Reduce score by number of modifications made to parse candidate.
		score -= parseCandidate.numberOfMods * m_LastLineModWeight;
		if (score < 0) {
			score = 0;
		}
		return score;
	}


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
	//				cityStatePostcode	The CityStatePostcode representing the
	//									current best last-line choice.
	// Outputs:
	//	int&		firstLineFlags		Flags indicating modifications
	//									to first-line to make it match
	//									the reference record.
	//	CityStatePostcode& 
	//				cityStatePostcode	A revised last-line choice based on the
	//									CityStatePostcode of the intersection.
	// Return value:
	//	int		the score between 0 and 1000, with 1000 being perfect.
	///////////////////////////////////////////////////////////////////////
	int GeocoderImp::ScoreStreetName(
		const AddressParserFirstLine::ParseCandidate& parseCandidate,
		const StreetName& streetName,
		const CityStatePostcode& cityStatePostcode,
		int& firstLineFlags,
		CityStatePostcode& chosenCityStatePostcode
	) {
		firstLineFlags = 0;

		// Match street name
		double streetScore = CompareStreetNames(parseCandidate.street, streetName.street);

		if (streetScore != 1.0) {
			firstLineFlags |= Geocoder::MatchChangedStName;
		}
		int score = (int)(streetScore * m_FirstLineStreetWeight);

		// Match Prefix
		if (strcmp(parseCandidate.prefix, streetName.prefix) == 0) {
			score += m_FirstLinePrefixWeight;
		} else {
			firstLineFlags |= Geocoder::MatchChangedStPrefix;
		}


		// Special case; small penalty for reversed predir/postdir.
		// This test is:
		//		Exactly one of the predir/postdir is set AND
		//		The candidate predir == reference postdir AND
		//		The candidate postdir == reference predir AND
		if (
			((parseCandidate.predir[0] == 0) ^ (parseCandidate.postdir[0] == 0)) &&
			strcmp(parseCandidate.predir, streetName.postdir) == 0 &&
			strcmp(parseCandidate.postdir, streetName.predir) == 0
		) {
			// Reversed directionals; small penalty
			score += (int)(0.7 * (m_FirstLinePredirWeight + m_FirstLinePostdirWeight));
			firstLineFlags |= (Geocoder::MatchChangedPredir | Geocoder::MatchChangedPostdir);
		} else {
			// Match predir
			if (strcmp(parseCandidate.predir, streetName.predir) == 0) {
				score += m_FirstLinePredirWeight;
			} else {
				firstLineFlags |= Geocoder::MatchChangedPredir;
			}

			// Match postdir
			if (strcmp(parseCandidate.postdir, streetName.postdir) == 0) {
				score += m_FirstLinePostdirWeight;
			} else {
				firstLineFlags |= Geocoder::MatchChangedPostdir;
			}
		}

		// Match suffix
		if (strcmp(parseCandidate.suffix, streetName.suffix) == 0) {
			score += m_FirstLineSuffixWeight;
		} else {
			firstLineFlags |= Geocoder::MatchChangedStSuffix;
			// Bigger penalty to change suffix than to add it.
			if (parseCandidate.suffix[0] == 0) {
				score += m_FirstLineSuffixWeight / 2;
			}
		}

		score -= parseCandidate.numberOfMods * m_FirstLineModWeight;
		if (score < 0) {
			score = 0;
		}

		// 
		// Attempt to choose a new last-line that best matches both the 
		// last-line parse candidate and the StreetName's city/state/postcode.
		// 
		CityStatePostcode tmpCityStatePostcode;

		if (streetName.cityStatePostcodeID == cityStatePostcode.ID) {
			// Exact CityStatePostcode match.  
			chosenCityStatePostcode = cityStatePostcode;
			return score;
		} 
		
		if (!queryItf->GetCityStatePostcode(streetName.cityStatePostcodeID, tmpCityStatePostcode)) {
			// This is really a data error.  All StreetName records should have a parent
			return 0;
		}

		// Found the StreetName's parent city/state/postcode
		if (
			tmpCityStatePostcode.streetNameIDFirst == cityStatePostcode.streetNameIDFirst &&
			tmpCityStatePostcode.streetNameIDLast == cityStatePostcode.streetNameIDLast
		) {
			// The two CSZ records are aliases of each other because they point to the same
			// set of StreetName records.  Keep the best lastline because it already has
			// an optimal match.
			chosenCityStatePostcode = cityStatePostcode;
			return score;
		} 


		// Notes about streetOwnerTreatment:
		//
		// For Canada, choose the owning CityStatePostcode instead of the original one.
		// This is done because cities that share the 3-letter postal code are close 
		// to each other but not necessarily "vanity" aliases as they are in the USA.
		//
		// In the USA, we normally choose the last-line if the postcode group is the same,
		// because cities that are in the same postcode group are considered "vanity" aliases,
		// and we choose the one the user typed (preferred last-line).

		bool chooseStreetOwner;
		switch (streetOwnerTreatment) {
		case Geocoder::ChooseStreetOwnerOverLastLine:
			chooseStreetOwner = true;
			break;
		case Geocoder::ChooseLastLineOverStreetOwner:
			chooseStreetOwner = false;
			break;
		default:
		case Geocoder::ChooseStreetOwnerCountrySpecific:
			chooseStreetOwner = (strcmp(cityStatePostcode.country, "CA") == 0);
			break;
		}

		if (!chooseStreetOwner) {
			PostcodeAlias postcodeGroupInput, postcodeGroup;
			if (
				queryItf->LookupPostcodeGroup(cityStatePostcode.postcode, postcodeGroupInput) &&
				queryItf->LookupPostcodeGroup(tmpCityStatePostcode.postcode, postcodeGroup) &&
				strcmp(postcodeGroupInput.postcodeGroup, postcodeGroup.postcodeGroup) == 0
			) {
				// The owner of the StreetName and the best last-line are both in the
				// same ZIP group.  In the USA they are considered "vanity" aliases, so
				// we choose the one the user typed (preferred last-line).
				chosenCityStatePostcode = cityStatePostcode;
				return score;
			} else if (strcmp(cityStatePostcode.city, tmpCityStatePostcode.city) == 0) {
				// Same city, different ZIP.  Choose the street-owner.
			} else if (FindCityStatePostcodeFromCityPostcode(cityStatePostcode.city, tmpCityStatePostcode.postcode, chosenCityStatePostcode)) {
				// Found a CityStatePostcode where the city name matches the input
				// and the Zip matches that of the StreetName record.  This is good enough.
				return score;
			}
		}

		// Choose the street owner
		chosenCityStatePostcode = tmpCityStatePostcode;
		return score;
	}


	///////////////////////////////////////////////////////////////////////
	// Given an address number and a StreetSegment, calculate a score in 
	// the range 0-1000 indicating the quality of match.
	// Inputs:
	//	const char*		parsedPostcodeExt	The postcode extension parsed from
	//										the input last-line, if any.
	//	const GeoUtil&	addrTemplate	The address number candidate
	//	const StreetSegment&
	//					streetSegment		The StreetSegment record fetched
	//									from the reference database.
	// Outputs:
	//	int&		streetSegmentFlags	Flags indicating mismatch of address
	//									number and range.
	//	double&		tieBreaker			If score is 1.0, then this will contain
	//									a "tiebreaker" number; higher numbers are better.
	// Return value:
	//	int		The score between 0 and 1000, with 1000 being perfect.
	///////////////////////////////////////////////////////////////////////
	int GeocoderImp::ScoreStreetSegment(
		const char* parsedPostcodeExt,
		const AddressTemplate& addrTemplate,
		const StreetSegment& streetSegment,
		int& streetSegmentFlags,
		double& tieBreaker
	) {
		streetSegmentFlags = 0;
		AddressRangeTemplate addrRangeTemplate(streetSegment.addrLow, streetSegment.addrHigh);
		bool inRange;
		bool evenOddMatch;
		double score = addrRangeTemplate.Score(addrTemplate, inRange, evenOddMatch);
		if (!inRange) {
			streetSegmentFlags |= Geocoder::MatchAddressNbrOutOfRange;
		}
		if (!evenOddMatch) {
			streetSegmentFlags |= Geocoder::MatchEvenOddDiffer;
		}

		if (
			parsedPostcodeExt[0] != 0 && 
			streetSegment.postcodeExt[0] != 0 &&
			strcmp(parsedPostcodeExt, streetSegment.postcodeExt) != 0
		) {
			// Slight penalty for incorrect postcode ext.  Just enough to tip
			// balance in favor of correct extension.
			streetSegmentFlags |= Geocoder::MatchChangedPostcodeExt;
			score -= .005;
		}

		if (score == 1.0) {
			// Perfect score; compute the tiebreaker.
			double range = addrRangeTemplate.GetRangeSize();
			tieBreaker = 1.0 / (range + 0.00000001);
		} else {
			tieBreaker = 0.0;
		}

		return (int)(score * 1000);
	}


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
	int GeocoderImp::ScoreStreetIntersection(
		const AddressParserFirstLine::ParseCandidate& parseCandidate,
		const StreetIntersection& streetIntersection,
		const CityStatePostcode& cityStatePostcode,
		int& firstLineFlags,
		CityStatePostcode& chosenCityStatePostcode
	) {
		firstLineFlags  = 0;

		// We ignore everything except the street name.
		const char* street1Name = streetIntersection.streetName1.street;
		const char* street2Name = streetIntersection.streetName2.street;

		float bestScore = 0.0;

		// Loop twice, reversing street order the second time.
		for (int loop = 0; loop < 2; loop++) {
			double streetScore = 
				CompareStreetNames(parseCandidate.street, street1Name) +
				CompareStreetNames(parseCandidate.street2, street2Name);

			if (streetScore > bestScore) {
				bestScore = float(streetScore);
			}

			// Swap names for next loop iteration
			const char* tmp = street1Name;
			street1Name = street2Name;
			street2Name = tmp;
		}

		int score = (int)(bestScore * 500);
		score -= parseCandidate.numberOfMods * m_FirstLineModWeight;

		// 
		// Check the last-line score and choose a last-line
		// 
		PostcodeAlias postcodeGroupInput, postcodeGroup1, postcodeGroup2;
		// Note: LookupPostcodeGroup always succeeds.
		if (
			queryItf->LookupPostcodeGroup(cityStatePostcode.postcode, postcodeGroupInput) &&
			queryItf->LookupPostcodeGroup(streetIntersection.cityStatePostcode1.postcode, postcodeGroup1) &&
			queryItf->LookupPostcodeGroup(streetIntersection.cityStatePostcode2.postcode, postcodeGroup2) &&
			(
				strcmp(postcodeGroupInput.postcodeGroup, postcodeGroup1.postcodeGroup) == 0 ||
				strcmp(postcodeGroupInput.postcodeGroup, postcodeGroup2.postcodeGroup) == 0
			)
		) {
			// Zip codes match.  Choose the input CityStatePostcode
			chosenCityStatePostcode = cityStatePostcode;
		} else if (FindCityStatePostcodeFromCityPostcode(cityStatePostcode.city, streetIntersection.cityStatePostcode1.postcode, chosenCityStatePostcode)) {
			// Found a CityStatePostcode where the city name matches the input
			// and the Zip matches the intersection.
		} else if (FindCityStatePostcodeFromCityPostcode(cityStatePostcode.city, streetIntersection.cityStatePostcode2.postcode, chosenCityStatePostcode)) {
			// Found a CityStatePostcode where the city name matches the input
			// and the Zip matches the intersection.
		} else if (streetIntersection.cityStatePostcode1.financeNumber == cityStatePostcode.financeNumber) {
			// No city/zip match but in the same FA.
			chosenCityStatePostcode = streetIntersection.cityStatePostcode1;
		} else if (streetIntersection.cityStatePostcode2.financeNumber == cityStatePostcode.financeNumber) {
			// No city/zip match but in the same FA.
			chosenCityStatePostcode = streetIntersection.cityStatePostcode2;
		} else {
			// Can't find anything in the same FA, so just pick one.
			chosenCityStatePostcode = streetIntersection.cityStatePostcode1;
		}
		
		return (score < 0) ? 0 : score;
	}

	///////////////////////////////////////////////////////////////////////
	// Create a geocoder results structure given a first-line parse 
	// candidate and a postal code centroid
	// Inputs:
	//	const AddressParserFirstLine::ParseCandidate& 
	//							candidate			The first-line parse candidate
	//	const CityStatePostcode&
	//							lastLine			The reference last line
	//	const PostcodeCentroid&
	//							postcodeCentroid	The reference postal code centroid
	//	const char*				postcodeExt			The postal code extension from the parsed last-line
	//	int						flags				Flags describing modifications
	//												that were made to achieve the match.
	//	int						score				Last line result score.
	// Outputs:
	//	GeocodeResultsPlus&		result				Results of the geocoding process,
	//												except for the lat/lon information.
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::MakeResult(
		const AddressParserFirstLine::ParseCandidate& candidate,
		const CityStatePostcode lastLine, 
		const PostcodeCentroid& postcodeCentroid,
		const char* postcodeExt,
		int flags,
		int score,
		GeocodeResultsPlus& result
	) {
		// Zero out unused fields.
		memset(&result, 0, sizeof(result));

		// Standard information
		strcpy(result.results.addrNbr, candidate.number);
		strcpy(result.results.prefix, candidate.prefix);
		strcpy(result.results.predir, candidate.predir);
		strcpy(result.results.street, candidate.street);
		strcpy(result.results.streetSuffix, candidate.suffix);
		strcpy(result.results.postdir, candidate.postdir);
		strcpy(result.results.unitDes, candidate.unitDesignator);
		strcpy(result.results.unit, candidate.unitNumber);
		strcpy(result.results.city, lastLine.city);
		result.results.state = lastLine.state;
		{
			const char* tmp = "";
			StateCodeToAbbr(lastLine.state, lastLine.country, tmp);
			strcpy(result.results.stateAbbr, tmp);
		}
		strcpy(result.results.countryCode, lastLine.country);
		strcpy(result.results.postcode, lastLine.postcode);
		strcpy(result.results.postcodeExt, postcodeExt);

		result.results.matchScore = score;
		result.results.matchStatus = (Geocoder::MatchStatus)flags;
		if (score == 1000) {
			result.results.matchStatus |= Geocoder::MatchPerfect;
		} else if (score >= matchThreshold) {
			result.results.matchStatus |= Geocoder::MatchIsValid;
		} else {
			result.results.matchStatus |= Geocoder::MatchFailed;
		}

		switch (strlen(postcodeCentroid.postcode)) {
		case 3:
			result.results.geoStatus = Geocoder::GeocodePostcode3Centroid;
			break;
		case 5:
			result.results.geoStatus = Geocoder::GeocodeZip5Centroid;
			break;
		case 6:
			result.results.geoStatus = Geocoder::GeocodePostcode6Centroid;
			break;
		case 7:
			result.results.geoStatus = Geocoder::GeocodeZip7Centroid;
			break;
		case 9:
			result.results.geoStatus = Geocoder::GeocodeZip9Centroid;
			break;
		default:
			assert(0);
			result.results.geoStatus = Geocoder::GeocodeZip5Centroid;
			break;
		}

		// Store lat/lon
		result.results.latitude = postcodeCentroid.latitude;
		result.results.longitude = postcodeCentroid.longitude;

	}

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
	void GeocoderImp::MakeResult(
		const CityStatePostcode lastLine, 
		const AddressParserFirstLine::ParseCandidate& candidate,
		const StreetIntersection& streetIntersection,
		int flags,
		int score,
		GeocodeResultsPlus& result
	) {
		const CityStatePostcode* chosenLastLine = &lastLine;

		// Zero out unused fields.
		memset(&result, 0, sizeof(result));

		// Reconcile last lines.
		if (
			streetIntersection.cityStatePostcode1.postcode != lastLine.postcode &&
			streetIntersection.cityStatePostcode2.postcode != lastLine.postcode
		) {
			// We don't have all possible last-lines available...
			// Choose a different last-line
			if (streetIntersection.cityStatePostcode1.financeNumber == lastLine.financeNumber) {
				chosenLastLine = &streetIntersection.cityStatePostcode1;
			} else if (streetIntersection.cityStatePostcode2.financeNumber == lastLine.financeNumber) {
				chosenLastLine = &streetIntersection.cityStatePostcode2;
			} else {
				// There's not a lot we can do, so choose arbitrarily.
				chosenLastLine = &streetIntersection.cityStatePostcode2;
			}
		}

		// Standard information
		result.results.addrNbr[0] = 0;
		strcpy(result.results.prefix, streetIntersection.streetName1.prefix);
		strcpy(result.results.predir, streetIntersection.streetName1.predir);
		strcpy(result.results.street, streetIntersection.streetName1.street);
		strcpy(result.results.streetSuffix, streetIntersection.streetName1.suffix);
		strcpy(result.results.postdir, streetIntersection.streetName1.postdir);
		strcpy(result.results.unitDes, candidate.unitDesignator);
		strcpy(result.results.unit, candidate.unitNumber);
		strcpy(result.results.city, chosenLastLine->city);
		result.results.state = chosenLastLine->state;
		{
			const char* tmp = "";
			StateCodeToAbbr(chosenLastLine->state, chosenLastLine->country, tmp);
			strcpy(result.results.stateAbbr, tmp);
		}
		strcpy(result.results.countryCode, chosenLastLine->country);
		result.results.countyCode = streetIntersection.streetSegment1.countyCode;
		strcpy(result.results.censusTract, streetIntersection.streetSegment1.censusTract);
		strcpy(result.results.censusBlock, streetIntersection.streetSegment1.censusBlock);
		strcpy(result.results.postcode, chosenLastLine->postcode);
		strcpy(result.results.postcodeExt, streetIntersection.streetSegment1.postcodeExt);

		result.results.matchScore = score;
		result.results.matchStatus = (Geocoder::MatchStatus)flags;
		result.results.geoStatus = Geocoder::GeocodeIntersection;

		// Overall matchs status
		if (score == 1000) {
			result.results.matchStatus |= Geocoder::MatchPerfect;
		} else if (score >= matchThreshold) {
			result.results.matchStatus |= Geocoder::MatchIsValid;
		} else {
			result.results.matchStatus |= Geocoder::MatchFailed;
		}

		// Extra intersection information
		strcpy(result.results.prefix2, streetIntersection.streetName1.prefix);
		strcpy(result.results.predir2, streetIntersection.streetName2.predir);
		strcpy(result.results.street2, streetIntersection.streetName2.street);
		strcpy(result.results.streetSuffix2, streetIntersection.streetName2.suffix);
		strcpy(result.results.postdir2, streetIntersection.streetName2.postdir);

		// Information for deduping by street name
		result.streetName1ID = streetIntersection.streetName1.ID;
		result.streetName2ID = streetIntersection.streetName2.ID;

		// Store information needed to calculate lat/lon, should this candidate
		// be chosen.
		result.streetSegment = streetIntersection.streetSegment1;
		result.streetSegment2 = streetIntersection.streetSegment2;
	}


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
	void GeocoderImp::MakeResult(
		const CityStatePostcode lastLine, 
		const AddressParserFirstLine::ParseCandidate& candidate,
		const StreetName& streetName,
		const StreetSegment& streetSegment,
		int flags,
		int score,
		GeocodeResultsPlus& result
	) {

		// Zero out unused fields.
		memset(&result, 0, sizeof(result));

		// Standard information
		strcpy(result.results.addrNbr, candidate.number);
		strcpy(result.results.prefix, streetName.prefix);
		strcpy(result.results.predir, streetName.predir);
		strcpy(result.results.street, streetName.street);
		strcpy(result.results.streetSuffix, streetName.suffix);
		strcpy(result.results.postdir, streetName.postdir);
		strcpy(result.results.unitDes, candidate.unitDesignator);
		strcpy(result.results.unit, candidate.unitNumber);
		strcpy(result.results.city, lastLine.city);
		result.results.state = lastLine.state;
		{
			const char* tmp = "";
			StateCodeToAbbr(lastLine.state, lastLine.country, tmp);
			strcpy(result.results.stateAbbr, tmp);
		}
		strcpy(result.results.countryCode, lastLine.country);
		result.results.countyCode = streetSegment.countyCode;
		strcpy(result.results.censusTract, streetSegment.censusTract);
		strcpy(result.results.censusBlock, streetSegment.censusBlock);
		strcpy(result.results.postcode, lastLine.postcode);
		strcpy(result.results.postcodeExt, streetSegment.postcodeExt);

		result.results.matchScore = score;
		result.results.matchStatus = (Geocoder::MatchStatus)flags;
		if (score == 1000) {
			result.results.matchStatus |= Geocoder::MatchPerfect;
		} else if (score >= matchThreshold) {
			result.results.matchStatus |= Geocoder::MatchIsValid;
		} else {
			result.results.matchStatus |= Geocoder::MatchFailed;
		}
		result.results.geoStatus = Geocoder::GeocodeAddress;

		// Information for deduping by street name
		result.streetName1ID = streetName.ID;
		result.streetName2ID = 0;

		// Store information needed to calculate lat/lon, should this candidate
		// be chosen.
		result.streetSegment = streetSegment;
	}

	///////////////////////////////////////////////////////////////////////
	// Geocode an intermediate result.
	// Inputs:
	//	GeocodeResultsPlus&		result		Contains all of the address information
	//										on input.
	// Outputs:
	//	GeocodeResultsPlus&		result		Result with lat/lon calculated.
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::CodeResult(
		GeocodeResultsPlus& result
	) {
		if ((result.results.geoStatus & Geocoder::GeocodePostcodeAnyCentriod) != 0) {
			// Already coded.
			return;
		}

		// Query all points for the main street segment.
		GetSegmentPointsForStreetSegment(result.streetSegment, tempPoints1);

		// 
		// Check for Intersection coding
		//
		if ((result.results.geoStatus & Geocoder::GeocodeIntersection) != 0) {
			// Query all points for the main street segment.
			GetSegmentPointsForStreetSegment(result.streetSegment2, tempPoints2);

			// Find the intersecting points.  They can only intersect on the endpoints.
			CoordinatePoint pt1 = tempPoints1[tempPoints1.size()-1];
			CoordinatePoint pt2 = tempPoints2[tempPoints2.size()-1];

			if (tempPoints1[0] == tempPoints2[0] || tempPoints1[0] == pt2) {
				result.results.latitude = tempPoints1[0].latitude;
				result.results.longitude = tempPoints1[0].longitude;
			} else if (pt1 == tempPoints2[0] || pt1 == pt2) {
				result.results.latitude = pt1.latitude;
				result.results.longitude = pt1.longitude;
			} else {
				// Does not really intersect.  This seems like an internal problem.
				result.results.geoStatus = Geocoder::GeocodeFailed;
				result.results.latitude = 0.0;
				result.results.longitude = 0.0;
			}
			return;
		}

		//
		// Address level coding
		// 
		if (tempPoints1.empty()) {
			// This is really an internal data error, but we call it failure
			result.results.geoStatus = Geocoder::GeocodeFailed;
			result.results.latitude = 0.0;
			result.results.longitude = 0.0;
			return;
		}

		// Make a templatized representation of the address and the range;
		AddressTemplate addrTemplate(result.results.addrNbr);
		AddressRangeTemplate addrRangeTemplate(result.streetSegment.addrLow, result.streetSegment.addrHigh);

		double latStart = tempPoints1[0].latitude;
		double lonStart = tempPoints1[0].longitude;
		double latEnd = tempPoints1[tempPoints1.size()-1].latitude;
		double lonEnd = tempPoints1[tempPoints1.size()-1].longitude;

		double interpolation;
		bool inRange;
		if (!addrRangeTemplate.Interpolate(addrTemplate, interpolation, inRange)) {
			// Cannot interpolate, so choose lower bound.
			// This case shouldn't really happen since we matched the range.
			result.results.geoStatus = Geocoder::GeocodeExtrapolate;
			result.results.latitude = latStart;
			result.results.longitude = lonStart;
			return;
		}

		if (!inRange || interpolation < 0.0 || interpolation > 1.0) {
			result.results.geoStatus = Geocoder::GeocodeExtrapolate;
		} else {
			result.results.geoStatus = Geocoder::GeocodeAddress;
		}

		if (tempPoints1.size() < 2) {
			// Degenerate segment information.
			// Call it an exact match
			result.results.latitude = tempPoints1[0].latitude;
			result.results.longitude = tempPoints1[0].longitude;
			return;
		}

		// Assume that the low and high address range is at the end of the 
		// street segment, and interpolate.
		tempLengths.clear();
		tempLengths.push_back(0.0);

		double deltaLat, deltaLon;
		if (!inRange || interpolation < minInterpolation || interpolation > maxInterpolation) {
			// Limit extrapolation to the size of the segment.
			interpolation = JHMIN(maxInterpolation, JHMAX(minInterpolation, interpolation));
			// Extrapolate from straight line.
			result.results.latitude = latStart + (latEnd - latStart) * interpolation;
			result.results.longitude = lonStart + (lonEnd - lonStart) * interpolation;
			deltaLat = latEnd - latStart;
			deltaLon = lonEnd - lonStart;
		} else {
			// Interpolate within segment.
			result.results.geoStatus = Geocoder::GeocodeAddress;
			// Interpolate within segment.
			// Find the total length of the segment.
			double length = 0.0;
			double feetPerDegreeLong = FeetPerDegreeLongitude(tempPoints1[0].latitude);
			double prevLat = latStart;
			double prevLon = lonStart;
			{for (unsigned i = 1; i < tempPoints1.size(); i++) {
				// Calculate real distance along the segment, in feet
				double segmentDeltaY = (tempPoints1[i].latitude - prevLat) * FEET_PER_DEG_LATITUDE;;
				double segmentDeltaX = (tempPoints1[i].longitude - prevLon) * feetPerDegreeLong;
				prevLat = tempPoints1[i].latitude;
				prevLon = tempPoints1[i].longitude;
				length += sqrt(segmentDeltaY * segmentDeltaY + segmentDeltaX * segmentDeltaX);
				// Also record the incremental lengths as we go along.
				tempLengths.push_back(length);
			}}

			// This is the cumulative length along the line segment at which
			// the address is found.
			double interpLength = interpolation * length;

			// If the street endpoint offset is set, then adjust interpolation length.
			if (streetEndpointOffsetInFeet != 0 && streetEndpointOffsetInFeet < length * 0.5) {
				// Pull back the interpolation length from the endpoint.
				if (interpLength < streetEndpointOffsetInFeet) {
					interpLength = streetEndpointOffsetInFeet;
				} else if (interpLength > length - streetEndpointOffsetInFeet) {
					interpLength = length - streetEndpointOffsetInFeet;
				}
			}

			// Loop over the incremental lengths until we find the first
			// relative position that exceeds the interpolation point.
			size_t interpIdx = tempLengths.size() - 2;
			{for (unsigned i = 1; i < tempLengths.size(); i++) {
				if (tempLengths[i] >= interpLength) {
					interpIdx = i-1;
					break;
				}
			}}
			assert(interpIdx >= 0 && interpIdx <= tempLengths.size() - 2);

			// interpIdx is now the interpolation index, which is equal to
			// the previous point's index, and one less than the next point's index.
			// The address lies between the two points.

			// Find the fraction of the distance between the two points.
			double segmentDistance = interpLength - tempLengths[interpIdx];
			double segmentFraction = segmentDistance / (tempLengths[interpIdx+1] - tempLengths[interpIdx]);

			// Calculate the interpolated point along the segment.
			double lat1 = tempPoints1[interpIdx].latitude;
			double lon1 = tempPoints1[interpIdx].longitude;
			double lat2 = tempPoints1[interpIdx + 1].latitude;
			double lon2 = tempPoints1[interpIdx + 1].longitude;
			deltaLat = lat2 - lat1;
			deltaLon = lon2 - lon1;
			result.results.latitude = lat1 + deltaLat * segmentFraction;
			result.results.longitude = lon1 + deltaLon * segmentFraction;
		}

		// Offset the result by the offset, perpendicular to the vector between the two points.
		double segmentLength = sqrt(deltaLat * deltaLat + deltaLon * deltaLon);
		if (segmentLength > 0.000000000001) {
			double normalLat;
			double normalLon;
			if (result.streetSegment.isRightSide) {
				// Right-hand side  normal unit vector
				normalLat = -deltaLon / segmentLength;
				normalLon = deltaLat / segmentLength;
			} else {
				// Left-hand side normal unit vector
				normalLat = deltaLon / segmentLength;
				normalLon = -deltaLat / segmentLength;
			}
			// Must account for longitudinal distortion in offset calculation.
			result.results.latitude += normalLat * (streetOffsetInFeet / FEET_PER_DEG_LATITUDE);
			result.results.longitude += normalLon * streetOffsetInFeet / FeetPerDegreeLongitude(result.results.latitude);
		}
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
	bool GeocoderImp::StateAbbrToCode(
		const char* stateAbbr,
		const char* countryCode,
		int& stateCodeReturn
	) {
		return queryItf->StateAbbrToCode(stateAbbr, countryCode, stateCodeReturn);
	}

	///////////////////////////////////////////////////////////////////////
	// Convert a state FIPS code to a state abbeviation.
	// Inputs:
	//	int				stateCode			The state FIPS code
	//	const char*		countryCode			The country code (US or CA)
	// Outputs:
	//	 const char*&	stateAbbrReturn		The state abbreviation
	// Return value:
	//	bool			true if state exists, false o/w.
	///////////////////////////////////////////////////////////////////////
	bool GeocoderImp::StateCodeToAbbr(
		int stateCode,
		const char* countryCode,
		const char*& stateAbbrReturn
	) {
		return queryItf->StateCodeToAbbr(stateCode, countryCode, stateAbbrReturn);
	}

	///////////////////////////////////////////////////////////////////////
	// Get the segment points for a street range.
	// Inputs:
	//	const StreetSegment& 
	//							streetSegment		The street range to query
	// Outputs:
	//	std::vector<Geocoder::CoordinatePoint>& 
	//							points			The points that were retrieved.
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::GetSegmentPointsForStreetSegment(
		const StreetSegment& streetSegment,
		std::vector<CoordinatePoint>& points
	) {
		points.clear();
		QueryImp::CoordinatePointsFromStreetSegmentIterator iter = 
			queryItf->LookupCoordinatePointsFromStreetSegment(streetSegment);
		CoordinatePoint point;
		CoordinatePoint lastPoint(1000.0, 0.0);
		while (iter.Next(point)) {
			// Avoid duplicate points.  They mess up interpolation.
			if (point != lastPoint) {
				points.push_back(point);
				lastPoint = point;
			}
		}
	}

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
	bool GeocoderImp::FindCityStatePostcodeFromCityPostcode(
		const char* city, 
		const char* postcode, 
		CityStatePostcode& cityStatePostcodeReturn
	) {
		// Loop over all zip aliases
		QueryImp::PostcodeAliasIterator postcodeAliasIter = queryItf->LookupPostcodeAliases(postcode);
		PostcodeAlias postcodeAlias;
		while (postcodeAliasIter.Next(postcodeAlias)) {
			QueryImp::CityStatePostcodeFromPostcodeIterator iter = 
				queryItf->LookupCityStatePostcodeFromPostcode(postcodeAlias.postcode);
			while (iter.Next(cityStatePostcodeReturn)) {
				if (strcmp(cityStatePostcodeReturn.city, city) == 0) {
					return true;
				}
				const char* newCity;
				if (
					FindReplacementCity(cityStatePostcodeReturn.postcode, cityStatePostcodeReturn.stateAbbr, city, newCity) &&
					strcmp(cityStatePostcodeReturn.city, newCity) == 0
				) {
					return true;
				}
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////
	// Does the result set contain multiples?
	///////////////////////////////////////////////////////////////////////
	bool GeocoderImp::ResultsAreMultiple()
	{
		if (sortedGeocodeResults.size() < 2)
			return false;

		// Check distance between first two results.
		if ((sortedGeocodeResults[0]->results.matchScore==1000 && sortedGeocodeResults[1]->results.matchScore!=1000) ||
			sortedGeocodeResults[0]->results.matchScore - sortedGeocodeResults[1]->results.matchScore >= multipleMatchThreshold
		) {
			return false;
		}

		// if all the candidates within the match threshold have the same lat/long, return the top one
		for (unsigned i = 1; i < sortedGeocodeResults.size(); i++)
		{
			GeocodeResultsPlus* result1 = sortedGeocodeResults[i];
			if ((sortedGeocodeResults[0]->results.matchScore-result1->results.matchScore) > multipleMatchThreshold)
				return false;
			GeocodeResultsPlus* result0 = sortedGeocodeResults[i-1];

			if (result0->results.latitude!=result1->results.latitude || result0->results.longitude!=result1->results.longitude)
				break;
		}
		// Given that first two results are too close, check that all results after
		// the first are either city/street aliases or far enough apart
		for (
			unsigned i = 1; 
			i < sortedGeocodeResults.size() && sortedGeocodeResults[0]->results.matchScore - sortedGeocodeResults[i]->results.matchScore < multipleMatchThreshold;
			i++
		)  {
			if (!ResultsAreAliased(*sortedGeocodeResults[0], *sortedGeocodeResults[i])) {
				return true;
			}
		}
		return false;
	}


	///////////////////////////////////////////////////////////////////////
	// Get all related finance areas, accounting for postal code's aliases.
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::GetAllFinanceAreasForCity(
		const char* city,
		int stateCode,
		const char* excludeFA,
		std::vector<const char*>& faListReturn
	) {
		faListReturn.clear();
		CityStatePostcode cityStatePostcode;

		// Get the city soundex
		char tmp[100], soundex[100];
		Soundex2(city, tmp, soundex);
		// Query the database
		QueryImp::CityStatePostcodeFromStateCityIterator iter = 
			queryItf->LookupCityStatePostcodeFromStateCity(stateCode, soundex);
		// Get FAs for city.
		while (iter.Next(cityStatePostcode)) {
			if (strcmp(excludeFA, cityStatePostcode.financeNumber) != 0) {
				faListReturn.push_back(
					(const char*)bulkAllocator->NewString(cityStatePostcode.financeNumber)
				);
			}
		}
		// Unique the Fa list
		CmpStr cmp;
		std::sort(faListReturn.begin(), faListReturn.end(), cmp);
		EqualStr cmp2;
		std::vector<const char*>::iterator uniqueFAIter = std::unique(faListReturn.begin(), faListReturn.end(), cmp2);
		faListReturn.resize(uniqueFAIter - faListReturn.begin());
	}


	///////////////////////////////////////////////////////////////////////
	// Get all related finance areas, accounting for postal code's aliases.
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::GetRelatedFinanceAreas(
		const char* postcode,
		const char* finance,
		std::vector<const char*>& faListReturn
	) {
		faListReturn.clear();

		// Place this finance area on this list.
		faListReturn.push_back((const char*)bulkAllocator->NewString(finance));

		// Get all alias postal codes.
		QueryImp::PostcodeAliasIterator postcodeAliasIter = queryItf->LookupPostcodeAliases(postcode);
		PostcodeAlias postcodeAlias;
		while (postcodeAliasIter.Next(postcodeAlias)) {
			if (strcmp(postcode, postcodeAlias.postcode) == 0) {
				// We already processed this postal code.
				continue;
			}
			QueryImp::CityStatePostcodeFromPostcodeIterator iter2 = 
				queryItf->LookupCityStatePostcodeFromPostcode(postcodeAlias.postcodeGroup);
			CityStatePostcode cityStatePostcode;
			if (iter2.Next(cityStatePostcode)) {
				faListReturn.push_back(
					(const char*)bulkAllocator->NewString(cityStatePostcode.financeNumber)
				);
			}
		}
		// Unique the Fa list
		CmpStr cmp;
		std::sort(faListReturn.begin(), faListReturn.end(), cmp);
		EqualStr cmp2;
		std::vector<const char*>::iterator uniqueFAIter = std::unique(faListReturn.begin(), faListReturn.end(), cmp2);
		faListReturn.resize(uniqueFAIter - faListReturn.begin());
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
	bool GeocoderImp::FindReplacementCity(
		const char* postCode,
		const char* stateAbbr,
		const char* oldCity,
		const char*& newCityReturn
	) {
		char buf[100];
		// Construct the lookup key.
		size_t len = strlen(stateAbbr);
		strcpy(buf, stateAbbr);
		buf[len] = '|';
		strcpy(buf + len + 1, oldCity);
		strcat(buf,"|");
		strcat(buf, postCode);
		return (cityReplacementTable->Find(buf, newCityReturn));
	}

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
	double GeocoderImp::CompareStreetNames(
		const char* candidateStreet,
		const char* referenceStreet
	) {
		// Look for numerics in street name
		double streetScore = 0.0;
		bool haveCandidateNumeric = false;
		const char* candidateNumericPtr;
		{for (const char* ptr = candidateStreet; *ptr != 0; ptr++) {
			if (isdigit(*ptr)) {
				haveCandidateNumeric = true;
				candidateNumericPtr = ptr;
				break;
			}
		}}

		if (haveCandidateNumeric) {
			// Compare numeric part with higher weight
			bool haveRefNumeric = false;
			const char* refNumericPtr;
			{for (const char* ptr = referenceStreet; *ptr != 0; ptr++) {
				if (isdigit(*ptr)) {
					haveRefNumeric = true;
					refNumericPtr = ptr;
					break;
				}
			}}
			if (haveRefNumeric) {
				// Numbers should match exactly.
				streetScore = 0.8;
				const char* canPtr = candidateNumericPtr;
				const char* refPtr = refNumericPtr;
				while (isdigit(*canPtr) || isdigit(*refPtr)) {
					if (*canPtr != *refPtr) {
						streetScore = 0.0;
						break;
					}
					canPtr++;
					refPtr++;
				}
				// Edit distance calculation on non-numeric portions.
				bool haveFirstPortion = 
					candidateNumericPtr != candidateStreet ||
					refNumericPtr != referenceStreet;
				bool haveLastPortion = *canPtr != 0 || *refPtr != 0;
				if (haveFirstPortion) {
					char temp1[100], temp2[100];
					int nlen1 = int(candidateNumericPtr - candidateStreet);
					memcpy(temp1, candidateStreet, nlen1);
					temp1[nlen1] = 0;
					while(nlen1>0 && temp1[nlen1-1]==' ')
						temp1[--nlen1] = 0;

					int nlen2 = int(refNumericPtr - referenceStreet);
					memcpy(temp2, referenceStreet, nlen2);
					temp2[nlen2] = 0;
					while(nlen2>0 && temp2[nlen2-1]==' ')
						temp2[--nlen2] = 0;
					float tmp = LocalCompareString(temp1, temp2);
					if (tmp<.8 && strstr(temp2, temp1)!=NULL)
						tmp = .8f;
					streetScore += haveLastPortion ? 0.1 * tmp : 0.2 * tmp;
				}
				if (haveLastPortion) 
				{
					const char *pTemp1 = canPtr;
					while(*pTemp1==' ')
						pTemp1++;
					const char *pTemp2 = refPtr;
					while(*pTemp2==' ')
						pTemp2++;

					float tmp = LocalCompareString(pTemp1, pTemp2);
					streetScore += haveFirstPortion ? 0.1 * tmp : 0.2 * tmp;
				}
			} else {
				// Only one of them has a numeric.  Use edit distance but penalize.
				streetScore = 0.5 * LocalCompareString(candidateStreet, referenceStreet);
			}
		} else {
			// Regular comparison
			streetScore = LocalCompareString(candidateStreet, referenceStreet);
		}
		return streetScore;
	}

	#ifndef NDEBUG
		void GeocoderImp::GEOTRACE(const TsString& x) {
			geocoder.TraceMessage(x.c_str());
		}
	#endif

	///////////////////////////////////////////////////////////////////////
	// Given the set of last-line parse candidates, choose the best last line.
	// 
	// Inputs: none
	// Inputs that are members of the class:
	//	lastLineParseCandidates
	// Outputs:
	//	CityStatePostcode&	bestCityStatePostcode		Chosen CityStatePostcode
	//	int					bestLastLineScore			Score achieved when choosing last-line
	//	int					bestLastLineCandidateIdx	Index of best parse candidate
	//	int					bestLastLineFlags			Flags indicating parse modifications or
	//													special cases in matching.
	// Return value:
	//	bool			true if match found, false if no match found.
	///////////////////////////////////////////////////////////////////////
	bool GeocoderImp::ChooseBestLastLine(
		CityStatePostcode& bestCityStatePostcode,
		int& bestLastLineScore,
		int& bestLastLineCandidateIdx,
		int& bestLastLineFlags
	) {
		bestLastLineScore = -1;
		bestLastLineCandidateIdx = -1;
		bestLastLineFlags = 0;

		// Loop over last-line parse candidates, query the City/State/Zip database, and
		// keep the best-matching candidate and matching CityStatePostcode record.
		{for (unsigned i = 0; i < lastLineParseCandidates.size(); i++) {
			CityStatePostcode cityStatePostcode;
			AddressParserLastLine::ParseCandidate& candidate = lastLineParseCandidates[i];
			GEOTRACE(TsString("Lookup last-line candidate: (") + candidate.city + ") (" + candidate.state + ") (" + candidate.postcode + ")");
			bool searchByCity = true;
			if (candidate.postcode[0] != 0) {
				GEOTRACE("\tSearching by postal code");
				// Retrieve entries by postal code
				const char* postcode = candidate.postcode;
				// Loop over all zip aliases
				QueryImp::PostcodeAliasIterator postcodeAliasIter = 
					queryItf->LookupPostcodeAliases(postcode);
				PostcodeAlias postcodeAlias;
				while (postcodeAliasIter.Next(postcodeAlias)) {
					GEOTRACE(TsString("\tFound ZipAlias: (") + postcodeAlias.postcode + ")");
					QueryImp::CityStatePostcodeFromPostcodeIterator iter = 
						queryItf->LookupCityStatePostcodeFromPostcode(postcodeAlias.postcode);
					// Score retrieved last lines against the parse candidate.
					while (iter.Next(cityStatePostcode)) {
						GEOTRACE(TsString("\t\tRetrieved CityStatePostcode: (") + cityStatePostcode.city + ") (" + FormatInteger(cityStatePostcode.state) + ") (" + cityStatePostcode.postcode + ")");
						int lastLineFlags = 0;
						int score = ScoreLastLine(candidate, cityStatePostcode, lastLineFlags);
						GEOTRACE("\t\t\tscore = " + FormatInteger(score));
						if (score >= m_LastLineThresholdZipOnly) {
							// This candidate is good enough that we need
							// not search by city.
							searchByCity = false;
						}
						if (
							score > bestLastLineScore &&
							ScoreCombined(1000, score) >= matchThreshold
						) {
							GEOTRACE("\t\t\tKeep as best last-line");
							bestLastLineScore = score;
							bestLastLineCandidateIdx = i;
							bestLastLineFlags = lastLineFlags;
							bestCityStatePostcode = cityStatePostcode;
						}
					}
				}
			}

			if (searchByCity) {
				GEOTRACE("\tSearching by city");
				// Searching by ZIP code failed, so search by city soundex.
				// Get the state code
				int stateCode;
				if (
					!StateAbbrToCode(
						candidate.state, 
						queryItf->CountryFromPostalCode(candidate.postcode), 
						stateCode
					)
				) {
					GEOTRACE("\tInvalid state");
					// Invalid state.
					continue;
					// TODO: get state from ZIP
				}
				// Loop performs two checks; first for city and second for city replacement
				// due to merger of cities.
				for (int cityLoop = 0; cityLoop < 2; cityLoop++) {
					const char* city;
					if (cityLoop == 0) {
						city = candidate.city;
					} else {
						if (!FindReplacementCity(candidate.postcode, candidate.state, candidate.city, city)) {
							break;
						}
						GEOTRACE(TsString("\tSearching on replacement city (") + city + ")");
					}
					// Get the city soundex
					char tmp[100], soundex[100];
					Soundex2(city, tmp, soundex);
					// Query the database
					QueryImp::CityStatePostcodeFromStateCityIterator iter = 
						queryItf->LookupCityStatePostcodeFromStateCity(stateCode, soundex);
					// Score retrieved last lines against the parse candidate.
					while (bestLastLineScore != 1000 && iter.Next(cityStatePostcode)) {
						GEOTRACE(TsString("\tRetrieved CityStatePostcode: (") + cityStatePostcode.city + ") (" + FormatInteger(cityStatePostcode.state) + ") (" + cityStatePostcode.postcode + ")");
						int lastLineFlags = 0;
						int score = ScoreLastLine(candidate, cityStatePostcode, lastLineFlags);
						GEOTRACE("\t\tscore = " + FormatInteger(score));
						if (
							score > bestLastLineScore &&
							ScoreCombined(1000, score) >= matchThreshold
						) {
							GEOTRACE("\t\tKeep as best last-line");
							if (strcmp(candidate.postcode, cityStatePostcode.postcode) != 0) {
								score -= m_ReplaceLastLinePostcodeWeight;
							}
							//lastLineFlags |= Geocoder::MatchChangedPostcode;
							bestLastLineScore = score;
							bestLastLineCandidateIdx = i;
							bestLastLineFlags = lastLineFlags;
							if (cityLoop == 1) {
								// Remove ChangedCityFlag because we matched on an alias.
								bestLastLineFlags |= Geocoder::MatchCityAliasUsed;
							}
							bestCityStatePostcode = cityStatePostcode;
						}
					}
				}
			}
		}}

		return bestLastLineCandidateIdx >= 0;
	}


	///////////////////////////////////////////////////////////////////////
	// Given the best last line, and the set of first-line parse
	// candidates, find the results set.
	// 
	// Inputs: none
	//	CityStatePostcode&	bestCityStatePostcode		The best last-line reference record
	//	int					bestLastLineScore			Score achieved when choosing last-line
	//	int					bestLastLineCandidateIdx	Index of best parse candidate
	//	int					bestLastLineFlags			Flags indicating parse modifications or
	// Inputs that are members of the class:
	//	firstLineParseCandidates
	//	uniqueFAList
	// Outputs: none
	// Outputs that are members of the class:
	//	geocodeResults
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::ChooseBestFirstLineResults(
		const CityStatePostcode& bestCityStatePostcode,
		int bestLastLineScore,
		int bestLastLineCandidateIdx,
		int bestLastLineFlags
	) {
		// Presence of postal code in last-line affects scoring.
		bool haveLastLinePostcode = (lastLineParseCandidates[bestLastLineCandidateIdx].postcode[0] != 0);

		// Search for first-line street range records 
		// for each first-line parse candidate
		{for (unsigned i = 0; i < firstLineParseCandidates.size(); i++) {
			StreetIntersection streetIntersection;
			StreetName streetName;
			AddressParserFirstLine::ParseCandidate& candidate = firstLineParseCandidates[i];
			GEOTRACE(TsString("Lookup first-line candidate: (") + candidate.number + ") (" + candidate.predir + ") (" + candidate.street + ") (" + candidate.suffix +") (" + candidate.postdir +") (" + candidate.unitDesignator +") (" + candidate.unitNumber + ") (" + candidate.street2 + ")");
			if (candidate.isIntersection) {
				// If an interection, search the intersection soundex
				// within the state of the last line.
				char tmp[100], soundex1[100], soundex2[100];
				Soundex3(candidate.street, tmp, soundex1);
				Soundex3(candidate.street2, tmp, soundex2);
				for (int loop = 0; loop < 2; loop++) {
					QueryImp::StreetIntersectionIterator iter;
					// Query the database
					if (loop == 0) {
						GEOTRACE(TsString("\tIntersection search on (") + FormatInteger(bestCityStatePostcode.state) + ") (" + soundex1 + ") (" + soundex2 + ")");
						iter = queryItf->LookupStreetIntersection(bestCityStatePostcode.state, soundex1, soundex2);
					} else {
						// Swap first/second soundex on second pass.
						GEOTRACE(TsString("\tIntersection search on (") + FormatInteger(bestCityStatePostcode.state) + ") (" + soundex2 + ") (" + soundex1 + ")");
						iter = queryItf->LookupStreetIntersection(bestCityStatePostcode.state, soundex2, soundex1);
					}

					// Score retrieved intersections against the parse candidate.
					
					// In the process of finding the intersection, we may choose the CityStatePostcode that
					// actually contains the intersection.
					CityStatePostcode newCityStatePostcode;
					while (iter.Next(streetIntersection)) {
						GEOTRACE(TsString("\tRetrieved intersection (") + FormatInteger(streetIntersection.cityStatePostcode1.state) + ") (" + streetIntersection.cityStatePostcode1.city + ") (" + streetIntersection.streetName1.street + ") (" + streetIntersection.streetSegment1.addrLow + "-" + streetIntersection.streetSegment1.addrHigh + ") (" + streetIntersection.streetName2.street + ") (" + streetIntersection.streetSegment2.addrLow + "-" + streetIntersection.streetSegment2.addrHigh + ")");
						int firstLineFlags = 0;
						int streetScore = ScoreStreetIntersection(
							candidate, 
							streetIntersection, 
							bestCityStatePostcode,
							firstLineFlags,
							newCityStatePostcode
						);
						GEOTRACE("\t\t score = " + FormatInteger(streetScore));

						int newLastLineFlags = bestLastLineFlags;
						UpdateLastLineMatchStatusFlags(
							lastLineParseCandidates[0],
							lastLineParseCandidates[bestLastLineCandidateIdx],
							&newCityStatePostcode,
							bestLastLineCandidateIdx,
							newLastLineFlags);

						int newLastLineScore = bestLastLineScore;
						if (bestCityStatePostcode.ID != newCityStatePostcode.ID) {
							GEOTRACE(TsString("\tChange CityStatePostcode to (") + newCityStatePostcode.city + ") (" +  FormatInteger(newCityStatePostcode.state) + ") (" + newCityStatePostcode.postcode + ")");
							// CityStatePostcode was changed to the owner of the matched street
							PenalizeLastLineChange(
								bestCityStatePostcode,
								newCityStatePostcode,
								haveLastLinePostcode,
								newLastLineFlags,
								newLastLineScore
							);

							GEOTRACE("\t\tnew last-line score = " +  FormatInteger(newLastLineScore));
						}
						int scoreCombined = ScoreCombined(streetScore, newLastLineScore);
						if (scoreCombined > matchThreshold) {
							CatchRemainingFirstLineMatchStatusFlags(
								firstLineParseCandidates[0],
								firstLineParseCandidates[i],
								i,
								scoreCombined,
								firstLineFlags);
							GEOTRACE("\t\tKeeping street intersection result");
							// Generate a result and add to the list.
							GeocodeResultsPlus& result = geocodeResults.UseExtraOnEnd();
							MakeResult(
								newCityStatePostcode,
								candidate,
								streetIntersection,
								newLastLineFlags | firstLineFlags,
								scoreCombined,
								result
							);
						}
					}
				}
			} else {
				// If not an intersection, search the street name soundex
				// within the finance area of the last-line.
				char tmp[100], soundex[100];
				Soundex3(candidate.street, tmp, soundex);

				// Address template for candidate used for comparison with address ranges.
				AddressTemplate addrTemplate(candidate.number);

				// Loop over all Finance areas.  
				for (unsigned uniqueFAIdx = 0; uniqueFAIdx < uniqueFAList.size(); uniqueFAIdx++) {
					// Search street in unique finance areas.
					const char* financeArea = uniqueFAList[uniqueFAIdx];
					// Query the database
					GEOTRACE(TsString("\tStreet name search on FINANCE=(") + financeArea + "), SOUNDEX=(" + soundex + ")");
					QueryImp::StreetNameFromFaStreetIterator iter = 
						queryItf->LookupStreetNameFromFaStreet(financeArea, soundex);

					// Score retrieved street names against the parse candidate.

					// In the process of scoring the street segment, it will 
					// locate the containing CityStatePostcode record.
					CityStatePostcode newCityStatePostcode;

					while (iter.Next(streetName)) {
						GEOTRACE("\tRetrieved StreetName[" + FormatInteger(streetName.ID) + "]:  (" + streetName.predir + ") (" + streetName.street + ") (" + streetName.suffix +") (" + streetName.postdir + ")");
						int firstLineFlags = 0;
						int streetNameScore = ScoreStreetName(
							candidate, 
							streetName, 
							bestCityStatePostcode,
							firstLineFlags,
							newCityStatePostcode
						);
						GEOTRACE("\t\tscore = " +  FormatInteger(streetNameScore));

						int newLastLineFlags = bestLastLineFlags;
						UpdateLastLineMatchStatusFlags(
							lastLineParseCandidates[0],
							lastLineParseCandidates[bestLastLineCandidateIdx],
							&newCityStatePostcode,
							bestLastLineCandidateIdx,
							newLastLineFlags);

						int newLastLineScore = bestLastLineScore;
						if (bestCityStatePostcode.ID != newCityStatePostcode.ID) {
							GEOTRACE(TsString("\tChange CityStatePostcode to (") + newCityStatePostcode.city + ") (" +  FormatInteger(newCityStatePostcode.state) + ") (" + newCityStatePostcode.postcode + ")");
							// CityStatePostcode was changed to the owner of the matched street
							PenalizeLastLineChange(
								bestCityStatePostcode,
								newCityStatePostcode,
								haveLastLinePostcode,
								newLastLineFlags,
								newLastLineScore
							);
							GEOTRACE("\t\tnew last-line score = " +  FormatInteger(newLastLineScore));
						} else {
							GEOTRACE(TsString("\tKeep original CityStatePostcode"));
						}

						// Make sure that the possible score is good enough if we get
						// an exact range match.
						if (ScoreCombined(streetNameScore, 1000, newLastLineScore) > matchThreshold) {
							// Query StreetSegment records and pick the best one.
							GEOTRACE("\t\tQuerying StreetSegment");
							QueryImp::StreetSegmentFromStreetNameIterator streetSegmentIter = 
								queryItf->LookupStreetSegmentFromStreetName(streetName);

							// TODO: Use address-number hint in StreetSegment query
							StreetSegment bestStreetSegment;
							int bestStreetSegmentScore = -1;	// 0 - 1000
							int bestStreetSegmentFlags = 0;
							double bestTieBreaker;
							StreetSegment streetSegment;
							while (streetSegmentIter.Next(streetSegment)) {
								if (streetSegment.addrLow[0] == 0) {
									// ignore empty addresses
									GEOTRACE("\t\tIgnored empty range");
									continue;
								}
								int streetSegmentFlags = 0;
								double tieBreaker;
								int streetSegmentScore = ScoreStreetSegment(
									lastLineParseCandidates[bestLastLineCandidateIdx].postcodeExt,
									addrTemplate,
									streetSegment, 
									streetSegmentFlags, 
									tieBreaker
								);
								if (
									streetSegmentScore > bestStreetSegmentScore ||
									streetSegmentScore >= bestStreetSegmentScore && tieBreaker > bestTieBreaker 
								) {
									GEOTRACE("\t\t*Found StreetSegment[" + FormatInteger(streetSegment.ID) + "] from (" + streetSegment.addrLow + ") to (" + streetSegment.addrHigh + "), score = " + FormatInteger(streetSegmentScore));
									bestStreetSegmentScore = streetSegmentScore;
									bestStreetSegment = streetSegment;
									bestStreetSegmentFlags = streetSegmentFlags;
									bestTieBreaker = tieBreaker;
								} else {
									GEOTRACE("\t\tFound StreetSegment[" + FormatInteger(streetSegment.ID) + "] from (" + streetSegment.addrLow + ") to (" + streetSegment.addrHigh + "), score = " + FormatInteger(streetSegmentScore));
								}
							}

							if (bestStreetSegmentScore >= 0) {
								// We had at least one StreetSegment.
								// Determine the overall score.
								int combinedScore = ScoreCombined(
									streetNameScore,
									bestStreetSegmentScore,
									newLastLineScore
								);

								if (combinedScore >= matchThreshold) {
									CatchRemainingFirstLineMatchStatusFlags(
										firstLineParseCandidates[0],
										firstLineParseCandidates[i],
										i,
										combinedScore,
										firstLineFlags);
									GEOTRACE(TsString("\t\tMaking result for StreetSegment[" + FormatInteger(bestStreetSegment.ID) + "] from : (") + bestStreetSegment.addrLow + ") (" + bestStreetSegment.addrHigh + "), Overall score = " + FormatInteger(combinedScore));
									// Generate a result and add to the list.
									GeocodeResultsPlus& result = geocodeResults.UseExtraOnEnd();
									MakeResult(
										newCityStatePostcode, 
										candidate,
										streetName,
										bestStreetSegment,
										newLastLineFlags | firstLineFlags | bestStreetSegmentFlags,
										combinedScore,
										result
									);
								}
							}
						}
					}
				}	// Fa zip alias loop
			}	// else isIntersection
		}}
	}

	///////////////////////////////////////////////////////////////////////
	// When a last-line is changed to the owning CSP record of the matched
	// street name from the "best match" last-line, a penalty is exacted
	// depending on whether the original last-line had a postal code or not.
	//
	// Removed the setting of MatchStatus flags from this method, as the 
	// comparison is made between a possible newly selected city-state-postcode
	// and the "best" city-state-postcode, which itself may be different
	// from the original data. The user will expect "changes" to have been
	// made against their data, not data from an intermediate step.
	// The corresponding changes will be caught by ChooseBestFirstLineResults,
	// which calls this method, and which makes the comparison against the
	// original data.
	//
	// Inputs:
	//	CityStatePostcode&		bestCityStatePostcode		The "best" last-line record
	//	CityStatePostcode&		newCityStatePostcode		The new last-line record
	//	bool					haveLastLinePostcode		true if the parse candidate has a postal code
	//	int&					lastLineScore				The last-line score before penalty
	// Outputs:
	//	int&					lastLineScore				The last-line score after penalty
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::PenalizeLastLineChange(
		const CityStatePostcode& bestCityStatePostcode,
		const CityStatePostcode& newCityStatePostcode,
		bool haveLastLinePostcode,
		int currLastLineFlags,
		int& lastLineScore
	) {
		bool changedCity = (strcmp(bestCityStatePostcode.city, newCityStatePostcode.city) != 0);
		if (haveLastLinePostcode) {
			// If postcode was given, then changing city name is not so bad
			if (changedCity) {
				lastLineScore -= m_ReplaceLastLineCityWeight;
			}
			if (strcmp(bestCityStatePostcode.postcode, newCityStatePostcode.postcode) != 0) {
				lastLineScore -= m_ReplaceLastLinePostcodeWeight;
			}
			if (strcmp(bestCityStatePostcode.financeNumber, newCityStatePostcode.financeNumber) != 0) {
				// Not even the right finance area.  Penalize this further.
				lastLineScore -= (changedCity ? // unless a legitimate post office recognized city alias used, then not as bad...
					(currLastLineFlags&Geocoder::MatchCityAliasUsed ? m_ReplaceLastLineFinanceWeightNewCity>>1 : m_ReplaceLastLineFinanceWeightNewCity)
					: m_ReplaceLastLineFinanceWeightSameCity);
			}
		} else {
			// If no postcode was give, then changing city name is higher penalty
			if (changedCity) {
				lastLineScore -= m_ReplaceLastLineCityNoPostcodeWeight;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Inputs:
	//	const CityStatePostcode&	cityStatePostcode		The current last-line record
	//	const char*					city					City to test as alias
	// Return value:
	//	bool		true if city is an alias
	///////////////////////////////////////////////////////////////////////
	bool GeocoderImp::CityIsAlias(
		const CityStatePostcode& cityStatePostcode, 
		const char* city
	) {
		const char* newCity;
		return 
			(
				FindReplacementCity(cityStatePostcode.postcode, cityStatePostcode.stateAbbr, cityStatePostcode.city, newCity) &&
				strcmp(city, newCity) == 0
			) ||
			(
				FindReplacementCity(cityStatePostcode.postcode, cityStatePostcode.stateAbbr, city, newCity) &&
				strcmp(cityStatePostcode.city, newCity) == 0
			);
	}

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
	inline void GeocoderImp::UpdateLastLineMatchStatusFlags(
		const AddressParserLastLine::ParseCandidate & baseline,
		const AddressParserLastLine::ParseCandidate & best,
		const CityStatePostcode * newCityStatePostcode,
		int bestLastLineIndex,
		int & lastLineFlags)
	{
		if (newCityStatePostcode != NULL)
		{
			if (baseline.city[0] != 0 && strcmp(newCityStatePostcode->city, baseline.city) != 0)
			{
				lastLineFlags |= Geocoder::MatchChangedCity;
				if (CityIsAlias(*newCityStatePostcode, baseline.city))
					lastLineFlags |= Geocoder::MatchCityAliasUsed;
			}
			if (baseline.state[0] != 0 && strcmp(newCityStatePostcode->stateAbbr, baseline.state) != 0)
				lastLineFlags |= Geocoder::MatchChangedState;
			if (baseline.postcode[0] != 0 && strcmp(newCityStatePostcode->postcode, baseline.postcode) != 0)
				lastLineFlags |= Geocoder::MatchChangedPostcode;
		}
		else if (bestLastLineIndex > 0) // The "current" isn't the first;
		{
			if (baseline.city[0] != 0 && strcmp(best.city, baseline.city) != 0)
				lastLineFlags |= Geocoder::MatchChangedCity;
			if (baseline.state[0] != 0 && strcmp(best.state, baseline.state) != 0)
				lastLineFlags |= Geocoder::MatchChangedState;
			if (baseline.postcode[0] != 0 && strcmp(best.postcode, baseline.postcode) != 0)
				lastLineFlags |= Geocoder::MatchChangedPostcode;
		}
	}

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
	inline void GeocoderImp::CatchRemainingFirstLineMatchStatusFlags(
		const AddressParserFirstLine::ParseCandidate & baseline,
		const AddressParserFirstLine::ParseCandidate & candidate,
		int candidateIndex,
		int currentScore,
		int & firstLineFlags)
	{
		if (candidateIndex > 0) // First candidate should be "unmodified";
		{
			// Quickly check this candidate for possible street name changes;
			if (strcmp(baseline.street,candidate.street) != 0)
				firstLineFlags |= Geocoder::MatchChangedStName;								
			// Use candidate's permutations to set any flags that may have
			// not been set during the various scoring operations. For right now,
			// only worry about this IF the score isn't "perfect". (I'm open to doing
			// this even if a pefect match, if it seems necessary.)
			if (currentScore < 1000)
			{
				if (candidate.permutations&AddressParserFirstLine::PermuteSplitAddrNbrToUnit)
					firstLineFlags |= Geocoder::MatchMovedAddrLetterToUnit;
				if (candidate.permutations&AddressParserFirstLine::PermuteStreetNumberToUnit)
					firstLineFlags |= Geocoder::MatchChangedUnitDes;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Check that the data and geocoder are the same version
	// Inputs:
	//  TsString		dataDir	Directory containing geocoder database files
	// Outputs:
	// Return value:
	//	bool			true if versions match, false o/w.
	///////////////////////////////////////////////////////////////////////
	bool GeocoderImp::CheckDataVersion(TsString dataDir)
	{
		// Open the file.
		TsString filename = dataDir + "/" + versionFilename;
		FILE* fp = fopen(filename.c_str(), "r");
		if (fp == 0) {
			return false;
		}
		
		// Read the file version
		char buf[48];
		bool retVal = false;
		if( fgets(buf, sizeof(buf), fp) != NULL ) {
			if (GEODATA_VERSION == atoi(buf)) {
				retVal = true;
			}
		}
		
		fclose(fp);
		return retVal;		
	}

	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	// Subclass of ReferenceQueryInterface to forward error messages.
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	void GeocoderImp::QueryImpErrorMsg::ErrorMessage(
		const TsString& msg
	) {
		geocoder.ErrorMessage(msg.c_str());
	}


	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	// Geocoder results structure
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	Geocoder::GeocodeResults::GeocodeResults()
	{
		resultsInternal = new GeocodeResultsInternal;
	}


	Geocoder::GeocodeResults::~GeocodeResults()
	{
		delete resultsInternal;
		resultsInternal = 0;
	}

	Geocoder::GeocodeResults::GeocodeResults(const GeocodeResults& rhs)
	{
		resultsInternal = new GeocodeResultsInternal;
		*resultsInternal = *rhs.resultsInternal;
	}

	Geocoder::GeocodeResults& Geocoder::GeocodeResults::operator=(const GeocodeResults& rhs)
	{
		if (this != &rhs) {
			*resultsInternal = *rhs.resultsInternal;
		}
		return *this;
	}

	void Geocoder::GeocodeResults::Clear()
	{
		memset(resultsInternal, 0, sizeof(*resultsInternal));
	}


	// Accessors
	const char* Geocoder::GeocodeResults::GetAddrNbr() { return resultsInternal->addrNbr; }
	const char* Geocoder::GeocodeResults::GetPrefix() { return resultsInternal->prefix; }
	const char* Geocoder::GeocodeResults::GetPredir() { return resultsInternal->predir; }
	const char* Geocoder::GeocodeResults::GetStreet() { return resultsInternal->street; }
	const char* Geocoder::GeocodeResults::GetSuffix() { return resultsInternal->streetSuffix; }
	const char* Geocoder::GeocodeResults::GetPostdir() { return resultsInternal->postdir; }
	const char* Geocoder::GeocodeResults::GetUnitDes() { return resultsInternal->unitDes; }
	const char* Geocoder::GeocodeResults::GetUnit() { return resultsInternal->unit; }
	const char* Geocoder::GeocodeResults::GetCity() { return resultsInternal->city; }
	int Geocoder::GeocodeResults::GetState() { return resultsInternal->state; }
	const char* Geocoder::GeocodeResults::GetStateAbbr() { return resultsInternal->stateAbbr; }
	const char* Geocoder::GeocodeResults::GetCountryCode() { return resultsInternal->countryCode; }
	int Geocoder::GeocodeResults::GetCountyCode() { return resultsInternal->countyCode; }
	const char* Geocoder::GeocodeResults::GetCensusTract() { return resultsInternal->censusTract; }
	const char* Geocoder::GeocodeResults::GetCensusBlock() { return resultsInternal->censusBlock; }
	const char* Geocoder::GeocodeResults::GetPostcode() { return resultsInternal->postcode; }
	const char* Geocoder::GeocodeResults::GetPostcodeExt() { return resultsInternal->postcodeExt; }
	double Geocoder::GeocodeResults::GetLatitude() { return resultsInternal->latitude; }
	double Geocoder::GeocodeResults::GetLongitude() { return resultsInternal->longitude; }
	int Geocoder::GeocodeResults::GetMatchScore() { return resultsInternal->matchScore; }
	int Geocoder::GeocodeResults::GetMatchStatus() { return resultsInternal->matchStatus; }
	int Geocoder::GeocodeResults::GetGeoStatus() { return resultsInternal->geoStatus; }
	const char* Geocoder::GeocodeResults::GetPrefix2() { return resultsInternal->prefix2; }
	const char* Geocoder::GeocodeResults::GetPredir2() { return resultsInternal->predir2; }
	const char* Geocoder::GeocodeResults::GetStreet2() { return resultsInternal->street2; }
	const char* Geocoder::GeocodeResults::GetSuffix2() { return resultsInternal->streetSuffix2; }
	const char* Geocoder::GeocodeResults::GetPostdir2() { return resultsInternal->postdir2; }
}


