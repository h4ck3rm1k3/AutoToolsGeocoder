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

// GeocodeResultsInternal.h: Declaration of internal Geocoder results.
// Not to be used across DLL boundary.

#ifndef INCL_GeocoderPortfolioExplorerResultsInternal_H
#define INCL_GeocoderPortfolioExplorerResultsInternal_H

#include "../geocommon/Geocoder_DllExport.h"
//#include "GeocoderItf.h"

namespace PortfolioExplorer {

	struct GeocodeResultsInternal {
		char addrNbr[11];		// address number
		char prefix[7];			// prefix (Canada only)
		char predir[3];			// predirectional
		char street[41];		// street name
		char streetSuffix[7];	// street suffix
		char postdir[3];		// postdirectional
		char unitDes[5];		// unit designator
		char unit[9];			// unit number
		char city[41];			// city
		int state;				// numeric state/province code
		char stateAbbr[4];		// two- or three-letter state abbreviation
		char countryCode[3];	// ISO two-letter country code
		int countyCode;			// County FIPS code (USA only)
		char censusTract[7];	// six-char census tract (USA only)
		char censusBlock[5];	// four-char census block (USA only)
		char postcode[7];		// five-digit ZIP code or three-letter leading Canadian postal code
		char postcodeExt[5];	// four-digit ZIP+4 or three-letter trailing Canadian  postal code
		double latitude;		// latitude
		double longitude;		// longitude
		int matchScore;			// match score, 0-1000
		int matchStatus;		// MatchStatus flag set
		int geoStatus;			// GeocodeStatus flag set
		// When a street intersection is coded, the geoStatus flags will
		// contain GeocodeIntersection.  When that is true, the following
		// fields will contain the intersecting street.
		char prefix2[7];		// intersecting street prefix
		char predir2[3];		// intersecting street predirectional
		char street2[41];		// intersecting street name
		char streetSuffix2[7];	// intersecting street suffix
		char postdir2[3];		// intersecting street postdirectional
	};

}

#endif
