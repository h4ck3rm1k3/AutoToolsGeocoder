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

// AbstractByteIO.h: Abstract base classes for byte reading and writing

#ifndef INCL_AbstractByteIO_H
#define INCL_AbstractByteIO_H

#include "Geocoder_DllExport.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////
	// Abstract base class that writes bytes to arbitrary output
	///////////////////////////////////////////////////////////////////////////
	class ByteWriter : public VRefCount {
	public:
		virtual ~ByteWriter() {}

		// Returns false if bytes cannot be written.
		virtual bool Write(int count, const unsigned char *buffer) = 0;
	};
	typedef refcnt_ptr<ByteWriter> ByteWriterRef;

	///////////////////////////////////////////////////////////////////////////
	// Adaptor class that provides bytes for the conversion
	///////////////////////////////////////////////////////////////////////////
	class ByteReader : public VRefCount {
	public:
		virtual ~ByteReader() {}
		// Read bytes from the stream.
		// Return the number of bytes successfully read.
		virtual int Read(
			int size,
			unsigned char* returnBuffer
		) = 0;

		// Read another byte from the stream, or return false if 
		// no more bytes are available.
		bool Read(unsigned char& returnValue) {
			return Read(1, &returnValue) == 1;
		}

		// Seek to given position in the file.
		// Returns true on success, false on failure
		virtual bool Seek(int pos) = 0;

		// Return the current position in the file.
		virtual int GetPosition() = 0;

	};
	typedef refcnt_ptr<ByteReader> ByteReaderRef;

}

#endif
