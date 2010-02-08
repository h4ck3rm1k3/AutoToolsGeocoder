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
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

//////////////////////////////////////////////////////////////////////
// AddressParserFirstLine.h: Address-parsing interface
//////////////////////////////////////////////////////////////////////

#ifndef INCL_AddressParserFirstLine_H
#define INCL_AddressParserFirstLine_H

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

	class AddressParserFirstLineImp;

	class AddressParserFirstLine : public VRefCount {	

	public:
		//////////////////////////////////////////////////////////////////////
		// Structure to hold parse candidates
		//////////////////////////////////////////////////////////////////////
		struct ParseCandidate {
			ParseCandidate() : 
				isIntersection(false), 
				permutations(0),
				numberOfMods(0)
			{}

			// All of these are terminated strings.
			char street[29];		// street name
			char unitDesignator[5];	// unit designator
			char unitNumber[9];		// unit number

			bool isIntersection;	// true if address is an intersection

			// Used only when address is NOT an intersection
			char number[11];		// address number
			char predir[10];		// predirectional
			char prefix[6];			// prefix
			char suffix[11];		// street suffix
			char postdir[10];		// postdirectional
			char pmbDesignator[5];	// PMB (private mailbox) designator
			char pmbNumber[9];		// PMB (private mailbox) number

			// Used only when address IS an intersection
			char predir2[10];	    // predirectional
			char street2[29];		// street name for cross street
			char suffix2[11];		// street suffix
			char postdir2[10];		// postdirectional

			// Permutations applied to this parse
			int permutations;

			// Number of modifications performed to the parse candidate.
			int numberOfMods;
		};


		// Sets of flags that can be ORed to together and passed to the PermuteAddress() method.
		enum Permutations {
			PermuteNone = 0x0,

			PermuteShiftSuffixUnitdes = 0x1,// Shift suffix and unitdes from street name
											//
											// 123 MAIN STREET SUITE  ->   123  MAIN   STREET SUITE
											//	^  ^---------------^        ^    ^       ^      ^
											// nbr         street          nbr street  suffix  unitdes

			PermuteStreetNameAlias = 0x2,	// Replaces many aliases in street names, including all of the below:
											// 
											// numerics: 
											//   HWY 10 <--> HWY TEN
											//   FIRST STREET <--> 1ST STREET
											//
											// suffix abbr:
											//   HIGHWAY 10 <--> HWY 10
											//   RTE 109 <--> ROUTE 109
											//
											// honorifics:
											//   SAINT CLAIRE <--> ST CLAIRE

			PermuteSplitSuffix = 0x4,		// e.g.
											//    MAINSTREET --> MAIN STREET
											//    14ST --> 14 ST
											// but NOT
											//    MAST --> MA ST
											//    23RD --> 23 RD
											//    21ST --> 21 ST
										


			PermuteSplitPredir= 0x8,		// e.g. WESTBAY to WEST BAY
			                                //      N W MAIN to NW MAIN


			PermuteSplitPostdir= 0x10,		// e.g. BAYWEST to BAY WEST

			PermuteSplitAddrNbrToUnit = 0x20,// e.g. 
											//      123A MAIN ST to 123 MAIN ST #A
											//      123-4 MAIN ST to 123 MAIN ST #4

			PermuteStreetNamePrefixAlias = 0x40, // e.g.
											//		123 ST-CLAIRE to 123 SAINT-CLAIRE
											//		123 SAINTE-GEORGE to 123 STE-GEORGE

			PermuteStreetNumberToUnit = 0x80,	// e.g.:
											//		123 MAIN ST 4  -->  123 MAIN ST   4  
											//		 ^  ^-------^        ^  ^-----^   ^
											//       nbr   street       nbr  street   unit

			PermuteAddrNumberAlias = 0x100,	// e.g.
											//  ONE MAIN ST --> 1 MAIN ST

			PermuteStreetNameMultiwordAlias = 0x200,
											// Replaces multi-word aliases in street names, including all of the below:
											//   NEW JERSEY TURNPIKE <--> NJ TURNPIKE


			PermuteShiftUnitdes = 0x400,	// Shift unitdes from street name
											//
											// 123 MAIN SUITE  ->   123  MAIN    SUITE
											//	^  ^--------^        ^    ^        ^
											// nbr   street         nbr  street  unitdes

			PermuteRemoveDoubleSuffix = 0x800,	// Remove double suffix 
											//
											// 123 MAIN STREET COURT   --> 123  MAIN    STREET
											//	^  ^---------^   ^          ^     ^      ^     
											// nbr   street			       nbr  street  suffix

			PermuteGlomSuffix = 0x1000,		// 123 FIRST ROAD  -->  123 FIRST ROAD
											//       ^     ^             ^------^
											//    street  suffix          street
											// 

			PermuteCleaveSuffix = 0x2000,	// 123 EAST LOOP  -->  123 EAST LOOP
											//     ^-------^             ^    ^
											//       street          street   suffix
											// 


			PermuteCleavePostdir = 0x4000,	// 123 CALLE E    -->   123 CALLE  E
											//     ^-----^                 ^   ^
											//      street             street  postdir
											//

			PermuteGlomPostdir = 0x8000,	// 123 32ND   NORTH --> 123 32ND NORTH
											//       ^      ^           ^--------^ 
											//    street   postdir        street
											//

			PermuteStreetSuffixToPredirStreet = 0x10000,
											// 123 WEST COURT  -->  123 WEST COURT
											//       ^    ^               ^    ^
											//   street  suffix       predir  street
											//

			PermuteCleaveUnitdes = 0x20000,	// Cleave unit designator off street, moving suffix to unit
											//   123 HIGH OFF  ST  -->  123 HIGH   OFF      ST
											//       ^------^  ^              ^     ^       ^  
											//        street  suffix       street  unitdes  unitnum        
											//
											// Cleave unit designator off street, moving postdir to unit
											//   123 THREE OFFICE   E      ->    123 THREE OFFICE    E
											//       ^----------^   ^                  ^     ^       ^  
											//          street	  postdir           street  unitdes  unitnum        
											//

			PermuteRemoveIntersection = 0x40000,
											// 123 HOLLY AND VINE ST --> 123 HOLLY AND VINE ST
											//      ^          ^             ^------------^ 
											//   street1     street2            street
											//

			PermuteShufflePmbUnit = 0x80000,// 123 HOLLY PIER   A    OFFICE 123    --> 123 HOLLY PIER A OFFICE 123
											//      ^      ^    ^      ^     ^             ^----------^    ^    ^
											//   street unitdes unit pmbdes pmbnum           street     unitdes unit
											//

			PermuteMoveFractionalStreetName = 0x100000,
											// Move a street name as fraction to the address number and unit designator to the street
											// 43     1/2    PIER -->  43 1/2  PIER
											//  ^      ^      ^          ^       ^
											// nbr   street unit        nbr  street


			PermuteMoveStreetLetterToNbr = 0x200000,
											// Move a leading single letter from the street name to the address number, e.g.:
											//  123  B MAIN  ST      -->   123B   MAIN     ST
											//   ^   ^----^  ^              ^     ^        ^
											//  nbr  street  suffix        nbr   street  suffix
											//
											//  A EL MIRADOR APTS    -->   A   EL MIRADOR APTS
											//  ^---------------^          ^   ^-------------^
											//      street                nbr       street


			PermuteMoveNbrWordToStreet = 0x400000,
											// Move a non-numeric word from the NBR to the STREET:
											//  COND  STA ANNA   -->   COND STA ANNA
											//   ^    ^------^         ^-----------^
											//  nbr    street             street


			PermuteSuffixUnitdesToStreet = 0x800000,
											// Move suffix/unitdes to street:
											//	CRESCENT BEACH  BLDG      -->  CRESCENT BEACH BLDG
											//    ^        ^      ^            ^-----------------^
											//   street  suffix  unitdes             street

			PermuteMoveStreetToUnitDes = 0x1000000,
											// Shuffle street-last-token-that-is-unitdes into unit
											//	2158 AVE GILBERTO MONROEG APT PH   -->   2158 AVE GILBERTO MONROEG   APT     PH
											//    ^   ^--------------------^   ^           ^  ^------------------^    ^      ^
											//   nbr           street        unitdes       nbr      street         unitdes  unit
											//
											//	211 KERR ADMINISTRATION BLDG --> 211 KERR ADMINISTRATION BLDG
											//    ^ ^----------------------^      ^  ^-----------------^   ^
											//   nbr        street                nbr      street         unitdes

			PermuteMoveUnitdesToStreet = 0x2000000,
											// Append unitdes to street
											//	2158 MONROE BLDG      -->   2158 MONROE BLDG
											//    ^     ^     ^              ^   ^---------^
											//   nbr  street  unitdes        nbr    street

			PermutePrPostdirToNbr = 0x4000000,
											// Move postdir to address number for PR, e.g.
											// Input like:
											//  CALLE JUAN MORELL CAMPOS N 15
											// is normally parsed as:
											//  15   CALLE JUAN MORELL CAMPOS   N
											//   ^   ^----------------------^   ^
											//	nbr           street          postdir
											// change to:
											//  N 15   CALLE JUAN MORELL CAMPOS 
											//  ^--^   ^----------------------^
											//   nbr			street


			PermuteSaltlakeSyndrome = 0x8000000,
											// Split predir and postdir from numeric addresses
											// Input like:
											//  100N 200E
											// is normally parsed as:
											//  100E    200N
											//   ^       ^
											//	nbr    street
											// change to:
											//  100    E      200    N
											//   ^     ^       ^     ^
											//	nbr  predir  street  postdir

			PermuteAlias = 0x10000000,
											// Substitute alias for component, e.g.
											// N STREET --> N ST
											// NORTH MAIN ST --> N MAIN ST
			PermuteUnitStreet = 0x20000000,
											// Permute unit/nbr to street for Military addresses
											// UNIT   123    -->    UNIT   123
											//   ^     ^             ^-------^
											// street  nbr            street
			PermuteRemoveDoubleUnit = 0x40000000,
											// Cleave unit from street and remove #
											// 123 MAIN APT   #       3      ->  123 MAIN  APT     3
											//     ^------^   ^       ^               ^     ^      ^
											//      street unitdes  unitnbr       street unitdes  unitnbr
			PermuteGlomPredir = 0x80000000,	
											// 123 WEST 32ND ST --> 123 WEST 32ND ST
											//           ^              ^------^ 
											//         street            street
											//

			PermuteAll = ~0UL
		};

		//////////////////////////////////////////////////////////////////////
		// constructor
		//////////////////////////////////////////////////////////////////////
		AddressParserFirstLine();

		//////////////////////////////////////////////////////////////////////
		// virtual destructor in case it gets deleted by pointer-to-base
		//////////////////////////////////////////////////////////////////////
		virtual ~AddressParserFirstLine();

		//////////////////////////////////////////////////////////////////////
		// Set Puerto-Rico-specific behavior (must be done before Open())
		//////////////////////////////////////////////////////////////////////
		void SetForPuertoRico(bool forPuertoRico_);
		bool GetForPuertoRico() const;

		//////////////////////////////////////////////////////////////////////
		// Initialize the address parser.
		// Inputs:
		//	const char*			dataDir		The directory containing data files.
		// Outputs:
		//	const char*&		errorMsg	Set to point to the error message if
		//									an error occurred.  This pointer is
		//									only valid until the next call to
		//									this object.
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
		// Inputs:
		//	const char*			addressLine			The first-line address to parse
		// Outputs:
		//	ParseCandidate&		parseCandidate		The return parsed address
		// Return value:
		//	bool				true on success, false if the address could not be parsed.
		//////////////////////////////////////////////////////////////////////
		bool Parse(
			const char* addressLine, 
			ParseCandidate& parseCandidate, 
			bool replaceAliases
		);

		//////////////////////////////////////////////////////////////////////
		// Create permutations of the original address parse.
		// Inputs:
		//	int			permutationFlags	ORed combination of the values 
		//									from the Permutations enumeration.
		//
		// Note: After creating permutations, all new address-parsing candidates
		// will be available through NextAddressPermutation().
		//
		// Note: Each time PermuteAddress() is called, new permutations are
		// attempted based on each previous parse candidate.  In this way,
		// multi-step permutations may be generated.  Check the numberOfMods
		// value in the ParseCandidate to determine how many permutations
		// have been applied.
		//////////////////////////////////////////////////////////////////////
		void PermuteAddress(int permutationFlags);

		//////////////////////////////////////////////////////////////////////
		// Retrieve the next address permutation.
		// Outputs:
		//	ParseCandidate&		parseCandidate		The returned parsed address
		// Return value:
		//	bool	true if the next permutation is returned, false
		//			if there are no more permutations.
		//
		// Note: After NextAddressPermutation() returns false, you may call
		// PermuteAddress() again (perhaps using new flags) to generate
		// more permutations, which will become available through
		// subsequent calls to NextAddressPermutation().
		//////////////////////////////////////////////////////////////////////
		bool NextAddressPermutation(
			ParseCandidate& parseCandidate,
			bool replaceAliases
		);

	private:
		// Pointer to implementation.
		AddressParserFirstLineImp* imp;

		// Holds the last error message, to avoid exposing TsString to wrapper interface.
		char* errorMsgPtr;
	};
	
	typedef refcnt_ptr<AddressParserFirstLine> AddressParserFirstLineRef;
}

#endif

