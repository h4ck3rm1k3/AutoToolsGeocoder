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

// Lexicon.h: An object that uses a Trie to build a lexicon data structure,
// which can be queried for the presence of a string.  This is less complex than
// a DAWG and does not have to be pre-built, but it uses more space.

#ifndef INCL_Lexicon_H
#define INCL_Lexicon_H

#include "TsString.h"
#include "Trie.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class Lexicon : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Default construction and destruction
		///////////////////////////////////////////////////////////////////////////////
		Lexicon(int bufSize_ = 1000) : bufSize(bufSize_) {}
		virtual ~Lexicon() {}

		///////////////////////////////////////////////////////////////////////////////
		// Does the given key exist?
		// Inputs:
		//	const char*		key				The key to find
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool Find(const char* key);
		// Unsigned version
		bool Find(const unsigned char* key) {
			return Find((const char*)key);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Does the given key exist as a prefix of some key in the lexicon?
		// Inputs:
		//	const char*		key				The key to find
		// Outputs:
		//	int&					length	Length of the prefix found (if one was found)
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool FindLexiconPrefix(const char* key, int& length);
		// Unsigned version
		bool FindFindLexiconPrefixPrefix(const unsigned char* key, int& length) {
			return FindLexiconPrefix((const char*)key, length);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Does some key in the lexicon exist that is a prefix of the given key?
		// Inputs:
		//	const char*		key				The key to find
		// Outputs:
		//	int&					length	Length of the prefix found (if one was found)
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool FindKeyPrefix(const char* key, int& length);
		// Unsigned version
		bool FindKeyPrefix(const unsigned char* key, int& length) {
			return FindKeyPrefix((const char*)key, length);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Like FindKetPrefix, but finds the longest prefix.
		// Inputs:
		//	const char*		key				The key to find
		// Outputs:
		//	int&					length	Length of the prefix found (if one was found)
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool FindLongestKeyPrefix(const char* key, int& length);
		// Unsigned version
		bool FindLongestKeyPrefix(const unsigned char* key, int& length) {
			return FindLongestKeyPrefix((const char*)key, length);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Insert an element into the lexicon
		// Inputs:
		//	const char*		key			A null-terminated key string
		///////////////////////////////////////////////////////////////////////////////
		void Insert(const char* key);
		void Insert(const unsigned char* key) {
			Insert((const char*)key);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Clear the lexicon
		///////////////////////////////////////////////////////////////////////////////
		void Clear();

		///////////////////////////////////////////////////////////////////////////////
		// Initialize from a file
		// Inputs:
		//	const TsString&	filename	The data file to load
		// Outputs:
		//	TsString&		errorMsg	The error message if load fails.
		// Return value:
		//	bool			true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////////
		bool LoadFromFile(
			const TsString& filename,
			TsString& errorMsg
		);

	protected:
		// Trie that implements the string map.  Note that the value parameter
		// of the Trie is unused.
		typedef Trie<char> StringMap;
		StringMap implementation;
		int bufSize;
	};
	typedef refcnt_ptr<Lexicon> LexiconRef;

}

#endif

