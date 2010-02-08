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

// GeoLoadStreetIntersectionSoundex.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <time.h>

#include "GeoLoadStreetIntersectionSoundex.h"

#include <fstream>
#include "BitStreamAdaptor.h"

namespace PortfolioExplorer {


	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadStreetIntersectionSoundex::Process()
	{
		numberOfOutputRecords = 0;


		// Get the first record from upstream first, to allow complex processing 
		// to complete before opening the output file.
		bool inputOK = m_readCSV.ReadRecord();

		if (!inputOK)
			throw TsString("No records to read");

		// Frequency-counting tables.
		FreqTable<int> stateFreqTable;
		FreqTable<int> soundex1FreqTable;
		FreqTable<int> streetNameID1FreqTable;
		FreqTable<int> streetSegmentOffset1FreqTable;
		FreqTable<int> soundex2FreqTable;
		FreqTable<int> streetNameID2FreqTable;
		FreqTable<int> streetSegmentOffset2FreqTable;

		// Some variables used in processing
		int tmpInt;
		unsigned int tmpUInt;

		int prevState;
		int prevSoundex1;
		int prevStreetNameID1;
		int prevSoundex2;
		int prevStreetNameID2;

		// Get local variable equivalents of bound fields.
		FieldAccessor stateValue = m_mapFieldAccessors["STATE"];
		FieldAccessor soundex1Value = m_mapFieldAccessors["SOUNDEX1"];
		FieldAccessor streetNameID1Value = m_mapFieldAccessors["STREET_NAME_ID1"];
		FieldAccessor streetSegmentOffset1Value = m_mapFieldAccessors["STREET_SEGMENT_OFFSET1"];
		FieldAccessor soundex2Value = m_mapFieldAccessors["SOUNDEX2"];
		FieldAccessor streetNameID2Value = m_mapFieldAccessors["STREET_NAME_ID2"];
		FieldAccessor streetSegmentOffset2Value = m_mapFieldAccessors["STREET_SEGMENT_OFFSET2"];

		// Read all records and analyze frequency counts for Huffman coding.
		// Buffer the records to a temp file so we can read them back later.
		{
			int chunkCount = 0;



			do {
				unsigned char tmpBuf[10];
				int varIntLength;
			
				// State
				tmpInt = stateValue.GetAsInt();
				if (chunkCount == 0) {
					// no freq count
				} else {
					int diff = tmpInt - prevState;
					varIntLength = IntToVarLengthBuf(diff, tmpBuf);
					AddArrayToFreqTable(stateFreqTable, tmpBuf, varIntLength);
				}

				// Soundex1
				tmpInt = PackSoundex(soundex1Value.GetAsString().c_str());
				if (chunkCount == 0) {
					// no freq count
				} else {
					int diff = tmpInt - prevSoundex1;
					varIntLength = IntToVarLengthBuf(diff, tmpBuf);
					AddArrayToFreqTable(soundex1FreqTable, tmpBuf, varIntLength);
				}

				// StreetNameID1
				tmpInt = streetNameID1Value.GetAsInt();
				if (chunkCount == 0) {
					// no freq count
				} else {
					int diff = tmpInt - prevStreetNameID1;
					varIntLength = IntToVarLengthBuf(diff, tmpBuf);
					AddArrayToFreqTable(streetNameID1FreqTable, tmpBuf, varIntLength);
				}

				// StreetSegmentOffset1
				tmpUInt = streetSegmentOffset1Value.GetAsInt();
				varIntLength = IntToVarLengthBuf(tmpUInt, tmpBuf);
				AddArrayToFreqTable(streetSegmentOffset1FreqTable, tmpBuf, varIntLength);

				// Soundex2
				tmpInt = PackSoundex(soundex2Value.GetAsString().c_str());
				if (chunkCount == 0) {
					// no freq count
				} else {
					int diff = tmpInt - prevSoundex2;
					varIntLength = IntToVarLengthBuf(diff, tmpBuf);
					AddArrayToFreqTable(soundex2FreqTable, tmpBuf, varIntLength);
				}

				// StreetNameID2
				tmpInt = streetNameID2Value.GetAsInt();
				if (chunkCount == 0) {
					// no freq count
				} else {
					int diff = tmpInt - prevStreetNameID2;
					varIntLength = IntToVarLengthBuf(diff, tmpBuf);
					AddArrayToFreqTable(streetNameID2FreqTable, tmpBuf, varIntLength);
				}

				// StreetSegmentOffset2
				tmpUInt = streetSegmentOffset2Value.GetAsInt();
				varIntLength = IntToVarLengthBuf(tmpUInt, tmpBuf);
				AddArrayToFreqTable(streetSegmentOffset2FreqTable, tmpBuf, varIntLength);

				// Save previous values
				prevState = stateValue.GetAsInt();
				prevSoundex1 = PackSoundex(soundex1Value.GetAsString().c_str());
				prevStreetNameID1 = streetNameID1Value.GetAsInt();
				prevSoundex2 = PackSoundex(soundex2Value.GetAsString().c_str());
				prevStreetNameID2 = streetNameID2Value.GetAsInt();

				chunkCount++;
				if (chunkCount == StreetIntersectionSoundexChunkSize) {
					chunkCount = 0;
				}
			} while (m_readCSV.ReadRecord());
		}

		// Open files
		File dataFile;
		TsString filename = outdir + "/" + STREET_INTERSECTION_SOUNDEX_FILE;
		if (!dataFile.Open(File::CreateAndWrite, filename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + filename + " for output"
			);
		}
		File positionIndexFile;
		filename = outdir + "/" + STREET_INTERSECTION_SOUNDEX_POSITION_INDEX_FILE;
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
		HuffmanCoder<int, std::less<int> > stateCoder;
		HuffmanCoder<int, std::less<int> > soundex1Coder;
		HuffmanCoder<int, std::less<int> > streetNameID1Coder;
		HuffmanCoder<int, std::less<int> > streetSegmentOffset1Coder;
		HuffmanCoder<int, std::less<int> > soundex2Coder;
		HuffmanCoder<int, std::less<int> > streetNameID2Coder;
		HuffmanCoder<int, std::less<int> > streetSegmentOffset2Coder;

		// Populate the huffman coder tables 
		stateCoder.AddEntries(stateFreqTable);
		soundex1Coder.AddEntries(soundex1FreqTable);
		streetNameID1Coder.AddEntries(streetNameID1FreqTable);
		streetSegmentOffset1Coder.AddEntries(streetSegmentOffset1FreqTable);
		soundex2Coder.AddEntries(soundex2FreqTable);
		streetNameID2Coder.AddEntries(streetNameID2FreqTable);
		streetSegmentOffset2Coder.AddEntries(streetSegmentOffset2FreqTable);

		// Generate Huffman codes
		stateCoder.MakeCodes();
		soundex1Coder.MakeCodes();
		streetNameID1Coder.MakeCodes();
		streetSegmentOffset1Coder.MakeCodes();
		soundex2Coder.MakeCodes();
		streetNameID2Coder.MakeCodes();
		streetSegmentOffset2Coder.MakeCodes();

		int chunkCount = 0;

		m_readCSV.ReOpen();
		m_readCSV.ReadRecord();


		while (m_readCSV.ReadRecord()) 
		{
			// Write position-index records.
			// Must be written before the data file, to get the correct bit offset.
			if (chunkCount == 0) {
				__int64 bitsWritten = dataBitStream.GetNumberOfBitsWritten();

				if (bitsWritten >= ((__int64)1 << StreetIntersectionPositionIndexBitSize)) {
					throw TsString("StreetIntersectionSoundex position index uses " + FormatInteger(StreetIntersectionPositionIndexBitSize) + " bits but must be larger");
				}

				if (StreetIntersectionPositionIndexBitSize > 32) {
					throw TsString("StreetIntersectionSoundex position index uses " + FormatInteger(StreetIntersectionPositionIndexBitSize) + " bits; must recode to use __int64 variables");
				}

				positionIndexBitStream.WriteBitsFromInt(StreetIntersectionPositionIndexBitSize, (int)bitsWritten);
			}

			// Write the fields of the data record.

			// state
			tmpInt = stateValue.GetAsInt();
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(StreetIntersectionStateBitSize, tmpInt);
			} else {
				CodeVarLengthIntToBitStream(
					tmpInt - prevState,
					stateCoder,
					dataBitStream
				);
			}

			// Soundex1
			tmpInt = PackSoundex(soundex1Value.GetAsString().c_str());
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(StreetIntersectionSoundexBitSize, tmpInt);
			} else {
				CodeVarLengthIntToBitStream(
					tmpInt - prevSoundex1,
					soundex1Coder,
					dataBitStream
				);
			}

			// StreetNameID1
			tmpInt = streetNameID1Value.GetAsInt();
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(StreetIntersectionStreetNameIDBitSize, tmpInt);
			} else {
				CodeVarLengthIntToBitStream(
					tmpInt - prevStreetNameID1,
					streetNameID1Coder,
					dataBitStream
				);
			}

			// StreetSegmentOffset1
			tmpUInt = streetSegmentOffset1Value.GetAsInt();
			CodeVarLengthIntToBitStream(
				tmpUInt,
				streetSegmentOffset1Coder,
				dataBitStream
			);

			// Soundex2
			tmpInt = PackSoundex(soundex2Value.GetAsString().c_str());
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(StreetIntersectionSoundexBitSize, tmpInt);
			} else {
				CodeVarLengthIntToBitStream(
					tmpInt - prevSoundex2,
					soundex2Coder,
					dataBitStream
				);
			}

			// StreetNameID2
			tmpInt = streetNameID2Value.GetAsInt();
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(StreetIntersectionStreetNameIDBitSize, tmpInt);
			} else {
				CodeVarLengthIntToBitStream(
					tmpInt - prevStreetNameID2,
					streetNameID2Coder,
					dataBitStream
				);
			}

			// StreetSegmentOffset2
			tmpUInt = streetSegmentOffset2Value.GetAsInt();
			CodeVarLengthIntToBitStream(
				tmpUInt,
				streetSegmentOffset2Coder,
				dataBitStream
			);

			// Save previous values
			prevState = stateValue.GetAsInt();
			prevSoundex1 = PackSoundex(soundex1Value.GetAsString().c_str());
			prevStreetNameID1 = streetNameID1Value.GetAsInt();
			prevSoundex2 = PackSoundex(soundex2Value.GetAsString().c_str());
			prevStreetNameID2 = streetNameID2Value.GetAsInt();

			chunkCount++;
			numberOfOutputRecords++;
			if (chunkCount == StreetIntersectionSoundexChunkSize) {
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
		struct FreqTableFiledef {
			FreqTableFiledef(
				FreqTable<int>& table_,
				const char* filename_
			) : table(table_), filename(filename_)
			{}
			FreqTable<int>& table;
			TsString filename;
		} freqTableFiledefs[] = {
			FreqTableFiledef(stateFreqTable, STREET_INTERSECTION_STATE_HUFF_FILE),
			FreqTableFiledef(soundex1FreqTable, STREET_INTERSECTION_SOUNDEX1_HUFF_FILE),
			FreqTableFiledef(streetNameID1FreqTable,STREET_INTERSECTION_STREET_NAME_ID1_HUFF_FILE),
			FreqTableFiledef(streetSegmentOffset1FreqTable, STREET_INTERSECTION_STREET_SEGMENT_OFFSET1_HUFF_FILE),
			FreqTableFiledef(soundex2FreqTable, STREET_INTERSECTION_SOUNDEX2_HUFF_FILE),
			FreqTableFiledef(streetNameID2FreqTable, STREET_INTERSECTION_STREET_NAME_ID2_HUFF_FILE),
			FreqTableFiledef(streetSegmentOffset2FreqTable, STREET_INTERSECTION_STREET_SEGMENT_OFFSET2_HUFF_FILE)
		};

		for (
			int freqTableIdx = 0; 
			freqTableIdx < sizeof(freqTableFiledefs)/sizeof(freqTableFiledefs[0]); 
			freqTableIdx++
		) {
			std::fstream fs;
			TsString filename = outdir + "/" + freqTableFiledefs[freqTableIdx].filename;
			fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
			if (fs.fail() || !freqTableFiledefs[freqTableIdx].table.Save(fs)) {
				throw TsString("Cannot write " + filename);
			}
			fs.close();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get a static array of FieldParameter entries, which will be used to
	// load up the fieldParameters vector.  The terminating element must
	// have an empty paramName.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	GeoLoadStreetIntersectionSoundex::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("STATE");
		retval.push_back("SOUNDEX1");
		retval.push_back("STREET_NAME_ID1");
		retval.push_back("STREET_SEGMENT_OFFSET1");
		retval.push_back("SOUNDEX2");
		retval.push_back("STREET_NAME_ID2");
		retval.push_back("STREET_SEGMENT_OFFSET2");

		return retval;
	};

} // namespace

