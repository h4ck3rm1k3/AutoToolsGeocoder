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
# $Rev: 72 $ 
# $Date: 2007-02-13 23:53:21 +0100 (Tue, 13 Feb 2007) $ 
*/

///////////////////////////////////////////////////////////////////////////////
//
// (C) 2002 SRC, LLC  -   All rights reserved
//
///////////////////////////////////////////////////////////////////////////////
//
// Module: ZIP9CODER.CPP
//
///////////////////////////////////////////////////////////////////////////////


#include "GeoCoder_Headers.h"
#include "GeoUtil.h"
#include "GeoAbstractByteIO.h"
#include "Z9CoderImp.h"
#include "Z9Coder.h"

namespace PortfolioExplorer
{
	class FILE_BitStreamAdaptor : public ByteWriter {
		FILE_BitStreamAdaptor(FILE_BitStreamAdaptor &);
		FILE_BitStreamAdaptor & operator =(FILE_BitStreamAdaptor &);
	public:
		FILE_BitStreamAdaptor(GeoFile & file)
			: m_file(file) 
		{}
		virtual bool Write(int size, const unsigned char* buffer) 
		{
			m_file.Write(buffer, size);
			return true;
		};
	private:
		GeoFile & m_file;
	};

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class FreqTableInt
	void FreqTableInt::SaveBinary(GeoFile &outFile)
	{
		unsigned nNumRecs = unsigned(freqTable.size());
		outFile.Write(&nNumRecs, sizeof(unsigned));

		for (iterator iter = begin(); iter != end(); ++iter) 
		{
			outFile.Write(&(iter->first), sizeof(int));
			outFile.Write(&(iter->second), sizeof(int));
		}
	}

	void FreqTableInt::LoadBinary(GeoFile &inFile)
	{
		freqTable.clear();
		unsigned nNumRecs;
		inFile.Read(&nNumRecs, sizeof(unsigned));
		for (unsigned x=0; x<nNumRecs; ++x)
		{
			int nVal;

			inFile.Read(&nVal, sizeof(int));
			int nCount;
			inFile.Read(&nCount, sizeof(int));
			freqTable.insert(std::map<int, int>::value_type(nVal, nCount));
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class RelCompressInt

	RelCompressInt::RelCompressInt()
		: m_nNumValuesInBuffer(0)
	{
	}


	void RelCompressInt::EndChunkStatistics()
	{
		int nLastVal = 0;
		int nCurrentRepeat = 1;

		for (unsigned nCurrentValue = 0; nCurrentValue<m_nNumValuesInBuffer; ++nCurrentValue)
		{
			int nVal = m_aValues[nCurrentValue];
			if (nCurrentValue!=0 && nVal==nLastVal)
			{
				nCurrentRepeat++;
			}
			else
			{
				if (nCurrentValue!=0 && nVal!=nLastVal)
					m_FreqRepeatCount.Count(nCurrentRepeat);

				FreqTableInt * pAFreq = nCurrentValue==0 ? m_aFreqInitialGroupValueBytes : m_aFreqValueBytes;
				
				int nCodeVal = int(nVal)-int(nLastVal);
				unsigned char tmpBuf[6];
				unsigned varIntLength = GeoUtil::IntToVarLengthBuf(nCodeVal, tmpBuf);
				pAFreq[0].Count(tmpBuf[0]);
				{for (unsigned i = 1; i < varIntLength; i++) {
					pAFreq[1].Count(tmpBuf[i]);
				}}

				nLastVal = nVal;
				nCurrentRepeat = 1;
			}
		}

		m_FreqRepeatCount.Count(nCurrentRepeat);
		m_nNumValuesInBuffer = 0;
	}

	void RelCompressInt::FinalizeStatistics()
	{
		// Generate Huffman codes
		for (unsigned x=0; x<2; ++x)
		{
			m_aHuffInitialGroupValueBytes[x].AddEntries(m_aFreqInitialGroupValueBytes[x]);
			m_aHuffInitialGroupValueBytes[x].MakeCodes();

			m_aHuffValueBytes[x].AddEntries(m_aFreqValueBytes[x]);
			m_aHuffValueBytes[x].MakeCodes();
		}

		m_HuffRepeatCount.AddEntries(m_FreqRepeatCount);
		m_HuffRepeatCount.MakeCodes();
		m_nNumValuesInBuffer = 0;
	}

	void RelCompressInt::WriteChunk(BitStreamWrite &bitStream)
	{
		int nLastVal = 0;
		int nCurrentRepeat = 1;

		for (unsigned nCurrentValue = 0; nCurrentValue<m_nNumValuesInBuffer; ++nCurrentValue)
		{
			int nVal = m_aValues[nCurrentValue];
			if (nCurrentValue!=0 && nVal==nLastVal)
			{
				nCurrentRepeat++;
			}
			else
			{
				if (nCurrentValue!=0 && nVal!=nLastVal)
					m_HuffRepeatCount.WriteCode(nCurrentRepeat, bitStream);

				HuffmanCoderInt * pAHuff = nCurrentValue==0 ? m_aHuffInitialGroupValueBytes : m_aHuffValueBytes;
				
				int nCodeVal = nVal-nLastVal;
				unsigned char tmpBuf[6];
				unsigned varIntLength = GeoUtil::IntToVarLengthBuf(nCodeVal, tmpBuf);
				pAHuff[0].WriteCode(tmpBuf[0], bitStream);
				{for (unsigned i = 1; i < varIntLength; i++) {
					pAHuff[1].WriteCode(tmpBuf[i], bitStream);
				}}

				nLastVal = nVal;
				nCurrentRepeat = 1;
			}
		}

		m_HuffRepeatCount.WriteCode(nCurrentRepeat, bitStream);
		m_nNumValuesInBuffer = 0;
	}

	void RelCompressInt::WriteStatistics(GeoFile &outFile)
	{
		// Generate Huffman codes
		for (unsigned x=0; x<2; ++x)
		{
			m_aFreqInitialGroupValueBytes[x].SaveBinary(outFile);
			m_aFreqValueBytes[x].SaveBinary(outFile);
		}

		m_FreqRepeatCount.SaveBinary(outFile);
	}

	void RelCompressInt::ReadStatistics(GeoFile &inFile)
	{
		for (unsigned x=0; x<2; ++x)
		{
			m_aFreqInitialGroupValueBytes[x].LoadBinary(inFile);
			m_aHuffInitialGroupValueBytes[x].AddEntries(m_aFreqInitialGroupValueBytes[x]);
			m_aHuffInitialGroupValueBytes[x].MakeCodes();
			
			m_aFreqValueBytes[x].LoadBinary(inFile);
			m_aHuffValueBytes[x].AddEntries(m_aFreqValueBytes[x]);
			m_aHuffValueBytes[x].MakeCodes();
		}

		m_FreqRepeatCount.LoadBinary(inFile);
		m_HuffRepeatCount.AddEntries(m_FreqRepeatCount);
		m_HuffRepeatCount.MakeCodes();
	}

	void RelCompressInt::ReadChunk(BitStreamRead &bitStream, unsigned nNumRecords, int *aResult)
	{
		int nLastVal = 0;
		unsigned nRecord=0;
		while(nRecord<nNumRecords)
		{
			HuffmanCoderInt * pAHuff = nRecord==0 ? m_aHuffInitialGroupValueBytes : m_aHuffValueBytes;
			GeoUtil::VarLengthBufToInt bufToInt;
			const int *pByte;;
			pAHuff[0].ReadCode(bitStream, pByte);
			bool bNeedMore = bufToInt.FirstByte(unsigned char(*pByte));
			while(bNeedMore)
			{
				pAHuff[1].ReadCode(bitStream, pByte);
				bNeedMore = bufToInt.NextByte(unsigned char(*pByte));
			}
			int nVal = bufToInt.GetSignedValue() + nLastVal;

			const int *p_nRepeat;
			m_HuffRepeatCount.ReadCode(bitStream, p_nRepeat);
			unsigned nEndRecord = (nRecord+*p_nRepeat);
			for (; nRecord<nEndRecord; ++nRecord)
				aResult[nRecord] = nVal;

			nLastVal = nVal;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class FileHeader
	const unsigned FileHeader::MagicNumber = 0x5A390002; // Z9 version 1


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class CompressZip4

	CompressZip4::CompressZip4()
		: m_nNumRecords(0), m_nCurrentRecordsInChunk(0), m_bStatisticsFinal(false)
	{
		m_nLastZip9 = 0;
	}

	bool CompressZip4::Create(const char *pFileName)
	{
		m_fileOutput.Open(pFileName, "wb");

		// we need to make sure we reserve space for this in the begining.
		// we will rewrite it later.
		FileHeader fileHeader;
		m_fileOutput.Write(&fileHeader, sizeof(fileHeader));

		// Create an output BitStream for the data file
		m_BitStreamOutput = new BitStreamWrite(new FILE_BitStreamAdaptor(m_fileOutput));

		return true;
	}

	void CompressZip4::EndChunkStatistics()
	{
		for (unsigned x=0; x<NUM_FIELDS; ++x)
		{
			m_aCompressFields[x].EndChunkStatistics();
		}
	}

	void CompressZip4::WriteChunk()
	{
		__int64 bitsWritten = m_BitStreamOutput->GetNumberOfBitsWritten();

		m_vChunkIndex.push_back(static_cast<unsigned>(bitsWritten));

		for (unsigned x=0; x<NUM_FIELDS; ++x)
		{
			m_aCompressFields[x].WriteChunk(*m_BitStreamOutput);
		}
		bitsWritten = m_BitStreamOutput->GetNumberOfBitsWritten();
		if (bitsWritten>__int64(0xffffffff))
			throw Z9CoderException("Fatal Internal Error:  File too large");
	}

	void CompressZip4::FinalizeStatistics()
	{
		m_nLastZip9 = 0;

		if (m_nCurrentRecordsInChunk>0)
			EndChunkStatistics();

		for (unsigned x=0; x<NUM_FIELDS; ++x)
		{
			m_aCompressFields[x].FinalizeStatistics();
		}
		m_bStatisticsFinal = true;
		m_nNumRecords = 0;
		m_nCurrentRecordsInChunk = 0 ;

	}

	bool CompressZip4::Add(int *aFields)
	{
		int nZip9 = aFields[0]*10000 + aFields[1];
		if (nZip9<m_nLastZip9)
			throw Z9CoderException("CompressZip4::Add: Records out of order.");
		m_nLastZip9 = nZip9;

		if (m_nCurrentRecordsInChunk==CHUNK_SIZE)
		{
			if (m_bStatisticsFinal)
				WriteChunk();
			else
				EndChunkStatistics();
			m_nCurrentRecordsInChunk = 0;
		}

		for (unsigned x=0; x<NUM_FIELDS; ++x)
		{
			m_aCompressFields[x].Add(aFields[x]);
		}

		++m_nNumRecords;
		++m_nCurrentRecordsInChunk;
		return true;
	}
	void CompressZip4::Close()
	{
		assert(m_nNumRecords>0 && m_bStatisticsFinal);
		if (m_fileOutput.IsOpen() && m_nNumRecords>0 && m_bStatisticsFinal)
		{
			// finish writing all the compressed data
			WriteChunk();
			m_BitStreamOutput->Flush();

			FileHeader fileHeader;
			fileHeader.m_nNumRecords = m_nNumRecords;

			// write the compression stats, saving the position
			fileHeader.m_nCompressionStatsFileOffset = m_fileOutput.Tell();
			for (unsigned x=0; x<NUM_FIELDS; ++x)
			{
				m_aCompressFields[x].WriteStatistics(m_fileOutput);
			}

			// write the Chunk index, saving the position
			fileHeader.m_nChunkIndexFileOffset = m_fileOutput.Tell();
			for (std::vector<unsigned>::const_iterator it = m_vChunkIndex.begin(); it != m_vChunkIndex.end(); it++)
			{
				m_fileOutput.Write(&(*it), sizeof(*it));
			}

			// and finally move back and rewrite the header
			m_fileOutput.Seek(0);
			m_fileOutput.Write(&fileHeader, sizeof(fileHeader));

			m_fileOutput.Close();
		}
	}

	CompressZip4::~CompressZip4()
	{
		Close();
	}
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class File_ByteReaderAdapter
	///////////////////////////////////////////////////////////////////////////
	// Adaptor class that provides bytes for the conversion
	///////////////////////////////////////////////////////////////////////////
	class File_ByteReaderAdapter : public ByteReader
	{
		File_ByteReaderAdapter(File_ByteReaderAdapter &);
		File_ByteReaderAdapter & operator =(File_ByteReaderAdapter &);
	public:
		// nStartBitStreamPos: if non 0, it fakes the bitstream reader
		// into thinking that the file does not start as the begining.
		inline File_ByteReaderAdapter(GeoFile & file, int nStartBitStreamPos)
			: m_file(file), m_nStartBitStreamPos(nStartBitStreamPos)
		{
		}
		// Read bytes from the stream.
		// Return the number of bytes successfully read.
		virtual int Read(int size,	unsigned char* returnBuffer)
		{
			m_file.Read(returnBuffer, size);
			return size;  // errors will be thrown with a Z9CoderException
		}

		// Seek to given position in the file.
		// Returns true on success, false on failure
		virtual bool Seek(int pos)
		{
			m_file.Seek(pos + m_nStartBitStreamPos);
			return true;  // errors will be thrown with a Z9CoderException
		}

		// Return the current position in the file.
		virtual int GetPosition()
		{
			return m_file.Tell()-m_nStartBitStreamPos; // errors will be thrown with a Z9CoderException
		}

	private:
		GeoFile & m_file;
		int m_nStartBitStreamPos;
	};

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// class DeCompressZip4
	DeCompressZip4::DeCompressZip4(int nCachePages /*=100*/)
			: m_CachedChuncks(nCachePages)
	{
	}

	void DeCompressZip4::ReadChunk(unsigned nChunkNumber, ChunkCachedDataRef & rChunkData)
	{
		int nNumRecordsInChunk = nChunkNumber==(m_fileHeader.m_nNumRecords/CHUNK_SIZE) ? m_fileHeader.m_nNumRecords%CHUNK_SIZE : CHUNK_SIZE;
		
		m_fileInput.Seek(m_fileHeader.m_nChunkIndexFileOffset + nChunkNumber*sizeof(unsigned));
		unsigned nPosChunk;
		m_fileInput.Read(&nPosChunk, sizeof(nPosChunk));

		m_pBitStreamReader->Seek(nPosChunk);

		if (rChunkData==NULL)
			rChunkData = new ChunkCachedData;

		for (unsigned x=0; x<NUM_FIELDS; ++x)
		{
			m_aCompressFields[x].ReadChunk(*m_pBitStreamReader, nNumRecordsInChunk, rChunkData->aValues[x]);
		}
	}
	
	void DeCompressZip4::GetRecord(int nRecord, ChunkCachedDataRef & rChunkData, int &rChunkRecordOffset)
	{
		assert(unsigned(nRecord)<m_fileHeader.m_nNumRecords);

		rChunkRecordOffset = nRecord%CHUNK_SIZE;
		int nChunkNumber = nRecord/CHUNK_SIZE;
		IntKey chunkKey(nChunkNumber);
		
		if (!m_CachedChuncks.Fetch(chunkKey, rChunkData))
		{
			ChunkCachedDataRef &pChunkDataChange = m_CachedChuncks.Change(chunkKey);
			ReadChunk(nChunkNumber, pChunkDataChange);
			rChunkData = pChunkDataChange;
		}
		assert(rChunkData!=NULL);
	}

	void DeCompressZip4::Open(const char *pFileName)
	{
		m_fileInput.Open(pFileName, "rb");
		m_fileInput.Read(&m_fileHeader, sizeof(m_fileHeader));

		if (m_fileHeader.m_nMagicNumber != FileHeader::MagicNumber)
			throw Z9CoderException("The file \"" + TsString(pFileName) + "\" is the wrong version or corrupt");

		ByteReaderRef readerRef(new File_ByteReaderAdapter(m_fileInput, sizeof(m_fileHeader)));
		m_pBitStreamReader = new BitStreamRead(readerRef);

		m_fileInput.Seek(m_fileHeader.m_nCompressionStatsFileOffset);

		for (unsigned x=0; x<NUM_FIELDS; ++x)
			m_aCompressFields[x].ReadStatistics(m_fileInput);
	}

	bool DeCompressZip4::CompareForLowerBound::operator ()(int nRec, const Zip9Key &zip9Key) const
	{
		ChunkCachedDataRef pChunkData;
		int nChunkRecordOffset;
		m_decompressZip4.GetRecord(nRec, pChunkData, nChunkRecordOffset);
		int nRecZip5 = pChunkData->aValues[0][nChunkRecordOffset];
		int nRecZip4Lo = pChunkData->aValues[1][nChunkRecordOffset];
		int nRecZip4Hi = nRecZip4Lo + pChunkData->aValues[2][nChunkRecordOffset];
		if (nRecZip5!=zip9Key.m_nZip5)
			return nRecZip5<zip9Key.m_nZip5;
		else
			return nRecZip4Hi<zip9Key.m_nZip4;
	}
	bool DeCompressZip4::CompareForLowerBound::operator ()(const Zip9Key &zip9Key, int nRec) const
	{
		ChunkCachedDataRef pChunkData;
		int nChunkRecordOffset;
		m_decompressZip4.GetRecord(nRec, pChunkData, nChunkRecordOffset);
		int nRecZip5 = pChunkData->aValues[0][nChunkRecordOffset];
		int nRecZip4Lo = pChunkData->aValues[1][nChunkRecordOffset];
		int nRecZip4Hi = nRecZip4Lo + pChunkData->aValues[2][nChunkRecordOffset];
		if (nRecZip5!=zip9Key.m_nZip5)
			return nRecZip5>zip9Key.m_nZip5;
		else
			return nRecZip4Hi>zip9Key.m_nZip4;
	}

	DeCompressZip4::Results::Results()
	{
		// better safe than sorry...
		Reset();

	}

	void DeCompressZip4::Results::Reset()
	{
		censusIDResultBuffer[0] = 0;
		stateFipsResultBuffer[0] = 0;
		countyFipsResultBuffer[0] = 0;
		tractResultBuffer[0] = 0;
		blockResultBuffer[0] = 0;
		cenTypeResultBuffer[0] = '0';
		cenTypeResultBuffer[1] = 0;

		m_dLatResult = 0.0;
		m_dLongResult = 0.0;
	}
	
	bool DeCompressZip4::Find(int nZip5, int nZip4, Results & rResults, bool bReturnFipsCodes /*=true*/)
	{
		rResults.Reset();

		Zip9Key key(nZip5, nZip4);
		unsigned nRecord = std::lower_bound(FakeIntIterator(0), FakeIntIterator(m_fileHeader.m_nNumRecords), key, CompareForLowerBound(*this));
		if (nRecord==m_fileHeader.m_nNumRecords)
			return false;

		ChunkCachedDataRef pChunkData;
		int nChunkRecordOffset;
		GetRecord(nRecord, pChunkData, nChunkRecordOffset);
		int nRecZip5 = pChunkData->aValues[0][nChunkRecordOffset];
		int nRecZip4Lo = pChunkData->aValues[1][nChunkRecordOffset];
		int nRecZip4Hi = nRecZip4Lo + pChunkData->aValues[2][nChunkRecordOffset];
		if (nZip5!=nRecZip5 || nZip4<nRecZip4Lo || nZip4>nRecZip4Hi)
			return false;

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
		// we have a match
		if (bReturnFipsCodes)
		{
			sprintf(rResults.stateFipsResultBuffer, "%02d", pChunkData->aValues[3][nChunkRecordOffset]);
			sprintf(rResults.countyFipsResultBuffer, "%03d", pChunkData->aValues[4][nChunkRecordOffset]);
			sprintf(rResults.tractResultBuffer, "%06d", pChunkData->aValues[5][nChunkRecordOffset]);
			sprintf(rResults.blockResultBuffer, "%04d", pChunkData->aValues[6][nChunkRecordOffset]);
			memcpy(rResults.censusIDResultBuffer, rResults.stateFipsResultBuffer, 2);
			memcpy(rResults.censusIDResultBuffer+2, rResults.countyFipsResultBuffer, 3);
			memcpy(rResults.censusIDResultBuffer+5, rResults.tractResultBuffer, 6);
			memcpy(rResults.censusIDResultBuffer+11, rResults.blockResultBuffer, 4);
			rResults.censusIDResultBuffer[15]=NULL;
		}

		rResults.m_dLatResult = double(pChunkData->aValues[7][nChunkRecordOffset])/100000.0;
		rResults.m_dLongResult = double(pChunkData->aValues[8][nChunkRecordOffset])/100000.0;



			// H - high-rise
			// F - firm
			// S - street
			// R - rural route/highway contract
			// P - post office box
			// G - general delivery
			// Z – GDT 5-digit ZIP
		static const char aRecTypes[] = {'H', 'F', 'S', 'R', 'P', 'G', 'Z'};
		assert(pChunkData->aValues[9][nChunkRecordOffset]<7);
		rResults.recTypeResultBuffer[0] = aRecTypes[pChunkData->aValues[9][nChunkRecordOffset] ];
		rResults.recTypeResultBuffer[1] = 0;


		static const char aCentroidTypes[] = {'0', '9', '7', '5'};
		assert(pChunkData->aValues[10][nChunkRecordOffset]<4);
		rResults.cenTypeResultBuffer[0] = aCentroidTypes[pChunkData->aValues[10][nChunkRecordOffset] ];
		rResults.cenTypeResultBuffer[1] = 0;

		return true;
	}

}

using namespace PortfolioExplorer;
namespace PinpointExplorer {

	int ZIP9_EXPORT _stdcall Z9_Load_Create(const char *pFileName, char *pError)
	{
		try
		{
			std::auto_ptr<CompressZip4> pRet(new CompressZip4);
			pRet->Create(pFileName);
			return reinterpret_cast<int>(pRet.release());
		} 
		catch (const ErrorException & ex) 
		{
			strncpy(pError, ex.message.c_str(), 256);
			pError[255] = 0;
			return false;
		}
	}

	int ZIP9_EXPORT _stdcall Z9_Load_Add(int handle, int *aFields)
	{
		try
		{
			CompressZip4 * p = reinterpret_cast<CompressZip4 *>(handle);
			return p->Add(aFields);
		} 
		catch (const ErrorException & /*ex*/) 
		{
			return false;
		}
	}

	void ZIP9_EXPORT _stdcall Z9_Load_FinalizeStatistics(int handle)
	{
		try
		{
			CompressZip4 * p = reinterpret_cast<CompressZip4 *>(handle);
			p->FinalizeStatistics();
		} 
		catch (const ErrorException & /*ex*/) 
		{
		}
	}

	int ZIP9_EXPORT _stdcall Z9_Load_Close(int handle, char *pError)
	{
		try
		{
			CompressZip4 * p = reinterpret_cast<CompressZip4 *>(handle);
			p->Close();
			delete p;
			return true;
		} 
		catch (const ErrorException & ex) 
		{
			strncpy(pError, ex.message.c_str(), 256);
			pError[255] = 0;
			return false;
		}
	}
}
