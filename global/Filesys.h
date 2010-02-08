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

// Filesys.h: Implementation of misc file-system functions.

#ifndef INCL_FileSYS_H
#define INCL_FileSYS_H

#include <vector>
#include "TsString.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class FileSys {
	public:

		///////////////////////////////////////////////////////////////////////////////
		// Replace path separators according to the platform standard.
		///////////////////////////////////////////////////////////////////////////////
		static TsString StandardizePathSeparators(const TsString& path);

		///////////////////////////////////////////////////////////////////////////////
		// Get the path separator for the file system.
		// Return value:
		//	TsString		The path separator for this filesystem
		///////////////////////////////////////////////////////////////////////////////
		static TsString GetPathSeparator();

		///////////////////////////////////////////////////////////////////////////////
		// Get the path separator for the file system.
		// Return value:
		//	char			The path separator for this filesystem
		///////////////////////////////////////////////////////////////////////////////
		static char GetPathSeparatorChar();

		///////////////////////////////////////////////////////////////////////////////
		// Expand a string containing file wildcard(s)
		// Inputs:
		//	const TsString&	pattern		The filename with wildcard(s).
		// Return value:
		//	std::vector<TsString>	The list of matching filenames, sorted.
		///////////////////////////////////////////////////////////////////////////////
		static std::vector<TsString> ExpandWildcards(
			const TsString& pattern
		);

		///////////////////////////////////////////////////////////////////////////////
		// Get the full path of the file, interpreted relative to the current
		// working directory if necessary
		///////////////////////////////////////////////////////////////////////////////
		static TsString GetAbsoluteFilePath(const TsString& relativePath);

		///////////////////////////////////////////////////////////////////////////////
		// Read the contents of a file into a string, in text mode.
		///////////////////////////////////////////////////////////////////////////////
		static bool ReadFileIntoString(
			const TsString &filename,
			TsString &fileInputAsString,
			TsString &errorMessage
		);

		///////////////////////////////////////////////////////////////////////////////
		// Write the contents of a string to a file, in text mode.
		///////////////////////////////////////////////////////////////////////////////
		static bool WriteFileFromString(
			const TsString &filename,
			const TsString &fileOutputAsString,
			TsString       &errorMessage
		);
		///////////////////////////////////////////////////////////////////////////////
		// Check if a file exists
		///////////////////////////////////////////////////////////////////////////////
		static bool FileExists(TsString path);

		///////////////////////////////////////////////////////////////////////////////
		// Null filename
		///////////////////////////////////////////////////////////////////////////////
		static TsString GetNullFilename();

		///////////////////////////////////////////////////////////////////////////////
		// Return name decorated via ACE as pipe name
		// Inputs:
		//  TsString	pipeName	undecorated name for pipe
		// Return Value:
		//  TsString	pipeName	pipe name as decorated by ace
		///////////////////////////////////////////////////////////////////////////////
//		static TsString GetACEPipename(const TsString& name);

		///////////////////////////////////////////////////////////////////////////////
		// Strip pipe identifier off front of string
		// Inputs:
		//  TsString	pipeName	string to strip
		// Return Value:
		//  TsString	pipeName	string without pipe identifier
		///////////////////////////////////////////////////////////////////////////////
//		static TsString StripPipePrefix(const TsString& name);

		///////////////////////////////////////////////////////////////////////////////
		// Make a unique temp pipe name
		// Return value:
		//	TsString		A new temp pipe name.
		///////////////////////////////////////////////////////////////////////////////
//		static TsString MakeTempPipeName();
	};

}

#endif
