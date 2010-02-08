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
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

// Filesys.cpp : Implementation portable filesystem functions.

#include <stdio.h>
#include "Global_Headers.h"
#include "Utility.h"
#include "Filesys.h"
#include <algorithm>
#include "CritSec.h"
//#include "ace/SPIPE_Addr.h"
#include "Brand.h"

#if defined(WIN32) || defined(_WIN32)
//	#include <afxwin.h>
	#include <direct.h>
	#include <io.h>
#elif defined(UNIX)
	#include <glob.h>
	#include <unistd.h>
#endif

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Return name decorated via ACE as pipe name
	// Inputs:
	//  TsString	pipeName	undecorated name for pipe
	// Return Value:
	//  TsString	pipeName	pipe name as decorated by ace
	///////////////////////////////////////////////////////////////////////////////
/*	TsString FileSys::GetACEPipename(const TsString& name)
	{
		ACE_SPIPE_Addr tempAddr(name.c_str());
		return tempAddr.get_path_name();
	}*/

	///////////////////////////////////////////////////////////////////////////////
	// Replace path separators according to the platform standard.
	///////////////////////////////////////////////////////////////////////////////
	TsString FileSys::StandardizePathSeparators(const TsString& path)
	{
		TsString retval;
		for (unsigned i = 0; i < path.size(); i++) {
			if (path[i] == '\\' || path[i] == '/') {
				retval += GetPathSeparatorChar();
			} else {
				retval += path[i];
			}
		}
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the full path of the file, interpreted relative to the current
	// working directory if necessary
	///////////////////////////////////////////////////////////////////////////////
	TsString FileSys::GetAbsoluteFilePath(const TsString& relativePath_)
	{
		TsString relativePath = StandardizePathSeparators(relativePath_);

		char buf[1024];
		strcpy(buf, ".");
#if defined(UNIX)
		getcwd(buf, sizeof(buf));
#else
		_getcwd(buf, sizeof(buf));
#endif
		TsString cwd = buf;

		if (relativePath.empty()) {
			// No filename?
			return cwd;
		}

		if (relativePath[0] == GetPathSeparatorChar()) {
			// Already an absolute path
			return relativePath;
		}

		#ifdef WIN32
		if (relativePath.size() > 2 && relativePath[1] == ':') {
				if (relativePath[2] == '/' || relativePath[2] == '\\') {
					// Already an absolute path
					return relativePath;
				} else {
					// Relative path on some other drive.  Blech!
					// TODO: Deal with this case...
					// For now we punt...
					return relativePath;
				}
			}
		#endif

		return cwd + GetPathSeparatorChar() + relativePath;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Load a string from a file, in text mode.
	// Inputs:
	//	const TsString&		filename			Name of the file to read.
	// Outputs:
	//	TsString&			fileInputAsString	Returned string
	//	TsString&			errorMessage		Error message if return value is false.
	// Return value:
	//	bool		true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool FileSys::ReadFileIntoString(
		const TsString& filename,
		TsString& fileInputAsString,
		TsString& errorMessage
	) {
		FILE* fp = fopen(filename.c_str(), "r");
		if (fp == 0) {
			errorMessage = "Cannot open file: " + filename;
			return false;
		}

		fileInputAsString = "";
		char buf[1025];
		size_t n;
		while ((n = fread(buf, 1, sizeof(buf)-1, fp)) != 0) {
			buf[n] = 0;
			fileInputAsString += buf;
		}
		fclose(fp);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Write a string to a file, in text mode.
	// Inputs:
	//	const TsString&		filename			Name of the file to write.
	//	const TsString&		fileOuputAsString	String to write
	// Outputs:
	//	TsString&			errorMessage		Error message if return value is false.
	// Return value:
	//	bool		true on success, false on error.
	///////////////////////////////////////////////////////////////////////////////
	bool FileSys::WriteFileFromString(
		const TsString& filename,
		const TsString& fileOutputAsString,
		TsString& errorMessage
	) {
		FILE* fp = fopen(filename.c_str(), "w");
		if (fp == 0) {
			errorMessage = "Cannot open file: " + filename;
			return false;
		}
		fwrite(fileOutputAsString.c_str(), 1, fileOutputAsString.size(), fp);
		if (ferror(fp)) {
			errorMessage = "Cannot write file: " + filename;
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Make a unique temp pipe name
	// Return value:
	//	TsString		A new temp pipe name.
	///////////////////////////////////////////////////////////////////////////////
/*	TsString FileSys::MakeTempPipeName()
	{
		static CritSecInfo tempFileSequenceLock;
		static int tempFileSequence = 0;
		CritSec critSec(tempFileSequenceLock);
		++tempFileSequence;
		char buf[60];
		TsString format = TsString(BrandInfo::GetString(BrandInfo::TempFilePrefix)) + "%x_%x.tmp";
		sprintf(buf, format.c_str(), getpid(), tempFileSequence);
		return buf;
	}*/

#if defined(WIN32) || defined(_WIN32)
	///////////////////////////////////////////////////////////////////////////////
	// Get the path separator for the file system.
	// Return value:
	//	TsString		The path separator for this filesystem
	///////////////////////////////////////////////////////////////////////////////
	TsString FileSys::GetPathSeparator() { return "\\"; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the path separator for the file system.
	// Return value:
	//	char			The path separator for this filesystem
	///////////////////////////////////////////////////////////////////////////////
	char FileSys::GetPathSeparatorChar() { return '\\'; }

	struct Win32CmpStringNocase_ {
		bool operator()(const TsString& lhs, const TsString& rhs) const {
			return STRICMP(lhs.c_str(), rhs.c_str()) < 0;
		}
	};

	///////////////////////////////////////////////////////////////////////////////
	// Expand a string containing file wildcard(s)
	// Inputs:
	//	const TsString&	pattern		The filename with wildcard(s).
	// Return value:
	//	std::vector<TsString>	The list of matching filenames, sorted.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	FileSys::ExpandWildcards(
		const TsString& pattern_
	) {
		std::vector<TsString> retval;
		if (!pattern_.empty() && pattern_[0] == '$') {
			// This is a server-side path reference
			retval.push_back(pattern_);
			return retval;
		}
		TsString pattern = StandardizePathSeparators(pattern_);
		if (!pattern.empty()) {
			// Get the path prefix from the pattern, if there is one.
			TsString::size_type pathEnd = pattern.rfind(GetPathSeparatorChar());
			TsString prefix;
			if (pathEnd != TsString::npos) {
				prefix = pattern.substr(0, pathEnd+1);
			}

			// Enumerate the files according to the pattern
			WIN32_FIND_DATA findData;
			HANDLE h = FindFirstFile(pattern.c_str(), &findData);
			if (h != INVALID_HANDLE_VALUE) {
				do {
					if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
						retval.push_back(prefix + findData.cFileName);
					}
				} while (FindNextFile(h, &findData));
				FindClose(h);
			}
		}
		Win32CmpStringNocase_ cmp;
		std::sort(retval.begin(), retval.end(), cmp);
		if (retval.empty()) {
			retval.push_back(pattern);
		}
		return retval;
	}

		///////////////////////////////////////////////////////////////////////////////
		// Null filename
		///////////////////////////////////////////////////////////////////////////////
		TsString FileSys::GetNullFilename()
		{
			return "nul";
		}

		///////////////////////////////////////////////////////////////////////////////
		// Strip pipe identifier off front of string
		// Inputs:
		//  TsString	pipeName	string to strip
		// Return Value:
		//  TsString	pipeName	string without pipe identifier
		///////////////////////////////////////////////////////////////////////////////
/*
		TsString FileSys::StripPipePrefix(const TsString& name)
		{
			if( name.substr(0, 9) ==  "\\\\.\\pipe\\" ) {
				return name.substr(9, JHMAX((int)name.size() - 9, 0));
			}
			return name;
		}
*/

#elif defined(UNIX)

	///////////////////////////////////////////////////////////////////////////////
	// Null filename
	///////////////////////////////////////////////////////////////////////////////
	TsString FileSys::GetNullFilename()
	{
		return "/dev/null";
	}

	///////////////////////////////////////////////////////////////////////////////
	// Strip pipe identifier off front of string
	// Inputs:
	//  TsString	pipeName	string to strip
	// Return Value:
	//  TsString	pipeName	string without pipe identifier
	///////////////////////////////////////////////////////////////////////////////
  /*	static TsString FileSys::StripPipePrefix(const TsString& name)
	{
		return name;
	}
  */	
	///////////////////////////////////////////////////////////////////////////////
	// Get the path separator for the file system.
	// Return value:
	//	TsString		The path separator for this filesystem
	///////////////////////////////////////////////////////////////////////////////
	TsString FileSys::GetPathSeparator() { return "/"; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the path separator for the file system.
	// Return value:
	//	char			The path separator for this filesystem
	///////////////////////////////////////////////////////////////////////////////
	char FileSys::GetPathSeparatorChar() { return '/'; }

	///////////////////////////////////////////////////////////////////////////////
	// Expand a string containing file wildcard(s)
	// Inputs:
	//	const TsString&	pattern		The filename with wildcard(s).
	// Return value:
	//	std::vector<TsString>	The list of matching filenames, sorted.
	///////////////////////////////////////////////////////////////////////////////
	std::vector<TsString> 
	FileSys::ExpandWildcards(
		const TsString& pattern_
	) {
		std::vector<TsString> retval;

		if (!pattern_.empty() && pattern_[0] == '$') {
			// This is a server-side path reference
			retval.push_back(pattern_);
			return retval;
		}

		TsString pattern = StandardizePathSeparators(pattern_);

		glob_t globResult;
		memset(&globResult, 0, sizeof(globResult));
		int status = glob(
			pattern.c_str(), 
			GLOB_NOCHECK | GLOB_ERR,
			0,		// errfunc
			&globResult
		);

		if (status == 0) {
			for (unsigned int i = 0; i < globResult.gl_pathc; i++) {
				retval.push_back(globResult.gl_pathv[i]);
			}
		} else {
			retval.push_back(pattern);
		}
		globfree(&globResult);
		if (retval.empty()) {
			retval.push_back(pattern);
		}
		return retval;
	}

#else
	#error "Must define Filesys methods for this platform"
#endif

	///////////////////////////////////////////////////////////////////////////////
	// Check if a file exists
	///////////////////////////////////////////////////////////////////////////////
	bool FileSys::FileExists(TsString path) 
	{
#if defined(UNIX)
	  return access(path.c_str(), 4) == 0;
#else
		return _access(path.c_str(), 4) == 0; 
#endif
	}

}
