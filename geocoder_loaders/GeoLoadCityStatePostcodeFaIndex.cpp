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

// GeoLoadCityStatePostcodeFaIndex.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <time.h>

#include "GeoLoadCityStatePostcodeFaIndex.h"
#include "BitStreamAdaptor.h"

namespace PortfolioExplorer {


	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadCityStatePostcodeFaIndex::Process()
	{
		bool inputOK = m_readCSV.ReadRecord();
		if (!inputOK)
			throw TsString("No records to read");


		// Open file
		File file;
		if (!file.Open(File::CreateAndWrite, outdir + "/" + CITY_STATE_POSTCODE_FA_INDEX_FILE, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + outdir + "/" + CITY_STATE_POSTCODE_FA_INDEX_FILE + " for output"
			);
		}


		// Bind local variables
		FieldAccessor financeValue = m_mapFieldAccessors["FINANCE"];
		FieldAccessor cityStatePostcodeIDValue = m_mapFieldAccessors["CITY_STATE_POSTCODE_ID"];

		numberOfOutputRecords = 0;

		do {

			// Write the next output record.

			// finance "number"
			{
				char buf[GeoUtil::CityStatePostcodeFaIndexFinanceFieldLength];
				memset(buf, 0, sizeof(buf));
				TsString strFinance = financeValue.GetAsString();
				memcpy(buf, strFinance.c_str(), JHMIN(unsigned(strFinance.length()), sizeof(buf)) );
				file.Write(sizeof(buf), buf);
			}

			// CityStatePostcodeID
			WriteThreeByteInt(file, (int)cityStatePostcodeIDValue.GetAsInt());

			numberOfOutputRecords++;

		} while (m_readCSV.ReadRecord());

		file.Close();

	}


	///////////////////////////////////////////////////////////////////////////////
	// Get a static array of FieldParameter entries, which will be used to
	// load up the fieldParameters vector.  The terminating element must
	// have an empty paramName.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	GeoLoadCityStatePostcodeFaIndex::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("FINANCE");
		retval.push_back("CITY_STATE_POSTCODE_ID");

		return retval;
	};

} // namespace

