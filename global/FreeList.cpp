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


#include "Global_Headers.h"
#include "Freelist.h"
#include "Basics.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	//                     GenericFreeList                                            //
	///////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// Constructor:
	// Inputs:
	//	int		elementSize		The size of each element to allocate
	//	int		blockSize		Size of mem blocks to allocate
	/////////////////////////////////////////////////////////////////////
	GenericFreeList::GenericFreeList(
		int elementSize_, 
		int blockSize_
	) :
		elementSize(JHMAX((int)sizeof(FreeElement), elementSize_)),
		freeList(0)
	{
		// make sure it's somewhat reasonable
		eltsPerBlock = blockSize_ / elementSize;
		eltsPerBlock = JHMAX(eltsPerBlock, 10);
		blocks = new CharPtrList;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////////
	GenericFreeList::~GenericFreeList()
	{
		// Note: we might want to add some bookkeeping to this, to
		// report unfreed elements
		for (
			CharPtrList::iterator iter(blocks->begin());
			iter != blocks->end();
			iter++
		)
		{
			delete [] *iter;
		}
	   delete blocks;
	}
		
	///////////////////////////////////////////////////////////////////////////////
	// Allocate another element
	///////////////////////////////////////////////////////////////////////////////
	void* GenericFreeList::New()
	{
		if (freeList == 0)
		{
			AddBlock();
		}
		void* ptr = freeList;
		freeList = freeList->next;
		return ptr;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Allocate another bock of elements to the free list.
	///////////////////////////////////////////////////////////////////////////////
	void GenericFreeList::AddBlock()
	{
		char* newBlockPtr = new char[eltsPerBlock * elementSize];
		assert(newBlockPtr != 0);
		char* blockEnd = newBlockPtr + eltsPerBlock * elementSize;
		blocks->push_back(newBlockPtr);
		
		// Add the elements to the free list
		for (; newBlockPtr < blockEnd; newBlockPtr += elementSize)
		{
			((FreeElement*)(void*)newBlockPtr)->next = freeList;
			freeList = (FreeElement*)(void*)newBlockPtr;
		}
	}

}

