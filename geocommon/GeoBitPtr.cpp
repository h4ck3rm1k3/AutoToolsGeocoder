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

// BitPtr.cpp:  Class that acts a pointer-to-bit.

#include "Geocoder_Headers.h"


#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "GeoBitPtr.h"

namespace PortfolioExplorer {

	unsigned char BitCRef::offsetToMask[8] = {
		(unsigned char)(~0x01),
		(unsigned char)(~0x02),
		(unsigned char)(~0x04),
		(unsigned char)(~0x08),
		(unsigned char)(~0x10),
		(unsigned char)(~0x20),
		(unsigned char)(~0x40),
		(unsigned char)(~0x80)
	};

}
