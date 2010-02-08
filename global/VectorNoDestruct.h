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

// vector<> template that does not destruct its elements when the number of
// active elements is reduced by pop_back(), clear() or resize().  Instead we simply track
// the number of valid elements.

#ifndef INCL_VECTOR_NODESTRUCT_H
#define INCL_VECTOR_NODESTRUCT_H

#include "Global_DllExport.h"
#include <vector>

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Instead of using the underlying size() method, we track the number
	// items and use that as the end-of-vector indicator.
	//////////////////////////////////////////////////////////////////////
	template <class T> class VectorNoDestruct : public std::vector<T> {
		typedef std::vector<T> _mybase_t;
	public:
		typedef typename std::vector<T>::const_iterator const_iterator;
		typedef typename std::vector<T>::iterator iterator;
		typedef typename std::vector<T>::size_type size_type;

		VectorNoDestruct() : active(0) {}

		VectorNoDestruct(const VectorNoDestruct& rhs) 
		{
			_mybase_t::resize(rhs.size());
			active = rhs.size();
			for (int i = 0; i < active; i++) {
				_mybase_t::at(i) = rhs[i];
			}
		}
 
		VectorNoDestruct& operator=(const VectorNoDestruct& rhs) 
		{
			if (*this != rhs) {
				_mybase_t::resize(rhs.size());
				active = rhs.size();
				for (int i = 0; i < active; i++) {
					_mybase_t::at(i) = rhs[i];
				}
			}
			return *this;
		}

		const_iterator end() const {
			return _mybase_t::begin() + active;
		}

		iterator end() {
			return _mybase_t::begin() + active;
		}

		size_type size() const {
			return active;
		}

		bool empty() const { 
			return size() == 0; 
		}

		iterator insert(iterator it, const T& value) {
			active = _mybase_t::size() + 1;
			return _mybase_t::insert(it, value);
		}
		iterator erase(iterator it) {
			active = _mybase_t::size() - 1;
			return _mybase_t::erase(it);
		}
		void push_back(const T& value) {
			if (_mybase_t::size() > active) {
				at(active++) = value;
			} else {
				_mybase_t::push_back(value);
				active = _mybase_t::size();
			}
		}
		void pop_back() {
			assert(active > 0);
			active--;
		}
		void clear() { 
			active = 0;
		}

		// Because construction of T() may be inefficient, we implement resize()
		// both with and without value argument.
		void resize(size_type n) {
			if (_mybase_t::size() < n) {
				// Need to add some default values
				T x = T();
				_mybase_t::resize(n, x);
			}
			active = n;
		}

		void resize(size_type n, T x) {
			if (_mybase_t::size() < n) {
				_mybase_t::resize(n, x);
			}
			active = n;
		}

		// Special interface for accessing the not-yet-destructed item(s) at the
		// end of the list, if one exists.  Returns the new Last() item.
		T& UseExtraOnEnd() {
			assert(active <= _mybase_t::size());
			if (active == _mybase_t::size()) {
				_mybase_t::push_back(T());
			}
			active++;
			return Last();
		}

		// Return the last item in the list.
		T& Last() { return at(active-1); }
		const T& Last() const { return at(active-1); }

	private:
		size_type active;
	};

}


#endif
