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

////////////////////////////////////////////////////////////////////////////
// RefPtr.h:  
//     reference-counting "pointer" object for handling pointers
//     to reference-counted objects implementing the following interface:
//         void ref()      increment refcount
//         void deref()    decrement refcount
//         unsigned nref()	get refcount
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Although this is listed as "platform-specific", the only thing that
// really must be re-implemented when porting are the calls to
// InterlockedIncrement and InterlockedDecrement, which perform atomic
// increment and decrements.
//
// Note: When porting to a new platform, we should use ACE_Atomic_Op
////////////////////////////////////////////////////////////////////////////

#ifndef INCL_REFPTR_H
#define INCL_REFPTR_H

#if _MSC_VER >= 1000
#pragma once
#endif

#if (defined(WIN32) || defined(_WIN32)) && defined(_MT)
	// multithreaded under Win32
	#define REFCOUNT_USE_WIN32_INTERLOCK
#elif defined(UNIX)
        //no lock
#else
	#define REFCOUNT_USE_ACE_ATOMIC_OP
#endif

#if defined(REFCOUNT_USE_WIN32_INTERLOCK)
	// Using in MFC
	//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
	//#include <afxwin.h>         // MFC core and standard components
#elif defined(REFCOUNT_USE_ACE_ATOMIC_OP)
	#include "ace/Atomic_Op.h"
#endif

////////////////////////////////////////////////////////////////////////////
// Generic reference-counter class.  Inheriting from this to
// get the functionality required by refcnt_ptr<T>. Note that ref() and
// deref() both must work for const objects.
////////////////////////////////////////////////////////////////////////////
class RefCount
{
	friend struct refcnt_ptr_base;
public:
	// default ctor
	RefCount() : refcount(0) {}
	// copy ctor *does not* copy refcount value
	RefCount(const RefCount&) : refcount(0) {}
	// assignment operator *does not* copy refcount
	RefCount& operator=(const RefCount&) { return *this; }

private:
	// Counting interface
	// Allow refcount operations on const objects
	// All ref/deref return the resulting new refcount
	// For Win32 multithreading, use the atomic increment-and-check
	// or decrement-and-check operations.
	#ifdef REFCOUNT_USE_WIN32_INTERLOCK
		typedef long REFCOUNT_TYPE;
		// Win32 multi-thread versions
		long ref() const { 
			return InterlockedIncrement(&refcount);
		}
		long deref() const { 
			return InterlockedDecrement(&refcount);
		}
		long nref() const { return refcount; }
	#elif defined(REFCOUNT_USE_ACE_ATOMIC_OP)
		typedef ACE_Atomic_Op<ACE_Thread_Mutex, long> REFCOUNT_TYPE;
		long ref() const { return ++refcount; }
		long deref() const { return --refcount; }
		// Note: nref is NOT mutex-protected.
		long nref() const { return refcount.value_i(); }
	#else
		typedef long REFCOUNT_TYPE;
		// Non-multi-thread versions
		long ref() const { return ++refcount; }
		long deref() const { return --refcount; }
		long nref() const { return refcount; }
	#endif

protected:
	// Only let destruction occur via derived class, because
	// this is not a virtual dtor
	~RefCount() {}
private:
	mutable REFCOUNT_TYPE refcount;
};

////////////////////////////////////////////////////////////////////////////
// Reference-counter base class with virtual dtor, so that objects
// may be deleted by pointer-to-VRefCount
////////////////////////////////////////////////////////////////////////////
class VRefCount : public RefCount
{
public:
   // no need to implement ctor or operator=, as memberwise copy is sufficient
   virtual ~VRefCount() {}
};

////////////////////////////////////////////////////////////////////////////
// Const reference-counted pointer
////////////////////////////////////////////////////////////////////////////

// forward declarations
template <class T> class refcnt_ptr;

////////////////////////////////////////////////////////////////////////////
// Base class gives ref-counted pointers access to RefCount private methods.
// 
// !!!! DO NOT INHERIT FROM THIS UNLESS YOU ARE IMPLEMENTING A SPECIAL-CASE
// REF-COUNTED POINTER !!!!
////////////////////////////////////////////////////////////////////////////
struct refcnt_ptr_base {
	long do_ref(const RefCount* rc) const { return rc->ref(); }
	long do_deref(const RefCount* rc) const { return rc->deref(); }
	long do_nref(const RefCount* rc) const { return rc->nref(); }
};

////////////////////////////////////////////////////////////////////////////
// const ref-counted ptr
////////////////////////////////////////////////////////////////////////////
template<class T> class refcnt_cptr : public refcnt_ptr_base {
   friend class refcnt_ptr<T>;

public:
   // Basic ctor
   refcnt_cptr(const T* ptr_ = 0);

   // copy ctor
   refcnt_cptr(const refcnt_cptr& rhs);

   // dtor
   ~refcnt_cptr();

   // Access to pointer
   const T* get() const { return ptr; }
   const T* operator->() const { return get(); }
   const T& operator*() const { return *get(); }

   // assignment from dumb pointer
   refcnt_cptr& operator=(const T* rhs);

   // Assignment from smart const pointer (also supports non-const)
   refcnt_cptr& operator=(const refcnt_cptr& rhs);

   // Comparison (also supports comparison to non-const ptr)
   bool operator==(const refcnt_cptr& rhs) const
   {
	  return ptr == rhs.get();
   }
   bool operator!=(const refcnt_cptr& rhs) const
   {
	  return ptr != rhs.get();
   }
   // Used for ordering in maps
   bool operator<(const refcnt_cptr& rhs) const
   {
	  return ptr < rhs.get();
   }
   bool operator>(const refcnt_cptr& rhs) const
   {
	  return ptr > rhs.get();
   }

	// What is the current reference count?  Note that if multiple threads are
	// accessing the same ptr, this can produce incorrect results.  However, if 
	// the result==1, this can be relied upon (because this is the only owner).
	//
	// Does NOT check for ptr==0 for performance reasons; 
	// the caller must perform that check if NULL is possible.
	long nref() const { return do_nref(ptr); }

private:
   // Decrement refcount on pointed-to object and delete if zero
   void release();
   void ref() const;
   void deref() const;
   const T* ptr;
};

//
// This are outside of the class because MSVC++ instantiates everything
// declared in the class even if it isn't used
//

template<class T>
inline refcnt_cptr<T>::refcnt_cptr(const T* ptr_) : ptr(ptr_) 
{ 
   if (ptr) ref(); 
}

template<class T>
inline refcnt_cptr<T>::refcnt_cptr(const refcnt_cptr& rhs) : ptr(rhs.get()) 
{ 
   if (ptr) ref();
}

template<class T>
inline refcnt_cptr<T>::~refcnt_cptr() 
{
   release(); 
}

template<class T>
inline refcnt_cptr<T>& refcnt_cptr<T>::operator=(const T* rhs)
{
	// works for rhs==this
	if (ptr != rhs) {
		release();
		if (rhs != 0) {
			ptr = rhs;
			do_ref(ptr);
		}
	}
	return *this;
}

template<class T>
inline refcnt_cptr<T>& refcnt_cptr<T>::operator=(const refcnt_cptr& rhs)
{
	return operator=(rhs.ptr);
}

template<class T>
inline void refcnt_cptr<T>::release()
{ 
   if (ptr) 
   { 
	  if (do_deref(ptr) == 0)
	  {
		 // delete object even if it is const
		 delete const_cast<T*>(ptr);
	  }
	  ptr = 0;
   }
}

template<class T>
inline void refcnt_cptr<T>::ref() const 
{
   do_ref(ptr); 
}

template<class T>
inline void refcnt_cptr<T>::deref() const 
{
   do_deref(ptr);
}

////////////////////////////////////////////////////////////////////////////
// non-const refcounted ptr -- note that a "const refcnt_ptr<T>" is *not* the
// same as a "refcnt_cptr<T>" -- the former still allows access to
// the non-const pointed-to object
////////////////////////////////////////////////////////////////////////////
template<class T> class refcnt_ptr : public refcnt_cptr<T>
{
public:
   // basic ctor
   refcnt_ptr(T* ptr_ = 0);

   // copy ctor
   // Don't allow construction from refcnt_cptr
   refcnt_ptr(const refcnt_ptr& rhs);

   // assignment operators
   // Don't allow assignment from refcnt_cptr
   refcnt_ptr& operator=(const refcnt_ptr& rhs);
   refcnt_ptr& operator=(T* rhs);

   // Access operators (non-const access to object)
   T* get() const { return const_cast<T*>(this->ptr); }
   T* operator->() const { return get(); }
   T& operator*() const { return *get(); }
};

//
// This are outside of the class because MSVC++ instantiates everything
// declared in the class even if it isn't used
//

template<class T>
inline refcnt_ptr<T>::refcnt_ptr(T* ptr_) : 
   refcnt_cptr<T>(ptr_) 
{}

template<class T>
inline refcnt_ptr<T>::refcnt_ptr(const refcnt_ptr& rhs) : 
   refcnt_cptr<T>(rhs.get()) 
{}

template<class T>
inline refcnt_ptr<T>& refcnt_ptr<T>::operator=(const refcnt_ptr& rhs)
{
   refcnt_cptr<T>::operator=(rhs);
   return *this;
}

template<class T>
inline refcnt_ptr<T>& refcnt_ptr<T>::operator=(T* rhs)
{
   refcnt_cptr<T>::operator=(rhs);
   return *this;
}

// Generic base-reference
typedef refcnt_ptr<VRefCount> Ref;



#endif
