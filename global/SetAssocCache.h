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

// SetAssocCache.h: N-way set-associative cache template

#ifndef INCL_SetAssocCache_H
#define INCL_SetAssocCache_H

#include "RefPtr.h"
#include "Global_DllExport.h"

#include <assert.h>

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////
	// Base class that implements some common methods for the template.
	// Duplicate keys are NOT supported!
	///////////////////////////////////////////////////////////////////////////
	class SetAssocCacheBase : public VRefCount {
	public:
		// Generate the first prime number that is >= value.
		int NextPrime(int value);
		///////////////////////////////////////////////////////////////////////////
		// Some common key types.
		///////////////////////////////////////////////////////////////////////////
		// Integer key
		struct IntKey {
			IntKey(unsigned int x_=0) : x(x_) {}  
			unsigned int Hash() const { return x; }
			bool operator==(const IntKey& rhs) const { return x == rhs.x; }
		private:
			unsigned int x;
		};
		// String key (limited length defined at compile-time)
		template<int I> struct FixedStringKey {
		public:
			FixedStringKey(const char* stringVal_ = "") {
				strncpy(stringVal, stringVal_, I);
				stringVal[I] = 0;
			}
			unsigned int Hash() const { 
				unsigned int hashCode = 0;
				for (const char* p = stringVal; *p != 0; p++) {
					hashCode = hashCode | (hashCode << 5) | (unsigned int)*p;
				}
				return hashCode;
			}
			bool operator==(const FixedStringKey& rhs) const { return strcmp(stringVal, rhs.stringVal) == 0; }
			char stringVal[I];
		};

	private:
		// Testing for primality
		bool IsPrime(int value);
	};

	///////////////////////////////////////////////////////////////////////////
	// Set-associative cache template.
	// 
	// The Data type parameter must support the following operations:
	// -- A default constructor.
	// -- An assignment operator taking (const Data&)
	// 
	// The Key type parameter must support the following operations:
	// -- A default constructor.
	// -- A hashing function of the form:
	//		unsigned int Hash() const;
	// -- Key& operator=(const Key& rhs)
	// -- operator==(const Key& rhs) const
	// 
	// The Cmp type parameter must support the following operations:
	///////////////////////////////////////////////////////////////////////////
	template <class Key, class Data, int N> class SetAssocCache : public SetAssocCacheBase {
	public:
		///////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	int		size_		The number of entires in the cache.
		///////////////////////////////////////////////////////////////////////////
		SetAssocCache(
			int size_
		)
		#ifndef NDEBUG
			: 
			hits(0),
			misses(0)
		#endif
		{
			size = NextPrime(size_ / N + 1);
			if (size <= 0) {
				size = 1;
			}
			table = new Entry[size];
		}

		virtual ~SetAssocCache() {
			delete [] table;
			table = 0;
		}

		// Resize the cache.  This also clears it.
		void Resize(int newCacheSize) {
			delete [] table;
			size = NextPrime(newCacheSize / N + 1);
			if (size <= 0) {
				size = 1;
			}
			table = new Entry[size];
		}

		// Is the given item in the cache?
		bool Present(const Key& key)
		{
			const Entry& entry = table[(unsigned int)key.Hash() % size];
			for (int i = 0; i < entry.filled; i++) {
				if (entry.bucket[i].key == key) {
					#ifndef NDEBUG
						hits++;
					#endif
					return true;
				}
			}
			#ifndef NDEBUG
				misses++;
			#endif
			return false;
		}

		// If the given item is in the cache, then retrieve the item.
		bool Fetch(const Key& key, Data& data)
		{
			Entry& entry = table[(unsigned int)key.Hash() % size];
			for (int i = 0; i < entry.filled; i++) {
				if (entry.bucket[i].key == key) {
					data = entry.bucket[i].data;
					// Make next entry go into following bucket.  This
					// rewards recently-accessed items.
					entry.next = char((i + 1) % N);
					#ifndef NDEBUG
						hits++;
					#endif
					return true;
				}
			}
			#ifndef NDEBUG
				misses++;
			#endif
			return false;
		}

		// Enter the item in the cache.  
		// The caller must check Present() or Fetch() first!
		// Do not add duplicates!
		void Enter(const Key& key, const Data& data) 
		{
			if (Present(key)) {
				#ifndef NDEBUG
					misses--;	// don't screw up counts...
				#endif
				return;
			}
			Entry& entry = table[(unsigned int)key.Hash() % size];
			entry.bucket[entry.next].key = key;
			entry.bucket[entry.next].data = data;
			if (entry.filled == entry.next) {
				entry.filled++;
				assert(entry.filled <= N);
			}
			entry.next = (entry.next + 1) % N;
		}

		// Similar to Enter, but it returns the object to be replaced so it can be recycled
        Data & Change(const Key& key) 
        {
			#ifndef NDEBUG
				assert(!Present(key));
				misses--;   // don't screw up counts...
			#endif
			Entry& entry = table[key.Hash() % size];
			entry.bucket[entry.next].key = key;
			Data & ret = entry.bucket[entry.next].data;
			if (entry.filled == entry.next) {
				entry.filled++;
				assert(entry.filled <= N);
			}
			entry.next = (entry.next + 1) % N;
			return ret;
		}

		// Purge the cache of all entries
		void Purge()
		{
			for (int i = 0; i < size; ++i) {
				table[i] = Entry();

			}
		}

	private:
		struct Entry {
			Entry() : next(0), filled(0) {}
			struct Bucket {
				Bucket() : key(), data() {}
				Key key; 
				Data data; 
			};
			Bucket bucket[N];
			char next;		// round-robin counter for buckets
			char filled;	// number of buckets filled
		};
		int size;			// Number of entries
		Entry* table;
		#ifndef NDEBUG
			int hits;
			int misses;
		#endif
	};

}

#endif
