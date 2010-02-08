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

// IndexerBitset.h: Object that allocates and provides helper functions to a variable sized
//           bitset.

#ifndef INCL_BITSET_H
#define INCL_BITSET_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include <assert.h>
#include "auto_ptr_array.h"
#include "File.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class BitsetIterator;

	class Bitset : public VRefCount {
		friend class BitsetIterator;
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	int		size		Size of bitset to allocate
		///////////////////////////////////////////////////////////////////////////////
		Bitset(int inSize);

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~Bitset() {}
		
		///////////////////////////////////////////////////////////////////////////////
		// Copy Constructor:
		// Inputs:
		//	Bitset&			origin	bitset to copy from
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		Bitset(const Bitset&);

		///////////////////////////////////////////////////////////////////////////////
		// Set: Set a bit in the bitset
		// Inputs:
		//  int			pos		position of bit to be set
		// 
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Set(int pos)
		{
 			assert(pos <= iBits - 1 && pos >= 0);
			byteArray[pos >> 3] |= 1 << (pos & 7);
		}

		///////////////////////////////////////////////////////////////////////////////
		// SetRange: Set a range of bits
		// Inputs:
		//  int			start	first bit to be set
		//  int			end		one past last bit to be set
		///////////////////////////////////////////////////////////////////////////////
		void SetRange(int start, int end)
		{
			if (start - end > 30) {
				// Worthwhile to optimize this...
				// Set bits at beginning
				while ((start & 7) != 0) {
					Set(start++);
				}
				// Set bytes at a time
				int byteCount = (end >> 3) - (start >> 3);
				memset(byteArray.get() + (start >> 3), 0xFF, byteCount);
				start += (byteCount << 3);
				// Fall through to set bits at end
			}
			// Bit-at-a-time algorithm
			while (start < end) {
				Set(start++);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Reset: Flip a bit in the bitset
		// Inputs:
		//  int			pos		position of bit to flip
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Reset(int pos)
		{	
			assert(pos <= iBits - 1 && pos >= 0);
			byteArray[pos >> 3] &= ~(1 << (pos & 7));
		}

		///////////////////////////////////////////////////////////////////////////////
		// Clear: Clear out all bits
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Clear()
		{	
			memset(byteArray, 0, iBytes);
		}

		///////////////////////////////////////////////////////////////////////////////
		// IsSet: Return the value of a bit at given position
		// Inputs:
		//  int			pos		position of bit to check
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		bool IsSet(int pos) const
		{
			assert(pos <= iBits - 1 && pos >= 0);
			return (byteArray[pos >> 3] & (1 << (pos & 7))) != 0;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Size: Return the size of the bitset
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		int Size() const { return iBits; }

		///////////////////////////////////////////////////////////////////////////////
		// CountBits: Return the count of set bits in the bitset
		// Inputs:
		//
		// Return value:
		//	int 			number of bits set in this bitset
		///////////////////////////////////////////////////////////////////////////////
		int CountBits() const;

		///////////////////////////////////////////////////////////////////////////////
		// Not: Perform a logical not on the bitset
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Not();

		///////////////////////////////////////////////////////////////////////////////
		// Intersection: Perform an intersection with given bitset
		// Inputs:
		//  Bitset&			bitset	bitset to intersect on
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Intersection(Bitset& bitset);

		///////////////////////////////////////////////////////////////////////////////
		// Union: Perform a union with given bitset
		// Inputs:
		//  Bitset&			bitset	bitset to union on
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Union(Bitset& bitset);

		///////////////////////////////////////////////////////////////////////////////
		// Interface classes for saving and loading a bitset.
		///////////////////////////////////////////////////////////////////////////////
		class WriteAdaptor : public VRefCount {
		public:
			WriteAdaptor() : position(0) {}
			bool Write(int size, const char* data) {
				position += size;
				return SpecificWrite(size, data);
			}

			__int64 GetPosition() const { return position; }

		private:
			__int64 position;

			// Note: May throw an exception if writing to a File.
			// Otherwise, returns true on success, false on error
			virtual bool SpecificWrite(int size, const char* data) = 0;
		};
		typedef refcnt_ptr<WriteAdaptor> WriteAdaptorRef;

		class ReadAdaptor : public VRefCount {
		public:
			ReadAdaptor() : position(0) {}

			// Returns number of bytes actually read
			int Read(int size, char* data) {
				position += size;
				return SpecificRead(size, data);
			}

			__int64 GetPosition() const { return position; }

			// Skip forward size bytes
			void Skip(int size) {
				position += size;
				SpecificSkip(size);
			}
		private:
			__int64 position;
			// Returns number of bytes actually read
			virtual int SpecificRead(int size, char* data) = 0;
			// Skip forward size bytes
			virtual void SpecificSkip(int size) = 0;
		};
		typedef refcnt_ptr<ReadAdaptor> ReadAdaptorRef;

		///////////////////////////////////////////////////////////////////////////////
		// Special case of above for files.
		///////////////////////////////////////////////////////////////////////////////
		class WriteAdaptorFile : public WriteAdaptor {
		public:
			WriteAdaptorFile(File& file_) : file(file_) {}
			virtual bool SpecificWrite(int size, const char* data) {
				file.Write(size, data);
				return true;
			}
		private:
			File& file;
		};
		class ReadAdaptorFile : public ReadAdaptor{
		public:
			ReadAdaptorFile(File& file_) : file(file_) {}
		private:
			// Returns number of bytes actually read
			virtual int SpecificRead(int size, char* data) {
				return file.Read(size, data);
			}
			// Skip forward size bytes
			virtual void SpecificSkip(int size) {
				file.Seek(file.GetPosition() + size);
			}
		private:
			File& file;
		};

		///////////////////////////////////////////////////////////////////////////////
		// Bitset write adaptor which simply records the size of the data
		///////////////////////////////////////////////////////////////////////////////
		class WriteAdaptorGetSize : public WriteAdaptor {
		public:
		private:
			virtual bool SpecificWrite(int , const char*) {
				// don't need to implement anything because
				// base class tracks size.
				return true;
			}
		};


		///////////////////////////////////////////////////////////////////////////////
		// Write: Write bitset out to an adaptor
		// Inputs:
		//  WriteAdaptor&	adaptor	object to write bitset to
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool Write(WriteAdaptor& adaptor);

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
		bool Read(
			ReadAdaptor& adaptor,
			int minBound = 0,
			int maxBound = 0x7FFFFFFF,
			bool orInPlace = false
		);

		///////////////////////////////////////////////////////////////////////////////
		// Write: Write bitset out to a file
		// Inputs:
		//  File&		file		file to write bitset to (must be opened)
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool Write(File& file) {
			WriteAdaptorFile adaptor(file);
			return Write(adaptor);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Read: Read bitset from a file
		// Inputs:
		//  File&		 file		file to read bitset from (must be opened)
		//	int				minBound	index of first bit to read
		//	int				maxBound	one past index of last bit to read
		//	bool			orInPlace	if true, contents are ORed over existing contents.
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool Read(
			File& file,
			int minBound = 0,
			int maxBound = 0x7FFFFFFF,
			bool orInPlace = false
		) {
			ReadAdaptorFile adaptor(file);
			return Read(adaptor, minBound, maxBound, orInPlace);
		}

	private:
		// Bitset storage formats
		enum BitsetFormat {
			BitsetFormatBits,		// raw bits
			BitsetFormatRLE			// run-length encoded
		};

		///////////////////////////////////////////////////////////////////////////////
		// Choose the smallest storage format
		///////////////////////////////////////////////////////////////////////////////
		BitsetFormat ChooseSmallestFormat();

		///////////////////////////////////////////////////////////////////////////////
		// ReadBitsFormat: Read bitset from an adaptor in raw bits format
		// Inputs:
		//  ReadAdaptor&	adaptor		object to read bitset from
		//	int				minBound	index of first bit to read
		//	int				maxBound	one past index of last bit to read
		//	bool			orInPlace	if true, contents are ORed over existing contents.
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool ReadBitsFormat(
			ReadAdaptor& adaptor,
			int minBound,
			int maxBound,
			bool orInPlace
		);

		///////////////////////////////////////////////////////////////////////////////
		// ReadRLEFormat: Read bitset from an adaptor in RLE format
		// Inputs:
		//  ReadAdaptor&	adaptor		object to read bitset from
		//	int				minBound	index of first bit to read
		//	int				maxBound	one past index of last bit to read
		//	bool			orInPlace	if true, contents are ORed over existing contents.
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool ReadRLEFormat(
			ReadAdaptor& adaptor,
			int minBound,
			int maxBound,
			bool orInPlace
		);

		///////////////////////////////////////////////////////////////////////////////
		// Write: Write bitset out to an adaptor in raw bits format
		// Inputs:
		//  WriteAdaptor&	adaptor	object to write bitset to
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool WriteBitsFormat(WriteAdaptor& adaptor);

		///////////////////////////////////////////////////////////////////////////////
		// Write: Write bitset out to an adaptor in RLE format
		// Inputs:
		//  WriteAdaptor&	adaptor				object to write bitset to
		//
		// Return value:
		//	bool			true on success, false on error.
		///////////////////////////////////////////////////////////////////////////////
		bool WriteRLEFormat(
			WriteAdaptor& adaptor
		);

		///////////////////////////////////////////////////////////////////////////////
		// Convert bit count to byte count
		///////////////////////////////////////////////////////////////////////////////
		static int BitsToBytes(int bits) {
			int bytes = bits / 8;
			//a remainder means we need another unsigned char to hold all the bits
			if (bits % 8 > 0) {
				bytes++;
			}
			return bytes;
		}

		auto_ptr_array<unsigned char> byteArray; //safe array which contains the actual bitset data
		int							  iBits;     //number of bits in the array
		int                           iBytes;    //number of bytes in the array

		// Array containing counts per byte
		static int bitCounts[256];
	};
	typedef refcnt_ptr<Bitset> BitsetRef;


	///////////////////////////////////////////////////////////////////////////////
	// Iterator that walks through a bitset.
	// Note: do not inherit from VRefCount.  These are lightweight enough that
	// it is not necessary to ever allocate from the heap or use smart pointers.
	///////////////////////////////////////////////////////////////////////////////
	class BitsetIterator {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	Bitset&	bitset		bitset which this object will iterate over
		///////////////////////////////////////////////////////////////////////////////
		BitsetIterator(const Bitset* bitset_ = 0) : bitset(bitset_)
		{
			if (bitset != 0) {
				size = bitset->Size();
				size_1 = bitset->Size() - 1;
			}
			iPos = 0;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~BitsetIterator() {}
		
		///////////////////////////////////////////////////////////////////////////////
		// First: Returns the position of the first SET value in the bitset and sets the
		// internal iterator position to that spot
		// Inputs:
		//
		// Return value:
		//		int			next set bit in the bitset or -1 if no further bits are set
		///////////////////////////////////////////////////////////////////////////////
		int First();

		///////////////////////////////////////////////////////////////////////////////
		// Last: Returns the position of the last SET value in the bitset and sets the
		// internal iterator position to that spot
		// Inputs:
		//
		// Return value:
		//		int			last set bit in the bitset or -1 if no bits are set
		///////////////////////////////////////////////////////////////////////////////
		int Last();

		///////////////////////////////////////////////////////////////////////////////
		// Next: Returns the position of the next SET value in the bitset and sets the
		// internal iterator position to that spot
		// Inputs:
		//
		// Return value:
		//		int			next set bit in the bitset or -1 if no further bits are set
		///////////////////////////////////////////////////////////////////////////////
		int Next() {
			// Quick test of adjacent case first
			if (iPos < size_1 && bitset->IsSet(++iPos)) {
				return iPos;
			}
			return SlowNext();
		}
		int SlowNext();

		///////////////////////////////////////////////////////////////////////////////
		// Set the position of the iterator
		// Inputs:
		//	int		position
		// Return value:
		//		int			next set bit in the bitset or -1 if no further bits are set
		///////////////////////////////////////////////////////////////////////////////
		void SetPosition(int position) { iPos = position; }

		///////////////////////////////////////////////////////////////////////////////
		// IsDone: has internal position has reached the end of the bitset?
		// Inputs:
		//
		// Return value:
		//  bool		true if pointer is at end of bitset, else false
		///////////////////////////////////////////////////////////////////////////////
		bool IsDone()
		{
			assert(bitset);
			int iSavePos = iPos;
			if( Next() == -1 ) {
				return true;
			}
			iPos = iSavePos;
			return false;
		}

	private:
		const Bitset* bitset;	// bitset over which this object will iterate
		int	  iPos;				// current position in bitset
		int   size;				// cached from the bitset.
		int   size_1;			// == size-1
	};

} //namespace indexer

#endif
