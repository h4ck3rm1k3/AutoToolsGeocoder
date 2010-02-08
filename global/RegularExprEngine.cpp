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
# $Rev: 54 $ 
# $Date: 2006-10-07 12:53:22 +0200 (Sat, 07 Oct 2006) $ 
*/

// RegularExprEngine.cpp: Defines the regular expression engine

#include "Global_Headers.h"
#include "RegularExprEngine.h"

namespace PortfolioExplorer {

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
	bool RegularExprEngine::Match(	
		std::vector<unsigned char*>::const_iterator beginValue, 
		std::vector<unsigned char*>::const_iterator endValue, 
		int& exprMatched, 
		std::vector<ActionResult>& actionsMatched,
		int& endPosition
	) {
		assert(bound);

		// Clear out the actions matched and ordinals list
		actionsMatched.clear();
		inputOrdinals.clear();

		int ordinalVal;
		//Find each of the ordinals in our trie
		for (; beginValue != endValue; ++beginValue) {
			if( trie.Find((*beginValue), ordinalVal) ) {
				inputOrdinals.push_back(ordinalVal);
			} else {
				inputOrdinals.push_back(unknownOrdinal);
			}
		}
		//End w/ EOF!
		inputOrdinals.push_back(ORDINAL_EOF);

		endPosition = -1;
		//Now try to match!
		if( beginNode->Match(inputOrdinals, 0, actionsMatched, endPosition) ) {
			exprMatched = endNode->arcID;
			std::vector<ActionResult>::iterator it = actionsMatched.begin();
			for(; it != actionsMatched.end(); ++it) {
				if((*it).exitPosition < 0 ) {
					it = actionsMatched.erase(it);
				}
			}
			return true;
		} else {
			return false;
		}
	}

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
	bool RegularExprEngine::Match(	
		const unsigned char* str,
		int& exprMatched, 
		std::vector<ActionResult>& actionsMatched,
		int& endPosition
	) {
		// Clear out the actions matched and ordinals list
		actionsMatched.clear();
		inputOrdinals.clear();

		while (*str != 0) {
			inputOrdinals.push_back(*str++);
		}
		//End w/ EOF!
		inputOrdinals.push_back(ORDINAL_EOF);

		endPosition = -1;
		//Now try to match!
		if( beginNode->Match(inputOrdinals, 0, actionsMatched, endPosition) ) {
			exprMatched = endNode->arcID;
			std::vector<ActionResult>::iterator it = actionsMatched.begin();
			for(; it != actionsMatched.end(); ++it) {
				if((*it).exitPosition < 0 ) {
					it = actionsMatched.erase(it);
				}
			}
			return true;
		} else {
			return false;
		}
	}

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
	bool RegularExprEngine::MultiMatch(	
		const VectorNoDestruct<std::vector<unsigned char*> >& symbols,
		int& exprMatched, 
		std::vector<ActionResult>& actionsMatched,
		int& endPosition
	) {
		assert(bound);

		// Clear out the actions matched and ordinals list
		actionsMatched.clear();
		multiInputOrdinals.clear();

		int ordinalVal;

		// Find each of the ordinals in our trie
		for (unsigned i = 0; i < symbols.size(); i++) {
			const std::vector<unsigned char*>& tokenSymbols = symbols[i];
			std::vector<int> &tempOrdinals = multiInputOrdinals.UseExtraOnEnd();
			tempOrdinals.clear();
			for (unsigned j = 0; j < tokenSymbols.size(); j++) {
				if( trie.Find((tokenSymbols[j]), ordinalVal) ) {
					tempOrdinals.push_back(ordinalVal);
				} else {
					tempOrdinals.push_back(unknownOrdinal);
				}
			}
		}
		//End w/ EOF!
		{
			std::vector<int> &tempOrdinals = multiInputOrdinals.UseExtraOnEnd();
			tempOrdinals.clear();
			tempOrdinals.push_back(ORDINAL_EOF);
		}

		endPosition = -1;
		//Now try to match!
		if( beginNode->MultiMatch(multiInputOrdinals, 0, actionsMatched, endPosition) ) {
			exprMatched = endNode->arcID;
			std::vector<ActionResult>::iterator it = actionsMatched.begin();
			for(; it != actionsMatched.end(); ++it) {
				if((*it).exitPosition < 0 ) {
					it = actionsMatched.erase(it);
				}
			}
			return true;
		} else {
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// AddRegularExpression: Add a regular expression to the engine.
	// Inputs:
	//  RegularExprRef	exp				The expression
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	int RegularExprEngine::AddRegularExpression(RegularExprRef exp)
	{
		assert(!bound);
		expressions.push_back(exp);
		return int(expressions.size()) - 1;
	}

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
	bool RegularExprEngine::AddExpressionSymbol(TsString symbol, ListenerRef listener)
	{
		std::pair<OrdinalMap::iterator, bool> insertPair = 
			map.insert(OrdinalMap::value_type(symbol, int(map.size())));
		//Adding the same symbol twice will simply fail, so no problem
		return true;
	}

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
	bool RegularExprEngine::Bind(ListenerRef listener)
	{
		assert(!bound);
		//Set up the NFA

		//Create the begin and end nodes
		beginNode = CreateNFANode();
		endNode = CreateNFANode();
		
		//Declare this temporary node as a ref since we don't want to maintain it beyond
		//the NFA creation
		NFANodeRef tempEndNode;

		//Get all literals which aren't already in the map
		{for (unsigned i = 0; i < expressions.size(); i++) {
			if( !expressions[i]->SetupBind(map, listener) ) {
				return false;
			}
		}}

		//The unknown ordinal is 1 greater than the highest map ordinal
		unknownOrdinal = int(map.size());
		
		//Iterate through the expressions, binding and creating a NFA for each one in turn
		{for (unsigned i = 0; i < expressions.size(); i++) {
			if( !expressions[i]->Bind(map, listener) ) {
				return false;
			}
			// Create temporary end node for this expression
			tempEndNode = new NFANode();

			if (!expressions[i]->GenerateNFA(this, beginNode.get(), tempEndNode.get()) ) {
				return false;
			}

			//Now pull off the temporary end node, set the reference number on the incoming arcs, and attach
			//the shared end node
			std::vector<NFAArcRef>::iterator iterator;
			for (
				iterator = tempEndNode->incomingArcs.begin();
			    iterator != tempEndNode->incomingArcs.end();
				iterator++
			) {
				//Set the refID field of each arc to match this expression
				(*iterator)->refID = i;
				(*iterator)->AttachToNode(endNode.get());
			}
		}}
		
		//The NFA graph needs to be checked for null arcs which will cause
		//a stack overflow during matching.
		std::vector<NFANodeRef>::iterator nodeIterator;
		for(nodeIterator = nodes.begin(); nodeIterator != nodes.end(); nodeIterator++) {
			std::set<NFANode*> nodesVisited;
			if( !(*nodeIterator)->CheckForNullCycle(nodesVisited) ) {
				listener->Error("Regular expression allows for repetition of empty subexpressions.");
				return false;
			}
		}

		//Now create the Trie for fast lookup on matching
		OrdinalMap::iterator iterator;
		for(iterator = map.begin(); iterator != map.end(); iterator++) {
			trie.Insert((*iterator).first.c_str(), (*iterator).second);
		}
		return(bound = true);
	}

	///////////////////////////////////////////////////////////////////////////////
	// CreateNFANode: Creates a node ref, adds it to the internal list, and returns
	//                the dumb pointer.  The engine must maintain the node list
	//                for proper de-allocation.  This is the ONLY correct way to get
	//                a node-- for now...
	// Inputs:
	//
	// Return value:
	//  NFANode*       The requested node.
	///////////////////////////////////////////////////////////////////////////////
	NFANode* RegularExprEngine::CreateNFANode()
	{
		NFANodeRef node = new NFANode();
		nodes.push_back(node);
		return node.get();
	}

} //namespace
