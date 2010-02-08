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
# $Rev: 47 $ 
# $Date: 2006-08-30 19:48:17 +0200 (Wed, 30 Aug 2006) $ 
*/

// geocoder_loaders.cpp : Defines the entry point for the console application.
//

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <string>
#include <iostream>

#include "GeoLoadCitySoundex.h"
#include "GeoLoadCityStatePostcode.h"
#include "GeoLoadCityStatePostcodeFaIndex.h"
#include "GeoLoadCoordinate.h"
#include "GeoLoadPostcodeAlias.h"
#include "GeoLoadPostcodeCentroid.h"
#include "GeoLoadStreetIntersectionSoundex.h"
#include "GeoLoadStreetName.h"
#include "GeoLoadStreetNameSoundex.h"
#include "GeoLoadStreetSegment.h"

#include "../geocoder/GeocoderVersion.h"

using namespace PortfolioExplorer;

int main(int argc, char* argv[])
{
	std::cout << "PortfolioExplorer Loaders Version " << APP_VERSION << "\n";
	try
	{
		if (argc!=4)
			throw TsString("Incorrect number of command line arguments");

		std::auto_ptr<GeoLoadBase> pGeoLoad;

		if (_stricmp(argv[1], "CitySoundex")==0)
			pGeoLoad.reset(new GeoLoadCitySoundex);
		else if (_stricmp(argv[1], "CityStatePostcode")==0)
			pGeoLoad.reset(new GeoLoadCityStatePostcode);
		else if (_stricmp(argv[1], "CityStatePostcodeFaIndex")==0)
			pGeoLoad.reset(new GeoLoadCityStatePostcodeFaIndex);
		else if (_stricmp(argv[1], "Coordinate")==0)
			pGeoLoad.reset(new GeoLoadCoordinate);
		else if (_stricmp(argv[1], "PostcodeAlias")==0)
			pGeoLoad.reset(new GeoLoadPostcodeAlias);
		else if (_stricmp(argv[1], "PostcodeCentroid")==0)
			pGeoLoad.reset(new GeoLoadPostcodeCentroid);
		else if (_stricmp(argv[1], "StreetIntersectionSoundex")==0)
			pGeoLoad.reset(new GeoLoadStreetIntersectionSoundex);
		else if (_stricmp(argv[1], "StreetName")==0)
			pGeoLoad.reset(new GeoLoadStreetName);
		else if (_stricmp(argv[1], "StreetNameSoundex")==0)
			pGeoLoad.reset(new GeoLoadStreetNameSoundex);
		else if (_stricmp(argv[1], "StreetSegment")==0)
			pGeoLoad.reset(new GeoLoadStreetSegment);
		else if (pGeoLoad.get()==NULL)
			throw TsString("Unknown LoaderName \"") + argv[1];
		
		pGeoLoad->Open(argv[2], argv[3]);
		pGeoLoad->Process();
		std::cout << pGeoLoad->GetNumberOfOutputRecords() << " records written\n";
	}
	catch (const TsString & strError)
	{
		std::cout << "There was an error: " << strError << "\n\n";
		std::cout << "Usage:\n"
			"	geocoder_loaders LoaderName InFile.csv OutDir\n"
			"Where LoaderName is one of:\n"
			"	CitySoundex\n"
			"	CityStatePostcode\n"
			"	CityStatePostcodeFaIndex\n"
			"	Coordinate\n"
			"	PostcodeAlias\n"
			"	PostcodeCentroid\n"
			"	StreetIntersectionSoundex\n"
			"	StreetName\n"
			"	StreetNameSoundex\n"
			"	StreetSegment\n";

		return 1;
	}
	return 0;
}

