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

// RegularExprSymbolizer.cpp: Defines the regular expression symbolizer object

#include "Global_Headers.h"
#include "RegularExprSymbolizer.h"
#include "RegularExpr.h"
#include "RegularExprLexer.h"
#include "RegularExprParser.h"
#include "LexerInput.h"

namespace PortfolioExplorer {


	///////////////////////////////////////////////////////////////////////////////
	// BindRegExpEngine: Private method to bind provided patterns to regular expression engine
	// Inputs:
	//  const std::vector<PatternSymbol>&	patternSymbols	vector of patterns and symbols
	//  ListenerRef				listener		receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool Symbolizer::BindRegExpEngine(
	    const std::vector<PatternSymbol>& patternSymbols,
		RegularExprEngineRef the_engine,
		ListenerRef listener  
    )
	{
	
		//Objects we need to analyze expressions
		ListenerFIFORef listenerFIFO = new ListenerFIFO;

		LexerInputRef lexInput;
		RegularExprLexerRef lexer;
		RegularExprParserRef parser;
		RegularExprRef result;

		{for(unsigned i = 0;i < patternSymbols.size(); i++) {
			lexInput  = new LexerInput_string(patternSymbols[i].pattern);
			lexer = new RegularExprLexerChar(lexInput, listenerFIFO.get());
			parser = new RegularExprParserChar(lexer, listenerFIFO.get());
			result = parser->Parse();
			if( result == 0 ) {
				// parse of regexp failed.
				while (listenerFIFO->HaveMessage()) {
					// Add prefix to message string and send to listener argument.
					ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
					listener->Error("Error parsing pattern " + FormatInteger(i+1) + ": " + msg.str);
				}
				return false;
			}
			//Check for top-level subexpressions here?
			the_engine->AddRegularExpression(result);
		}}
		//Bind the ascii character set
		{for(int i = 0; i < 256; i++) {
			TsString thechar;
			thechar = char(i);
			the_engine->AddExpressionSymbol(thechar, listenerFIFO.get());
		}}

		bool ret = the_engine->Bind(listener);
		if( !ret ) {
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
	// Bind: Bind the symbolizer object to specified settings
	// Inputs:
	//  const std::vector<PatternSymbol>&	patternSymbols	vector of patterns and symbols
	//  ListenerRef				listener		receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool Symbolizer::Bind(
	    const std::vector<PatternSymbol>& patternSymbols,
		TsString defaultSymbol,
		ListenerRef listener  // receives error emssages for bind
    )
	{
		bound = false;
		//Initialize engine member variable
		engine = new RegularExprEngine();
		
		//Set our local attributes (needed for processing)
		symbols = patternSymbols;
		default_symbol = defaultSymbol;

		//Setup the Regular Expression engine and make sure all of the patterns compile
		bound = BindRegExpEngine(patternSymbols, engine, listener);
		return bound;

	}

	///////////////////////////////////////////////////////////////////////////////
	// Bind: Bind the tokenizer object to specified settings
	// Inputs:
	//  DataItemRef&	config		configuration containing items necessary to
	//								set up tokenizer
	//  ListenerRef		listener	receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool Symbolizer::Bind(
	    DataItemRef& config,
		ListenerRef listener  // receives error emssages for bind
    )
	{
		DataItemRef tmp;
		TsString defaultSymbol;
	    const std::vector<PatternSymbol> patternSymbols;

		// Default Symbol
		tmp = config["DEFAULT"];
		if (tmp != 0) {
			defaultSymbol = TsString(*tmp);
		}

		DataItemRef patternsRef = config["PATTERNS"];
		if (patternsRef == 0 || patternsRef->GetSize() == 0) {
			listener->Error("No symbol patterns specified");
			return false;
		}
		TsString pattern;
		TsString symbol;
		for (int patternIdx = 0; patternIdx < patternsRef->GetSize(); patternIdx++) {
			DataItemRef patternSpec = patternsRef[patternIdx];

			// Pattern
			tmp = patternSpec["PATTERN"];
			if (tmp != 0) {
				pattern = TsString(*tmp);
			}
			tmp = patternSpec["SYMBOL"];
			if (tmp != 0) {
				symbol = TsString(*tmp);
			}
			symbols.push_back(PatternSymbol(pattern, symbol));
		}

		if( symbols.size() == 0 ) {
			listener->Error("No symbol patterns specified");
			return false;
		}
		return Bind(symbols, defaultSymbol, listener);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	// Unbind: Unbind the symbolizer
	// Inputs:
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
    void Symbolizer::Unbind()
	{
		//Setting ref to 0 will force freeing of member engine
		engine = 0;
		bound = false;
		symbols.clear();
		default_symbol = "";
	}

	///////////////////////////////////////////////////////////////////////////////
	// Process: Process a stream of characters and return a vector of tokens
	// Inputs:
	//  std::vecotr<const char*>&		tokens			tokens to process
	//	BulkAllocatorRef	bulkAllocator	bulk allocator to allocate 
	//													symbols
	// Outputs:
	//  std::vector<char*>&	symbolsReturn	vector of processed symbols
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
    void Symbolizer::Process(
            const std::vector<const char*>& tokens,
            std::vector<const char*>& symbolsReturn,
			BulkAllocatorRef bulkAllocator    
    )
	{
		// Vector to hold tokens during processing 
		std::vector<const char *> tempTokens;

		symbolsReturn.clear();
		//bulkAllocator->Reset();

		for (unsigned i = 0; i < tokens.size(); i++) {
			const char* matchSymbol = MatchString(tokens[i], bulkAllocator);
			if (matchSymbol == 0) {
				matchSymbol = default_symbol.c_str();
			}
			symbolsReturn.push_back(matchSymbol);
		}

	}

	//////////////////////////////////////////////////////////////////////
	// Match a string against the patterns
	// Inputs:
	//	const char*						text		The text to match
	//  BulkAllocatorRef	allocator	allocator for strings
	// Return value:
	//	const char*		The corresponding symbol, or zero if no match.
	//////////////////////////////////////////////////////////////////////
	const char* Symbolizer::MatchString(
		const char* text, 
		BulkAllocatorRef allocator
	)
	{
		assert(engine != 0);
		
		matchVals.clear();

		// Variables needed for expression engine
		unsigned char* tempChar;

		// Push each character into the match vector
		for (
			const char* ptr = text;
			*ptr != 0;
			ptr++
		) {
			tempChar = (unsigned char*)allocator->New(2);
			tempChar[0] = *ptr;
			tempChar[1] = 0;
			matchVals.push_back(tempChar);
		}

		int exprMatched;
		int endPosition;
		actions.clear();
		if (engine->Match(matchVals.begin(), matchVals.end(), exprMatched, actions, endPosition)) {
			assert(exprMatched < int(symbols.size()));
			return symbols[exprMatched].symbol.c_str();
		} else {
			return 0;
		}
	}

} //namespace
