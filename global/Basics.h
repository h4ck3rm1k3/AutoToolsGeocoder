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

// Basics.h: Definitions of basic data types.

#ifndef INCL_BASICS_H
#define INCL_BASICS_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "Global_DllExport.h"

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

namespace PortfolioExplorer {

	#if defined(_WIN32) || defined(WIN32)
		typedef unsigned int RecordID;
	#else
		// For now, this works on other platforms too
		typedef unsigned int RecordID;
	#endif

	#if defined(WIN32)
		// __int64 defined already
		typedef unsigned __int64 __uint64;
	#elif defined(UNIX)
		typedef long long __int64;
		typedef unsigned long long __uint64;
		typedef unsigned int uint32;
	#endif


	template <class T> inline T T_JHMIN(T x, T y) { return x < y ? x : y; }
	inline int JHMIN(int x, int y) { return T_JHMIN(x,y); }
	inline unsigned int JHMIN(unsigned int x, unsigned int y) { return T_JHMIN(x,y); }
	inline __int64 JHMIN(__int64 x, __int64 y) { return T_JHMIN(x,y); }
	inline double JHMIN(double x, double y) { return T_JHMIN(x,y); }

	template <class T> inline T T_JHMAX(T x, T y) { return x > y ? x : y; }
	inline int JHMAX(int x, int y) { return T_JHMAX(x,y); }
	inline unsigned int JHMAX(unsigned int x, unsigned int y) { return T_JHMAX(x,y); }
	inline __int64 JHMAX(__int64 x, __int64 y) { return T_JHMAX(x,y); }
	inline double JHMAX(double x, double y) { return T_JHMAX(x,y); }

	template <class T> inline T JHABS(T x) { return (x > 0) ? x : -x; }

	///////////////////////////////////////////////////////////////////////////////
	// Round a value down to the next multiple
	///////////////////////////////////////////////////////////////////////////////
	template <class T> inline T RoundDown(T value, int multiple) {
#if defined(UNIX)
	        return value;
#else
		return value / multiple * multiple;
#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// Round a value up to the next multiple
	///////////////////////////////////////////////////////////////////////////////
	template <class T> inline T RoundUp(T value, int multiple) {
		return ((value - 1) / multiple + 1) * multiple;
	}

}

#endif
