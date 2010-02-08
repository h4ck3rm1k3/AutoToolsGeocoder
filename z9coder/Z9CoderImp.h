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

///////////////////////////////////////////////////////////////////////////////
// Module: ZIP9CODER.H
///////////////////////////////////////////////////////////////////////////////

#ifndef __ZIP9CODER_H__
#define __ZIP9CODER_H__
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Disable that pesky long-mangled-name warning
#pragma warning(disable:4786)
// Disable non-exported class warning; this is wierd for templates
#pragma warning(disable:4251)
#pragma warning(disable:4275)

#include <typeinfo>
#include <assert.h>

// Utility headers from the Geocoder
#include "stdio.h"
#include "GeoBitStream.h"
#include "GeoHuffman.h"
#include "SetAssocCache.h"
#include "Exception.h"
#include "Exception.h"

namespace PortfolioExplorer
{
	enum { 
		CHUNK_SIZE = 50,
		NUM_FIELDS = 11
	};

	typedef HuffmanCoder<int, std::less<int> > HuffmanCoderInt;

	class Z9CoderException : public ErrorException 
	{
	public:
		inline Z9CoderException(const TsString& message_) : ErrorException(message_) {}
	};

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class File
	class GeoFile   // throw Zip9CoderException
	{
		FILE *m_fp;
		TsString m_strFileName;

		// disable copying
		GeoFile(const GeoFile &);
		GeoFile & operator =(GeoFile &);
	public:
		inline GeoFile()
			: m_fp(NULL)
		{
		}

		inline ~GeoFile()
		{
			Close();
		}

		inline void Open(TsString strFileName, const char *pMode)
		{
			m_strFileName = strFileName;
			m_fp = fopen(m_strFileName.c_str(), pMode);
			if (!m_fp)
				throw Z9CoderException("Unable to open the file \"" + m_strFileName + "\"");
		}

		inline void Close()
		{
			if (m_fp)
				fclose(m_fp);
			m_fp = NULL;

		}

		inline bool IsOpen()
		{
			return m_fp!=NULL;
		}

		void Read(void *pBuffer, int nSize)
		{
			if (1!=fread(pBuffer, nSize, 1, m_fp))
				throw Z9CoderException("Error reading from the file \"" + m_strFileName + "\"");
		}

		void Write(const void *pBuffer, int nSize)
		{
			if (1!=fwrite(pBuffer, nSize, 1, m_fp))
				throw Z9CoderException("Error writing to the file \"" + m_strFileName + "\"");
		}

		int Tell()
		{
			int nRet = ftell(m_fp);
			if (nRet<0)
				throw Z9CoderException("Error in the file \"" + m_strFileName + "\"");
			return nRet;
		}

		void Seek(int nPos, int nOrigin=SEEK_SET)
		{
			if (0!=fseek(m_fp, nPos, nOrigin))
				throw Z9CoderException("Error in the file \"" + m_strFileName + "\"");
		}
	};
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class FreqTableInt
	class FreqTableInt : public FreqTable<int>
	{
	public:
		void SaveBinary(GeoFile &inFile);
		void LoadBinary(GeoFile &outFile);
	};


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class RelCompressInt

	// usage:
	//		this requires 2 passes through all the data, 1 to collect stats and a 2nd to build
	//		in the first pass, call Add repeatedly with your data, making sure to call EndChunkStatistics
	//		everwhere a chuck will end in the real build.
	//
	//		To prepare for actual compression, call FinalizeStatistics
	//
	//		Now call add again for each piece of data, making sure to call WriteChunk at the end of each chunk.
	class RelCompressInt
	{

		HuffmanCoderInt m_aHuffInitialGroupValueBytes[2];
		HuffmanCoderInt m_aHuffValueBytes[2];
		HuffmanCoderInt m_HuffRepeatCount;

		FreqTableInt m_aFreqInitialGroupValueBytes[2];
		FreqTableInt m_aFreqValueBytes[2];
		FreqTableInt m_FreqRepeatCount;


		int m_aValues[CHUNK_SIZE];
		unsigned m_nNumValuesInBuffer;


	public:
		RelCompressInt();

		// the following members are used for writing a file
		inline void Add(int nVal)
		{
			assert(m_nNumValuesInBuffer<CHUNK_SIZE);
			m_aValues[m_nNumValuesInBuffer++] = nVal;
		}

		void EndChunkStatistics();
		void FinalizeStatistics();

		void WriteChunk(BitStreamWrite &bitStream);

		void WriteStatistics(GeoFile &outFile);

		// and these are use for reading a file
		void ReadStatistics(GeoFile &inFile);

		///////////////////////////////////////////////////////////////////////////////
		// Function name	: ReadChunk
		// Description: 
		// Return "void": 
		// Arguments:
		//    GeoUtil::BitStreamRead &bitStream: 
		//    unsigned nNumRecords: typicly this would be CHUNK_SIZE, but for the last chunk in the file it will be lower
		//		int *aResult: MUST be at least nNumRecords in length
		///////////////////////////////////////////////////////////////////////////////
		void ReadChunk(BitStreamRead &bitStream, unsigned nNumRecords, int *aResult);
	};

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	struct FileHeader
	{
		// this is used to verify that we are trying to open the correct file type and version
		unsigned m_nMagicNumber;  
		
		unsigned m_nNumRecords;

		unsigned m_nCompressionStatsFileOffset;
		unsigned m_nChunkIndexFileOffset;

		unsigned m_nBitBucket[32]; // reserved for future use, set to 0

		inline FileHeader()
		{
			memset(this, 0, sizeof(this));
			m_nMagicNumber = MagicNumber;
		}

		// class FileHeader
		static const unsigned MagicNumber;
	};

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class CompressZip4
	class CompressZip4
	{
		// the fields are in the following order.  It's easier to use an array in the code.
		// Zip5
		// Zip4Lo
		// Zip4Hi - Zip4Lo
		// StateFips
		// CountyFips
		// Tract
		// Block
		// Lat*100000
		// Long*100000
		// RecType
		// CentroidType

		RelCompressInt m_aCompressFields[NUM_FIELDS];

		GeoFile m_fileOutput;
		BitStreamWriteRef m_BitStreamOutput;

		unsigned m_nNumRecords;
		unsigned m_nCurrentRecordsInChunk;
		bool m_bStatisticsFinal;

		void EndChunkStatistics();

		std::vector<unsigned> m_vChunkIndex;
		void WriteChunk();

		int m_nLastZip9;

	public:
		CompressZip4();
		void Close();
		~CompressZip4();

		bool Create(const char *pFileName);

		// Add must be called twice for every record.
		// call it for EVERY record once and then call FinalizeStatistics
		// and then call it again for EVERY record
		bool Add(int *aFields);

		void FinalizeStatistics();
	};
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class DeCompressZip4
	class DeCompressZip4 : public RefCount
	{
		struct Zip9Key
		{
			int m_nZip5;
			int m_nZip4;

			inline Zip9Key(int nZip5, int nZip4)
				: m_nZip5(nZip5), m_nZip4(nZip4)
			{}

		};

		class FakeIntIterator : 
			public std::iterator<std::random_access_iterator_tag, int, int>
		{
			int m_val;
		public:
			inline FakeIntIterator(int val=0)
				: m_val(val)
			{
			}

			inline FakeIntIterator & operator =(int val)	{	m_val = val; return *this; }

			// Access to "value" pointed at by iterator.
			inline int & operator*() { return m_val; }
			
			// and just let everything like postfix ++, etc just default to using the int methods
			inline operator int &() { return m_val; }
		};

		class CompareForLowerBound
		{
			DeCompressZip4 &m_decompressZip4;

//			CompareForLowerBound(CompareForLowerBound &);
			CompareForLowerBound & operator =(CompareForLowerBound &);

		public:
			inline CompareForLowerBound(DeCompressZip4 &decompressZip4)
				: m_decompressZip4(decompressZip4)
			{
			}
			inline bool operator ()(int nRecA, int nRecB) const
			{
				return nRecA<nRecB;
			}
			bool operator ()(int nRec, const Zip9Key &zip9Key) const;
			bool operator ()(const Zip9Key &zip9Key, int nRec) const;
		};
		friend class CompareForLowerBound;
	protected:

		struct ChunkCachedData : public RefCount
		{
			typedef int TChunkArray[CHUNK_SIZE];
			TChunkArray aValues[NUM_FIELDS];
		};
		typedef refcnt_ptr<ChunkCachedData> ChunkCachedDataRef;

		// Generic "key" classes used by the caching mechanism
		struct IntKey {
		public:
			IntKey(int x_ = -1) : x(x_) {}
			unsigned int Hash() const { return x; }
			bool operator==(const IntKey& rhs) const { return x == rhs.x; }
			int x;
		};

		SetAssocCache<IntKey, ChunkCachedDataRef, 4> m_CachedChuncks;

		void ReadChunk(unsigned nChunkNumber, ChunkCachedDataRef & rChunkData);
		void GetRecord(int nRecord, ChunkCachedDataRef & rChunkData, int &rChunkRecordOffset);

		GeoFile  m_fileInput;
		BitStreamReadRef m_pBitStreamReader;
		FileHeader m_fileHeader;

		RelCompressInt m_aCompressFields[NUM_FIELDS];

	public:
		DeCompressZip4(int nCachePages=100);

		void Open(const char *pFileName); // throw Z9CoderException

		struct Results
		{
			// Buffer to hold combined results fields
			enum { 
				CensusIDBufferSize = 16,
				StateFipsBufferSize = 3,
				CountyFipsBufferSize = 4,
				TractResultBufferSize = 7,
				BlockResultBufferSize = 6,
				RecTypeResultBufferSize = 2,
				CenTypeResultBufferSize = 2,
			};
			char censusIDResultBuffer[CensusIDBufferSize];
			char stateFipsResultBuffer[StateFipsBufferSize];
			char countyFipsResultBuffer[CountyFipsBufferSize];
			char tractResultBuffer[TractResultBufferSize];
			char blockResultBuffer[BlockResultBufferSize];

			// H - high-rise
			// F - firm
			// S - street
			// R - rural route/highway contract
			// P - post office box
			// G - general delivery
			// Z - GDT 5-digit ZIP
			char recTypeResultBuffer[RecTypeResultBufferSize];

			//9, 7, 5 or 0 (0 probably will never happen)
			char cenTypeResultBuffer[CenTypeResultBufferSize];

			double m_dLatResult;
			double m_dLongResult;

			Results();

			void Reset();
		};
		///////////////////////////////////////////////////////////////////////////////
		// Function name	: Find
		// Description: 
		// Inputs:
		//	int			nZip5		Five-digit ZIP
		//	int			nZip4		Four-digit ZIP extension
		//	bool		returnFips	true to return FIPS codes
		// Outputs:
		//	Results&	results		The results
		// Return value:
		//	bool		true if a record was found, false if not 
		// throws Z9CoderException on error
		///////////////////////////////////////////////////////////////////////////////
		bool Find(
			int nZip5, 
			int nZip4, 
			Results & rResults, 
			bool bReturnFipsCodes=true
		);
	};
	typedef refcnt_ptr<DeCompressZip4> DeCompressZip4Ref;

} //namespace Zip9Coder

#endif //__ZIP9CODER_H__
