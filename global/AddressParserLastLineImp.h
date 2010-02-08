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
// AddressParserLastLineImp.h: Address-parsing tools
//////////////////////////////////////////////////////////////////////

#ifndef INCL_AddressParserLastLineImp_H
#define INCL_AddressParserLastLineImp_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <map>
#include "TsString.h"
#include <vector>

#include "RefPtr.h"
#include "AddressTokenizer.h"
#include "LookupTable.h"
#include "VectorNoDestruct.h"
#include "AddressParserLastLine.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	// Don't need to export this internal class (we hope)
	class AddressParserLastLineImp : public VRefCount {
	public:
		//////////////////////////////////////////////////////////////////////
		// constructor
		//////////////////////////////////////////////////////////////////////
		AddressParserLastLineImp();

		//////////////////////////////////////////////////////////////////////
		// virtual destructor in case it gets deleted by pointer-to-base
		//////////////////////////////////////////////////////////////////////
		virtual ~AddressParserLastLineImp();

		//////////////////////////////////////////////////////////////////////
		// Initialize the address parser.
		// Inputs:
		//	const TsString&	dataDir		The directory containing data files.
		// Outputs:
		//	std::String&		errorMsg	The message if an error occured.
		// Return value:
		//	bool	true on success, false on error.
		//////////////////////////////////////////////////////////////////////
		bool Open(
			const TsString& dataDir,
			TsString& errorMsg
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
			AddressParserLastLine::ParseCandidate& parseCandidate, 
			bool replaceAliases
		);

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
			AddressParserLastLine::ParseCandidate& parseCandidate,
			bool replaceAliases
		);

	private:
		//////////////////////////////////////////////////////////////////////
		// Replace the text of a token.
		// Inputs:
		//	Token&			token	The token of interest.
		//	const char*		text	The new text
		//////////////////////////////////////////////////////////////////////
		void SetTokenText(Token& token, const char* text) {
			addressTokenizer->SetTokenText(token, text);
		}

		//////////////////////////////////////////////////////////////////////
		// Replace the text of a token.  Like SetTokenText() above, except
		// that the given text is explicitly truncated at the given length.
		// Inputs:
		//	Token&			token	The token of interest.
		//	const char*		text	The new text
		//	int				size	The size of the text
		//////////////////////////////////////////////////////////////////////
		void SetTokenText(Token& token, const char* text, int size) {
			addressTokenizer->SetTokenText(token, text, size);
		}	

		//////////////////////////////////////////////////////////////////////
		// Stores a TokenList plus modification flags
		//////////////////////////////////////////////////////////////////////
		struct TokenListAndFlags {
			TokenListAndFlags() { Clear(); }

			void Clear() {
				flags = AddressParserLastLine::ParseCandidate::None;
				numberOfMods = 0;
				tokenList.clear();
			}

			TokenList tokenList;
			AddressParserLastLine::ParseCandidate::Flags flags;
			int numberOfMods;		// Total count of modifications performed on
									// the address to generate this candidate.
		};

		//////////////////////////////////////////////////////////////////////
		// Stores parsed address tokens, and allows for synchronization with
		// an "original" form.
		//////////////////////////////////////////////////////////////////////
		struct ParsedTokens {
			ParsedTokens() : 
				flags(AddressParserLastLine::ParseCandidate::None), 
				numberOfMods(0)
			{}

			// Clear all of the token lists.
			void Clear();

			// Equality comparison
			bool operator==(const ParsedTokens& rhs) const;

			// Hash the token lists into a hash buffer.
			void Hash(TokenHashBuffer& hashBuffer) const;

			// Lists of tokens for each parsed category.
			TokenList city;
			TokenList state;
			TokenList postcode;
			TokenList postcodeExt;
			TokenList discarded;

			AddressParserLastLine::ParseCandidate::Flags flags;
			int numberOfMods;		// Total count of modifications performed on
									// the address to generate this candidate.
		};

		//////////////////////////////////////////////////////////////////////
		// Perform a first-cut parse of the tokens using "rules of thumb" 
		// for splitting the address into components.
		// Inputs:
		//	const TokenListAndFlags&	tokenListAndFlags	The token list to parse
		// Outputs:
		//	ParsedTokens&				parsedTokens		The resulting list of token
		//													positions.
		//////////////////////////////////////////////////////////////////////
		void ParseGuess(
			const TokenListAndFlags& tokenListAndFlags,
			ParsedTokens& parsedTokens
		);

		//////////////////////////////////////////////////////////////////////
		// Check the last parse candidate added to the parsedTokens vector, and
		// remove it if it is duplicate.
		//////////////////////////////////////////////////////////////////////
		void CheckNewParseCandidateForDuplication();

		//////////////////////////////////////////////////////////////////////
		// Convert a ParsedTokens to a ParseCandidate
		//////////////////////////////////////////////////////////////////////
		void ParsedTokensToParseCandidate(
			AddressParserLastLine::ParseCandidate& parseCandidate, 
			const ParsedTokens& parsedTokens,
			bool replaceAliases
		);

		// Data directory.  Set in ctor
		TsString dataDir;

		// Holds tokenized input string variants.
		// Note that VectorNoDestruct<> does not destruct its elements when cleared.
		// This prevents a bunch of TokenList construction/destruction, which 
		// avoids excess heap activity.
		VectorNoDestruct<TokenListAndFlags> inputTokenLists;
		TokenList tmpTokenList;

		// Holds list of parsed candidates
		// Note that VectorNoDestruct<> does not destruct its elements when cleared.
		VectorNoDestruct<ParsedTokens> parsedTokens;

		// Index of the next candidate in the parsedTokens list
		int nextCandidateIdx;

		// Data structures used to compared hashed versions of each candidate parse.
		// This is used to eliminate duplicates.  Very simple, but fast and effective
		// because of the normally-small number of candidates.
		HashedCandidateTable hashedCandidates;

		// Tokenizer used to transform raw input into basic tokens.
		AddressTokenizerRef addressTokenizer;

		// Object used to allocate memory for token text.
		// Shared with the AddressTokenizer.
		BulkAllocatorRef bulkAllocator;

		// Table of state aliases.
		LookupTableRef stateAliasTable;

		// Table of city component aliases.
		LookupTableRef cityComponentAliasTable;

		TsString tempStr1, tempStr2;
	};

}

#endif

