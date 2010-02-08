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

// QueryItf.h: Object used by the query interface.

#ifndef INCL_GeocoderPortfolioExplorerQueryItf_H
#define INCL_GeocoderPortfolioExplorerQueryItf_H

#include "../geocommon/Geocoder_DllExport.h"

namespace PortfolioExplorer {

	struct CityStatePostcodeFaIndex {
		int ID;
		char financeNumber[7];		// 6-digit USPS FA or three letters of Canada postcode*
		int cityStatePostcodeID;	// position of CityStatePostcode record
	};

	struct CityStatePostcode {
		int ID;					// unique ID of this record.
		char country[3];		// 2-letter ISO country code, null-terminated
		int state;				// numeric state/province code.
		char stateAbbr[4];		// State abbreviation, null-terminated
		char city[41];			// null-terminated city name
		char postcode[7];		// null-terminated postal code (USPS ZIP or 
								// Canada FSA)
		char financeNumber[7];	// USPS finance number or modified Canada FSA
		int streetNameIDFirst;	// ID of the first street name record
		int streetNameIDLast;	// ID of the last street name
	};

	struct StreetName {
		int cityStatePostcodeID;// parent cityStatePostcode record ID
		int ID;					// unique ID of this record
		char prefix[7];			// null-terminated prefix (Canada only)
		char predir[3];			// null-terminated predirectional
		char street[41];		// null-terminated street name
		char suffix[7];			// null-terminated street suffix
		char postdir[3];		// null-terminated postdirectional
		int streetSegmentIDFirst;	// ID of the first StreetSegment record
		int streetSegmentCount;	// Number of StreetSegment records
	};

	struct StreetSegment{
		int ID;
		int coordinateID;	// ID of first CoordinatePoint record
		int coordinateCount;// number of CoordinatePoint records
		short countyCode;	// county FIPS code
		bool isRightSide;	// false=left, true=right
		char addrLow[11];	// null-terminated address-range-low
		char addrHigh[11];	// null-terminated address-range-high
		char censusTract[7];// Census tract ID
		char censusBlock[5];// Census block ID
		char postcodeExt[5];// Postal code extension (ZIP+4 or Canada LRU)
							// Blank if unavailable

	};

	struct CoordinatePoint {
		CoordinatePoint(double latitude_, double longitude_) : latitude(latitude_), longitude(longitude_) {}
		CoordinatePoint(const CoordinatePoint& rhs) : latitude(rhs.latitude), longitude(rhs.longitude) {}
		CoordinatePoint() {}
		double latitude;
		double longitude;
		bool operator==(const CoordinatePoint& rhs) const {
			return latitude == rhs.latitude && longitude == rhs.longitude;
		}
		bool operator!=(const CoordinatePoint& rhs) const {
			return latitude != rhs.latitude || longitude != rhs.longitude;
		}
	};

	struct StreetIntersection {
		CityStatePostcode cityStatePostcode1;
		StreetName streetName1;
		StreetSegment streetSegment1;
		CityStatePostcode cityStatePostcode2;
		StreetName streetName2;
		StreetSegment streetSegment2;
	};

	struct PostcodeCentroid {
		char postcode[7];
		double latitude;
		double longitude;
	};

	struct PostcodeAlias {
		char postcode[7];
		char postcodeGroup[7];
	};

}

#endif

