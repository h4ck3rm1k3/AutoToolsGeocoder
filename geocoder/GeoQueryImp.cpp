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
# $Rev: 112 $ 
# $Date: 2009-03-11 19:54:59 +0100 (Wed, 11 Mar 2009) $ 
*/

// RefQuery.cpp: The standard file-based reference query object

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "../geocommon/Geocoder_Headers.h"

#include <fstream>
#include "GeoQueryImp.h"
#include "../geocommon/GeoUtil.h"
#include <algorithm>
#include <stdlib.h>

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////
	// Constructor
	// Inputs:
	//	const TsString&	tableDir	The directory containing the geocoder lookup tables
	//	const TsString&	databaseDir	The directory containing the geocoder database
	//	MemUse				memUse		Parameter controlling memory usage.
	///////////////////////////////////////////////////////////////////////
	QueryImp::QueryImp(
		const TsString& tableDir_,
		const TsString& databaseDir_,
		Geocoder::MemUse memUse_
	) :
		isOpen(false),
		databaseDir(databaseDir_),
		tableDir(tableDir_),
		memUse(memUse_)
	{}

	///////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////
	QueryImp::~QueryImp()
	{
	}

	///////////////////////////////////////////////////////////////////////////
	// Opens the reference query interface using the parameters
	// given in the subclass constructor.
	// Returns true on success, false on failure.
	// Override ErrorMessage() to get the error messages.
	///////////////////////////////////////////////////////////////////////////
	bool QueryImp::Open()
	{
		if (isOpen) {
			return true;
		}

		stateAbbrToFipsTable = new LookupTable;
		stateFipsToAbbrTable = new LookupTable;

		TsString errorMsg;
		if (
			!stateAbbrToFipsTable->LoadFromFile(tableDir + "/state_abbr_to_fips.csv", errorMsg) ||
			!stateFipsToAbbrTable->LoadFromFile(tableDir + "/state_fips_to_abbr.csv", errorMsg)
		) {
			ErrorMessage(errorMsg);
			return false;
		}

		// Read all of the Huffman-coder frequency table files
		// and initialize the associated Huffman coders.

		// This is for integer coders
		struct IntCoderFiledef {
			IntCoderFiledef(
				HuffmanCoder<int, std::less<int> >& coder_,
				const char* filename_
			) : coder(coder_), filename(filename_)
			{}
			HuffmanCoder<int, std::less<int> >& coder;
			TsString filename;
		} intCoderFiledefs[] = {
			// StreetName
			IntCoderFiledef(streetNameCityStatePostcodeIDCoder,STREET_NAME_CITY_STATE_POSTCODE_ID_HUFF_FILE),
			IntCoderFiledef(streetNameNameCoder,STREET_NAME_NAME_HUFF_FILE),
			IntCoderFiledef(streetNameStreetSegmentIDFirstCoder,STREET_NAME_STREET_SEGMENT_ID_FIRST_HUFF_FILE),
			IntCoderFiledef(streetNameStreetSegmentCountCoder,STREET_NAME_STREET_SEGMENT_COUNT_HUFF_FILE),
			// StreetSegment
			IntCoderFiledef(streetSegmentAddrLowKeyCoder1, STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE1),
			IntCoderFiledef(streetSegmentAddrLowKeyCoder2, STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE2),
			IntCoderFiledef(streetSegmentAddrLowNonkeyCoder1, STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE1),
			IntCoderFiledef(streetSegmentAddrLowNonkeyCoder2, STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE2),
			IntCoderFiledef(streetSegmentAddrHighCoder1, STREET_SEGMENT_ADDR_HIGH_HUFF_FILE1),
			IntCoderFiledef(streetSegmentAddrHighCoder2, STREET_SEGMENT_ADDR_HIGH_HUFF_FILE2),
			IntCoderFiledef(streetSegmentCountyKeyCoder, STREET_SEGMENT_COUNTY_KEY_HUFF_FILE),
			IntCoderFiledef(streetSegmentCountyNonkeyCoder, STREET_SEGMENT_COUNTY_NONKEY_HUFF_FILE),
			IntCoderFiledef(streetSegmentCensusTractKeyCoder1, STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE1),
			IntCoderFiledef(streetSegmentCensusTractKeyCoder2, STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE2),
			IntCoderFiledef(streetSegmentCensusTractNonkeyCoder1, STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE1),
			IntCoderFiledef(streetSegmentCensusTractNonkeyCoder2, STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE2),
			IntCoderFiledef(streetSegmentCensusBlockKeyCoder1, STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE1),
			IntCoderFiledef(streetSegmentCensusBlockKeyCoder2, STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE2),
			IntCoderFiledef(streetSegmentCensusBlockNonkeyCoder1, STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE1),
			IntCoderFiledef(streetSegmentCensusBlockNonkeyCoder2, STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE2),
			IntCoderFiledef(streetSegmentPostcodeExtKeyCoder, STREET_SEGMENT_POSTCODE_EXT_KEY_HUFF_FILE),
			IntCoderFiledef(streetSegmentPostcodeExtNonkeyCoder, STREET_SEGMENT_POSTCODE_EXT_NONKEY_HUFF_FILE),
			IntCoderFiledef(streetSegmentCoordinateIDCoder1, STREET_SEGMENT_COORDINATE_ID_HUFF_FILE1),
			IntCoderFiledef(streetSegmentCoordinateIDCoder2, STREET_SEGMENT_COORDINATE_ID_HUFF_FILE2),
			IntCoderFiledef(streetSegmentCoordinateCountCoder, STREET_SEGMENT_COORDINATE_COUNT_HUFF_FILE),
			// Coordinate
			IntCoderFiledef(coordinateLatitudeCoder1, COORDINATE_LATITUDE_HUFF_FILE1),
			IntCoderFiledef(coordinateLatitudeCoder2, COORDINATE_LATITUDE_HUFF_FILE2),
			IntCoderFiledef(coordinateLongitudeCoder1, COORDINATE_LONGITUDE_HUFF_FILE1),
			IntCoderFiledef(coordinateLongitudeCoder2, COORDINATE_LONGITUDE_HUFF_FILE2),
			// StreetIntersection
			IntCoderFiledef(streetIntersectionStateCoder, STREET_INTERSECTION_STATE_HUFF_FILE),
			IntCoderFiledef(streetIntersectionSoundex1Coder, STREET_INTERSECTION_SOUNDEX1_HUFF_FILE),
			IntCoderFiledef(streetIntersectionStreetNameID1Coder,STREET_INTERSECTION_STREET_NAME_ID1_HUFF_FILE),
			IntCoderFiledef(streetIntersectionStreetSegmentOffset1Coder, STREET_INTERSECTION_STREET_SEGMENT_OFFSET1_HUFF_FILE),
			IntCoderFiledef(streetIntersectionSoundex2Coder, STREET_INTERSECTION_SOUNDEX2_HUFF_FILE),
			IntCoderFiledef(streetIntersectionStreetNameID2Coder, STREET_INTERSECTION_STREET_NAME_ID2_HUFF_FILE),
			IntCoderFiledef(streetIntersectionStreetSegmentOffset2Coder, STREET_INTERSECTION_STREET_SEGMENT_OFFSET2_HUFF_FILE)
		};

		for (
			unsigned int intCoderIdx = 0; 
			intCoderIdx < sizeof(intCoderFiledefs)/sizeof(intCoderFiledefs[0]); 
			intCoderIdx++
		) {
			std::fstream fs;
			TsString filename = databaseDir + "/" + intCoderFiledefs[intCoderIdx].filename;
			fs.open(filename.c_str(), std::ios_base::in);
			FreqTable<int> freqTable;
			if (fs.fail() || !freqTable.Load(fs)) {
				ErrorMessage("Cannot load huffman frequency table " + filename);
				return false;
			}
			intCoderFiledefs[intCoderIdx].coder.Clear();
			intCoderFiledefs[intCoderIdx].coder.AddEntries(freqTable);
			intCoderFiledefs[intCoderIdx].coder.MakeCodes();
			fs.close();
		}

		// This is for string coders
		struct StringCoderFiledef {
			StringCoderFiledef(
				HuffmanCoder<TsString, std::less<TsString> >& coder_,
				const char* filename_
			) : coder(coder_), filename(filename_)
			{}
			HuffmanCoder<TsString, std::less<TsString> >& coder;
			TsString filename;
		} stringCoderFiledefs[] = {
			// StreetName
			StringCoderFiledef(streetNamePrefixCoder, STREET_NAME_PREFIX_HUFF_FILE),
			StringCoderFiledef(streetNamePredirCoder, STREET_NAME_PREDIR_HUFF_FILE),
			StringCoderFiledef(streetNameSuffixCoder, STREET_NAME_SUFFIX_HUFF_FILE),
			StringCoderFiledef(streetNamePostdirCoder, STREET_NAME_POSTDIR_HUFF_FILE)
		};
		for (
			unsigned int stringCoderIdx = 0; 
			stringCoderIdx < sizeof(stringCoderFiledefs)/sizeof(stringCoderFiledefs[0]); 
			stringCoderIdx++
		) {
			std::fstream fs;
			TsString filename = databaseDir + "/" + stringCoderFiledefs[stringCoderIdx].filename;
			fs.open(filename.c_str(), std::ios_base::in);
			StringFreqTable freqTable;
			if (fs.fail() || !freqTable.Load(fs)) {
				ErrorMessage("Cannot load huffman frequency table " + filename);
				return false;
			}
			stringCoderFiledefs[stringCoderIdx].coder.Clear();
			stringCoderFiledefs[stringCoderIdx].coder.AddEntries(freqTable);
			stringCoderFiledefs[stringCoderIdx].coder.MakeCodes();
			fs.close();
		}

		// Open all data files.
		struct InputFileDef {
			InputFileDef(
				DataInput& dataInput_,
				TsString filename_
			) : 
				dataInput(dataInput_),
				filename(filename_)
			{}
			DataInput& dataInput;
			TsString filename;
		} inputFiledefs[] = {
			InputFileDef(cityStatePostcodeInput, CITY_STATE_POSTCODE_FILE ),
			InputFileDef(cityStatePostcodeFaIndexInput, CITY_STATE_POSTCODE_FA_INDEX_FILE ),
			InputFileDef(citySoundexInput, CITY_SOUNDEX_FILE ),
			InputFileDef(streetNameInput, STREET_NAME_FILE ),
			InputFileDef(streetNamePositionIndexInput, STREET_NAME_POSITION_INDEX_FILE ),
			InputFileDef(streetNameSoundexInput, STREET_NAME_SOUNDEX_FILE ),
			InputFileDef(streetSegmentInput, STREET_SEGMENT_FILE ),
			InputFileDef(streetSegmentPositionIndexInput, STREET_SEGMENT_POSITION_INDEX_FILE ),
			InputFileDef(coordinateInput, COORDINATE_FILE ),
			InputFileDef(coordinatePositionIndexInput, COORDINATE_POSITION_INDEX_FILE ),
			InputFileDef(streetIntersectionSoundexInput, STREET_INTERSECTION_SOUNDEX_FILE ),
			InputFileDef(streetIntersectionSoundexPositionIndexInput, STREET_INTERSECTION_SOUNDEX_POSITION_INDEX_FILE ),
			InputFileDef(postcodeAliasByPostcodeInput, POSTCODE_ALIAS_BY_POSTCODE_FILE ),
			InputFileDef(postcodeAliasByGroupInput, POSTCODE_ALIAS_BY_GROUP_FILE ),
			InputFileDef(postcodeCentroidInput, POSTCODE_CENTROID_FILE )
		};
		{for (unsigned int fileIdx = 0; fileIdx < sizeof(inputFiledefs)/sizeof(inputFiledefs[0]); fileIdx++) {
			TsString filename = databaseDir + "/" + inputFiledefs[fileIdx].filename;
			if (!inputFiledefs[fileIdx].dataInput.Open(filename)) {
				Close();
				ErrorMessage("Cannot open data file " + inputFiledefs[fileIdx].filename);
				return false;
			}
		}}

		// Determine the number of CityStatePostcode records.
		if (cityStatePostcodeInput.GetFileSize() % GeoUtil::CityStatePostcodeRecordLength != 0) {
			ErrorMessage("Error reading CityStatePostcode data file");
			return false;
		}
		cityStatePostcodeCount = cityStatePostcodeInput.GetFileSize() / GeoUtil::CityStatePostcodeRecordLength;

		// Determine the number of records in the CityStatePostcode soundex.
		if (citySoundexInput.GetFileSize() % GeoUtil::CitySoundexRecordLength != 0) {
			ErrorMessage("Error reading CityStatePostcodeSoundex data file");
			return false;
		}
		cityStatePostcodeSoundexCount = citySoundexInput.GetFileSize() / GeoUtil::CitySoundexRecordLength;

		// Determine the number of records in the CityStatePostcode Fa Index.
		if (cityStatePostcodeFaIndexInput.GetFileSize() % GeoUtil::CityStatePostcodeFaIndexRecordLength != 0) {
			ErrorMessage("Error reading CityStatePostcodeFaIndex data file");
			return false;
		}
		cityStatePostcodeFaIndexCount = cityStatePostcodeFaIndexInput.GetFileSize() / GeoUtil::CityStatePostcodeFaIndexRecordLength;

		// StreetName table.
		// Read the number of StreetName records from the position index.
		if (
			!streetNamePositionIndexInput.GetBitStream().Seek((__int64)streetNamePositionIndexInput.GetFileSize() * 8 - 32) ||
			!streetNamePositionIndexInput.GetBitStream().ReadBitsIntoInt(32, streetNameCount) ||
			!streetNamePositionIndexInput.Seek(0)
		) {
			ErrorMessage("Error reading StreetName position index");
			return false;
		}

		// Determine the number of records in the StreetNameSoundex index.
		streetNameSoundexCount = streetNameSoundexInput.GetFileSize() * 8 / GeoUtil::StreetNameSoundexRecordBitSize;

		// Determine the number of records in the StreetSegment table
		// by reading the position index
		if (
			!streetSegmentPositionIndexInput.GetBitStream().Seek((__int64)streetSegmentPositionIndexInput.GetFileSize() * 8 - 32) ||
			!streetSegmentPositionIndexInput.GetBitStream().ReadBitsIntoInt(32, streetSegmentCount) ||
			!streetSegmentPositionIndexInput.GetBitStream().Seek(0)
		) {
			ErrorMessage("Error reading StreetSegment position index");
			return false;
		}

		// Determine the number of records in the Coordinate table
		// by reading the position index
		if (
			!coordinatePositionIndexInput.GetBitStream().Seek((__int64)coordinatePositionIndexInput.GetFileSize() * 8 - 32) ||
			!coordinatePositionIndexInput.GetBitStream().ReadBitsIntoInt(32, coordinateCount) ||
			!coordinatePositionIndexInput.GetBitStream().Seek(0)
		) {
			ErrorMessage("Error reading Coordinate position index");
			return false;
		}

		// Determine the number of records in the StreetIntersection table
		// by reading the position index
		if (
			!streetIntersectionSoundexPositionIndexInput.GetBitStream().Seek((__int64)streetIntersectionSoundexPositionIndexInput.GetFileSize() * 8 - 32) ||
			!streetIntersectionSoundexPositionIndexInput.GetBitStream().ReadBitsIntoInt(32, streetIntersectionSoundexCount) ||
			!streetIntersectionSoundexPositionIndexInput.GetBitStream().Seek(0)
		) {
			ErrorMessage("Error reading StreetIntersectionSoundex position index ");
			return false;
		}

		// Determine the number of PostcodeAlias recrds
		if (postcodeAliasByPostcodeInput.GetFileSize() % GeoUtil::PostcodeAliasRecordLength != 0) {
			ErrorMessage("Error reading PostcodeAlias data file");
			return false;
		}
		postcodeAliasCount = postcodeAliasByPostcodeInput.GetFileSize() / GeoUtil::PostcodeAliasRecordLength;

		// Determine the number of PostcodeCentroid records.
		if (postcodeCentroidInput.GetFileSize() % GeoUtil::PostcodeCentroidRecordLength != 0) {
			ErrorMessage("Error reading PostcodeCentroid data file");
			return false;
		}
		postcodeCentroidCount = postcodeCentroidInput.GetFileSize() / GeoUtil::PostcodeCentroidRecordLength;

		// Initialize MRU objects
		// Make sure these don't point to the same chunk as a valid item.
		prevStreetNameID = -10000;
		prevStreetSegmentID = -10000;
		prevCoordinateID = -10000;
		prevStreetIntersectionSoundexID = -10000;

		// Initialize caches
		double scale = 1.0;
		switch (memUse) {
		case Geocoder::MemUseSmall: scale = 0.33; break;
		case Geocoder::MemUseNormal: scale = 1.0; break;
		case Geocoder::MemUseLarge: scale = 3.0; break;
		}

		cityStatePostcodeFaIndexFromFaCache = new CityStatePostcodeFaIndexFromFaCache(
			(int)(CityStatePostcodeFaIndexFromFaCacheSize * scale)
		);
		cityStatePostcodeFaIndexByIDCache = new CityStatePostcodeFaIndexByIDCache(
			(int)(CityStatePostcodeFaIndexByIDCacheSize * scale)
		);
		cityStatePostcodeByIDCache = new CityStatePostcodeIDCache(
			(int)(CityStatePostcodeIDCacheSize * scale)
		);
		cityStatePostcodeSoundexIDCache = new CityStatePostcodeSoundexIDCache(
			(int)(CityStatePostcodeSoundexIDCacheSize * scale)
		);
		streetNameIDCache = new StreetNameIDCache(
			(int)(StreetNameIDCacheSize * scale)
		);
		streetNameSoundexIDCache = new StreetNameSoundexIDCache(
			(int)(StreetNameSoundexIDCacheSize * scale)
		);
		streetNameSoundexFaSoundexCache = new StreetNameSoundexFaSoundexCache(
			(int)(StreetNameSoundexFaSoundexCacheSize * scale)
		);
		streetSegmentIDCache = new StreetSegmentIDCache(
			(int)(StreetSegmentIDCacheSize * scale)
		);
		coordinateIDCache = new CoordinateIDCache(
			(int)(CoordinateIDCacheSize * scale)
		);
		streetIntersectionSoundexIDCache = new StreetIntersectionSoundexIDCache(
			(int)(StreetIntersectionSoundexIDCacheSize * scale)
		);
		postcodeAliasByPostcodeIDCache = new PostcodeAliasByPostcodeIDCache(
			(int)(PostcodeAliasCacheSize * scale)
		);
		postcodeAliasByGroupIDCache = new PostcodeAliasByGroupIDCache(
			(int)(PostcodeAliasCacheSize * scale)
		);
		postcodeGroupFromPostcodeCache = new PostcodeGroupFromPostcodeCache(
			(int)(PostcodeAliasCacheSize * scale)
		);
		postcodeGroupIDFromPostcodeGroupCache = new PostcodeGroupIDFromPostcodeGroupCache(
			(int)(PostcodeAliasCacheSize * scale)
		);
		postcodeCentroidByIDCache = new PostcodeCentroidByIDCache(
			(int)(PostcodeCentroidByIDCacheSize * scale)
		);
		postcodeCentroidFromPostcodeCache = new PostcodeCentroidFromPostcodeCache(
			(int)(PostcodeCentroidFromPostcodeCacheSize * scale)
		);

		isOpen = true;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// Close the reference query interface.
	///////////////////////////////////////////////////////////////////////////
	void QueryImp::Close()
	{
		if (isOpen) {
			// Close input data files
			postcodeCentroidInput.Close();
			cityStatePostcodeInput.Close();
			cityStatePostcodeFaIndexInput.Close();
			citySoundexInput.Close();
			streetNameInput.Close();
			streetNamePositionIndexInput.Close();
			streetNameSoundexInput.Close();
			streetSegmentInput.Close();
			streetSegmentPositionIndexInput.Close();
			coordinateInput.Close();
			coordinatePositionIndexInput.Close();
			streetIntersectionSoundexInput.Close();
			streetIntersectionSoundexPositionIndexInput.Close();

			cityStatePostcodeFaIndexFromFaCache = 0;
			cityStatePostcodeFaIndexByIDCache = 0;
			cityStatePostcodeByIDCache = 0;
			cityStatePostcodeSoundexIDCache = 0;
			streetNameIDCache = 0;
			streetNameSoundexIDCache = 0;
			streetNameSoundexFaSoundexCache = 0;
			streetSegmentIDCache = 0;
			coordinateIDCache = 0;
			streetIntersectionSoundexIDCache = 0;
			postcodeAliasByPostcodeIDCache = 0;
			postcodeAliasByGroupIDCache = 0;
			postcodeGroupFromPostcodeCache = 0;
			postcodeGroupIDFromPostcodeGroupCache = 0;
			postcodeCentroidByIDCache = 0;
			postcodeCentroidFromPostcodeCache = 0;

			stateAbbrToFipsTable = 0;
			stateFipsToAbbrTable = 0;

			isOpen = false;
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Given a postal code, find the list of 
	// associated CityStatePostcode records.
	///////////////////////////////////////////////////////////////////////
	QueryImp::CityStatePostcodeFromPostcodeIterator 
	QueryImp::LookupCityStatePostcodeFromPostcode(
		const char* postcode
	) {
		CityStatePostcode cityStatePostcodeTmp;

		// CityStatePostcode is sorted by postal code.
		// Binary search to find the lower bound
		int first = 0;
		int count = cityStatePostcodeCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetCityStatePostcodeByIDCached(mid, cityStatePostcodeTmp)) {
				return CityStatePostcodeFromPostcodeIterator(this);
			}	
			
			if (strcmp(cityStatePostcodeTmp.postcode, postcode) < 0) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		if (first == cityStatePostcodeCount) {
			// No records have a postal  code >= this one.
			return CityStatePostcodeFromPostcodeIterator(this);
		}	

		// When we get here, first will be the index of the first item whose
		// postal code is >= the given postal code.
		if (
			!GetCityStatePostcodeByIDCached(first, cityStatePostcodeTmp) ||
			strcmp(cityStatePostcodeTmp.postcode, postcode) != 0
		) {
			// No records equal this postal code
			return CityStatePostcodeFromPostcodeIterator(this);
		}

		// We've found the first CityStatePostcode with a postal code == this one.
		return CityStatePostcodeFromPostcodeIterator(this, postcode, first);
	}


	///////////////////////////////////////////////////////////////////////
	// Given a postal code, find the list of all aliased postal codes, defined
	// as those postal codes that are aliases of each other.  Do not return
	// the original postal code.
	///////////////////////////////////////////////////////////////////////
	QueryImp::PostcodeAliasIterator 
	QueryImp::LookupPostcodeAliases(
		const char* postcode
	) {
		// Find the postal code group and its position in the PostcodeAliasByGroup table
		PostcodeAlias postcodeGroup;
		int postcodeGroupID;
		if (
			!GetPostcodeGroupFromPostcodeCached(postcode, postcodeGroup) ||
			!GetPostcodeGroupIDFromPostcodeGroupCached(postcodeGroup.postcodeGroup, postcodeGroupID)
		) {
			// No postal code group found for this postal code.
			// Return an iterator that includes only this postcode
			return PostcodeAliasIterator(this, postcode);
		} else {
			return PostcodeAliasIterator(this, postcodeGroup.postcode, postcodeGroupID);
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Given a city soundex and numeric state code, find
	// the associated CityStatePostcode records.
	///////////////////////////////////////////////////////////////////////
	QueryImp::CityStatePostcodeFromStateCityIterator 
	QueryImp::LookupCityStatePostcodeFromStateCity(
		int state,
		const char* citySoundex
	) {
		CityStatePostcodeSoundex cityStatePostcodeSoundexTmp;

		// CityStatePostcode is sorted by postal code.
		// Binary search to find the lower bound
		int first = 0;
		int count = cityStatePostcodeSoundexCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetCityStatePostcodeSoundexByIDCached(mid, cityStatePostcodeSoundexTmp)) {
				return CityStatePostcodeFromStateCityIterator(this);
			}	

			// Compare midpoint value to keys
			if (
				cityStatePostcodeSoundexTmp.state < state ||
				(
					cityStatePostcodeSoundexTmp.state == state &&
					strcmp(cityStatePostcodeSoundexTmp.citySoundex, citySoundex) < 0
				)
			) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		// When we get here, first will be the index of the first item whose
		// postal code is >= the given postal code.
		if (first == cityStatePostcodeSoundexCount) {
			// No records sort >= this fa/soundex.
			return CityStatePostcodeFromStateCityIterator(this);
		}
		
		if (
			!GetCityStatePostcodeSoundexByIDCached(first, cityStatePostcodeSoundexTmp) ||
			cityStatePostcodeSoundexTmp.state != state ||
			strcmp(cityStatePostcodeSoundexTmp.citySoundex, citySoundex) != 0
		) {
			// No records equal this Fa/soundex
			return CityStatePostcodeFromStateCityIterator(this);
		}

		// We've found the first CityStatePostcode with a postal code == this one.
		return CityStatePostcodeFromStateCityIterator(this, state, citySoundex, first);
	}

	///////////////////////////////////////////////////////////////////////
	// Given Finance Number and soundex-of-street-name, find 
	// a list of StreetName records and associated CityStatePostcode
	///////////////////////////////////////////////////////////////////////
	QueryImp::StreetNameFromFaStreetIterator 
	QueryImp::LookupStreetNameFromFaStreet(
		const char* financeNumber,
		const char* streetSoundex
	) {
		StreetNameSoundex streetNameSoundexTmp;

		// StreetName is sorted by postal code.
		// Binary search to find the lower bound
		int first = 0;
		int count = streetNameSoundexCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetStreetNameSoundexByIDCached(mid, streetNameSoundexTmp)) {
				return StreetNameFromFaStreetIterator(this);
			}	

			// Compare midpoint value to keys
			int cmp = strcmp(streetNameSoundexTmp.financeNumber, financeNumber);
			if (cmp < 0 ||
				(
					cmp == 0 &&
					strcmp(streetNameSoundexTmp.streetSoundex, streetSoundex) < 0
				)
			) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		// When we get here, first will be the index of the first item whose
		// postal code is >= the given postal code.
		if (first == streetNameSoundexCount) {
			// No records sort >= this fa/soundex.
			return StreetNameFromFaStreetIterator(this);
		}
		
		if (
			!GetStreetNameSoundexByIDCached(first, streetNameSoundexTmp) ||
			strcmp(streetNameSoundexTmp.financeNumber, financeNumber) != 0 ||
			strcmp(streetNameSoundexTmp.streetSoundex, streetSoundex) != 0
		) {
			// No records equal this Fa/soundex
			return StreetNameFromFaStreetIterator(this);
		}

		// We've found the first StreetName with a FA == this one.
		return StreetNameFromFaStreetIterator(this, financeNumber, streetSoundex, first);
	}

	///////////////////////////////////////////////////////////////////////
	// Given the soundex for two streets, find the intersections
	///////////////////////////////////////////////////////////////////////
	QueryImp::StreetIntersectionIterator 
	QueryImp::LookupStreetIntersection(
		int stateCode,		// state FIPS code
		const char* street1Soundex,
		const char* street2Soundex
	) {
		if (stateCode < 0 || stateCode > 99) {
			return false;
		}

		StreetIntersectionSoundex streetIntersectionSoundexTmp;

		// Concatenation of soundexes aids in comparison
		char inputSoundexes[8];
		memcpy(inputSoundexes, street1Soundex, 4);
		memcpy(inputSoundexes + 4, street2Soundex, 4);

		// Starting conditions for search
		int first = 0;
		int count = streetIntersectionSoundexCount;

		// Binary search to find the lower bound
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;

			// Read the midpoint.
			if (!GetStreetIntersectionSoundexByIDCached(mid, streetIntersectionSoundexTmp)) {
				return StreetIntersectionIterator(this);
			}

			// Compare midpoint value to keys
			char testSoundexes[8];
			memcpy(testSoundexes, streetIntersectionSoundexTmp.streetSoundex1, 4);
			memcpy(testSoundexes + 4, streetIntersectionSoundexTmp.streetSoundex2, 4);

			if (
				streetIntersectionSoundexTmp.state == stateCode ?
					memcmp(testSoundexes, inputSoundexes, 8) < 0 :
					streetIntersectionSoundexTmp.state < stateCode
			) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		// When we get here, first will be the index of the first item whose
		// keys are >= the given keys
		if (first == streetIntersectionSoundexCount) {
			// No records sort >= this state/soundex.
			return StreetIntersectionIterator(this);
		}
		
		if (
			!GetStreetIntersectionSoundexByIDCached(first, streetIntersectionSoundexTmp) ||
			streetIntersectionSoundexTmp.state != stateCode ||
			strcmp(streetIntersectionSoundexTmp.streetSoundex1, street1Soundex) != 0 ||
			strcmp(streetIntersectionSoundexTmp.streetSoundex2, street2Soundex) != 0
		) {
			// No records equal this state/soundex
			return StreetIntersectionIterator(this);
		}

		// We've found the first StreetIntersectionSoundex with a keys == this one.
		return StreetIntersectionIterator(this, stateCode, street1Soundex, street2Soundex, first);
	}

	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	// Utility methods -- not part of the statndard query interface.
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	// Given the ID (postition) of a CityStatePostcodeFaIndex record, get the record.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetCityStatePostcodeFaIndexByID(
		int cityStatePostcodeFaIndexID,
		CityStatePostcodeFaIndex& cityStatePostcodeFaIndexReturn
	) {
		// Terminate fields
		memset(&cityStatePostcodeFaIndexReturn, 0, sizeof(cityStatePostcodeFaIndexReturn));
		cityStatePostcodeFaIndexReturn.ID = cityStatePostcodeFaIndexID;

		// Seek into file and read fields
		if (
			!cityStatePostcodeFaIndexInput.Seek(cityStatePostcodeFaIndexID * GeoUtil::CityStatePostcodeFaIndexRecordLength) ||
			cityStatePostcodeFaIndexInput.Read(GeoUtil::CityStatePostcodeFaIndexFinanceFieldLength, cityStatePostcodeFaIndexReturn.financeNumber) != GeoUtil::CityStatePostcodeFaIndexFinanceFieldLength ||
			!cityStatePostcodeFaIndexInput.ReadThreeByteInt(cityStatePostcodeFaIndexReturn.cityStatePostcodeID)
		) {
			return false;
		}

		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Get the ID of the first CityStatePostcodeFaIndex record by postcode, uncached
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetCityStatePostcodeFaIndexFromFa(
		const char* finance,
		CityStatePostcodeFaIndex& cityStatePostcodeFaIndexReturn
	) {
		// Binary-search for postal code in CityStatePostcodeFaIndex file.
		int first = 0;
		int count = cityStatePostcodeFaIndexCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetCityStatePostcodeFaIndexByIDCached(mid, cityStatePostcodeFaIndexReturn)) {
				return false;
			}	
			
			if (strcmp(cityStatePostcodeFaIndexReturn.financeNumber, finance) < 0) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		if (first == cityStatePostcodeFaIndexCount) {
			// No records have a postal code >= this one.
			return false;
		}	

		// When we get here, first will be the index of the first item that
		// sorts >= the given postal code.
		if (
			GetCityStatePostcodeFaIndexByIDCached(first, cityStatePostcodeFaIndexReturn) &&
			strcmp(cityStatePostcodeFaIndexReturn.financeNumber, finance) == 0
		) {
			cityStatePostcodeFaIndexFromFaCache->Enter(PostcodeKey(finance), cityStatePostcodeFaIndexReturn);
			return true;
		} else {
			return false;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Get a CityStatePostcode record by ID, uncached.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetCityStatePostcodeByID(
		int cityStatePostcodeID,
		CityStatePostcode& cityStatePostcodeReturn
	) {
		if ((unsigned int)cityStatePostcodeID >= cityStatePostcodeCount) {
			return false;
		}

		// Ensure termination of fields because they are not stored as terminated in the database.
		memset(&cityStatePostcodeReturn, 0, sizeof(cityStatePostcodeReturn));

		cityStatePostcodeReturn.ID = cityStatePostcodeID;

		// Seek to position in file using byte-oriented I/O
		if (
			!cityStatePostcodeInput.Seek(cityStatePostcodeID * GeoUtil::CityStatePostcodeRecordLength) ||
			cityStatePostcodeInput.Read(GeoUtil::CityStatePostcodeCountryFieldLength, cityStatePostcodeReturn.country) != GeoUtil::CityStatePostcodeCountryFieldLength ||
			!cityStatePostcodeInput.ReadOneByteInt(cityStatePostcodeReturn.state) ||
			cityStatePostcodeInput.Read(GeoUtil::CityStatePostcodePostcodeFieldLength, cityStatePostcodeReturn.postcode) != GeoUtil::CityStatePostcodePostcodeFieldLength ||
			cityStatePostcodeInput.Read(GeoUtil::CityStatePostcodeCityNameFieldLength, cityStatePostcodeReturn.city) != GeoUtil::CityStatePostcodeCityNameFieldLength ||
			cityStatePostcodeInput.Read(GeoUtil::CityStatePostcodeFinanceFieldLength, cityStatePostcodeReturn.financeNumber) != GeoUtil::CityStatePostcodeFinanceFieldLength ||
			!cityStatePostcodeInput.ReadThreeByteInt(cityStatePostcodeReturn.streetNameIDFirst) ||
			!cityStatePostcodeInput.ReadThreeByteInt(cityStatePostcodeReturn.streetNameIDLast)
		) {
			return false;
		}

		// Get State abbreviation
		const char *stateAbbr;
		if (StateCodeToAbbr(cityStatePostcodeReturn.state, cityStatePostcodeReturn.country, stateAbbr)) {
			strcpy(cityStatePostcodeReturn.stateAbbr, stateAbbr);
		}

		// Cache result.
		cityStatePostcodeByIDCache->Enter(IntKey(cityStatePostcodeID), cityStatePostcodeReturn);

		return true;
	}

	///////////////////////////////////////////////////////////////////////
	// Get a PostcodeAlias record by ID (position) from the PostcodeAlias records
	// sorted by Group.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetPostcodeAliasByGroupID(
		int postcodeAliasID,
		PostcodeAlias& postcodeAliasReturn
	) {
		if ((unsigned int)postcodeAliasID >= GetPostcodeAliasCount()) {
			return false;
		}
		// Seek to position in file using byte-oriented I/O
		if (
			postcodeAliasByGroupInput.Seek(postcodeAliasID * GeoUtil::PostcodeAliasRecordLength) &&
			postcodeAliasByGroupInput.Read(GeoUtil::PostcodeAliasPostcodeFieldLength, postcodeAliasReturn.postcode) &&
			postcodeAliasByGroupInput.Read(GeoUtil::PostcodeAliasGroupFieldLength, postcodeAliasReturn.postcodeGroup)
		) {
			// Null-termination
			postcodeAliasReturn.postcode[sizeof(postcodeAliasReturn.postcode)-1] = 0;
			postcodeAliasReturn.postcodeGroup[sizeof(postcodeAliasReturn.postcodeGroup)-1] = 0;
			// Cache result.
			postcodeAliasByGroupIDCache->Enter(IntKey(postcodeAliasID), postcodeAliasReturn);
			return true;
		} else {
			return false;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Get a PostcodeAlias record by ID (position) from the PostcodeAlias records
	// sorted by postal code.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetPostcodeAliasByPostcodeID(
		int postcodeAliasID,
		PostcodeAlias& postcodeAliasReturn
	) {
		if ((unsigned int)postcodeAliasID >= GetPostcodeAliasCount()) {
			return false;
		}
		// Seek to position in file using byte-oriented I/O
		if (
			postcodeAliasByPostcodeInput.Seek(postcodeAliasID * GeoUtil::PostcodeAliasRecordLength) &&
			postcodeAliasByPostcodeInput.Read(GeoUtil::PostcodeAliasPostcodeFieldLength, postcodeAliasReturn.postcode) &&
			postcodeAliasByPostcodeInput.Read(GeoUtil::PostcodeAliasGroupFieldLength, postcodeAliasReturn.postcodeGroup)
		) {
			// Cache results.
			postcodeAliasByPostcodeIDCache->Enter(IntKey(postcodeAliasID), postcodeAliasReturn);
			return true;
		} else {
			return false;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Given a postal code, find the associated postal code group
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetPostcodeGroupFromPostcode(
		const char* postcode,
		PostcodeAlias& postcodeGroupReturn
	) {
		// Binary-search for postal code in PostcodeAliasByPostcode file.
		int first = 0;
		int count = postcodeAliasCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetPostcodeAliasByPostcodeIDCached(mid, postcodeGroupReturn)) {
				return false;
			}	
			
			if (strcmp(postcodeGroupReturn.postcode, postcode) < 0) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		if (first == postcodeAliasCount) {
			// No records have a postal code >= this one.
			return false;
		}	

		// When we get here, first will be the index of the first item that
		// sorts >= the given postal code.
		if (
			GetPostcodeAliasByPostcodeIDCached(first, postcodeGroupReturn) &&
			strcmp(postcodeGroupReturn.postcode, postcode) == 0
		) {
			// Cache results.
			postcodeGroupFromPostcodeCache->Enter(PostcodeKey(postcode), postcodeGroupReturn);
			return true;
		} else {
			return false;
		}
	}



	///////////////////////////////////////////////////////////////////////
	// Given a postal code group, find the ID (position) of the first matching 
	// record in the PostcodeAliasByGroup table
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetPostcodeGroupIDFromPostcodeGroup(
		const char* postcodeGroup,
		int& postcodeGroupIDReturn
	) {
		PostcodeAlias postcodeAliasTmp;
		// Binary-search for postal code in PostcodeAliasByGroup file.
		int first = 0;
		int count = postcodeAliasCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetPostcodeAliasByGroupIDCached(mid, postcodeAliasTmp)) {
				return false;
			}	
			
			if (strcmp(postcodeAliasTmp.postcodeGroup, postcodeGroup) < 0) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		if (first == postcodeAliasCount) {
			// No records have a postal code >= this one.
			return false;
		}	

		// When we get here, first will be the index of the first item that
		// sorts >= the given postal code.
		if (
			GetPostcodeAliasByGroupIDCached(first, postcodeAliasTmp) &&
			strcmp(postcodeAliasTmp.postcodeGroup, postcodeGroup) == 0
		) {
			postcodeGroupIDReturn = first;
			// Cache it.
			postcodeGroupIDFromPostcodeGroupCache->Enter(PostcodeKey(postcodeGroup), postcodeGroupIDReturn);
			return true;
		} else {
			return false;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Get a CityStatePostcodeSoundex entry, uncached.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetCityStatePostcodeSoundexByID(
		int cityStatePostcodeSoundexID,
		CityStatePostcodeSoundex& cityStatePostcodeSoundexReturn
	) {
		if (cityStatePostcodeSoundexID < 0 || unsigned(cityStatePostcodeSoundexID) >= cityStatePostcodeSoundexCount) {
			return false;
		}
		unsigned int soundexValue;
		if (
			!citySoundexInput.Seek(cityStatePostcodeSoundexID * GeoUtil::CitySoundexRecordLength) ||
			!citySoundexInput.ReadOneByteInt((unsigned int&)cityStatePostcodeSoundexReturn.state) ||
			!citySoundexInput.ReadTwoByteInt(soundexValue) ||
			!citySoundexInput.ReadThreeByteInt((unsigned int&)cityStatePostcodeSoundexReturn.cityStatePostcodeID)
		) {
			return false;
		}

		GeoUtil::UnpackSoundex(cityStatePostcodeSoundexReturn.citySoundex, soundexValue);
		// Cache results.
		cityStatePostcodeSoundexIDCache->Enter(
			IntKey(cityStatePostcodeSoundexID), 
			cityStatePostcodeSoundexReturn
		);
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Get a StreetName record by ID, uncached
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetStreetNameByID(
		int streetNameID,
		StreetName& streetNameReturn
	) {
		if (streetNameID < 0 || unsigned(streetNameID) >= streetNameCount) {
			return false;
		}

		// If the same as the previous item, then return it.
		if (streetNameID == prevStreetNameID) {
			streetNameReturn = prevStreetName;
			return true;
		}

		// Find block using positional index.
		int indexID = streetNameID / GeoUtil::StreetNameChunkSize;
		int prevIndexID = prevStreetNameID / GeoUtil::StreetNameChunkSize;
		int chunkOffset = streetNameID % GeoUtil::StreetNameChunkSize;
		int prevChunkOffset = prevStreetNameID % GeoUtil::StreetNameChunkSize;

		// Set return ID.
		streetNameReturn.ID = streetNameID;

		bool followsInPrevChunk = 
			indexID == prevIndexID &&
			streetNameID > prevStreetNameID;

		prevStreetNameID = streetNameID;

		// If this sequentially follows in the same chunk as the previous item,
		// then skip the chunk-reading.
		if (!followsInPrevChunk) {
			// Seek into positional index and read chunk-boundary offset
			unsigned int streetNameBitOffset;
			if (
				!streetNamePositionIndexInput.GetBitStream().Seek(indexID * GeoUtil::StreetNamePositionIndexBitSize) ||
				!streetNamePositionIndexInput.GetBitStream().ReadBitsIntoInt(GeoUtil::StreetNamePositionIndexBitSize, streetNameBitOffset) ||
				!streetNameInput.GetBitStream().Seek(streetNameBitOffset)
			) {
				// Cannot process position index
				prevStreetNameID = -10000;
				return false;
			}

			//
			// Read the chunk start record
			//
			if (
				!streetNameInput.ReadBitsIntoInt(GeoUtil::StreetNameCityStatePostcodeIDBitSize, (unsigned int&)streetNameReturn.cityStatePostcodeID) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.prefix, sizeof(streetNameReturn.prefix), streetNamePrefixCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.predir, sizeof(streetNameReturn.predir), streetNamePredirCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.street, sizeof(streetNameReturn.street), streetNameNameCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.suffix, sizeof(streetNameReturn.suffix), streetNameSuffixCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.postdir, sizeof(streetNameReturn.postdir), streetNamePostdirCoder) ||
				!streetNameInput.ReadBitsIntoInt(GeoUtil::StreetNameStreetSegmentIDFirstBitSize, (unsigned int&)streetNameReturn.streetSegmentIDFirst) ||
				!streetNameInput.ReadVarLengthCodedInt((unsigned int&)streetNameReturn.streetSegmentCount, streetNameStreetSegmentCountCoder)
			) {
				prevStreetNameID = -10000;
				return false;
			}

			prevStreetName = streetNameReturn;
			prevChunkOffset = 0;
		}

		// Read non-chunk records until we get to the one we want.
		{for (; prevChunkOffset < chunkOffset; prevChunkOffset++) {
			int cityStatePostcodeIDDiff;
			int streetSegmentIDFirstDiff;
			if (
				!streetNameInput.ReadIntFromCoder(cityStatePostcodeIDDiff, streetNameCityStatePostcodeIDCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.prefix, sizeof(streetNameReturn.prefix), streetNamePrefixCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.predir, sizeof(streetNameReturn.predir), streetNamePredirCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.street, sizeof(streetNameReturn.street), streetNameNameCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.suffix, sizeof(streetNameReturn.suffix), streetNameSuffixCoder) ||
				!streetNameInput.ReadStringFromCoder(streetNameReturn.postdir, sizeof(streetNameReturn.postdir), streetNamePostdirCoder) ||
				!streetNameInput.ReadVarLengthCodedInt(streetSegmentIDFirstDiff, streetNameStreetSegmentIDFirstCoder) ||
				!streetNameInput.ReadVarLengthCodedInt((unsigned int&)streetNameReturn.streetSegmentCount, streetNameStreetSegmentCountCoder)
			) {
				prevStreetNameID = -10000;
				return false;
			}
			streetNameReturn.cityStatePostcodeID = prevStreetName.cityStatePostcodeID + cityStatePostcodeIDDiff;
			streetNameReturn.streetSegmentIDFirst = prevStreetName.streetSegmentIDFirst + streetSegmentIDFirstDiff;
			prevStreetName = streetNameReturn;
		}}

		// Cache result.
		streetNameIDCache->Enter(IntKey(streetNameID), streetNameReturn);
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Get a StreetNameSoundex record by ID, uncached
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetStreetNameSoundexByID(
		int streetNameSoundexID,
		StreetNameSoundex& streetNameSoundexReturn
	) {
		if (streetNameSoundexID < 0 || unsigned(streetNameSoundexID) >= streetNameSoundexCount) {
			return false;
		}

		streetNameSoundexReturn.ID = streetNameSoundexID;

		// Read StreetNameSoundex record
		unsigned int packedSoundex, packedFa;
		if (
			!streetNameSoundexInput.GetBitStream().Seek(streetNameSoundexID * GeoUtil::StreetNameSoundexRecordBitSize) ||
			!streetNameSoundexInput.ReadBitsIntoInt(GeoUtil::StreetNameSoundexFaBitSize, packedFa) ||
			!streetNameSoundexInput.ReadBitsIntoInt(GeoUtil::StreetNameSoundexBitSize, packedSoundex) ||
			!streetNameSoundexInput.ReadBitsIntoInt(GeoUtil::StreetNameSoundexStreetNameIDBitSize, (unsigned int&)streetNameSoundexReturn.streetNameID)
		) {
			return false;
		}
		GeoUtil::UnpackSoundex(streetNameSoundexReturn.streetSoundex, packedSoundex);
		GeoUtil::UnpackFa(streetNameSoundexReturn.financeNumber, packedFa);

		// Cache it.
		streetNameSoundexIDCache->Enter(
			IntKey(streetNameSoundexID), 
			streetNameSoundexReturn
		);
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Find the first StreetNameSoundex record by Finance Number and 
	// Street Name soundex.  Return false if none are found.
	// Uncached version.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::FindStreetNameSoundexByFaSoundex(
		const char* financeNumber,
		const char* soundex,
		StreetNameSoundex& streetNameSoundexReturn
	) {
		// Binary search for the first matching soundex record using 
		// fa as first key and soundex as second.
		int first = 0;
		int count = streetNameSoundexCount;
		while (count > 0) {
			int count2 = count / 2;
			int mid = first + count2;
			// Read the midpoint.
			if (!GetStreetNameSoundexByIDCached(mid, streetNameSoundexReturn)) {
				return false;
			}	
			
			if (
				strcmp(streetNameSoundexReturn.financeNumber, financeNumber) < 0 ||
				(
					strcmp(streetNameSoundexReturn.financeNumber, financeNumber) == 0 &&
					strcmp(streetNameSoundexReturn.streetSoundex, soundex) < 0
				)
			) {
				first = mid + 1;
				count -= count2 + 1;
			} else {
				count = count2; 
			}
		}

		if (first == streetNameSoundexCount) {
			// No records have keys >= this one.
			return false;
		}	

		// When we get here, first will be the index of the first item that
		// sorts >= the given keys.
		if (
			GetStreetNameSoundexByIDCached(first, streetNameSoundexReturn) &&
			strcmp(streetNameSoundexReturn.financeNumber, financeNumber) == 0 &&
			strcmp(streetNameSoundexReturn.streetSoundex, soundex) == 0
		) {
			// Cache result.
			streetNameSoundexFaSoundexCache->Enter(
				StreetNameSoundexFaSoundexKey(financeNumber, soundex), 
				streetNameSoundexReturn
			);
			return true;
		} else {
			return false;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Get a StreetSegment record by ID.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetStreetSegmentByID(
		int streetSegmentID,
		StreetSegment& streetSegmentReturn
	) {
		if (streetSegmentID < 0 || unsigned(streetSegmentID) >= streetSegmentCount) {
			return false;
		}

		// If the same as the previous item, then return it.
		if (streetSegmentID == prevStreetSegmentID) {
			streetSegmentReturn = prevStreetSegment;
			return true;
		}

		// Find block using positional index.
		int indexID = streetSegmentID / GeoUtil::StreetSegmentChunkSize;
		int prevIndexID = prevStreetSegmentID / GeoUtil::StreetSegmentChunkSize;
		int chunkOffset = streetSegmentID % GeoUtil::StreetSegmentChunkSize;
		int prevChunkOffset = prevStreetSegmentID % GeoUtil::StreetSegmentChunkSize;

		// Set return ID.
		streetSegmentReturn.ID = streetSegmentID;

		bool followsInPrevChunk = 
			indexID == prevIndexID &&
			streetSegmentID > prevStreetSegmentID;

		prevStreetSegmentID = streetSegmentID;

		unsigned int isRightSide;
		int countyCode;

		// If this sequentially follows in the same chunk as the previous item,
		// then skip the chunk-reading.
		if (!followsInPrevChunk) {
			// Seek into positional index and read chunk-boundary offset
			unsigned int streetSegmentBitOffset;
			if (
				!streetSegmentPositionIndexInput.GetBitStream().Seek(indexID * GeoUtil::StreetSegmentPositionIndexBitSize) ||
				!streetSegmentPositionIndexInput.GetBitStream().ReadBitsIntoInt(GeoUtil::StreetSegmentPositionIndexBitSize, streetSegmentBitOffset) ||
				!streetSegmentInput.GetBitStream().Seek(streetSegmentBitOffset)
			) {
				// Cannot process position index
				prevStreetSegmentID = -10000;
				return false;
			}

			//
			// Read the chunk start record
			//
			if (
				!streetSegmentInput.ReadRLECompressedStr(streetSegmentReturn.addrLow, sizeof(streetSegmentReturn.addrLow), streetSegmentAddrLowKeyCoder1, streetSegmentAddrLowKeyCoder2, '0') ||
				!streetSegmentInput.ReadRLECompressedStrDiff(streetSegmentReturn.addrHigh, sizeof(streetSegmentReturn.addrHigh), streetSegmentAddrHighCoder1, streetSegmentAddrHighCoder2, streetSegmentReturn.addrLow, '0') ||
				!streetSegmentInput.GetBitStream().ReadBitsIntoInt(GeoUtil::StreetSegmentLeftRightBitSize, isRightSide) ||
				!streetSegmentInput.ReadIntFromCoder(countyCode, streetSegmentCountyKeyCoder) ||
				!streetSegmentInput.ReadRLECompressedStr(streetSegmentReturn.censusTract, sizeof(streetSegmentReturn.censusTract), streetSegmentCensusTractKeyCoder1, streetSegmentCensusTractKeyCoder2, '0') ||
				!streetSegmentInput.ReadRLECompressedStr(streetSegmentReturn.censusBlock, sizeof(streetSegmentReturn.censusBlock), streetSegmentCensusBlockKeyCoder1, streetSegmentCensusBlockKeyCoder2, '0') ||
				!streetSegmentInput.ReadRLECompressedStr(streetSegmentReturn.postcodeExt, sizeof(streetSegmentReturn.postcodeExt), streetSegmentPostcodeExtKeyCoder, '0') ||
				!streetSegmentInput.ReadBitsIntoInt(GeoUtil::StreetSegmentCoordinateIDBitSize, (unsigned int&)streetSegmentReturn.coordinateID) ||
				!streetSegmentInput.ReadIntFromCoder(streetSegmentReturn.coordinateCount, streetSegmentCoordinateCountCoder)
			) {
				prevStreetSegmentID = -10000;
				return false;
			}
			streetSegmentReturn.isRightSide = isRightSide != 0;
			streetSegmentReturn.countyCode = countyCode;
			prevStreetSegment = streetSegmentReturn;
			prevChunkOffset = 0;
		}

		// Read non-chunk records until we get to the one we want.
		{for (; prevChunkOffset < chunkOffset; prevChunkOffset++) {
			int coordinateIDDiff;
			int countyCodeDiff;
			if (
				!streetSegmentInput.ReadRLECompressedStrDiff(streetSegmentReturn.addrLow, sizeof(streetSegmentReturn.addrLow), streetSegmentAddrLowNonkeyCoder1, streetSegmentAddrLowNonkeyCoder2, prevStreetSegment.addrLow, '0') ||
				!streetSegmentInput.ReadRLECompressedStrDiff(streetSegmentReturn.addrHigh, sizeof(streetSegmentReturn.addrHigh), streetSegmentAddrHighCoder1, streetSegmentAddrHighCoder2, streetSegmentReturn.addrLow, '0') ||
				!streetSegmentInput.GetBitStream().ReadBitsIntoInt(GeoUtil::StreetSegmentLeftRightBitSize, isRightSide) ||
				!streetSegmentInput.ReadIntFromCoder(countyCodeDiff, streetSegmentCountyNonkeyCoder) ||
				!streetSegmentInput.ReadRLECompressedStrDiff(streetSegmentReturn.censusTract, sizeof(streetSegmentReturn.censusTract), streetSegmentCensusTractNonkeyCoder1, streetSegmentCensusTractNonkeyCoder2, prevStreetSegment.censusTract, '0') ||
				!streetSegmentInput.ReadRLECompressedStrDiff(streetSegmentReturn.censusBlock, sizeof(streetSegmentReturn.censusBlock), streetSegmentCensusBlockNonkeyCoder1, streetSegmentCensusBlockNonkeyCoder2, prevStreetSegment.censusBlock, '0') ||
				!streetSegmentInput.ReadRLECompressedStrDiff(streetSegmentReturn.postcodeExt, sizeof(streetSegmentReturn.postcodeExt), streetSegmentPostcodeExtNonkeyCoder, prevStreetSegment.postcodeExt, '0') ||
				!streetSegmentInput.ReadVarLengthCodedInt(coordinateIDDiff, streetSegmentCoordinateIDCoder1, streetSegmentCoordinateIDCoder2)
			) {
				prevStreetSegmentID = -10000;
				return false;
			}
			if (coordinateIDDiff == 0) {
				// The coordinate count will also be the same as previous count.
				streetSegmentReturn.coordinateCount = prevStreetSegment.coordinateCount;
			} else if (!streetSegmentInput.ReadIntFromCoder(streetSegmentReturn.coordinateCount, streetSegmentCoordinateCountCoder)) {
				prevStreetSegmentID = -10000;
				return false;
			}
			streetSegmentReturn.isRightSide = isRightSide != 0;
			streetSegmentReturn.countyCode = prevStreetSegment.countyCode + countyCodeDiff;
			streetSegmentReturn.coordinateID = prevStreetSegment.coordinateID + coordinateIDDiff;
			prevStreetSegment = streetSegmentReturn;
		}}

		// Cache it.
		streetSegmentIDCache->Enter(IntKey(streetSegmentID), streetSegmentReturn);
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Get a Coordinate record by ID.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetCoordinateByID(
		int coordinateID,
		CoordinatePoint& coordinateReturn
	) {
		if (coordinateID < 0 || unsigned(coordinateID) >= coordinateCount) {
			return false;
		}

		// If the same as the previous item, then return it.
		if (coordinateID == prevCoordinateID) {
			coordinateReturn.latitude = (double)prevCoordinateLat / 100000.0;
			coordinateReturn.longitude = (double)prevCoordinateLon / 100000.0;
			return true;
		}

		// Find block using positional index.
		int indexID = coordinateID / GeoUtil::CoordinateChunkSize;
		int prevIndexID = prevCoordinateID / GeoUtil::CoordinateChunkSize;
		int chunkOffset = coordinateID % GeoUtil::CoordinateChunkSize;
		int prevChunkOffset = prevCoordinateID % GeoUtil::CoordinateChunkSize;

		bool followsInPrevChunk = 
			indexID == prevIndexID &&
			coordinateID > prevCoordinateID;

		prevCoordinateID = coordinateID;

		// If this sequentially follows in the same chunk as the previous item,
		// then skip the chunk-reading.
		if (!followsInPrevChunk) {
			unsigned int coordinateBitOffset;
			if (
				!coordinatePositionIndexInput.GetBitStream().Seek(indexID * GeoUtil::CoordinatePositionIndexBitSize) ||
				!coordinatePositionIndexInput.GetBitStream().ReadBitsIntoInt(GeoUtil::CoordinatePositionIndexBitSize, coordinateBitOffset) ||
				!coordinateInput.GetBitStream().Seek(coordinateBitOffset)
			) {
				// Cannot process position index
				prevCoordinateID = -10000;
				return false;
			}

			//
			// Read the chunk start record
			//
			if (
				!coordinateInput.ReadBitsIntoInt(GeoUtil::CoordinateLatitudeBitSize, prevCoordinateLat) ||
				!coordinateInput.ReadBitsIntoInt(GeoUtil::CoordinateLongitudeBitSize, prevCoordinateLon)
			) {
				prevCoordinateID = -10000;
				return false;
			}
			coordinateReturn.latitude = (double)prevCoordinateLat / 100000.0;
			coordinateReturn.longitude = (double)prevCoordinateLon / 100000.0;
			prevChunkOffset = 0;
		}

		// Read non-chunk records until we get to the one we want.
		{for (; prevChunkOffset < chunkOffset; prevChunkOffset++) {
			int latDiff;
			int lonDiff;
			if (
				!coordinateInput.ReadVarLengthCodedInt(latDiff, coordinateLatitudeCoder1, coordinateLatitudeCoder2) ||
				!coordinateInput.ReadVarLengthCodedInt(lonDiff, coordinateLongitudeCoder1, coordinateLongitudeCoder2)
			) {
				prevCoordinateID = -10000;
				return false;
			}
			prevCoordinateLat += latDiff;
			prevCoordinateLon += lonDiff;
			coordinateReturn.latitude = (double)prevCoordinateLat / 100000.0;
			coordinateReturn.longitude = (double)prevCoordinateLon / 100000.0;
		}}

		// Cache it.
		coordinateIDCache->Enter(IntKey(coordinateID), coordinateReturn);
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Get a StreetIntersectionSoundex record by ID, uncached version.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetStreetIntersectionSoundexByID(
		int streetIntersectionSoundexID,
		StreetIntersectionSoundex& streetIntersectionSoundexReturn
	) {
		if (streetIntersectionSoundexID < 0 || unsigned(streetIntersectionSoundexID) >= streetIntersectionSoundexCount) {
			return false;
		}

		// If the same as the previous item, then return it.
		if (streetIntersectionSoundexID == prevStreetIntersectionSoundexID) {
			streetIntersectionSoundexReturn = prevStreetIntersectionSoundex;
			return true;
		}

		// Find block using positional index.
		int indexID = streetIntersectionSoundexID / GeoUtil::StreetIntersectionSoundexChunkSize;
		int prevIndexID = prevStreetIntersectionSoundexID / GeoUtil::StreetIntersectionSoundexChunkSize;
		int chunkOffset = streetIntersectionSoundexID % GeoUtil::StreetIntersectionSoundexChunkSize;
		int prevChunkOffset = prevStreetIntersectionSoundexID % GeoUtil::StreetIntersectionSoundexChunkSize;

		bool followsInPrevChunk = 
			indexID == prevIndexID &&
			streetIntersectionSoundexID > prevStreetIntersectionSoundexID;

		prevStreetIntersectionSoundexID = streetIntersectionSoundexID;


		// If this sequentially follows in the same chunk as the previous item,
		// then skip the chunk-reading.
		if (!followsInPrevChunk) {
			// Seek into positional index and read chunk-boundary offset
			unsigned int streetIntersectionSoundexBitOffset;
			if (
				!streetIntersectionSoundexPositionIndexInput.GetBitStream().Seek(indexID * GeoUtil::StreetIntersectionPositionIndexBitSize) ||
				!streetIntersectionSoundexPositionIndexInput.GetBitStream().ReadBitsIntoInt(GeoUtil::StreetIntersectionPositionIndexBitSize, streetIntersectionSoundexBitOffset) ||
				!streetIntersectionSoundexInput.GetBitStream().Seek(streetIntersectionSoundexBitOffset)
			) {
				// Cannot process position index
				prevStreetIntersectionSoundexID = -10000;
				return false;
			}

			//
			// Read the chunk start record
			//
			unsigned int soundexValue1, soundexValue2;
			if (
				!streetIntersectionSoundexInput.ReadBitsIntoInt(GeoUtil::StreetIntersectionStateBitSize, (unsigned int&)streetIntersectionSoundexReturn.state) ||
				!streetIntersectionSoundexInput.ReadBitsIntoInt(GeoUtil::StreetIntersectionSoundexBitSize, soundexValue1) ||
				!streetIntersectionSoundexInput.ReadBitsIntoInt(GeoUtil::StreetIntersectionStreetNameIDBitSize, (unsigned int&)streetIntersectionSoundexReturn.streetNameID1) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt((unsigned int&)streetIntersectionSoundexReturn.streetSegmentOffset1, streetIntersectionStreetSegmentOffset1Coder) ||
				!streetIntersectionSoundexInput.ReadBitsIntoInt(GeoUtil::StreetIntersectionSoundexBitSize, soundexValue2) ||
				!streetIntersectionSoundexInput.ReadBitsIntoInt(GeoUtil::StreetIntersectionStreetNameIDBitSize, (unsigned int&)streetIntersectionSoundexReturn.streetNameID2) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt((unsigned int&)streetIntersectionSoundexReturn.streetSegmentOffset2, streetIntersectionStreetSegmentOffset2Coder)
			) {
				prevStreetIntersectionSoundexID = -10000;
				return false;
			}
			GeoUtil::UnpackSoundex(streetIntersectionSoundexReturn.streetSoundex1, soundexValue1);
			GeoUtil::UnpackSoundex(streetIntersectionSoundexReturn.streetSoundex2, soundexValue2);

			prevStreetIntersectionSoundex = streetIntersectionSoundexReturn;
			prevChunkOffset = 0;
		}

		// Read non-chunk records until we get to the one we want.
		{for (; prevChunkOffset < chunkOffset; prevChunkOffset++) {
			int soundex1Diff, soundex2Diff;
			int streetNameID1Diff, streetNameID2Diff;
			int stateDiff;
			if (
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt(stateDiff, streetIntersectionStateCoder) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt(soundex1Diff, streetIntersectionSoundex1Coder) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt(streetNameID1Diff, streetIntersectionStreetNameID1Coder) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt((unsigned int&)streetIntersectionSoundexReturn.streetSegmentOffset1, streetIntersectionStreetSegmentOffset1Coder) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt(soundex2Diff, streetIntersectionSoundex2Coder) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt(streetNameID2Diff, streetIntersectionStreetNameID2Coder) ||
				!streetIntersectionSoundexInput.ReadVarLengthCodedInt((unsigned int&)streetIntersectionSoundexReturn.streetSegmentOffset2, streetIntersectionStreetSegmentOffset2Coder)
			) {
				prevStreetIntersectionSoundexID = -10000;
				return false;
			}
			// Add differences
			streetIntersectionSoundexReturn.state = prevStreetIntersectionSoundex.state + stateDiff;
			GeoUtil::UnpackSoundex(
				streetIntersectionSoundexReturn.streetSoundex1,
				GeoUtil::PackSoundex(prevStreetIntersectionSoundex.streetSoundex1) + soundex1Diff
			);
			GeoUtil::UnpackSoundex(
				streetIntersectionSoundexReturn.streetSoundex2,
				GeoUtil::PackSoundex(prevStreetIntersectionSoundex.streetSoundex2) + soundex2Diff
			);
			streetIntersectionSoundexReturn.streetNameID1 = prevStreetIntersectionSoundex.streetNameID1 + streetNameID1Diff;
			streetIntersectionSoundexReturn.streetNameID2 = prevStreetIntersectionSoundex.streetNameID2 + streetNameID2Diff;
			// Save as prev value
			prevStreetIntersectionSoundex = streetIntersectionSoundexReturn;
		}}

		// Cache it.
		streetIntersectionSoundexIDCache->Enter(
			IntKey(streetIntersectionSoundexID), 
			streetIntersectionSoundexReturn
		);
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Given a StreetIntersectionSoundex record, build the associated
	// StreetIntersection record.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetStreetIntersectionFromStreetIntersectionSoundex(
		const StreetIntersectionSoundex& streetIntersectionSoundex,
		StreetIntersection& streetIntersectionReturn
	) {
		return
			GetStreetNameByIDCached(streetIntersectionSoundex.streetNameID1, streetIntersectionReturn.streetName1) &&
			GetCityStatePostcodeByIDCached(streetIntersectionReturn.streetName1.cityStatePostcodeID, streetIntersectionReturn.cityStatePostcode1) &&
			GetStreetSegmentByIDCached(
				streetIntersectionReturn.streetName1.streetSegmentIDFirst + streetIntersectionSoundex.streetSegmentOffset1,
				streetIntersectionReturn.streetSegment1
			) &&
			GetStreetNameByIDCached(streetIntersectionSoundex.streetNameID2, streetIntersectionReturn.streetName2) &&
			GetCityStatePostcodeByIDCached(streetIntersectionReturn.streetName2.cityStatePostcodeID, streetIntersectionReturn.cityStatePostcode2) &&
			GetStreetSegmentByIDCached(
				streetIntersectionReturn.streetName2.streetSegmentIDFirst + streetIntersectionSoundex.streetSegmentOffset2,
				streetIntersectionReturn.streetSegment2
			);
	}


	///////////////////////////////////////////////////////////////////////
	// Given a postal code code, find the associated centroid
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetPostcodeCentroidFromPostcode(
		const char* postcode,
		PostcodeCentroid& postcodeCentroidReturn
	) {
		// Binary-search for postal code in postal code centroid file.
		int left = 0;
		int right = postcodeCentroidCount - 1;
		while (right >= left) {
			int mid = (left + right) / 2;
			// Read the midpoint.
			if (!GetPostcodeCentroidByIDCached(mid, postcodeCentroidReturn)) {
				return false;
			}	

			int cmp = strcmp(postcode, postcodeCentroidReturn.postcode);
			if (cmp == 0) {
				// Cache result.
				postcodeCentroidFromPostcodeCache->Enter(PostcodeKey(postcode), postcodeCentroidReturn);
				return true;
			} else if (cmp < 0) {
				right = mid - 1;
			} else {
				left = mid + 1;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////
	// Read a postcode centroid given its positional ID.
	///////////////////////////////////////////////////////////////////////
	bool QueryImp::GetPostcodeCentroidByID(
		int postcodeCentroidID,
		PostcodeCentroid& postcodeCentroidReturn
	) {
		if ((unsigned int)postcodeCentroidID >= postcodeCentroidCount) {
			return false;
		}
		// Seek to position in file using byte-oriented I/O
		unsigned char clat[4], clon[4];
		int lat, lon;

		if (
			postcodeCentroidInput.Seek(postcodeCentroidID * GeoUtil::PostcodeCentroidRecordLength) &&
			postcodeCentroidInput.Read(GeoUtil::PostcodeCentroidPostcodeFieldLength, postcodeCentroidReturn.postcode) == GeoUtil::PostcodeCentroidPostcodeFieldLength &&
			postcodeCentroidInput.Read(4, clat) &&
			postcodeCentroidInput.Read(4, clon)
		) {
		  	unsigned int i = 1;
		  	if (*((char *)(&i)) != 1) { // big endian, so swap the bytes
				std::swap(clat[0], clat[3]);
				std::swap(clat[1], clat[2]);
				std::swap(clon[0], clon[3]);
				std::swap(clon[1], clon[2]);
				// Copy to the last four bytes
				memcpy(((unsigned char *)&lat) + (sizeof lat - 4), clat, 4);
				memcpy(((unsigned char *)&lon) + (sizeof lon - 4), clon, 4);
			} else { // little endian, so the bytes can go in the front
				memcpy(&lat, clat, 4);
				memcpy(&lon, clon, 4);
			}

			postcodeCentroidReturn.postcode[GeoUtil::PostcodeCentroidPostcodeFieldLength] = 0;
			postcodeCentroidReturn.latitude = (double)lat / 100000.0;
			postcodeCentroidReturn.longitude = (double)lon / 100000.0;
			return true;
		} else {
			return false;
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
	bool QueryImp::StateAbbrToCode(
		const char* stateAbbr,
		const char* countryCode,
		int& stateCodeReturn
	) {
		char tmp[32];
		if (*countryCode != 0) {
			// Use country in table lookup
			strcpy(tmp, countryCode);
			strcat(tmp, stateAbbr);
			for (int i = 0; tmp[i] != 0; i++) {
				tmp[i] = toupper(tmp[i]);
			}
		} else {
			// No country code; this will incur some abiguity which will be resolved
			// in favor of US
			strcpy(tmp, stateAbbr);
			for (int i = 0; tmp[i] != 0; i++) {
				tmp[i] = toupper(tmp[i]);
			}
		}
		const char* tmpStr2;
		if (stateAbbrToFipsTable->Find(tmp, tmpStr2)) {
			stateCodeReturn = atoi(tmpStr2);
			return true;
		} else {
			stateCodeReturn = 0;
			return false;
		}
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
	bool QueryImp::StateCodeToAbbr(
		int stateCode,
		const char* countryCode,
		const char*& stateAbbrReturn
	) {
		// Just in case...
		stateAbbrReturn = "";

		char tmp[20];
		if (*countryCode != 0) {
			tmp[0] = toupper(countryCode[0]);
			tmp[1] = toupper(countryCode[1]);
			sprintf(tmp+2, "%02d", stateCode);
		} else {
			// No country code available.
			sprintf(tmp, "%02d", stateCode);
		}
		return stateFipsToAbbrTable->Find(tmp, stateAbbrReturn);
	}


}

