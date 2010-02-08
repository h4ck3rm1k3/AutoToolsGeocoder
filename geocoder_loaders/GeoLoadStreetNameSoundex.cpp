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
# $Rev: 39 $ 
# $Date: 2006-08-02 18:15:24 +0200 (Wed, 02 Aug 2006) $ 
*/

// GeoLoadStreetNameSoundex.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <time.h>

#include "GeoLoadStreetNameSoundex.h"
#include "BitStreamAdaptor.h"

namespace PortfolioExplorer {


	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadStreetNameSoundex::Process()
	{
		// Get the first record from upstream first, to allow complex processing 
		// to complete before opening the output file.
		bool inputOK = m_readCSV.ReadRecord();
		if (!inputOK)
			throw TsString("No records to read");

		// Open file
		File file;
		TsString dataFileName = outdir + "/" + STREET_NAME_SOUNDEX_FILE;
		if (!file.Open(File::CreateAndWrite, dataFileName, FileBufferSize))
			throw TsString("Cannot open file " + dataFileName + " for output");

		// Create an output BitStream for the data file
		BitStreamWrite dataBitStream(new FileBitStreamAdaptor(file));

		// Bind local variables
		FieldAccessor streetIndexIDValue = m_mapFieldAccessors["STREET_INDEX_ID"];
		FieldAccessor financeNumberValue = m_mapFieldAccessors["FINANCE_NUMBER"];
		FieldAccessor streetSoundexValue = m_mapFieldAccessors["STREET_SOUNDEX"];
		FieldAccessor streetNameIDValue = m_mapFieldAccessors["STREET_NAME_ID"];

		numberOfOutputRecords = 0;

		do {
			assert((int)streetIndexIDValue.GetAsInt() == numberOfOutputRecords);

			// Write the next output record.

			// finance "number"
			unsigned int packedFa = PackFa(financeNumberValue.GetAsString().c_str());
			dataBitStream.WriteBitsFromInt(StreetNameSoundexFaBitSize, packedFa);

			// Street Name Soundex
			// Soundex is compacted into a 16 bit integer.
			int soundexValue = PackSoundex(streetSoundexValue.GetAsString().c_str());
			dataBitStream.WriteBitsFromInt(StreetNameSoundexBitSize, soundexValue);

			// Check that StreetNameID fits in allocated bits.
			int streetNameID = streetNameIDValue.GetAsInt();
			if (streetNameID >= (1 << StreetNameSoundexStreetNameIDBitSize)) {
				// Does not fit
				throw TsString(
					"StreetNameSoundexStreetNameIDBitSize is " + FormatInteger(StreetNameSoundexStreetNameIDBitSize) + " bits, but is not large enough to hold StreetName ID value"
				);
			}

			// StreetNameID
			dataBitStream.WriteBitsFromInt(
				StreetNameSoundexStreetNameIDBitSize,
				streetNameID
			);

			numberOfOutputRecords++;
		} while (m_readCSV.ReadRecord());

		dataBitStream.Flush();
	}



	///////////////////////////////////////////////////////////////////////////////
	// Get a static array of FieldParameter entries, which will be used to
	// load up the fieldParameters vector.  The terminating element must
	// have an empty paramName.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	GeoLoadStreetNameSoundex::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("STREET_INDEX_ID");
		retval.push_back("FINANCE_NUMBER");
		retval.push_back("STREET_SOUNDEX");
		retval.push_back("STREET_NAME_ID");

		return retval;
	};

} // namespace
