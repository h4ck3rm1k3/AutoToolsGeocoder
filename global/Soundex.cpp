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

// Soundex.cpp: Soundex routines

#include "Global_Headers.h"
#include "Soundex.h"
#include "Utility.h"

#include <algorithm>

namespace PortfolioExplorer {

	static unsigned char baseCode[26] =
		{  0,1,2,3,0,1,2,0,0,2,2,4,5,5,0,1,2,6,2,3,0,1,0,2,0,2 };
		/* a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z */
	static unsigned char code[256];
	static bool initialized = false;

	static void Init() {
		if (!initialized) {
			for (int i = 0; i < 256; i++) {
				code[i] = 0;
				if (ISALPHA(i)) {
					code[i] = baseCode[TOUPPER(i) - 'A'];
				}
			}
			initialized = true;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Data structures used in substring replacement.
	///////////////////////////////////////////////////////////////////////////
	struct SoundexReplacement {
		SoundexReplacement(
			const unsigned char* lookFor_, 
			const unsigned char* replaceWith_
		) :
			lookFor(lookFor_),
			replaceWith(replaceWith_)
		{}
		SoundexReplacement(
			const char* lookFor_, 
			const char* replaceWith_
		) :
			lookFor((const unsigned char*)lookFor_),
			replaceWith((const unsigned char*)replaceWith_)
		{}
		SoundexReplacement(char c) {
			buf[0] = TOUPPER(c);
			buf[1] = 0;
			lookFor = buf;
			replaceWith = 0;
		}
		const unsigned char* lookFor;
		const unsigned char* replaceWith;
		unsigned char buf[2];

		bool operator<(const SoundexReplacement& rhs) const {
			return lookFor[0] < rhs.lookFor[0];
		}
	};


	///////////////////////////////////////////////////////////////////////////
	// Replace common substrings with phonetic equivalents before computing
	// a soundex.
	// Inputs:
	//	const unsigned char*	inStr	Input string, null-terminated
	// Outputs:
	//	unsigned char*			workBuf Buffer that is at least as large
	//									as inStr with termination.
	///////////////////////////////////////////////////////////////////////////
	static void ModifySoundexBuffer(
		const unsigned char* inStr,
		unsigned char* workBuf
	) {
		// Table of replacements to be performed
		// These must be sorted by lookFor[0]
		static SoundexReplacement leadingReplacements[] = {
			SoundexReplacement("AV", "AF" ),
			SoundexReplacement("AH", "A" ),
			SoundexReplacement("AW", "A" ),
			SoundexReplacement("CAAN", "TAAN" ),
			SoundexReplacement("DG", "G" ),
			SoundexReplacement("D", "G" ),
			SoundexReplacement("HA", "A" ),
			SoundexReplacement("KN", "K" ),
			SoundexReplacement("K", "C" ),
			SoundexReplacement("MAC", "MC" ),
			SoundexReplacement("M", "N" ),
			SoundexReplacement("NST", "NS" ),
			SoundexReplacement("PF", "F" ),
			SoundexReplacement("PH", "F" ),
			SoundexReplacement("Q", "G" ),
			SoundexReplacement("SCH", "SH" ),
			SoundexReplacement("Z", "S" )
		};
		static SoundexReplacement inlineReplacements[] = {
			SoundexReplacement("AV", "AF" ),
			SoundexReplacement("AH", "A" ),
			SoundexReplacement("AW", "A" ),
			SoundexReplacement("CAAN", "TAAN" ),
			SoundexReplacement("DG", "G" ),
			SoundexReplacement("D", "G" ),
			SoundexReplacement("HA", "A" ),
			SoundexReplacement("KN", "K" ),
			SoundexReplacement("K", "C" ),
			SoundexReplacement("M", "N" ),
			SoundexReplacement("NST", "NS" ),
			SoundexReplacement("PH", "F" ),
			SoundexReplacement("Q", "G" ),
			SoundexReplacement("SCH", "SH" ),
			SoundexReplacement("Z", "S" )
		};

		#ifdef NELS
			#undef NELS
		#endif
		#define NELS(x) (sizeof(x) / sizeof(x[0]))

		// Preprocess the input into the work buffer.
		unsigned char* workPtr = workBuf;
		if (*inStr != 0) {
			// Perform first-character replPortfolioExplorernt
			SoundexReplacement* replPtr = std::lower_bound(
				leadingReplacements,
				leadingReplacements + NELS(leadingReplacements),
				SoundexReplacement(inStr[0])
			);
			bool replaced = false;
			while (
				replPtr != leadingReplacements + NELS(leadingReplacements) &&
				replPtr->lookFor[0] == TOUPPER(inStr[0])
			) {
				size_t lookForLen = strlen((const char*)replPtr->lookFor);
				size_t replaceWithLen = strlen((const char*)replPtr->replaceWith);
				if (MEMICMP(inStr, replPtr->lookFor, lookForLen) == 0) {
					memcpy(workPtr, replPtr->replaceWith, replaceWithLen);
					workPtr += replaceWithLen;
					inStr += lookForLen;
					replaced = true;
					break;
				}
				replPtr++;
			}
			if (!replaced) {
				*workPtr++ = *inStr++;
			}
		}

		// Perform inner-character replPortfolioExplorernt
		while (*inStr != 0) {
			bool replaced = false;
			SoundexReplacement* replPtr = std::lower_bound(
				inlineReplacements,
				inlineReplacements + NELS(inlineReplacements),
				SoundexReplacement(inStr[0])
			);
			while (
				replPtr != inlineReplacements + NELS(inlineReplacements) &&
				replPtr->lookFor[0] == TOUPPER(inStr[0])
			) {
				size_t lookForLen = strlen((const char*)replPtr->lookFor);
				size_t replaceWithLen = strlen((const char*)replPtr->replaceWith);
				if (MEMICMP(inStr, replPtr->lookFor, lookForLen) == 0) {
					memcpy(workPtr, replPtr->replaceWith, replaceWithLen);
					workPtr += replaceWithLen;
					inStr += lookForLen;
					replaced = true;
					break;
				}
				replPtr++;
			}

			if (!replaced) {
				*workPtr++ = *inStr++;
			}
		}
		*workPtr = 0;
	}


	///////////////////////////////////////////////////////////////////////////
	// Check for digits in the incoming string, and if present construct
	// a soundex using only the digits.
	//
	// Inputs:
	//	const unsigned char*	inStr	Input string, null-terminated
	// Outputs:
	//	unsigned char*			outStr	Output string.  Must point to a buffer
	//									at least five bytes long.
	///////////////////////////////////////////////////////////////////////////
	static inline bool CheckForDigits(
		const unsigned char* inStr,
		unsigned char* outStr
	) {

		// Look for digits
		unsigned char* outPtr = outStr;
		for (const unsigned char* p = inStr; *p != 0; p++) {
			if (ISDIGIT(*p)) {
				*outPtr++ = *p++;
				while (*p != 0 && outPtr < outStr + 4) {
					if (ISDIGIT(*p)) {
						*outPtr++ = *p;
					}
					p++;
				}
				while (outPtr < outStr + 4) {
					*outPtr++ = ' ';
				}
				*outPtr = 0;
				return true;
			}
		}
		return false;
	}

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
	) {
		Init();

		// Set up default key, complete with trailing '0's
		strcpy((char*)outStr, "Z000");

		// Advance to the first letter.
		while (*inStr != 0  &&  !ISALPHA(*inStr)) {
            ++inStr;
		}
		if (*inStr != 0) {
			char lastCode;

			// Pull out the first letter, and set up for main loop
			outStr[0] = TOUPPER(*inStr);
			lastCode = code[*inStr];
			++inStr;

			// Scan rest of string, stop at end of string or when the key is full
			for (int count = 1; count < 4 && *inStr != 0; ++inStr) {
				if (ISALPHA(*inStr)) {
					char thisCode = code[*inStr];
					// Fold together adjacent letters sharing the same code
					if (lastCode != thisCode) {
						lastCode = thisCode;
						// Ignore code == 0 letters except as separators
						if (thisCode != 0) {
							outStr[count++] = '0' + thisCode;
						}
					}
				}
			}
		}
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
	) {
		// Perform phonetic replacements
		ModifySoundexBuffer(inStr, workBuf);

		// Call the normal soundex
		Soundex(workBuf, outStr);
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
	) {
		if (!CheckForDigits(inStr, outStr)) {
			// Failing to find digits, use the Soundex2 algorithm
			Soundex2(inStr, workBuf, outStr);
		}
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
	) {
		if (CheckForDigits(inStr, outStr)) {
			return;
		}

		// Perform phonetic replacements
		ModifySoundexBuffer(inStr, workBuf);

		// Also map T onto G
		unsigned char* ptr;
		for (ptr = workBuf; *ptr != 0; ptr++) {
			if (*ptr == 'T') {
				*ptr = 'G';
			}
		}

		// Look for trailing characters to change
		int len = int(ptr - workBuf);

/*
		// First remove trailing vowels because they mess with the ability to get the same
		// soundex out of e.g. LIND and LINDA.
		if (len >= 2 && code[workBuf[len-1]] == 0) {
			workBuf[len-1] = 0;
			len--;
		}
*/

		if (len >= 2) {
			switch (workBuf[len-1]) {
			case 'S':
			case 'Z':
			case 'X':
				workBuf[len-1] = 0;
				len--;
				break;
			}
		}

		// Call the normal soundex
		Soundex(workBuf, outStr);
	}

}
