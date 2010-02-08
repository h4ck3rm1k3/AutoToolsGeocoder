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

// Utility.cpp:  Some common functions 

#include "Global_Headers.h"
#include "TsString.h"
#include "Utility.h"
#include "Basics.h"
#include <fstream>
#include <stdio.h>
#include <time.h>

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// return a string consisting of spaces of length tabCount * tabStop
	///////////////////////////////////////////////////////////////////////////////
	TsString GetTabAsSpaces(unsigned int tabCount, int tabStop)
	{
		return TsString(tabCount * tabStop, ' ');
	} 

	///////////////////////////////////////////////////////////////////////////////
	// Add double-quotes to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString Quote(TsString s)
	{
		return "\"" + s + "\"";
	}

	///////////////////////////////////////////////////////////////////////////////
	// Remove double-quotes from the start and end of a string.
	// Leave the string untouched if not quoted.
	///////////////////////////////////////////////////////////////////////////////
	TsString StripQuotes(TsString s)
	{
		int last = int(s.size()-1);
		if (s.size() >= 2 && s[0] == '"' && s[last] == '"') {
			return TsString(s.begin()+1, s.end()-1);
		} else {
			return s;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Remove double-quotes from the start and end of a string.
	// Leave the string untouched if not quoted.
	///////////////////////////////////////////////////////////////////////////////
	TsString StripDSQuotes(TsString s)
	{
		int last = int(s.size()-1);
		if (
			s.size() >= 2 && 
			(s[0] == '"' || s[0] == '\'') &&
			(s[last] == '"' || s[last] == '\'')
		) {
			return TsString(s.begin()+1, s.end()-1);
		} else {
			return s;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove trailing characters from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	//	char					c		The character to remove
	// Return value:
	//	TsString		The string without the trailing character(s)
	///////////////////////////////////////////////////////////////////////////////
	TsString StripTrailing(const TsString& str, char c) 
	{
		TsString retval = str;
		while (!retval.empty() && *(retval.end() - 1) == c) {
			retval.resize(retval.size() - 1);
		}
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove trailing chars of type isspace from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	// Return value:
	//	TsString		The string without the trailing chars of type isspace
	///////////////////////////////////////////////////////////////////////////////
	TsString StripTrailingSpace(const TsString& str) 
	{
		TsString retval = str;
		while (!retval.empty() && isspace(*(retval.end() - 1))) {
			retval.resize(retval.size() - 1);
		}
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove leading characters from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	//	char					c		The character to remove
	// Return value:
	//	TsString		The string without the leading character(s)
	///////////////////////////////////////////////////////////////////////////////
	TsString StripLeading(const TsString& str, char c) 
	{
		TsString retval = str;
		unsigned i = 0;
		for (; i < retval.size(); i++) {
			if (retval[i] != c) {
				break;
			}
		}
		return TsString(retval.begin() + i, retval.end());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove leading chars of type isspace from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	// Return value:
	//	TsString		The string without the leading chars of type isspace
	///////////////////////////////////////////////////////////////////////////////
	TsString StripLeadingSpace(const TsString& str) 
	{
		TsString retval = str;
		unsigned i = 0;
		for (; i < retval.size(); i++) {
			if (!isspace(retval[i])) {
				break;
			}
		}
		return TsString(retval.begin() + i, retval.end());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to remove leading and trailing of chars type isspace from a string
	// Inputs:
	//	const TsString&		str		The string to strip
	// Return value:
	//	TsString		The string without the leading and trailing chars of type isspace
	///////////////////////////////////////////////////////////////////////////////
	TsString TrimSpaces(const TsString& str)
	{ 
		return StripLeadingSpace(StripTrailingSpace(str)); 
	}
	
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
	) {
		tokens.clear();

		if (inputString.empty() || separator.empty()) {
			// either the incoming message is empty
			// or no sentinel was supplied so we just return after
			// streaming out the incoming string as is
			tokens.push_back(inputString); 
			return;
		} 

		TsString::size_type token_begin = 0; 
		TsString::size_type token_end = 0;

		// loop through the one string with zero or more separators
		while (
			(token_end = inputString.find(separator, token_end)) != TsString::npos
		) {
			tokens.push_back(
				inputString.substr(
					token_begin, 
					token_end - token_begin
				)
			);

			// now skip over the last sentinel, to the beginning of the next line
			token_begin = token_end = token_end + separator.length();
		}
		
		// Get message after last sentinel
		if (token_begin < inputString.size())  
		{
			// Get remaining stuff on line
			tokens.push_back(
				inputString.substr(token_begin, inputString.size() - token_begin)				
			);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Escape any embedded quotes and backslashes in a quoted string.
	// The quotes are assumed to already exist at the front and end of the string.
	// Inputs:
	//	const string&	text		The string to escape.
	// Return value:
	//	string		The escaped string.
	///////////////////////////////////////////////////////////////////////////////
	TsString EscapeQuotedString(const TsString& str)
	{
		TsString retval = "\"";
		for (int i = 1; i < (int)str.size() - 1; i++) {
			if (str[i] == '\\' || str[i] == '"') {
				retval += '\\';
			}
			retval += str[i];
		}
		retval += "\"";
		return retval;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Given a string, escape any non-printing characters, blackslashes, and quotes
	// according to the standing C/C++ language interpretation.  Does not process
	// octal or hex escapes (e.g. \007 or \x7).
	// Inputs:
	//	const string&	text		The string to escape.
	// Return value:
	//	string		The escaped string.
	///////////////////////////////////////////////////////////////////////////////
	TsString EscapeString_C(const TsString& text)
	{
		TsString result;
		// Make some space to avoid loads of reallocation.
		result.reserve(text.size() + 2);
		for (unsigned i = 0; i < text.size(); i++) {
			char c = 0;
			switch (text[i]) {
			case '\a':
				c =  'a';
				break;
			case '\b':
				c =  'b';
				break;
			case '\f':
				c =  'f';
				break;
			case '\n':
				c =  'n';
				break;
			case '\r':
				c =  'r';
				break;
			case '\t':
				c =  't';
				break;
			case '\'':
				c =  '\'';
				break;
			case '\"':
				c =  '\"';
				break;
			case '\\':
				c =  '\\';
				break;
			default:
				result += text[i];
				break;
			}
			if (c != 0) {
				result += '\\';
				result += c;
			}
		}
		return result;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Given a string from the above function, "unescape" it.
	// Inputs:
	//	const string&	text		The string to unescape.
	// Return value:
	//	string		The unescaped string.
	///////////////////////////////////////////////////////////////////////////////
	TsString UnescapeString_C(const TsString& text)
	{
		TsString result;
		// Make some space to avoid loads of reallocation.
		result.reserve(text.size() + 2);
		for (unsigned i = 0; i < text.size(); i++) {
			if (text[i] == '\\' && i + 1 < text.size()) {
				i++;
				char c = 0;
				switch (text[i]) {
				case 'a':
					c = '\a';
					break;
				case 'b':
					c = '\b';
					break;
				case 'f':
					c = '\f';
					break;
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				case '\'':
					c = '\'';
					break;
				case '\"':
					c = '\"';
					break;
				case '\\':
					c = '\\';
					break;
				default:
					result += '\\';
					result += text[i];
					break;
				}
				if (c != 0) {
					result += c;
				}
			} else {
				result += text[i];
			}
		}
		return result;
	}

	////////////////////////////////////////////////////////////////////////////
	///
	// return a string of spaces with length equal to the argument
	////////////////////////////////////////////////////////////////////////////
	///
	TsString Spaces(int length)
	{
		return TsString(length, ' ');	
	}
	 
	///////////////////////////////////////////////////////////////////////////////
	// return a new string with the str argument left justified, space filled on the 
	// right such that the returned string is of length str_field_width
	///////////////////////////////////////////////////////////////////////////////
	TsString LeftJustified(
		const TsString & str,
		int                 str_field_width)
	{
		int copySize = JHMIN(str_field_width, static_cast<int>(str.size()));
		return
			TsString(str.begin(), str.begin() + copySize) +
			Spaces(str_field_width - copySize);
	}	

	///////////////////////////////////////////////////////////////////////////////
	// return a new string with the str argument right justified, space filled on the 
	// right such that the returned string is of length str_field_width
	///////////////////////////////////////////////////////////////////////////////
	TsString RightJustified(
		const TsString & str,
		int str_field_width)
	{
		int copySize = JHMIN(str_field_width, static_cast<int>(str.size()));
		return
			Spaces(str_field_width - copySize) +
			TsString(str.begin(), str.begin() + copySize);
	}	
	///////////////////////////////////////////////////////////////////////////////
	// Copy the line_with_expanded_tabs string to the line_with_unexpanded_tabs 
	// string, with each tab expanded to tab_size spaces.
	///////////////////////////////////////////////////////////////////////////////
	 TsString ExpandTabs(
		const TsString & line_with_unexpanded_tabs,
		int                 tab_size)
	{
		TsString line_with_expanded_tabs;

		for (unsigned i = 0; i < line_with_unexpanded_tabs.size(); i++) {
			if (line_with_unexpanded_tabs[i] != '\t') {
				line_with_expanded_tabs += line_with_unexpanded_tabs[i];
			} else {
				do {
					line_with_expanded_tabs += ' ';
				} while (line_with_expanded_tabs.size() % tab_size != 0);
			}
		}
		return line_with_expanded_tabs;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to convert an integer to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatInteger(int value)
	{
		char buf[30];
		sprintf(buf, "%d", value);
		return buf;
	}
	TsString FormatInteger(unsigned int value)
	{
		char buf[30];
		sprintf(buf, "%u", value);
		return buf;
	}
	TsString FormatInteger(__int64 value)
	{
		char buf[30];
		sprintf(buf, "%I64d", value);
		return buf;
	}

	// Function to convert an integer to a string
	TsString FormatIntegerHex(int value)
	{ 
		char buf[30]; 
		sprintf(buf, "0x%x", value); 
		return buf;
	}

	// Function to convert an integer to a string, 
	// with a specified right justified fill size
	TsString FormatIntegerWithFill(int value, int fillSize)
	{
		char buf[30];
		sprintf(buf, "%*d", fillSize, value);
		return buf;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to convert a float to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatBool(bool value)
	{ 
		char buf[30]; 
		sprintf(buf, "%s", value ? "true" : "false"); 
		return buf;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to scan integer values from a string
	///////////////////////////////////////////////////////////////////////////////
	bool ScanInteger(const char* str, int& retval)
	{
		retval = 0;
		return sscanf(str, "%d", &retval) == 1;
	}
	bool ScanInteger(const char* str, unsigned int& retval)
	{
		retval = 0;
		return sscanf(str, "%u", &retval) == 1;
	}
	bool ScanInteger(const char* str, __int64& retval)
	{
		retval = 0;
		#if defined(WIN32)
			return sscanf(str, "%I64d", &retval) == 1;
		#elif defined (UNIX)
			return sscanf(str, "%Ld", &retval) == 1;
		#else
			#error "Must implement I64 scanning"
		#endif

	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to scan float values from a string
	///////////////////////////////////////////////////////////////////////////////
	bool ScanFloat(const char* str, float& retval)
	{
		retval = 0.0;
		return sscanf(str, "%f", &retval) == 1;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to convert a floating-piont value to a string
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatFloat(double value)
	{
		char buf[80];
		sprintf(buf, "%g", value);
		return buf;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to format an absolute time, including the Day, and AM, PM
	//
	// Return value:
	//	string		The current formatted time as in: Wed Nov 11 09:14:44 AM 1998
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatTimestamp()
	{
		TsString     time_string;        
		TsString     am_pm(" AM ");        
    
		struct tm *newtime;        
		time_t     long_time;

		time( &long_time );                /* Get time as long integer. */
		newtime = localtime( &long_time ); /* Convert to local time. */
		if( newtime->tm_hour > 12 )        /* Set up extension. */
				am_pm = " PM ";
		if( newtime->tm_hour > 12 )        /* Convert from 24-hour */
				newtime->tm_hour -= 12;    /*   to 12-hour clock.  */
		if( newtime->tm_hour == 0 )        /*Set hour to 12 if midnight. */
				newtime->tm_hour = 12;

		char *time_char_string = asctime( newtime );
		time_char_string[19] = '\0'; // split off the trailing year
		time_char_string[24] = '\0'; // remove the \n off the trailing year
		time_string += time_char_string + am_pm ;
		time_string += (time_char_string + 20) ; // append the year

		return time_string;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Function to format an absolute time
	//
	// Note that <time.h> is required for time_t.
	//
	// Return value:
	//	string		The formatted time in HH:MM:SS format.
	///////////////////////////////////////////////////////////////////////////////
	TsString FormatTimeAbsolute(time_t timeVal)
	{
		char buf[200];
		struct tm *timePtr = localtime(&timeVal);
		struct tm tmVal;
		if (timePtr == 0) {
			memset(&tmVal, 0, sizeof(tmVal));
		} else {
			tmVal = *timePtr;
		}
		sprintf(buf, "%02d:%02d:%02d", tmVal.tm_hour, tmVal.tm_min, tmVal.tm_sec);
		return buf;
	}

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
	TsString FormatTimeElapsed(time_t timeVal) 
	{
		char buf[200];
		int hours, minutes, seconds;
		hours = int(timeVal / 3600);
		timeVal -= hours * 3600;
		minutes = int(timeVal / 60);
		seconds = int(timeVal % 60);
		sprintf(buf, "%02d:%02d:%02d", hours, minutes, seconds);
		return buf;
	}

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
	TsString FormatTimeInterval(time_t timeAtStart, int divisor /* = 1 */) 
	{
		time_t currentTime;
		time_t totalTime;
		
		time(&currentTime);
		if(divisor != 0) {
			totalTime = (currentTime - timeAtStart)/divisor;
			return FormatTimeElapsed(totalTime);
		}
		else {
			totalTime = (currentTime - timeAtStart)/1;
			return FormatTimeElapsed(totalTime) + " WARNING: divisor is 0";
			
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Tables for yroutines that are faster inline equivalents to the standard
	// toupper, tolower, isdigit, isalpha, isalnum.
	///////////////////////////////////////////////////////////////////////////
	unsigned char toUpperTable[256];
	unsigned char toLowerTable[256];
	bool isDigitTable[256];
	bool isAlphaTable[256];
	bool isAlnumTable[256];

	///////////////////////////////////////////////////////////////////////////
	// Object to initialize the tables
	///////////////////////////////////////////////////////////////////////////
	CharTableInitializer::CharTableInitializer() {
		for (int i = 0; i < 256; i++) {
			toUpperTable[i] = (unsigned char)(toupper(i));
			toLowerTable[i] = (unsigned char)(tolower(i));
			isDigitTable[i] = isdigit(i) != 0;
			isAlphaTable[i] = isalpha(i) != 0;
			isAlnumTable[i] = isalnum(i) != 0;
		}
	};

	static CharTableInitializer theCharTableInitializer;


} // namespace 

