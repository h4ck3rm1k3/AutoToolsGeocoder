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

// GeoLoadCoordinate.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <time.h>

#include "GeoLoadCoordinate.h"

#include <fstream>
#include "BitStreamAdaptor.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadCoordinate::Process()
	{
		numberOfOutputRecords = 0;

		bool inputOK = m_readCSV.ReadRecord();

		if (!inputOK)
			throw TsString("No records to read");

		// Frequency-counting tables.
		FreqTable<int> latitudeFreqTable1;
		FreqTable<int> latitudeFreqTable2;
		FreqTable<int> longitudeFreqTable1;
		FreqTable<int> longitudeFreqTable2;


		// Used to convert lat/lon to integers.
		int const100000 = 100000;

		// Variables used inside loops
		int tmpLongitude;
		int tmpLatitude;
		int prevLongitude;
		int prevLatitude;
		unsigned char tmpBuf[100];
		int varIntLength;

		// Get local variable equivalents of bound fields.
		FieldAccessor coordinateIDValue = m_mapFieldAccessors["COORDINATE_ID"];
		FieldAccessor latitudeValue = m_mapFieldAccessors["LATITUDE"];
		FieldAccessor longitudeValue = m_mapFieldAccessors["LONGITUDE"];

		// Read all records and analyze frequency counts for Huffman coding.
		{
			int chunkCount = 0;

			do {
				tmpLatitude = int(latitudeValue.GetAsDouble() * const100000 + 0.5);
				tmpLongitude = int(longitudeValue.GetAsDouble() * const100000 + 0.5);

				if (chunkCount == 0) {
					// Key-record 
				} else {
					// Non-key-record
					varIntLength = IntToVarLengthBuf(tmpLatitude - prevLatitude, tmpBuf);
					latitudeFreqTable1.Count(tmpBuf[0]);
					{for (int i = 1; i < varIntLength; i++) {
						latitudeFreqTable2.Count(tmpBuf[i]);
					}}

					varIntLength = IntToVarLengthBuf(tmpLongitude - prevLongitude, tmpBuf);
					longitudeFreqTable1.Count(tmpBuf[0]);
					{for (int i = 1; i < varIntLength; i++) {
						longitudeFreqTable2.Count(tmpBuf[i]);
					}}
				}

				// Save previous values
				prevLatitude = tmpLatitude;
				prevLongitude = tmpLongitude;

				chunkCount++;
				if (chunkCount == CoordinateChunkSize) {
					chunkCount = 0;
				}
			} while (m_readCSV.ReadRecord());
		}


		// Open files
		File dataFile;
		TsString filename = outdir + "/" + COORDINATE_FILE;
		if (!dataFile.Open(File::CreateAndWrite, filename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + filename + " for output"
			);
		}
		File positionIndexFile;
		filename = outdir + "/" + COORDINATE_POSITION_INDEX_FILE;
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
		HuffmanCoder<int, std::less<int> > latitudeCoder1;
		HuffmanCoder<int, std::less<int> > latitudeCoder2;
		HuffmanCoder<int, std::less<int> > longitudeCoder1;
		HuffmanCoder<int, std::less<int> > longitudeCoder2;

		// Populate the huffman coder tables 
		latitudeCoder1.AddEntries(latitudeFreqTable1);
		latitudeCoder2.AddEntries(latitudeFreqTable2);
		longitudeCoder1.AddEntries(longitudeFreqTable1);
		longitudeCoder2.AddEntries(longitudeFreqTable2);

		// Generate Huffman codes
		latitudeCoder1.MakeCodes();
		latitudeCoder2.MakeCodes();
		longitudeCoder1.MakeCodes();
		longitudeCoder2.MakeCodes();

		int chunkCount = 0;

		m_readCSV.ReOpen();
		m_readCSV.ReadRecord();


		while (m_readCSV.ReadRecord()) 
		{


			// Write position-index records.
			// Must be written before the data file, to get the correct bit offset.
			if (chunkCount == 0) {
				__int64 bitsWritten = dataBitStream.GetNumberOfBitsWritten();

				if (bitsWritten >= ((__int64)1 << CoordinatePositionIndexBitSize)) {
					throw TsString("Coordinate position index uses " + FormatInteger(CoordinatePositionIndexBitSize) + " bits but must be larger");
				}

				if (CoordinatePositionIndexBitSize > 32) {
					throw TsString("Coordinate position index uses " + FormatInteger(CoordinatePositionIndexBitSize) + " bits; must recode to use __int64 variables");
				}

				positionIndexBitStream.WriteBitsFromInt(CoordinatePositionIndexBitSize, (int)bitsWritten);
			}


			// Check validity of data
			int coordinateID = (int)coordinateIDValue.GetAsInt();
			
			if (coordinateID != numberOfOutputRecords) {
				throw TsString("Coordinate ID is out of sequence");
			}

			// Write the fields of the data record.

			tmpLatitude = int(latitudeValue.GetAsDouble() * const100000 + 0.5);
			tmpLongitude = int(longitudeValue.GetAsDouble() * const100000 + 0.5);

			// Latitude
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(CoordinateLatitudeBitSize, tmpLatitude);
			} else {
				varIntLength = IntToVarLengthBuf(tmpLatitude - prevLatitude, tmpBuf);
				if (!latitudeCoder1.WriteCode(tmpBuf[0], dataBitStream)) {
					throw TsString("Error in huffman code table");
				}
				for (int i = 1; i < varIntLength; i++) {
					if (!latitudeCoder2.WriteCode(tmpBuf[i], dataBitStream)) {
						throw TsString("Error in huffman code table");
					}
				}
			}

			// Latitude
			if (chunkCount == 0) {
				dataBitStream.WriteBitsFromInt(CoordinateLongitudeBitSize, tmpLongitude);
			} else {
				varIntLength = IntToVarLengthBuf(tmpLongitude - prevLongitude, tmpBuf);
				if (!longitudeCoder1.WriteCode(tmpBuf[0], dataBitStream)) {
					throw TsString("Error in huffman code table");
				}
				for (int i = 1; i < varIntLength; i++) {
					if (!longitudeCoder2.WriteCode(tmpBuf[i], dataBitStream)) {
						throw TsString("Error in huffman code table");
					}
				}
			}

			// Save previous values
			prevLatitude = tmpLatitude;
			prevLongitude = tmpLongitude;

			chunkCount++;
			numberOfOutputRecords++;
			if (chunkCount == CoordinateChunkSize) {
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
				TsString filename_
			) : table(table_), filename(filename_)
			{}
			FreqTable<int>& table;
			TsString filename;
		} freqTableFiledefs[] = {
			FreqTableFiledef(latitudeFreqTable1, COORDINATE_LATITUDE_HUFF_FILE1),
			FreqTableFiledef(latitudeFreqTable2, COORDINATE_LATITUDE_HUFF_FILE2),
			FreqTableFiledef(longitudeFreqTable1, COORDINATE_LONGITUDE_HUFF_FILE1),
			FreqTableFiledef(longitudeFreqTable2, COORDINATE_LONGITUDE_HUFF_FILE2),
		};

		for (
			int freqTableIdx = 0; 
			freqTableIdx < sizeof(freqTableFiledefs)/sizeof(freqTableFiledefs[0]); 
			freqTableIdx++
		) {
			std::fstream fs;
			filename = outdir + "/" + freqTableFiledefs[freqTableIdx].filename;
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
	GeoLoadCoordinate::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("COORDINATE_ID");
		retval.push_back("LATITUDE");
		retval.push_back("LONGITUDE");

		return retval;
	};


} // namespace

