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
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

// RegularExprToken.h: Token struct for query parsing

#ifndef INCL_REGULAREXPRTOKEN_H
#define INCL_REGULAREXPRTOKEN_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "TsString.h"
#include "Utility.h"
#include "Global_DllExport.h"

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

namespace PortfolioExplorer {

	////////////////////////////////////////////////////////////////////////////////
	// ExprToken class.  Objects of this type will hold token information.  A token
	// consists of a type, and some text.
	////////////////////////////////////////////////////////////////////////////////
	struct RegularExprToken {
		////////////////////////////////////////////////////////////////////////////////
		// Enumeration of possible token types.  See the lexer source for details of
		// how input patterns map onto token types.
		////////////////////////////////////////////////////////////////////////////////
		enum Type { 
			Eof, 
			Error, 
			Literal,		// e.g. 'x' for text matching.
							// e.g. WORD for symbol matching
			LeftParen,		// (
			RightParen,		// )
			LeftBracket,	// ]
			RightBracket,	// [
			Plus,			// +
			Star,			// *
			Optional,		// ?
			Wildcard,		// .
			Dash,			// -
			Caret,			// ^
			Or,				// |
			// The following are only for text matching
			String,			// e.g. "John"
			Digit,			// d
			NotDigit,		// D
			Whitespace,		// s
			NotWhitespace,	// S
			Alpha,			// a
			NotAlpha,		// A
			AlphaNumeric,	// w
			NotAlphaNumeric,// W
			LeftBrace,		// {
			RightBrace,		// }
			Comma,			// ,
			Integer,		// e.g. 12345
			Equal			// =
		};

		////////////////////////////////////////////////////////////////////////////////
		// Default constructor needed for containers.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprToken() : type(Error), column(0), line(0) {}

		////////////////////////////////////////////////////////////////////////////////
		// Constructor from actual values.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprToken(Type type_, TsString text_, int line_, int column_) : 
			type(type_), column(column_), line(line_), text(text_)
		{}

		///////////////////////////////////////////////////////////////////////////
		// FormatPosition: Format the position of the token as a string
		// like "(line,column,size)"
		// Inputs:
		//
		// Return value:
		//	TsString	The formatted position.
		///////////////////////////////////////////////////////////////////////////
		TsString FormatPosition() const {
			return "(" + FormatInteger(line) + "," + FormatInteger(column) + "," + FormatInteger(unsigned(text.size())) + ")";
		}

		///////////////////////////////////////////////////////////////////////////
		// GetTokenDescription: Return a string describing a token type.
		// Inputs:
		//  Type		The token type to retrieve a description for
		//
		// Return value:
		//  TsString	the token description
		///////////////////////////////////////////////////////////////////////////
		static TsString GetTokenTypeDescription(Type type);

		////////////////////////////////////////////////////////////////////////////////
		// All data members are public
		////////////////////////////////////////////////////////////////////////////////
		Type type;
		int column;
		int line;
		TsString text;
	};



} // namespace

#endif
