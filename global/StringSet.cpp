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
# $Rev: 51 $ 
# $Date: 2006-10-05 01:07:17 +0200 (Thu, 05 Oct 2006) $ 
*/

// JH_StringSet.cpp: An object that wraps std::set<TsString> in a
// non-inline interface, so as to make it DLL safe.
//
// Some background: std::map<> inherits from std::_Tree, which uses a static
// data member _Nil.  std::_Tree::iterator tests against _Nil in many locations.
// Since _Nil is static, each module gets its own copy.  So a std::map<> created
// in one module will crash in another due to testing against the wrong _Nil.

#include "Global_Headers.h"
#include "StringSet.h"
#include <set>
#include <cassert>
#include <string.h>

namespace PortfolioExplorer {
#if 0
	typedef std::set<TsString> REAL_TYPE;
	#define IMP ((REAL_TYPE*)imp)
	#define CIMP ((const REAL_TYPE*)imp)

	typedef REAL_TYPE::iterator REAL_ITER;
	typedef REAL_TYPE::const_iterator REAL_CONST_ITER;


	static inline void CopyIter(char* buffer, const REAL_ITER& iter) {
		new (buffer) REAL_ITER(iter);
	}

	static inline void CopyConstIter(char* buffer, const REAL_CONST_ITER& iter) {
		new (buffer) REAL_CONST_ITER(iter);
	}

	static inline REAL_ITER RealIter(const StringSet::iterator& iter) {
		return *(REAL_ITER*)iter.buffer;
	}
	static inline REAL_CONST_ITER RealConstIter(const StringSet::const_iterator& iter) {
		return *(REAL_CONST_ITER*)iter.buffer;
	}

	///////////////////////////////////////////////////////////////////////////////
	// methods
	///////////////////////////////////////////////////////////////////////////////
	StringSet::StringSet() {
		imp = (void*)(new REAL_TYPE);
	}

	StringSet::~StringSet() {
		delete IMP;
	}

	// Copy ctor and assignment
	StringSet::StringSet(const StringSet& rhs)
	{
		imp = (void*)(new REAL_TYPE);
		*IMP = *(REAL_TYPE*)rhs.imp;
	}

	StringSet& StringSet::operator=(const StringSet& rhs)
	{
		if (this != &rhs) {
			*IMP = *(REAL_TYPE*)rhs.imp;
		}
		return *this;
	}

	StringSet::iterator StringSet::begin()
	{
		StringSet::iterator iter;
		CopyIter(iter.buffer, IMP->begin());
		return iter;
	}

	StringSet::const_iterator StringSet::begin() const
	{
		StringSet::iterator iter;
		CopyConstIter(iter.buffer, CIMP->begin());
		return iter;
	}

	StringSet::iterator StringSet::end()
	{
		StringSet::iterator iter;
		CopyIter(iter.buffer, IMP->end());
		return iter;
	}

	StringSet::const_iterator StringSet::end() const
	{
		StringSet::iterator iter;
		CopyConstIter(iter.buffer, CIMP->end());
		return iter;
	}

	StringSet::size_type StringSet::size() const
	{
		return StringSet::size_type(CIMP->size());
	}

	bool StringSet::empty() const
	{
		return CIMP->empty();
	}

	StringSet::_Pairib StringSet::insert(const value_type& ty)
	{
		std::pair<REAL_TYPE::iterator, bool> realIbPair = IMP->insert(ty);
		_Pairib ibPair;
		CopyIter(ibPair.first.buffer, realIbPair.first);
		ibPair.second = realIbPair.second;
		return ibPair;
	}

	void StringSet::erase(iterator it)
	{
		IMP->erase(RealIter(it));
	}

	void StringSet::clear()
	{
		IMP->clear();
	}

	StringSet::iterator StringSet::find(const TsString& _Kv)
	{
		StringSet::iterator iter;
		CopyIter(iter.buffer, IMP->find(_Kv));
		return iter;
	}

	StringSet::const_iterator StringSet::find(const TsString& _Kv) const
	{
		StringSet::const_iterator iter;
		CopyConstIter(iter.buffer, CIMP->find(_Kv));
		return iter;
	}

	StringSet::iterator StringSet::lower_bound(const TsString& _Kv)
	{
		StringSet::iterator iter;
		CopyIter(iter.buffer, IMP->lower_bound(_Kv));
		return iter;
	}

	StringSet::const_iterator StringSet::lower_bound(const TsString& _Kv) const
	{
		StringSet::const_iterator iter;
		CopyConstIter(iter.buffer, CIMP->lower_bound(_Kv));
		return iter;
	}

	StringSet::iterator StringSet::upper_bound(const TsString& _Kv)
	{
		StringSet::iterator iter;
		CopyIter(iter.buffer, IMP->upper_bound(_Kv));
		return iter;
	}

	StringSet::const_iterator StringSet::upper_bound(const TsString& _Kv) const
	{
		StringSet::const_iterator iter;
		CopyConstIter(iter.buffer, CIMP->upper_bound(_Kv));
		return iter;
	}

	///////////////////////////////////////////////////////////////////////////////
	// const_iterator
	///////////////////////////////////////////////////////////////////////////////
	StringSet::const_iterator::const_iterator() 
	{
		assert(sizeof(REAL_ITER) == IteratorSize);
		REAL_CONST_ITER iter;
		CopyConstIter(buffer, iter);
	}

	StringSet::const_iterator::const_iterator(const const_iterator& ty)
	{
		memcpy(buffer, ty.buffer, IteratorSize);
	}

	StringSet::const_iterator::~const_iterator()
	{
	  ((REAL_CONST_ITER*)buffer)->~REAL_CONST_ITER();
	}

	StringSet::const_reference StringSet::const_iterator::operator*() const
	{
		return *RealConstIter(*this);
	}

	StringSet::const_iterator& StringSet::const_iterator::operator++() 
	{
		// Pre-increment
		REAL_CONST_ITER iter = RealConstIter(*this);
		++iter;
		CopyConstIter(buffer, iter);
		return *this;
	}

	StringSet::const_iterator StringSet::const_iterator::operator++(int)
	{
		// Post-increment
		const_iterator tmp(*this);
		REAL_CONST_ITER iter = RealConstIter(*this);
		++iter;
		CopyConstIter(buffer, iter);
		return tmp;
	}

	StringSet::const_iterator& StringSet::const_iterator::operator--()
	{
		// Pre-decrement
		REAL_CONST_ITER iter = RealConstIter(*this);
		--iter;
		CopyConstIter(buffer, iter);
		return *this;
	}

	StringSet::const_iterator StringSet::const_iterator::operator--(int)
	{
		// Post-decrement
		const_iterator tmp(*this);
		REAL_CONST_ITER iter = RealConstIter(*this);
		--iter;
		CopyConstIter(buffer, iter);
		return tmp;
	}

	bool StringSet::const_iterator::operator==(const const_iterator& ty) const
	{
		return RealConstIter(*this) == RealConstIter(ty);
	}

	bool StringSet::const_iterator::operator!=(const const_iterator& ty) const
	{
		return RealConstIter(*this) != RealConstIter(ty);
	}
#endif
}


