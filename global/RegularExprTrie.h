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

///////////////////////////////////////////////////////////////////////////////
// RegularExprTrie.h: Implementation of trie modified to support regular expression
//                    mapping
///////////////////////////////////////////////////////////////////////////////

#ifndef INCL_REGULAREXPRTRIE_H
#define INCL_REGULAREXPRTRIE_H

#include "Trie.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	typedef Trie<int> TrieBase;

	class RegularExprTrie : public TrieBase {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor/destructor
		///////////////////////////////////////////////////////////////////////////////
		RegularExprTrie()
		{
		}

		virtual ~RegularExprTrie() {
		}

		///////////////////////////////////////////////////////////////////////////////
		// Insert a string and associated data item to the trie.
		// Inptus:
		//	const unsigned char*	key		The key to insert
		//	T						data	The data to assoicate with the key
		//
		// Note: If the key already exists, then the new data will replace the old
		// data in the trie.
		///////////////////////////////////////////////////////////////////////////////
		void Insert(const unsigned char* key, int data)
		{
			TrieBase::Insert(key, data + 1);
		}
		void Insert(const char* key, int data) { Insert((const unsigned char*)key, data); }

		///////////////////////////////////////////////////////////////////////////////
		// Find the data associated with the key.
		// Inputs:
		//	const unsigned char*	key		The key to find
		// Return value:
		//	T		The found value, or T(0) if the key is not found or there
		//			is no data associated with the key.
		///////////////////////////////////////////////////////////////////////////////
		bool Find(const unsigned char* key, int& data)
		{
			data = TrieBase::Find(key);
			if( data == 0 ) {
				return false;
			}
			data--;
			return true;
		}
		bool Find(const char* key, int& data) { return Find((const unsigned char*)key, data); }

	};
}

#endif
