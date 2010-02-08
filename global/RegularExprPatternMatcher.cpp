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

// RegularExprPatternMatcher.cpp: Defines the regular expression PatternMatcher object

#include "Global_Headers.h"
#include "RegularExprPatternMatcher.h"
#include "RegularExpr.h"
#include "RegularExprLexer.h"
#include "RegularExprParser.h"
#include "LexerInput.h"
#include "StringSet.h"

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
	bool PatternMatcher::BindRegExpEngine(
		const std::vector<TsString>& patterns,
		const std::vector<SymbolSetEntry>& symbolSets,
		RegularExprEngineRef the_engine,
		std::vector<std::vector<TsString> >& classesReturn,
		ListenerRef listener  
    )
	{
	
 		//Objects we need to analyze expressions
		ListenerFIFORef listenerFIFO = new ListenerFIFO;

		LexerInputRef lexInput;
		RegularExprLexerRef lexer;
		RegularExprParserRef parser;
		RegularExprRef result;

		classesReturn.clear();
		std::vector<TsString> tmpClasses;
		{for (unsigned i = 0;i < patterns.size(); i++) {
			tmpClasses.clear();
			lexInput  = new LexerInput_string(patterns[i]);
			lexer = new RegularExprLexerSymbol(lexInput, listenerFIFO.get());
			parser = new RegularExprParserSymbol(lexer, listenerFIFO.get());
			for (unsigned j = 0; j < symbolSets.size(); j++) {
				parser->AddLiteralSet(symbolSets[j].name, symbolSets[j].symbolSet);
			}
			result = parser->Parse();
			tmpClasses = parser->GetActionMappings();
			if (tmpClasses.size() == 0) {
				listener->Error("No CLASSES specified for pattern " + FormatInteger(i+1));
				return false;
			}
			if( result == 0 ) {
				// parse of regexp failed.
				while (listenerFIFO->HaveMessage()) {
					// Add prefix to message string and send to listener argument.
					ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
					listener->Error("Error parsing pattern " + FormatInteger(i+1) + ": " + msg.str);
				}
				return false;
			}
			the_engine->AddRegularExpression(result);
			classesReturn.push_back(tmpClasses);			
		}}
		//Bind the ascii character set
		{for (int i = 0; i < 256; i++) {
			TsString thechar;
			thechar = char(i);
			the_engine->AddExpressionSymbol(thechar, listenerFIFO.get());
		}}

		bool ret = the_engine->Bind(listenerFIFO.get());
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
	// Bind: Bind the PatternMatcher object to specified settings
	// Inputs:
	//  const std::vector<PatternClasses>&	patternClasses	vector of patterns and classes
	//  ListenerRef				listener		receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool PatternMatcher::Bind(
	    const std::vector<TsString>& patterns,
		const std::vector<SymbolSetEntry>& symbolSets,
		ListenerRef listener  // receives error messages for bind
    )
	{
		bound = false;
		//Initialize engine member variable
		engine = new RegularExprEngine();
		
		//Setup the Regular Expression engine and make sure all of the patterns compile
		bound = BindRegExpEngine(patterns, symbolSets, engine, classes, listener);
		return bound;

	}

	///////////////////////////////////////////////////////////////////////////////
	// Bind: Bind the Pattern Matcher object to specified settings
	// Inputs:
	//  DataItemRef&	config		configuration containing items necessary to
	//								set up Pattern Matcher
	//  ListenerRef		listener	receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool PatternMatcher::Bind(
	    DataItemRef& config,
		ListenerRef listener  // receives error messages for bind
    )
	{
		// Symbol Sets
		std::vector<SymbolSetEntry> symbolSets;
		StringSet usedSymbolSets;
		std::vector<TsString> patterns;

		DataItemRef tmp;
		DataItemRef symbolSetSpecs = config["SYMBOL_SETS"];
		if (symbolSetSpecs != 0) {
			for (int symbolSetIdx = 0; symbolSetIdx < symbolSetSpecs->GetSize(); symbolSetIdx++) {
				DataItemRef symbolSetSpec = symbolSetSpecs[symbolSetIdx];

				// Symbol Set name
				tmp = symbolSetSpec["NAME"];
				TsString name;
				if (tmp != 0) {
					name = TsString(*tmp);
				}
				if (name.empty()) {
					listener->Error("No NAME specified for SYMBOL_SET " + FormatInteger(symbolSetIdx+1));
					continue;
				}
				for (unsigned i = 0; i < name.size(); i++) {
					if (!ISALNUM(name[i])) {
						listener->Error("SYMBOL_SETS entry '" + name + "' must be only alphanumeric");
					}
				}
				if (usedSymbolSets.find(name) != usedSymbolSets.end()) {
					listener->Error("SYMBOL_SET '" + name + "' specified more than once");
				}
				usedSymbolSets.insert(name);

				// Replacement
				std::vector<TsString> replacementSymbols;
				DataItemRef symbolSetSpecs = symbolSetSpec["SYMBOL_SET"];
				if (symbolSetSpecs != 0) {
					for (int symbolSetIdx = 0; symbolSetIdx < symbolSetSpecs->GetSize(); symbolSetIdx++) {
						tmp = symbolSetSpecs[symbolSetIdx];
						TsString replacementSymbol;
						if (tmp != 0) {
							replacementSymbol = TsString(*tmp);
						}
						if (replacementSymbol.empty()) {
							listener->Error("Empty replacement specified for SYMBOL_SET '" + name + "'");
							continue;
						}
						replacementSymbols.push_back(replacementSymbol);
					}
				}

				symbolSets.push_back(PatternMatcher::SymbolSetEntry(name, replacementSymbols));
			}
		}
		DataItemRef patternSpecs = config["PATTERNS"];
		if (patternSpecs == 0) {
			patternSpecs = new DataItemArray;
		}

		for (int patternIdx = 0; patternIdx < patternSpecs->GetSize(); patternIdx++) {
			DataItemRef patternSpec = patternSpecs[patternIdx];

			// Pattern
			tmp = patternSpec["PATTERN"];
			TsString pattern;
			if (tmp != 0) {
				pattern = TsString(*tmp);
			}
			if (pattern.empty()) {
				listener->Error("No PATTERN specified for pattern " + FormatInteger(patternIdx+1));
				continue;
			}
			//Add the pattern to the patterns collection
			patterns.push_back(pattern);
		}
		return Bind(patterns, symbolSets, listener);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Unbind: Unbind the symbolizer
	// Inputs:
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
    void PatternMatcher::Unbind()
	{
		//Setting ref to 0 will force freeing of member engine
		engine = 0;
		classes.clear();
		bound = false;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Process: Process a vector of symbols and return a vector of classes
	// Inputs:
	//  std::vector<const char*>&		symbols			symbols to process
	//	BulkAllocatorRef	bulkAllocator	bulk allocator to allocate 
	//													classes
	// Outputs:
	//  std::vector<char*>&				classesReturn	vector of processed classes
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	int PatternMatcher::Process(
		const std::vector<unsigned char*>& symbols,
		std::vector<char*>& classesReturn,
		BulkAllocatorRef bulkAllocator    
	) {
		int exprNumber;
		int endPosition;
		
		// Initialize to blank results
		classesReturn.resize(symbols.size());
		{for (unsigned i = 0; i < classesReturn.size(); i++) {
			classesReturn[i] = "";
		}}

		actionResults.clear();
		if (engine->Match(symbols.begin(), symbols.end(), exprNumber, actionResults, endPosition)) {
			// A regular expr matched!
			assert(exprNumber < int(classes.size()));
			unsigned maxClassIdx = unsigned(classes[exprNumber].size() - 1);
			for (unsigned actionIdx = 0; actionIdx < actionResults.size(); actionIdx++) {
				unsigned classIdx = actionResults[actionIdx].subExpr;
				assert(classIdx >= 0);
				if (classIdx > maxClassIdx) {
					assert(false);
					//listener->Warning("In pattern " + FormatInteger(exprNumber+1) + ", action " + FormatInteger(classIdx+1) + " exceeds number of specified classes");
					continue;
				}
				assert(actionResults[actionIdx].exitPosition <= int(classesReturn.size()));
				for (int i = actionResults[actionIdx].enterPosition; i < actionResults[actionIdx].exitPosition; i++) {
					classesReturn[i] = (char*)bulkAllocator->NewString(classes[exprNumber][classIdx].c_str());
				}
			}
		} else {
			exprNumber = -1;
		}
		return exprNumber;	
   }

	///////////////////////////////////////////////////////////////////////////////
	// MultiProcess: Process a vector of "multi-symbols" and return a vector of classes
	// Used for cases where multiple symbol may be present for each token, such as:
	//
	// 3RD:    Name suffix or company
	// FORD:   Name or company
	// W:      Directional or initial
	//
	// Inputs:
	//  VectorNoDestruct<vector<const char*> >&		
	//						symbols			symbols to process
	//	BulkAllocatorRef	bulkAllocator	bulk allocator to use to allocate classes
	// Outputs:
	//  std::vector<char*>&	classesReturn	vector of processed classes
	//										Caller should NOT free pointers!
	//
	// Return value:
	//  int				Pattern index of match or -1 for no match
	///////////////////////////////////////////////////////////////////////////////
	int PatternMatcher::MultiProcess(
		const VectorNoDestruct<std::vector<unsigned char*> >& symbols,
		std::vector<char*>& classesReturn,
		BulkAllocatorRef bulkAllocator    
	) {
		int exprNumber;
		int endPosition;
		
		// Initialize to blank results
		classesReturn.resize(symbols.size());
		{for (unsigned i = 0; i < classesReturn.size(); i++) {
			classesReturn[i] = "";
		}}

		actionResults.clear();
		if (engine->MultiMatch(symbols, exprNumber, actionResults, endPosition)) {
			// A regular expr matched!
			assert(exprNumber < int(classes.size()));
			unsigned maxClassIdx = unsigned(classes[exprNumber].size() - 1);
			for (unsigned actionIdx = 0; actionIdx < actionResults.size(); actionIdx++) {
				unsigned classIdx = actionResults[actionIdx].subExpr;
				assert(classIdx >= 0);
				if (classIdx > maxClassIdx) {
					assert(false);
					//listener->Warning("In pattern " + FormatInteger(exprNumber+1) + ", action " + FormatInteger(classIdx+1) + " exceeds number of specified classes");
					continue;
				}
				assert(actionResults[actionIdx].exitPosition <= int(classesReturn.size()));
				for (int i = actionResults[actionIdx].enterPosition; i < actionResults[actionIdx].exitPosition; i++) {
					classesReturn[i] = (char*)bulkAllocator->NewString(classes[exprNumber][classIdx].c_str());
				}
			}
		} else {
			exprNumber = -1;
		}
		return exprNumber;	
	}

}

