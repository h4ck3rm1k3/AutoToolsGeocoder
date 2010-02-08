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

// RegularExprLiteral.cpp: Defines the regular expression literal object

#include "Global_Headers.h"
#include "RegularExprEngineBase.h"
#include "RegularExprWildcard.h"
#include "BitSet.h"

#include <assert.h>

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
	bool RegularExprWildcard::GenerateNFA(RegularExprEngineBase* engine, NFANode* startNode, NFANode* endNode)
	{
		assert(bound);
		//Create an arc to hold our ordinal.  Arcs take a bitset vector 
		NFAArcRef arc = new NFAArc(ordinals);
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
	bool RegularExprWildcard::SetupBind(OrdinalMap& map, ListenerRef listener)
	{
		//Nothing to do here... the unknown ordinal is setup in the engine
		return true;
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
	bool RegularExprWildcard::Bind(OrdinalMap& map, ListenerRef listener)
	{
		//Okay, the wildcard is the set of all ordinals.  So create the bitset and simply Not() it to
		//all 1's.
		//Make the ordinal bitset 1 greater than map size... the last value is
		//for unknown
		ordinals = new Bitset(int(map.size() + 1));
		ordinals->Not();
		return (bound = true);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Dump: output this regular expression to the ostream provided.
	// Inputs:
	//  ostream&		os				stream to output into
	//  TsString		prefix			string to prepend to expression info
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void RegularExprWildcard::Dump(std::ostream& os, TsString prefix)
	{
		os << prefix.c_str() << "." << '\n';
	}

} //namespace
