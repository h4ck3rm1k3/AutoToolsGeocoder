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

// Geocoder_DllExport.h


#ifndef INCL_GeocoderPortfolioExplorer_DllExport
#define INCL_GeocoderPortfolioExplorer_DllExport

#include "../global/ImportExport.h"

#ifdef WIN32
#pragma once
#endif

#ifdef GEOCODER_EXPORTS
    #define GEOCODER_CLASS EXPORT_CLASSXXX
    #define GEOCODER_FUNC EXPORT_FUNCXXX
    #define GEOCODER_DATA EXPORT_DATAXXX
#elif defined(GEOCODER_STATIC)
    // No import or export
    #define GEOCODER_CLASS
    #define GEOCODER_FUNC
    #define GEOCODER_DATA
#else
    #define GEOCODER_CLASS IMPORT_CLASS
    #define GEOCODER_FUNC IMPORT_FUNC
    #define GEOCODER_DATA IMPORT_DATA
#endif


#endif
