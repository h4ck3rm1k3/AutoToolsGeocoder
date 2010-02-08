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
# $Rev: 11990 $ 
# $Date: 2006-02-22 13:46:58 -0700 (Wed, 22 Feb 2006) $ 
*/

#ifndef INCL_Geocoder_C_H
#define INCL_Geocoder_C_H

#ifdef __cplusplus
	#define EXTERN_C	extern "C"
#else
	#define EXTERN_C	extern
#endif

#ifdef GEOCODER_EXPORTS  
	#define GEO_EXPORT( retType ) EXTERN_C  retType __stdcall
#else
	#define GEO_EXPORT( retType ) EXTERN_C __declspec(dllimport) retType __stdcall
#endif

///////////////////////////////////////////////////////////////////////////////
// Memory usage control
const int GEO_MemUseSmall = 0;		// 30% of normal ~= 3MB
const int GEO_MemUseNormal = 1;		// ~= 9MB
const int GEO_MemUseLarge = 2;			// 3x normal ~= 27MB

///////////////////////////////////////////////////////////////////////////////
// Possible global status return values from CodeAddress().
const int GEO_GlobalSingle = 0;		// There is a distinguished "best" result
const int GEO_GlobalMultiple = 1;		// There are multiple close results 
const int GEO_GlobalFailure = 2;		// There are no results that exceed the threshold.

///////////////////////////////////////////////////////////////////////////////
// Possible address-matching status flags
// Possible failure codes
const int GEO_MatchFailed =                0x80000000;// Match score did not exceed threshold

// Possible success codes
const int GEO_MatchPerfect =               0x04000000;	// everything matched perfectly
const int GEO_MatchIsValid =               0x02000000;	// exceeded threshold but not perfect

// Modifications of success status -- first-line address
const int GEO_MatchChangedPredir =         0x00000001;
const int GEO_MatchChangedStName =         0x00000002;
const int GEO_MatchChangedStPrefix =       0x00000004;
const int GEO_MatchChangedStSuffix =       0x00000008;
const int GEO_MatchChangedPostdir =        0x00000010;
const int GEO_MatchChangedUnitDes =        0x00000020;
const int GEO_MatchAddressNbrOutOfRange =  0x00000040;
const int GEO_MatchUnitNbrOutOfRange =     0x00000080;
const int GEO_MatchMovedAddrLetterToUnit = 0x00000100;	// e.g. "123A MAIN ST" to 
											//		"123 MAIN ST #A"
const int GEO_MatchEvenOddDiffer =         0x00000200;	// even segment and odd address or vice-versa

// Modifications of success status -- last-line address
const int GEO_MatchSuppliedCity =          0x00001000;	// City was added
const int GEO_MatchSuppliedState =         0x00002000;	// State was added
const int GEO_MatchSuppliedPostcode =      0x00004000;	// Postal code was added
const int GEO_MatchChangedCity =           0x00010000;	// City was changed
const int GEO_MatchChangedState =          0x00020000;	// State was changed
const int GEO_MatchChangedPostcode =       0x00040000;	// Postal code was changed
const int GEO_MatchChangedPostcodeExt =    0x00080000;	// Postal code extension was changed
										// (4-digit for US, 3-letter for CA)
const int GEO_MatchCityAliasUsed =         0x00100000;	// City was matched to alias or replacement

// Note that for intersections, the MatchChanged* flags will be set
// if either street name is altered.


///////////////////////////////////////////////////////////////////////////////
// Possible geocoding status flags
// Possible failure codes
const int GEO_GeocodeFailed =              0x80000000;	// No street match or no lat/lon segment.

// Possible success codes
const int GEO_GeocodeAddress =             0x08000000;	// Correctly coded from street address
const int GEO_GeocodeExtrapolate =         0x04000000;	// Extrapolate addr number out of range
const int GEO_GeocodeIntersection =        0x02000000;	// Street intersection
const int GEO_GeocodeZip9Centroid =        0x01000000;	// 9-digit zip centroid (not yet implemented)
const int GEO_GeocodeZip7Centroid =        0x00800000;	// 7-digit zip centroid (not yet implemented)
const int GEO_GeocodeZip5Centroid =        0x00400000;	// 5-digit zip centroid 
const int GEO_GeocodePostcode6Centroid =   0x00200000;	// 6-letter Canadian postcode centroid
const int GEO_GeocodePostcode3Centroid =   0x00100000;	// 3-letter Canadian postcode centroid
const int GEO_GeocodePostcodeAnyCentriod = 
	GEO_GeocodeZip9Centroid | GEO_GeocodeZip7Centroid | 
	GEO_GeocodeZip5Centroid | GEO_GeocodePostcode6Centroid | 
	GEO_GeocodePostcode3Centroid;			// Any centroid

///////////////////////////////////////////////////////////////////////////////
// Default threshold values
const int GEO_DefaultMatchThreshold = 800;
const int GEO_DefaultMultipleMatchThreshold = 25;


///////////////////////////////////////////////////////////////////////////////
// Possible treatment of how the City/State/Postcode record that "owns" a street segments
// is reconciled with with the City/State/Postcode that best matches the entered last-line.
const int GEO_ChooseStreetOwnerOverLastLine = 0;		// Always choose the City/State/Postcode that "owns" the street segment
const int GEO_ChooseLastLineOverStreetOwner = 1;		// Always choose the City/State/Postcode that best matches the last-line
const int GEO_ChooseStreetOwnerCountrySpecific = 2;	// DEFAULT
									// Use country-specific logic, because for USA we want the best match to
									// last-line (for vanity city names), whereas for CA we want the street-segment
									// owner (because three-letter postcode granularity holds several cities).


///////////////////////////////////////////////////////////////////////////////
// pErrorReturn MUST point to a 256 character buffer and will receive any error message
// if this fails (returns NULL)
GEO_EXPORT(intptr_t) GEO_Open(const char* tableDir, const char* databaseDir, int nMemUse, char *pErrorReturn);
GEO_EXPORT(void) GEO_Close(intptr_t nHandle);

// returns the status flags
GEO_EXPORT(int) GEO_CodeAddress(intptr_t nHandle, 
	const char* line1,					// street address
	const char* line2					// city, state, zip
);

// returns 1 if there is a next candidate, 0 if none
GEO_EXPORT(int) GEO_GetNextCandidate(intptr_t nHandle);

///////////////////////////////////////////////////////////////////////////////
// the following functions return the current result.  There MUST have been a successfull
// call to GEO_CodeAddress & GEO_GetNextCandidate to use them
GEO_EXPORT(const char*) GEO_RESULT_GetAddrNbr(intptr_t nHandle);		// address number
GEO_EXPORT(const char*) GEO_RESULT_GetPrefix(intptr_t nHandle);		// prefix (Canada only)
GEO_EXPORT(const char*) GEO_RESULT_GetPredir(intptr_t nHandle);		// predirectional
GEO_EXPORT(const char*) GEO_RESULT_GetStreet(intptr_t nHandle);		// street name
GEO_EXPORT(const char*) GEO_RESULT_GetSuffix(intptr_t nHandle);		// street suffix
GEO_EXPORT(const char*) GEO_RESULT_GetPostdir(intptr_t nHandle);		// postdirectional
GEO_EXPORT(const char*) GEO_RESULT_GetUnitDes(intptr_t nHandle);		// unit designator
GEO_EXPORT(const char*) GEO_RESULT_GetUnit(intptr_t nHandle);			// unit
GEO_EXPORT(const char*) GEO_RESULT_GetCity(intptr_t nHandle);			// city name
GEO_EXPORT(int) GEO_RESULT_GetState(intptr_t nHandle);					// numeric state/province code
GEO_EXPORT(const char*) GEO_RESULT_GetStateAbbr(intptr_t nHandle);		// two- or three-letter state abbreviation
GEO_EXPORT(const char*) GEO_RESULT_GetCountryCode(intptr_t nHandle);	// ISO two-letter country code
GEO_EXPORT(int) GEO_RESULT_GetCountyCode(intptr_t nHandle);			// County FIPS code (USA only)
GEO_EXPORT(const char*) GEO_RESULT_GetCensusTract(intptr_t nHandle);	// six-char census tract (USA only)
GEO_EXPORT(const char*) GEO_RESULT_GetCensusBlock(intptr_t nHandle);	// four-char census block (USA only)
GEO_EXPORT(const char*) GEO_RESULT_GetPostcode(intptr_t nHandle);		// five-digit ZIP code or three-letter leading Canadian postal code
GEO_EXPORT(const char*) GEO_RESULT_GetPostcodeExt(intptr_t nHandle);	// four-digit ZIP+4 or three-letter trailing Canadian  postal code
GEO_EXPORT(double) GEO_RESULT_GetLatitude(intptr_t nHandle);			// latitude
GEO_EXPORT(double) GEO_RESULT_GetLongitude(intptr_t nHandle);			// longitude
GEO_EXPORT(int) GEO_RESULT_GetMatchScore(intptr_t nHandle);			// match score, 0-1000
GEO_EXPORT(int) GEO_RESULT_GetMatchStatus(intptr_t nHandle)	;		// MatchStatus flag set
GEO_EXPORT(int) GEO_RESULT_GetGeoStatus(intptr_t nHandle);				// GeocodeStatus flag set
			// When a street intersection is coded, the geoStatus flags will
			// contain GeocodeIntersection.  When that is true, the following
			// fields will contain the intersecting street.
GEO_EXPORT(const char*) GEO_RESULT_GetPrefix2(intptr_t nHandle);		// intersecting street prefix
GEO_EXPORT(const char*) GEO_RESULT_GetPredir2(intptr_t nHandle);		// intersecting street predirectional
GEO_EXPORT(const char*) GEO_RESULT_GetStreet2(intptr_t nHandle);		// intersecting street name
GEO_EXPORT(const char*) GEO_RESULT_GetSuffix2(intptr_t nHandle);		// intersecting street suffix
GEO_EXPORT(const char*) GEO_RESULT_GetPostdir2(intptr_t nHandle);		// intersecting street postdirectional




#endif //INCL_Geocoder_C_H
