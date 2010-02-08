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
# $Rev: 52 $ 
# $Date: 2006-10-06 05:33:29 +0200 (Fri, 06 Oct 2006) $ 
*/

///////////////////////////////////////////////////////////////////////////////
// File.h: Wrapper over native file I/O
///////////////////////////////////////////////////////////////////////////////

#ifndef INCL_File_H
#define INCL_File_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "RefPtr.h"
#include "CritSec.h"

#include "Basics.h"
#include "RawFile.h"
#include <assert.h>
#include "TsString.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class AlignedAllocator;
	
	///////////////////////////////////////////////////////////////////////////////
	// File: Wrapper over native file I/O
	///////////////////////////////////////////////////////////////////////////////

	class File : public VRefCount {
		// FileFollower is given special access to this class so that it can
		// access the data not yet written to file.
		friend class FileFollower;
	public:
		
		///////////////////////////////////////////////////////////////////////////////
		// File-open flags
		///////////////////////////////////////////////////////////////////////////////
		enum OpenFlag {
			ReadOnly,			// open an existing file for reading
			CreateAndWrite,		// create a new file for writing
			Append				// append to an existing file
		};

		///////////////////////////////////////////////////////////////////////////////
		// constructor
		// Note: does not yet open the file.
		///////////////////////////////////////////////////////////////////////////////
		File();

		///////////////////////////////////////////////////////////////////////////////
		// destructor
		// Note: will also close the file.
		///////////////////////////////////////////////////////////////////////////////
		virtual ~File();

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
		bool Open(
			OpenFlag openFlag,
			const TsString& filepath,
			int bufferSize = 0,
			bool useOS_Buffer = false
		);

		///////////////////////////////////////////////////////////////////////////////
		// IsOpen: Is the file open?
		///////////////////////////////////////////////////////////////////////////////
		bool IsOpen() const { return isOpen; }

		///////////////////////////////////////////////////////////////////////////////
		// Attach rawfile for read
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void AttachRawFileForRead(RawFileRef rawFile_);

		///////////////////////////////////////////////////////////////////////////////
		// Attach rawfile for write
		// Return value:
		///////////////////////////////////////////////////////////////////////////////
		void AttachRawFileForWrite(RawFileRef rawFile_);

		///////////////////////////////////////////////////////////////////////////////
		// IsOpen: Is the file open?
		///////////////////////////////////////////////////////////////////////////////
		void SetCloseRawFileOnClose(bool closeRawFile_){ closeRawFile = closeRawFile_; }

		///////////////////////////////////////////////////////////////////////////////
		// Get the name of the open file
		// Return value:
		//	const TsString&		The name of the open file.
		// Note: This is meaningless if the file is not open.
		///////////////////////////////////////////////////////////////////////////////
		const TsString& GetFilepath() const { return filepath; }

		///////////////////////////////////////////////////////////////////////////////
		// Close a file.
		// Exceptions:
		//	TsString thrown on failure.
		///////////////////////////////////////////////////////////////////////////////
		void Close();

		///////////////////////////////////////////////////////////////////////////////
		// Read from a file.
		// Inputs:
		//	unsigned int	size			Number of bytes to read from the file.
		// Outputs:
		//	char*			bufferReturn	Will be filled with data from the file.
		// Return value:
		//	unsigned int	Number of bytes actually read from the file.  Check
		//					HaveError() to tell the difference between EOF and error.
		///////////////////////////////////////////////////////////////////////////////
		unsigned int Read(
			unsigned int size,
			char* bufferReturn
		) {
			assert(IsOpen());
			if (bufferPtr + size <= endOfBufferPtr) {
				memcpy(bufferReturn, bufferPtr, size);
				bufferPtr += size;
				return size;
			} else {
				return OutOfBufferRead(size, bufferReturn);
			}
		}

		// Unsigned data version.  Note that we cannot simply inline this
		// to the signed-char version, because some compilers (like MSVC++) don't
		// generate optimal memcpy() intrinsics through two layers of inlining.
		unsigned int Read(
			unsigned int size, 
			unsigned char* bufferReturn
		) {
			assert(IsOpen());
			if (bufferPtr + size <= endOfBufferPtr) {
				memcpy(bufferReturn, bufferPtr, size);
				bufferPtr += size;
				return size;
			} else {
				return OutOfBufferRead(size, (char*)bufferReturn);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetPosition: Get the current file position.
		// Return Value:
		//	__int64		Position in the file.
		///////////////////////////////////////////////////////////////////////////////
		__int64 GetPosition() const;

		///////////////////////////////////////////////////////////////////////////////
		// Seek: Set the position from which the next read will occur.
		// This is only valid for ReadOnly.
		// Inputs:
		//	__int64		position		New position in the file
		// Return Value:
		//	bool		true on success, false on failure
		///////////////////////////////////////////////////////////////////////////////
		bool Seek(__int64 position);

		///////////////////////////////////////////////////////////////////////////////
		// PutbackChar: Put back a single character only
		// This is only valid for Read only.
		// Inputs:
		//	char		putbackChar		character to put back
		// Return Value:
		//	bool		true on success, false on failure
		///////////////////////////////////////////////////////////////////////////////
		void PutbackChar(char putbackChar)
		{
			//Only valid for reading
			assert(IsOpen() && mode == Reading);
			//Must have space to put one back
			assert(bufferPtr != buffer);
			bufferPtr--;
			*bufferPtr = putbackChar;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Write to a file.  An assertion will occur if file was opened for reading.
		// Inputs:
		//	unsigned int	size			Number of bytes to write to the file.
		//	const char*		bufferToWrite	Buffer containing data to write to the file.
		// Exceptions:
		//	TsString thrown on failure.
		///////////////////////////////////////////////////////////////////////////////
		void Write(
			unsigned int size,
			const char* bufferToWrite
		) {
			assert(IsOpen() && mode == Writing);
			if (bufferPtr + size <= endOfBufferPtr) {
				memcpy(bufferPtr, bufferToWrite, size);
				bufferPtr += size;
			} else {
				OutOfBufferWrite(size, bufferToWrite);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Unsigned data version.  Note that we cannot simply inline this
		// to the signed-char version, because some compilers (like MSVC++) don't
		// generate optimal memcpy() intrinsics through two layers of inlining.
		///////////////////////////////////////////////////////////////////////////////
		void Write(
			unsigned int size, 
			const unsigned char* bufferToWrite
		) { 
			assert(IsOpen() && mode == Writing);
			if (bufferPtr + size <= endOfBufferPtr) {
				memcpy(bufferPtr, bufferToWrite, size);
				bufferPtr += size;
			} else {
				OutOfBufferWrite(size, (const char*)bufferToWrite);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Method to flush pending writes.  This may write extra "padding" on the
		// end of the file to satisfy granularity requirements of the filesystem.
		// Exceptions:
		//	TsString thrown on failure.
		///////////////////////////////////////////////////////////////////////////////
		void Flush();

		///////////////////////////////////////////////////////////////////////////////
		// Get the size of the physical file.  The file must be open.
		// Return value:
		//	__int64		The size of the file in bytes, or -1 on error.
		///////////////////////////////////////////////////////////////////////////////
		__int64 GetFileSize() const {
			return rawFile->GetFileSize();
		}

		///////////////////////////////////////////////////////////////////////////////
		// Has an error occurred?
		// Return value:
		//	bool		true if an error has occurred since opening, false o/w
		///////////////////////////////////////////////////////////////////////////////
		bool HaveError() const { return haveError; }

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Private methods designed for access by FileFollower.
		// FileFollower should not access other private parts.
		///////////////////////////////////////////////////////////////////////////////
		unsigned int FileFollower_GetBufferedDataSize() const { return int(bufferPtr - buffer); }
		const char *FileFollower_GetBuffer() const { return buffer; }
		const CritSecInfo& FileFollower_GetLock() const { return followerLock; }
		void FileFollower_GetBufferState(
			__int64& trackedFilePosition, 
			int& bufferedDataSize
		) const {
			// Can only follow files being written.
			assert(mode == Writing);
			// Note: bufferPtr is only accessed once, to ensure consistency.
			bufferedDataSize = int(bufferPtr - buffer);
			trackedFilePosition = rawFile->GetPosition() + bufferedDataSize;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Called when Read() does not have enough buffered data
		// Inputs:
		//	unsigned int	size			Number of bytes to read from the file.
		// Outputs:
		//	char*			bufferReturn	Will be filled with data from the file.
		// Return value:
		//	unsigned int	Number of bytes actually read from the file.  Check
		//					HaveError() to tell the difference between EOF and error.
		///////////////////////////////////////////////////////////////////////////////
		unsigned int OutOfBufferRead(unsigned int size, char* bufferReturn);

		///////////////////////////////////////////////////////////////////////////////
		// Called when when Write() would overflow data buffer
		// Inputs:
		//	unsigned int	size			Number of bytes to write to the file.
		//	const char*		bufferToWrite	Buffer containing data to write to the file.
		// Exceptions:
		//	TsString thrown on failure.
		///////////////////////////////////////////////////////////////////////////////
		void OutOfBufferWrite(unsigned int size, const char* bufferToWrite);

		// Reading or Writing?
		enum Mode { Reading, Writing };

		// Data set by constructor
		Mode mode;					// Reading or Writing
		TsString filepath;		// path of the file 
		bool useOS_Buffer;			// was useOS_Buffer requested?

		bool isOpen;

		// Data set when file is opened.
		unsigned bufferSize;				// size of the buffer
		char* buffer;				// the buffer
		char* endOfBufferPtr;		// points past end of available bytes during read,
									// or past end of entire buffer during write.
		__int64 blockPosition;		// file-position of the last block read/written
		char* bufferPtr;			// pointer into buffer for next byte to be 
									// read or written
		bool haveError;

		// Pointer to the "raw" file object
		RawFileRef rawFile;

		// Boolean determines whether we close rawfile object on Close()
		// True by default
		bool closeRawFile;

		// Disallow copy constructor and assignment operator
		File(const File&) {assert(0); }
		File& operator=(const File&) {assert(0); return *this;}

		// Lock object that is used to prevent simultaneous operations.
		// This is only used by "Follower" objects, for the purpose of locking
		// down the contents of the file-buffer while it readss such data.
		// This does *not* lock out all Read/Write calls -- that would be too much
		// of a performance hit.  It only locks out calls that are attempting to
		// change the file buffer status with repsect to the disk.
		CritSecInfo followerLock;
	};

	typedef refcnt_ptr<File> FileRef;
	typedef refcnt_cptr<File> FileCRef;

} // namespace

#endif
