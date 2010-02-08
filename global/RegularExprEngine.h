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

// RegularExprNFA.h: Defines classes for the regular expression NFA

#ifndef INCL_REGULAREXPRENGINE_H
#define INCL_REGULAREXPRENGINE_H

#if _MSC_VER >= 1000
	#pragma once
#endif



#include <vector>

#include "RegularExprDefs.h"
#include "RegularExprEngineBase.h"
#include "RegularExpr.h"
#include "RegularExprNFA.h"
#include "RegularExprTrie.h"
#include "Listener.h"
#include "VectorNoDestruct.h"

#include "Global_DllExport.h"

namespace PortfolioExplorer {
		
	class RegularExprEngine : public RegularExprDefs, public RegularExprEngineBase {
	
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		RegularExprEngine() : bound(false), unknownOrdinal(-1) {};

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprEngine() {};
		
		///////////////////////////////////////////////////////////////////////////////
		// AddExpressionSymbol: Add a symbol to the engines list of know regular expression
		//  symbols
		// Inputs:
		//  TsString		symbol			the symbol
		//  ListenerRef 	listener		reports any errors encountered
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool AddExpressionSymbol(TsString symbol, ListenerRef listener);

		///////////////////////////////////////////////////////////////////////////////
		// Bind: Bind all expressions to a map of ordinals, Then generate a single
		//       NFA.
		// Inputs:
		//  OrdinalMap&		map				map of token->ordinal
		//  ListenerRef 	listener		reports any errors encountered
		//
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Bind(ListenerRef listener);

		///////////////////////////////////////////////////////////////////////////////
		// Match: Analyze this sequence of symbols against the NFA, returning true 
		// if it matched an expression, or false if not.  Symbol sequences are passed in
		// as iterator ranges, which are converted to ordinals before processing.
		// Inputs:
		//	vector<unsigned char*>::const_iterator beginValue First symbol to match
		//	vector<unsigned char*>::const_iterator endValue   Last symbol to match
		//
		// Outputs:
		//  int&		 			exprMatched		The number of the expression that matched
		//	vector<ActionRef>&		actionsMatched  Information on the subexpressions matched
		//  int&		 			endPosition		The highest position in the ordinal sequence
		//                                          when the NFA end node is reached
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Match(	
			std::vector<unsigned char*>::const_iterator beginValue, 
			std::vector<unsigned char*>::const_iterator endValue, 
			int& exprMatched, 
			std::vector<ActionResult>& actionsMatched,
			int& endPosition
		);

		///////////////////////////////////////////////////////////////////////////////
		// Match: Special-case matching using input string as ordinal sequence.
		// Inputs:
		//	const unsigned char*	str		The input string of ordinals, null-terminated.
		//
		// Outputs:
		//  int&		 			exprMatched		The number of the expression that matched
		//	vector<ActionRef>&		actionsMatched  Information on the subexpressions matched
		//  int&		 			endPosition		The highest position in the ordinal sequence
		//                                          when the NFA end node is reached
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Match(	
			const unsigned char* str,
			int& exprMatched, 
			std::vector<ActionResult>& actionsMatched,
			int& endPosition
		);


		///////////////////////////////////////////////////////////////////////////////
		// MultiMatch: Analyze a sequence of symbol sets against the NFA.  A sequence of
		// symbol sets is like a sequence of symbols, except that each item may have more
		// than one symbol.  This allows for processing of common cases where a given token
		// can be more than one thing (allows for token-level amgibguity).
		//
		// 3RD:    Name suffix or company
		// FORD:   Name or company
		// W:      Directional or initial
		// 
		// Inputs:
		//	const VectorNoDestruct<vector<unsigned char*> >&
		//							symbols			List of multi-symbols.  Each element is
		//											a vector of possible symbol for the item.
		//											The engine will try multiple possibilities
		//											if the first symbol does not match.
		// Outputs:
		//  int&		 			exprMatched		The number of the expression that matched
		//	vector<ActionRef>&		actionsMatched  Information on the subexpressions matched
		//  int&		 			endPosition		The highest position in the ordinal sequence
		//                                          when the NFA end node is reached
		// Return value:
		//	bool		true if bind succeeded, else false
		///////////////////////////////////////////////////////////////////////////////
		bool MultiMatch(	
			const VectorNoDestruct<std::vector<unsigned char*> >& symbols,
			int& exprMatched, 
			std::vector<ActionResult>& actionsMatched,
			int& endPosition
		);

		///////////////////////////////////////////////////////////////////////////////
		// AddRegularExpression: Add a regular expression to the engine.
		// Inputs:
		//  RegularExprRef	exp				The expression
		//
		// Return value:
		//  int				index specifying this expression (will be returned in match)
		///////////////////////////////////////////////////////////////////////////////
		int AddRegularExpression(RegularExprRef exp);

		///////////////////////////////////////////////////////////////////////////////
		// CreateNFANode: Creates a node ref, adds it to the internal list, and returns
		//                the dumb pointer.
		// Inputs:
		//
		// Return value:
		//  NFANode*       The requested node.
		///////////////////////////////////////////////////////////////////////////////
		virtual NFANode* CreateNFANode();


	private:
		std::vector<RegularExprRef> expressions;         //vector of expressions
		std::vector<NFANodeRef>     nodes;               //Vector of nodes in the NFA 
		NFANodeRef					beginNode;           //First node in the graph
		NFANodeRef					endNode;             //Last node in the graph
		bool                        bound;               //boolean to track if we're ready to process
		int							unknownOrdinal;      //ordinal value assigned to unknown
		OrdinalMap                  map;                 //map that holds defined expression symbols
		RegularExprTrie				trie;                //trie for faster lookups when creating ordinal
		std::vector<int>			inputOrdinals;		// Vector to hold the ordinal values

		// Vector of ordinal vectors, for MultiMatch
		VectorNoDestruct<std::vector<int> > multiInputOrdinals;
									
	};
	typedef refcnt_ptr<RegularExprEngine> RegularExprEngineRef;

} //namespace

#endif
