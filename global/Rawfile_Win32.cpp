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

// RawFile_win32.cpp: Implementation of OS-specifc unbuffered I/O for Win32

#include "Global_Headers.h"
#include "RawFile.h"
#include "RawFile_win32.h"
#include "Exception.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Constructor: create a file-buffer with no file opened
	///////////////////////////////////////////////////////////////////////////////
	RawFileImpWin32::RawFileImpWin32() :
		fileHandle(INVALID_HANDLE_VALUE)
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		memGranularity = info.dwAllocationGranularity;
		// This granularity works for all disks
		diskGranularity = info.dwPageSize;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Will flush and close any open file.
	///////////////////////////////////////////////////////////////////////////////
	RawFileImpWin32::~RawFileImpWin32()
	{
		// Must call Close() here!
		Close();
	}

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
	bool RawFileImpWin32::Open(
		OpenFlag openFlag_,
		const TsString& filepath_,
		int bufferSize_,
		bool useOS_Buffer_
	) {
		// close any open file.
		Close();

		openFlag = openFlag_;
		filepath = filepath_;
		bufferSize = bufferSize_;
		useOS_Buffer = useOS_Buffer_;
		haveError = false;
		position = 0;
		lastPosition = 0;

		// Get the full path name
		{
			char buf[1024];
			buf[0] = 0;
			char* dummy;
			GetFullPathName(filepath.c_str(), sizeof(buf), buf, &dummy);
			fullpath = buf;
		}

		// Allocate the buffer
		if (bufferSize == 0) {
			// Use default sizes.
			bufferSize = useOS_Buffer ? CachedDefaultBufferSize : NonCachedDefaultBufferSize;
		}
		bufferSize = RoundUp(bufferSize, memGranularity);
		buffer = (char*)VirtualAlloc(NULL, bufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (buffer == 0) {
			haveError = true;
			return false;
		}

		DWORD win32FileAccess;
		DWORD win32ShareMode;
		DWORD win32CreationDisposition;
		DWORD win32FileFlags;

		switch (openFlag) {
		case ReadOnly:
			win32FileAccess = GENERIC_READ;
			win32ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
			win32CreationDisposition = OPEN_EXISTING;
			break;
		case CreateAndWrite:
			win32FileAccess = GENERIC_WRITE;
			win32ShareMode = FILE_SHARE_READ;
			win32CreationDisposition = CREATE_ALWAYS;
			break;
		case Append:
			win32FileAccess = GENERIC_READ | GENERIC_WRITE;
			win32ShareMode = FILE_SHARE_READ;
			win32CreationDisposition = OPEN_ALWAYS;
			break;
		default:
			assert(0);
			return false;
		}

		if (useOS_Buffer) {
			win32FileFlags = 0;
		} else {
			win32FileFlags = FILE_FLAG_NO_BUFFERING;
		}

		fileHandle = CreateFile(
			filepath.c_str(),
			win32FileAccess,
			win32ShareMode,
			NULL,									// no security
			win32CreationDisposition,
			win32FileFlags,
			NULL									// no template file
		);
		
		if (fileHandle == INVALID_HANDLE_VALUE) {
			VirtualFree(buffer, 0, MEM_RELEASE);
			haveError = true;
			buffer = 0;
		}
		return fileHandle != INVALID_HANDLE_VALUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Close a file.
	// Return Value:
	//	bool		true on success, false on failure.
	///////////////////////////////////////////////////////////////////////////////
	bool RawFileImpWin32::Close()
	{
		if (!IsOpen()) {
			// already closed
			return true;
		}
		bool retval = (CloseHandle(fileHandle) != 0);
		fileHandle = INVALID_HANDLE_VALUE;
		if (buffer != 0) {
			VirtualFree(buffer, 0, MEM_RELEASE);
			buffer = 0;
		}
		return retval;
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
	unsigned int RawFileImpWin32::Read(
		unsigned int size,
		char* bufferReturn
	) {
		char* origBufferReturn = bufferReturn;
		// Break the block into chunks so Windows doesn't get unhappy.
		// If you don't then you can get an "out of resource" error.
		while (size != 0) {
			unsigned int bytesToRead = GLOBAL_MIN(size, (unsigned int)MaxIOBlock);
			DWORD bytesRead = 0;
			if (!ReadFile(fileHandle, bufferReturn, bytesToRead, &bytesRead, NULL)) {
				// Read error
				haveError = true;
				break;
			}
			size -= bytesRead;
			bufferReturn += bytesRead;
			if (bytesRead != bytesToRead) {
				// End of file.  Not as much data as we expected.
				break;
			}
		}
		lastPosition = position;
		position += bufferReturn - origBufferReturn;
		return int(bufferReturn - origBufferReturn);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Write to a file.  An assertion will occur if file was opened for reading.
	// Inputs:
	//	unsigned int	size			Number of bytes to write to the file.
	//	const char*		bufferToWrite	Buffer containing data to write to the file.
	// Exceptions:
	//	ErrorException thrown on failure.
	///////////////////////////////////////////////////////////////////////////////
	void RawFileImpWin32::Write(
		unsigned int size,
		const char* bufferToWrite
	) {
		const char* origBufferToWrite = bufferToWrite;
		// Break the block into chunks so Windows doesn't get unhappy.
		// If you don't then you can get an "out of resource" error.
		while (size != 0) {
			unsigned int bytesToWrite = GLOBAL_MIN(size, (unsigned int)MaxIOBlock);
			DWORD bytesWritten = 0;
			if (!WriteFile(fileHandle, bufferToWrite, bytesToWrite, &bytesWritten, NULL)) {
				// Write error
				haveError = true;
				break;
			}
			size -= bytesWritten;
			bufferToWrite += bytesWritten;
			if (bytesWritten != bytesToWrite) {
				// Could not write all data to the file.  Out of disk space?
				haveError = true;
				break;
			}
		}
		lastPosition = position;
		position += bufferToWrite - origBufferToWrite;

		if (size != 0) {
			ThrowErrorException(GetLastError());
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Seek: Set the position from which the next read will occur.
	// This is only valid for ReadOnly.  Seeks must be block-aligned.
	// Use RoundDownAlignment() to ensure block-alignment.
	// Inputs:
	//	__int64		position		New position in the file
	// Return Value:
	//	bool		true on success, false on failure
	///////////////////////////////////////////////////////////////////////////////
	bool RawFileImpWin32::Seek(__int64 position_)
	{
		// Update cached file position.
		lastPosition = position_;
		position = position_;
		// Extract 32-bit low/high values
		unsigned int posLow;
		int posHigh;
		posLow = (unsigned int)position;
		posHigh = (int)(position >> 32);
		LONG longPosHigh = posHigh;
		DWORD tmp = SetFilePointer(fileHandle, (LONG)posLow, &longPosHigh, FILE_BEGIN);
		return tmp == (DWORD)posLow;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the size of the physical file.  The file must be open.
	// Return value:
	//	__int64		The size of the file in bytes, or -1 on error;
	///////////////////////////////////////////////////////////////////////////////
	__int64 RawFileImpWin32::GetFileSize() const
	{
		assert(IsOpen());
		if (!IsOpen()) {
			return -1;
		} else {
			DWORD sizeHigh;
			DWORD sizeLow = ::GetFileSize(fileHandle, &sizeHigh);
			if (sizeLow == 0xFFFFFFFF) {
				if (GetLastError() != NO_ERROR) {
					return -1;
				}
			}
			return ((__int64)sizeHigh << 32) | sizeLow;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Write a non-aligned/non-granular block, and close the file.  This must be
	// done as a single operation, because otherwise non-buffered I/O will fail.
	// Inputs:
	//	unsigned int	size			Number of bytes to write to the file.
	//	const char*		bufferToWrite	Buffer containing data to write to the file.
	// Exceptions:
	//	throws ErrorException on file I/O error.
	///////////////////////////////////////////////////////////////////////////////
	void RawFileImpWin32::WriteAndClose(
		unsigned int size,
		const char* bufferToWrite
	) {
		assert(IsOpen());

		bool alreadyHaveError = haveError;

		DWORD bytesWritten = 0;

		TsString propagateMessage;
		bool haveWriteError = false;

		// Write the granular portion of the data using fast method.
		unsigned int granularSize = RoundDownGranularity(size);
		if (granularSize != 0) {
			// Catch an exception thrown here, to ensure that the 
			// close process continues to completion.
			try {
				Write(granularSize, bufferToWrite);
				size -= granularSize;
				bufferToWrite += granularSize;
			} catch (ErrorException ex) {
				// Could not write the file.  Close it and rethrow exception.
				propagateMessage = ex.message;
				haveWriteError = true;
				size = 0;
			}
		}
		unsigned int totalBytesWritten = bytesWritten;

		// Remember where we are.
		__int64 savePosition = position;

		// Save the error value because VirtualFree resets it.
		DWORD errorCode;

		// Close the non-buffered file-handle.
		if (CloseHandle(fileHandle)) {
			bytesWritten = 0;
			if (size != 0) {
				// Open a buffered file-handle
				fileHandle = CreateFile(
					filepath.c_str(),
					GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,					// no security
					OPEN_EXISTING,
					0,						// no file flags
					NULL					// no template file
				);
				if (fileHandle != INVALID_HANDLE_VALUE) {
					unsigned int low;
					int high;
					low = (unsigned int)savePosition;
					high = (int)(savePosition >> 32);

					// Position to the file end.
					SetFilePointer(fileHandle, low, (long*)&high, FILE_BEGIN);

					// Write the remaining data.
					if (!WriteFile(fileHandle, bufferToWrite, size, &bytesWritten, NULL)) {
						// Write error
						haveError = true;
						errorCode = GetLastError();
					}
					if (!CloseHandle(fileHandle)) {
						haveError = true;
						errorCode = GetLastError();
					}
				} else {
					haveError = true;
					errorCode = GetLastError();
				}
			}
		} else {
			haveError = true;
			errorCode = GetLastError();
		}

		fileHandle = INVALID_HANDLE_VALUE;

		// Free the buffer
		if (buffer != 0) {
			VirtualFree(buffer, 0, MEM_RELEASE);
			buffer = 0;
		}

		totalBytesWritten += bytesWritten;

		// throw exceptions on error, but do not re-throw exception if already thrown.
		if (!alreadyHaveError) {
			if (haveWriteError) {
				throw ErrorException(propagateMessage);
			} else if (haveError) {
				ThrowErrorException(errorCode);
			}
		}
	}

	
	///////////////////////////////////////////////////////////////////////////////
	// Gets the most recent error message and throws an exception.
	///////////////////////////////////////////////////////////////////////////////
	void RawFileImpWin32::ThrowErrorException(DWORD errorCode)
	{
		char msg[2048];
		msg[0] = 0;
		FormatMessage(  
			FORMAT_MESSAGE_FROM_SYSTEM,
			0,					// lpSource
			errorCode,
			0,					// dwLanguageId
			msg,
			sizeof(msg),
			0					// Arguments
		);
		throw ErrorException(
			TsString("Error writing file '") + 
			fullpath +
			"'.  Error from system is: " +
			msg
		);
	}

}

