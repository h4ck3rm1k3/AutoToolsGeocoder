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

// RegularExprParser.cpp:  Parser for regular expressions

#include "Global_Headers.h"

#include <vector>
#include <stdlib.h>
#include "RegularExprLiteral.h"
#include "RegularExprLiteralRange.h"
#include "RegularExprOneOrMore.h"
#include "RegularExprOptional.h"
#include "RegularExprOr.h"
#include "RegularExprSequence.h"
#include "RegularExprSet.h"
#include "RegularExprWildcard.h"
#include "RegularExprZeroOrMore.h"
#include "RegularExprAction.h"
#include "RegularExprCounted.h"

#include "RegularExprParser.h"

namespace PortfolioExplorer {
	///////////////////////////////////////////////////////////////////////////////
	// Constructor
	// Inputs:
	//	RegularExprLexerRef	lexer		The lexer supplying input tokens
	//	ListenerRef		    listener	The listener to which messages will be sent
	//	RecordCRef		    record		The upstream record from which fields are obtained.
	///////////////////////////////////////////////////////////////////////////////
	RegularExprParser::RegularExprParser(
		RegularExprLexerRef lexer_,
		ListenerRef listener_
	) :
		listener(listener_),
		lexer(lexer_)
	{
	}

	///////////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////////
	RegularExprParser::~RegularExprParser()
	{
	}

	///////////////////////////////////////////////////////////////////////////////
	// Clear the list of literal sets
	///////////////////////////////////////////////////////////////////////////////
	void RegularExprParser::ClearLiteralSets()
	{
		literalSets.clear();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Add a literal set.  Expansion of any named literal setswill
	// occur upon insertion.
	// Inputs:
	//	const TsString&				name		The name of the literal set to add
	//	const std::vector<TsString>&	literalSet	The set of equivalent literals
	// Return value:
	//	bool		true on success, false if the name already exists.
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprParser::AddLiteralSet(
		const TsString& name,
		const std::vector<TsString>& literalSet
	) {
		// Does this literal set already exist?
		if (literalSets.find(name) != literalSets.end()) {
			return false;
		}
		StringSetRef newLiteralSet = new StringSet;
		// Expand this token set based on previous literal sets before inserting the new definition.
		for (unsigned i = 0; i < literalSet.size(); i++) {
			TsString token = literalSet[i];
			LiteralSetReplacement::iterator iter = literalSets.find(token);
			if (iter != literalSets.end()) {
				// Insert the expanded token set.
				StringSetRef foundliteralSet = (*iter).second;
				for (
					StringSet::iterator iter2 = foundliteralSet->begin();
					iter2 != foundliteralSet->end();
					++iter2
				) {
					// Insert the token
					newLiteralSet->insert(*iter2);
				}
			} else {
				// Insert the token
				newLiteralSet->insert(token);
			}
		}
		literalSets.insert(LiteralSetReplacement::value_type(name, newLiteralSet));
		return true;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Parse: Parse the input and produce an expression tree
	// Inputs:
	//
	// Return value:
	//	ExpressionRef	The expression tree, or zero on failure.
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::Parse()
	{
		// Prime the input
		Consume();

		RegularExprRef retval;

		nextActionId = 0;

		try {
			retval = ParseOr();
			// Syntax error: garbage at end of otherwise-valid expression
			if( currentToken.type != RegularExprToken::Eof ) {
				listener->Error(
					currentToken.FormatPosition() + ": Invalid expression syntax"
				);
				retval = 0;
			}
		} catch (int) {
			// Syntax exception thrown. Error already reported.
		}
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Match: Match the given token type and consume it.  If the token type does not
	// match the one given, then a syntax error is reported and an integer
	// exception is thrown.
	// Inputs:
	//	ExprToken::Type	type		The type of token that must match.
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void RegularExprParser::Match(RegularExprToken::Type type) 
	{
		if (currentToken.type != type) {
			TsString errorMsg = currentToken.FormatPosition() + 
				": Expected " + RegularExprToken::GetTokenTypeDescription(type);
			listener->Error(errorMsg);
			// This will be caught by Parse()
			throw 1;
		}
		Consume();
	}

	///////////////////////////////////////////////////////////////////////////////
	// ParseOr: Parse a logical-OR expression
	// Inputs:
	//
	// Return value:
	//  RegularExprRef	the resulting expression
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::ParseOr()
	{
		std::vector<RegularExprRef> exprs;
		exprs.push_back(ParseSequence());
		while (currentToken.type == RegularExprToken::Or) {
			Consume();
			exprs.push_back(ParseSequence());
		}

		if( exprs.size() > 1 ) {
			return new RegularExprOr(exprs);
		} else {
			return exprs[0];
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// ParseSequence: Parse a sequence
	// Inputs:
	//
	// Return value:
	//  RegularExprRef	the resulting expression
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::ParseSequence()
	{
		RegularExprRef expr;
		std::vector<RegularExprRef> exprs;
		do {
			exprs.push_back(ParseTerm());
		}  while(
			currentToken.type != RegularExprToken::Eof && 
			currentToken.type != RegularExprToken::Or &&
			currentToken.type != RegularExprToken::RightParen
		); 
		expr = new RegularExprSequence(exprs);
		
		return expr;
	}

	///////////////////////////////////////////////////////////////////////////////
	// ParseTerm: Parse a term expression
	// Inputs:
	//
	// Return value:
	//  RegularExprRef		the resulting expression
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::ParseTerm()
	{
		bool haveAction = false;
		int actionId = 0;

		RegularExprToken token(currentToken);

		// Look for optional action: =expr for char parser
		// or LITERAL=expr for symbol parser
		if( dynamic_cast<RegularExprParserSymbol*>(this) != 0 ) {
			if ( Lookahead(1).type == RegularExprToken:: Equal ) {
				if( currentToken.type != RegularExprToken:: Literal &&
					currentToken.type != RegularExprToken::Integer
				) {
					listener->Error(
						token.FormatPosition() + ": Expected a literal or integer here"
					);
					throw 1;
				}
				// LITERAL=expr
				haveAction = true;
				actionId = nextActionId++;
				assert(actionId == actionMappings.size());
				actionMappings.push_back(currentToken.text);
				Consume();
				Match(RegularExprToken::Equal);
			}
		} else if( dynamic_cast<RegularExprParserChar*>(this) != 0 ) {
			if (currentToken.type == RegularExprToken::Equal) {
				// =expr
				haveAction = true;
				actionId = nextActionId++;
				Consume();
			}
		} else {
			//This shouldn't be possible!
			assert(false);
		}

		// Parse the lhs expression
		RegularExprRef expr = ParseSubterm();
		assert(expr != 0);

		//sub term _may_ be followed by a postfix operator
		//else, assume it's something legal and let someone 
		//else deal with it.
		switch (currentToken.type) {
		case RegularExprToken::Plus:
			expr = new RegularExprOneOrMore(expr);
			Consume();
			break;

		case RegularExprToken::Star:
			expr = new RegularExprZeroOrMore(expr);
			Consume(); 
			break;

		case RegularExprToken::Optional:
			expr = new RegularExprOptional(expr);
			Consume();
			break;

		case RegularExprToken::LeftBrace:
			expr = ParseCount(expr);
			break;
		}

		if (haveAction) {
			expr = new RegularExprAction(expr, actionId);
		}

		return expr;
	}

	///////////////////////////////////////////////////////////////////////////////
	// ParseSubterm: Parse a subterm expression
	// Inputs:
	//
	// Return value:
	//  RegularExprRef	the resulting expression
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::ParseSubterm()
	{
		RegularExprRef expr;

		RegularExprToken token = currentToken;
		Consume();
		
		bool negateSet = false;

		switch (token.type) {
		case RegularExprToken::Literal:
			expr = MakeLiteralOrSetExpression(token);
			break;

		case RegularExprToken::LeftParen:
			expr = ParseOr();
			Match(RegularExprToken::RightParen);
			break;
		
		case RegularExprToken::LeftBracket:
			expr = ParseSet();
			Match(RegularExprToken::RightBracket);
			break;

		case RegularExprToken::Wildcard:
			expr = new RegularExprWildcard();
			break;

		case RegularExprToken::String:
		{
			std::vector<RegularExprRef> exprs;
			RegularExprToken tmpToken(token);
			for (unsigned i = 0; i < token.text.size(); i++) {
				tmpToken.text = "";
				tmpToken.text += token.text[i];
				exprs.push_back(new RegularExprLiteral(tmpToken));
			}
			expr = new RegularExprSequence(exprs);
			break;
		}

		case RegularExprToken::NotDigit:
			negateSet = true;
			// fall through
		case RegularExprToken::Digit:
		{
			std::vector<RegularExprLiteralRef> arguments;
			// Synthesize some tokens for the range
			RegularExprToken token1(token);
			RegularExprToken token2(token);
			token1.type = RegularExprToken::Literal;
			token1.text = "0";
			token2.type = RegularExprToken::Literal;
			token2.text = "9";
			arguments.push_back(new RegularExprLiteralRange(token1, token2));
			expr =  new RegularExprSet(negateSet, arguments);
			break;
		}

		case RegularExprToken::NotAlpha:
			negateSet = true;
			// fall through
		case RegularExprToken::Alpha:
		{
			std::vector<RegularExprLiteralRef> arguments;
			// Synthesize some tokens for the range
			RegularExprToken token1(token);
			RegularExprToken token2(token);
			token1.type = RegularExprToken::Literal;
			token2.type = RegularExprToken::Literal;
			token1.text = "a";
			token2.text = "z";
			arguments.push_back(new RegularExprLiteralRange(token1, token2));
			token1.text = "A";
			token2.text = "Z";
			arguments.push_back(new RegularExprLiteralRange(token1, token2));
			expr =  new RegularExprSet(negateSet, arguments);
			break;
		}

		case RegularExprToken::NotWhitespace:
			negateSet = true;
			// fall through
		case RegularExprToken::Whitespace:
		{
			std::vector<RegularExprLiteralRef> arguments;
			// Synthesize some tokens for the range
			RegularExprToken tmpToken(token);
			token.type = RegularExprToken::Literal;
			token.text = " ";
			arguments.push_back(new RegularExprLiteral(tmpToken));
			token.text = "\t";
			arguments.push_back(new RegularExprLiteral(tmpToken));
			token.text = "\n";
			arguments.push_back(new RegularExprLiteral(tmpToken));
			token.text = "\r";
			arguments.push_back(new RegularExprLiteral(tmpToken));
			token.text = "\f";
			arguments.push_back(new RegularExprLiteral(tmpToken));
			token.text = "\v";
			arguments.push_back(new RegularExprLiteral(tmpToken));
			expr =  new RegularExprSet(negateSet, arguments);
			break;
		}

		case RegularExprToken::NotAlphaNumeric:
			negateSet = true;
			// fall through
		case RegularExprToken::AlphaNumeric:
		{
			std::vector<RegularExprLiteralRef> arguments;
			// Synthesize some tokens for the range
			RegularExprToken token1(token);
			RegularExprToken token2(token);
			token1.type = RegularExprToken::Literal;
			token2.type = RegularExprToken::Literal;
			token1.text = "a";
			token2.text = "z";
			arguments.push_back(new RegularExprLiteralRange(token1, token2));
			token1.text = "A";
			token2.text = "Z";
			arguments.push_back(new RegularExprLiteralRange(token1, token2));
			token1.text = "0";
			token2.text = "9";
			arguments.push_back(new RegularExprLiteralRange(token1, token2));
			expr =  new RegularExprSet(negateSet, arguments);
			break;
		}

		case RegularExprToken::Error:
			// Malformed token
			listener->Error(
				token.FormatPosition() + ": Unrecognizable expression term"
			);
			throw 1;
		case RegularExprToken::Eof:
			// End of input
			listener->Error(
				token.FormatPosition() + ": Unexpected end of expression"
			);
			throw 1;

		default:
			// End of input
			listener->Error(
				token.FormatPosition() + ": Expected an expression here"
			);
			throw 1;
		}

		return expr;
	}

	///////////////////////////////////////////////////////////////////////////////
	// ParseSet: Parse a set expression: [^? set_item+]
	// Inputs:
	//
	// Return value:
	//  RegularExprRef	the resulting expression
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::ParseSet()
	{
		std::vector<RegularExprLiteralRef> arguments;
		bool negate = false;

		//The set must contain at least one literal
		if( currentToken.type ==  RegularExprToken::Caret)
		{
			Consume();
			negate = true;
		}
		
		//We must have at least one literal in the set!
		if( currentToken.type != RegularExprToken::Literal ) {
			//illegal!
			listener->Error(
			currentToken.FormatPosition() + ": Expected a literal here"
			);
			throw 1;
		}

		RegularExprLiteralRef argument;
		RegularExprToken token;
		do {
			
			token = currentToken;
			Consume();
			if( currentToken.type == RegularExprToken::Dash ) {
				//We've got a range
				Consume();
				//Dash must be followed by a literal
				if( currentToken.type != RegularExprToken::Literal ) {
					//illegal!
					listener->Error(
						currentToken.FormatPosition() + ": Expected a literal here"
					);
					throw 1;
				}
				argument = new RegularExprLiteralRange(token, currentToken);
				//Move to the next token
				Consume();
			} else {
				argument = new RegularExprLiteral(token);
			}
			arguments.push_back(argument);
		} while( currentToken.type == RegularExprToken::Literal );
		
		return new RegularExprSet(negate, arguments);
	}

	///////////////////////////////////////////////////////////////////////////////
	// ParseCount: Parse a count expression like expr{n} or expr{m,n}
	// Inputs:
	//	RegularExprRef	expr	The operand expresion
	//
	// Return value:
	//  RegularExprRef	the resulting expression
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::ParseCount(
		RegularExprRef	expr
	) {
		int minCount;
		int maxCount;

		Match(RegularExprToken::LeftBrace);

		if (currentToken.type != RegularExprToken::Integer) {
			listener->Error(
				currentToken.FormatPosition() + ": Expected a number here"
			);
			throw 1;
		}

		minCount = atoi(currentToken.text.c_str());
		if (minCount < 0 || minCount > MaxAllowedCount) {
			listener->Error(
				currentToken.FormatPosition() + ": Min count must be >= 0 and <= " + FormatInteger(MaxAllowedCount)
			);
			throw 1;
		}
		Consume();

		if (currentToken.type == RegularExprToken::Comma) {
			// Of the form {min,max}
			Consume();
			if (currentToken.type != RegularExprToken::Integer) {
				listener->Error(
					currentToken.FormatPosition() + ": Expected a number here"
				);
				throw 1;
			}

			maxCount = atoi(currentToken.text.c_str());
			if (maxCount <= 0 || maxCount > MaxAllowedCount) {
				listener->Error(
					currentToken.FormatPosition() + ": Max count must be > 0 and <= " + FormatInteger(MaxAllowedCount)
				);
				throw 1;
			}

			if (maxCount < minCount) {
				listener->Error(
					currentToken.FormatPosition() + ": Max count must be >= Min count"
				);
				throw 1;
			}

			Consume();
		} else {
			// Of the form {count}
			maxCount = minCount;
		}

		Match(RegularExprToken::RightBrace);

		return new RegularExprCounted(expr, minCount, maxCount);
	}

	///////////////////////////////////////////////////////////////////////////////
	// If the given token is a literal set, than make a set expression for the name,
	// otherwise make a literal expression.
	///////////////////////////////////////////////////////////////////////////////
	RegularExprRef RegularExprParser::MakeLiteralOrSetExpression(
		const RegularExprToken& token
	) {
		LiteralSetReplacement::iterator iter = literalSets.find(token.text);
		if (iter != literalSets.end()) {
			// Expand name into literal set.
			std::vector<RegularExprLiteralRef> arguments;
			for (
				StringSet::iterator iter2 = (*iter).second->begin();
				iter2 != (*iter).second->end();
				++iter2
			) {
				RegularExprToken newToken(token);
				newToken.type = RegularExprToken::Literal;
				newToken.text = (*iter2);
				arguments.push_back(new RegularExprLiteral(newToken));
			}
			return new RegularExprSet(false, arguments);
		} else {
			return new RegularExprLiteral(token);
		}
	}

} // namespace
