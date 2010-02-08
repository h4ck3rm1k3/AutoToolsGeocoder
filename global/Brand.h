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

// Brand.h: Header file for support of branding

// NOTE: This header file really belongs with the jh_brand.dll projects, but
// we place it here for simplicity of access.


#ifndef INCL_BRAND_H
#define INCL_BRAND_H

#include "ImportExport.h"

// Provide macros to control import/export definitions
// Currently this is only needed for building DLLs for Windows.

#ifdef BRAND_EXPORTS
	#define BRAND_CLASS EXPORT_CLASS
	#define BRAND_FUNC EXPORT_FUNC
	#define BRAND_DATA EXPORT_DATA
#elif defined(BRAND_STATIC)
	// No import or export
	#define BRAND_CLASS
	#define BRAND_FUNC
	#define BRAND_DATA
#else
	#define BRAND_CLASS IMPORT_CLASS
	#define BRAND_FUNC IMPORT_FUNC
	#define BRAND_DATA IMPORT_DATA
#endif

namespace PortfolioExplorer {

	class BRAND_CLASS BrandInfo {
	public:
		// Branded object strings that can be returned.

		//
		// NOTE: DO NOT REORDER OR DELETE EXISTING BRAND STRINGS.
		// WE DESIRE TO MAINTAIN BACKWARDS COMPATIBILITY OF OLDER 
		// APPLICATIONS WITH NEWER JH_BRAND DLLs
		//
		enum BrandObject {
			Brand,						// "PortfolioExplorer", "MarketModels", or "SRC"
			CompanyName,
			Phone,
			Website,
			Url,
			Address1,
			Address2,
			TempFilePrefix,
			ProgramName,
			ProgramFile,
			CommandLineFile,			// filename of the command line, e.g. "cmd".
			RegistryKeyCompany,			// top-level reg key
			RegistryKeyProgram,			// second-level reg key for desktop application
			RegistryKeyDataServer,		// second-level reg key for data server
			RegistryKeyObsolete1,		// no longer used
			DataFileExtension,			// data file extension
			DataFileFilter,				// filter used for opening data files in Windows GUI (contains embedded nulls!)
			ProjectFileExtension,		// project file extension
			ProjectFileFilter,			// filter used for opening project files in Windows GUI (contains embedded nulls!)
			RegistryKeyObsolete2,		// no longer used
			Email,						// The email address to contact.
			ConversionUtility			// Filename of the old-project conversion utility, e.g. "convert"
		};

		static const char* GetString(BrandObject brandObject);
	};
}

#endif

