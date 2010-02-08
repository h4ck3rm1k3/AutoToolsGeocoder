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

// BulkAllocator.cpp: Object that allocates many items, all belonging to a single
// group.  When the object is destructed, it deletes all allocated memory.
// This is especially good at quickly allocating large numbers of small objects.

#include "Global_Headers.h"
#include "BulkAllocator.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Constructor
	// Inputs:
	//	int		blockSize		Size of block to allocate
	///////////////////////////////////////////////////////////////////////////////
	BulkAllocator::BulkAllocator(
		int blockSize_
	) : 
		blockSize(blockSize_),
		maxFragmentSize(blockSize_ / 8)
	{
		lastBlock = new Block(blockSize);
		blocks.push_back(lastBlock);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Out-of-line version of New()
	///////////////////////////////////////////////////////////////////////////////
	void* BulkAllocator::SlowNew(size_t size)
	{
		if (size > maxFragmentSize) {
			// Allocate using new
			char* ptr = new char[size];
			// Remember to delete it at the end.
			allocations.push_back(ptr);
			return ptr;
		}

		// Try to allocate from existing blocks.
		for (unsigned i = 0; i < blocks.size(); i++) {
			if (size <= blocks[i]->Available()) {
				return blocks[i]->New(size);
			}
		}

		// Get a new block and allocate from that.
		lastBlock = new Block(blockSize);
		blocks.push_back(lastBlock);
		return lastBlock->New(size);
	}

	// Table for 4-byte alignment
	int BulkAllocator::Roundup[4] = { 0, 3, 2, 1 };

}
