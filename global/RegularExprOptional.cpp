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

// RegularExprOptional.cpp: Defines the regular expression ? object

#include "Global_Headers.h"
#include "RegularExprEngineBase.h"
#include "RegularExprOptional.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// GenerateNFA: Generate NFA node(s) and arc(s) applicable to this expression 
	//              and attach them to the provided start and end nodes.
	// Inputs:
	//  NFANodeRef		startNode		first node in provided subgraph
	//  NFANodeRef		endNode			last node in provided subgraph
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprOptional::GenerateNFA(RegularExprEngineBase* engine, NFANode* startNode, NFANode* endNode)
	{

		assert(bound);

		//Create two NFA arcs for the expression: one NULL to the end state, and one
		//for the expression
		assert(bound);

		if( !expression->GenerateNFA(engine, startNode, endNode) ) {
			return false;
		}
		
		NFAArcRef arc = new NFAArc(0);
		startNode->AttachOutgoingArc(arc);
		arc->AttachToNode(endNode);

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// SetupBind: Check if this expressions ordinals are represented in the map
	//            and add them if need be.
	// Inputs:
	//  OrdinalMap&		map				map of token->ordinal
	//  ListenerRef 	listener		reports any errors encountered
	//
	// Return value:
	//	bool		true if add succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprOptional::SetupBind(OrdinalMap& map, ListenerRef listener)
	{
		return expression->SetupBind(map, listener);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Bind: Bind this expression to a map of ordinals, replacing token values with
	//       the appropriate ordinal.
	// Inputs:
	//  OrdinalMap&		map				map of token->ordinal
	//  ListenerRef 	listener		reports any errors encountered
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprOptional::Bind(OrdinalMap& map, ListenerRef listener)
	{
		bound = expression->Bind(map, listener);
		return bound;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Dump: output this regular expression to the ostream provided.
	// Inputs:
	//  ostream&		os				stream to output into
	//  TsString		prefix			string to prepend to expression info
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void RegularExprOptional::Dump(std::ostream& os, TsString prefix)
	{
		os << prefix.c_str() << "?" << '\n';
		expression->Dump(os, prefix + '-');
	}

} //namespace
