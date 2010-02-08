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
# $Rev: 52 $ 
# $Date: 2006-10-06 05:33:29 +0200 (Fri, 06 Oct 2006) $ 
*/

// RegularExprLexer.cpp: Simple lexer for regular expressions

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "Global_Headers.h"
#include "RegularExprLexer.h"
#include "Utility.h"
#include <ctype.h>

namespace PortfolioExplorer {

	////////////////////////////////////////////////////////////////////////////////
	// NextToken: Scan the input and construct the next token.
	// Inputs:
	//
	// Return Value:
	//	ExprToken		The next token.  Type will be Token::Eof on end-of-file,
	//					or Token::Error if a syntax error was encountered.
	////////////////////////////////////////////////////////////////////////////////
	RegularExprToken RegularExprLexer::NextToken()
	{
		// Skip whitespace 
		while (isspace(current)) {
			Skip();
		}

		// Remember the starting position
		int startLine = line;
		int startColumn = column;

		// Clear out the token
		tokenText.erase();

		RegularExprToken::Type type = RegularExprToken::Error;

		try {
			switch (current) {
			case 0:
				type = RegularExprToken::Eof;
				break;
			case '-':
				type = RegularExprToken::Dash;
				Consume();
				break;
			case '.':
				type = RegularExprToken::Wildcard;
				Consume();
				break;
			case '[':
				type = RegularExprToken::LeftBracket;
				Consume();
				break;
			case ']':
				type = RegularExprToken::RightBracket;
				Consume();
				break;
			case '^':
				type = RegularExprToken::Caret;
				Consume();
				break;
			case '(':
				type = RegularExprToken::LeftParen;
				Consume();
				break;
			case ')':
				type = RegularExprToken::RightParen;
				Consume();
				break;
			case '?':
				type = RegularExprToken::Optional;
				Consume();
				break;
			case '|':
				type = RegularExprToken::Or;
				Consume();
				break;
			case '*':
				type = RegularExprToken::Star;
				Consume();
				break;
			case '+':
				type = RegularExprToken::Plus;
				Consume();
				break;
			case '{':
				type = RegularExprToken::LeftBrace;
				Consume();
				break;
			case '}':
				type = RegularExprToken::RightBrace;
				Consume();
				break;
			case ',':
				type = RegularExprToken::Comma;
				Consume();
				break;
			case '=':
				type = RegularExprToken::Equal;
				Consume();
				break;
			default:
				if (ISDIGIT(current)) {
					type = LexNumber();
				} else {
					type = LexOther();
				}
				break;
			}
		} catch (int) {
			// Don't get stuck on an error token
			NextChar();
		}
		return RegularExprToken(type, tokenText, startLine, startColumn);
	}

	////////////////////////////////////////////////////////////////////////////////
	// LexNumber:  Lex a number
	// Return value:
	//	RegularExprToken::Type		The type of the matched token.
	//
	// Note: May throw an integer to indicate error.
	////////////////////////////////////////////////////////////////////////////////
	RegularExprToken::Type RegularExprLexer::LexNumber()
	{
		if (!ISDIGIT(current)) {
			throw 1;
		}
		
		while (ISDIGIT(current)) {
			Consume();
		}
		return RegularExprToken::Integer;
	}

	////////////////////////////////////////////////////////////////////////////////
	// LexOther:  Lex the token types known to subclass (must be defined by subclass)
	// Return value:
	//	RegularExprToken::Type		The type of the matched token.
	//
	// Note: May throw an integer to indicate error.
	////////////////////////////////////////////////////////////////////////////////
	RegularExprToken::Type RegularExprLexerChar::LexOther()
	{
		switch (current) {
		case 'd':
			Consume();
			return RegularExprToken::Digit;
		case 'D':
			Consume();
			return RegularExprToken::NotDigit;
		case 'a':
			Consume();
			return RegularExprToken::Alpha;
		case 'A':
			Consume();
			return RegularExprToken::NotAlpha;
		case 's':
			Consume();
			return RegularExprToken::Whitespace;
		case 'S':
			Consume();
			return RegularExprToken::NotWhitespace;
		case 'w':
			Consume();
			return RegularExprToken::AlphaNumeric;
		case 'W':
			Consume();
			return RegularExprToken::NotAlphaNumeric;
		case '"':
			return LexString();
		default:
			// fall through
			break;
		}

		if (current != '\'') {
			// Will be caught by NextToken()
			throw 1;
		}
		// Skip opening ' 
		Skip();

		//The next character can't be a '.  They need to escape it with a '\' if that's
		//what they intend
		if (current == '\'') {
			throw 1;
		}
		//Check for the escaped single quote and backslash here
		if (current == '\\') {
			//Skip the backslash
			Skip();
			if (current != '\'' && current != '\\') {
				//whoops, it wasn't an escaped character after all.  Put the backslash back in!
				tokenText += '\\';
			} else {
				//It is an escaped character.  Consume it here so the following code doesn't end
				//the token with no text
				Consume();
			}
		} else {
			//Consume our one character
			Consume();
		}

		// Must end with a '
		if (current != '\'') {
			throw 1;
		}
		
		// Skip closing '
		Skip();
		
		return RegularExprToken::Literal;
	}

	////////////////////////////////////////////////////////////////////////////////
	// LexString:  Lex a quoted string
	// Return value:
	//	RegularExprToken::Type		The type of the matched token.
	//
	// Note: May throw an integer to indicate error.
	////////////////////////////////////////////////////////////////////////////////
	RegularExprToken::Type RegularExprLexerChar::LexString()
	{
		if (current != '"') {
			// Will be caught by NextToken()
			throw 1;
		}
		// Skip opening " 
		Skip();

		//The next character can't be a ".  They need to escape it with a '\' if that's
		//what they intend
		if (current == '"') {
			throw 1;
		}

		while (current != '"' && current > 0) {
			// Check for the escaped quote and backslash here
			if (current == '\\') {
				// Skip the backslash
				Skip();
				if (current != '"' && current != '\\') {
					// whoops, it wasn't an escaped character after all.  Put the backslash back in!
					tokenText += '\\';
				} else {
					// It is an escaped character.  Consume it here so the following code doesn't end
					// the token with no text
					Consume();
				}
			} else {
				// Consume our one character
				Consume();
			}
		}

		// Must end with a '
		if (current != '"') {
			throw 1;
		}
		
		// Skip closing "
		Skip();
		
		return RegularExprToken::String;
	}

	////////////////////////////////////////////////////////////////////////////////
	// LexOther:  Lex the token types known to subclass (must be defined by subclass)
	// Return value:
	//	RegularExprToken::Type		The type of the matched token.
	//
	// Note: May throw an integer to indicate error.
	////////////////////////////////////////////////////////////////////////////////
	RegularExprToken::Type RegularExprLexerSymbol::LexOther()
	{
		//the symbol strings are separated by spaces
		if (!ISALNUM(current)) {
			throw 1;
		}
		while (ISALNUM(current) || current == '_') {
			Consume();
		}

		return RegularExprToken::Literal;
	}


	///////////////////////////////////////////////////////////////////////////
	// Return a string describing a token type.
	// Inputs:
	//
	// Return Value:
	//	TsString		token description
	////////////////////////////////////////////////////////////////////////////////
	//static 
	TsString RegularExprToken::GetTokenTypeDescription(Type type)
	{
		switch (type) {
		case Eof: return "End of expression";
		case Error: return "Error";
		case Literal: return "a literal value name";
		case LeftParen: return "'('";
		case RightParen: return "')'";
		case LeftBracket: return "'['";
		case RightBracket: return "']'";
		case Plus: return "'+'";
		case Star: return "'*'";
		case Optional: return "'?'";
		case Wildcard: return "'.'";
		case Dash: return "'-'";
		case Caret: return "'^'";
		case Or: return "|";
		case String: return "a string";
		case Digit: return "d";
		case NotDigit: return "D";
		case Whitespace: return "s";
		case NotWhitespace: return "S";
		case AlphaNumeric: return "w";
		case NotAlphaNumeric: return "W";
		case LeftBrace: return "{";
		case RightBrace: return "}";
		case Comma: return ",";
		case Integer: return "an integer";
		default: return "Unknown";
		}
	}

} //namespace
