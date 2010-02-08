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

// GeoLoadBase.h: Base class for GeoLoad

#ifndef INCL_GeoLoadBase_H
#define INCL_GeoLoadBase_H

#if _MSC_VER >= 1000
#pragma once
#endif
#include "ReadCSV.h"
#include "../geocommon/geoutil.h"
#include "GeoloadUtilities.h"
#include "math.h"

namespace PortfolioExplorer {

	class GeoLoadBase : public GeoUtil 
	{
	public:
		GeoLoadBase();
		void Open(const char * pFileName, const char *pOutputDir);
		virtual void Process() = 0;

	protected:
		virtual std::vector<TsString> GetFieldParameters() = 0;

		class FieldAccessor
		{
		public:
			FieldAccessor(const GeoLoadBase * pParent=NULL, int nFieldNum=-1)
				: m_pParent(pParent)
				, m_nFieldNum(nFieldNum)
			{
			}

			const GeoLoadBase * m_pParent;
			int m_nFieldNum;

			TsString GetAsString()
			{
				return m_pParent->m_readCSV.GetValue(m_nFieldNum);
			}

			int GetAsInt()
			{
				return atol(GetAsString().c_str());
			}
			double GetAsDouble()
			{
				return atof(GetAsString().c_str());
			}

			bool IsValidDouble()
			{
				return fabs(GetAsDouble())>.001;
			}

			bool GetAsBoolean()
			{
				char c = GetAsString()[0];
				return (c!='0' && iswdigit(c)) || 'T'==towupper(c);
			}
		};
		std::map<TsString, FieldAccessor> m_mapFieldAccessors;

		// The output directory
		TsString outdir;
		ReadCSV m_readCSV;

		static const int FileBufferSize = 0x100000;

		// The number of records that have been written to the output.
		int numberOfOutputRecords;
	public:
		int GetNumberOfOutputRecords() const { return numberOfOutputRecords; }
	};
}

#endif

