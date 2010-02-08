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

//////////////////////////////////////////////////////////////////////
// AddressParserLastLine.h: Address-parsing tools
//////////////////////////////////////////////////////////////////////

#ifndef INCL_AddressParserLastLine_H
#define INCL_AddressParserLastLine_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "RefPtr.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class AddressParserLastLineImp;

	class AddressParserLastLine : public VRefCount {
	public:
		//////////////////////////////////////////////////////////////////////
		// Structure to hold parse candidates
		//////////////////////////////////////////////////////////////////////
		struct ParseCandidate {
			ParseCandidate() : flags(None), numberOfMods(0) {}

			// All of these are terminated strings.
			char city[29];		// city name
			char state[3];		// state abbreviation
			char postcode[7];	// 5-digit ZIP code or 3-letter Canadian postal code
			char postcodeExt[5];// 4-digit ZIP+4 or 3-letter Canadian postal code ext

			// Modifications made to address by parse process.  This indiciates the
			// sum of transformations applied to the address to generate this
			// candidate.  These flags should be used as part of the 
			// address-weighting process.
			typedef int Flags;
			enum {
				None = 0x0,

				// Small modifications
				TextNumber = 0x1,				// e.g. 1000 OAKS to THOUSAND OAKS
				Reorder = 0x2,					// e.g. 80303 DENVER CO --> DENVER CO 80303
				StateAlias = 0x4,				// e.g. WASHINGTON --> WA
				ShortenedCity = 0x8,			// e.g. SAN DIEGO HEIGHTS --> SAN DIEGO
												//  EAST LOS ANGELES --> LOS ANGELES
				CityAlias = 0x10				// e.g. MT PRINCETON --> MOUNT PRINCETON
			};

			Flags flags;
			int numberOfMods;		// Total count of modifications performed on
									// the address to generate this candidate.
		};

		//////////////////////////////////////////////////////////////////////
		// constructor
		//////////////////////////////////////////////////////////////////////
		AddressParserLastLine();

		//////////////////////////////////////////////////////////////////////
		// virtual destructor in case it gets deleted by pointer-to-base
		//////////////////////////////////////////////////////////////////////
		virtual ~AddressParserLastLine();

		//////////////////////////////////////////////////////////////////////
		// Initialize the address parser.
		// Inputs:
		//	const char*			dataDir		The directory containing data files.
		// Outputs:
		//	const char*&		errorMsg	If an error occurred, this will point
		//									to a string containing the message.
		//									The pointer is only valid until the
		//									next call into the object.
		// Return value:
		//	bool	true on success, false on error.
		//////////////////////////////////////////////////////////////////////
		bool Open(
			const char* dataDir,
			const char*& errorMsg
		);

		///////////////////////////////////////////////////////////////////////////////
		// Release any resources that were in use during processing.
		///////////////////////////////////////////////////////////////////////////////
		void Close();

		//////////////////////////////////////////////////////////////////////
		// Read an address, generate tokens, and perform preprocessing of
		// address tokens.
		//////////////////////////////////////////////////////////////////////
		bool Parse(
			const char* addressLine, 
			ParseCandidate& parseCandidate, 
			bool replaceAliases
		);

		// Permuation flags
		enum {
			PermuteShortenLongCityName = 0x1,
			PermuteCityComponentAlias = 0x2
		};

		//////////////////////////////////////////////////////////////////////
		// Perform permutations on the address.
		// Inputs:
		//	int				permutationFlags Bits set in this flag describe the
		//                                   permutations desired.
		// Return value:
		//////////////////////////////////////////////////////////////////////
		void PermuteAddress(int permutationFlags);

		//////////////////////////////////////////////////////////////////////
		// Retrieve the next address permutation.
		// Return value:
		//	bool	true if the next permutation is returned, false
		//			if there are no more permutations.
		//////////////////////////////////////////////////////////////////////
		bool NextAddressPermutation(
			ParseCandidate& parseCandidate,
			bool replaceAliases
		);

	private:
		// Points to internal implementation
		AddressParserLastLineImp* imp;

		// Holds pointer to allocated buffer for error-message return.
		char* errorMsgPtr;
	};
	typedef refcnt_ptr<AddressParserLastLine> AddressParserLastLineRef;

}

#endif

