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

// RegularExprPatternMatcher.h: Definition of pattern matcher class

#ifndef INCL_REGULAREXPRPATTERNMATCHER_H
#define INCL_REGULAREXPRPATTERNMATCHER_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "RegularExprEngine.h"
#include "BulkAllocator.h"
#include "DataItem.h"
#include "ListenerFIFO.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class PatternMatcher : public VRefCount {
	
	public:
    
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		PatternMatcher() : bound(false) {};

		// Holds replacement symbol sets
		struct SymbolSetEntry {
			SymbolSetEntry(
				const TsString& name_,
				const std::vector<TsString>& symbolSet_
			) :
				name(name_),
				symbolSet(symbolSet_)
			{}
			TsString name;
			std::vector<TsString> symbolSet;
		};

		///////////////////////////////////////////////////////////////////////////////
		// Bind: Bind the PatternMatcher object to specified settings
		// Inputs:
		//  const std::vector<PatternClasses>&	patternClasses	vector of patterns and classes
		//  ListenerRef				listener		receives error messages
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Bind(
			const std::vector<TsString>& patterns,
			const std::vector<SymbolSetEntry>& symbolSets,
			ListenerRef listener  // receives error emssages for bind
		);

		///////////////////////////////////////////////////////////////////////////////
		// Bind: Bind the PatternMatcher object to specified settings
		// Inputs:
		//  DataItemRef&	config		configuration containing items necessary to
		//								set up Pattern Matcher
		//  ListenerRef		listener	receives error messages
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Bind(
			DataItemRef& config,
			ListenerRef listener  // receives error messages for bind
		);

		///////////////////////////////////////////////////////////////////////////////
		// Unbind: Unbind the PatternMatcher
		// Inputs:
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void Unbind();
        
		///////////////////////////////////////////////////////////////////////////////
		// Bound Returns the bound state of the Pattern Matcher
		// Inputs:
		//
		// Return value:
		//  bool		The bound state of the Pattern Matcher
		///////////////////////////////////////////////////////////////////////////////
		bool Bound() { return bound; }

		///////////////////////////////////////////////////////////////////////////////
		// GetPatternClasses: Retrieve pattern classes extracted from patterns in Bind()
		// Inputs:
		//
		// Return value:
		//  std::vector<std::vector<TsString> >	classes extracted from patterns
		///////////////////////////////////////////////////////////////////////////////
		std::vector<std::vector<TsString> > GetPatternClasses() { return classes; }

		///////////////////////////////////////////////////////////////////////////////
		// Process: Process a vector of symbols and return a vector of classes
		// Inputs:
		//  std::vector<const char*>&		
		//						symbols			symbols to process
		//	BulkAllocatorRef	bulkAllocator	bulk allocator to allocate classes
		// Outputs:
		//  std::vector<char*>&	classesReturn	vector of processed classes
		//										Caller should NOT free pointers!
		//
		// Return value:
		//  int				Pattern index of match or -1 for no match
		///////////////////////////////////////////////////////////////////////////////
		int Process(
			const std::vector<unsigned char*>& symbols,
			std::vector<char*>& classesReturn,
			BulkAllocatorRef bulkAllocator    
		);
	
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
		int MultiProcess(
			const VectorNoDestruct<std::vector<unsigned char*> >& symbols,
			std::vector<char*>& classesReturn,
			BulkAllocatorRef bulkAllocator    
		);
	
	private:
		///////////////////////////////////////////////////////////////////////////////
		// BindRegExpEngine: Private method to bind provided patterns to regular expression engine
		// Inputs:
		//  const std::vector<PatternSymbol>&	patternSymbols	vector of patterns and symbols
		//  ListenerRef				listener		receives error messages
		// Outputs:
		//  std::vector<TsString>&			classesReturn	vector of classes mapped to engine
		//                                                      actions
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool BindRegExpEngine(
			const std::vector<TsString>& patterns,
			const std::vector<SymbolSetEntry>& symbolSets,
			RegularExprEngineRef the_engine,
			std::vector<std::vector<TsString> >& classesReturn,
			ListenerRef listener  
        );

	private:
		//Expression engine
		RegularExprEngineRef engine;

		// Vector to hold patterns and symbols provided in Bind()
		std::vector<std::vector<TsString> > classes;

		// Vector to hold action results.
		std::vector <ActionResult> actionResults;

		bool bound;
	};

	typedef refcnt_ptr<PatternMatcher> PatternMatcherRef;

} //namespace

#endif
