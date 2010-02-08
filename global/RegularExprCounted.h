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

// RegularExprCounted.h: Implementation of ? in regular expressions

#ifndef INCL_REGULAREXPRCounted_H
#define INCL_REGULAREXPRCounted_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "RegularExpr.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {
	class RegularExprCounted : public RegularExpr {
		
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		RegularExprCounted(
			RegularExprRef expression_,
			int minCount_,
			int maxCount_
		) : 
			expression(expression_),
			minCount(minCount_),
			maxCount(maxCount_)
		{}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprCounted() {}
		
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
		virtual bool GenerateNFA(
			RegularExprEngineBase* engine, 
			NFANode* startNode, 
			NFANode* endNode
		);

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
		virtual bool SetupBind(OrdinalMap&, ListenerRef);

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
		virtual bool Bind(OrdinalMap&, ListenerRef);

		///////////////////////////////////////////////////////////////////////////////
		// Dump: output this regular expression to the ostream provided.
		// Inputs:
		//  std::ostream&	os				stream to output into
		//  TsString		prefix			string to prepend to expression info
		//
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		virtual void Dump(std::ostream& os, TsString prefix);

		///////////////////////////////////////////////////////////////////////////////
		// Is this regular expression a simple sequence (sequence of literals)?
		///////////////////////////////////////////////////////////////////////////////
		virtual bool SimpleSequence() { return false; }

		///////////////////////////////////////////////////////////////////////////////
		// Is any regular expression in this tree an action?
		///////////////////////////////////////////////////////////////////////////////
		virtual bool FindAction() { return expression->FindAction(); }


	protected:
		// The sub-expression
		RegularExprRef expression;

		// Min, max counts
		int minCount;
		int maxCount;
	};
} //namespace indexer

#endif
