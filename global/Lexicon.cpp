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

// Lexicon.cpp: An object that uses a Trie to build a lexicon data structure,
// which can be queried for the presence of a string.  This is less complex than
// a DAWG and does not have to be pre-built, but it uses more space.

#include "Global_Headers.h"
#include <stdio.h>
#include "auto_ptr_array.h"
#include "Lexicon.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Does the given key exist?
	// Inputs:
	//	const char*		key				The key to find
	// Return value:
	//	bool		true if found, false otherwise
	///////////////////////////////////////////////////////////////////////////////
	bool Lexicon::Find(
		const char* key
	) {
		return implementation.Find(key) != 0;
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
	bool Lexicon::FindLexiconPrefix(const char* key, int& length)
	{
		return implementation.FindTriePrefix(key, length);
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
	bool Lexicon::FindKeyPrefix(const char* key, int& length)
	{
		return implementation.FindKeyPrefix(key, length);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Like FindKeyPrefix, but finds longest prefix instead of shortest one.
	// Inputs:
	//	const char*		key				The key to find
	// Outputs:
	//	int&					length	Length of the prefix found (if one was found)
	// Return value:
	//	bool		true if found, false otherwise
	///////////////////////////////////////////////////////////////////////////////
	bool Lexicon::FindLongestKeyPrefix(const char* key, int& length)
	{
		return implementation.FindLongestKeyPrefix(key, length);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Clear the lookup table.
	///////////////////////////////////////////////////////////////////////////////
	void Lexicon::Clear() {
		implementation.Clear();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Insert an element into the lexicon
	// Inputs:
	//	const char*		key			A null-terminated key string
	///////////////////////////////////////////////////////////////////////////////
	void Lexicon::Insert(
		const char* key
	) {
		if (implementation.Find(key) == 0) {
			implementation.Insert(key, true);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Initialize from a file
	// Inputs:
	//	const TsString&	filename	The data file to load
	// Outputs:
	//	TsString&		errorMsg	The error message if load fails.
	// Return value:
	//	bool			true on success, false on failure.
	///////////////////////////////////////////////////////////////////////////////
	bool Lexicon::LoadFromFile(
		const TsString& filename,
		TsString& errorMsg
	) {
		FILE* fp = fopen(filename.c_str(), "r");
		if (fp == 0) {
			errorMsg = "Cannot open data file '" + filename + "'";
			return false;
		}

		// Read lines from the lexicon file.
		TsString tmpStr;
		auto_ptr_array<char> buf(new char[bufSize]);
		while (fgets(buf, bufSize, fp)) {
			tmpStr = buf;
			// Remove whitespace
			while (tmpStr.size() > 0 && isspace(tmpStr[tmpStr.size()-1])) {
				tmpStr.resize(tmpStr.size() - 1);
			}
			while (tmpStr.size() > 0 && isspace(tmpStr[0])) {
				tmpStr.erase(tmpStr.begin());
			}
			Insert(tmpStr.c_str());
		}
		fclose(fp);
		return true;
	}
}

