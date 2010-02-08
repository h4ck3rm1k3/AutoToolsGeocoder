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
# $Rev: 56 $ 
# $Date: 2006-11-28 21:53:56 +0100 (Tue, 28 Nov 2006) $ 
*/

// Utility.h:  Some common functions 

#ifndef INCL_UTILITY_H
#define INCL_UTILITY_H

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <vector>
#include <cstring>
#include "Basics.h"
#include "Global_DllExport.h"
#include "TsString.h"


namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// return a string consisting of spaces of length tabCount * tabStop
	///////////////////////////////////////////////////////////////////////////////
	TsString GetTabAsSpaces(unsigned int tabCount, int tabStop = 2);

	///////////////////////////////////////////////////////////////////////////////
	// Add double-quotes to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString Quote(TsString s);

	///////////////////////////////////////////////////////////////////////////////
	// Remove double-quotes from the start and end of a string.
	// Leave the string untouched if not quoted.
	///////////////////////////////////////////////////////////////////////////////
	TsString StripQuotes(TsString);

	///////////////////////////////////////////////////////////////////////////////
	// Remove double- and single-quotes from the start and end of a string.
	// Leave the string untouched if not quoted.
	///////////////////////////////////////////////////////////////////////////////
	TsString StripDSQuotes(TsString);

	///////////////////////////////////////////////////////////////////////////////
	// Escape any embedded quotes and backslashes in a quoted string.
	// The quotes are assumed to already exist at the front and end of the string.
	// Inputs:
	//	const string&	text		The string to escape.
	// Return value:
	//	string		The escaped string.
	///////////////////////////////////////////////////////////////////////////////
	TsString EscapeQuotedString(const TsString& text);

	///////////////////////////////////////////////////////////////////////////////
	// Given a string, escape any non-printing characters, blackslashes, and quotes
	// according to the standing C/C++ language interpretation.  Does not process
	// octal or hex escapes (e.g. \007 or \x7).
	// Inputs:
	//	const string&	text		The string to escape.
	// Return value:
	//	string		The escaped string.
	///////////////////////////////////////////////////////////////////////////////
	TsString EscapeString_C(const TsString& text);

	///////////////////////////////////////////////////////////////////////////////
	// Given a string from the above function, "unescape" it.
	// Inputs:
	//	const string&	text		The string to unescape.
	// Return value:
	//	string		The unescaped string.
	///////////////////////////////////////////////////////////////////////////////
	TsString UnescapeString_C(const TsString& text);

	///////////////////////////////////////////////////////////////////////////////
	// Function to return a string of spaces with length equal to the argument
	///////////////////////////////////////////////////////////////////////////////
	TsString Spaces(int length);

	///////////////////////////////////////////////////////////////////////////////
	// Function to return a left justified str, right filled with spaces
	///////////////////////////////////////////////////////////////////////////////
	TsString LeftJustified(
		const TsString & str,
		int                 str_field_width
	);
	inline TsString LeftJustifiedString(
		const TsString & str,
		int                 str_field_width
	) {
		return LeftJustified(str, str_field_width);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to return a right justified str, left filled with spaces
	///////////////////////////////////////////////////////////////////////////////
	TsString RightJustified(
		const TsString & str,
		int str_field_width
	);

	///////////////////////////////////////////////////////////////////////////////
	// Function to return a string with all tabs expanded
	///////////////////////////////////////////////////////////////////////////////
	TsString ExpandTabs(
		const TsString &line_with_unexpanded_tabs,
		int tab_size
	);
 
	///////////////////////////////////////////////////////////////////////////////
	// Function to convert an integer to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatInteger(int value);
	TsString FormatInteger(unsigned int value);
	TsString FormatInteger(__int64 value);
	TsString FormatIntegerHex(int value);
	TsString FormatIntegerWithFill(int value, int fillSize);

	///////////////////////////////////////////////////////////////////////////////
	// Function to convert a floating-point value to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatFloat(double value);

	///////////////////////////////////////////////////////////////////////////////
	// Function to convert a bol to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatBool(bool value);

	///////////////////////////////////////////////////////////////////////////////
	// Function to scan integer values from a string
	///////////////////////////////////////////////////////////////////////////////
	bool ScanInteger(const char* str, int& retval);
	bool ScanInteger(const char* str, unsigned int& retval);
	bool ScanInteger(const char* str, __int64& retval);

	///////////////////////////////////////////////////////////////////////////////
	// Function to scan float values from a string
	///////////////////////////////////////////////////////////////////////////////
	bool ScanFloat(const char* str, float& retval);

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove trailing characters from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	//	char					c		The character to remove
	// Return value:
	//	TsString		The string without the trailing character(s)
	///////////////////////////////////////////////////////////////////////////////
	TsString StripTrailing(const TsString& str, char c);

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove trailing chars of type isspace from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	// Return value:
	//	TsString		The string without the trailing chars of type isspace
	///////////////////////////////////////////////////////////////////////////////
	TsString StripTrailingSpace(const TsString& str);
	
	///////////////////////////////////////////////////////////////////////////////
	// Function to remove leading characters from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	//	char					c		The character to remove
	// Return value:
	//	TsString		The string without the leading character(s)
	///////////////////////////////////////////////////////////////////////////////
	TsString StripLeading(const TsString& str, char c);

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove leading of chars type isspace from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	// Return value:
	//	TsString		The string without the leading chars of type isspace
	///////////////////////////////////////////////////////////////////////////////
	TsString StripLeadingSpace(const TsString& str); 

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove leading and trailing of chars type isspace from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	// Return value:
	//	TsString		The string without the leading and trailing chars of type isspace
	///////////////////////////////////////////////////////////////////////////////
	TsString TrimSpaces(const TsString& str);

	///////////////////////////////////////////////////////////////////////////////
	// Separate a multi-line message into parts.
	// Inputs:
	//	const TsString&			inputString		The input string
	//	TsString&				separator		The separator string.
	// Outputs:
	//	std::vector<TsString>&	tokens			The resulting tokens
	///////////////////////////////////////////////////////////////////////////////
	void TokenizeString(
		const TsString& inputString,
		const TsString& separator,
		std::vector<TsString> &tokens
	);

	///////////////////////////////////////////////////////////////////////////////
	// Function to format an absolute time, including the Day, and AM, PM
	//
	// Return value:
	//	string		The current formatted time as in: Wed Nov 11 09:14:44 AM 1998
	///////////////////////////////////////////////////////////////////////////////
 	TsString FormatTimestamp();

	///////////////////////////////////////////////////////////////////////////////
	// Function to format an absolute time
	//
	// Note that <time.h> is required for time_t.
	//
	// Return value:
	//	string		The formatted time in HH:MM:SS format.
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatTimeAbsolute(time_t timeVal);

	///////////////////////////////////////////////////////////////////////////////
	// Function to format an elapsed time.
	// This does not actually calculate a time difference, it just formats
	// it as a delta time instead of an absolute time.
	//
	// Note that <time.h> is required for time_t.
	//
	// Inputs:
	// time_t timeVal         the time in seconds to be formatted
	//
	// Return value:
	//	string		The formatted time in HH:MM:SS format.
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatTimeElapsed(time_t timeVal);  

	///////////////////////////////////////////////////////////////////////////////
	// Function to format an elapsed time interval.
	// This function calculates a time difference, from the timeAtStart to the
	// current time, then returns the results of the function FormatTimeElapsed
	// for the calculated time interval. 
	//
	// Note that <time.h> is required for time_t.
	//
	// Inputs:
	// time_t timeAtStart         the time in seconds at the start of interval
	//
	// Return value:
	//	string		The formatted time in HH:MM:SS format.
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatTimeInterval(time_t timeAtStart, int divisor = 1); 

	///////////////////////////////////////////////////////////////////////////
	// Routines that are faster inline equivalents to the standard routines.
	///////////////////////////////////////////////////////////////////////////
	extern unsigned char toUpperTable[256];
	extern unsigned char toLowerTable[256];
	extern bool isDigitTable[256];
	extern bool isAlphaTable[256];
	extern bool isAlnumTable[256];
	inline unsigned char TOUPPER(unsigned char c) { return toUpperTable[c]; };
	inline unsigned char TOLOWER(unsigned char c) { return toLowerTable[c]; };
	inline bool ISDIGIT(unsigned char c) { return isDigitTable[c]; };
	inline bool ISALPHA(unsigned char c) { return isAlphaTable[c]; };
	inline bool ISALNUM(unsigned char c) { return isAlnumTable[c]; };
	inline void STRUPPER(char* str) {
		while (*str != 0) {
			*str = TOUPPER(*str);
			str++;
		}
	}
	inline void STRUPPER(unsigned char* str) { STRUPPER((char*)str); }
	inline void STRLOWER(char* str) {
		while (*str != 0) {
			*str = TOLOWER(*str);
			str++;
		}
	}
	inline void STRLOWER(unsigned char* str) { STRLOWER((char*)str); }
	inline int STRICMP(const char* str1, const char* str2) {
		while (*str1 != 0 && TOLOWER(*str1) == TOLOWER(*str2)) {
			str1++;
			str2++;
		}
		return (int)*(const unsigned char*)str1 - (int)*(const unsigned char*)str2;
	}
	inline int STRICMP(const unsigned char* str1, const unsigned char* str2) {
		while (*str1 != 0 && TOLOWER(*str1) == TOLOWER(*str2)) {
			str1++;
			str2++;
		}
		return (int)*str1 - (int)*str2;
	}
	// case-insensitive memcmp()
	inline int MEMICMP(const unsigned char* str1, const unsigned char* str2, size_t n) {
		while (n > 0 && TOLOWER(*str1) == TOLOWER(*str2)) {
			str1++;
			str2++;
			n--;
		}
		return (n == 0) ? 0 : ((int)*str1 - (int)*str2);
	}
	inline int MEMICMP(const char* str1, const char* str2, size_t n) {
		return MEMICMP((const unsigned char*)str1, (const unsigned char*)str2, n);
	}
	// Like strcmp(), but with unsigned semantics
	inline int USTRCMP(const unsigned char* str1, const unsigned char* str2) {
		while (*str1 != 0 && *str1 == *str2) {
			str1++;
			str2++;
		}
		return (int)*str1 - (int)*str2;
	}

	// like strncmp, but with unsigned semantics
	inline int USTRNCMP(const unsigned char* str1, const unsigned char* str2, int n) {
		while (*str1 != 0 && *str1 == *str2) {
			n--;
			str1++;
			str2++;
			if (n == 0) {
				return 0;
			}
		}
		return (int)*str1 - (int)*str2;
	}

	inline void STRCPY(unsigned char* dest, const unsigned char* src) {
		strcpy((char*)dest, (const char*) src);
	}
	inline void STRCPY(char* dest, const unsigned char* src) {
		strcpy((char*)dest, (const char*) src);
	}
	inline void STRCPY(unsigned char* dest, const char* src) {
		strcpy((char*)dest, (const char*) src);
	}

	// Like strlen(), but stops at a maximum length if no termination is found
	// before that.
	inline int NSTRLEN(const char* str, int n) {
		int length = 0;
		while (length < n && str[length] != 0) {
			length++;
		}
		return length;
	}
	inline int NSTRLEN(const unsigned char* str, int n) {
		int length = 0;
		while (length < n && str[length] != 0) {
			length++;
		}
		return length;
	}

	///////////////////////////////////////////////////////////////////////////
	// Convert a string to uppercase
	///////////////////////////////////////////////////////////////////////////
	inline TsString Upper(const TsString& str) {
		TsString retval = str;
		for (unsigned i = 0; i < str.size(); i++) {
			retval[i] = TOUPPER(retval[i]);
		}
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////
	// Convert a string to uppercase
	///////////////////////////////////////////////////////////////////////////
	inline TsString Lower(const TsString& str) {
		TsString retval = str;
		for (unsigned i = 0; i < str.size(); i++) {
			retval[i] = TOLOWER(retval[i]);
		}
		return retval;
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Object to initialize the tables.  Declare one of these in any module that
	// uses the ctype-like functions (TOLOWER(), Lower(), etc) in static 
	// initializers.
	//
	// Note: initializer is automatically included if you link Utility.cpp.
	///////////////////////////////////////////////////////////////////////////
	class CharTableInitializer {
	public:
		CharTableInitializer();
	};

	inline void AddHexByteToString(TsString& str, int digit)
	{
		static char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		str += hexDigits[(digit & 0xF0) >> 4];
		str += hexDigits[digit & 0x0F];
	}

	template <class T> inline T T_GLOBAL_MIN(T x, T y) { return x < y ? x : y; }
	inline int GLOBAL_MIN(int x, int y) { return T_GLOBAL_MIN(x,y); }
	inline unsigned int GLOBAL_MIN(unsigned int x, unsigned int y) { return T_GLOBAL_MIN(x,y); }
	inline __int64 GLOBAL_MIN(__int64 x, __int64 y) { return T_GLOBAL_MIN(x,y); }
	inline double GLOBAL_MIN(double x, double y) { return T_GLOBAL_MIN(x,y); }

	template <class T> inline T T_GLOBAL_MAX(T x, T y) { return x > y ? x : y; }
	inline int GLOBAL_MAX(int x, int y) { return T_GLOBAL_MAX(x,y); }
	inline unsigned int GLOBAL_MAX(unsigned int x, unsigned int y) { return T_GLOBAL_MAX(x,y); }
	inline __int64 GLOBAL_MAX(__int64 x, __int64 y) { return T_GLOBAL_MAX(x,y); }
	inline double GLOBAL_MAX(double x, double y) { return T_GLOBAL_MAX(x,y); }

	template <class T> inline T GLOBAL_ABS(T x) { return (x > 0) ? x : -x; }


	///////////////////////////////////////////////////////////////////////////
	// Does lhs start with rhs?
	///////////////////////////////////////////////////////////////////////////
	inline bool StartsWith(const char* lhsval, const char* rhsval)
	{	
		while (*rhsval != 0)
		{
			if (*lhsval != *rhsval) { return false; }
			lhsval++;
			rhsval++;
		}
		return true;
	}
	inline bool StartsWith(const unsigned char* lhsval, const unsigned char* rhsval) {
		return StartsWith((const char*)lhsval, (const char*)rhsval);
	}


} // namespace

#endif
