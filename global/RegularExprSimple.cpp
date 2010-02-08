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

// RegularExprSimple.cpp: Simple textual regular expression object.

#include "Global_Headers.h"
#include "RegularExprSimple.h"
#include "RegularExprLexer.h"
#include "RegularExprParser.h"
#include "LexerInput.h"
#include "ListenerFIFO.h"

namespace PortfolioExplorer {
		
	///////////////////////////////////////////////////////////////////////////////
	// Bind the regular expression
	// Inputs:
	//	const TsString&		regularExprStr		String representation of the regular expr.
	//	ListenerRef			listener			Will receive any error messages.
	// Return value:
	//	bool			true is successfully bound, false o/w
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprSimple::Bind(
		const TsString& regularExprStr,
		ListenerRef listener
	) {
		//Initialize engine member variable
		engine = new RegularExprEngine;

		//Objects we need to analyze expressions
		ListenerFIFORef listenerFIFO = new ListenerFIFO;

		//Expression strings need quotes stripped
		LexerInputRef lexInput  = new LexerInput_string(regularExprStr);
		RegularExprLexerRef lexer = new RegularExprLexerChar(lexInput, listenerFIFO.get());
		RegularExprParserRef parser = new RegularExprParserChar(lexer, listenerFIFO.get());
		expr = parser->Parse();
		if (expr == 0) {
			// parse of regexp failed.
			while (listenerFIFO->HaveMessage()) {
				// Add prefix to message string and send to listener argument.
				ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
				listener->Error("Error parsing pattern " + msg.str);
			}
			return false;
		}
		
		engine->AddRegularExpression(expr);
		// Bind the ASCII character set; this is needed to make symbols recognized in expression language.
		for(int i = 0; i < 256; i++) {
			TsString thechar;
			thechar = char(i);
			engine->AddExpressionSymbol(thechar, listenerFIFO.get());
		}
		bool ret = engine->Bind(listenerFIFO.get());

		if (!ret) {
			// parse of regexp failed.
			while (listenerFIFO->HaveMessage()) {
				// Add prefix to message string and send to listener argument.
				ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
				listener->Error("Error binding patterns:  " + msg.str);
			}
			return false;
		}
		
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Match: Analyze this sequence of symbols against the NFA, returning true 
	// if it matched an expression, or false if not.  The entire string must match.
	// Also returns action codes.
	// Inputs:
	//	const TsString&			str				String to match
	// Outputs:
	//	vector<ActionRef>&		actionsMatched  Information on the subexpressions matched
	// Return value:
	//	bool		true if matched, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprSimple::Match(	
		const TsString& str,
		std::vector<ActionResult>& actionsMatched
	) {
		int exprMatched;
		int endPosition;
		actionsMatched.clear();
		return engine->Match(
			(unsigned const char*)str.c_str(), 
			exprMatched, 
			actionsMatched, 
			endPosition
		) &&
		endPosition == static_cast<int>(str.size());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Match: Analyze this sequence of symbols against the NFA, returning true 
	// if it matched an expression, or false if not.  The entire string must match.
	// No action codes.
	// Inputs:
	//	const TsString&		str			String to match
	// Return value:
	//	bool		true if matched, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprSimple::Match(	
		const TsString& str
	) {
		int exprMatched;
		int endPosition;
		actions.clear();
		return engine->Match(
			(unsigned const char*)str.c_str(), 
			exprMatched, 
			actions, 
			endPosition
		) &&
		endPosition == static_cast<int>(str.size());
	}
}

