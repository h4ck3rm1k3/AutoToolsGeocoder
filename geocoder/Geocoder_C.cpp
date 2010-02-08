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

#include "../geocommon/Geocoder_Headers.h"
#include "Geocoder.h"
#include "Geocoder_C.h"

namespace PortfolioExplorer {

	struct Geocoder_C_Helper : public Geocoder
	{
		Geocoder_C_Helper(const char* tableDir, const char* databaseDir, int memUse)
			: Geocoder(tableDir, databaseDir, (MemUse)memUse)
		{
		}

		TsString m_strLastError;
		virtual void ErrorMessage(const char* message)
		{
			m_strLastError = message;
		}
		GeocodeResults m_lastResults;
	};
}

GEO_EXPORT(intptr_t) GEO_Open(const char* tableDir, const char* databaseDir, int nMemUse, char *pErrorReturn)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = new PortfolioExplorer::Geocoder_C_Helper(tableDir, databaseDir, nMemUse);
	if (!pGeocoder->Open())
	{
		strncpy(pErrorReturn, pGeocoder->m_strLastError.c_str(), 256);
		pErrorReturn[255] = 0;
		delete pGeocoder;
		return NULL;
	}

	return reinterpret_cast<intptr_t>(pGeocoder);
}

GEO_EXPORT(void) GEO_Close(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	delete pGeocoder;
}

GEO_EXPORT(int) GEO_CodeAddress(intptr_t nHandle, const char* line1, const char* line2)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->CodeAddress(line1, line2);
}

GEO_EXPORT(int) GEO_GetNextCandidate(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->GetNextCandidate(pGeocoder->m_lastResults);
}

///////////////////////////////////////////////////////////////////////////////
// the following functions return the current result.  There MUST have been a successfull
// call to GEO_CodeAddress & GEO_GetNextCandidate to use them
GEO_EXPORT(const char*) GEO_RESULT_GetAddrNbr(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetAddrNbr();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPrefix(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPrefix();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPredir(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPredir();
}
GEO_EXPORT(const char*) GEO_RESULT_GetStreet(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetStreet();
}
GEO_EXPORT(const char*) GEO_RESULT_GetSuffix(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetSuffix();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPostdir(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPostdir();
}
GEO_EXPORT(const char*) GEO_RESULT_GetUnitDes(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetUnitDes();
}
GEO_EXPORT(const char*) GEO_RESULT_GetUnit(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetUnit();
}
GEO_EXPORT(const char*) GEO_RESULT_GetCity(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetCity();
}
GEO_EXPORT(int) GEO_RESULT_GetState(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetState();
}
GEO_EXPORT(const char*) GEO_RESULT_GetStateAbbr(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetStateAbbr();
}
GEO_EXPORT(const char*) GEO_RESULT_GetCountryCode(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetCountryCode();
}
GEO_EXPORT(int) GEO_RESULT_GetCountyCode(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetCountyCode();
}
GEO_EXPORT(const char*) GEO_RESULT_GetCensusTract(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetCensusTract();
}
GEO_EXPORT(const char*) GEO_RESULT_GetCensusBlock(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetCensusBlock();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPostcode(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPostcode();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPostcodeExt(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPostcodeExt();
}
GEO_EXPORT(double) GEO_RESULT_GetLatitude(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetLatitude();
}
GEO_EXPORT(double) GEO_RESULT_GetLongitude(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetLongitude();
}
GEO_EXPORT(int) GEO_RESULT_GetMatchScore(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetMatchScore();
}
GEO_EXPORT(int) GEO_RESULT_GetMatchStatus(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetMatchStatus();
}
GEO_EXPORT(int) GEO_RESULT_GetGeoStatus(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetGeoStatus();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPrefix2(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPrefix2();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPredir2(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPredir2();
}
GEO_EXPORT(const char*) GEO_RESULT_GetStreet2(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetStreet2();
}
GEO_EXPORT(const char*) GEO_RESULT_GetSuffix2(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetSuffix2();
}
GEO_EXPORT(const char*) GEO_RESULT_GetPostdir2(intptr_t nHandle)
{
	PortfolioExplorer::Geocoder_C_Helper *pGeocoder = reinterpret_cast<PortfolioExplorer::Geocoder_C_Helper *>(nHandle);
	return pGeocoder->m_lastResults.GetPostdir2();
}

