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

// RegularExprAction.cpp: Defines the regular expression sub expression object

#include "Global_Headers.h"
#include "RegularExprEngineBase.h"
#include "RegularExprAction.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// GenerateNFA: Generate NFA node(s) and arc(s) applicable to this expression 
	//              and attach them to the provided start and end nodes.
	// Inputs:
	//  NFANodeRef		startNode		first node in provided subgraph
	//  NFANodeRef		endNode			last node in provided subgraph
	//
	// Return value:
	//	bool		true if NFA creation succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprAction::GenerateNFA(RegularExprEngineBase* engine, NFANode* startNode, NFANode* endNode)
	{	
		assert(bound);

		// Record the number of arcs out of the start node and into the end node.
		int startArcCount = int(startNode->outgoingArcs.size());
		int endArcCount = int(endNode->incomingArcs.size());

		//Generate the NFA piece on the subexpression, if possible
		if( !expression->GenerateNFA(engine, startNode, endNode) ) {
			return false;
		}

		// Create the actions for every new arc originating from the start node.
		{for (unsigned i = startArcCount; i < startNode->outgoingArcs.size(); i++) {
			startNode->outgoingArcs[i]->AddAction(new Action(true, actionIndex));
		}}

		// Create the actions for every new arc terminating in the end node.
		{for (unsigned i = endArcCount; i < endNode->incomingArcs.size(); i++) {
			endNode->incomingArcs[i]->AddAction(new Action(false, actionIndex));
		}}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Dump: output this regular expression to the ostream provided.
	// Inputs:
	//  std::ostream&	os				stream to output into
	//  TsString		prefix			string to prepend to expression info
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void RegularExprAction::Dump(std::ostream& os, TsString prefix)
	{
		os << prefix.c_str() << actionIndex << "(";
		os << '\n';
		expression->Dump(os, prefix + '-');
		os << prefix.c_str() << ")" << '\n';
	}
} //namespace
