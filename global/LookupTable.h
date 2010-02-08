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

// LookupTable.h: An object that supports fast initialization
// and lookup of key values.

#ifndef INCL_LOOKUPTABLE_H
#define INCL_LOOKUPTABLE_H

#include "TsString.h"
#include "Trie.h"
#include "BulkAllocator.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class LookupTable : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Default construction and destruction
		///////////////////////////////////////////////////////////////////////////////
		LookupTable() {}
		virtual ~LookupTable() {}

		///////////////////////////////////////////////////////////////////////////////
		// Insert a null-terminated string into the table.
		// Inputs:
		//	const char*		key			A null-terminated key string
		//	const char*		data		Pointer to datum (not terminated).
		// Outputs:
		//	TsString&	errorMsg	If an error occurs (false return), this will
		//								contain the message.
		// Return value:
		//	bool		true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////////
		bool Insert(
			const char* key,
			const char* value,
			TsString& errorMsg
		) {
			return Insert(key, value, (unsigned short)(strlen(value) + 1), errorMsg);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Insert a length-prefixed value into the table.
		// Inputs:
		//	const char*		key			A null-terminated key string
		//	const char*		data		Pointer to datum (not terminated).
		//	unsigned short	length		Length of datum.
		// Outputs:
		//	TsString&	errorMsg	If an error occurs (false return), this will
		//								contain the message.
		// Return value:
		//	bool		true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////////
		bool Insert(
			const char* key,
			const char* value,
			unsigned short length,
			TsString& errorMsg
		);

		///////////////////////////////////////////////////////////////////////////////
		// Given a key, find the associated null-terminated string .
		// Inputs:
		//	const char*		key				The key to find
		// Outputs:
		//	const char*&	valueReturn		The found value
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool Find(
			const char* key,
			const char*& valueReturn
		) {
			unsigned short length;		// dummy
			return Find(key, valueReturn, length);
		}
		bool Find(const unsigned char* key, const unsigned char*& valueReturn) {
			return Find((const char*)key, (const char*&)valueReturn);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Given a key, find the associated length-prefixed value.
		// Inputs:
		//	const char*		key				The key to find
		// Outputs:
		//	const char*&	valueReturn		The found value
		//	unsigned short&	lengthReturn	The returned length
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool Find(
			const char* key,
			const char*& valueReturn,
			unsigned short& lengthReturn
		);

		///////////////////////////////////////////////////////////////////////////////
		// Given a value, find the longest entry that matches a prefix of the value.
		// Inputs:
		//	const char*		value			The value to search
		// Outputs:
		//	const char*&	valueReturn		The found value
		//	int&			prefixLengthReturn	The length of the matched prefix
		// Return value:
		//	bool		true if found, false otherwise
		///////////////////////////////////////////////////////////////////////////////
		bool FindLongestPrefix(
			const char* key,
			const char*& valueReturn,
			int& prefixLengthReturn
		);
		bool FindLongestPrefix(const unsigned char* key, const unsigned char*& valueReturn, int& prefixLengthReturn) {
			return FindLongestPrefix((const char*)key, (const char*&)valueReturn, prefixLengthReturn);
		}



		///////////////////////////////////////////////////////////////////////////////
		// Clear the lookup table.
		///////////////////////////////////////////////////////////////////////////////
		void Clear();

		//////////////////////////////////////////////////////////////////////
		// Load a table from a comma- or tab-separated file.  This is simple
		// and does not handle quotes or escapement!
		// Inputs:
		//	const TsString&	filename	Path to file containing table.
		// Outputs:
		//	TsString&		errorMsg	If false is returned, this is the error message.
		// Return value:
		//	bool		true on success, false on error.
		//////////////////////////////////////////////////////////////////////
		bool LoadFromFile(
			const TsString& filename,
			TsString& errorMsg
		);

	protected:
		// Trie that implements the string map
		typedef Trie<const unsigned char*> StringMap;
		StringMap implementation;

		// Allocator for values stored in the trie.
		BulkAllocator bulkAllocator;
	};
	typedef refcnt_ptr<LookupTable> LookupTableRef;

}

#endif

