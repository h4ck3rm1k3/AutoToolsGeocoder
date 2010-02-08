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

////////////////////////////////////////////////////////////////////////////////
// Freelist memory allocator
////////////////////////////////////////////////////////////////////////////////

#ifndef INCL_FREELIST_H
#define INCL_FREELIST_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <list>
#include <cassert>

#include "RefPtr.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	/////////////////////////////////////////////////////////////////////
	// GenericFreeList:  Fast fixed-size memory allocator
	// Note: All allocated blocks are freed when freelist is deleted!
	/////////////////////////////////////////////////////////////////////
	class GenericFreeList : public VRefCount
	{
	public:
		/////////////////////////////////////////////////////////////////////
		// Constructor:
		// Inputs:
		//	int		elementSize		The size of each element to allocate
		//	int		blockSize		Size of memory block to allocate
		/////////////////////////////////////////////////////////////////////
		GenericFreeList(
			int elementSize_, 
			int blockSize_ = 0x20000
		);
		~GenericFreeList();
		
		void* New();

		void Delete(void* ptr)
		{
			if (ptr != 0) {
				((FreeElement*)ptr)->next = freeList;
				freeList = (FreeElement*)ptr;
			}
		}

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Do not allow copy / assignment
		///////////////////////////////////////////////////////////////////////////////
		GenericFreeList(GenericFreeList& ) { assert(0); }
		GenericFreeList& operator=(const GenericFreeList&) { assert(0); return *this;}
		
		///////////////////////////////////////////////////////////////////////////////
		// Allocate another bock of elements to the free list.
		///////////////////////////////////////////////////////////////////////////////
		void AddBlock();

		///////////////////////////////////////////////////////////////////////////////
		// Dummy object used to manage the free-list.
		///////////////////////////////////////////////////////////////////////////////
		struct FreeElement
		{
			FreeElement* next;
		};

		int elementSize;		// size of each element
		int eltsPerBlock;		// Number of elements in each new block
		FreeElement* freeList;	// threaded list of free objects
		typedef std::list<char*> CharPtrList;
		CharPtrList *blocks;
	};

	///////////////////////////////////////////////////////////////////////////////
	// Templated free-list subclass that provides a type-safe wrapper.
	///////////////////////////////////////////////////////////////////////////////
	template <class T> class FreeList : public GenericFreeList {
	public:
		FreeList() : GenericFreeList(sizeof(T)) {}
		~FreeList() {}
		T* New() { 
			void *ptr = GenericFreeList::New();
			// new with placement to construct in the new space.
			new (ptr) T;
			return (T*)ptr;
		}
		void Delete(T* ptr) { 
			ptr->~T();
			GenericFreeList::Delete(ptr); 
		}
	};

}

#endif
