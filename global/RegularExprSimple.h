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

// RegularExprSimple.h: Simple textual regular expression object.

#ifndef INCL_REGULAREXPRSIMPLE_H
#define INCL_REGULAREXPRSIMPLE_H

#if _MSC_VER >= 1000
	#pragma once
#endif



#include <vector>

#include "RegularExprEngine.h"
#include "Listener.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {
		
	class RegularExprSimple : public VRefCount {
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////////
		RegularExprSimple() {}

		///////////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RegularExprSimple() {};

		///////////////////////////////////////////////////////////////////////////////
		// Bind the regular expression
		// Inputs:
		//	const TsString&		regularExprStr		String representation of the regular expr.
		//	ListenerRef			listener			Will receive any error messages.
		// Return value:
		//	bool			true is successfully bound, false o/w
		///////////////////////////////////////////////////////////////////////////////
		bool Bind(
			const TsString& regularExprStr,
			ListenerRef listener
		);

		///////////////////////////////////////////////////////////////////////////////
		// Match: Analyze this sequence of symbols against the NFA, returning true 
		// if it matched an expression, or false if not.  The entire string must match.
		// Also returns action codes.
		// Inputs:
		//	const TsString&			str				String to match
		// Outputs:
		//	vector<ActionRef>&		actionsMatched  Information on the subexpressions matched
		// Return value:
		//	bool		true if matched, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Match(	
			const TsString& str,
			std::vector<ActionResult>& actionsMatched
		);

		///////////////////////////////////////////////////////////////////////////////
		// Match: Analyze this sequence of symbols against the NFA, returning true 
		// if it matched an expression, or false if not.  The entire string must match.
		// No action codes.
		// Inputs:
		//	const TsString&		str			String to match
		// Return value:
		//	bool		true if matched, else false
		///////////////////////////////////////////////////////////////////////////////
		bool Match(	
			const TsString& str
		);

	private:
		RegularExprEngineRef engine;			// The engine used to matching.
		std::vector<ActionResult> actions;		// Used to store action returns.
		RegularExprRef expr;					// The parsed regular expression.
	};
	typedef refcnt_ptr<RegularExprSimple> RegularExprSimpleRef;

}

#endif
