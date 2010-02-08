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

// RegularExprNFA.h: Defines classes for the regular expression NFA

#ifndef INCL_REGULAREXPRNFA_H
#define INCL_REGULAREXPRNFA_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include <iostream>
#include <vector>
#include <set>

#include "RegularExprEngineBase.h"
#include "BitSet.h"
#include "Listener.h"
#include "RegularExprDefs.h"
#include "VectorNoDestruct.h"

#include "Global_DllExport.h"

namespace PortfolioExplorer {

	//forward definitions
	struct  NFAArc;
	typedef refcnt_ptr<NFAArc> NFAArcRef;
	struct  Action;
	typedef refcnt_ptr<Action> ActionRef;

	class NFANode : public RegularExprDefs, public VRefCount {
	
		friend class RegularExprEngine;

	public:

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~NFANode();
		
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
		bool Match(
			std::vector<int>& ordinals, 
			int pos, 
			std::vector<ActionResult>& actions,
			int& endPosition
		);

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
		bool MultiMatch(
			VectorNoDestruct<std::vector<int> >& ordinals, 
			int pos, 
			std::vector<ActionResult>& actions,
			int& endPosition
		);

		///////////////////////////////////////////////////////////////////////////////
		// AttachOutgoingArc: Attach an arc going out from this node
		// Inputs:
		//  NFAArcRef			arc				the arc in question
		//                                      in the NFA
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void AttachOutgoingArc(NFAArcRef arc);

		///////////////////////////////////////////////////////////////////////////////
		// CheckForNullCycle: Make sure this node doesn't contain any cycles that 
		//  can return to it.
		// Inputs:
		//  NFAArcRef			arc				the arc in question
		//                                      in the NFA
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		bool CheckForNullCycle(std::set<NFANode*>&);

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor-- private so only NFAEngine can create nodes
		///////////////////////////////////////////////////////////////////////////////
		NFANode() {};

	public:
		//This seems stupid-- why have accessor functions to add arcs if we're just
		//going to leave them public?
		std::vector<NFAArcRef> incomingArcs;
		std::vector<NFAArcRef> outgoingArcs;
		int                  arcID; //ID of the last incoming arc that matched this node
	};
	typedef refcnt_ptr<NFANode> NFANodeRef;

	//Action struct
	struct Action : public RegularExprDefs, public VRefCount {
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		Action(bool beginEnd_, int subExpr_) : 
			beginEnd(beginEnd_), subExpr(subExpr_) {}
		
		bool beginEnd; //true if this action is the beginning of a subexpr, false if end
		int  subExpr;  //the subExpression number this action represents
	};
	
	//Arc struct
	struct NFAArc : public RegularExprDefs, public VRefCount {
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		NFAArc(BitsetRef ordinalSet, int ID = -1) : refID(ID), ordinals(ordinalSet) {}

		///////////////////////////////////////////////////////////////////////////////
		// AttachToNode: Attach this arc to a node
		// Inputs:
		//  NFANodeRef				node		the node in question
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void AttachToNode(NFANode* node_) {node = node_; node->incomingArcs.push_back(this);}

		///////////////////////////////////////////////////////////////////////////////
		// AddAction: Add an action to this node
		// Inputs:
		//  ActionRef				action		the action in question
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void AddAction(ActionRef action) { actions.push_back(action); }

		NFANode* node;                   //The node this arc points to
		int refID;                       //ID set at graph creation
		BitsetRef ordinals;  //set of ordinals that must match on this arc
		std::vector<ActionRef> actions;  //actions on this arc
	};

} //namespace indexer

#endif
