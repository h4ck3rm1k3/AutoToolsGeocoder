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

// RegularExprNFA.cpp: Defines the regular expression + object

#include "Global_Headers.h"
#include "RegularExprNFA.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////////
	NFANode::~NFANode() 
	{
	}
	
	///////////////////////////////////////////////////////////////////////////////
	// Match: Analyze the incoming ordinals against this node's arcs.
	// Inputs:
	//  vector<int>&		ordinals		ordinals to process
	//	int					pos				current position in ordinals
	//                                      in the NFA
	// Outputs:
	//	vector<ActionRef>&  actions         vector of actions that have matched
	//  int&				endPosition		Highest ordinal position reached at the end node
	// Return value:
	//	bool		true if match succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool NFANode::Match(
		std::vector<int>& ordinals, 
		int pos, 
		std::vector<ActionResult>& actions, 
		int& endPosition
	) {
		if( outgoingArcs.size() == 0 ) {
			//end node!  If we're at EOF, we matched
			//Set the endposition return, but maintain the highest position reached
			if( pos > endPosition ) {
				endPosition = pos;
			}
			return (ordinals[pos] == ORDINAL_EOF ? true : false);
		}
		
		//Save the action vector size in case we have to backtrack from a failed path
		int savedActionSize;

		//Try matching on each outgoing arc
		std::vector<NFAArcRef>::iterator iterator;
		for (
			iterator = outgoingArcs.begin();
		    iterator != outgoingArcs.end();
			++iterator
		) {
			bool matched = false;
			int nextPos = pos;
			if( (*iterator)->ordinals == 0 ) {
				//A null arc means we automatically succeed
				matched = true;
			} else if (
				ordinals[pos] != ORDINAL_EOF && 
				(*iterator)->ordinals->IsSet(ordinals[pos]) 
			) {
				// We matched the ordinal to the arc's set.
				matched = true;
				// Bump next position because we consume this ordinal.
				nextPos++;
			}

			if (matched) {
				// Set the arc id that matched in the node
				(*iterator)->node->arcID = (*iterator)->refID;

				// Push back every action on this arc if we succeeded
				std::vector<ActionRef>::iterator actionIterator;
				savedActionSize = int(actions.size());

				for (
					actionIterator = (*iterator)->actions.begin();
					actionIterator != (*iterator)->actions.end();
					++actionIterator
				) {
					if ((*actionIterator)->beginEnd) {
						 // Push new begin action
						ActionResult result;
						result.subExpr = (*actionIterator)->subExpr;
						result.enterPosition = pos;
						actions.push_back(result);
					} else {
						// This is an end action.
						// Search back for matching being action.
						#ifndef NDEBUG
							bool found = false;
						#endif
						for (int i = int(actions.size()) - 1; i >= 0; i--) {
							if (actions[i].subExpr == (*actionIterator)->subExpr) {
								// Found matching begin
								actions[i].exitPosition = nextPos;
								#ifndef NDEBUG
									found = true;
								#endif
								break;
							}
						}
						#ifndef NDEBUG
							assert(found);
						#endif
					}
				}

				// Follow the arc.
				if( (*iterator)->node->Match(ordinals, nextPos, actions, endPosition) ) {
					return true;
				} else {
					//resize actions!
					actions.resize(savedActionSize);
				}
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Match: Analyze the incoming ordinals against this node's arcs.
	// Inputs:
	//  VectorNoDestruct<vector<int> >&	ordinals	ordinals to process
	//	int								pos			current position in ordinals
	//												in the NFA
	// Outputs:
	//	vector<ActionRef>&  actions         vector of actions that have matched
	//  int&				endPosition		Highest ordinal position reached at the end node
	// Return value:
	//	bool		true if match succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool NFANode::MultiMatch(
		VectorNoDestruct<std::vector<int> >& ordinals, 
		int pos, 
		std::vector<ActionResult>& actions,
		int& endPosition
	) {
		if( outgoingArcs.size() == 0 ) {
			//end node!  If we're at EOF, we matched
			//Set the endposition return, but maintain the highest position reached
			if( pos > endPosition ) {
				endPosition = pos;
			}
			return (ordinals[pos][0] == ORDINAL_EOF ? true : false);
		}
		
		// Loop over each multi-ordinal at this position
		std::vector<int> &tokenOrdinals = ordinals[pos];

		for (unsigned int ordinalIdx = 0; ordinalIdx != tokenOrdinals.size(); ordinalIdx++) {
			//Try matching on each outgoing arc
			int thisOrdinal = tokenOrdinals[ordinalIdx];
			for (
				std::vector<NFAArcRef>::iterator iterator = outgoingArcs.begin();
				iterator != outgoingArcs.end();
				++iterator
			) {
				bool matched = false;
				int nextPos = pos;
				if( (*iterator)->ordinals == 0 ) {
					//A null arc means we automatically succeed
					matched = true;
				} else if (
					thisOrdinal != ORDINAL_EOF && 
					(*iterator)->ordinals->IsSet(thisOrdinal) 
				) {
					// We matched the ordinal to the arc's set.
					matched = true;
					// Bump next position because we consume this ordinal.
					nextPos++;
				}

				if (matched) {
					// Set the arc id that matched in the node
					(*iterator)->node->arcID = (*iterator)->refID;

					//Save the action vector size in case we have to backtrack from a failed path
					int savedActionSize = int(actions.size());

					// Push back every action on this arc if we succeeded
					for (
						std::vector<ActionRef>::iterator actionIterator = (*iterator)->actions.begin();
						actionIterator != (*iterator)->actions.end();
						++actionIterator
					) {
						if ((*actionIterator)->beginEnd) {
							 // Push new begin action
							ActionResult result;
							result.subExpr = (*actionIterator)->subExpr;
							result.enterPosition = pos;
							actions.push_back(result);
						} else {
							// This is an end action.
							// Search back for matching being action.
							#ifndef NDEBUG
								bool found = false;
							#endif
							for (int i = int(actions.size()) - 1; i >= 0; i--) {
								if (actions[i].subExpr == (*actionIterator)->subExpr) {
									// Found matching begin
									actions[i].exitPosition = nextPos;
									#ifndef NDEBUG
										found = true;
									#endif
									break;
								}
							}
							#ifndef NDEBUG
								assert(found);
							#endif
						}
					}

					// Follow the arc.
					if( (*iterator)->node->MultiMatch(ordinals, nextPos, actions, endPosition) ) {
						return true;
					} else {
						//resize actions!
						actions.resize(savedActionSize);
					}
				}
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	// CheckForNullCycle: Make sure this node doesn't contain any cycles that 
	//  can return to it.
	// Inputs:
	//  NFAArcRef			arc				the arc in question
	//                                      in the NFA
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	bool NFANode::CheckForNullCycle(std::set<NFANode*>& nodesVisited)
	{
		if( nodesVisited.find(this) != nodesVisited.end() ) {
			//been here before...
			return false;
		}

		nodesVisited.insert(this);
		//Try every outgoing null arc
		std::vector<NFAArcRef>::iterator iterator;
		for(iterator = outgoingArcs.begin();
		    iterator != outgoingArcs.end();
			iterator++
		) {
			if( (*iterator)->ordinals == 0 ) {
				 return (*iterator)->node->CheckForNullCycle(nodesVisited);
			}
	
		}
		
		nodesVisited.erase(this);
		return true;
	}

        void NFANode::AttachOutgoingArc( NFAArcRef arc) {
                outgoingArcs.push_back(arc);
        }

} //namespace
