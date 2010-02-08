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

// Soundex.h: Soundex routines

#ifndef INCL_SOUNDEX
#define INCL_SOUNDEX

#include "Global_DllExport.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////
	// Soundex:  Compute Knuth original soundex for a string.
	// Inputs:
	//	const unsigned char*	inStr	Input string, null-terminated
	// Outputs:
	//	unsigned char*			outStr	Output string.  Must point to a buffer
	//									at least five bytes long.
	///////////////////////////////////////////////////////////////////////////
	void Soundex(
		const unsigned char* inStr,
		unsigned char* outStr
	);
	inline void Soundex(
		const char* inStr,
		char* outStr
	) {
		Soundex(
			(const unsigned char*)inStr,
			(unsigned char*)outStr
		);
	}

	///////////////////////////////////////////////////////////////////////////
	// Soundex2:  Compute Knuth original soundex for a string, modified by
	// replacing common substrings with phonetic equivalents before computing
	// the soundex.
	// Inputs:
	//	const unsigned char*	inStr	Input string, null-terminated
	// Outputs:
	//	unsigned char*			workBuf Buffer that is at least as large
	//									as inStr with termination.  
	//									Will be destroyed.
	//	unsigned char*			outStr	Output string.  Must point to a buffer
	//									at least five bytes long.
	///////////////////////////////////////////////////////////////////////////
	void Soundex2(
		const unsigned char* inStr,
		unsigned char* workBuf,
		unsigned char* outStr
	);
	inline void Soundex2(
		const char* inStr,
		char* workBuf,
		char* outStr
	) {
		Soundex2(
			(const unsigned char*)inStr,
			(unsigned char*)workBuf,
			(unsigned char*)outStr
		);
	}

	///////////////////////////////////////////////////////////////////////////
	// Soundex3:  Like Soundex2, but adds special handling for strings that
	// contain numbers.  If any digits are found in the string, then up to
	// four digits will be used as the soundex.  If digits are found but there
	// are less than four, the soundex will be padded with spaces.  If no
	// digits are found, then Soundex2() is used.
	//
	// Inputs:
	//	const unsigned char*	inStr	Input string, null-terminated
	// Outputs:
	//	unsigned char*			workBuf Buffer that is at least as large
	//									as inStr with termination.  
	//									Will be destroyed.
	//	unsigned char*			outStr	Output string.  Must point to a buffer
	//									at least five bytes long.
	///////////////////////////////////////////////////////////////////////////
	void Soundex3(
		const unsigned char* inStr,
		unsigned char* workBuf,
		unsigned char* outStr
	);
	inline void Soundex3(
		const char* inStr,
		char* workBuf,
		char* outStr
	) {
		Soundex3(
			(const unsigned char*)inStr,
			(unsigned char*)workBuf,
			(unsigned char*)outStr
		);
	}


	///////////////////////////////////////////////////////////////////////////
	// Soundex4:  Like Soundex3, but augments the soundex algorithm to:
	// 1) Ignore trailing S
	// 2) Ignore trailing Z after consonant
	// 3) Fold D and T together
	//
	// Inputs:
	//	const unsigned char*	inStr	Input string, null-terminated
	// Outputs:
	//	unsigned char*			workBuf Buffer that is at least as large
	//									as inStr with termination.  
	//									Will be destroyed.
	//	unsigned char*			outStr	Output string.  Must point to a buffer
	//									at least five bytes long.
	///////////////////////////////////////////////////////////////////////////
	void Soundex4(
		const unsigned char* inStr,
		unsigned char* workBuf,
		unsigned char* outStr
	);
	inline void Soundex4(
		const char* inStr,
		char* workBuf,
		char* outStr
	) {
		Soundex4(
			(const unsigned char*)inStr,
			(unsigned char*)workBuf,
			(unsigned char*)outStr
		);
	}


}

#endif
