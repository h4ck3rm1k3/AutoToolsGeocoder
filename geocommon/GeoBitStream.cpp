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
# $Rev: 52 $ 
# $Date: 2006-10-06 05:33:29 +0200 (Fri, 06 Oct 2006) $ 
*/

// BitStream.cpp:  Classes to read and write streams of bits.


#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "Geocoder_Headers.h"
#include <assert.h>
#include "GeoBitStream.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////
	// Indexed by nbrBits, not bit position.
	// Used to sign-extend numbers
	///////////////////////////////////////////////////////////////////////////
	unsigned int BitStreamRead::signBit[33] = {
		0u,
		1u << 0,
		1u << 1,
		1u << 2,
		1u << 3,
		1u << 4,
		1u << 5,
		1u << 6,
		1u << 7,
		1u << 8,
		1u << 9,
		1u << 10,
		1u << 11,
		1u << 12,
		1u << 13,
		1u << 14,
		1u << 15,
		1u << 16,
		1u << 17,
		1u << 18,
		1u << 19,
		1u << 20,
		1u << 21,
		1u << 22,
		1u << 23,
		1u << 24,
		1u << 25,
		1u << 26,
		1u << 27,
		1u << 28,
		1u << 29,
		1u << 30,
		1u << 31
	};
	unsigned int BitStreamRead::signExtension[33] = {
		0,
		0xFFFFFFFF << 1,
		0xFFFFFFFF << 2,
		0xFFFFFFFF << 3,
		0xFFFFFFFF << 4,
		0xFFFFFFFF << 5,
		0xFFFFFFFF << 6,
		0xFFFFFFFF << 7,
		0xFFFFFFFF << 8,
		0xFFFFFFFF << 9,
		0xFFFFFFFF << 10,
		0xFFFFFFFF << 11,
		0xFFFFFFFF << 12,
		0xFFFFFFFF << 13,
		0xFFFFFFFF << 14,
		0xFFFFFFFF << 15,
		0xFFFFFFFF << 16,
		0xFFFFFFFF << 17,
		0xFFFFFFFF << 18,
		0xFFFFFFFF << 19,
		0xFFFFFFFF << 20,
		0xFFFFFFFF << 21,
		0xFFFFFFFF << 22,
		0xFFFFFFFF << 23,
		0xFFFFFFFF << 24,
		0xFFFFFFFF << 25,
		0xFFFFFFFF << 26,
		0xFFFFFFFF << 27,
		0xFFFFFFFF << 28,
		0xFFFFFFFF << 29,
		0xFFFFFFFF << 30,
		0xFFFFFFFF << 31,
		0
	};


	///////////////////////////////////////////////////////////////////////////
	// Constructor/destructor
	///////////////////////////////////////////////////////////////////////////
	BitStreamRead::BitStreamRead(ByteReaderRef byteReader_) :
		byteReader(byteReader_)
	{
		bufferSize = DefaultBufferSize;
		buffer = new unsigned char[bufferSize];
		startPtr = BitPtr(buffer, 0);
		current = startPtr;
		endPtr = current;
	}

	BitStreamRead::~BitStreamRead() {}

	///////////////////////////////////////////////////////////////////////////
	// Read the next N bits, for use when all the bits are not in memory.
	// Inptus:
	//	int				nbrBits			The number of bits to read.
	// Outputs:
	//	unsigned char*	returnBuffer	Filled with the bits read, starting
	//									with the LSB of the first byte
	// Return value:
	//	int			The number of bits actually read
	///////////////////////////////////////////////////////////////////////////
	int BitStreamRead::SlowReadBits(
		int nbrBits,
		unsigned char* returnBuffer
	) {
		int bitsRemain = nbrBits;
		BitPtr outPtr(returnBuffer, 0);
		while (bitsRemain > 0) {
			int availBits = endPtr - current;
			if (availBits > bitsRemain) {
				availBits = bitsRemain;
			}
			bitsRemain -= availBits;
			// TODO: Optimize the bit copy using bytewise shift/mask
			while (availBits != 0) {
				*outPtr = (int)*current;
				availBits--;
				++current;
				++outPtr;
			}
			if (bitsRemain > 0) {
				// Fill the buffer again.
				int bytesRead = byteReader->Read(bufferSize, buffer);
				current = startPtr;
				endPtr = startPtr + bytesRead * 8;
				if (bytesRead == 0) {
					break;
				}
			}
		}
		return nbrBits - bitsRemain;
	}

	///////////////////////////////////////////////////////////////////////////
	// Read the next bit, going out to the byte reader as necessary.
	// Outputs:
	//	int&		value		The next bit value
	// Return value:
	//	bool					true if another bit was available, false o/w
	///////////////////////////////////////////////////////////////////////////
	bool BitStreamRead::SlowNextBit(int &value)
	{
		// Should only be called by NextBit() when empty.
		assert(current == endPtr);

		// Reload the buffer
		current = startPtr;
		int bytesRead = byteReader->Read(bufferSize, buffer);
		endPtr = startPtr + bytesRead * 8;

		if (bytesRead == 0) {
			return false;
		} else {
			value = *current;
			++current;
			return true;
		}
	}


	///////////////////////////////////////////////////////////////////////////
	// Constructor/destructor
	///////////////////////////////////////////////////////////////////////////
	BitStreamWrite::BitStreamWrite(ByteWriterRef byteWriter_) :
		byteWriter(byteWriter_)
	{
		bufferSize = DefaultBufferSize;
		buffer = new unsigned char[bufferSize];
		startPtr = BitPtr(buffer, 0);
		current = BitPtr(buffer, 0);
		endPtr = BitPtr(buffer + bufferSize, 0);
		bitsWritten = 0;
	}

	BitStreamWrite::~BitStreamWrite() 
	{
		Flush();
	}

	///////////////////////////////////////////////////////////////////////////
	// Write the next bit, going out to the byte writer as necessary.
	// Inputs:
	//	int		value		The bit value to write
	///////////////////////////////////////////////////////////////////////////
	void BitStreamWrite::SlowWriteBit(int value)
	{
		// Should only be called by WriteBit() when full.
		assert(current == endPtr);

		// Write the buffer
		byteWriter->Write((current - startPtr) / 8, buffer);

		// Reset the pointer
		current = startPtr;

		// Write the new bit.
		*current = value;
		++current;
		++bitsWritten;
	}

	///////////////////////////////////////////////////////////////////////////
	// Write the next N bits
	// Inputs:
	//	int						nbrBits		The number of bits to write
	//	const unsigned char*	inBuffer	Buffer containing the bits, starting
	//										with the LSB of the first byte.
	///////////////////////////////////////////////////////////////////////////
	void BitStreamWrite::WriteBits(
		int nbrBits, 
		const unsigned char* inBuffer
	) {
		BitCPtr inPtr(inBuffer, 0);
		while (nbrBits > 0) {
			int availBits = endPtr - current;
			if (availBits > nbrBits) {
				availBits = nbrBits;
			}
			nbrBits -= availBits;
			bitsWritten += availBits;
			// TODO: Optimize the bit copy using bytewise shift/mask
			while (availBits != 0) {
				*current = (int)*inPtr;
				availBits--;
				++current;
				++inPtr;
			}
			if (nbrBits > 0) {
				// Buffer must be full; write it out
				byteWriter->Write(bufferSize, buffer);
				current = startPtr;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Flush the bit stream and pad the last byte with zeros.
	///////////////////////////////////////////////////////////////////////////
	void BitStreamWrite::Flush()
	{
		// Pad last byte with zeros
		while ((current - startPtr) % 8 != 0) {
			*current = 0;
			++current;
			++bitsWritten;
		}
		assert(current <= endPtr);

		// Write the buffer
		if (current != startPtr) {
			byteWriter->Write((current - startPtr) / 8, buffer);
		}
		
		// Reset the pointer
		current = startPtr;
	}

}


