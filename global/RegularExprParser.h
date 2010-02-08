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

// RegularExprParser.h:  Parser base class used by all objects that parse query expressions

#ifndef INCL_REGULAREXPRPARSER_H
#define INCL_REGULAREXPRPARSER_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "Global_DllExport.h"
#include "RegularExprLexer.h"
#include "Listener.h"
#include "RegularExpr.h"
#include "StringSet.h"
#include "StringTorefMap.h"

#include <deque>


namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// This class will parse a textual Config specification, and create a
	// DataItem tree that represents the Config file.
	///////////////////////////////////////////////////////////////////////////////
	class RegularExprParser : public VRefCount {
	protected:
		//Protected constructor because we want to force creation of one of the derived 
		//parsers.

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	RegularExprLexerRef	lexer		The lexer supplying input tokens
		//	ListenerRef		    listener	The listener to which messages will be sent
		///////////////////////////////////////////////////////////////////////////////
		RegularExprParser(
			RegularExprLexerRef lexer,
			ListenerRef listener
		);

	public:
		///////////////////////////////////////////////////////////////////////////////
		// Destructor.  This is virtual to handle deletion of pointer-to-base.
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprParser();

		///////////////////////////////////////////////////////////////////////////////
		// Manage the list of literal sets
		///////////////////////////////////////////////////////////////////////////////
		void ClearLiteralSets();

		///////////////////////////////////////////////////////////////////////////////
		// Add a literal set.  Expansion of any set names will occur upon insertion.
		// Inputs:
		//	const TsString&				name		The name of the literal set to add
		//	const std::vector<TsString>&	literalSet	The set of equivalent literals
		// Return value:
		//	bool		true on success, false if the name already exists.
		///////////////////////////////////////////////////////////////////////////////
		bool AddLiteralSet(
			const TsString& name,
			const std::vector<TsString>& literalSet
		);

		///////////////////////////////////////////////////////////////////////////////
		// Parse: Parse the input and produce an expression tree
		// Inputs:
		//
		// Return value:
		//	RegularExprRef	The expression tree, or zero on failure.
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef Parse();

		///////////////////////////////////////////////////////////////////////////////
		// Eof: Are we at the end-of-input? This can be used by callers to verify that 
		// all input was consumed by this parser.
		// Inputs:
		//
		// Return value:
		//	bool		true if EOF was reached, false o/w
		///////////////////////////////////////////////////////////////////////////////
		bool Eof() { return currentToken.type == RegularExprToken::Eof; }	

		///////////////////////////////////////////////////////////////////////////////
		// Get the current token.  This is useful to find the point at which
		// parsing stopped.
		// Inputs:
		//
		// Return value:
		//	const RegularExprToken&	The last token
		///////////////////////////////////////////////////////////////////////////////
		const RegularExprToken& GetCurrentToken() const { return currentToken; }

		///////////////////////////////////////////////////////////////////////////////
		// Get the vector of action->string mappings.
		// Inputs:
		//
		// Return value:
		//	std::vector<TsString> ActionMappings	A vector indexed by action numbers
		///////////////////////////////////////////////////////////////////////////////
		std::vector<TsString> GetActionMappings() { return actionMappings; }
	
	private:
		///////////////////////////////////////////////////////////////////////////////
		// Match: Match the given token type and consume it.  If the token type does not
		// match the one given, then a syntax error is reported and an integer
		// exception is thrown.
		// Inputs:
		//	ExprToken::Type	type		The type of token that must match.
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Match(RegularExprToken::Type type);

		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////
		// Consume: Consume the current token and get the next one.
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Consume() 
		{
			if(lookaheadQueue.size() > 0) {
				lookaheadQueue.pop_front();
			}
			currentToken = Lookahead(0);
		}

		///////////////////////////////////////////////////////////////////////////////
		// ParseOr: Parse a logical-OR expression
		// Inputs:
		//
		// Return value:
		//  RegularExprRef	the resulting expression
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef ParseOr();

		///////////////////////////////////////////////////////////////////////////////
		// ParseSequence: Parse a sequence
		// Inputs:
		//
		// Return value:
		//  RegularExprRef	the resulting expression
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef ParseSequence();

		///////////////////////////////////////////////////////////////////////////////
		// ParseTerm: Parse a term expression
		// Inputs:
		//
		// Return value:
		//  RegularExprRef		the resulting expression
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef ParseTerm();

		///////////////////////////////////////////////////////////////////////////////
		// ParseSubterm: Parse a subterm expression
		// Inputs:
		//
		// Return value:
		//  RegularExprRef	the resulting expression
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef ParseSubterm();

		///////////////////////////////////////////////////////////////////////////////
		// ParseSet: Parse a set expression: [^? set_item+]
		// Inputs:
		//
		// Return value:
		//  RegularExprRef	the resulting expression
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef ParseSet();

		///////////////////////////////////////////////////////////////////////////////
		// ParseCount: Parse a count expression like expr{n} or expr{m,n}
		// Inputs:
		//	RegularExprRef	expr	The operand expresion
		//
		// Return value:
		//  RegularExprRef	the resulting expression
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef ParseCount(RegularExprRef expr);

		///////////////////////////////////////////////////////////////////////////////
		// If the given token is a named symbol set, than make a set expression for 
		// it, otherwise make a literal expression.
		///////////////////////////////////////////////////////////////////////////////
		RegularExprRef MakeLiteralOrSetExpression(
			const RegularExprToken& token
		);

		///////////////////////////////////////////////////////////////////////////////
		// Return the token at the 
		///////////////////////////////////////////////////////////////////////////////
		inline RegularExprToken Lookahead(size_t i)
		{
			//if the queue doesn't contain enough tokens, we need to "look ahead" 
			//far enough to get that token
			RegularExprToken token;
			while(lookaheadQueue.size() <= i) {
				token = lexer->NextToken();
				if (token.type == RegularExprToken::Eof) {
					return token;
				}
				lookaheadQueue.push_back(token);
			}
			return lookaheadQueue[i];
		}
		
		// Will receive any errors from the parse
		ListenerRef listener;

		// The lexer input
		RegularExprLexerRef lexer;

		// The current lookahead token
		RegularExprToken currentToken;

		// Tracks next action Id;
		int nextActionId;

		// Since we unroll expression to NFA, limit count to avoid explosion.
		enum { MaxAllowedCount = 10 };

		//vector containing string mappings to token names
		std::vector<TsString> actionMappings;

		//look ahead deque
		std::deque<RegularExprToken> lookaheadQueue;

		// Token sets support creation of "literal sets" for token classes.
		// This is not a general replacement mechanism, just a token-set replacement.
		typedef StringToRefMap<StringSetRef> LiteralSetReplacement;
		LiteralSetReplacement literalSets;

	};
	typedef refcnt_ptr<RegularExprParser> RegularExprParserRef;

	///////////////////////////////////////////////////////////////////////////////
	// This class will parse a textual Config specification, and create a
	// DataItem tree that represents the Config file.
	///////////////////////////////////////////////////////////////////////////////
	class RegularExprParserChar : public RegularExprParser {
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	RegularExprLexerRef	lexer		The lexer supplying input tokens
		//	ListenerRef		    listener	The listener to which messages will be sent
		///////////////////////////////////////////////////////////////////////////////
		RegularExprParserChar(
			RegularExprLexerRef lexer,
			ListenerRef listener
			) : RegularExprParser(lexer, listener)
		{}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor.  This is virtual to handle deletion of pointer-to-base.
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprParserChar()
		{}
	};
	///////////////////////////////////////////////////////////////////////////////
	// This class will parse a textual Config specification, and create a
	// DataItem tree that represents the Config file.
	///////////////////////////////////////////////////////////////////////////////
	class RegularExprParserSymbol : public RegularExprParser {
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	RegularExprLexerRef	lexer		The lexer supplying input tokens
		//	ListenerRef		    listener	The listener to which messages will be sent
		///////////////////////////////////////////////////////////////////////////////
		RegularExprParserSymbol(
			RegularExprLexerRef lexer,
			ListenerRef listener
			) : RegularExprParser(lexer, listener)
		{}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor.  This is virtual to handle deletion of pointer-to-base.
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprParserSymbol()
		{}
	};
}	// namespace

#endif
