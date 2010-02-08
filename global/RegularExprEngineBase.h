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

// RegularExprEngineBase.h: Defines classes for the regular expression NFA

#ifndef INCL_REGULAREXPRENGINEBASE_H
#define INCL_REGULAREXPRENGINEBASE_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "Global_DllExport.h"
#include "RefPtr.h"

namespace PortfolioExplorer {
		
	//forward declaration
	class NFANode;

	class RegularExprEngineBase : public VRefCount {

	public:
		///////////////////////////////////////////////////////////////////////////////
		// CreateNFANode: Creates a node ref, adds it to the internal list, and returns
		//                the dumb pointer.
		// Inputs:
		//
		// Return value:
		//  NFANode*       The requested node.
		///////////////////////////////////////////////////////////////////////////////
		virtual NFANode* CreateNFANode() = 0;
	};

	//struct to track action results for caller
	struct ActionResult  {
		ActionResult() : exitPosition(-1) {}
		int subExpr;		// action number
		int enterPosition;	// index of start ordinal
		int exitPosition;	// index of end ordinal
	};
} //namespace

#endif
