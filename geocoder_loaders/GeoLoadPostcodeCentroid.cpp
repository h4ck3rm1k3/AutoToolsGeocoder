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

// GeoLoadPostcodeCentroid.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <time.h>

#include "GeoLoadPostcodeCentroid.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadPostcodeCentroid::Process()
	{

		// Bind output variables
		FieldAccessor postcodeValue = m_mapFieldAccessors["POSTCODE"];
		FieldAccessor latitudeValue = m_mapFieldAccessors["LATITUDE"];
		FieldAccessor longitudeValue = m_mapFieldAccessors["LONGITUDE"];


		// Get the first record from upstream first, to allow complex processing 
		// to complete before opening the output file.
		bool inputOK = m_readCSV.ReadRecord();

		// Open file
		File file;
		TsString filename = outdir + "/" + POSTCODE_CENTROID_FILE;
		if (!file.Open(File::CreateAndWrite, filename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + filename + " for output"
			);
		}

		if (!inputOK)
			throw TsString("No records to read");

		numberOfOutputRecords = 0;

		do {

			if (!latitudeValue.IsValidDouble() || !longitudeValue.IsValidDouble()) {
				// Ignore bad coordinates
				continue;
			}

			// Write the next output record.

			// postcode
			{
				char buf[GeoUtil::PostcodeCentroidPostcodeFieldLength];
				memset(buf, 0, sizeof(buf));
				TsString strPostCode = postcodeValue.GetAsString();
				memcpy(buf, strPostCode.c_str(), JHMIN(unsigned(strPostCode.length()), sizeof(buf)) );
				file.Write(sizeof(buf), buf);
			}

			// latitude
			int latitude = int(latitudeValue.GetAsDouble() * 100000 + 0.5);
			file.Write(4, (char*)&latitude);

			// longitude
			int longitude = int(longitudeValue.GetAsDouble() * 100000 + 0.5);
			file.Write(4, (char*)&longitude);

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
	GeoLoadPostcodeCentroid::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("POSTCODE");
		retval.push_back("LATITUDE");
		retval.push_back("LONGITUDE");

		return retval;
	};


} // namespace

