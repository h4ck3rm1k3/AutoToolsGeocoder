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
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

// BitPtr.h:  Class that acts a pointer-to-bit.

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif


#include "Geocoder_DllExport.h"

#ifndef INCL_BITPTR_H
#define INCL_BITPTR_H

namespace PortfolioExplorer {

	// forward decl
	class BitPtr;
	int operator-(const BitPtr& rhs, const BitPtr& lhs);

	///////////////////////////////////////////////////////////////////////////
	// Class implementing a "reference-to-const-bit"
	///////////////////////////////////////////////////////////////////////////
	class BitCRef {
		friend class BitCPtr;

	public:
		// Copy constructor
		BitCRef(const BitCRef& rhs) : ptr(rhs.ptr), offset(rhs.offset) {}

		// Dereferencing
		operator int() const {
			return (*ptr >> offset) & 1;
		}
		operator bool() const {
			return (*ptr >> offset) & 1;
		}

	protected:
		// Construction from ptr/offset only by BitPtr or BitRef
		BitCRef(unsigned char* ptr_, int offset_) : ptr(ptr_), offset(offset_) {}

		unsigned char* ptr;
		int offset;

		static unsigned char offsetToMask[8];
	private:
		// No assignment allowed
		void operator=(const BitCRef &rhs);

	};

	///////////////////////////////////////////////////////////////////////////
	// Class implementing a "reference-to-bit"
	///////////////////////////////////////////////////////////////////////////
	class BitRef : public BitCRef {
		friend class BitPtr;

	public:
		// Copy constructor
		BitRef(const BitRef& rhs) : BitCRef(rhs) {}

		// Assignment (note no assignment from BitRef -- it's a reference after all).
		void operator=(int bit) {
			*ptr = (unsigned char)((*ptr & offsetToMask[offset]) | (bit << offset));
		}
		void operator=(bool bit) {
			*ptr = (unsigned char)((*ptr & offsetToMask[offset]) | ((int)bit << offset));
		}
		
	private:
		// Construction from ptr/offset only by BitPtr.
		BitRef(unsigned char* ptr_, int offset_) : BitCRef(ptr_, offset_) {}
	};

	///////////////////////////////////////////////////////////////////////////
	// Class implementing "const pointer-to-bit"
	///////////////////////////////////////////////////////////////////////////
	class BitCPtr {
		friend int operator-(const BitCPtr& rhs, const BitCPtr& lhs);
		friend void BitCopy(const BitPtr& dest, const BitCPtr& source, int count);

	public:
		BitCPtr() : ptr(0), offset(0) {}
		BitCPtr(const unsigned char* ptr_, int offset_) : 
			ptr((unsigned char*)ptr_), 
			offset(offset_) 
		{
			Normalize();
		}
		BitCPtr(const BitCPtr& rhs) : ptr(rhs.ptr), offset(rhs.offset) {}

		BitCPtr& operator=(const BitCPtr& rhs) {
			ptr = rhs.ptr;
			offset = rhs.offset;
			return *this;
		}

		bool operator==(const BitCPtr& rhs) {
			return ptr == rhs.ptr && offset == rhs.offset;
		}
		bool operator!=(const BitCPtr& rhs) {
			return ptr != rhs.ptr || offset != rhs.offset;
		}

		// Returns reference to the pointed-to bit.
		BitCRef operator*() const {
			return BitCRef(ptr, offset); 
		}
			
		// Returns reference to the addressed bit
		BitCRef operator[](int idx) const {
			BitCPtr tmp(*this);
			tmp += idx;
			return *tmp;
		}

		// prefix
		BitCPtr operator++() {
			++offset;
			Normalize();
			return *this;
		}
		// postfix
		BitCPtr operator++(int) {
			BitCPtr tmp(*this);
			++offset;
			Normalize();
			return tmp;
		}

		// prefix
		BitCPtr operator--() {
			--offset;
			Normalize();
			return *this;
		}
		// postfix
		BitCPtr operator--(int) {
			BitCPtr tmp(*this);
			--offset;
			Normalize();
			return tmp;
		}

		// In-place addition/subtraction
		BitCPtr& operator+=(int k) {
			offset += k;
			Normalize();
			return *this;
		}
		BitCPtr& operator-=(int k) {
			offset -= k;
			Normalize();
			return *this;
		}

		// Relational operators
		bool operator<(const BitCPtr& rhs) const {
			return 
				ptr < rhs.ptr ||
				ptr == rhs.ptr && offset < rhs.offset;
		}
		bool operator<=(const BitCPtr& rhs) const {
			return 
				ptr < rhs.ptr ||
				ptr == rhs.ptr && offset <= rhs.offset;
		}
		bool operator>(const BitCPtr& rhs) const {
			return 
				ptr > rhs.ptr ||
				ptr == rhs.ptr && offset > rhs.offset;
		}
		bool operator>=(const BitCPtr& rhs) const {
			return 
				ptr > rhs.ptr ||
				ptr == rhs.ptr && offset >= rhs.offset;
		}
			
	protected:
		unsigned char* ptr;
		int offset;

		// Adjust so offset is in range[0..7]
		void Normalize()
		{
			ptr += offset / 8;
			offset &= 7;
		}
	};


	///////////////////////////////////////////////////////////////////////////
	// Class implementing "pointer-to-bit"
	///////////////////////////////////////////////////////////////////////////
	class BitPtr : public BitCPtr {
		friend int operator-(const BitPtr& rhs, const BitPtr& lhs);
	public:
		BitPtr() {}
		BitPtr(unsigned char* ptr_, int offset_) : BitCPtr(ptr_, offset_) {}
		BitPtr(const BitPtr& rhs) : BitCPtr(rhs) {}

		BitPtr& operator=(const BitPtr& rhs) {
			BitCPtr::operator=(rhs);
			return *this;
		}

		bool operator==(const BitPtr& rhs) {
			return ptr == rhs.ptr && offset == rhs.offset;
		}
		bool operator!=(const BitPtr& rhs) {
			return ptr != rhs.ptr || offset != rhs.offset;
		}

		// Returns reference to the pointed-to bit.
		BitRef operator*() const {
			return BitRef(ptr, offset); 
		}
			
		// Returns reference to the addressed bit
		BitRef operator[](int idx) const {
			BitPtr tmp(*this);
			tmp += idx;
			return *tmp;
		}

		// prefix
		BitPtr operator++() {
			++offset;
			Normalize();
			return *this;
		}
		// postfix
		BitPtr operator++(int) {
			BitPtr tmp(*this);
			++offset;
			Normalize();
			return tmp;
		}

		// prefix
		BitPtr operator--() {
			--offset;
			Normalize();
			return *this;
		}
		// postfix
		BitPtr operator--(int) {
			BitPtr tmp(*this);
			--offset;
			Normalize();
			return tmp;
		}

		// In-place addition/subtraction
		BitPtr& operator+=(int k) {
			offset += k;
			Normalize();
			return *this;
		}
		BitPtr& operator-=(int k) {
			offset -= k;
			Normalize();
			return *this;
		}

		// Relational operators
		bool operator<(const BitPtr& rhs) const {
			return 
				ptr < rhs.ptr ||
				ptr == rhs.ptr && offset < rhs.offset;
		}
		bool operator<=(const BitPtr& rhs) const {
			return 
				ptr < rhs.ptr ||
				ptr == rhs.ptr && offset <= rhs.offset;
		}
		bool operator>(const BitPtr& rhs) const {
			return 
				ptr > rhs.ptr ||
				ptr == rhs.ptr && offset > rhs.offset;
		}
		bool operator>=(const BitPtr& rhs) const {
			return 
				ptr > rhs.ptr ||
				ptr == rhs.ptr && offset >= rhs.offset;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// Global methods
	///////////////////////////////////////////////////////////////////////////

	// Addition
	inline BitPtr operator+(const BitPtr& rhs, int lhs) {
		BitPtr tmp(rhs);
		tmp += lhs;
		return tmp;
	}
	inline BitPtr operator-(const BitPtr& rhs, int lhs) {
		BitPtr tmp(rhs);
		tmp -= lhs;
		return tmp;
	}

	// Subtraction
	inline int operator-(const BitPtr& rhs, const BitPtr& lhs) {
		int diff = int((rhs.ptr - lhs.ptr) * 8);
		diff += (rhs.offset - lhs.offset);
		return diff;
	}

#if 0
	// Copying of bits from source to destination
	inline void BitCopy(const BitPtr& dest_, const BitCPtr& source_, int count) 
	{
		// Make copies
		BitPtr dest(dest_);
		BitPtr source(source_);
	}
#endif

}


#endif
