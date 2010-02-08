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
# $Rev: 49 $ 
# $Date: 2006-09-25 20:00:58 +0200 (Mon, 25 Sep 2006) $ 
*/

// Thread-safe string header file.
// Implements a thread-safe string that follows the STL API.
// However, there are some notes to make:
// 1) It assumes that the template parameter ELT is POD (plain old data*) and scalar.
// 2) It uses new/delete instead of parameterized allocators
// 3) It uses ELT* as the iterator type.
// 4) It omits reverse_iterator
// 5) It omits a lot of rarely-used stuff
// 6) It uses no traits

// *Go read the C++ ARM if you want to know about POD

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef INCL_TSSTRING_H
#define INCL_TSSTRING_H

#if defined(UNIX)
	#include <string>
	namespace PortfolioExplorer {
	typedef std::string TsString;
	}
#else
	
	#ifdef TSSTRING_STATS
		#include <ace/Atomic_Op.h>
	#endif
	
	#include <memory.h>
	#include <assert.h>
	#include <string>
	#include "RefPtr.h"
	#include "Basics.h"
	#include <ostream>
	#include "Global_DllExport.h"
	
	namespace PortfolioExplorer {
	
	// Template that defines basic types and static helper methods
	// This is a lot like the STL traits stuff.
	// Note: this can be specialized to optimize for e.g. char.
	template<class ELT> class TsString_Base {
	public:
		typedef size_t size_type;
		typedef int difference_type;
		typedef ELT* pointer;
		typedef const ELT* const_pointer;
		typedef ELT& reference;
		typedef const ELT& const_reference;
		typedef ELT* iterator;
		typedef const ELT* const_iterator;
		typedef ELT value_type;
	
		// Like strlen()
		static inline size_type StrLen(const ELT* str)
		{
			size_type size = 0;
			while (*str != 0) {
				str++;
				size++;
			}
			return size;
		}
	
		// Copies without termination.
		static inline void Copy(ELT* dest, const ELT* src, size_type length) {
			memcpy(dest, src, sizeof(ELT) * length);
		}
	
		// Fills without termination.
		static inline void Fill(ELT* dest, size_type count, ELT elt) {
			while (count != 0) {
				count--;
				*dest++ = elt;
			}
		}
	
		// Moves segment using non-overlapping logic
		static inline void Move(ELT* dest, const ELT* source, size_type length) {
			memmove(dest, source, length * sizeof(ELT));
		}
	
		// Compares a sequence, does not check for null termination
		// Similar to memcmp(). but operates on chars.
		static inline int Cmp(const ELT* str1, const ELT* str2, size_type length) {
			while (length != 0) {
				length--;
				if (*str1 != *str2) {
					return (int)*str1 - (int)*str2;
				}
				str1++;
				str2++;	
			}
			return 0;
		}
	
		// Compares a analogous to strcmp()
		static inline int StrCmp(const ELT* str1, const ELT* str2) {
			while (*str1 != 0 && *str1 == *str2) {
				str1++;
				str2++;
			}
			return (int)*str1 - (int)*str2;
		}
	
	};
	
	
	// Performance specialization for common <char> usage.
	// On Intel P4 CPU this seems to be slower than C++ inlines defined above.
	/*
	template<> inline TsString_Base<char>::size_type TsString_Base<char>::StrLen(const ELT* str) {
		return strlen(str);
	}
	template<> inline int TsString_Base<char>::Cmp(const char* str1, const char* str2, size_type length) {
		return memcmp(str1, str2, length);
	}
	template<> inline int TsString_Base<char>::StrCmp(const char* str1, const char* str2) {
		return strcmp(str1, str2);
	}
	template<> inline void TsString_Base<char>::Fill(ELT* dest, size_type count, ELT elt) {
		memset(dest, elt, count);
	}
	*/
	
	// Base class for string body, used so we can calculate class size without buffer.
	template<class ELT> class TsString_Body_Base : 	
		public RefCount, 
		public TsString_Base<ELT> {
	public:
		typedef typename TsString_Base<ELT>::size_type size_type;
		TsString_Body_Base(size_type bufSize_) :
			size(0),
			bufSize(bufSize_)
		{}
	
		size_type size;
		size_type bufSize;
	};
	
	template<class ELT> class TsString_T;
	
	template<class ELT> class TsString_Body : 
		public TsString_Body_Base<ELT>
	{
		friend class TsString_T<ELT>;
	public:
		typedef TsString_Body<ELT> mytype;
		typedef typename TsString_Body_Base<ELT>::size_type size_type;
	
		// Overloaded operator new and delete.  This is necessary because of the
		// unusual class layout in which the buffer at the end of the class
		// may contain more than the declared size.
	
		// Operator new must allocate more space than the size of the object
		// The bufSize is the amount of space to allocate in the buffer.
		// Note the normal size argument is ignored intentionally.
		void* operator new(size_t, size_type bufSize_)
		{
			return new char[AllocSizeNeeded(bufSize_)];
		}
	
		// Operator delete must delete the object as if it were a char buffer
		// Note: JEL is not sure if we need both of these, but it doesn't seem to hurt
		void operator delete(void* ptr)
		{
			// Opposite of how it was allocated.
			delete [] (char*)ptr;
		}
		void operator delete(void* ptr, size_t)
		{
			// Opposite of how it was allocated.
			delete [] (char*)ptr;
		}
	
		// Constructor where the buffer size is given.
		TsString_Body(size_type bufSize_) : 
			TsString_Body_Base<ELT>(bufSize_)
		{
			buffer[0] = 0;
		}
	
		// Set the string, using the given size
		void assign(const ELT* ptr, size_type length) {
			assert(bufSize >= length + 1);
			Copy(buffer, ptr, length);
			buffer[length] = 0;
			size = length;
		}
	
		// Append the given string, using the given length
		void append(const ELT* ptr, size_type length)
		{
			assert(bufSize >= size + length + 1);
			Copy(buffer + size, ptr, length);
			size += length;
			buffer[size] = 0;
		}
	
		// Append <count> elements
		void appendFill(size_type count, ELT elt)
		{
			assert(bufSize >= size + count + 1);
			Fill(buffer + size, count, elt);
			size += count;
			buffer[size] = 0;
		}
	
		void erase(size_type pos, size_type count)
		{
			pos = JHMIN(unsigned(pos), unsigned(size));
			count = JHMIN(unsigned(count), unsigned(size - pos));
			// Remember to also move the null termination!
			Move(buffer + pos, buffer + pos + count, size - pos - count + 1);
			size -= count;
		}
	
	private:
		// Default operator new format should never be called, and has no body
		void* operator new(size_t size);
		// Default constructor should never be called
		TsString_Body();
	
		// This buffer is really char[bufSize], but we declare it this way to avoid a
		// pointer and extra allocation.
		char buffer[sizeof(size_type)];
	
		// How much space to allocate for this object to get a given buffer size.
		static inline size_type AllocSizeNeeded(size_type bufSize_) {
			// We assume that the buffer adds exactly size_type bytes to the size of the class.
			// Knowing this fact is critical to computing the number of bytes allocation needed
			// to produce a buffer of a known size.
			assert(sizeof(TsString_Body<ELT>) - sizeof(TsString_Body_Base<ELT>) == sizeof(size_type));
			return sizeof(TsString_Body_Base<ELT>) + bufSize_;
		}
	};
	
	
	
	template<class ELT> class TsString_T : 
		public TsString_Base<ELT> 
	{
	public:
		typedef TsString_T<ELT> mytype;
		typedef typename TsString_Base<ELT>::size_type size_type;
		typedef typename TsString_Base<ELT>::iterator iterator;
		typedef typename TsString_Base<ELT>::const_iterator const_iterator;
		typedef typename TsString_Base<ELT>::reference reference;
		typedef typename TsString_Base<ELT>::const_reference const_reference;
	
		// Constant for non-existent index
		static const size_type npos;
	
		TsString_T() {
			handle = NewBody(InitialBufSize);
		}
	
		TsString_T(const mytype& other) 
		{
			handle = other.handle;
		}
	
		TsString_T(
			const mytype& other, 
			size_type position, 
			size_type count
		) {
			position = JHMIN(position, other.size());
			count = JHMIN(count, other.size() - position);
			handle = NewBody(count+1);
			handle->assign(other.c_str() + position, count);
		}
	
		TsString_T(const ELT *str, size_type count)
		{
			// count = JHMIN(count, StrLen(str));  // don't do this; str may not be terminated
			handle = NewBody(count+1);
			handle->assign(str, count);
		}
	
		TsString_T(const ELT *str)
		{
			size_type len = StrLen(str);
			handle = NewBody(len+1);
			handle->assign(str, len);
		}
	
		// conversion from std::basic_string
		TsString_T(const std::basic_string<ELT>& str)
		{
			size_type len = str.size();
			handle = NewBody(len+1);
			handle->assign(str.c_str(), len);
		}
	
		TsString_T(size_type count, ELT elt)
		{
			handle = NewBody(count + 1);
			handle->appendFill(count, elt);
		}
	
		TsString_T(const_iterator first, const_iterator last)
		{
			// We cheat by using knowledge that iterator is really ELT*
			size_type len = last - first;
			handle = NewBody(len+1);
			handle->assign(first, len);
		}
	
		// Cleanup is taken care of by smart pointer
		~TsString_T() {}
	
		mytype& operator=(const mytype& other)
		{return (assign(other)); }
	
		mytype& operator=(const ELT *str)
		{return (assign(str)); }
	
		mytype& operator=(ELT elt)
		{return (assign(1, elt)); }
	
		// conversion from std::basic_string
		mytype& operator=(const std::basic_string<ELT>& str)
		{return (assign(str.c_str())); }
	
	/*
		// conversion to std::basic_string
		operator std::basic_string<ELT>() const {
			return c_str();
		}
	*/
		mytype& operator+=(const mytype& other)
		{return (append(other)); }
	
		mytype& operator+=(const ELT *str)
		{return (append(str)); }
	
		mytype& operator+=(ELT elt)
		{return (append(1, elt)); }
	
		mytype& append(const mytype& other)
		{
			CopyOnWrite(size() + other.size() + 1);
			handle->append(other.c_str(), other.size());
			return *this;
		}
	
		mytype& append(const ELT *str)
		{
			return append(str, StrLen(str));
		}
	
		mytype& append(const ELT *str, size_type count)
		{
			// count = JHMIN(count, StrLen(str));  // don't do this; str may not be terminated
			CopyOnWrite(size() + count + 1);
			handle->append(str, count);
			return *this;
		}
	
		mytype& append(size_type count, ELT elt)
		{
			CopyOnWrite(size() + count + 1);
			handle->appendFill(count, elt);
			return *this;
		}
	
		mytype& append(const_iterator first, const_iterator last)
		{
			size_type len = last - first;
			CopyOnWrite(size() + len + 1);
			// We cheat by using knowledge that iterator is really ELT*
			handle->append(first, len);
			return *this;
		}
	
		mytype& assign(const mytype& other)
		{
			handle = other.handle;
			return *this;
		}
	
		mytype& assign(const mytype& other, size_type count)
		{
			count = JHMIN(count, other.size());
			SplitOnWrite(count + 1);
			handle->assign(other.c_str(), count);
			return *this;
		}
	
		mytype& assign(const ELT *str)
		{
			size_type count = StrLen(str);
			SplitOnWrite(count+1);
			handle->assign(str, count);
			return *this;
		}
	
		mytype& assign(const ELT *str, size_type count)
		{
			// count = JHMIN(count, StrLen(str));  // don't do this; str may not be terminated
			SplitOnWrite(count+1);
			handle->assign(str, count);
			return *this;
		}
	
		mytype& assign(size_type count, ELT elt)
		{
			SplitOnWrite(count+1);
			// Reset it
			handle->size = 0;
			buffer()[0] = ELT(0);
			// Fill with chars
			handle->appendFill(count, elt);
			return *this;
		}
	
		mytype& assign(const_iterator first, const_iterator last)
		{
			return assign(first, last-first);
		}
	
		void insert(size_type pos, const mytype& other)
		{
			insert(pos, other.c_str(), other.size());
		}
	
		void insert(size_type pos, const mytype& other, size_type count)
		{
			insert(pos, other.c_str(), JHMIN(other.size(), count));
		}
	
		void insert(size_type pos, const ELT *str)
		{
			insert(pos, str, StrLen(str));
		}
	
		void insert(size_type pos, const ELT *str, size_type count)
		{
			// count = JHMIN(count, StrLen(str));  // don't do this; str may not be terminated
			pos = JHMIN(pos, size());
			CopyOnWrite(size() + count + 1);
			// Don't forget to move null-termination!
			Move(buffer() + pos + count, buffer() + pos, size() - pos + 1);
			Copy(buffer() + pos, str, count);
			handle->size += count;
		}
	
		void insert(size_type pos, size_type count, ELT elt)
		{
			pos = JHMIN(pos, size());
			CopyOnWrite(size() + count + 1);
			// Don't forget to move null-termination!
			Move(buffer() + pos + count, buffer() + pos, size() - pos + 1);
			Fill(buffer() + pos, count, elt);
			handle->size += count;
		}
	
		iterator insert(iterator position, ELT elt)
		{
			size_type pos = position - begin();
			insert(pos, elt);
			return begin() + pos;
		}
	
		void insert(iterator position, size_type count, ELT elt)
		{
			insert(position - begin(), count, elt);
		}
	
		void insert(iterator position, const_iterator first, const_iterator last)
		{
			insert(position - begin(), first, last-first);
		}
	
		void erase(size_type pos = 0, size_type count = npos)
		{
			CopyOnWrite(size() + 1);
			handle->erase(pos, count);
		}
	
		iterator erase(iterator position)
		{
			size_type pos = position - begin();
			erase(pos, 1);
			return begin() + pos;
		}
	
		iterator erase(iterator first, iterator last)
		{
			size_type pos = first - begin();
			erase(pos, last - first);
			return begin() + pos;
		}
	
		void replace(size_type position, size_type count, const mytype& other)
		{
			replace(position, count, other.c_str(), other.size());
		}
	
		void replace(
			size_type position, 
			size_type count, 
			const mytype& other,
			size_type position2, 
			size_type count2
		) {
			position2 = JHMIN(position2, other.size());
			replace(position, count, other.c_str() + position2, count2);
		}
	
		void replace(size_type position, size_type count, const ELT *str)
		{
			replace(position, count, str, StrLen(str));
		}
	
		void replace(
			size_type position, 
			size_type count, 
			const ELT *str,
			size_type count2
		) {
			position = JHMIN(position, size());
			count = JHMIN(count, size() - position);
			// count2 = JHMIN(count2, StrLen(str));	// don't do this, str may not be terminated
			if (count == count2) {
				CopyOnWrite(handle->bufSize);
				Copy(buffer() + position, str, count);
			} else {
				erase(position, count);
				insert(position, str, count2);
			}
		}
	
		void replace(
			size_type position, 
			size_type count,
			size_type count2, 
			ELT elt
		) {
			position = JHMIN(position, size());
			count = JHMIN(count, size() - position);
			if (count == count2) {
				CopyOnWrite(handle->bufSize);
				Fill(buffer() + position, count2, elt);
			} else {
				erase(position, count);
				insert(position, count2, elt);
			}
		}
	
		void replace(iterator first, iterator last, const mytype& other)
		{
			replace(first-begin(), last-first, other.c_str(), other.size());
		}
	
		void replace(
			iterator first, 
			iterator last, 
			const mytype& other,
			size_type first2,
			size_type count2
		) {
			first2 = JHMIN(first2, other.size());
			replace(first-begin(), last-first, other.c_str() + first2, count2);
		}
	
	
		void replace(
			iterator first, 
			iterator last, 
			const ELT *str,
			size_type count2
		) {
			replace(first-begin(), last-first, str, count2);
		}
	
		void replace(
			iterator first, 
			iterator last, 
			const ELT *str
		) {
			replace(first-begin(), last-first, str, StrLen(str));
		}
	
		void replace(
			iterator first, 
			iterator last,	
			size_type count2, 
			ELT elt
		) {
			replace(first-begin(), last-first, count2, elt);
		}
	
		void replace(
			iterator first1,
			iterator last1,
			const_iterator first2,
			const_iterator last2
		) {
			replace(first1-begin(), last1-first, first2, last2-first2);
		}
	
		iterator begin() { 
			CopyOnWrite(size()+1);
			return buffer(); 
		}
		const_iterator begin() const { return buffer(); }
		iterator end() { 
			CopyOnWrite(size()+1);
			return buffer() + size(); 
		}
		const_iterator end() const { return buffer() + size(); }
	
		reference operator[](size_type position) {
			CopyOnWrite(size() + 1);
			return buffer()[position];
		}
		const_reference operator[](size_type position) const
		{
			return buffer()[position];
		}
	
		const ELT *c_str() const { return buffer(); }
		size_type size() const { return handle->size; }
		size_type length() const { return handle->size; }
	
		void resize(size_type newSize, ELT elt)
		{
			CopyOnWrite(newSize + 1);
			if (newSize > size()) {
				// Growing
				handle->appendFill(newSize - size(), elt);
			} else {
				// Shrinking or staying the same
				handle->size = newSize;
				buffer()[newSize] = 0;
			}
		}
	
		void resize(size_type count)
		{
			resize(count, ELT(0));
		}
	
		size_type capacity() const {
			return handle->bufSize - 1;
		}
	
		void reserve(size_type count = 0) {
			// Never reduces the capacity.
			if (count > capacity()) {
				CopyOnWrite(count + 1);
			}
		}
	
		bool empty() const { return size() == 0; }
	
		size_type find(const mytype& other, size_type start_pos = 0) const {
			return find(other.c_str(), other.size(), start_pos);
		}
	
		size_type find(const ELT *str, size_type start_pos = 0) const
		{
			return find(str, StrLen(str), start_pos);
		}
	
		size_type find(ELT elt, size_type start_pos = 0) const
		{
			return find(&elt, 1, start_pos);
		}
	
		size_type find(const ELT* str, size_type len, size_type start_pos) const 
		{
			len = JHMIN(unsigned(len), unsigned(StrLen(str)));
			if (len > size()) {
				return npos;
			}
			size_type lastpos = size() - len;
			while (start_pos <= lastpos) {
				if (Cmp(str, buffer() + start_pos, len) == 0) {
					return start_pos;
				}
				start_pos++;
			}
			return npos;
		}
	
		size_type rfind(const mytype& other, size_type position = npos) const
		{
			return rfind(other.c_str(), other.size(), position);
		}
	
		size_type rfind(const ELT *str, size_type position = npos) const
		{
			return rfind(str, StrLen(str), position);
		}
	
		size_type rfind(const ELT *str, size_type len, size_type position) const
		{
			if (len > size()) {
				return npos;
			}
			// Note use signed int here to allow check for pos>=0 to fail.
			int pos = JHMIN(int(position), int(size() - len));
			while (pos >= 0) {
				if (Cmp(str, buffer() + pos, len) == 0) {
					return pos;
				}
				pos--;
			}
			return npos;
		}
	
		size_type rfind(ELT elt, size_type position = npos) const
		{
			return rfind(&elt, 1, position);
		}

		size_type find_first_of(const ELT *str, size_type start_pos = 0) const
		{
			int lastpos = (int)size() - 1;
			while ((int)start_pos <= lastpos) {
				ELT c = buffer()[start_pos];
				for (int i = 0; str[i] != 0; i++) {
					if (str[i] == c) {
						return start_pos;
					}
				}
				start_pos++;
			}
			return npos;
		}
	
		size_type find_first_of(const mytype& other, size_type start_pos = 0) const
		{
			return find_first_of(other.c_str(), start_pos);
		}
	
		mytype substr(size_type position = 0, size_type count = npos) const
		{
			position = JHMIN(unsigned(position), unsigned(size()));
			count = JHMIN(unsigned(count), unsigned(size() - position));
			BodyRefPtr newHandle = NewBody(count+1);
			Copy(newHandle->buffer, buffer() + position, count);
			newHandle->buffer[count] = 0;
			newHandle->size = count;
			return newHandle;
		}
	
		int compare(const mytype& other) const 
		{
			return StrCmp(buffer(), other.c_str());
		}
		
		int compare(const ELT* str) const
		{
			return StrCmp(buffer(), str);
		}
	
		bool operator==(const mytype& rhs) const { return compare(rhs) == 0; }
		bool operator!=(const mytype& rhs) const { return compare(rhs) != 0; }
		bool operator<(const mytype& rhs) const { return compare(rhs) < 0; }
		bool operator>(const mytype& rhs) const { return compare(rhs) > 0; }
		bool operator>=(const mytype& rhs) const { return compare(rhs) >= 0; }
		bool operator<=(const mytype& rhs) const { return compare(rhs) <= 0; }
	
		bool operator==(const ELT* rhs) const { return compare(rhs) == 0; }
		bool operator!=(const ELT* rhs) const { return compare(rhs) != 0; }
		bool operator<(const ELT* rhs) const { return compare(rhs) < 0; }
		bool operator>(const ELT* rhs) const { return compare(rhs) > 0; }
		bool operator>=(const ELT* rhs) const { return compare(rhs) >= 0; }
		bool operator<=(const ELT* rhs) const { return compare(rhs) <= 0; }
	
	private:
		// Size of buffer to allocate by default
		enum {
			InitialBufSize = 8
		};
	
		// Type of reference-counted pointer to body.
		typedef TsString_Body<ELT> Body;
		typedef refcnt_ptr<Body> BodyRefPtr;
	
		// Smart-pointer to body
		BodyRefPtr handle;
	
		// Shortcut to get to buffer
		ELT* buffer() { return handle->buffer; }
		const ELT* buffer() const { return handle->buffer; }
	
		// Call when the string contents is about to change.
		// This exists to manage the ref-counting of handle, and to ensure
		// that we only change the body for THIS owner, and no other owner
		// of the same body.
		void CopyOnWrite(
			size_type newBufSize
		) {
			#ifdef TSSTRING_STATS
				stats.copyOnWriteCalls++;
			#endif
			// Will copy the body if either:
			// 1) There is more than one string referencing the body
			// 2) The body is growing beyond current capacity.
			if (handle.nref() != 1 || newBufSize > handle->bufSize) {
				#ifdef TSSTRING_STATS
					stats.copyOnWriteNewBuffer++;
				#endif
				// Adjust size to get Nlog(N) behavior for appends.
				if (newBufSize > handle->bufSize) {
					newBufSize = JHMAX(unsigned(newBufSize), unsigned(handle->bufSize * 2));
				}
				BodyRefPtr handleCopy = handle;
				// Allocate a new body
				handle = NewBody(newBufSize);
				// Copy the current contents
				handle->assign(
					handleCopy->buffer,
					JHMIN(unsigned(handleCopy->size), unsigned(newBufSize-1))
				);
			}
		}
	
		// Call when the string contents is about to change.
		// Like CopyOnWrite, but does not copy the old string.
		void SplitOnWrite(
			size_type newBufSize
		) {
			#ifdef TSSTRING_STATS
				stats.splitOnWriteCalls++;
			#endif
			if (handle.nref() != 1 || newBufSize > handle->bufSize) {
				#ifdef TSSTRING_STATS
					stats.copyOnWriteNewBuffer++;
				#endif
				// Adjust size to get Nlog(N) behavior for appends.
				if (newBufSize > handle->bufSize) {
					newBufSize = JHMAX(unsigned(newBufSize), unsigned(handle->bufSize * 2));
				}
				// Allocate a new body
				handle = NewBody(newBufSize);
			}
		}
	
		// Allocates a new string body
		static inline Body* NewBody(size_type newBufSize)
		{
			#ifdef TSSTRING_STATS
				stats.newBody++;
			#endif
			newBufSize = RoundUp(JHMAX(unsigned(newBufSize), (unsigned)InitialBufSize), sizeof(size_type));
			// This calls the specialized operator new() in the body.  It is not "placement new"
			// which is the old form of the override.  It simply passes the extra size parameter 
			// to the operator new() method.
			return new (newBufSize) Body(newBufSize);
			//             ^                  ^
			//    operator new() parm       size parm to constructor
		}
	
		// Private constructor from pre-built handle
		TsString_T(BodyRefPtr handle_) : 
			handle(handle_) 
		{}
	
		#ifdef TSSTRING_STATS
		public:
			struct Stats {
				ACE_Atomic_Op<ACE_Thread_Mutex, long> copyOnWriteNewBuffer;
				ACE_Atomic_Op<ACE_Thread_Mutex, long> copyOnWriteCalls;
				ACE_Atomic_Op<ACE_Thread_Mutex, long> splitOnWriteNewBuffer;
				ACE_Atomic_Op<ACE_Thread_Mutex, long> splitOnWriteCalls;
				ACE_Atomic_Op<ACE_Thread_Mutex, long> newBody;
			};
			static Stats stats;
		private:
		#endif
	};
	
	// npos definition
	template<class ELT> const typename TsString_T<ELT>::size_type TsString_T<ELT>::npos = unsigned(-1);
	
	#ifdef TSSTRING_STATS
		// stats definition
		template<class ELT> TsString_T<ELT>::Stats TsString_T<ELT>::stats;
	#endif
	
	// Helper comparison functions
	template<class ELT> inline bool operator==(const ELT* lhs, const TsString_T<ELT>& rhs) { return rhs.compare(lhs) == 0; }
	template<class ELT> inline bool operator!=(const ELT* lhs, const TsString_T<ELT>& rhs) { return rhs.compare(lhs) != 0; }
	template<class ELT> inline bool operator>(const ELT* lhs, const TsString_T<ELT>& rhs) { return rhs.compare(lhs) < 0; }
	template<class ELT> inline bool operator<(const ELT* lhs, const TsString_T<ELT>& rhs) { return rhs.compare(lhs) > 0; }
	template<class ELT> inline bool operator>=(const ELT* lhs, const TsString_T<ELT>& rhs) { return rhs.compare(lhs) <= 0; }
	template<class ELT> inline bool operator<=(const ELT* lhs, const TsString_T<ELT>& rhs) { return rhs.compare(lhs) >= 0; }
	
	// Concatentation helper functions
	template<class ELT> TsString_T<ELT> inline operator+(const TsString_T<ELT>& lhs, const TsString_T<ELT>& rhs) { 
		TsString_T<ELT> temp(lhs);
		temp += rhs;
		return temp;
	}
	template<class ELT> TsString_T<ELT> inline operator+(const TsString_T<ELT>& lhs, const ELT* rhs) { 
		TsString_T<ELT> temp(lhs);
		temp += rhs;
		return temp;
	}
	template<class ELT> TsString_T<ELT> inline operator+(const TsString_T<ELT>& lhs, ELT rhs) { 
		TsString_T<ELT> temp(lhs);
		temp += rhs;
		return temp;
	}
	template<class ELT> TsString_T<ELT> inline operator+(const ELT* lhs, const TsString_T<ELT>& rhs) { 
		TsString_T<ELT> temp(lhs);
		temp += rhs;
		return temp;
	}
	
	template<class ELT> TsString_T<ELT> inline operator+(const TsString_T<ELT>& lhs, const std::basic_string<ELT>& rhs) { 
		TsString_T<ELT> temp(lhs);
		temp += rhs.c_str();
		return temp;
	}
	template<class ELT> TsString_T<ELT> inline operator+(const std::basic_string<ELT>& lhs, const TsString_T<ELT>& rhs) { 
		TsString_T<ELT> temp(lhs.c_str());
		temp += rhs;
		return temp;
	}
	
	// Stream helper
	template<class ELT> inline std::basic_ostream<ELT>& operator<<(
		std::basic_ostream<ELT>& stream,
		const TsString_T<ELT>& str
	) {
		stream << str.c_str();
		return stream;
	}
	
	
	typedef TsString_T<char> TsString;
	
	}	// namespace

#endif

#endif 
