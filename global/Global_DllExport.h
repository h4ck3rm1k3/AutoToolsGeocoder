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

// Global_DllExport.h: Provide macros to control import/export definitions
// Currently this is only needed for building DLLs for Windows.

#ifndef INCL_Global_Dllexport_h
#define INCL_Global_Dllexport_h

#include "ImportExport.h"

#ifdef WIN32
#pragma once
#endif

#ifdef GLOBAL_EXPORTS
	#define GLOBAL_CLASS EXPORT_CLASSXXX
	#define GLOBAL_FUNC EXPORT_FUNCXXX
	#define GLOBAL_DATA EXPORT_DATAXXX
#elif defined(GLOBAL_STATIC)
	// No import or export
	#define GLOBAL_CLASS
	#define GLOBAL_FUNC
	#define GLOBAL_DATA
#else
	#define GLOBAL_CLASS IMPORT_CLASS
	#define GLOBAL_FUNC IMPORT_FUNC
	#define GLOBAL_DATA IMPORT_DATA
#endif


#endif
