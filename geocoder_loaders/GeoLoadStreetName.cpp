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

// GeoLoadStreetName.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <time.h>

#include "GeoLoadStreetName.h"

#include <fstream>
#include "BitStreamAdaptor.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Local utility function.
	// Add the elements of a string to a freq table.
	///////////////////////////////////////////////////////////////////////////////
	static void AddStrToFreqTable(
		FreqTable<int>& freqTable,
		const TsString& str
	) {
		for (unsigned i = 0; i < str.size(); i++) {
			freqTable.Count((unsigned char)str[i]);
		}
		// For termination
		freqTable.Count(0);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Local utility function.
	// Add the elements of an unsigned char array to a freq table.
	///////////////////////////////////////////////////////////////////////////////
	static void AddArrayToFreqTable(
		FreqTable<int>& freqTable,
		const unsigned char* array,
		int arraySize
	) {
		for (int i = 0; i < arraySize; i++) {
			freqTable.Count(array[i]);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadStreetName::Process()
	{
		numberOfOutputRecords = 0;

		bool inputOK = m_readCSV.ReadRecord();

		if (!inputOK)
			throw TsString("No records to read");

		// Frequency-counting tables.
		FreqTable<int> cityStatePostcodeIDFreqTable;
		FreqTable<TsString> prefixFreqTable;
		FreqTable<TsString> predirFreqTable;
		FreqTable<int> streetNameFreqTable;
		FreqTable<TsString> suffixFreqTable;
		FreqTable<TsString> postdirFreqTable;
		FreqTable<int> streetRangeIDFirstFreqTable;
		FreqTable<int> streetRangeCountFreqTable;

		// Get local variable equivalents of bound fields.
		FieldAccessor cityStatePostcodeIDValue = m_mapFieldAccessors["CITY_STATE_POSTCODE_ID"];
		FieldAccessor streetNameIDValue = m_mapFieldAccessors["STREET_NAME_ID"];
		FieldAccessor prefixValue = m_mapFieldAccessors["PREFIX"];
		FieldAccessor predirValue = m_mapFieldAccessors["PREDIR"];
		FieldAccessor nameValue = m_mapFieldAccessors["NAME"];
		FieldAccessor suffixValue = m_mapFieldAccessors["SUFFIX"];
		FieldAccessor postdirValue = m_mapFieldAccessors["POSTDIR"];
		FieldAccessor streetRangeIDFirstValue = m_mapFieldAccessors["STREET_SEGMENT_ID_FIRST"];
		FieldAccessor streetRangeCountValue = m_mapFieldAccessors["STREET_SEGMENT_COUNT"];

		// Read all records and analyze frequency counts for Huffman coding.
		// Buffer the records to a temp file so we can read them back later.
		{
			int chunkCount = 0;

			// Put outside loop to avoid repeated construction.
			TsString tmpStr;

			int prevStreetRangeIDFirst = 0;
			int prevCityStatePostcodeID = 0;

			do {
				unsigned char tmpBuf[10];
				unsigned int tmpUInt;
				int varIntLength;
				int streetRangeIDFirst;
				int cityStatePostcodeID;
			

				cityStatePostcodeID = cityStatePostcodeIDValue.GetAsInt();
				streetRangeIDFirst = streetRangeIDFirstValue.GetAsInt();

				if (chunkCount == 0) {
					// Key-record-only freq counts
				} else {
					// Non-key-record freq counts
					// StreetRangeIDFirst
					int diff = streetRangeIDFirst - prevStreetRangeIDFirst;
					varIntLength = IntToVarLengthBuf(diff, tmpBuf);
					AddArrayToFreqTable(streetRangeIDFirstFreqTable, tmpBuf, varIntLength);

					cityStatePostcodeIDFreqTable.Count(cityStatePostcodeID - prevCityStatePostcodeID);
				}

				// Freq counts for key and non-key records
				// Prefix
				tmpStr = prefixValue.GetAsString();
				prefixFreqTable.Count(tmpStr);
				// Predir
				tmpStr = predirValue.GetAsString();
				predirFreqTable.Count(tmpStr);
				// Name
				tmpStr = nameValue.GetAsString();
				AddStrToFreqTable(streetNameFreqTable, tmpStr);
				// Suffix
				tmpStr = suffixValue.GetAsString();
				suffixFreqTable.Count(tmpStr);
				// Postdir
				tmpStr = postdirValue.GetAsString();
				postdirFreqTable.Count(tmpStr);
				// StreetRangeCount
				tmpUInt = streetRangeCountValue.GetAsInt();
				varIntLength = IntToVarLengthBuf(tmpUInt, tmpBuf);
				AddArrayToFreqTable(streetRangeCountFreqTable, tmpBuf, varIntLength);

				// Remember these values as prev values
				prevStreetRangeIDFirst = streetRangeIDFirst;
				prevCityStatePostcodeID = cityStatePostcodeID;

				chunkCount++;
				if (chunkCount == StreetNameChunkSize) {
					chunkCount = 0;
				}
			} while (m_readCSV.ReadRecord());
		}

		// Open files
		File dataFile;
		TsString filename = outdir + "/" + STREET_NAME_FILE;
		if (!dataFile.Open(File::CreateAndWrite, filename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + filename + " for output"
			);
		}
		File positionIndexFile;
		filename = outdir + "/" + STREET_NAME_POSITION_INDEX_FILE;
		if (!positionIndexFile.Open(File::CreateAndWrite, filename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + filename + " for output"
			);
		}

		// Create an output BitStream for the data file
		BitStreamWrite dataBitStream(new FileBitStreamAdaptor(dataFile));

		// Create an output BitStream for the position-index file
		BitStreamWrite positionIndexBitStream(new FileBitStreamAdaptor(positionIndexFile));

		// Set up Huffman coding.
		HuffmanCoder<int, std::less<int> > cityStatePostcodeIDCoder;
		HuffmanCoder<TsString, std::less<TsString> > prefixCoder;
		HuffmanCoder<TsString, std::less<TsString> > predirCoder;
		HuffmanCoder<int, std::less<int> > streetNameCoder;
		HuffmanCoder<TsString, std::less<TsString> > suffixCoder;
		HuffmanCoder<TsString, std::less<TsString> > postdirCoder;
		HuffmanCoder<int, std::less<int> > streetRangeIDFirstCoder;
		HuffmanCoder<int, std::less<int> > streetRangeCountCoder;

		// Populate the huffman coder tables 
		cityStatePostcodeIDCoder.AddEntries(cityStatePostcodeIDFreqTable);
		prefixCoder.AddEntries(prefixFreqTable);
		predirCoder.AddEntries(predirFreqTable);
		streetNameCoder.AddEntries(streetNameFreqTable);
		suffixCoder.AddEntries(suffixFreqTable);
		postdirCoder.AddEntries(postdirFreqTable);
		streetRangeIDFirstCoder.AddEntries(streetRangeIDFirstFreqTable);
		streetRangeCountCoder.AddEntries(streetRangeCountFreqTable);

		// Generate Huffman codes
		cityStatePostcodeIDCoder.MakeCodes();
		prefixCoder.MakeCodes();
		predirCoder.MakeCodes();
		streetNameCoder.MakeCodes();
		suffixCoder.MakeCodes();
		postdirCoder.MakeCodes();
		streetRangeIDFirstCoder.MakeCodes();
		streetRangeCountCoder.MakeCodes();

		int chunkCount = 0;


		m_readCSV.ReOpen();
		m_readCSV.ReadRecord();


		// Temp variables 
		TsString tmpStr;
		int varIntLength;
		unsigned char tmpBuf[100];
		
		// previous-record values.
		int prevCityStatePostcodeID = 0;
		int prevStreetRangeIDFirst = 0;

		while (m_readCSV.ReadRecord()) {

			// Read output variables
			int streetNameID = streetNameIDValue.GetAsInt();
			assert(streetNameID == numberOfOutputRecords);

			// Write position-index records.
			// Must be written before the data file, to get the correct bit offset.
			if (chunkCount == 0) {
				__int64 bitsWritten = dataBitStream.GetNumberOfBitsWritten();

				if (bitsWritten >= ((__int64)1 << StreetNamePositionIndexBitSize)) {
					throw TsString("StreetName position index uses " + FormatInteger(StreetNamePositionIndexBitSize) + " bits but must be larger");
				}

				if (StreetNamePositionIndexBitSize > 32) {
					throw TsString("StreetName position index uses " + FormatInteger(StreetNamePositionIndexBitSize) + " bits; must recode to use __int64 variables");
				}

				positionIndexBitStream.WriteBitsFromInt(StreetNamePositionIndexBitSize, (int)bitsWritten);
			}

			// Write the fields of the data record.

			// CityStatePostcodeID
			int cityStatePostcodeID = cityStatePostcodeIDValue.GetAsInt();
			if (chunkCount == 0) {
				// Key record
				if (cityStatePostcodeID >= (1 << StreetNameCityStatePostcodeIDBitSize)) {
					// Does not fit
					throw TsString(
						"StreetNameCityStatePostcodeIDBitSize is " + FormatInteger(StreetNameCityStatePostcodeIDBitSize) + " bits, but is not large enough to hold CityStatePostcode ID value"
					);
				}
				dataBitStream.WriteBitsFromInt(StreetNameCityStatePostcodeIDBitSize, cityStatePostcodeID);
			} else {
				int diff = cityStatePostcodeID - prevCityStatePostcodeID;
				if (!cityStatePostcodeIDCoder.WriteCode(diff, dataBitStream)) {
					throw TsString("Error in Huffman code table");
				}
			}

			// Prefix
			tmpStr = prefixValue.GetAsString();
			if (!prefixCoder.WriteCode(tmpStr, dataBitStream)) {
				throw TsString("Error in Huffman code table");
			}

			// Predir
			tmpStr = predirValue.GetAsString();
			if (!predirCoder.WriteCode(tmpStr, dataBitStream)) {
				throw TsString("Error in Huffman code table");
			}

			// Name
			tmpStr = nameValue.GetAsString();
			{for (unsigned i = 0; i < tmpStr.size(); i++) {
				if (!streetNameCoder.WriteCode((unsigned char)tmpStr[i], dataBitStream)) {
					throw TsString("Error in Huffman code table");
				}
			}}
			// ... and name termination
			if (!streetNameCoder.WriteCode(0, dataBitStream)) {
				throw TsString("Error in Huffman code table");
			}

			// Suffix
			tmpStr = suffixValue.GetAsString();
			if (!suffixCoder.WriteCode(tmpStr, dataBitStream)) {
				throw TsString("Error in Huffman code table");
			}

			// Postdir
			tmpStr = postdirValue.GetAsString();
			if (!postdirCoder.WriteCode(tmpStr, dataBitStream)) {
				throw TsString("Error in Huffman code table");
			}

			// StreetRangeIDFirst
			int streetRangeIDFirst = streetRangeIDFirstValue.GetAsInt();
			if (chunkCount == 0) {
				// Key record
				if (streetRangeIDFirst >= (1 << StreetNameStreetSegmentIDFirstBitSize)) {
					// Does not fit
					throw TsString(
						"StreetNameStreetSegmentIDFirstBitSize is " + FormatInteger(StreetNameStreetSegmentIDFirstBitSize) + " bits, but is not large enough to hold StreetNameStreetRangeID value"
					);
				}
				dataBitStream.WriteBitsFromInt(StreetNameStreetSegmentIDFirstBitSize, streetRangeIDFirst);
			} else {
				// Non-key-record
				// Remember this value as prev value
				varIntLength = IntToVarLengthBuf((int)(streetRangeIDFirst - prevStreetRangeIDFirst), tmpBuf);
				for (int i = 0; i < varIntLength; i++) {
					if (!streetRangeIDFirstCoder.WriteCode(tmpBuf[i], dataBitStream)) {
						throw TsString("Error in Huffman code table");
					}
				}
			}

			// StreetRangeCount 
			// Must be an unsigned int!
			unsigned int streetRangeCount = streetRangeCountValue.GetAsInt();
			varIntLength = IntToVarLengthBuf(streetRangeCount, tmpBuf);
			for (int i = 0; i < varIntLength; i++) {
				if (!streetRangeCountCoder.WriteCode(tmpBuf[i], dataBitStream)) {
					throw TsString("Error in Huffman code table");
				}
			}

			// Remember these values as prev values
			prevStreetRangeIDFirst = streetRangeIDFirst;
			prevCityStatePostcodeID = cityStatePostcodeID;

			chunkCount++;
			numberOfOutputRecords++;
			if (chunkCount == StreetNameChunkSize) {
				chunkCount = 0;
			}


		}

		// Flush bitstream to byte-align it.
		positionIndexBitStream.Flush();
		// Write the total number of records at the end of the position index.
		positionIndexBitStream.WriteBitsFromInt(32, numberOfOutputRecords);
		positionIndexBitStream.Flush();
		positionIndexFile.Close();

		dataBitStream.Flush();
		dataFile.Close();

		// Write out the huffman code tables.
		std::fstream fs;

		// ID
		filename = outdir + "/" + STREET_NAME_CITY_STATE_POSTCODE_ID_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !cityStatePostcodeIDFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// Prefix
		filename = outdir + "/" + STREET_NAME_PREFIX_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !prefixFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// Predir
		filename = outdir + "/" + STREET_NAME_PREDIR_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !predirFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// Name
		filename = outdir + "/" + STREET_NAME_NAME_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !streetNameFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// Suffix
		filename = outdir + "/" + STREET_NAME_SUFFIX_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !suffixFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// Postdir
		filename = outdir + "/" + STREET_NAME_POSTDIR_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !postdirFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// StreetRangeIDFirst
		filename = outdir + "/" + STREET_NAME_STREET_SEGMENT_ID_FIRST_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !streetRangeIDFirstFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

		// StreetRangeCount
		filename = outdir + "/" + STREET_NAME_STREET_SEGMENT_COUNT_HUFF_FILE;
		fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (fs.fail() || !streetRangeCountFreqTable.Save(fs)) {
			throw TsString("Cannot write " + filename);
		}
		fs.close();

	}

	///////////////////////////////////////////////////////////////////////////////
	// Get a static array of FieldParameter entries, which will be used to
	// load up the fieldParameters vector.  The terminating element must
	// have an empty paramName.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	GeoLoadStreetName::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("CITY_STATE_POSTCODE_ID");
		retval.push_back("STREET_NAME_ID");
		retval.push_back("PREFIX");
		retval.push_back("PREDIR");
		retval.push_back("NAME");
		retval.push_back("SUFFIX");
		retval.push_back("POSTDIR");
		retval.push_back("STREET_SEGMENT_ID_FIRST");
		retval.push_back("STREET_SEGMENT_COUNT");

		return retval;
	};

} // namespace

