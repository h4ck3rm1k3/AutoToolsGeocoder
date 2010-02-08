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

// Bitset.cpp: bitset class and supporting functions

#include "Global_Headers.h"
#include "BitSet.h"
#include "Basics.h"

namespace PortfolioExplorer {

	int Bitset::bitCounts[256];

	///////////////////////////////////////////////////////////////////////////////
	// Constructor:
	// Inputs:
	//	int				inSize	Size of bitset to allocate
	///////////////////////////////////////////////////////////////////////////////
	Bitset::Bitset(int inSize) 
	{
		//Create the unsigned char array.  Note that inSize defines the number of bits, NOT bytes
		//so we only want to allocate iSize / 8 
		iBits = inSize;
		iBytes = BitsToBytes(inSize);

		byteArray = new unsigned char[iBytes];
		memset(byteArray, 0, iBytes);

		// Initialize bit counts array if first time through
		if (bitCounts[1] == 0) {
			for (int i = 0; i < 256; i++) {
				int count = 0;
				unsigned char c = i;
				for (int j = 0; j < 8; j++) {
					if( c & 1 ) {
						count++;
					}
					c >>= 1;
				}
				bitCounts[i] = count;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Copy Constructor:
	// Inputs:
	//	Bitset&			origin	bitset to copy from
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	Bitset::Bitset(const Bitset& origin) {
		byteArray = new unsigned char[origin.iBytes];
		memcpy((void*)byteArray, (void*)origin.byteArray, origin.iBytes);
		iBits = origin.iBits;
		iBytes = origin.iBytes;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Not: Perform a logical not on the bitset
	// Inputs:
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void Bitset::Not()
	{
		// Not every byte until the last one (which may have extra bits not 
		// included in the bitset)
		{for (int i = 0; i < iBytes; i++) {
			byteArray[i] = ~(byteArray[i]);
		}}
		// Reset the last bits
		{for (int i = iBits; i < iBytes * 8;i++) { 
			byteArray[i >> 3] &= ~(1 << (i & 7));
		}}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Intersection: Perform an intersection with given bitset
	// Inputs:
	//  Bitset&			bitset	bitset to intersect on
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void Bitset::Intersection(Bitset& bitset)
	{
		assert(iBytes == bitset.iBytes);
		for(int i = 0; i < iBytes; i++) {
			byteArray[i] &= bitset.byteArray[i];
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Union: Perform a union with given bitset
	// Inputs:
	//  Bitset&			bitset	bitset to union on
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void Bitset::Union(Bitset& bitset)
	{
		assert(iBytes == bitset.iBytes);
		for(int i = 0; i < iBytes; i++) {
			byteArray[i] |= bitset.byteArray[i];
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// CountBits: Return the count of set bits in the bitset
	// Inputs:
	//
	// Return value:
	//	int 			number of bits set in this bitset
	///////////////////////////////////////////////////////////////////////////////
	int Bitset::CountBits() const{
		
		int iBitCount = 0;
		for (int i = 0; i < iBytes; i++) {
			iBitCount += bitCounts[byteArray[i]];
		}
		return iBitCount;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Write: Write bitset out to an adaptor
	// Inputs:
	//  WriteAdaptor&	adaptor	object to write bitset to
	//
	// Return value:
	//	bool			true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool Bitset::Write(WriteAdaptor& adaptor)
	{
		if (ChooseSmallestFormat() == BitsetFormatRLE) {
			return WriteRLEFormat(adaptor);
		} else {
			return WriteBitsFormat(adaptor);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Choose the smallest storage format
	///////////////////////////////////////////////////////////////////////////////
	Bitset::BitsetFormat Bitset::ChooseSmallestFormat()
	{
		// Try both methods to dummay adaptor and see which is smallest.
		WriteAdaptorGetSize bitsSizeAdaptor;
		WriteAdaptorGetSize RLESizeAdaptor;
		WriteBitsFormat(bitsSizeAdaptor);
		WriteRLEFormat(RLESizeAdaptor);

		// Prefer bits format since it seeks better
		if (RLESizeAdaptor.GetPosition() < bitsSizeAdaptor.GetPosition() / 4) {
			return BitsetFormatRLE;
		} else {
			return BitsetFormatBits;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Write: Write bitset out to an adaptor in raw bits format
	// Inputs:
	//  WriteAdaptor&	adaptor	object to write bitset to
	//
	// Return value:
	//	bool			true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool Bitset::WriteBitsFormat(WriteAdaptor& adaptor)
	{
		int type = BitsetFormatBits;
		return 
			adaptor.Write(sizeof(int), (const char*)&type) &&
			adaptor.Write(sizeof(int), (const char*)&iBytes) &&
			adaptor.Write(sizeof(int), (const char*)&iBits) &&
			adaptor.Write(iBytes, (const char*)byteArray.get());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Write: Write bitset out to an adaptor in RLE format
	// Inputs:
	//  WriteAdaptor&	adaptor				object to write bitset to
	//
	// Return value:
	//	bool			true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool Bitset::WriteRLEFormat(
		WriteAdaptor& adaptor
	) {
		int type = BitsetFormatRLE;
		if (
			!adaptor.Write(sizeof(int), (const char*)&type) ||
			!adaptor.Write(sizeof(int), (const char*)&iBits)
		) {
			return false;
		}

		BitsetIterator iter(this);
		for (int pos = iter.First(); pos >= 0; pos = iter.Next()) {
			unsigned int code = pos;
			pos++;
			// Optimize byte-wise compression if possible.
			// First do the odd bits at front.
			while (pos < iBits && (pos & 7) != 0 && IsSet(pos)) {
				pos++;
			}
			if (pos < iBits && (pos & 7) == 0 && IsSet(pos)) {
				// We have a chance for byte-wise compression
				int byteIdx = pos >> 3;
				while (byteArray[byteIdx] == 0xFF) {
					byteIdx++;
				}
				pos = byteIdx << 3;
			}
			// Do end bits
			while (pos < iBits && IsSet(pos)) {
				pos++;
			}
			int totalCount = pos - code;
			iter.SetPosition(pos);
			if (totalCount > 1) {
				code |= 0x80000000;
				while (totalCount != 0) {
					unsigned short count = (totalCount > 0xFFFE) ? 0xFFFE : totalCount;
					totalCount -= count;
					if (
						!adaptor.Write(sizeof(unsigned int), (const char*)&code) ||
						!adaptor.Write(sizeof(unsigned short), (const char*)&count)
					) {
						return false;
					}
					code += count;
				}
			} else {
				if (!adaptor.Write(sizeof(unsigned int), (const char*)&code)) {
					return false;
				}
			}
		}
		unsigned int finalCode = 0xFFFFFFFF;
		return adaptor.Write(sizeof(unsigned int), (const char*)&finalCode);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Read: Read bitset from an adaptor
	// Inputs:
	//  ReadAdaptor&	adaptor		object to read bitset from
	//	int				minBound	index of first bit to read
	//	int				maxBound	one past index of last bit to read
	//	bool			orInPlace	if true, contents are ORed over existing contents.
	//
	// Return value:
	//	bool			true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool Bitset::Read(
		ReadAdaptor& adaptor,
		int minBound,
		int maxBound,
		bool orInPlace
	) {
		int type;
		if (adaptor.Read(sizeof(int), (char*)&type) != sizeof(int)) {
			return false;
		}

		switch (type) {
		case BitsetFormatRLE:
			return ReadRLEFormat(adaptor, minBound, maxBound, orInPlace);
		case BitsetFormatBits:
			return ReadBitsFormat(adaptor, minBound, maxBound, orInPlace);
		default:
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// ReadBitsFormat: Read bitset from an adaptor in the raw bits format
	// Inputs:
	//  ReadAdaptor&	adaptor		object to read bitset from
	//	int				minBound	index of first bit to read
	//	int				maxBound	one past index of last bit to read
	//	bool			orInPlace	if true, contents are ORed over existing contents.
	//
	// Return value:
	//	bool			true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool Bitset::ReadBitsFormat(
		ReadAdaptor& adaptor,
		int minBound,
		int maxBound,
		bool orInPlace
	) {
		int newByteCount;
		int newBitCount;
		if (
			adaptor.Read(sizeof(int), (char*)&newByteCount) != sizeof(int) ||
			adaptor.Read(sizeof(int), (char*)&newBitCount) != sizeof(int)
		) {
			return false;
		}

		if (iBits != newBitCount) {
			assert(!orInPlace);
			iBits = newBitCount;
			iBytes = newByteCount;
			byteArray = 0;
			byteArray = new unsigned char[iBytes];
		}
		
		// Skip front end if not needed
		if (minBound >= iBits) {
			minBound = iBits;
		}
		int minByte = minBound / 8;
		if (minByte != 0) {
			if (!orInPlace) {
				memset(byteArray, 0, minByte);
			}
			adaptor.Skip(minByte);
		}

		// Determine number of bytes needed
		if (maxBound > iBits) {
			maxBound = iBits;
		}
		int maxByte = (maxBound - 1) / 8 + 1;
		int bytesToRead = maxByte - minByte;

		// Read center section of bytes
		if (orInPlace) {
			char buf[1024];
			char* ptr = (char*)byteArray.get() + minByte;
			while (bytesToRead != 0) {
				int n = JHMIN(bytesToRead, (int)sizeof(buf));
				if (adaptor.Read(n, buf) != n) {
					return false;
				}
				char *ptrFrom = buf;
				char *ptrEnd = ptr + n;
				while (ptr < ptrEnd) {
					*ptr++ |= *ptrFrom++;
				}
				bytesToRead -= n;
			}
		} else {
			if (adaptor.Read(bytesToRead, (char*)byteArray.get() + minByte) != bytesToRead) {
				return false;
			}
		}

		// Reset extra bytes
		if (!orInPlace && maxByte < iBytes) {
			memset(byteArray.get() + maxByte, 0, iBytes - maxByte);
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// ReadRLEFormat: Read bitset from an adaptor in the RLE format
	// Inputs:
	//  ReadAdaptor&	adaptor		object to read bitset from
	//	int				minBound	index of first bit to read
	//	int				maxBound	one past index of last bit to read
	//	bool			orInPlace	if true, contents are ORed over existing contents.
	//
	// Return value:
	//	bool			true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool Bitset::ReadRLEFormat(
		ReadAdaptor& adaptor,
		int minBound,
		int maxBound,
		bool orInPlace
	) {
		int newBitCount;
		if (adaptor.Read(sizeof(int), (char*)&newBitCount) != sizeof(int)) {
			return false;
		}
		
		if (iBits != newBitCount) {
			assert(!orInPlace);
			iBits = newBitCount;
			iBytes = BitsToBytes(newBitCount);
			byteArray = 0;
			byteArray = new unsigned char[iBytes];
		}
		if (!orInPlace) {
			memset(byteArray, 0, iBytes);
		}

		unsigned int code;
		unsigned short count;

		while (true) {
			if (adaptor.Read(sizeof(unsigned int), (char*)&code) != sizeof(int)) {
				return false;
			}
			if ((code & 0x80000000) != 0) {
				// Count or finish marker
				if (code == 0xFFFFFFFF) {
					// We are done
					return true;
				}
				code &= 0x7FFFFFFF;
				if (code >= unsigned(maxBound)) {
					// Past upper bound
					return true;
				}
				// We have a count
				if (adaptor.Read(sizeof(unsigned short), (char*)&count) != sizeof(unsigned short)) {
					return false;
				}
				if (code + count > unsigned(minBound)) {
					SetRange(code, code + count);
				}
			} else {
				if (code < unsigned(minBound)) {
					continue;
				}
				if (code >= unsigned(maxBound)) {
					return true;
				}
				// Single-bit case
				Set(code);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// First: Returns the position of the first SET value in the bitset and sets the
	// internal iterator position to that spot
	// Inputs:
	//
	// Return value:
	//		int			next set bit in the bitset or -1 if no further bits are set
	///////////////////////////////////////////////////////////////////////////////
	int BitsetIterator::First() 
	{
		assert(bitset);
		
		int byte = 0;
		while ((byte < bitset->iBytes - 1) && bitset->byteArray[byte] == 0) {
			byte++;
		}
		if( bitset->byteArray[byte] == 0 ) {
			return -1;
		}

		iPos = byte * 8;
		while ((iPos < (bitset->Size() - 1))  && !bitset->IsSet(iPos)) {
			iPos++;
		}

		if (bitset->IsSet(iPos)) {
			return iPos;
		}

		return -1;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Last: Returns the position of the last SET value in the bitset and sets the
	// internal iterator position to that spot
	// Inputs:
	//
	// Return value:
	//		int			last set bit in the bitset or -1 if no bits are set
	///////////////////////////////////////////////////////////////////////////////
	int BitsetIterator::Last() 
	{
		assert(bitset);
		
		int byte = bitset->iBytes - 1;
		while( byte > 0 && bitset->byteArray[byte] == 0 ) {
			byte --;
		}
		if( bitset->byteArray[byte] == 0 ) {
			return -1;
		}

		iPos = JHMIN(byte * 8 + 7, size_1);
		while (iPos > 0  && !bitset->IsSet(iPos)) {
			iPos--;
		}

		if (bitset->IsSet(iPos)) {
			return iPos;
		}

		return -1;
	}

	///////////////////////////////////////////////////////////////////////////////
	// SlowNext: Returns the position of the next SET value in the bitset and 
	// sets the internal iterator position to that spot.  Called from Next()
	// when fast test fails.
	// Inputs:
	//
	// Return value:
	//		int			next set bit in the bitset or -1 if no further bits are set
	///////////////////////////////////////////////////////////////////////////////
	int BitsetIterator::SlowNext() 
	{
		assert(bitset);

		if (++iPos >= size) {
			return -1;
		}

		// Check bits in current byte first
		int currentByte = iPos >> 3;
		{
			int limit = JHMIN((currentByte + 1) << 3, size);
			for (; iPos < limit; iPos++) {
				if (bitset->IsSet(iPos)) {
					return iPos;
				}
			}
		}
		assert(iPos == size || iPos == (currentByte + 1) * 8);
				
		// Skip bytes
		for (
			++currentByte;
			currentByte < bitset->iBytes && bitset->byteArray[currentByte] == 0;
			currentByte++
		) {}

		assert(iPos <= currentByte * 8);
		iPos = currentByte << 3;

		// Skip bits
		while (true) {
			if (iPos >= size) {
				return -1;
			} else if (bitset->IsSet(iPos)) {
				return iPos;
			}
			iPos++;
		}
		return -1;
	}

}

