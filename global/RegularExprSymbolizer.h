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

// RegularExprSymbolizer.h: Definition of symbolizer class

#ifndef INCL_REGULAREXPRSYMBOLIZER_H
#define INCL_REGULAREXPRSYMBOLIZER_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "RegularExprEngine.h"
#include "BulkAllocator.h"
#include "ListenerFIFO.h"
#include "DataItem.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class Symbolizer : public VRefCount {
	
	public:
    
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		Symbolizer() : bound(false) {};

	    struct PatternSymbol {
		    PatternSymbol(
			    const TsString& pattern_,
                const TsString& symbol_
            ) :
				pattern(pattern_),
                symbol(symbol_)
            {}
            TsString pattern;
            TsString symbol;
		};
        
		///////////////////////////////////////////////////////////////////////////////
		// Bind: Bind the symbolizer object to specified settings
		// Inputs:
		//  const std::vector<PatternSymbol>&	patternSymbols	vector of patterns and symbols
		//  ListenerRef				listener		receives error messages
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Bind(
	        const std::vector<PatternSymbol>& patternSymbols,
			const TsString defaultSymbol,
			ListenerRef listener  // receives error emssages for bind
        );

		///////////////////////////////////////////////////////////////////////////////
		// Bind: Bind the symbolizer object to specified settings
		// Inputs:
		//  const std::vector<PatternSymbol>&	patternSymbols	vector of patterns and symbols
		//  ListenerRef				listener		receives error messages
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Bind(
			DataItemRef& config,
			ListenerRef listener  // receives error emssages for bind
		);

		///////////////////////////////////////////////////////////////////////////////
		// Unbind: Unbind the symbolizer
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
        void Unbind();
        
		///////////////////////////////////////////////////////////////////////////////
		// Process: Process a stream of characters and return a vector of tokens
		// Inputs:
		//  std::vector<const char*>&		tokens			tokens to process
		//	BulkAllocatorRef	bulkAllocator	bulk allocator to allocate 
		//													symbols
		// Outputs:
		//  std::vector<char*>&	symbolsReturn	vector of processed symbols
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
        void Process(
                const std::vector<const char*>& tokens,
                std::vector<const char*>& symbolsReturn,
				BulkAllocatorRef bulkAllocator    
        );
	
		///////////////////////////////////////////////////////////////////////////////
		// Bound Returns the bound state of the tokenizer
		// Inputs:
		//
		// Return value:
		//  bool		The bound state of the tokenizer
		///////////////////////////////////////////////////////////////////////////////
        bool Bound() { return bound; }

		///////////////////////////////////////////////////////////////////////////////
		// GetSymbols Returns the PatternSymbol vector extracted in Bind()
		// Inputs:
		//
		// Return value:
		//  const std::vector<PatternSymbol>	PatternSymbol vector
		///////////////////////////////////////////////////////////////////////////////
		const std::vector<PatternSymbol> GetSymbols() { return symbols; }

	private:
		///////////////////////////////////////////////////////////////////////////////
		// BindRegExpEngine: Private method to bind provided patterns to regular expression engine
		// Inputs:
		//  const std::vector<PatternSymbol>&	patternSymbols	vector of patterns and symbols
		//  ListenerRef				listener		receives error messages
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool BindRegExpEngine(
	        const std::vector<PatternSymbol>& patternSymbols,
			RegularExprEngineRef the_engine,
			ListenerRef listener  
        );

		//////////////////////////////////////////////////////////////////////
		// Match a string against the patterns
		// Inputs:
		//	const char*						text		The text to match
		//  BulkAllocatorRef	allocator	allocator for strings
		// Return value:
		//	const char*		The corresponding symbol, or zero if no match.
		//////////////////////////////////////////////////////////////////////
		const char* MatchString(
			const char* text, 
			BulkAllocatorRef allocator
		);

	private:
		//Expression engine
		RegularExprEngineRef engine;

		bool bound;

		//Default symbol (when nothing else matches)
		TsString default_symbol;

		// Vector to hold patterns and symbols provided in Bind()
		std::vector<PatternSymbol> symbols;


		//For processing
		std::vector<unsigned char*> matchVals;
		std::vector<ActionResult> actions;
	};

	typedef refcnt_ptr<Symbolizer> SymbolizerRef;

} //namespace

#endif
