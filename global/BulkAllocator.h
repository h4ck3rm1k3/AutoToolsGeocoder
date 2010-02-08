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

// BulkAllocator.h: Object that allocates variable-sized items, 
// all belonging to a single group.  When the object is destructed, 
// it deletes all allocated memory. This is especially good at quickly 
// allocating large numbers of small, variable-sized buffers.

#ifndef INCL_BULK_ALLOCATOR_H
#define INCL_BULK_ALLOCATOR_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include <vector>
#include <assert.h>
#include <string.h>

#include "RefPtr.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class BulkAllocator : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	int		blockSize		Size of block to allocate
		///////////////////////////////////////////////////////////////////////////////
		BulkAllocator(int blockSize_ = 128 * 1024);

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~BulkAllocator() {
			Reset();
		}

		///////////////////////////////////////////////////////////////////////////////
		// Allocate some memory
		///////////////////////////////////////////////////////////////////////////////
		void* New(size_t size) {
			// Align to four-byte boundary for best performance.
			size += Roundup[(unsigned int)size & 3];
			if (size <= maxFragmentSize && size <= lastBlock->Available()) {
				return lastBlock->New(size);
			} else {
				return SlowNew(size);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Copy the given string to newly-allocated memory
		///////////////////////////////////////////////////////////////////////////////
		void* NewString(const char* str) {
			char* ptr = (char*)New(strlen(str) + 1);
			strcpy(ptr, str);
			return ptr;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Free all memory, but keep first allocated block around because we
		// are going to do more allocation
		///////////////////////////////////////////////////////////////////////////////
		void Reset() {
			lastBlock = blocks[0];
			blocks.resize(1);
			blocks[0]->Reset();
			for (unsigned i = 0; i < allocations.size(); i++) {
				delete [] allocations[i];
			}
			allocations.clear();
		}

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Out-of-line version of New()
		///////////////////////////////////////////////////////////////////////////////
		void* SlowNew(size_t size);

		// Data structure describing a block
		class Block : public VRefCount {
		public:
			Block(size_t size)
			{
				ptr = new char[size];
				endPtr = ptr + size;
				current = ptr;
			}
			virtual ~Block() { delete [] ptr; }
			size_t Available() const { return endPtr - current; }
			void* New(size_t amount) { 
				assert(amount <= Available());
				void* p = current;
				current += amount;
				return p;
			}
			void Reset() { current = ptr; }
			char* ptr;		// the buffer
			char* endPtr;	// end of buffer
			char* current;	// pointer to next allocation
		private:
			// Copy/assign not allowed
			Block(const Block& rhs);
			Block& operator=(const Block& rhs);
		};
		typedef refcnt_ptr<Block> BlockRef;

		// Block size
		size_t blockSize;
		// Largest fragment to be allocated from a block
		size_t maxFragmentSize;
		// List of active blocks
		std::vector<BlockRef> blocks;
		// The last block in the list
		BlockRef lastBlock;
		// List of allocations that were made with new because they were too large.
		std::vector<char*> allocations;

		// Table for 4-byte alignment
		static int Roundup[4];
	};
	typedef refcnt_ptr<BulkAllocator> BulkAllocatorRef;

}

#endif

