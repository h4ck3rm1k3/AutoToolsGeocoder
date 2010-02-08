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
# $Rev: 39 $ 
# $Date: 2006-08-02 18:15:24 +0200 (Wed, 02 Aug 2006) $ 
*/

// Geoload_Utilities.h

#ifndef INCL_Geoload_Utilities_h
#define INCL_Geoload_Utilities_h

#include "io.h"

namespace PortfolioExplorer {
	
	inline void WriteThreeByteInt(File& file, unsigned int value) {
		assert(value < 0x1000000);
		unsigned char c;
		c = value;
		file.Write(1, &c);
		c = value >> 8;
		file.Write(1, &c);
		c = value >> 16;
		file.Write(1, &c);
	}

	inline void WriteTwoByteInt(File& file, unsigned int value) {
		assert(value < 0x10000);
		unsigned char c;
		c = value;
		file.Write(1, &c);
		c = value >> 8;
		file.Write(1, &c);
	}

	inline void WriteOneByteInt(File& file, unsigned int value) {
		assert(value < 0x100);
		unsigned char c = value;
		file.Write(1, &c);
	}

	inline void WriteThreeByteInt(File& file, int value) {
		assert(value <= 0x7fffff && value > -0x800000);
		unsigned char c;
		c = value;
		file.Write(1, &c);
		c = value >> 8;
		file.Write(1, &c);
		c = value >> 16;
		file.Write(1, &c);
	}

	inline void WriteTwoByteInt(File& file, int value) {
		assert(value <= 0x7fff && value > -0x8000);
		unsigned char c;
		c = value;
		file.Write(1, &c);
		c = value >> 8;
		file.Write(1, &c);
	}

	inline void WriteOneByteInt(File& file, int value) {
		assert(value <= 0x7f && value > -0x80);
		unsigned char c;
		c = value;
		file.Write(1, &c);
	}
}

#endif
