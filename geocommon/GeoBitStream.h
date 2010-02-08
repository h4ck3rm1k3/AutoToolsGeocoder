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

// BitStream.h:  Classes to read and write streams of bits.

#ifndef INCL_BITSTREAM_H
#define INCL_BITSTREAM_H

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif


#include "Geocoder_DllExport.h"
#include "../global/Basics.h"
#include "../global/RefPtr.h"
#include "../global/auto_ptr_array.h"
#include "GeoBitPtr.h"
#include "GeoAbstractByteIO.h"
#include <memory.h>
#include <assert.h>


namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////
	// Class to read bits from a byte stream
	///////////////////////////////////////////////////////////////////////////
	class BitStreamRead : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////
		// Constructor/destructor
		///////////////////////////////////////////////////////////////////////////
		BitStreamRead(ByteReaderRef byteReader_);
		virtual ~BitStreamRead();

		///////////////////////////////////////////////////////////////////////////
		// Read the next bit
		// Outputs:
		//	int&		value		The next bit value
		// Return value:
		//	bool					true if another bit was available, false o/w
		///////////////////////////////////////////////////////////////////////////
		bool NextBit(int &value) {
			if (current != endPtr) {
				value = *current;
				++current;
				return true;
			} else {
				return SlowNextBit(value);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		// Read the next N bits
		// Inptus:
		//	int				nbrBits			The number of bits to read.
		// Outputs:
		//	unsigned char*	returnBuffer	Filled with the bits read, starting
		//									with the LSB of the first byte
		// Return value:
		//	int			The number of bits actually read
		///////////////////////////////////////////////////////////////////////////
		int ReadBits(
			int nbrBits,
			unsigned char* returnBuffer
		) {
			if (endPtr - current >= nbrBits) {
				// Simple case
				BitPtr outPtr(returnBuffer, 0);
				for (int i = nbrBits-1; i >= 0; i--) {
					*outPtr = (int)*current;
					++current;
					++outPtr;
				}
				return nbrBits;
			} else {
				return SlowReadBits(nbrBits, returnBuffer);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		// Read N bits into a signed integer.  If the MSB is set, then the
		// value will be interpreted as a negative number and sign-extended.
		// Inputs:
		//	int		nbrBits		The number of bits to read
		//	int&	value		Integer into which bits will be loaded, starting
		//						with the LSB.
		// Return value:
		//	bool		true if the requested number of bits were read, false o/w.
		///////////////////////////////////////////////////////////////////////////
		bool ReadBitsIntoInt(int nbrBits, int& value) {
			assert(nbrBits > 0 && nbrBits <= 32);
			unsigned char tmp[4];
			memset(tmp, 0, 4);
			if (ReadBits(nbrBits, tmp) != nbrBits) {
				return false;
			}
			value = 
				tmp[0] | 
				(tmp[1] << 8) | 
				(tmp[2] << 16) | 
				(tmp[3] << 24); 
			if (value & signBit[nbrBits]) {
				// Sign-extend
				value |= signExtension[nbrBits];
			}
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Read N bits into an unsigned integer.
		// Inputs:
		//	int				nbrBits		The number of bits to read
		//	unsigned int	value		Integer into which bits will be loaded,
		//								starting with the LSB.
		// Return value:
		//	bool		true if the requested number of bits were read, false o/w.
		///////////////////////////////////////////////////////////////////////////
		bool ReadBitsIntoInt(int nbrBits, unsigned int& value) {
			unsigned char tmp[4];
			memset(tmp, 0, 4);
			if (ReadBits(nbrBits, tmp) != nbrBits) {
				return false;
			}
			value = 
				tmp[0] | 
				(tmp[1] << 8) | 
				(tmp[2] << 16) | 
				(tmp[3] << 24); 
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Seek to the given bit position
		///////////////////////////////////////////////////////////////////////////
		bool Seek(__int64 pos) {
			unsigned char c;
			return
				byteReader->Seek((int)(pos / 8)) &&
				SyncReader() &&
				ReadBits((unsigned int)pos % 8, &c) == (int)((unsigned int)pos % 8);		// Skip bits to start position.
		}

		///////////////////////////////////////////////////////////////////////////
		// Returns the current position in the file.
		///////////////////////////////////////////////////////////////////////////
		__int64 GetPosition() {
			return (__int64)byteReader->GetPosition() * 8 + (current - startPtr);
		}

		///////////////////////////////////////////////////////////////////////////
		// Skip forward the given number of bits.
		///////////////////////////////////////////////////////////////////////////
		bool Skip(int amount) {
			if (endPtr - current >= amount) {
				current += amount;
				return true;
			} else {
				return Seek(GetPosition() + amount);
			}
		}

	private:
		///////////////////////////////////////////////////////////////////////////
		// Read the next N bits.  Used when all the bits are not currently in memory.
		// Inptus:
		//	int				nbrBits			The number of bits to read.
		// Outputs:
		//	unsigned char*	returnBuffer	Filled with the bits read, starting
		//									with the LSB of the first byte
		// Return value:
		//	int			The number of bits actually read
		///////////////////////////////////////////////////////////////////////////
		int SlowReadBits(
			int nbrBits,
			unsigned char* returnBuffer
		);

		///////////////////////////////////////////////////////////////////////////
		// Re-sync the bitstream at the current Reader location.
		// Do this after seeking the byte reader.
		///////////////////////////////////////////////////////////////////////////
		bool SyncReader() {
			// Force buffer to be reloaded before next read
			current = BitPtr(buffer, 0);
			endPtr = current;
			return true;
		}


		///////////////////////////////////////////////////////////////////////////
		// Read the next bit, going out to the byte reader as necessary.
		// Outputs:
		//	int&		value		The next bit value
		// Return value:
		//	bool					true if another bit was available, false o/w
		///////////////////////////////////////////////////////////////////////////
		bool SlowNextBit(int &value);

		enum { DefaultBufferSize = 128 };
		int bufferSize;
		auto_ptr_array<unsigned char> buffer;
		BitPtr startPtr;
		BitPtr current;
		BitPtr endPtr;

		// Object from which bytes are read
		ByteReaderRef byteReader;

		// Masks and bits used for manipulating bits of integers
		// Indexed by nbrBits, not bit position.
		static unsigned int signExtension[33];
		static unsigned int signBit[33];
	};
	typedef refcnt_ptr<BitStreamRead> BitStreamReadRef;



	///////////////////////////////////////////////////////////////////////////
	// Class to write bits to a byte stream
	///////////////////////////////////////////////////////////////////////////
	class BitStreamWrite : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////
		// Constructor/destructor
		///////////////////////////////////////////////////////////////////////////
		BitStreamWrite(ByteWriterRef byteWriter_);
		virtual ~BitStreamWrite();

		///////////////////////////////////////////////////////////////////////////
		// Write the next bit
		// Inputs:
		//	int		value		The bit value to write 
		///////////////////////////////////////////////////////////////////////////
		void WriteBit(int value) {
			if (current != endPtr) {
				*current = value;
				++current;
				++bitsWritten;
			} else {
				SlowWriteBit(value);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		// Write the next N bits
		// Inputs:
		//	int						nbrBits		The number of bits to write
		//	const unsigned char*	buf			Buffer containing the bits, starting
		//										with the LSB of the first byte.
		///////////////////////////////////////////////////////////////////////////
		void WriteBits(int nbrBits, const unsigned char* buf);

		///////////////////////////////////////////////////////////////////////////
		// Write the lower N bits of an integer.  This works for both signed and
		// unsigned values.
		// Inputs:
		//	int		nbrBits		The number of bits to write
		//	int		value		Integer from which bits will be written, starting
		//						with the LSB.
		///////////////////////////////////////////////////////////////////////////
		void WriteBitsFromInt(int nbrBits, int value) {
			assert(nbrBits > 0 && nbrBits <= 32);
			unsigned char tmp[4];
			tmp[0] = (unsigned char)(value);
			tmp[1] = (unsigned char)(value >> 8);
			tmp[2] = (unsigned char)(value >> 16);
			tmp[3] = (unsigned char)(value >> 24);
			WriteBits(nbrBits, tmp);
		}

		///////////////////////////////////////////////////////////////////////////
		// Flush the bit stream and pad the last byte with zeros.
		///////////////////////////////////////////////////////////////////////////
		void Flush();

		///////////////////////////////////////////////////////////////////////////
		// Get the number of bits written
		///////////////////////////////////////////////////////////////////////////
		__int64 GetNumberOfBitsWritten() { return bitsWritten; }

	private:
		///////////////////////////////////////////////////////////////////////////
		// Write the next bit, going out to the byte writer as necessary.
		// Inputs:
		//	int			value		The bit value to write
		///////////////////////////////////////////////////////////////////////////
		void SlowWriteBit(int value);

		enum { DefaultBufferSize = 128 };
		int bufferSize;
		auto_ptr_array<unsigned char> buffer;
		BitPtr startPtr;
		BitPtr current;
		BitPtr endPtr;
		__int64 bitsWritten;

		// Object to which bytes are written
		ByteWriterRef byteWriter;

	};
	typedef refcnt_ptr<BitStreamWrite> BitStreamWriteRef;

}

#endif
