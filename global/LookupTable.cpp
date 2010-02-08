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

// LookupTable.cpp: An object that supports fast initialization
// and lookup of key values.

#include "Global_Headers.h"
#include <stdio.h>
#include "LookupTable.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Read a two-value CSV line
	// Inputs:
	//	FILE*			fp			The file to read
	// Outputs:
	//	TsString&	key			The first value
	//	TsString&	value		The second value
	//	bool			haveError	If return value is false, this indiciates an
	//								error (as opposed to EOF).
	// Return value:
	//	bool			true on success, false on error or EOF
	///////////////////////////////////////////////////////////////////////////////
	static bool ReadTwoValueCsvLine(
		FILE* fp,
		TsString& key,
		TsString& value,
		bool haveError
	) {
		// Read first field
		key = "";
		value = "";
		haveError = false;

		bool fieldEnd = false;
		int c;
		while (!fieldEnd && (c = fgetc(fp)) != EOF) {
			switch (c) {
			case '\n':
			case '\r':
			{
				// Error.
				haveError = true;
				return false;
			}
			case '\t':
			case ',':
				// End of field
				fieldEnd = true;
				break;
			default:
				key += c;
				break;
			}
		}

		// Read second field.
		fieldEnd = false;
		while (!fieldEnd && (c = fgetc(fp)) != EOF) {
			switch (c) {
			case '\r':
			        c = fgetc(fp);
				if( c == '\n' || c == EOF ) {
				  fieldEnd = true;
				  break;
				}
				//Not end of field.  Put \r back
				ungetc(c, fp);
				break;
			case '\n':
			        // End of field
			        fieldEnd = true;
			        break;
			case '\t':
			case ',':
				// Extra separator in field.  Ignore it.
				continue;
			default:
				value += c;
				break;
			}
		}

		return !key.empty() && !value.empty();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Clear the lookup table.
	///////////////////////////////////////////////////////////////////////////////
	void LookupTable::Clear() {
		implementation.Clear();
		bulkAllocator.Reset();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Insert a length-prefixed value into the table.
	// The value storage mode must be set to ValueLengthPrefixed
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
	bool LookupTable::Insert(
		const char* key,
		const char* value,
		unsigned short length,
		TsString& errorMsg
	) {
		if (implementation.Find(key) != 0) {
			errorMsg = TsString("Lookup table input contains duplicate key value '") + key + "'";
			return false;
		}
		
		// Make a copy of the data with space for the length prefix.
		// This will be deleted when the table is cleared.
		unsigned char* ptr = (unsigned char*)bulkAllocator.New((int)length + 2);
		ptr[0] = (unsigned char)(length >> 8);
		ptr[1] = (unsigned char)(length);
		memcpy(ptr + 2, value, length);
		implementation.Insert(key, ptr);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Given a key, find the associated length-prefixed value.
	// The value storage mode must be set to ValueLengthPrefixed
	// Inputs:
	//	const char*		key				The key to find
	// Outputs:
	//	const char*&	valueReturn		The found value
	//	unsigned short&	length			the length return
	// Return value:
	//	bool		true if found, false otherwise
	///////////////////////////////////////////////////////////////////////////////
	bool LookupTable::Find(
		const char* key,
		const char*& valueReturn,
		unsigned short& lengthReturn
	) {
		const unsigned char* ptr = implementation.Find(key);
		if (ptr == 0) {
			return false;
		}
		valueReturn = (const char*)ptr + 2;
		lengthReturn = (ptr[0] << 8) | (ptr[1]);
		return true;
	}

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
	bool LookupTable::FindLongestPrefix(
		const char* key,
		const char*& valueReturn,
		int& prefixLengthReturn
	) {
		const unsigned char* ptr;
		if (
			!implementation.FindLongestKeyPrefix(key, ptr, prefixLengthReturn) ||
			ptr == 0
		) {
			return false;
		}
		valueReturn = (const char*)ptr + 2;
		return true;
	}

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
	bool LookupTable::LoadFromFile(
		const TsString& filename,
		TsString& errorMsg
	) {
		errorMsg = "";
		FILE* fp = fopen(filename.c_str(), "r");
		if (fp == 0) {
			errorMsg = "Cannot open data file '" + filename + "'";
			return false;
		}

		TsString key;
		TsString value;
		TsString tmp;
		int line = 1;

		while (!feof(fp) && !ferror(fp)) {
			bool haveError = false;
			if (!ReadTwoValueCsvLine(fp, key, value, haveError)) {
				if (haveError) {
					char buf[10];
#if defined(UNIX)
					snprintf(buf, sizeof(buf), "%d", line);
#else
					_snprintf(buf, sizeof(buf), "%d", line);
#endif
					errorMsg = TsString("Error reading line ") + buf + " of file '" + filename + "'";
					return false;
				}
				break;
			} else {
				line++;
				Insert(key.c_str(), value.c_str(), tmp);
			}
		}

		fclose(fp);
		return true;
	}

}
