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

// RawFile.cpp: Base class for unbuffered OS-specific I/O

#include "Global_Headers.h"
#include "RawFile.h"
#include "Exception.h"

#if defined(WIN32)
#include "RawFile_win32.h"
#elif defined(UNIX)
#include "RawFileLinux.h"
#endif

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// RawFile; Class that is used to buffer random file input.  It is faster than
	// read() or ReadFile() for small chunks, but supports 64-bit offsets,
	// unlike FILE*.
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Factory to make the proper kind of derived class
	///////////////////////////////////////////////////////////////////////////////
	RawFileRef RawFile::MakeRawFile()
	{
#if defined(WIN32)
	return new RawFileImpWin32;
#elif defined(UNIX)
	return new RawFileLinux();
#else
	return new RawFileImpFileDes;
#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	// Constructor: only accessible from the derived class
	///////////////////////////////////////////////////////////////////////////////
	RawFile::RawFile() :
		haveError(false),
		position(0),
		lastPosition(0),
		buffer(0)
	{
	}

	///////////////////////////////////////////////////////////////////////////////
	// Will flush and close any open file.
	///////////////////////////////////////////////////////////////////////////////
	RawFile::~RawFile()
	{
		// Do not call Close() here!  It will call the already-destructed base class.
	}

}

