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

// TSFIFO.h: A thread-safe FIFO queue template.
// Uses the critical-section facility in CritSec.h

#ifndef INCL_TSFIFO_H
#define INCL_TSFIFO_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <assert.h>
#include <list>
#include "CritSec.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	template <class T> class ThreadSafeFIFO {
	public:
		ThreadSafeFIFO() {}
		virtual ~ThreadSafeFIFO() {}
		void add(const T& t) {
			CritSec critSec(lock);	// Serialize access to this object
			implementation.push_back(t);
		}
		T remove() {
			CritSec critSec(lock);	// Serialize access to this object
			assert(!implementation.empty());
			T t = implementation.front();
			implementation.pop_front();
			return t;
		}
		void clear() {
			CritSec critSec(lock);	// Serialize access to this object
			implementation.clear();
		}
		size_t count() { 
			CritSec critSec(lock);	// Serialize access to this object
			return implementation.size(); 
		}
		size_t count_nolock() { 
			return implementation.size(); 
		}
		std::list<T> getAll() const {
			CritSec critSec(lock);	// Serialize access to this object
			return implementation;
		}
	private:
		mutable CritSecInfo lock;
		std::list<T> implementation;
	};
}


#endif

