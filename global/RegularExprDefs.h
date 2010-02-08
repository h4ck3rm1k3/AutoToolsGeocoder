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

// RegularExprDefs.h: Some constants used in regular expressions

#ifndef INCL_REGULAREXPRDEFS_H
#define INCL_REGULAREXPRDEFS_H

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "StringToIntMap.h"
#include "TsString.h"
#include "Trie.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class RegularExprDefs {
	public:
		//typedef for the ordinal map container
		typedef StringToIntMap OrdinalMap;
		enum { ORDINAL_EOF = -1 };
	};

} //namespace

#endif
