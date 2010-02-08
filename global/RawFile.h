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

// RawFile.h: Base class for unbuffered file-I/O written in blocks to filesystem.

#ifndef INCL_RawFile_Base_H
#define INCL_RawFile_Base_H

#include <assert.h>
#include "TsString.h"
#include "RefPtr.h"
#include "Utility.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// RawFile base class
	///////////////////////////////////////////////////////////////////////////////


	class RawFile;
	typedef refcnt_ptr<RawFile> RawFileRef;

	class RawFile : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Factory to make the proper kind of derived class
		///////////////////////////////////////////////////////////////////////////////
		static RawFileRef MakeRawFile();
		
	protected:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor: only accessible from the derived class
		///////////////////////////////////////////////////////////////////////////////
		RawFile();

	public:
		///////////////////////////////////////////////////////////////////////////////
		// Will flush and close any open file.
		// Note: to catch write errors, you should call Close() explicitly and check 
		// its return value before destructing the object
		///////////////////////////////////////////////////////////////////////////////
		virtual ~RawFile();

		///////////////////////////////////////////////////////////////////////////////
		// Read/Write modes
		///////////////////////////////////////////////////////////////////////////////
		enum OpenFlag {
			ReadOnly,			// Open an existing file for reading
			CreateAndWrite,		// Create a new file for writing
			Append				// Append to existing file
		};

		///////////////////////////////////////////////////////////////////////////////
		// GetPosition: Get the current file position.
		// Return Value:
		//	__int64		Position in the file.
		///////////////////////////////////////////////////////////////////////////////
		__int64 GetPosition() const
		{ 
			return position; 
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetLastPosition: Get the file position at the last read or write
		// Return Value:
		//	__int64		Last position in the file.
		///////////////////////////////////////////////////////////////////////////////
		__int64 GetLastPosition() const
		{ 
			return lastPosition;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Get the size of the buffer.  Only valid when the file is open.
		// Return value:
		//	unsigned int	The size of the buffer
		///////////////////////////////////////////////////////////////////////////////
		unsigned int GetBufferSize() const
		{
			assert(IsOpen());
			return bufferSize;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Get the file buffer.  Only valid when the file is open.
		// Return value:
		//	char*		The file buffer.
		// Note: Do not delete the returned pointer!!
		///////////////////////////////////////////////////////////////////////////////
		char* GetBuffer() const
		{
			assert(IsOpen());
			return buffer;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Get the granularity required for reads and writes on this file.
		// All reads and writes must be in multiples of this granularity.
		// This is only important on some OS's (NT) using non-buffered I/O.
		///////////////////////////////////////////////////////////////////////////////
		unsigned int GetGranularity() const
		{
			assert(IsOpen());
			return diskGranularity;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Is the given address properly algined for file I/O?  This is only 
		// important on some OS's (e.g. NT) using non-buffered I/O.
		// Return value:
		//	bool	true if the buffer address is suitable, false o/w.
		///////////////////////////////////////////////////////////////////////////////
		bool BufferIsAligned(const char* ptr) {
#if defined(UNIX)
		        return true;
#else
			return INT_PTR(ptr) % GetGranularity() == 0;
#endif
		}

		///////////////////////////////////////////////////////////////////////////////
		// Round down the given value to that appropriate for I/O on this file.
		// This is only important on some OS's (NT) using non-buffered I/O.
		// Inputs:
		//	unsigned int	value	The value (usually data size)
		// Return Value:
		//	unsigned int	The rounded-down size that is appropriate for this file.
		///////////////////////////////////////////////////////////////////////////////
		unsigned int RoundDownGranularity(unsigned int value) const
		{
			return RoundDown(value, GetGranularity());
		}

		///////////////////////////////////////////////////////////////////////////////
		// Round down the given value to that appropriate for I/O on this file.
		// This is only important on some OS's (NT) using non-buffered I/O.
		// Inputs:
		//	__int64		value	The value (usually a file position)
		// Return Value:
		//	__int64		The rounded-down value that is appropriate for this file.
		///////////////////////////////////////////////////////////////////////////////
		__int64 RoundDownGranularity(__int64 value) const
		{
			return RoundDown(value, GetGranularity());
		}

		///////////////////////////////////////////////////////////////////////////////
		// Round down the given value to that appropriate for I/O on this file.
		// This is only important on some OS's (NT) using non-buffered I/O.
		// Inputs:
		//	unsigned int	value	The value (usually data size)
		// Return Value:
		//	unsigned int	The rounded-down size that is appropriate for this file.
		///////////////////////////////////////////////////////////////////////////////
		unsigned int RoundUpGranularity(unsigned int value) const
		{
			return RoundUp(value, GetGranularity());
		}

		///////////////////////////////////////////////////////////////////////////////
		// Round down the given value to that appropriate for I/O on this file.
		// This is only important on some OS's (NT) using non-buffered I/O.
		// Inputs:
		//	__int64		value	The value (usually a file position)
		// Return Value:
		//	__int64		The rounded-down value that is appropriate for this file.
		///////////////////////////////////////////////////////////////////////////////
		__int64 RoundUpGranularity(__int64 value) const
		{
			return RoundUp(value, GetGranularity());
		}


		///////////////////////////////////////////////////////////////////////////////
		// Has an error occurred?
		// Return value:
		//	bool	true if an error has occurred
		///////////////////////////////////////////////////////////////////////////////
		bool HaveError() const { return haveError; }


		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////
		// Methods to be overridden by derived classes
		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////
		
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
		) = 0;

		///////////////////////////////////////////////////////////////////////////////
		// Close a file.
		// Return Value:
		//	bool		true on success, false on failure.
		///////////////////////////////////////////////////////////////////////////////
		virtual bool Close() = 0;

		///////////////////////////////////////////////////////////////////////////////
		// Is the file open?
		///////////////////////////////////////////////////////////////////////////////
		virtual bool IsOpen() const = 0;

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
		) = 0;

		///////////////////////////////////////////////////////////////////////////////
		// Seek: Set the position from which the next read will occur.
		// This is only valid for ReadOnly.
		// Seeks must be block-aligned.  Use RoundDownAlignment().
		// Inputs:
		//	__int64		position		New position in the file
		// Return Value:
		//	bool		true on success, false on failure
		///////////////////////////////////////////////////////////////////////////////
		virtual bool Seek(__int64 position) = 0;

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
		) = 0;

		///////////////////////////////////////////////////////////////////////////////
		// Get the size of the physical file.
		// Return value:
		//	__int64		The size of the file in bytes, or -1 on error;
		///////////////////////////////////////////////////////////////////////////////
		virtual __int64 GetFileSize() const = 0;

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
		) = 0;

	protected:
		// These values are set by the derived class when the file is opened.
		OpenFlag openFlag;				// Mode of opening the file.
		TsString filepath;				// Path of the opened file.
		bool haveError;					// true if errors have been encountered.
		__int64 position;				// Position of the next read or write operation.
		__int64 lastPosition;			// Position of the last read or write operation.
		char* buffer;					// The buffer (zero when file is closed)
										// This is allocated by derived class
										// and must be freed by derived class
		unsigned int bufferSize;		// Size of the buffer.

		// Set by the derived class during construction.
		unsigned int diskGranularity;	// Granularity of disk I/O
	};

} // namespace

#endif

