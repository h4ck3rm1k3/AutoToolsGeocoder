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

//////////////////////////////////////////////////////////////////////
// AddressParserLastLine.cpp:  Class that implements parsing of address lines
// into possible interpretations.
//////////////////////////////////////////////////////////////////////

#include "Global_Headers.h"
#include "AddressParserLastLine.h"
#include "AddressParserLastLineImp.h"

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Static variables
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Lookup tables
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// constructor
	//////////////////////////////////////////////////////////////////////
	AddressParserLastLine::AddressParserLastLine()
	{
		errorMsgPtr = new char[2];
		errorMsgPtr[0] = 0;
		imp = new AddressParserLastLineImp;
	}

	//////////////////////////////////////////////////////////////////////
	// destructor in case it gets deleted by pointer-to-base
	//////////////////////////////////////////////////////////////////////
	AddressParserLastLine::~AddressParserLastLine()
	{
		delete imp;
		imp = 0;
		delete [] errorMsgPtr;
	}

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
	bool AddressParserLastLine::Open(
		const char* dataDir,
		const char*& errorMsg
	) {
		TsString temp;
		bool retval = imp->Open(dataDir, temp);
		if (!retval) {
			delete [] errorMsgPtr;
			errorMsgPtr = new char[temp.size() + 1];
			strcpy(errorMsgPtr, temp.c_str());
		}
		errorMsg = errorMsgPtr;
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Release any resources that were in use during processing.
	///////////////////////////////////////////////////////////////////////////////
	void AddressParserLastLine::Close()
	{
		imp->Close();
	}

	//////////////////////////////////////////////////////////////////////
	// Read an address, generate tokens, and perform preprocessing of
	// address tokens.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserLastLine::Parse(
		const char* addressLine, 
		AddressParserLastLine::ParseCandidate& parseCandidate, 
		bool replaceAliases
	) {
		return imp->Parse(addressLine, parseCandidate, replaceAliases);
	}



	//////////////////////////////////////////////////////////////////////
	// Perform permutations on the address.
	// Inputs:
	//	int				permutationFlags Bits set in this flag describe the
	//                                   permutations desired.
	// Return value:
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLine::PermuteAddress(int permutationFlags)
	{
		imp->PermuteAddress(permutationFlags);
	}

	//////////////////////////////////////////////////////////////////////
	// Retrieve the next address permutation.
	// Return value:
	//	bool	true if the next permutation is returned, false
	//			if there are no more permutations.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserLastLine::NextAddressPermutation(
		AddressParserLastLine::ParseCandidate& parseCandidate,
		bool replaceAliases
	) {
		return imp->NextAddressPermutation(parseCandidate, replaceAliases);
	}



}	// namespace 




