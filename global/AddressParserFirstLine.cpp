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
// AddressParserFirstLine.cpp: Address-parsing interface
//////////////////////////////////////////////////////////////////////

#include "Global_Headers.h"
#include "AddressParserFirstLine.h"
#include "AddressParserFirstLineImp.h"

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// constructor
	//////////////////////////////////////////////////////////////////////
	AddressParserFirstLine::AddressParserFirstLine()
	{
		errorMsgPtr = new char[2];
		errorMsgPtr[0] = 0;
		imp = new AddressParserFirstLineImp;
	}

	//////////////////////////////////////////////////////////////////////
	// virtual destructor in case it gets deleted by pointer-to-base
	//////////////////////////////////////////////////////////////////////
	AddressParserFirstLine::~AddressParserFirstLine()
	{
		delete imp;
		imp = 0;
		delete [] errorMsgPtr;
	}

	//////////////////////////////////////////////////////////////////////
	// Set Puerto-Rico-specific behavior (must be done before Open())
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLine::SetForPuertoRico(bool forPuertoRico_) {
		imp->SetForPuertoRico(forPuertoRico_);
	}
	bool AddressParserFirstLine::GetForPuertoRico() const {
		return imp->GetForPuertoRico();
	}

	//////////////////////////////////////////////////////////////////////
	// Initialize the address parser.
	//	const char*			dataDir		The directory containing data files.
	// Outputs:
	//	const char*&		errorMsg	Set to point to the error message if
	//									an error occurred.  This pointer is
	//									only valid until the next call to
	//									this object.
	// Return value:
	//	bool	true on success, false on error.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserFirstLine::Open(
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
	void AddressParserFirstLine::Close()
	{
		imp->Close();
	}

	//////////////////////////////////////////////////////////////////////
	// Read an address, generate tokens, and perform preprocessing of
	// address tokens.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserFirstLine::Parse(
		const char* addressLine, 
		ParseCandidate& parseCandidate, 
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
	void AddressParserFirstLine::PermuteAddress(int permutationFlags)
	{
		imp->PermuteAddress(permutationFlags);
	}

	//////////////////////////////////////////////////////////////////////
	// Retrieve the next address permutation.
	// Return value:
	//	bool	true if the next permutation is returned, false
	//			if there are no more permutations.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserFirstLine::NextAddressPermutation(
		ParseCandidate& parseCandidate,
		bool replaceAliases
	) {
		return imp->NextAddressPermutation(parseCandidate, replaceAliases);
	}

}

