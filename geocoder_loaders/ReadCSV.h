#ifndef __READCSV_H__
#define __READCSV_H__

#include "../global/file.h"
#include <vector>

namespace PortfolioExplorer 
{
	class ReadCSV
	{
		File m_file;
		bool m_bIsEOF;

		char m_pCurrentBuffer[0x10000];
		const char * m_pCurrentBufferPos;
		const char * m_pCurrentBufferEnd;

		inline bool ReadBuffer();

		// returns NULL for no more on this line (or file)
		bool GetNextField(TsString & strRet);
		std::vector<TsString> m_vValues;
		TsString m_strFile;

	public:
		inline ReadCSV()
			: m_pCurrentBufferPos(m_pCurrentBuffer+sizeof(m_pCurrentBuffer))
			, m_pCurrentBufferEnd(m_pCurrentBuffer+sizeof(m_pCurrentBuffer))
			, m_bIsEOF(false)
		{}
		void Open(TsString strFile);
		void ReOpen();
		
		// returns false when done
		bool ReadRecord();
		size_t GetNumValues() const { return m_vValues.size(); }
		TsString GetValue(unsigned nWhich) const
		{
			if (nWhich>=m_vValues.size())
			{
				assert(false);
				return TsString();
			}
			return m_vValues[nWhich];
		}
	};
}
#endif //__READCSV_H__
