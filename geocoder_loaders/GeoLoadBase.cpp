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
# $Rev: 39 $ 
# $Date: 2006-08-02 18:15:24 +0200 (Wed, 02 Aug 2006) $ 
*/

// GeoLoadBase.cpp:  

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <time.h>

#include "GeoLoadBase.h"
#include "../global/File.h"

namespace PortfolioExplorer {

	GeoLoadBase::GeoLoadBase()
		: numberOfOutputRecords(0)
	{
	}

	void GeoLoadBase::Open(const char *pFileName, const char *pOutputDir)
	{
		outdir = pOutputDir;
		m_readCSV.Open(pFileName);
		m_readCSV.ReadRecord();
		
		std::map<TsString, unsigned> mapFieldNames;
		for (unsigned x=0; x<m_readCSV.GetNumValues(); x++)
			mapFieldNames[m_readCSV.GetValue(x)] = x;

		std::vector<TsString> vFields = GetFieldParameters();
		for (std::vector<TsString>::const_iterator it = vFields.begin(); it!=vFields.end(); ++it)
		{
			if (mapFieldNames.find(*it)==mapFieldNames.end())
				throw TsString("The field \"") + *it + "\" does not exist in input.";
			m_mapFieldAccessors[*it] = FieldAccessor(this, mapFieldNames[*it]);
		}

	}
} // namespace
