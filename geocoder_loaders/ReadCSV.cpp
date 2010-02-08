#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "ReadCSV.h"

namespace PortfolioExplorer 
{
	inline bool ReadCSV::ReadBuffer()
	{
		if (m_bIsEOF)
			return false;

		size_t nSize = m_file.Read(sizeof(m_pCurrentBuffer), m_pCurrentBuffer);
		m_pCurrentBufferPos = m_pCurrentBuffer;
		if (nSize != sizeof(m_pCurrentBuffer))
		{
			m_bIsEOF = true;
			if (nSize==0)
				return false;

			// move it up in the buffer so the end is the end
			memmove(m_pCurrentBuffer+(sizeof(m_pCurrentBuffer)-nSize), m_pCurrentBuffer, nSize);
			m_pCurrentBufferPos += (sizeof(m_pCurrentBuffer)-nSize);
		}
		return true;
	}


	// Returns NULL if EOL or EOF, "" for an empty field
	bool ReadCSV::GetNextField(TsString & strRet)
	{
		strRet.resize(0);
		const char cDelimeter = ',';

		char cQuote = 0;
		// the position of a quote who's staus is in question.
		// this default condition can not be -1 because nLastQuotePos!=(nCurrentLen-1) would test wrong for position 0
		int nLastQuotePos = -99; 
		int nCurrentLen = 0;
		bool bStart = cDelimeter!=0;

		for ( ; ;m_pCurrentBufferPos++)
		{
			if (m_pCurrentBufferPos==m_pCurrentBufferEnd && !ReadBuffer())
			{
				if (strRet.empty())
					return false;
				else 
					break;
			}
			// trim leading whitespace and look for initial quote
			char c=*m_pCurrentBufferPos;

			// We treat nulls as white space
			// and ignore them even in middle of data field
			if (c == '\0') {
				continue;
			}

			if (bStart)
			{
				if (c=='\n')
				{
					m_pCurrentBufferPos++; // increment past the \n for next time
					return false;
				}

				// Ignore leading whitespace on a field if it isn't delimeter
				if (c != cDelimeter && (c==' ' || c=='\t'))
					continue;

				bStart = false;  

				if (c=='"' || c=='\'')
				{
					cQuote = c;
					continue;
				}
			}

			if (c=='\n')
				break;

			if ((cQuote==0 || nLastQuotePos>=0) && c==cDelimeter)
			{
				m_pCurrentBufferPos++; // increment past the , for next time
				break;
			}

			if (c==cQuote && cDelimeter!=0)
			{
				if (nLastQuotePos==(nCurrentLen-1))
				{
					nLastQuotePos = -99;  // an escaped quote was found, ignore the second
					continue;
				}
				else
					nLastQuotePos = nCurrentLen; // record it and fall through to append this quote to output
			}
			else if (nLastQuotePos>=0 && !isspace(c))
				nLastQuotePos = -99;

			nCurrentLen++;
			strRet += c;
		}

		assert(cQuote==0 || nLastQuotePos>=0);

		if (nLastQuotePos>=0)
		{
			strRet.resize(nLastQuotePos);
		}
		else if (cDelimeter!=0)
		{
			size_t nNewLen = strRet.length();
			while(nNewLen!=0 && isspace(strRet[nNewLen-1]))
				nNewLen--;
			strRet.resize(nNewLen);
		}

		return true;
	}

	bool ReadCSV::ReadRecord()
	{
		m_vValues.clear();

		bool bEOL = false;
		TsString strVal;
		while(GetNextField(strVal))
			m_vValues.push_back(strVal);

		return m_vValues.size()!=0;
	}


	void ReadCSV::Open(TsString strFile)
	{
		m_pCurrentBufferPos = m_pCurrentBuffer+sizeof(m_pCurrentBuffer);
		m_pCurrentBufferEnd = (m_pCurrentBuffer+sizeof(m_pCurrentBuffer));
		m_bIsEOF = false;
		m_strFile = strFile;
		m_file.Open(File::ReadOnly, m_strFile);
	}
	void ReadCSV::ReOpen()
	{
		m_file.Close();
		Open(m_strFile);
	}

}
