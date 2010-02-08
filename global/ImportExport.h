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
# $Rev: 49 $ 
# $Date: 2006-09-25 20:00:58 +0200 (Mon, 25 Sep 2006) $ 
*/

#ifndef INCL_IMPORT_EXPORT_H
#define INCL_IMPORT_EXPORT_H

// Provide macros to control import/export definitions
// Currently this is only needed for building DLLs for Windows.

#ifdef _MSC_VER
	#pragma once
	#define EXPORT_CLASS __declspec(dllexport)
	#define EXPORT_FUNC __declspec(dllexport)
	#define EXPORT_DATA __declspec(dllexport)
	#define IMPORT_CLASS __declspec(dllimport)
	#define IMPORT_FUNC __declspec(dllimport)
	#define IMPORT_DATA __declspec(dllimport)
#elif defined(UNIX)
	// Define this if needed for other platforms
	#define EXPORT_CLASS 
	#define EXPORT_FUNC 
	#define EXPORT_DATA 
	#define IMPORT_CLASS 
	#define IMPORT_FUNC 
	#define IMPORT_DATA 
#else
	#error "Must define import/export for other platforms"
#endif


#endif
