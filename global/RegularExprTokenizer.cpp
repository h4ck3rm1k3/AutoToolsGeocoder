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
# $Rev: 49 $ 
# $Date: 2006-09-25 20:00:58 +0200 (Mon, 25 Sep 2006) $ 
*/

// RegularExprTokenizer.cpp: Defines the regular expression tokenizer object

#include "Global_Headers.h"
#include "RegularExprTokenizer.h"
#include "RegularExpr.h"
#include "RegularExprLexer.h"
#include "RegularExprParser.h"
#include "LexerInput.h"

#include <algorithm> 

namespace PortfolioExplorer {

	static inline void Reverse(
		const char* src,
		TsString& dest
	) {
		dest.erase();
		for (int i = int(strlen(src)-1); i >= 0; --i) {
			dest += src[i];
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// BindRegExpEngine: Private method to bind provided patterns to regular expression engine
	// Inputs:
	//  const std::vector<TsString>& splitApartPatterns	vector of strings describing
	//														regexp patterns to be split
	//  const std::vector<TsString>&	noSplitPatterns		vector of strings describing
	//														regexp patterns NOT to be split
	//  ListenerRef			listener	receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool Tokenizer::BindRegExpEngine(
		const std::vector<TsString>& splitApartPatterns,
		const std::vector<TsString>& noSplitPatterns,
		ListenerRef listener  
    ) {
		// Clear out the trie containing do-not-split strings.
		noSplitTrie.Clear();
		splitTrie.Clear();
		splitReverseTrie.Clear();

		//Objects we need to analyze expressions
		ListenerFIFORef listenerFIFO = new ListenerFIFO;

		LexerInputRef lexInput;
		RegularExprLexerRef lexer;
		RegularExprParserRef parser;
		RegularExprRef result;

		//Create all split apart patterns and add them to the engine
		{for (unsigned i = 0;i < splitApartPatterns.size(); i++) {
			lexInput  = new LexerInput_string(splitApartPatterns[i]);
			lexer = new RegularExprLexerChar(lexInput, listenerFIFO.get());
			parser = new RegularExprParserChar(lexer, listenerFIFO.get());
			result = parser->Parse();
			if (result == 0 ) {
				// parse of regexp failed.
				while (listenerFIFO->HaveMessage()) {
					// Add prefix to message string and send to listener argument.
					ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
					listener->Error("Error parsing pattern " + FormatInteger(i+1) + ": " + msg.str);
				}
				return false;
			}
			// Check that we have at least one action
			if (!result->FindAction()) {
				listener->Warning("No action specified in pattern " + FormatInteger(i+1) + ", no tokens will be generated by this pattern");
			}

			splitEngine->AddRegularExpression(result);
		}}
		//Bind the ascii character set
		{for(int i = 0; i < 256; i++) {
			TsString thechar;
			thechar = char(i);
			splitEngine->AddExpressionSymbol(thechar, listenerFIFO.get());
		}}

		if (!splitEngine->Bind(listener)) {
			// parse of regexp failed.
			while (listenerFIFO->HaveMessage()) {
				// Add prefix to message string and send to listener argument.
				ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
				listener->Error("Error binding patterns:  " + msg.str);
			}
			return false;
		}
		
		//Create a separate engine for each of the no-split patterns.  This is necessary because
		//the input text only has to match part of a no-split pattern and a single engine might
		//simply fail to match
		noSplitEngines.clear();
		{for (unsigned i = 0;i < noSplitPatterns.size(); i++) {
			lexInput  = new LexerInput_string(noSplitPatterns[i]);
			lexer = new RegularExprLexerChar(lexInput, listenerFIFO.get());
			parser = new RegularExprParserChar(lexer, listenerFIFO.get());
			result = parser->Parse();
			if (result == 0 ) {
				// parse of regexp failed.
				while (listenerFIFO->HaveMessage()) {
					// Add prefix to message string and send to listener argument.
					ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
					listener->Error("Error parsing no-split pattern " + FormatInteger(i+1) + ": " + msg.str);
				}
				return false;
			}
			RegularExprEngineRef noSplitEngine = new RegularExprEngine();
			noSplitEngine->AddRegularExpression(result);

			//Bind the ascii character set
			for(int j = 0; j < 256; j++) {
				TsString thechar;
				thechar = char(j);
				noSplitEngine->AddExpressionSymbol(thechar, listenerFIFO.get());
			}

			if (!noSplitEngine->Bind(listener)) {
				// parse of regexp failed.
				while (listenerFIFO->HaveMessage()) {
					// Add prefix to message string and send to listener argument.
					ListenerFIFO::Message msg = listenerFIFO->GetNextMessage();
					listener->Error("Error binding no-split patterns:  " + msg.str);
				}
				return false;
			}
			//Put this engine in the vector
			noSplitEngines.push_back(noSplitEngine);
		}}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Bind: Bind the tokenizer object to specified settings
	// Inputs:
	//  const TsString&	whitespace	string of whitespace characters
	//  const TsString&	framing		string of framing characters
	//  const std::vector<TsString>&	splitApartPatterns	vector of strings describing
	//														regexp patterns
	//  const std::vector<TsString>&	noSplitPatterns		vector of strings describing
	//                                                      regexp patterns to NOT split
	//  ListenerRef			listener			receives error messages
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool Tokenizer::Bind(
        const TsString& whitespace_,
        const TsString& framing_,
        const std::vector<TsString>& splitApartPatterns,
        const std::vector<TsString>& noSplitPatterns,
		ListenerRef listener  
    ) {
		bound = false;

		//Initialize engine member variable
		splitEngine = new RegularExprEngine();
		
		// Set up char flags
		charFlags.clear();
		{for (int i = 0; i < 256; i++) {
			charFlags.push_back(IsNone);
		}}
		{for (unsigned i = 0; i < whitespace_.size(); i++) {
			charFlags[(unsigned char)whitespace_[i]] = IsWhitespace;
		}}
		{for (unsigned i = 0; i < framing_.size(); i++) {
			charFlags[(unsigned char)framing_[i]] = IsFraming;
		}}

		//Setup the Regular Expression engine and make sure all of the patterns compile
		bound = BindRegExpEngine(splitApartPatterns, noSplitPatterns, listener);
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
	bool Tokenizer::Bind(
        DataItemRef& config,
		ListenerRef listener  
    ) {
		TsString whitespace;
		TsString framing;
		std::vector<TsString> noSplitPatterns;
		std::vector<TsString> splitPatterns;

		DataItemRef tmp = config["WHITESPACE"];
		if (tmp != 0) {
			whitespace = UnescapeString_C(TsString(*tmp));
		}

		tmp = config["FRAMING"];
		if (tmp != 0) {
			framing = UnescapeString_C(TsString(*tmp));
		}

		// No-split tokens
		DataItemRef noSplitRef = config["NOSPLIT"];
		if (noSplitRef != 0) {
			for (int idx = 0; idx < noSplitRef->GetSize(); idx++) {
				tmp = (*noSplitRef)[idx];
				if (tmp == 0 ) {
					continue;
				}
				TsString tmpStr = TsString(*tmp);
				if (tmpStr.empty()) {
					continue;
				}
				noSplitPatterns.push_back(tmpStr);
			}
		}

		// Patterns and symbols
		DataItemRef patternsRef = config["PATTERNS"];
		if (patternsRef != 0) {
			for (int idx = 0; idx < patternsRef->GetSize(); idx++) {
				tmp = (*patternsRef)[idx];
				if (tmp == 0 ) {
					continue;
				}
				TsString pattern = TsString(*tmp);
				if (!pattern.empty()) {
					splitPatterns.push_back(pattern);
				}
			}
		}

		if (whitespace.empty() && splitPatterns.empty() && framing.empty()) {
			listener->Error("No whitespace characters or patterns specified");
			return false;
		}

		return Bind(whitespace, framing, splitPatterns, noSplitPatterns, listener);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	// Unbind: Unbind the tokenizer
	// Inputs:
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
    void Tokenizer::Unbind()
	{
		//Setting ref to 0 will force freeing of member engine
		splitEngine = 0;
		noSplitEngines.clear();
		noSplitTrie.Clear();
		splitTrie.Clear();
		splitReverseTrie.Clear();
		bound = false;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Process: Process a stream of characters and return a vector of tokens
	// Inputs:
	//  const char*						text			text stream to process
	//	BulkAllocatorRef	bulkAllocator	bulk allocator to allocate 
	//													tokens
	// Outputs:
	//  std::vector<char*>	tokensReturn	vector of processed tokens
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
    void Tokenizer::Process(
        const char* text,
        std::vector<const char*>& tokensReturn,
		BulkAllocatorRef bulkAllocator    
    ) {
		tempTokens.clear();
		actions.clear();
		tokensReturn.clear();

		//
		// Split the input on whitespace -- if the string doesn't contain any whitespace it
		// will be put into the tempTokens vector whole.  While doing this, suppress token
		// splits using the noSplitEngines.
		//
		fieldValue = text;
		TsString::iterator fieldEnd = fieldValue.end();
		TsString::iterator iter = fieldValue.begin();;
		TsString::iterator newIter;

		// Split the tokens by the whitespace and framing characters
		PredCharFlag pred(charFlags);
		int matchReturn;
		int exprMatched;
		while ( 
			iter < fieldEnd &&
			(newIter = std::find_if(iter, fieldEnd, pred)) != fieldEnd
		) {
			// Found a whitespace/framing char
			// Check for nosplit
			bool matchedNoSplit = false;
			for(
				std::vector<RegularExprEngineRef>::iterator engine_iter = noSplitEngines.begin();
				engine_iter != noSplitEngines.end();
				++engine_iter
			) {
				bool match = (*engine_iter)->Match((const unsigned char*)(&(*iter)), exprMatched, actions, matchReturn);
				if (matchReturn > -1) {
					if (iter + matchReturn == fieldEnd) {
						// Nosplit token spans to end of string.  Stop searching.
						newIter = fieldEnd;
						if( match ) {
							matchedNoSplit = true;
							break;
						}
					}
					if (pred(iter[matchReturn])) {
						// Nosplit token stops at framing.  Break token here instead.
						newIter = iter + matchReturn;
						if( match ) {
							matchedNoSplit = true;
							break;
						}
					}
				}
			}

			if (!matchedNoSplit) {
				// Look for nosplit in trie
				int length;
				if (noSplitTrie.FindLongestKeyPrefix((const unsigned char*)(&(*iter)), length)) {
					if (iter + length == fieldEnd) {
						// Nosplit token spans to end of string.  Stop searching.
						newIter = fieldEnd;
					}
					if (pred(iter[length])) {
						// Nosplit token stops at framing.  Break token here instead.
						newIter = iter + length;
					}
				}
			}

			// Now we're OK to proceed
			if (newIter != iter) {
				// We skipped some text.  Store it.
				// Allocate a new string and copy the substring into it
				int length = int(newIter - iter);
				char* ptr = (char*)bulkAllocator->New(length + 1);
				memcpy(ptr, &*iter, length);
				ptr[length] = 0;
				tempTokens.push_back(ptr);
			}

			// Deal with the found whitespace/framing char
			if (charFlags[(unsigned char)*newIter] == IsFraming) {
				// Framing char, make a token out of it
				char* ptr = (char*)bulkAllocator->New(2);
				ptr[0] = *newIter;
				ptr[1] = 0;
				tempTokens.push_back(ptr);
			} else {
				// Whitespace char, skip it
			}

			iter = newIter + 1;
		}

		if (iter < fieldEnd) {
			// Get string left on end
			// Allocate a new string and copy the substring into it
			int length = int(fieldEnd - iter);
			char* ptr = (char*)bulkAllocator->New(length + 1);
			memcpy(ptr, &*iter, length);
			ptr[length] = 0;
			tempTokens.push_back(ptr);
		}

		//
		// Run each token through the regexp engine or split true, and split them if necessary.
		//
		{
			//Variables needed for expression engine
			std::vector<ActionResult>::iterator actionIterator;
			int exprMatched;

			std::vector<const char *>::iterator tokenIter;
			int endPosition;
			int length;
			// Try matching each of the tokens in tempTokens
			for (tokenIter = tempTokens.begin(); tokenIter != tempTokens.end(); tokenIter++) {
				const unsigned char* iterPtr = (const unsigned char*)(*tokenIter);
				actions.clear();
				if (
					splitEngine != 0 &&
					splitEngine->Match(iterPtr, exprMatched, actions, endPosition) &&
					actions.size() > 0 
				) {
					// Did we get any subexpressions?
					int enter, exit;
					for (actionIterator = actions.begin(); actionIterator != actions.end(); actionIterator++) {
						enter = (*actionIterator).enterPosition;
						exit = (*actionIterator).exitPosition;
						if (exit - enter > 0) {
							char* ptr = (char *)bulkAllocator->New(exit - enter + 1);
							memcpy(ptr, &(*tokenIter)[enter], exit - enter);
							ptr[exit - enter] = 0;
							tokensReturn.push_back(ptr);
						}
					}
				} else if (
					splitTrie.FindKeyPrefix(iterPtr, length) && 
					length > 0 &&
					length < int(strlen(*tokenIter))
				) {
					// Token before split
					char* ptr = (char *)bulkAllocator->New(length + 1);
					memcpy(ptr, iterPtr, length);
					ptr[length] = 0;
					tokensReturn.push_back(ptr);
					// Token after split
					int remainLength = int(strlen(*tokenIter) - length);
					ptr = (char *)bulkAllocator->New(remainLength + 1);
					memcpy(ptr, iterPtr, remainLength);
					ptr[remainLength] = 0;
					tokensReturn.push_back(ptr);
				} else {
					// Check tail split
					Reverse((const char*)iterPtr, tmpReverseStr);
					if (
						splitReverseTrie.FindKeyPrefix((const unsigned char*)tmpReverseStr.c_str(), length) && 
						length > 0 &&
						length < int(tmpReverseStr.size())
					) {
						int remainLength = int(tmpReverseStr.size() - length);
						// Token after (tail) split
						char* ptr = (char *)bulkAllocator->New(length + 1);
						memcpy(ptr, iterPtr + remainLength, length);
						ptr[length] = 0;
						tokensReturn.push_back(ptr);
						// Token after split
						ptr = (char *)bulkAllocator->New(remainLength + 1);
						memcpy(ptr, iterPtr, remainLength);
						ptr[remainLength] = 0;
						tokensReturn.push_back(ptr);
					} else {
						// No actions here... put the whole token onto the vector
						tokensReturn.push_back(*tokenIter);
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Add do-not-split strings
	///////////////////////////////////////////////////////////////////////////////
	void Tokenizer::AddNoSplitString(
		const unsigned char* str
	) {
		if (*str != 0) {
			noSplitTrie.Insert(str, 1);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Add split strings (split front or back)
	///////////////////////////////////////////////////////////////////////////////
	void Tokenizer::AddSplitString(
		const unsigned char* str
	) {
		if (*str != 0) {
			splitTrie.Insert(str, 1);
		}
	}


} //namespace