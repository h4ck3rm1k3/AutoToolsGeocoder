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

// RegularExprAction.h: A wrapper class so we can determine top level sub expressions by their type

#ifndef INCL_RegularExprAction_H
#define INCL_RegularExprAction_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "RegularExpr.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {
	class RegularExprAction : public RegularExpr {
		
		friend class RegularExprParser;

	public:

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		RegularExprAction(RegularExprRef expr_, int actionIndex_) : 
			expression(expr_), actionIndex(actionIndex_) 
		{
			assert(expression != 0);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprAction() {};
		
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
		virtual bool GenerateNFA(RegularExprEngineBase* engine, NFANode* startNode, NFANode* endNode);

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
		virtual bool SetupBind(OrdinalMap& map, ListenerRef listener) 
		{ return expression->SetupBind(map, listener); }

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
		virtual bool Bind(OrdinalMap& map, ListenerRef listener)
		{ bound = expression->Bind(map, listener); return bound; }

		///////////////////////////////////////////////////////////////////////////////
		// GetSubExprNumber: Return the number of this subexpression (position in
		//                   the expression).
		// Inputs:
		//
		// Return value:
		//	int				The subexpression number
		///////////////////////////////////////////////////////////////////////////////
		int  GetSubExprNumber() { return actionIndex; }

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
		virtual bool SimpleSequence() { return expression->SimpleSequence(); }

		///////////////////////////////////////////////////////////////////////////////
		// Is this regular expression an action?
		///////////////////////////////////////////////////////////////////////////////
		virtual bool IsAction() { return true; }

	protected:
		RegularExprRef expression;
		int            actionIndex;    //The number(position) of this subexpression in the regular expression
	};
} //namespace indexer

#endif
