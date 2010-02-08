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

// GeoLoadStreetSegment.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <time.h>

#include "GeoLoadStreetSegment.h"

#include <fstream>
#include "BitStreamAdaptor.h"
namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Local utility: add the elements of a vector to a fequency table
	///////////////////////////////////////////////////////////////////////////////

	// Add a vector to frequency-count tables.  The first table will count
	// the sizes, the second for the bytes
	inline static void AddVecToFreqTable(
		FreqTable<int>& tableSize,
		FreqTable<int>& tableBytes,
		const std::vector<int>& vec
	) {
		for (unsigned i = 0; i < vec.size(); i++) {
			tableBytes.Count(vec[i]);
		}
		// Add count because that will be writen as the leading byte
		tableSize.Count(int(vec.size()));
	}

	// Single table version of above.
	inline static void AddVecToFreqTable(
		FreqTable<int>& table,
		const std::vector<int>& vec
	) {
		AddVecToFreqTable(table, table, vec);
	}

	// Code a vector to a bitstream using a separate coder for the
	// leading count and the bytes
	inline static void CodeVecToBitStream(
		BitStreamWrite& bitStream,
		HuffmanCoder<int, std::less<int> >& coderSize,
		HuffmanCoder<int, std::less<int> >& coderBytes,
		const std::vector<int>& vec
	) {
		// Write leading length value.
		if (!coderSize.WriteCode(int(vec.size()), bitStream)) {
			throw TsString("Error in Huffman code table");
		}
		// Write elements of the vector
		for (unsigned i = 0; i < vec.size(); i++) {
			if (!coderBytes.WriteCode(vec[i], bitStream)) {
				throw TsString("Error in Huffman code table");
			}
		}
	}
	// single-coder version of the above
	inline static void CodeVecToBitStream(
		BitStreamWrite& bitStream,
		HuffmanCoder<int, std::less<int> >& coder,
		const std::vector<int>& vec
	) {
		CodeVecToBitStream(bitStream, coder, coder, vec);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadStreetSegment::Process()
	{
		numberOfOutputRecords = 0;


		// Get the first record from upstream first, to allow complex processing 
		// to complete before opening the output file.
		bool inputOK = m_readCSV.ReadRecord();
		if (!inputOK)
			throw TsString("No records to read");


		// Frequency-counting tables.
		FreqTable<int> addrLowKeyFreqTable1;
		FreqTable<int> addrLowKeyFreqTable2;
		FreqTable<int> addrLowNonkeyFreqTable1;
		FreqTable<int> addrLowNonkeyFreqTable2;
		FreqTable<int> addrHighFreqTable1;
		FreqTable<int> addrHighFreqTable2;
		FreqTable<int> countyKeyFreqTable;
		FreqTable<int> countyNonkeyFreqTable;
		FreqTable<int> censusTractKeyFreqTable1;
		FreqTable<int> censusTractKeyFreqTable2;
		FreqTable<int> censusTractNonkeyFreqTable1;
		FreqTable<int> censusTractNonkeyFreqTable2;
		FreqTable<int> censusBlockKeyFreqTable1;
		FreqTable<int> censusBlockKeyFreqTable2;
		FreqTable<int> censusBlockNonkeyFreqTable1;
		FreqTable<int> censusBlockNonkeyFreqTable2;
		FreqTable<int> postcodeExtKeyFreqTable;
		FreqTable<int> postcodeExtNonkeyFreqTable;
		FreqTable<int> coordinateIDFreqTable1;
		FreqTable<int> coordinateIDFreqTable2;
		FreqTable<int> coordinateCountFreqTable;


		// Some temp variables
		TsString tmpStr;
		TsString tmpStr2;
		std::vector<int> tmpVec;
		std::vector<int> tmpVec2;
		TsString tmpAddrLow;
		TsString prevAddrLow;
		int prevCounty;
		TsString prevCensusTract;
		TsString prevCensusBlock;
		TsString prevPostcodeExt;
		int prevCoordinateID;
		int tmpInt;

		// Get local variable equivalents of bound fields.
		FieldAccessor streetSegmentIDValue = m_mapFieldAccessors["STREET_SEGMENT_ID"];
		FieldAccessor addrLowValue = m_mapFieldAccessors["ADDR_LOW"];
		FieldAccessor addrHighValue = m_mapFieldAccessors["ADDR_HIGH"];
		FieldAccessor leftRightValue = m_mapFieldAccessors["LEFT_RIGHT"];
		FieldAccessor countyValue = m_mapFieldAccessors["COUNTY"];
		FieldAccessor censusTractValue = m_mapFieldAccessors["CENSUS_TRACT"];
		FieldAccessor censusBlockValue = m_mapFieldAccessors["CENSUS_BLOCK"];
		FieldAccessor postcodeExtValue = m_mapFieldAccessors["POSTCODE_EXT"];
		FieldAccessor coordinateIDValue = m_mapFieldAccessors["COORDINATE_ID"];
		FieldAccessor coordinateCountValue = m_mapFieldAccessors["COORDINATE_COUNT"];

		// Read all records and analyze frequency counts for Huffman coding.
		// Buffer the records to a temp file so we can read them back later.
		{
			int chunkCount = 0;

			// Put outside loop to avoid repeated construction.
			int recordID = 0;

			do {
				unsigned char tmpBuf[100];
				int varIntLength;
			

				if (recordID != (int)streetSegmentIDValue.GetAsInt())
					throw TsString("LoadStreetSegment: Record position " + FormatInteger(recordID) + " has ID of " + FormatInteger((int)streetSegmentIDValue.GetAsInt()));

				// Determine if the value for CoordinateCount should be skipped.
				// This is done when (a) it is a non-key record and (b) this
				// value for coordinateID is the same as the previous value.
				// When this happens the CoordinateCount is also the same as the
				// previous value and can be skipped.
				bool skipCoordinateCount = false;

				if (chunkCount == 0) {
					// Key-record-only freq counts

					// AddrLow
					tmpStr = addrLowValue.GetAsString();
					RLECompressStrToVec(tmpVec, tmpStr, '0');
					AddVecToFreqTable(addrLowKeyFreqTable1, addrLowKeyFreqTable2, tmpVec);
					tmpAddrLow = tmpStr;

					// County
					prevCounty = countyValue.GetAsInt();
					countyKeyFreqTable.Count(prevCounty);

					// Census tract
					tmpStr = censusTractValue.GetAsString();
					prevCensusTract = tmpStr;
					RLECompressStrToVec(tmpVec, tmpStr, '0');
					AddVecToFreqTable(censusTractKeyFreqTable1, censusTractKeyFreqTable2, tmpVec);

					// Census block
					tmpStr = censusBlockValue.GetAsString();
					prevCensusBlock = tmpStr;
					RLECompressStrToVec(tmpVec, tmpStr, '0');
					AddVecToFreqTable(censusBlockKeyFreqTable1, censusBlockKeyFreqTable2, tmpVec);

					// Postal code extension
					tmpStr = postcodeExtValue.GetAsString();
					prevPostcodeExt = tmpStr;
					RLECompressStrToVec(tmpVec, tmpStr, '0');
					AddVecToFreqTable(postcodeExtKeyFreqTable, tmpVec);
				} else {
					// Non-key-record freq counts

					// AddrLow
					tmpStr = addrLowValue.GetAsString();
					StrToVec(tmpVec, tmpStr, '0');
					StrToVec(tmpVec2, prevAddrLow, '0');
					VecDiff(tmpVec, tmpVec2);
					RLECompressVec(tmpVec);
					AddVecToFreqTable(addrLowNonkeyFreqTable1, addrLowNonkeyFreqTable2, tmpVec);
					tmpAddrLow = tmpStr;

					// County
					tmpInt = countyValue.GetAsInt();
					countyNonkeyFreqTable.Count(tmpInt - prevCounty);

					// Census tract
					tmpStr = censusTractValue.GetAsString();
					StrToVec(tmpVec, tmpStr, '0');
					StrToVec(tmpVec2, prevCensusTract, '0');
					VecDiff(tmpVec, tmpVec2);
					RLECompressVec(tmpVec);
					AddVecToFreqTable(censusTractNonkeyFreqTable1, censusTractNonkeyFreqTable2, tmpVec);
					
					// Census block
					tmpStr = censusBlockValue.GetAsString();
					StrToVec(tmpVec, tmpStr, '0');
					StrToVec(tmpVec2, prevCensusBlock, '0');
					VecDiff(tmpVec, tmpVec2);
					RLECompressVec(tmpVec);
					AddVecToFreqTable(censusBlockNonkeyFreqTable1, censusBlockNonkeyFreqTable2, tmpVec);

					// Postal code extension
					tmpStr = postcodeExtValue.GetAsString();
					StrToVec(tmpVec, tmpStr, '0');
					StrToVec(tmpVec2, prevPostcodeExt, '0');
					VecDiff(tmpVec, tmpVec2);
					RLECompressVec(tmpVec);
					AddVecToFreqTable(postcodeExtNonkeyFreqTable, tmpVec);

					// Coordinate ID
					if (tmpInt == prevCoordinateID) {
						skipCoordinateCount = true;
					}
					tmpInt = coordinateIDValue.GetAsInt();
					varIntLength = IntToVarLengthBuf(tmpInt - prevCoordinateID, tmpBuf);
					coordinateIDFreqTable1.Count(tmpBuf[0]);
					{for (int i = 1; i < varIntLength; i++) {
						coordinateIDFreqTable2.Count(tmpBuf[i]);
					}}

				}

				// Freq counts for key and non-key records

				if (!skipCoordinateCount) {
					// Coordinate Count
					tmpInt = coordinateCountValue.GetAsInt();
					coordinateCountFreqTable.Count(tmpInt);
				}

				// AddrHigh
				tmpStr = addrHighValue.GetAsString();
				StrToVec(tmpVec, tmpStr, '0');
				StrToVec(tmpVec2, addrLowValue.GetAsString(), '0');
				VecDiff(tmpVec, tmpVec2);
				RLECompressVec(tmpVec);
				AddVecToFreqTable(addrHighFreqTable1, addrHighFreqTable2, tmpVec);

				// Get previous values
				prevCoordinateID = coordinateIDValue.GetAsInt();
				prevCounty = countyValue.GetAsInt();
				prevAddrLow = addrLowValue.GetAsString();
				prevPostcodeExt = postcodeExtValue.GetAsString();
				prevCensusBlock = censusBlockValue.GetAsString();
				prevCensusTract = censusTractValue.GetAsString();

				chunkCount++;
				if (chunkCount == StreetSegmentChunkSize) {
					chunkCount = 0;
				}
				recordID++;
			} while (m_readCSV.ReadRecord());
		}

		// Open files
		File dataFile;
		TsString filename = outdir + "/" + STREET_SEGMENT_FILE;
		if (!dataFile.Open(File::CreateAndWrite, filename, FileBufferSize)) 
			throw TsString("Cannot open file " + filename + " for output");

		File positionIndexFile;
		filename = outdir + "/" + STREET_SEGMENT_POSITION_INDEX_FILE;
		if (!positionIndexFile.Open(File::CreateAndWrite, filename, FileBufferSize))
			throw TsString("Cannot open file " + filename + " for output");

		// Create an output BitStream for the data file
		BitStreamWrite dataBitStream(new FileBitStreamAdaptor(dataFile));

		// Create an output BitStream for the position-index file
		BitStreamWrite positionIndexBitStream(new FileBitStreamAdaptor(positionIndexFile));

		// Set up Huffman coding.
		HuffmanCoder<int, std::less<int> > addrLowKeyCoder1;
		HuffmanCoder<int, std::less<int> > addrLowKeyCoder2;
		HuffmanCoder<int, std::less<int> > addrLowNonkeyCoder1;
		HuffmanCoder<int, std::less<int> > addrLowNonkeyCoder2;
		HuffmanCoder<int, std::less<int> > addrHighCoder1;
		HuffmanCoder<int, std::less<int> > addrHighCoder2;
		HuffmanCoder<int, std::less<int> > countyKeyCoder;
		HuffmanCoder<int, std::less<int> > countyNonkeyCoder;
		HuffmanCoder<int, std::less<int> > censusTractKeyCoder1;
		HuffmanCoder<int, std::less<int> > censusTractKeyCoder2;
		HuffmanCoder<int, std::less<int> > censusTractNonkeyCoder1;
		HuffmanCoder<int, std::less<int> > censusTractNonkeyCoder2;
		HuffmanCoder<int, std::less<int> > censusBlockKeyCoder1;
		HuffmanCoder<int, std::less<int> > censusBlockKeyCoder2;
		HuffmanCoder<int, std::less<int> > censusBlockNonkeyCoder1;
		HuffmanCoder<int, std::less<int> > censusBlockNonkeyCoder2;
		HuffmanCoder<int, std::less<int> > postcodeExtKeyCoder;
		HuffmanCoder<int, std::less<int> > postcodeExtNonkeyCoder;
		HuffmanCoder<int, std::less<int> > coordinateIDCoder1;
		HuffmanCoder<int, std::less<int> > coordinateIDCoder2;
		HuffmanCoder<int, std::less<int> > coordinateCountCoder;

		// Populate the huffman coder tables 
		addrLowKeyCoder1.AddEntries(addrLowKeyFreqTable1);
		addrLowKeyCoder2.AddEntries(addrLowKeyFreqTable2);
		addrLowNonkeyCoder1.AddEntries(addrLowNonkeyFreqTable1);
		addrLowNonkeyCoder2.AddEntries(addrLowNonkeyFreqTable2);
		addrHighCoder1.AddEntries(addrHighFreqTable1);
		addrHighCoder2.AddEntries(addrHighFreqTable2);
		countyKeyCoder.AddEntries(countyKeyFreqTable);
		countyNonkeyCoder.AddEntries(countyNonkeyFreqTable);
		censusTractKeyCoder1.AddEntries(censusTractKeyFreqTable1);
		censusTractKeyCoder2.AddEntries(censusTractKeyFreqTable2);
		censusTractNonkeyCoder1.AddEntries(censusTractNonkeyFreqTable1);
		censusTractNonkeyCoder2.AddEntries(censusTractNonkeyFreqTable2);
		censusBlockKeyCoder1.AddEntries(censusBlockKeyFreqTable1);
		censusBlockKeyCoder2.AddEntries(censusBlockKeyFreqTable2);
		censusBlockNonkeyCoder1.AddEntries(censusBlockNonkeyFreqTable1);
		censusBlockNonkeyCoder2.AddEntries(censusBlockNonkeyFreqTable2);
		postcodeExtKeyCoder.AddEntries(postcodeExtKeyFreqTable);
		postcodeExtNonkeyCoder.AddEntries(postcodeExtNonkeyFreqTable);
		coordinateIDCoder1.AddEntries(coordinateIDFreqTable1);
		coordinateIDCoder2.AddEntries(coordinateIDFreqTable2);
		coordinateCountCoder.AddEntries(coordinateCountFreqTable);

		// Generate Huffman codes
		addrLowKeyCoder1.MakeCodes();
		addrLowKeyCoder2.MakeCodes();
		addrLowNonkeyCoder1.MakeCodes();
		addrLowNonkeyCoder2.MakeCodes();
		addrHighCoder1.MakeCodes();
		addrHighCoder2.MakeCodes();
		countyKeyCoder.MakeCodes();
		countyNonkeyCoder.MakeCodes();
		censusTractKeyCoder1.MakeCodes();
		censusTractKeyCoder2.MakeCodes();
		censusTractNonkeyCoder1.MakeCodes();
		censusTractNonkeyCoder2.MakeCodes();
		censusBlockKeyCoder1.MakeCodes();
		censusBlockKeyCoder2.MakeCodes();
		censusBlockNonkeyCoder1.MakeCodes();
		censusBlockNonkeyCoder2.MakeCodes();
		postcodeExtKeyCoder.MakeCodes();
		postcodeExtNonkeyCoder.MakeCodes();
		coordinateIDCoder1.MakeCodes();
		coordinateIDCoder2.MakeCodes();
		coordinateCountCoder.MakeCodes();

		int chunkCount = 0;

		m_readCSV.ReOpen();
		m_readCSV.ReadRecord();

		// Put outside loop to avoid repeated construction.
		int varIntLength;
		unsigned char tmpBuf[100];
		
		while (m_readCSV.ReadRecord()) 
		{

			if ((int)streetSegmentIDValue.GetAsInt() != numberOfOutputRecords) {
				throw TsString("Street Segment ID is out of sequence");
			}

			// Write position-index records.
			// Must be written before the data file, to get the correct bit offset.
			if (chunkCount == 0) {
				__int64 bitsWritten = dataBitStream.GetNumberOfBitsWritten();

				if (bitsWritten >= ((__int64)1 << StreetSegmentPositionIndexBitSize))
					throw TsString("StreetSegment position index uses " + FormatInteger(StreetSegmentPositionIndexBitSize) + " bits but must be larger");

				if (StreetSegmentPositionIndexBitSize > 32)
					throw TsString("StreetSegment position index uses " + FormatInteger(StreetSegmentPositionIndexBitSize) + " bits; must recode to use __int64 variables");

				positionIndexBitStream.WriteBitsFromInt(StreetSegmentPositionIndexBitSize, (int)bitsWritten);
			}

			// Write the fields of the data record.

			// AddrLow
			tmpStr = addrLowValue.GetAsString();
			if (chunkCount == 0) {
				RLECompressStrToVec(tmpVec, tmpStr, '0');
				CodeVecToBitStream(dataBitStream, addrLowKeyCoder1, addrLowKeyCoder2, tmpVec);
			} else {
				StrToVec(tmpVec, tmpStr, '0');
				StrToVec(tmpVec2, prevAddrLow, '0');
				VecDiff(tmpVec, tmpVec2);
				RLECompressVec(tmpVec);
				CodeVecToBitStream(dataBitStream, addrLowNonkeyCoder1, addrLowNonkeyCoder2, tmpVec);
			}

			// AddrHigh
			tmpAddrLow = tmpStr;
			tmpStr = addrHighValue.GetAsString();
			StrToVec(tmpVec, tmpStr, '0');
			StrToVec(tmpVec2, tmpAddrLow, '0');
			VecDiff(tmpVec, tmpVec2);
			RLECompressVec(tmpVec);
			CodeVecToBitStream(dataBitStream, addrHighCoder1, addrHighCoder2, tmpVec);

			// LeftRight
			tmpInt = (bool)leftRightValue.GetAsBoolean();
			dataBitStream.WriteBitsFromInt(StreetSegmentLeftRightBitSize, tmpInt);

			// County
			tmpInt = countyValue.GetAsInt();
			if (chunkCount == 0) {
				if (!countyKeyCoder.WriteCode(tmpInt, dataBitStream)) {
					throw TsString("Error in huffman code table");
				}
			} else {
				if (!countyNonkeyCoder.WriteCode(tmpInt - prevCounty, dataBitStream)) {
					throw TsString("Error in huffman code table");
				}
			}

			// Census tract
			tmpStr = censusTractValue.GetAsString();
			if (chunkCount == 0) {
				RLECompressStrToVec(tmpVec, tmpStr, '0');
				CodeVecToBitStream(dataBitStream, censusTractKeyCoder1, censusTractKeyCoder2, tmpVec);
			} else {
				StrToVec(tmpVec, tmpStr, '0');
				StrToVec(tmpVec2, prevCensusTract, '0');
				VecDiff(tmpVec, tmpVec2);
				RLECompressVec(tmpVec);
				CodeVecToBitStream(dataBitStream, censusTractNonkeyCoder1, censusTractNonkeyCoder2, tmpVec);
			}

			// Census block
			tmpStr = censusBlockValue.GetAsString();
			if (chunkCount == 0) {
				RLECompressStrToVec(tmpVec, tmpStr, '0');
				CodeVecToBitStream(dataBitStream, censusBlockKeyCoder1, censusBlockKeyCoder2, tmpVec);
			} else {
				StrToVec(tmpVec, tmpStr, '0');
				StrToVec(tmpVec2, prevCensusBlock, '0');
				VecDiff(tmpVec, tmpVec2);
				RLECompressVec(tmpVec);
				CodeVecToBitStream(dataBitStream, censusBlockNonkeyCoder1, censusBlockNonkeyCoder2, tmpVec);
			}

			// Postal code extensions
			tmpStr = postcodeExtValue.GetAsString();
			if (chunkCount == 0) {
				RLECompressStrToVec(tmpVec, tmpStr, '0');
				CodeVecToBitStream(dataBitStream, postcodeExtKeyCoder, tmpVec);
			} else {
				StrToVec(tmpVec, tmpStr, '0');
				StrToVec(tmpVec2, prevPostcodeExt, '0');
				VecDiff(tmpVec, tmpVec2);
				RLECompressVec(tmpVec);
				CodeVecToBitStream(dataBitStream, postcodeExtNonkeyCoder, tmpVec);
			}

			// Coordinate ID... must be signed int.
			bool skipCoordinateCount = false;
			tmpInt = coordinateIDValue.GetAsInt();
			if (chunkCount == 0) {
				if (tmpInt >= (1 << StreetSegmentCoordinateIDBitSize)) 
					// Does not fit
					throw TsString("StreetSegmentCoordinateIDBitSize is " + FormatInteger(StreetSegmentCoordinateIDBitSize) + " bits, but is not large enough to hold StreetSegmentCoordinateID value");
				dataBitStream.WriteBitsFromInt(StreetSegmentCoordinateIDBitSize, tmpInt);
			} else {
				if (tmpInt == prevCoordinateID) {
					// Don't need to write coordinate count when ID is the same
					skipCoordinateCount = true;
				}
				varIntLength = IntToVarLengthBuf(tmpInt - prevCoordinateID, tmpBuf);
				if (!coordinateIDCoder1.WriteCode(tmpBuf[0], dataBitStream)) {
					throw TsString("Error in huffman code table");
				}
				for (int i = 1; i < varIntLength; i++) {
					if (!coordinateIDCoder2.WriteCode(tmpBuf[i], dataBitStream)) {
						throw TsString("Error in huffman code table");
					}
				}
			}

			// Coordinate Count
			if (!skipCoordinateCount) {
				tmpInt = coordinateCountValue.GetAsInt();
				if (!coordinateCountCoder.WriteCode(tmpInt, dataBitStream)) {
					throw TsString("Error in huffman code table");
				}
			}


			// Get previous values
			prevCoordinateID = coordinateIDValue.GetAsInt();
			prevCounty = countyValue.GetAsInt();
			prevAddrLow = addrLowValue.GetAsString();
			prevPostcodeExt = postcodeExtValue.GetAsString();
			prevCensusBlock = censusBlockValue.GetAsString();
			prevCensusTract = censusTractValue.GetAsString();

			chunkCount++;
			numberOfOutputRecords++;
			if (chunkCount == StreetSegmentChunkSize) {
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
			FreqTableFiledef(addrLowKeyFreqTable1, STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE1),
			FreqTableFiledef(addrLowKeyFreqTable2, STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE2),
			FreqTableFiledef(addrLowNonkeyFreqTable1,STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE1),
			FreqTableFiledef(addrLowNonkeyFreqTable2,STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE2),
			FreqTableFiledef(addrHighFreqTable1, STREET_SEGMENT_ADDR_HIGH_HUFF_FILE1),
			FreqTableFiledef(addrHighFreqTable2, STREET_SEGMENT_ADDR_HIGH_HUFF_FILE2),
			FreqTableFiledef(countyKeyFreqTable, STREET_SEGMENT_COUNTY_KEY_HUFF_FILE),
			FreqTableFiledef(countyNonkeyFreqTable, STREET_SEGMENT_COUNTY_NONKEY_HUFF_FILE),
			FreqTableFiledef(censusTractKeyFreqTable1, STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE1),
			FreqTableFiledef(censusTractKeyFreqTable2, STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE2),
			FreqTableFiledef(censusTractNonkeyFreqTable1, STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE1),
			FreqTableFiledef(censusTractNonkeyFreqTable2, STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE2),
			FreqTableFiledef(censusBlockKeyFreqTable1, STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE1),
			FreqTableFiledef(censusBlockKeyFreqTable2, STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE2),
			FreqTableFiledef(censusBlockNonkeyFreqTable1, STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE1),
			FreqTableFiledef(censusBlockNonkeyFreqTable2, STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE2),
			FreqTableFiledef(postcodeExtKeyFreqTable, STREET_SEGMENT_POSTCODE_EXT_KEY_HUFF_FILE),
			FreqTableFiledef(postcodeExtNonkeyFreqTable, STREET_SEGMENT_POSTCODE_EXT_NONKEY_HUFF_FILE),
			FreqTableFiledef(coordinateIDFreqTable1, STREET_SEGMENT_COORDINATE_ID_HUFF_FILE1),
			FreqTableFiledef(coordinateIDFreqTable2, STREET_SEGMENT_COORDINATE_ID_HUFF_FILE2),
			FreqTableFiledef(coordinateCountFreqTable, STREET_SEGMENT_COORDINATE_COUNT_HUFF_FILE),
		};

		for (
			int freqTableIdx = 0; 
			freqTableIdx < sizeof(freqTableFiledefs)/sizeof(freqTableFiledefs[0]); 
			freqTableIdx++
		) {
			std::fstream fs;
			filename = outdir + "/" + freqTableFiledefs[freqTableIdx].filename;
			fs.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
			if (fs.fail() || !freqTableFiledefs[freqTableIdx].table.Save(fs))
				throw TsString("Cannot write " + filename);
			fs.close();
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// Get a static array of FieldParameter entries, which will be used to
	// load up the fieldParameters vector.  The terminating element must
	// have an empty paramName.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	GeoLoadStreetSegment::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("STREET_SEGMENT_ID");
		retval.push_back("ADDR_LOW");
		retval.push_back("ADDR_HIGH");
		retval.push_back("LEFT_RIGHT");
		retval.push_back("COUNTY");
		retval.push_back("CENSUS_TRACT");
		retval.push_back("CENSUS_BLOCK");
		retval.push_back("POSTCODE_EXT");
		retval.push_back("COORDINATE_ID");
		retval.push_back("COORDINATE_COUNT");

		return retval;
	};


} // namespace

