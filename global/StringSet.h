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
# $Rev: 52 $ 
# $Date: 2006-10-06 05:33:29 +0200 (Fri, 06 Oct 2006) $ 
*/

// JH_StringSet.h: An object that wraps std::map<TsString, TsString> in a
// non-inline interface, so as to make it DLL safe.
//
// Some background: std::map<> inherits from std::_Tree, which uses a static
// data member _Nil.  std::_Tree::iterator tests against _Nil in many locations.
// Since _Nil is static, each module gets its own copy.  So a std::map<> created
// in one module will crash in another due to testing against the wrong _Nil.

#ifndef JH_StringSet_H
#define JH_StringSet_H

#include <iterator>
#include "TsString.h"
#include "RefPtr.h"
#include "Global_DllExport.h"
#include <set>

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

namespace PortfolioExplorer {
	
	class StringSet : public std::set<TsString>, public VRefCount 
	{
	};
	typedef refcnt_ptr<StringSet> StringSetRef;
}

#endif

