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

// RawFile_win32.h: Implementation of RawFile for Win32

#ifndef INCL_RawFile_WIN32_H
#define INCL_RawFile_WIN32_H

#include <assert.h>

#if !defined(WIN32) && !defined(_WIN32)
	// This header file is only for Win32.  See "RawFile.h" for details
	// on overriding RawFileBase.
	#error "This header file is only valid for Win32"
#endif


namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Win32-specific implementation of RawFile
	///////////////////////////////////////////////////////////////////////////////

	class RawFileImpWin32 : public RawFile {
	public:
		
		///////////////////////////////////////////////////////////////////////////////
		// Constructor: create a file-buffer with no file opened
		///////////////////////////////////////////////////////////////////////////////
		RawFileImpWin32();

		///////////////////////////////////////////////////////////////////////////////
		// Will flush and close any open file.
		// Note: to catch write errors, you should call Close() explicitly and check 
		// its return value before destructing the object
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RawFileImpWin32();

		///////////////////////////////////////////////////////////////////////////////
		// Open a file.
		// Inputs:
		//	OpenFlag		openFlag		File access type (read or write).
		//	const string&	filepath		File path.
		//	int				bufferSize		Desired buffer size, or DefaultBufferSize.
		//	bool			useOS_Buffer	true if you want the OS to cache data, false o/w
		// Return Value:
		//	bool		true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////////
		virtual bool Open(
			OpenFlag openFlag,
			const TsString& filepath,
			int bufferSize,
			bool useOS_Buffer
		);

		///////////////////////////////////////////////////////////////////////////////
		// Close a file.
		// Return Value:
		//	bool		true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////////
		virtual bool Close();

		///////////////////////////////////////////////////////////////////////////////
		// Is the file open?
		///////////////////////////////////////////////////////////////////////////////
		virtual bool IsOpen() const
		{
			return fileHandle != INVALID_HANDLE_VALUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Read from a file.
		// Inputs:
		//	unsigned int	size			Number of bytes to read form the file.
		// Outputs:
		//	char*			bufferReturn	Will be filled with data from the file.
		// Return value:
		//	unsigned int	Number of bytes actually read from the file.  Check
		//					HaveError() to tell the difference between EOF and error.
		///////////////////////////////////////////////////////////////////////////////
		virtual unsigned int Read(
			unsigned int size,
			char* bufferReturn
		);

		///////////////////////////////////////////////////////////////////////////////
		// Seek: Set the position from which the next read will occur.
		// This is only valid for ReadOnly.
		// Seeks must be block-aligned.  Use RoundDownAlignment().
		// Inputs:
		//	__int64		position		New position in the file
		// Return Value:
		//	bool		true on success, false on failure
		///////////////////////////////////////////////////////////////////////////////
		virtual bool Seek(__int64 position);

		///////////////////////////////////////////////////////////////////////////////
		// Write to a file.  An assertion will occur if file was opened for reading.
		// Inputs:
		//	unsigned int	size			Number of bytes to write to the file.
		//	const char*		bufferToWrite	Buffer containing data to write to the file.
		// Exceptions:
		//	ErrorException thrown on failure.
		///////////////////////////////////////////////////////////////////////////////
		virtual void Write(
			unsigned int size,
			const char* bufferToWrite
		);

		///////////////////////////////////////////////////////////////////////////////
		// Get the size of the physical file.
		// Return value:
		//	__int64		The size of the file in bytes, or -1 on error;
		///////////////////////////////////////////////////////////////////////////////
		virtual __int64 GetFileSize() const;

		///////////////////////////////////////////////////////////////////////////////
		// Write a non-aligned/non-granular block, and close the file.  This must be
		// done as a single operation, because otherwise non-buffered I/O will fail.
		// Inputs:
		//	unsigned int	size			Number of bytes to write to the file.
		//	const char*		bufferToWrite	Buffer containing data to write to the file.
		// Exceptions:
		//	throws ErrorException on file I/O error.
		///////////////////////////////////////////////////////////////////////////////
		virtual void WriteAndClose(
			unsigned int size,
			const char* bufferToWrite
		);

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Gets the most recent error message and throws an exception.
		// Inputs:
		//	DWORD		errorCode		Error code from ::GetLastError()
		///////////////////////////////////////////////////////////////////////////////
		void ThrowErrorException(DWORD errorCode);

		///////////////////////////////////////////////////////////////////////////////
		// Constants
		///////////////////////////////////////////////////////////////////////////////
		enum {
			CachedDefaultBufferSize = 8192,
			NonCachedDefaultBufferSize = 1024 * 1024,
			MaxIOBlock = 128 * 1024			// NT has trouble with big blocks
		};


		///////////////////////////////////////////////////////////////////////////////
		// OS-Specific data members
		///////////////////////////////////////////////////////////////////////////////
		HANDLE fileHandle;				// Windows file handle for serial I/O
		TsString fullpath;				// Complete path of the opened file.
		bool useOS_Buffer;				// true for OS buffering, false o/w
		unsigned int memGranularity;	// Granularity of memory allocation.
	};

} // namespace

#endif

