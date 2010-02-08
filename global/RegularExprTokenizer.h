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

// RegularExprTokenizer.h: Definition of tokenizer class

#ifndef INCL_REGULAREXPRTOKENIZER_H
#define INCL_REGULAREXPRTOKENIZER_H

#if _MSC_VER >= 1000
	#pragma once
#endif


#include "RegularExprEngine.h"
#include "DataItem.h"
#include "ListenerFIFO.h"
#include "BulkAllocator.h"
#include "Global_DllExport.h"
#include "Trie.h"

namespace PortfolioExplorer {

	class Tokenizer : public VRefCount {
	
	public:
    
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		Tokenizer() : bound(false) {};

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
		bool Bind(
                const TsString& whitespace_,
                const TsString& framing_,
                const std::vector<TsString>& splitApartPatterns,
                const std::vector<TsString>& noSplitPatterns,
				ListenerRef listener  
        );

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
		bool Bind(
				DataItemRef& config,
				ListenerRef listener  
		);

		///////////////////////////////////////////////////////////////////////////////
		// Add do-not-split strings
		///////////////////////////////////////////////////////////////////////////////
		void AddNoSplitString(const unsigned char* str);

		///////////////////////////////////////////////////////////////////////////////
		// Add split strings (splits at front or back)
		///////////////////////////////////////////////////////////////////////////////
		void AddSplitString(const unsigned char* str);

		///////////////////////////////////////////////////////////////////////////////
		// Unbind: Unbind the tokenizer
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
        void Unbind();

		///////////////////////////////////////////////////////////////////////////////
		// Bound Returns the bound state of the tokenizer
		// Inputs:
		//
		// Return value:
		//  bool		The bound state of the tokenizer
		///////////////////////////////////////////////////////////////////////////////
        bool Bound() { return bound; }

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
        void Process(
                const char* text,
                std::vector<const char*>& tokensReturn,
				BulkAllocatorRef bulkAllocator    
        );
		
	private:
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
		bool BindRegExpEngine(
                const std::vector<TsString>& splitApartPatterns,
				const std::vector<TsString>& noSplitPatterns,
				ListenerRef listener  
        );


	private:
		//Expression engine to split out tokenns
		RegularExprEngineRef splitEngine;

		bool bound;

		// Defines characters to be broken for whitespace or framing
		enum CharFlag { IsNone = 0 , IsWhitespace = 1, IsFraming = 2};
		std::vector<CharFlag> charFlags;

		// Predicate object to assist finding flagged characters
		struct PredCharFlag;
		friend struct PredCharFlag;
		struct PredCharFlag {
			PredCharFlag(const std::vector<CharFlag>& flags_) : flags(flags_) {}
			const std::vector<CharFlag>& flags;
			bool operator()(unsigned char c) const {
				return flags[c] != IsNone;
			}
		};

		// Vector of engines to process
		std::vector<RegularExprEngineRef> noSplitEngines;

		// Vector to hold tokens during processing 
		std::vector<const char *> tempTokens;

		// Vectors to hold match results
		std::vector<ActionResult> actions;

		//String for text processing (don't want it created on the stack)
		TsString fieldValue;

		// Trie for do-not-split strings
		Trie<unsigned char> noSplitTrie;

		// Trie for split strings
		Trie<unsigned char> splitTrie;
		// Trie for split strings at end of token
		Trie<unsigned char> splitReverseTrie;

		TsString tmpReverseStr;
	};
	typedef refcnt_ptr<Tokenizer> TokenizerRef;

} //namespace

#endif
