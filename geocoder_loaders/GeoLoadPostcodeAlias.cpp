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

// GeoLoadPostcodeAlias.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <time.h>

#include "GeoLoadPostcodeAlias.h"
#include "BitStreamAdaptor.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Process the records for a terminal node.
	// Return value:
	//	bool		true on success, false on error or abort
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadPostcodeAlias::Process()
	{
		// Bind local output variables
		FieldAccessor postcodeValue = m_mapFieldAccessors["POSTCODE"];
		FieldAccessor postcodeGroupValue = m_mapFieldAccessors["POSTCODE_GROUP"];

		// Read the input into memory
		std::vector<PostcodeGroup> postcodeGroups;
		while (m_readCSV.ReadRecord()) {
			PostcodeGroup postcodeGroup;
			strncpy(postcodeGroup.postcode, postcodeValue.GetAsString().c_str(), sizeof(postcodeGroup.postcode));
			postcodeGroup.postcode[sizeof(postcodeGroup.postcode)-1] = 0;
			strncpy(postcodeGroup.group, postcodeGroupValue.GetAsString().c_str(), sizeof(postcodeGroup.group));
			postcodeGroup.group[sizeof(postcodeGroup.group)-1] = 0;
			postcodeGroups.push_back(postcodeGroup);
		};

		numberOfOutputRecords = int(postcodeGroups.size());
		// Sort by postcode
		{
			PostcodeCmp postcodeCmp;
			std::sort(postcodeGroups.begin(), postcodeGroups.end(), postcodeCmp);
		}

		// Write by-postcodefile
		File byPostcodeFile;

		TsString byPostcodeFilename = outdir + "/" + POSTCODE_ALIAS_BY_POSTCODE_FILE;
		if (!byPostcodeFile.Open(File::CreateAndWrite, byPostcodeFilename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + byPostcodeFilename + " for output"
			);
		}
		{for (unsigned i = 0; i < postcodeGroups.size(); i++) {
			// Write the next output record.
			byPostcodeFile.Write(
				GeoUtil::PostcodeAliasPostcodeFieldLength,
				postcodeGroups[i].postcode
			);
			byPostcodeFile.Write(
				GeoUtil::PostcodeAliasGroupFieldLength, 
				postcodeGroups[i].group
			);
		}}
		byPostcodeFile.Close();


		// Sort by group
		{
			GroupCmp groupCmp;
			std::sort(postcodeGroups.begin(), postcodeGroups.end(), groupCmp);
		}
		// Write by-group file
		File byGroupFile;
		TsString byGroupFilename = outdir + "/" + POSTCODE_ALIAS_BY_GROUP_FILE;
		if (!byGroupFile.Open(File::CreateAndWrite, byGroupFilename, FileBufferSize)) {
			throw TsString(
				"Cannot open file " + byGroupFilename + " for output"
			);
		}
		{for (unsigned i = 0; i < postcodeGroups.size(); i++) {
			// Write the next output record.
			byGroupFile.Write(
				sizeof(postcodeGroups[i].postcode) - 1, 
				postcodeGroups[i].postcode
			);
			byGroupFile.Write(
				sizeof(postcodeGroups[i].group) - 1, 
				postcodeGroups[i].group
			);
		}}
		byGroupFile.Close();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get a static array of FieldParameter entries, which will be used to
	// load up the fieldParameters vector.  The terminating element must
	// have an empty paramName.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	GeoLoadPostcodeAlias::GetFieldParameters() 
	{
		std::vector<TsString> retval;

		retval.push_back("POSTCODE");
		retval.push_back("POSTCODE_GROUP");

		return retval;
	};

} // namespace

