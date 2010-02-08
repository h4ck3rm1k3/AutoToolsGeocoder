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

// LexerInput: class for supplying characters to lexers

#ifndef INCL_LEXERINPUT_H
#define INCL_LEXERINPUT_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <stdio.h>
#include "TsString.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	////////////////////////////////////////////////////////////////////////////////
	// Abstract base class for supplying characters to lexers
	////////////////////////////////////////////////////////////////////////////////
	class  LexerInput : public VRefCount {
	public:
		////////////////////////////////////////////////////////////////////////////////
		// NextChar:  Get the next character of input.
		// Return Value:
		//	char	The next character.  Will always return zero after EOF.
		////////////////////////////////////////////////////////////////////////////////
		virtual char NextChar() = 0;

		////////////////////////////////////////////////////////////////////////////////
		// Have we reached the end of the input?
		// Return Value:
		//	bool		true after Eof has been reached.
		////////////////////////////////////////////////////////////////////////////////
		virtual bool Eof() const = 0;
	};
	typedef refcnt_ptr<LexerInput> LexerInputRef;


	////////////////////////////////////////////////////////////////////////////////
	// Lexer input from a FILE
	////////////////////////////////////////////////////////////////////////////////
	class LexerInput_FILE : public LexerInput {
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor for Lexer_FILE
		// Inputs:
		//	FILE*		fp			The FILE from which input is read.
		// Note: This is not closed by this object.
		////////////////////////////////////////////////////////////////////////////////
		LexerInput_FILE(FILE* fp_) : fp(fp_), eof(false)
		{
		}
		////////////////////////////////////////////////////////////////////////////////
		// NextChar:  Get the next character of input.
		// Return Value:
		//	char	The next character.  Will always return zero after EOF.
		////////////////////////////////////////////////////////////////////////////////
		virtual char NextChar() 
		{
			// We cannot rely on stdio to return eof repeatedly when reading
			// from the console.
			int c;
			if (eof || (c = getc(fp)) <= 0) {
				// Note that reading a zero-byte will set EOF indicator.
				eof = true;
				return 0;
			} else {
				return (char)c;
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// Have we reached the end of the input?
		// Return Value:
		//	bool		true after Eof has been reached.
		////////////////////////////////////////////////////////////////////////////////
		virtual bool Eof() const { return eof; }

	private:
		FILE* fp;
		bool eof;
	};


	////////////////////////////////////////////////////////////////////////////////
	// Lexer input from a string
	////////////////////////////////////////////////////////////////////////////////
	class LexerInput_string : public LexerInput {
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor for Lexer_FILE
		// Inputs:
		//	FILE*		fp			The FILE from which input is read.
		// Note: This is not closed by this object.
		////////////////////////////////////////////////////////////////////////////////
		LexerInput_string(
			const TsString& str_
		) : 
			str(str_), idx(0)
		{
		}

		////////////////////////////////////////////////////////////////////////////////
		// NextChar:  Get the next character of input.
		// Return Value:
		//	char	The next character.  Will always return zero after EOF.
		////////////////////////////////////////////////////////////////////////////////
		virtual char NextChar() 
		{
			if (idx < int(str.size())) {
				return str[idx++];
			} else {
				return 0;
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// Have we reached the end of the input?
		// Return Value:
		//	bool		true after Eof has been reached.
		////////////////////////////////////////////////////////////////////////////////
		virtual bool Eof() const { return idx >= int(str.size()); }

	private:
		TsString str;
		int idx;
	};

}

#endif
