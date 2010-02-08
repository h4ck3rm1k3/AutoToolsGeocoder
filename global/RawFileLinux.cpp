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
# $Rev: 4 $ 
# $Date: 2006-04-19 16:29:34 -0600 (Wed, 19 Apr 2006) $ 
*/

// RawFileUnix.cpp Implementation of OS-specifc unbuffered I/O for Linux
#include <limits.h> 
#include <stdlib.h>

#if defined(UNIX)
#include <unistd.h>
#endif


#include "Global_Headers.h"
#include "RawFile.h"
#include "RawFileLinux.h"
#include "Exception.h"

namespace PortfolioExplorer {

  ///////////////////////////////////////////////////////////////////////////////
  // Constructor: create a file-buffer with no file opened
  ///////////////////////////////////////////////////////////////////////////////
  RawFileLinux::RawFileLinux()
  {
    buffer = NULL;
    memGranularity = getpagesize();
    diskGranularity = getpagesize();

  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // Will flush and close any open file.
  ///////////////////////////////////////////////////////////////////////////////
  RawFileLinux::~RawFileLinux()
  {
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
  bool RawFileLinux::Open(
			  OpenFlag openFlag_,
			  const TsString& filepath_,
			  int bufferSize_,
			  bool useOS_Buffer_
			  ) {
    // close any open file.
    Close();
    openFlag = openFlag_;
    haveError = false;
    position = 0;
    lastPosition = 0;
    useOS_Buffer = false;
    bufferSize = bufferSize_;

    //get real path used for exception messeges
    {
      char buf[1024];
      realpath( filepath_.c_str(), buf );
      fullpath = *buf;
    }

    // Allocate the buffer - This part is needed by file.h when used by PortfolioLoaderExplorers
    if (bufferSize == 0) {
      // Use default sizes.
      bufferSize = useOS_Buffer ? CachedDefaultBufferSize : NonCachedDefaultBufferSize;
    }

    bufferSize = RoundUp(bufferSize, memGranularity);
    buffer = new char[ bufferSize ];
    if (buffer == NULL) {
      haveError = true;
      return false;
    }


    //decide how to open the file stream.
    switch (openFlag) {
    case ReadOnly:
      filestream.open( filepath_.c_str(), std::fstream::in );
      break;
    case CreateAndWrite:
      filestream.open( filepath_.c_str(), std::fstream::out );
      break;
    case Append:
      filestream.open( filepath_.c_str(), std::fstream::app );
      break;
    default:
      assert(0);
      haveError = true;
      return false;
    }
    
    if ( !IsOpen() ) {
      haveError = true;
      delete [] buffer;
      buffer = NULL;
    }

    return IsOpen();
    
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Close a file.
  // Return Value:
  //	bool		true on success, false on failure.
  ///////////////////////////////////////////////////////////////////////////////
  bool RawFileLinux::Close()
  {
    if (!IsOpen())
      return true;
    filestream.close();
    if( buffer != NULL ) {
      delete [] buffer;
      buffer = NULL;
    }
    return !IsOpen();
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
  unsigned int RawFileLinux::Read(
				  unsigned int size,
				  char* bufferReturn
				  ) {

      //read file in chunks of MaxIOBlock size
    assert( IsOpen() );

    filestream.clear();
    filestream.seekg( 0, std::ios::beg );
    filestream.exceptions( std::fstream::badbit | std::fstream::failbit | std::fstream::goodbit );
    char* origBufferReturn = bufferReturn;
    uint32 bytesToRead = 0;
    uint32 bytesRead = 0;

    while( size > 0 ) {

      bytesToRead = GLOBAL_MIN(size, (uint32)MaxIOBlock);
      bytesRead = 0;
      
      bytesRead = filestream.readsome( bufferReturn, bytesToRead );

      if( !filestream.good() ) {
        haveError = true;
        break;
      }

      size -= bytesRead;
      bufferReturn += bytesRead;

      if (bytesRead != bytesToRead) //We're done.
        break;


    }
    lastPosition = position;
    position += bufferReturn - origBufferReturn;
    return (uint32)(bufferReturn - origBufferReturn);

  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // Write to a file.  An assertion will occur if file was opened for reading.
  // Inputs:
  //	unsigned int	size			Number of bytes to write to the file.
  //	const char*		bufferToWrite	Buffer containing data to write to the file.
  // Exceptions:
  //	ErrorException thrown on failure.
  ///////////////////////////////////////////////////////////////////////////////
  void RawFileLinux::Write(
			   unsigned int size,
			   const char* bufferToWrite
			   ) {

    //Write everything in one pass

    assert( IsOpen() );
    filestream.exceptions( std::fstream::badbit | std::fstream::failbit | std::fstream::goodbit );
    
    try {
      filestream.write( bufferToWrite, size );
      lastPosition = position;
      position = filestream.tellp();
      filestream.clear();
    } catch( std::fstream::failure e ) { //throw exception
      ThrowErrorException( e );
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
  bool RawFileLinux::Seek(__int64 position_)
  {
    assert( IsOpen() );
    if( position_ == filestream.seekg(position_).tellg() ) {
      lastPosition = position_;
      position = position_;
      return true;
    }
    return false;
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // Get the size of the physical file.  The file must be open.
  // Return value:
  //	__int64		The size of the file in bytes, or -1 on error;
  ///////////////////////////////////////////////////////////////////////////////
  __int64 RawFileLinux::GetFileSize() const
  {
    assert(IsOpen());
    std::streampos fileSize = filestream.seekg( 0, std::ios::end ).tellg();
    filestream.seekg( 0, std::ios::beg );
    return fileSize;
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
  void RawFileLinux::WriteAndClose(
				   unsigned int size,
				   const char* bufferToWrite
				   ) {
    assert(IsOpen());
    
    bool alreadyHaveError = haveError;
    bool haveWriteError = false;
    TsString propagateMessage;
    
    try {
      Write( size, bufferToWrite );
    } catch( std::fstream::failure e ) {
      haveWriteError  = true;
      propagateMessage = e.what();
    }
    
    Close();
    
    // throw exceptions on error, but do not re-throw exception if already thrown.
    if (!alreadyHaveError) {
      if (haveWriteError) {
	throw propagateMessage;
      } else if (haveError) {
	throw "Error occued in WriteAndClose()";
      }
    }
  }
  
	
  ///////////////////////////////////////////////////////////////////////////////
  // Gets the most recent error message and throws an exception.
  ///////////////////////////////////////////////////////////////////////////////
  void RawFileLinux::ThrowErrorException( std::fstream::failure e )
  {
    throw e.what();
  }
  
}
