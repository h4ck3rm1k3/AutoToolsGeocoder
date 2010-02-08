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
# $Rev: 123 $ 
# $Date: 2009-10-12 13:03:59 +0200 (Mon, 12 Oct 2009) $ 
*/

#include <stdlib.h>
#include <iostream>
#include "../geocoder/Geocoder.h"

#ifdef WIN32
const std::string gInstBaseDir("C:\\Program Files\\SRC\\PortfolioExplorer");
#else
const std::string gInstBaseDir("tiger");
#endif

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <address> <city, state, zip>" << std::endl; 
		return 1;
	}

	std::string tables(gInstBaseDir);
	tables += "/tables";
	
	std::string database(gInstBaseDir);
	database += "/database";

	PortfolioExplorer::Geocoder geoCoder(tables.c_str(), database.c_str());
	PortfolioExplorer::Geocoder::GeocodeResults geoResults;

	if( geoCoder.Open() == false ) {
	  std::cerr << "geoCoder.Open() failed.  Please check database and tables paths and ensure that database exists." << std::endl;
	  std::cerr << "gInstBaseDir = " << gInstBaseDir.c_str() << std::endl;
	  exit(1);
	}

	geoCoder.CodeAddress(argv[1], argv[2]);

	while (geoCoder.GetNextCandidate(geoResults)) {
		std::cout << "Match score (0-1000): " << geoResults.GetMatchScore() << std::endl 
                  << "Census Block: " << geoResults.GetCensusBlock() << std::endl
                  << "City: " << geoResults.GetCity() << std::endl
                  << "Country: " << geoResults.GetCountryCode() << std::endl
                  << "County: " << geoResults.GetCountyCode() << std::endl
                  << "Latitude: " << geoResults.GetLatitude() << std::endl
		  << "Longitude: " << geoResults.GetLongitude() << std::endl
                  << "State: " << geoResults.GetState() << std::endl
                  << "Street: " << geoResults.GetStreet() << std::endl
                  << "Street 2:" << geoResults.GetStreet2() << std::endl
				  << std::endl 
				  << std::endl;
	}

	return 0;
}
