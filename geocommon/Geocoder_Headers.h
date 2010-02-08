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

// Geocoder_Headers.h
#define _CRT_SECURE_NO_DEPRECATE 1

// Need to include this first since RefPtr uses ACE mutex
//#include "ace/OS.h"

#if defined(WIN32)
#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

#include <limits.h>
#include "../global/SetAssocCache.h"
#include "../global/TsString.h"

#include "GeoUtil.h"
#include "GeoHuffman.h"
#include "GeoDataInput.h"

#if (INT_MAX != 2147483647)
#error "int must be 32 bits"
#endif

