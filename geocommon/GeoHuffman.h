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

// Huffman.h: Template class for huffman-coding

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "Geocoder_DllExport.h"

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <assert.h>
#include "../global/RefPtr.h"
#include "GeoFreqTable.h"
#include "GeoBitStream.h"

#ifndef INCL_HUFFMAN_H
#define INCL_HUFFMAN_H

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// This template class is responsible for building huffman codes based on a
	// count table.  The template type parameter T indicates the type of 
	// object being coded, and will be the "key" of the count table.
	// 
	// The type T must support the comparison operator <
	///////////////////////////////////////////////////////////////////////////////
	template <class T, class CMP > class HuffmanCoder : public VRefCount {
	public:
		// Constructor/destructor
		HuffmanCoder(CMP cmp_ = CMP()) : 
			codeTree(0), 
			valueMap(cmp_),
			maxCodeLength(0), 
			decodePtr(0),
			cmp(cmp_)
		{}
		~HuffmanCoder() {}

		// Reset the table
		void Clear() {
			valueMap.clear();
			codeTree = 0;
			freqTable.clear();
		}			

		// Add another entry to the code count table
		void AddEntry(const T& value, int count);

		// Generate codes from the count table
		void MakeCodes();

		// Add entries using the given frequency table.
		// Type parameter of the freq table must match that of Huffman table.
		void AddEntries(const FreqTable<T>& counts);

		///////////////////////////////////////////////////////////////////////////////
		// Get the maximum code length in bits.
		// Only valid after MakeCodes() is called.
		///////////////////////////////////////////////////////////////////////////////
		int GetMaxCodeLength() const {
			return maxCodeLength;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Given a value, return its code.  Because the code is variable
		// length, it is returned as an array of bytes and a bit-count.
		// Bits are arranged starting from the LSB of the first byte.
		// Inputs:
		//	const T&		value			The value for which a code is wanted.
		// Outputs:
		//	unsigned char*	bytesReturn		An array of bytes containing the code,
		//									starting with the LSB of the first byte
		//	int&			lengthReturn	The number of bits in the code.
		// Return value:
		//	bool		true if the value was found in the table, false otherwise.
		///////////////////////////////////////////////////////////////////////////////
		bool GetCode(
			const T& value,
			unsigned char* bytesReturn,
			int& lengthReturn
		);

		///////////////////////////////////////////////////////////////////////////////
		// Given a value, find its code and write to a bitstream.
		// Inputs:
		//	const T&		value			The value for which a code is wanted.
		//	BitStreamWrite&	bitStream		The bitstream to receive code bits.
		// Return value:
		//	bool		true if the value was found in the table, false otherwise.
		///////////////////////////////////////////////////////////////////////////////
		bool WriteCode(
			const T& value,
			BitStreamWrite& bitStream
		);

		///////////////////////////////////////////////////////////////////////////////
		// Read code bits from a stream and produce the resulting code.
		// Inputs:
		//	BitStreamRead&	bitStream		The bitstream that will read code bits
		// Outputs:
		//	const T*&		valueReturn		If the return code is true, then the code is 
		// Return value:
		//	bool		true if a valid value was read, false o/w
		///////////////////////////////////////////////////////////////////////////////
		bool ReadCode(
			BitStreamRead& bitStream,
			const T*& valueReturn
		) {
			int bit;
			bool finished = StartDecode(valueReturn);
			while (!finished) {
				if (!bitStream.NextBit(bit)) {
					return false;
				}
				finished = Decode(bit, valueReturn);
			}
			return true;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Prepare for decoding
		// Outputs:
		//	const T*&	valueReturn		If the return code is true , then the code is 
		//								complete and this points to the return value.
		// Return value:
		//	bool		true if the code is complete without reading any bits,
		//				indicating that there is only a single value in the code.
		///////////////////////////////////////////////////////////////////////////////
		bool StartDecode(const T*& valueReturn) 
		{ 
			decodePtr = codeTree.get();
			valueReturn = &decodePtr->value;
			return decodePtr->left == 0;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Decode the next bit.
		// Inputs:
		//	int			bit				The next bit to decode.
		// Outputs:
		//	const T*&	valueReturn		If the return code is true, then the code is 
		//								complete and this points to the return value.
		// Return value:
		//	bool		true if the code is complete, false if more bits are required.
		//				If true is returned, then call StartDecode() to start decoding
		//				the next value.
		///////////////////////////////////////////////////////////////////////////////
		bool Decode(
			int bit,
			const T*& valueReturn
		) {
			decodePtr = (bit ? decodePtr->right.get() : decodePtr->left.get());
			if (decodePtr->IsLeaf()) {
				valueReturn = &decodePtr->value;
				return true;
			} else {
				return false;
			}
		}


	private:
		// Structure used to hold count table and build the code tree.
		struct Entry;
		typedef refcnt_ptr<Entry> EntryRef;
		struct Entry : public RefCount {
			Entry(const T& value_, int count_) :
				value(value_),
				count(count_),
				left(0),
				right(0),
				parent(0)
			{}
			bool IsInterior() const { return left != 0; }
			bool IsLeaf() const { return left == 0; }
			bool IsRoot() const { return parent == 0; }
			T value;
			int count;
			EntryRef left;		// Child pointers for building the code tree.
			EntryRef right;
			Entry* parent;		// Parent pointer for building code tree.
								// This must be a dumb pointer to avoid circular refs.

		};

		///////////////////////////////////////////////////////////////////////////////
		// Given an entry in the tree, find its code length
		///////////////////////////////////////////////////////////////////////////////
		int CodeLength(EntryRef entry)
		{
			int length = 0;
			while (!entry->IsRoot()) {
				length++;
				entry = entry->parent;
			}
			return length;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Set a code bit
		///////////////////////////////////////////////////////////////////////////////
		void SetCodeBit(unsigned char* bytes, int bitNbr, int value) {
			if (value) {
				bytes[bitNbr / 8] |= (1 << (bitNbr % 8));
			} else {
				bytes[bitNbr / 8] &= ~(1 << (bitNbr % 8));
			}
		}

		// The frequency table.
		std::vector<EntryRef> freqTable;

		// Mapping from values to freq table entries.
		typedef typename std::map<T, EntryRef, CMP> ValueMap;
		typedef typename ValueMap::iterator VM_iterator;
		ValueMap valueMap;

		// The code tree.  Nodes point into the frequency table or the node list.
		EntryRef codeTree;

		// Longest code in use.  Only valid after MakeCodes() is called
		int maxCodeLength;

		// Pointer to current node during decoding process.
		const Entry* decodePtr;

		// Comparator object used for ordering values.
		CMP cmp;

		// Table of shortcuts.  Each entry contains a "jump" to a 
		// code-tree node, given the next eight bits of bitstream.
		const Entry* shortcuts[256];

		// To avoid typename problems
		typedef typename FreqTable<T>::const_iterator FT_const_iterator;

	public:
		///////////////////////////////////////////////////////////////////////////////
		// Comparator for entries.  Should sort from high to low count.
		///////////////////////////////////////////////////////////////////////////////
		struct HuffmanEntryPtrCmp {
			HuffmanEntryPtrCmp(CMP cmp_) : cmp(cmp_) {}
			bool operator()(EntryRef lhs, EntryRef rhs) const {
				if (lhs->count != rhs->count) {
					return lhs->count > rhs->count;
				} else {
					return cmp(lhs->value, rhs->value);
				}
			}
			CMP cmp;
		};

		typedef typename std::set<EntryRef, HuffmanEntryPtrCmp> HuffEntryFreeList;
		typedef typename HuffEntryFreeList::iterator HEFL_iterator;

	};


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// Out-of-line template functions
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Add another entry to the code table
	///////////////////////////////////////////////////////////////////////////////
	template <class T, class CMP> void HuffmanCoder<T, CMP>::AddEntry(
		const T& value, 
		int count
	) 
	{
		// Invalidate previous code tables
		valueMap.clear();
		codeTree = 0;

		freqTable.push_back(new Entry(value, count));
	}

	///////////////////////////////////////////////////////////////////////////////
	// Calculate the optimal code set for the table entries
	///////////////////////////////////////////////////////////////////////////////
	template <class T, class CMP> void HuffmanCoder<T, CMP>::MakeCodes()
	{
		// If the freq table is empty then fake something.
		if (freqTable.empty()) {
			T t;
			AddEntry(t, 1);
		}

		// Populate the value map
		valueMap.clear();
		{for (unsigned i = 0; i < freqTable.size(); i++) {
		  valueMap.insert( std::make_pair( freqTable[i]->value, freqTable[i] ) );
		}}

#if 0
// Old method using sorted vector.
		// Populate the free list from the frequency table
		std::vector<EntryRef> freelist(freqTable);

		// Reset the code tree
		codeTree = 0;

		// Repeat until there is a single entry in the free list.
		// It will be the root.
		HuffmanEntryPtrCmp entryCmp(cmp);

		// Order the free list from highest to lowest freqeuncy.
		std::sort(freelist.begin(), freelist.end(), entryCmp);

		while (freelist.size() > 1) {
			// Pull the two lowest entries.
			EntryRef right = freelist.back();
			freelist.pop_back();
			EntryRef left = freelist.back();
			freelist.pop_back();

			// Combine to form a new node with left and right as children.
			EntryRef newNode = new Entry(left->value, left->count + right->count);
			newNode->left = left;
			newNode->right = right;
			left->parent = newNode.get();
			right->parent = newNode.get();

			// Insert the new node onto the free list, keeping the list ordered.
			freelist.insert(
				std::upper_bound(freelist.begin(), freelist.end(), newNode, entryCmp),
				newNode
			);
		}

		// Remaining free list item is the tree root.
		assert(!freelist.empty());
		codeTree = freelist.back();
#endif

		// Populate the free list from the frequency table
		HuffmanEntryPtrCmp entryCmp(cmp);
		HuffEntryFreeList freelist(freqTable.begin(), freqTable.end(), entryCmp);

		// Reset the code tree
		codeTree = 0;

		// Repeat until there is a single entry in the free list.
		// It will be the root.

		while (freelist.size() > 1) {
			// Pull the two lowest entries.
			HEFL_iterator iter = freelist.end();
			--iter;
			EntryRef right = (*iter);
			freelist.erase(iter);

			iter = freelist.end();
			--iter;
			EntryRef left = (*iter);
			freelist.erase(iter);

			// Combine to form a new node with left and right as children.
			EntryRef newNode = new Entry(left->value, left->count + right->count);
			newNode->left = left;
			newNode->right = right;
			left->parent = newNode.get();
			right->parent = newNode.get();

			// Insert the new node onto the free list, keeping the list ordered.
			freelist.insert(newNode);
		}

		// Remaining free list item is the tree root.
		assert(!freelist.empty());
		codeTree = *freelist.begin();

		// Find the longest code length
		maxCodeLength = 0;
		{for (unsigned i = 0; i < freqTable.size(); i++) {
			int codeLength = CodeLength(freqTable[i]);
			if (codeLength > maxCodeLength) {
				maxCodeLength = codeLength;
			}
		}}
	}


	///////////////////////////////////////////////////////////////////////////////
	// Get a code for a given symbol.
	// Inputs:
	//	T				value			The symbol
	// Outputs:
	//	unsigned char*	bytesReturn		Buffer containing a string of bits,
	//									starting with the LSB of the first byte.
	//	int&			lengthReturn	Number of bits returned in the buffer.
	// Return value:
	//	bool			true if the code was found, false o/w
	///////////////////////////////////////////////////////////////////////////////
	template <class T, class CMP> bool HuffmanCoder<T, CMP>::GetCode(
		const T& value,
		unsigned char* bytesReturn,
		int& lengthReturn
	) {
		// Find the entry
		VM_iterator iter = valueMap.find(value);
		if (iter == valueMap.end()) {
			return false;
		}

		Entry* entry = (*iter).second.get();
		lengthReturn = CodeLength(entry);
		int bitNbr = lengthReturn - 1;
		while (!entry->IsRoot()) {
			Entry* parent = entry->parent;
			int bitValue = (parent->left.get() == entry) ? 0 : 1;
			SetCodeBit(bytesReturn, bitNbr, bitValue);
			bitNbr--;
			entry = parent;
		}
		assert(bitNbr == -1);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Given a value, find its code and write to a bitstream.
	// Inputs:
	//	const T&		value			The value for which a code is wanted.
	//	BitStreamWrite&	bitStream		The bitstream to receive code bits.
	// Return value:
	//	bool		true if the value was found in the table, false otherwise.
	///////////////////////////////////////////////////////////////////////////////
	template <class T, class CMP> bool HuffmanCoder<T, CMP>::WriteCode(
		const T& value,
		BitStreamWrite& bitStream
	) {
		// For now do this the simple way.
		unsigned char buf[20];
		int length;
		assert(maxCodeLength < static_cast<int>(sizeof(buf) * 8));
		if (!GetCode(value, buf, length)) {
			return false;
		}
		bitStream.WriteBits(length, buf);
		return true;
	}

	// Add entries using the given frequency table.
	// Type parameter of the freq table must match that of Huffman table.
	template <class T, class CMP> void HuffmanCoder<T, CMP>::AddEntries(
		const FreqTable<T>& counts
	) {
		for (
			FT_const_iterator iter = counts.begin();
			iter != counts.end();
			++iter
		) {
			AddEntry((*iter).first, (*iter).second);
		}
	}



} // namespace

#endif
