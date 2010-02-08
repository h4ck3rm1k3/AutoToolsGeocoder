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

//////////////////////////////////////////////////////////////////////
// RegularExprWrapper.h: Wrapper for regular expression
//						manipulation of parsing
//////////////////////////////////////////////////////////////////////

#ifndef INCL_RegularExprWrapper_H
#define INCL_RegularExprWrapper_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif


#include "RegularExprTokenizer.h"
#include "RegularExprSymbolizer.h"
#include "RegularExprPatternMatcher.h"
#include "BulkAllocator.h"
#include "VectorNoDestruct.h"
#include "Global_DllExport.h"



namespace PortfolioExplorer {

	struct TokSymCls {
		TsString orginalTokenVal;
		TsString token;
		TsString symbol;
		TsString token_class;
	};

	class RegularExprWrapper : public VRefCount {
	
	public:
		//////////////////////////////////////////////////////////////////////
		// constructor
		//////////////////////////////////////////////////////////////////////
		RegularExprWrapper() {}

		//////////////////////////////////////////////////////////////////////
		// virtual destructor in case it gets deleted by pointer-to-base
		//////////////////////////////////////////////////////////////////////
		virtual ~RegularExprWrapper() {}

		//////////////////////////////////////////////////////////////////////
		// Initialize the regular expression wrapper.
		// Inputs:
		//	const TsString&	dataDir		The directory containing data files.
		// Outputs:
		//	std::String&		errorMsg	The message if an error occured.
		// Return value:
		//	bool	true on success, false on error.
		//////////////////////////////////////////////////////////////////////
		bool ReadPatternToolConfiguration(
			const TsString& patternsConfigurationFile,
			TsString& errorMsg
		);

		//////////////////////////////////////////////////////////////////////
		// Initialize the RegularExprWrapper
		//////////////////////////////////////////////////////////////////////
		bool Open() 
		{ 
			return(
				(tokenizer != 0 && tokenizer->Bound()) &&
				(symbolizer != 0 && symbolizer->Bound()) &&
				(patternMatcher != 0 && patternMatcher->Bound())
			);
		}

		//////////////////////////////////////////////////////////////////////
		// Produce tokens and add them to the result vector
		//////////////////////////////////////////////////////////////////////
		void ProduceTokens(
			char const* value, 
			VectorNoDestruct<TokSymCls>& result, 
			BulkAllocatorRef bulkAllocator
		);

		//////////////////////////////////////////////////////////////////////
		// Produce symbols and add them to the result vector
		//////////////////////////////////////////////////////////////////////
		void ProduceSymbols(VectorNoDestruct<TokSymCls>& result, BulkAllocatorRef bulkAllocator);

		//////////////////////////////////////////////////////////////////////
		// Produce classes and add them to the result vector
		//////////////////////////////////////////////////////////////////////
		void ProduceClasses(VectorNoDestruct<TokSymCls>& result, BulkAllocatorRef bulkAllocator);

	private:
		// Intermediate vector of tokens used during processing.
		std::vector<const char*> tokens;
		std::vector<const char*> symbols;
		std::vector<unsigned char*> uSymbols;
		std::vector<char*> classes;

		// Regular Expression tools needed to parse values
		TokenizerRef tokenizer;
		SymbolizerRef symbolizer;
		PatternMatcherRef patternMatcher;
	};
	typedef refcnt_ptr<RegularExprWrapper> RegularExprWrapperRef;
}

#endif
