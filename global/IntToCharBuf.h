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

// IntToCharBuf.h: Platform-specific routines for moving an integer
// to/from a char buffer.

#ifndef INCL_JHINTTOCHARBUF_H
#define INCL_JHINTTOCHARBUF_H

#include "Basics.h"

namespace PortfolioExplorer {

	#if defined(_M_IX86) || defined(i386) || defined(__i386__)
		#define BASICS_LITTLE_ENDIAN 1
	#else
		#error "Must defined endian-ness of CPU"
	#endif

	///////////////////////////////////////////////////////////////////////////////
	// IntToCharBufMSBFirst: Store an int in a char buffer (using the same
	// number of bytes as the size of the integer), such that the MSB is stored
	// first, LSB last.  This facilitates storing and sorting of records that
	// contain integers, especially where integers must be aligned.
	// Inputs:
	//	unsigned int		i		The integer to store
	// Outputs:
	//	unsigned char*		buf		Buf where int will be stored MSB first
	///////////////////////////////////////////////////////////////////////////////
	inline void IntToCharBufMSBFirst(
		unsigned int i, 
		unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==1
			// little-endian
			buf[0] = ((unsigned char*)&i)[3];
			buf[1] = ((unsigned char*)&i)[2];
			buf[2] = ((unsigned char*)&i)[1];
			buf[3] = ((unsigned char*)&i)[0];
		#else
			// big-endian
			memcpy(buf, &i, 4);
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// CharBufToIntMSBFirst: Opposite of IntToCharBufMSBFirst
	// Outputs:
	//	unsigned char*		buf		Buf containing int
	// Inputs:
	//	unsigned int		The integer result
	///////////////////////////////////////////////////////////////////////////////
	inline unsigned int CharBufToIntMSBFirst(
		const unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==1
			// little-endian
			unsigned int i;
			((unsigned char*)&i)[3] = buf[0];
			((unsigned char*)&i)[2] = buf[1];
			((unsigned char*)&i)[1] = buf[2];
			((unsigned char*)&i)[0] = buf[3];
			return i;
		#else
			// Big-endian
			unsigned int i;
			memcpy(&i, buf, 4);
			return i;
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// Int64ToCharBufMSBFirst: Store an int in a char buffer (using the same
	// number of bytes as the size of the integer), such that the MSB is stored
	// first, LSB last.  This facilitates storing and sorting of records that
	// contain integers, especially where integers must be aligned.
	// Inputs:
	//	__uint64	i		The integer to store
	// Outputs:
	//	unsigned char*		buf		Buf where int will be stored MSB first
	///////////////////////////////////////////////////////////////////////////////
	inline void Int64ToCharBufMSBFirst(
		__uint64 i, 
		unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==1
			// little-endian
			buf[0] = ((unsigned char*)&i)[7];
			buf[1] = ((unsigned char*)&i)[6];
			buf[2] = ((unsigned char*)&i)[5];
			buf[3] = ((unsigned char*)&i)[4];
			buf[4] = ((unsigned char*)&i)[3];
			buf[5] = ((unsigned char*)&i)[2];
			buf[6] = ((unsigned char*)&i)[1];
			buf[7] = ((unsigned char*)&i)[0];
		#else
			// Big-endian
			memcpy(buf, &i, 8);
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// CharBufToInt64MSBFirst: Opposite of Int64ToCharBufMSBFirst
	// Outputs:
	//	unsigned char*		buf		Buf containing int
	// Inputs:
	//	__uint64	The integer result
	///////////////////////////////////////////////////////////////////////////////
	inline __uint64 CharBufToInt64MSBFirst(
		const unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==1
			// little-endian
			__uint64 i;
			((unsigned char*)&i)[7] = buf[0];
			((unsigned char*)&i)[6] = buf[1];
			((unsigned char*)&i)[5] = buf[2];
			((unsigned char*)&i)[4] = buf[3];
			((unsigned char*)&i)[3] = buf[4];
			((unsigned char*)&i)[2] = buf[5];
			((unsigned char*)&i)[1] = buf[6];
			((unsigned char*)&i)[0] = buf[7];
			return i;
		#else
			// Big-endian
			__uint64 i;
			memcpy(&i, buf, 8);
			return i;
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// ShortToCharBufMSBFirst: Store an short in a char buffer (using the same
	// number of bytes as the size of the integer), such that the MSB is stored
	// first, LSB last.  This facilitates storing and sorting of records that
	// contain integers, especially where integers must be aligned.
	// Inputs:
	//	unsigned short		i		The integer to store
	// Outputs:
	//	unsigned char*		buf		Buf where short will be stored MSB first
	///////////////////////////////////////////////////////////////////////////////
	inline void ShortToCharBufMSBFirst(
		unsigned short i, 
		unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==1
			// little-endian
			buf[0] = ((unsigned char*)&i)[1];
			buf[1] = ((unsigned char*)&i)[0];
		#else
			// Big-endian
			memcpy(buf, &i, 2);
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// CharBufToShortMSBFirst: Opposite of ShortToCharBufMSBFirst
	// Outputs:
	//	unsigned char*		buf		Buf containing short
	// Inputs:
	//	unsigned short		The integer result
	///////////////////////////////////////////////////////////////////////////////
	inline unsigned short CharBufToShortMSBFirst(
		const unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==1
			// little-endian
			unsigned short i;
			((unsigned char*)&i)[1] = buf[0];
			((unsigned char*)&i)[0] = buf[1];
			return i;
		#else
			// Big-endian
			unsigned short i;
			memcpy(&i, buf, 2);
			return i;
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// IntToCharBufLSBFirst: Store an int in a char buffer (using the same
	// number of bytes as the size of the integer), such that the LSB is stored
	// first, LSB last.  This facilitates storing and sorting of records that
	// contain integers, especially where integers must be aligned.
	// Inputs:
	//	unsigned int		i		The integer to store
	// Outputs:
	//	unsigned char*		buf		Buf where int will be stored LSB first
	///////////////////////////////////////////////////////////////////////////////
	inline void IntToCharBufLSBFirst(
		unsigned int i, 
		unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==0
			// big-endian
			buf[0] = ((unsigned char*)&i)[3];
			buf[1] = ((unsigned char*)&i)[2];
			buf[2] = ((unsigned char*)&i)[1];
			buf[3] = ((unsigned char*)&i)[0];
		#else
			// little-endian
			memcpy(buf, &i, 4);
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// CharBufToIntLSBFirst: Opposite of IntToCharBufLSBFirst
	// Outputs:
	//	unsigned char*		buf		Buf containing int
	// Inputs:
	//	unsigned int		The integer result
	///////////////////////////////////////////////////////////////////////////////
	inline unsigned int CharBufToIntLSBFirst(
		const unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==0
			// big-endian
			unsigned int i;
			((unsigned char*)&i)[3] = buf[0];
			((unsigned char*)&i)[2] = buf[1];
			((unsigned char*)&i)[1] = buf[2];
			((unsigned char*)&i)[0] = buf[3];
			return i;
		#else
			// little-endian
			unsigned int i;
			memcpy(&i, buf, 4);
			return i;
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// Int64ToCharBufLSBFirst: Store an int in a char buffer (using the same
	// number of bytes as the size of the integer), such that the LSB is stored
	// first, LSB last.  This facilitates storing and sorting of records that
	// contain integers, especially where integers must be aligned.
	// Inputs:
	//	__uint64	i		The integer to store
	// Outputs:
	//	unsigned char*		buf		Buf where int will be stored LSB first
	///////////////////////////////////////////////////////////////////////////////
	inline void Int64ToCharBufLSBFirst(
		__uint64 i, 
		unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==0
			// big-endian
			buf[0] = ((unsigned char*)&i)[7];
			buf[1] = ((unsigned char*)&i)[6];
			buf[2] = ((unsigned char*)&i)[5];
			buf[3] = ((unsigned char*)&i)[4];
			buf[4] = ((unsigned char*)&i)[3];
			buf[5] = ((unsigned char*)&i)[2];
			buf[6] = ((unsigned char*)&i)[1];
			buf[7] = ((unsigned char*)&i)[0];
		#else
			// little-endian
			memcpy(buf, &i, 8);
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// CharBufToInt64LSBFirst: Opposite of Int64ToCharBufLSBFirst
	// Outputs:
	//	unsigned char*		buf		Buf containing int
	// Inputs:
	//	__uint64	The integer result
	///////////////////////////////////////////////////////////////////////////////
	inline __uint64 CharBufToInt64LSBFirst(
		const unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==0
			// big-endian
			__uint64 i;
			((unsigned char*)&i)[7] = buf[0];
			((unsigned char*)&i)[6] = buf[1];
			((unsigned char*)&i)[5] = buf[2];
			((unsigned char*)&i)[4] = buf[3];
			((unsigned char*)&i)[3] = buf[4];
			((unsigned char*)&i)[2] = buf[5];
			((unsigned char*)&i)[1] = buf[6];
			((unsigned char*)&i)[0] = buf[7];
			return i;
		#else
			// little-endian
			__uint64 i;
			memcpy(&i, buf, 8);
			return i;
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// ShortToCharBufLSBFirst: Store an short in a char buffer (using the same
	// number of bytes as the size of the integer), such that the LSB is stored
	// first, LSB last.  This facilitates storing and sorting of records that
	// contain integers, especially where integers must be aligned.
	// Inputs:
	//	unsigned short		i		The integer to store
	// Outputs:
	//	unsigned char*		buf		Buf where short will be stored LSB first
	///////////////////////////////////////////////////////////////////////////////
	inline void ShortToCharBufLSBFirst(
		unsigned short i, 
		unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==0
			// big-endian
			buf[0] = ((unsigned char*)&i)[1];
			buf[1] = ((unsigned char*)&i)[0];
		#else
			// little-endian
			memcpy(buf, &i, 2);
		#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// CharBufToShortLSBFirst: Opposite of ShortToCharBufLSBFirst
	// Outputs:
	//	unsigned char*		buf		Buf containing short
	// Inputs:
	//	unsigned short		The integer result
	///////////////////////////////////////////////////////////////////////////////
	inline unsigned short CharBufToShortLSBFirst(
		const unsigned char* buf
	) {
		#if BASICS_LITTLE_ENDIAN==0
			// big-endian
			unsigned short i;
			((unsigned char*)&i)[1] = buf[0];
			((unsigned char*)&i)[0] = buf[1];
			return i;
		#else
			// little-endian
			unsigned short i;
			memcpy(&i, buf, 2);
			return i;
		#endif
	}

}

#endif

