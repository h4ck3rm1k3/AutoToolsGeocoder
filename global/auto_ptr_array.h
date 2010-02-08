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
# $Rev: 52 $ 
# $Date: 2006-10-06 05:33:29 +0200 (Fri, 06 Oct 2006) $ 
*/

// auto_ptr_array.h: Similar to the standard auto_ptr_array, 
// but works on arrays instead.  Do not use this for non-array pointers.
// The "ownership transfer" concept is not implemented.  
// The pointer is always owned.

#ifndef INCL_auto_ptr_array_H
#define INCL_auto_ptr_array_H

#include "Global_DllExport.h"

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

template<class T> class auto_ptr_array {
public:
	auto_ptr_array(T *ptr_ = 0) : ptr(ptr_) {}
	~auto_ptr_array() { delete [] ptr; }
	// Assignment from pointer
	void operator=(T* rhs) {
		if (ptr != rhs) {
			delete [] ptr;
			ptr = rhs;
		}
	}
	T& operator*() const { return *get(); }
	operator T* () const { return get(); }
	T* get() const { return ptr; }
private:
	T *ptr;
	// Copy/assignment not allowed
	auto_ptr_array(const auto_ptr_array<T>&);
	auto_ptr_array<T>& operator=(const auto_ptr_array<T>&);
};

#endif
