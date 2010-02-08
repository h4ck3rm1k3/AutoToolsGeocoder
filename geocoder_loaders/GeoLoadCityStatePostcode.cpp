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

// GeoLoadCityStatePostcode.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <time.h>

#include "GeoLoadCityStatePostcode.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadCityStatePostcode::Process()
	{
		// Bind local output variables
		FieldAccessor cityStatePostcodeIDValue = m_mapFieldAccessors["CITY_STATE_POSTCODE_ID"];
		FieldAccessor countryValue = m_mapFieldAccessors["COUNTRY"];
		FieldAccessor stateValue = m_mapFieldAccessors["STATE"];
		FieldAccessor postcodeValue = m_mapFieldAccessors["POSTCODE"];
		FieldAccessor cityNameValue = m_mapFieldAccessors["CITY_NAME"];
		FieldAccessor financeNumberValue = m_mapFieldAccessors["FINANCE"];
		FieldAccessor streetNameIDFirstValue = m_mapFieldAccessors["STREET_NAME_ID_FIRST"];
		FieldAccessor streetNameIDLastValue = m_mapFieldAccessors["STREET_NAME_ID_LAST"];

		// Get the first record from upstream first, to allow complex processing 
		// to complete before opening the output file.
		bool inputOK = m_readCSV.ReadRecord();

		// Open file
		File file;
		TsString filename = outdir + "/" + CITY_STATE_POSTCODE_FILE;
		if (!file.Open(File::CreateAndWrite, filename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + filename + " for output"
			);
		}

		if (!inputOK)
			throw TsString("No records to read");

		numberOfOutputRecords = 0;

		do {

			assert((int)cityStatePostcodeIDValue.GetAsInt() == numberOfOutputRecords);

			// Write the next output record.

			// Country
			file.Write(GeoUtil::CityStatePostcodeCountryFieldLength, countryValue.GetAsString().c_str());

			// State
			WriteOneByteInt(file, (unsigned int)stateValue.GetAsInt());

			// postal code
			{
				char buf[GeoUtil::CityStatePostcodePostcodeFieldLength];
				memset(buf, 0, sizeof(buf));
				TsString strPostcode = postcodeValue.GetAsString();
				memcpy(buf, strPostcode.c_str(), JHMIN(unsigned(strPostcode.length()), sizeof(buf)) );
				file.Write(sizeof(buf), buf);
			}

			// city name
			{
				char buf[GeoUtil::CityStatePostcodeCityNameFieldLength];
				memset(buf, 0, sizeof(buf));
				TsString strCity = cityNameValue.GetAsString();
				memcpy(buf, strCity.c_str(), JHMIN(unsigned(strCity.length()), sizeof(buf)) );
				file.Write(sizeof(buf), buf);
			}

			// finance "number"
			{
				char buf[GeoUtil::CityStatePostcodeFinanceFieldLength];
				memset(buf, 0, sizeof(buf));
				TsString strFinance = financeNumberValue.GetAsString();
				memcpy(buf, strFinance.c_str(), JHMIN(unsigned(strFinance.length()), sizeof(buf)) );
				file.Write(sizeof(buf), buf);
			}

			// first/last street name IDs
			WriteThreeByteInt(file, (int)streetNameIDFirstValue.GetAsInt());
			WriteThreeByteInt(file, (int)streetNameIDLastValue.GetAsInt());

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
	GeoLoadCityStatePostcode::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("CITY_STATE_POSTCODE_ID");
		retval.push_back("COUNTRY");
		retval.push_back("STATE");
		retval.push_back("POSTCODE");
		retval.push_back("CITY_NAME");
		retval.push_back("FINANCE");
		retval.push_back("STREET_NAME_ID_FIRST");
		retval.push_back("STREET_NAME_ID_LAST");

		return retval;
	};

} // namespace

