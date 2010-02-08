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
# $Rev: 72 $ 
# $Date: 2007-02-13 23:53:21 +0100 (Tue, 13 Feb 2007) $ 
*/

// File.cpp: Wrapper over native I/O
//

#include "Global_Headers.h"
#include "File.h"
#include "Utility.h"
#include "Exception.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// File: Wrapper over native file I/O
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Constructor: create a file-buffer with no file opened
	///////////////////////////////////////////////////////////////////////////////
	File::File() :
		isOpen(false),
		buffer(0),
		bufferPtr(0),
		haveError(true),
		closeRawFile(true)
	{
	}

	///////////////////////////////////////////////////////////////////////////////
	// Will flush and close any open file.
	// Note: to catch write errors, you should call Close() explicitly and check 
	// its return value before destructing the object
	///////////////////////////////////////////////////////////////////////////////
	File::~File()
	{
		// Don't let exceptions propagate out of destructors.  See Scott
		// Meyer's "More Effective C++" for why.
		// If someone really needs to know that the file was correctly
		// closed, they should close it explicitly rather than letting the
		// destructor do it.
		try {
			Close();
		} catch (ErrorException) {
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Open a file.
	// Inputs:
	//	OpenFlag		openFlag		File opening mode
	//	const string&	filepath		File path.
	//	int				bufferSize		Desired buffer size, or DefaultBufferSize.
	//	bool			useOS_Buffer	true if you want the OS to cache data, false o/w
	// Return Value:
	//	bool		true on success, false on failure.
	///////////////////////////////////////////////////////////////////////////////
	bool File::Open(
		OpenFlag openFlag,
		const TsString& filepath_,
		int bufferSize_,
		bool useOS_Buffer_
	) {
		// Allocate a platform-specific file object
		rawFile = RawFile::MakeRawFile();

		CritSec critSec(followerLock);

		// Close previous opened file, if any.
		Close();

		// Start out with the default values for these.
		filepath = filepath_;

		RawFile::OpenFlag rawOpenFlag;

		switch (openFlag) {
		case ReadOnly:
			mode = Reading;
			rawOpenFlag = RawFile::ReadOnly;
			break;
		case CreateAndWrite:
			rawOpenFlag = RawFile::CreateAndWrite;
			mode = Writing;
			break;
		case Append:
			rawOpenFlag = RawFile::Append;
			mode = Writing;
			break;
		default:
			assert(0);
			haveError = true;
			return false;
		}
		useOS_Buffer = useOS_Buffer_;
		bufferSize = bufferSize_;


		// Open the platform-specific part.
		// This also allocates the properly-aligned buffer and sets bufferSize.
		if (
			!rawFile->Open(
				rawOpenFlag,
				filepath, 
				bufferSize, 
				useOS_Buffer
			)
		) {
			haveError = true;
			return false;
		}

		//File is open
		isOpen = true;

		// Get the buffer allocated by the raw file.
		// Note that the raw file has to allocate the buffer, because only
		// it knows how to meet the platform-specific alignment requirements.
		bufferSize = rawFile->GetBufferSize();
		buffer = rawFile->GetBuffer();

		// Initialize buffer settings
		bufferPtr = buffer;
		haveError = false;

		// For reading, endOfBufferPtr points to end of available data, which is initially empty.
		// For writing, endOfBufferPtr points to end of available buffer space.
		switch (openFlag) {
		case ReadOnly:
			// No data yet available
			endOfBufferPtr = buffer;
			break;
		case CreateAndWrite:
			endOfBufferPtr = buffer + bufferSize;
			break;
		case Append:
		{
			endOfBufferPtr = buffer + bufferSize;
			// Read the last partial block of data from the raw file.
			__int64 fileSize = rawFile->GetFileSize();
			__int64 startingOffset = rawFile->RoundDownGranularity(fileSize);
			unsigned int partialBlockSize = static_cast<unsigned int>(fileSize % rawFile->GetGranularity());
			rawFile->Seek(startingOffset);
			if (partialBlockSize != 0) {
				if (rawFile->Read(rawFile->GetGranularity(), buffer) != partialBlockSize) {
					// Not as much data as we deduced from file size, or file is unreadable.
					haveError = true;
					return false;
				}
				// Seek back to block boundary.  We'll overwrite last block on the close/flush.
				rawFile->Seek(startingOffset);
			}
			bufferPtr = buffer + partialBlockSize;
			break;
		}
		default:
			assert(0);
			haveError = true;
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Attach rawfile for read
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void File::AttachRawFileForRead(RawFileRef rawFile_) { 
		rawFile = rawFile_; 
		mode = Reading;
		//File is open
		isOpen = true;

		// Get the buffer allocated by the raw file.
		// Note that the raw file has to allocate the buffer, because only
		// it knows how to meet the platform-specific alignment requirements.
		bufferSize = rawFile->GetBufferSize();
		buffer = rawFile->GetBuffer();
		// Initialize buffer settings
		bufferPtr = buffer;
		endOfBufferPtr = buffer;
		haveError = rawFile->HaveError();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Attach rawfile for write
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void File::AttachRawFileForWrite(RawFileRef rawFile_) { 
		rawFile = rawFile_; 
		mode = Writing;
		//File is open
		isOpen = true;

		// Get the buffer allocated by the raw file.
		// Note that the raw file has to allocate the buffer, because only
		// it knows how to meet the platform-specific alignment requirements.
		bufferSize = rawFile->GetBufferSize();
		buffer = rawFile->GetBuffer();
		bufferPtr = buffer;
		endOfBufferPtr = buffer + bufferSize;
		haveError = rawFile->HaveError();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Close a file.
	// Exceptions:
	//	ErrorException thrown on failure.
	///////////////////////////////////////////////////////////////////////////////
	void File::Close()
	{
		CritSec critSec(followerLock);

		if( rawFile != 0 ) {
			if (IsOpen() && mode == Writing) {
				unsigned size = unsigned(bufferPtr - buffer);
				if (size > 0) {
					if (closeRawFile) {
						rawFile->WriteAndClose(size, buffer);
					} else {
						rawFile->Write(size, buffer);
					}
				} else {
					if( closeRawFile ) {
						rawFile->Close();
					}
				}
			} else {
				if( closeRawFile ) {
					rawFile->Close();
				}
			}
		}
		isOpen = false;
		// Do not delete the buffer!  It is owned by the raw file!
		buffer = bufferPtr = endOfBufferPtr = 0;
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
	unsigned int File::OutOfBufferRead(
		unsigned int size,
		char* bufferReturn
	) {
		CritSec critSec(followerLock);

		char* originalBufferReturn = bufferReturn;

		while (unsigned(endOfBufferPtr - bufferPtr) < size) {
			int availableSize = int(endOfBufferPtr - bufferPtr);
			// We don't have the requested data in the buffer.
			// Copy all available data
			if (availableSize != 0) {
				memcpy(bufferReturn, bufferPtr, availableSize);
				size -= availableSize;
				bufferReturn += availableSize;
				bufferPtr += availableSize;
			}

			int bytesToSkip = 0;
			if ((bufferPtr - buffer) % rawFile->GetGranularity() != 0) {
				// The buffer did NOT end on a sector boundary.  This means
				// that the file ended on a non-sector boundary during the
				// last read.  However, it may have since been extended by writes via
				// some other file object.  So, we back up to the nearest sector
				// boundary and read from there, then skip the stuff that we've already
				// seen.
				int bufferedBytes = int(bufferPtr - buffer);
				bytesToSkip = bufferedBytes % rawFile->GetGranularity();
				int seekForward = bufferedBytes - bytesToSkip;
				rawFile->Seek(rawFile->GetLastPosition() + seekForward);
			}

			// We have now emptied the buffer
			endOfBufferPtr = bufferPtr = buffer;

			// Read as many blocks directly into "bufferReturn" as we can,
			// if the alignment requirements of the raw file allow it.
			//if (bytesToSkip == 0 && rawFile->BufferIsAligned(bufferReturn)) {
			if (size >= bufferSize && bytesToSkip == 0 && rawFile->BufferIsAligned(bufferReturn)) {  
				int rawReadSize = rawFile->RoundDownGranularity(size);
				if (rawReadSize != 0) {
					int rawBytesRead = rawFile->Read(rawReadSize, bufferReturn);
					bufferReturn += rawBytesRead;
					size -= rawBytesRead;
					if (rawBytesRead != rawReadSize) {
						// End of file
						if (rawFile->HaveError()) {
							haveError = true;
						}
						break;
					}
				}
			}

			// Read another buffer-full
			int rawBytesRead = rawFile->Read(bufferSize, buffer);
			endOfBufferPtr = buffer + rawBytesRead;

			// Compensate for bytes we must skip due to backing up and re-aligning
			// the I/O after a partial read.
			rawBytesRead -= bytesToSkip;
			bufferPtr += bytesToSkip;

			if (rawBytesRead <= 0) {
				// No more data
				if (rawFile->HaveError()) {
					haveError = true;
				}
				bufferPtr = endOfBufferPtr;
				break;
			}
		}

		// If more data is still needed, copy it from the buffer
		unsigned int bytesToCopy = JHMIN(size, (unsigned int)(endOfBufferPtr - bufferPtr));
		if (bytesToCopy != 0) {
			memcpy(bufferReturn, bufferPtr, bytesToCopy);
			bufferPtr += bytesToCopy;
			bufferReturn += bytesToCopy;
		}

		// Number of bytes read is known by how far we've advanced the buffer.
		return unsigned(bufferReturn - originalBufferReturn);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Called when when Write() would overflow data buffer
	// Inputs:
	//	unsigned int	size			Number of bytes to write to the file.
	//	const char*		bufferToWrite	Buffer containing data to write to the file.
	// Exceptions:
	//	ErrorException thrown on failure.
	///////////////////////////////////////////////////////////////////////////////
	void File::OutOfBufferWrite(
		unsigned int size,
		const char* bufferToWrite
	) {
		CritSec critSec(followerLock);

		while (unsigned(endOfBufferPtr - bufferPtr) < size) {
			// How much will fit?
			int availableSize = int(endOfBufferPtr - bufferPtr);
			// Copy as much as will fit
			if (availableSize != 0) {
				memcpy(bufferPtr, bufferToWrite, availableSize);
				bufferPtr += availableSize;
				size -= availableSize;
				bufferToWrite += availableSize;
			}

			// The memory buffer should be full now.
			assert(bufferPtr == endOfBufferPtr);

			try {
				// Write the buffer.  We are guaranteed that it is aligned,
				// because we've only written full buffers, and the buffer
				// was allocated for proper alignment by the raw file.
				rawFile->Write(bufferSize, buffer);
			} catch (ErrorException ex) {
				// Write failed.  Set buffer to empty so that if file is
				// destructed by exception unwinding the stack it does not
				// attempt to write again on file flush.
				bufferPtr = buffer;
				throw ex;
			}
			
			// The write buffer is now empty
			bufferPtr = buffer;
		}

		// The above logic should guarantee that we've flushed the buffer
		// at this point.
		assert(buffer = bufferPtr);

		// The above logic should guarantee that the amount of data left
		// to write will fit into the buffer.
		assert(size <= bufferSize);

		int rawBytesToWrite = JHMIN(size, (unsigned int)bufferSize);
		if (rawBytesToWrite != 0) {
			// Copy remaining to buffer for later write
			memcpy(bufferPtr, bufferToWrite, rawBytesToWrite);
			bufferPtr += rawBytesToWrite;
			bufferToWrite += rawBytesToWrite;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Seek: Set the position from which the next read will occur.
	// Inputs:
	//	__int64		position	Position in the file.
	// Return Value:
	//	bool		true on success, false on failure
	///////////////////////////////////////////////////////////////////////////////
	bool File::Seek(
		__int64 position
	) {
		// Cannot seek before file beginning.
		assert(position >= 0);
		// This only works when we are reading.
		assert(mode == Reading);
		if (mode != Reading) {
			haveError = true;
			return false;
		}

		if (
			position < rawFile->GetLastPosition() ||
			position >= rawFile->GetLastPosition() + (endOfBufferPtr - buffer)
		) {
			// Lock follower out during multi-step update of position
			CritSec critSec(followerLock);
			// The desired position it not found in the current buffer.
			// Seek the raw file.
			if (!rawFile->Seek(rawFile->RoundDownGranularity(position))) {
				haveError = true;
				return false;
			}
			// Read a block from the raw file.
			unsigned int rawBytesRead = rawFile->Read(bufferSize, buffer);
			endOfBufferPtr = buffer + rawBytesRead;
		}

		// Is the data within the block?
		if (position - rawFile->GetLastPosition() > endOfBufferPtr - buffer) {
			// The desired position is past the end-of-file.
			haveError = true;
			return false;
		}			

		// Position the buffer pointer to locate the data within the block
		bufferPtr = buffer + (int)(position - rawFile->GetLastPosition());
		// Everything is OK 
		haveError = false;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetPosition: Get the current file position.
	// Return Value:
	//	__int64		Position in the file.
	///////////////////////////////////////////////////////////////////////////////
	__int64 File::GetPosition() const
	{ 
		if (mode == Reading) {
			// Use previous raw position because that is the position of
			// the block we just read.
			return rawFile->GetLastPosition() + (bufferPtr - buffer); 
		} else {
			// Use current raw position because that is the position
			// of the block we are about to write.
			return rawFile->GetPosition() + (bufferPtr - buffer); 
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// Method to flush pending writes.  This may write extra "padding" on the
	// end of the file to satisfy granularity requirements of the filesystem.
	// Exceptions:
	//	ErrorException thrown on failure.
	///////////////////////////////////////////////////////////////////////////////
	void File::Flush()
	{
		CritSec critSec(followerLock);

		if (mode == Writing) {
			// Pad the buffer
			unsigned int sizeToWrite = 
				rawFile->RoundUpGranularity((unsigned int)(bufferPtr - buffer));
			unsigned int padding = unsigned(sizeToWrite - (bufferPtr - buffer));
			memset(bufferPtr, 0, padding);

			// flush pending writes
			rawFile->Write(sizeToWrite, buffer);

			// Reset buffer
			bufferPtr = buffer;
		}
	}

}  // namespace
