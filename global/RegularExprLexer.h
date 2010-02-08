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

// RegularExprLexer.h: Simple lexer for query statement

#ifndef INCL_RegularExprLexer_H
#define INCL_RegularExprLexer_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "TsString.h"
#include "Listener.h"
#include "LexerInput.h"
#include "Utility.h"
#include "RegularExprToken.h"
#include "Global_DllExport.h"

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

namespace PortfolioExplorer {

	////////////////////////////////////////////////////////////////////////////////
	// Lexer to lex simple token types.
	////////////////////////////////////////////////////////////////////////////////
	class RegularExprLexer : public VRefCount {
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor for LexerBase base class.
		// Inputs:
		//	LexerInputRef		input		The source of characters.
		//	ListenerRef			listener	Pointer to the object that will receive any
		//									error messages via its Error() method.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprLexer(
			LexerInputRef input_,
			ListenerRef listener_
		)  :
			listener(listener_),
			line(1),
			column(1),
			current(-1),
			input(input_)
		{
			// Prime the input
			NextChar();
		}

		////////////////////////////////////////////////////////////////////////////////
		// Destructor.  This is virtual in case of deletion via pointer-to-base.
		////////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprLexer() {}

		////////////////////////////////////////////////////////////////////////////////
		// NextToken:  Scan the input and construct the next token.
		// Return Value:
		//	ExprToken		The next token.  If end-of-input is reached, the type of
		//					the token type will be Token::Eof.  If a lexical error
		//					is ecountered, then the token type will be Token::Error.
		////////////////////////////////////////////////////////////////////////////////
		virtual RegularExprToken NextToken();

	protected:

		////////////////////////////////////////////////////////////////////////////////
		// Data members
		////////////////////////////////////////////////////////////////////////////////
		ListenerRef listener;		// sink for messages
		int line;					// current input line
		int column;					// current input column
		char current;				// current input character
		TsString tokenText;		// accumulated text for the token

		////////////////////////////////////////////////////////////////////////////////
		// Eof: Signalled when the current character is zero.
		// Inputs:
		//
		// Return value:
		//  bool		true if eof, else false
		////////////////////////////////////////////////////////////////////////////////
		bool Eof() { return current <= 0; }

		////////////////////////////////////////////////////////////////////////////////
		// Skip: Skip the current input character and get the
		// next input character.
		// Inputs:
		//
		// Return value;
		////////////////////////////////////////////////////////////////////////////////
		void Skip() 
		{
			if (!Eof()) {
				if (current == '\n') {
					line++;
					column = 0;
				} else {
					column++;
				}
				NextChar();
			}
		}


		////////////////////////////////////////////////////////////////////////////////
		// Consume:  Add the current input character to the tokenText and get the
		// next input character.
		// Inputs:
		//
		// Return value:
		////////////////////////////////////////////////////////////////////////////////
		void Consume() 
		{
			if (!Eof()) {
				if (current == '\n') {
					line++;
					column = 0;
				} else {
					column++;
				}
				tokenText += current;
				NextChar();
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// NextChar:  Get the next character of input and put it in the 'current' member.
		// Inputs:
		//
		// Return value:
		////////////////////////////////////////////////////////////////////////////////
		void NextChar() {
			current = input->NextChar();
		}

		////////////////////////////////////////////////////////////////////////////////
		// LexNumber:  Lex a number
		// Return value:
		//	RegularExprToken::Type		The type of the matched token.
		//
		// Note: May throw an integer to indicate error.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprToken::Type LexNumber();

		////////////////////////////////////////////////////////////////////////////////
		// LexOther:  Lex the token types known to subclass (must be defined by subclass)
		// Return value:
		//	RegularExprToken::Type		The type of the matched token.
		//
		// Note: May throw an integer to indicate error.
		////////////////////////////////////////////////////////////////////////////////
		virtual RegularExprToken::Type LexOther() = 0;

		// Text input source;
		LexerInputRef input;
	};
	
	typedef refcnt_ptr<RegularExprLexer> RegularExprLexerRef;

	////////////////////////////////////////////////////////////////////////////////
	// Lexer to lex char token types.
	////////////////////////////////////////////////////////////////////////////////
	class RegularExprLexerChar : public RegularExprLexer {
	
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor for LexerBase base class.
		// Inputs:
		//	LexerInputRef		input		The source of characters.
		//	ListenerRef			listener	Pointer to the object that will receive any
		//									error messages via its Error() method.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprLexerChar(
			LexerInputRef input_,
			ListenerRef listener_
		)  : RegularExprLexer (
				input_,
				listener_
			)
		{ }

		////////////////////////////////////////////////////////////////////////////////
		// Destructor.  This is virtual in case of deletion via pointer-to-base.
		////////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprLexerChar() {}

	private:
		////////////////////////////////////////////////////////////////////////////////
		// LexOther:  Lex the token types known to subclass (must be defined by subclass)
		// Return value:
		//	RegularExprToken::Type		The type of the matched token.
		//
		// Note: May throw an integer to indicate error.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprToken::Type LexOther();

		////////////////////////////////////////////////////////////////////////////////
		// LexString:  Lex a quoted string
		// Return value:
		//	RegularExprToken::Type		The type of the matched token.
		//
		// Note: May throw an integer to indicate error.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprToken::Type LexString();

	};

	////////////////////////////////////////////////////////////////////////////////
	// Lexer to lex symbol token types.
	////////////////////////////////////////////////////////////////////////////////
	class RegularExprLexerSymbol : public RegularExprLexer {
	
	public:
		////////////////////////////////////////////////////////////////////////////////
		// Constructor for LexerBase base class.
		// Inputs:
		//	LexerInputRef		input		The source of characters.
		//	ListenerRef			listener	Pointer to the object that will receive any
		//									error messages via its Error() method.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprLexerSymbol(
			LexerInputRef input_,
			ListenerRef listener_
		)  : RegularExprLexer (
				input_,
				listener_
			)
		{ }

		////////////////////////////////////////////////////////////////////////////////
		// Destructor.  This is virtual in case of deletion via pointer-to-base.
		////////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprLexerSymbol() {}

	private:
		////////////////////////////////////////////////////////////////////////////////
		// LexOther:  Lex the token types known to subclass (must be defined by subclass)
		// Return value:
		//	RegularExprToken::Type		The type of the matched token.
		//
		// Note: May throw an integer to indicate error.
		////////////////////////////////////////////////////////////////////////////////
		RegularExprToken::Type LexOther();
	};

} // namespace

#endif
