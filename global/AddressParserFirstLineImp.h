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
// AddressParserFirstLineImp.h: Address-parsing tools
//////////////////////////////////////////////////////////////////////

#ifndef INCL_AddressParserFirstLineImp_H
#define INCL_AddressParserFirstLineImp_H

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
#include "Lexicon.h"
#include "CritSec.h"
#include "VectorNoDestruct.h"
#include "RegularExprWrapper.h"
#include "AddressParserFirstLine.h"
#include "AddressToken.h"
#include "LookupTable.h"
#include "Global_DllExport.h"


namespace PortfolioExplorer {

	// Don't need to export this internal class (we hope)
	class AddressParserFirstLineImp : public VRefCount {	

	public:
		//////////////////////////////////////////////////////////////////////
		// constructor
		//////////////////////////////////////////////////////////////////////
		AddressParserFirstLineImp();

		//////////////////////////////////////////////////////////////////////
		// virtual destructor in case it gets deleted by pointer-to-base
		//////////////////////////////////////////////////////////////////////
		virtual ~AddressParserFirstLineImp();

		//////////////////////////////////////////////////////////////////////
		// Set Puerto-Rico-specific behavior (must be done before Open())
		//////////////////////////////////////////////////////////////////////
		void SetForPuertoRico(bool forPuertoRico_) { forPuertoRico = forPuertoRico_; }
		bool GetForPuertoRico() const { return forPuertoRico; }

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
			AddressParserFirstLine::ParseCandidate& parseCandidate, 
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
			AddressParserFirstLine::ParseCandidate& parseCandidate,
			bool replaceAliases
		);

	private:
		//////////////////////////////////////////////////////////////////////
		// Replace the text of a token.
		// Inputs:
		//	Token&			token	The token of interest.
		//	const char*		text	The new text
		//////////////////////////////////////////////////////////////////////
		void SetTokenText(
			Token& token, 
			const char* text
		) {
			token.size = int(strlen((const char*)text));
			char* ptr = (char*)bulkAllocator->New(token.size + 1);
			strcpy((char*)ptr, (const char*)text);
			token.text = ptr;
		}

		//////////////////////////////////////////////////////////////////////
		// Replace the text of a token.  Like SetTokenText() above, except
		// that the given text is explicitly truncated at the given length.
		// Inputs:
		//	Token&			token	The token of interest.
		//	const char*		text	The new text
		//	int				size	The size of the text
		//////////////////////////////////////////////////////////////////////
		void SetTokenText(
			Token& token, 
			const char* text, 
			int size
		) {
			token.size = size;
			char* ptr = (char*)bulkAllocator->New(size + 1);
			memcpy(ptr, text, size);
			ptr[size] = 0;
			token.text = ptr;
		}	

		//Struct to assist in assembling parsed address tokens into address
		struct AssemBridging : public VRefCount {
			AssemBridging(TsString tokenClass_, TokenList* outputList_) :
				tokenClass(tokenClass_), outputList(outputList_)
			{}
			~AssemBridging() {}

			TsString  tokenClass; //Class to be assembled
			TokenList*   outputList; //pointer to output assembly string
		};
		typedef refcnt_ptr<AssemBridging> AssemBridgingRef;

		//////////////////////////////////////////////////////////////////////
		// Fill the assembly trie with the proper assemBridging structs
		//////////////////////////////////////////////////////////////////////
		void GenerateAssemblyTrie();

		//Struct to assist in setting token flags based on symbol
		struct SymbolFlagMap : public VRefCount {
			SymbolFlagMap(TsString symbol_, int tokenFlag_) :
				symbol(symbol_), tokenFlag(tokenFlag_)
			{}
			~SymbolFlagMap() {}

			TsString  symbol;	 //symbol assigned to token
			int			 tokenFlag;  //corresponding addresstoken flag
		};
		typedef refcnt_ptr<SymbolFlagMap> SymbolFlagMapRef;

		//////////////////////////////////////////////////////////////////////
		// Fill the assembly trie with the proper assemBridging structs
		//////////////////////////////////////////////////////////////////////
		void GenerateSymbolTrie();
	
		//////////////////////////////////////////////////////////////////////
		// Create token lists out of parsed addresses
		//////////////////////////////////////////////////////////////////////
		void GenerateTokenLists();

		//////////////////////////////////////////////////////////////////////
		// Stores parsed address tokens, and allows for synchronization with
		// an "original" form.
		//////////////////////////////////////////////////////////////////////
		struct ParsedTokens {
			ParsedTokens() : 
				numberOfMods(0),
				permutations(0)
			{}

			// Clear all of the token lists.
			void Clear();

			// Equality comparison
			bool operator==(const ParsedTokens& rhs) const;

			// Hash the token lists into a hash buffer.
			void Hash(TokenHashBuffer& hashBuffer) const;

			// Lists of tokens for each parsed category.
			TokenList street;
			TokenList unitDesignator;
			TokenList unitNumber;
			TokenList discarded;		// unassigned tokens 

			// Used only when address is NOT an intersection
			TokenList number;
			TokenList predir;
			TokenList prefix;
			TokenList suffix;
			TokenList postdir;
			
			TokenList andTokens;

			TokenList pmbDesignator;
			TokenList pmbNumber;

			// Used only when address IS an intersection
			TokenList predir2;
			TokenList street2;
			TokenList suffix2;
			TokenList postdir2;

			// Number of modifications
			int numberOfMods;

			// Permutations which were applied
			int permutations;
		};

		//////////////////////////////////////////////////////////////////////
		// Convert a ParsedTokens to a AddressParserFirstLine::ParseCandidate
		//////////////////////////////////////////////////////////////////////
		void ParsedTokensToParseCandidate(
			AddressParserFirstLine::ParseCandidate& parseCandidate, 
			const ParsedTokens& parsedTokens,
			bool replaceAliases
		);

		//////////////////////////////////////////////////////////////////////
		// Perform some cleanup of initial parse that is hard for straight pattern-matcher
		//////////////////////////////////////////////////////////////////////
		void CleanupInitialParse();

		//////////////////////////////////////////////////////////////////////
		// Perform permutations on the address, using a specific baseline
		// Inputs:
		//	int				permutationFlags	Bits set in this flag describe the
		//										permutations desired.
		//	ParsedTokens&	baseline			The baseline parsedTokens to permute.
		// Return value:
		//////////////////////////////////////////////////////////////////////
		void PermuteAddress(
			int permutationFlags,
			const ParsedTokens& baseline
		);

		//////////////////////////////////////////////////////////////////////
		// Create a new, cleared out, ParsedTokens for use in parsing
		//////////////////////////////////////////////////////////////////////
		ParsedTokens& GetNewParsedTokens();

		//////////////////////////////////////////////////////////////////////
		// Check the last parse candidate added to the parsedTokens vector, and
		// remove it if it is duplicate.
		//////////////////////////////////////////////////////////////////////
		void CheckNewParseCandidateForDuplication();

		//////////////////////////////////////////////////////////////////////
		// Is a letter a directional?
		//////////////////////////////////////////////////////////////////////
		bool IsDirectionalLetter(unsigned char c) {
			return isDirectional[c];
		}

		// Data directory.  Set in ctor
		TsString dataDir;

		//Regular expression wrapper to provide for patterns
		RegularExprWrapperRef regularExprWrapper;

		// Temp string to be used during string replacement operations
		TsString tempString1, tempString2;

		// Holds list of parsed candidates
		// Note that VectorNoDestruct<> does not destruct its elements when cleared.
		VectorNoDestruct<ParsedTokens> parsedTokens;

		// Maximum number of parsedTokens allowed.  This is reserved ahead of time
		// to avoid reallocation of the vector while the assembly trie points to it.
		enum { 
			MaxParseCandidates = 100		// max candidates and the number to reserve
		};

		// When walking the list of permutations, this is the index of the 
		// next parse candidate to return from NextAddressPermutation()
		unsigned nextCandidateIdx;

		// Data structures used to compared hashed versions of each candidate parse.
		// This is used to eliminate duplicates.  Very simple, but fast and effective
		// because of the normally-small number of candidates.
		HashedCandidateTable hashedCandidates;

		// Object used to allocate memory for token text.
		BulkAllocatorRef bulkAllocator;

		VectorNoDestruct<TokSymCls> addressParse;
		Trie<AssemBridgingRef> assemblyTrie;
		Trie<SymbolFlagMapRef> symbolFlagTrie;
		
		// Lexicon containing standard unabbreviated directionals
		LexiconRef directionalsLexicon;

		// Lexicon containing standard unabbreviated directionals, reversed.
		LexiconRef reverseDirectionalsLexicon;

		// Lexicon containing standard street suffixes, reversed.
		LexiconRef reverseSuffixesLexicon;

		// Lookup table from suffix variants to standard forms.
		LookupTableRef suffixAliasTable;

		// Lookup table from suffix variants to standard forms.
		LookupTableRef directionalAliasTable;

		// Lookup table from suffix variants to standard forms.
		LookupTableRef unitDesignatorAliasTable;

		//LookupTable containing address token symbols
		LookupTableRef addressTokenTable;

		//Lookuptable containing street name aliases
		LookupTableRef streetnameAliasesTable;

		// LookupTable containing street-name prefix aliases
		LookupTableRef streetNamePrefixAliasesTable;

		//Lexicon containing first word of multiword street name aliases
		LexiconRef streetnameMultiwordAliasesLexicon;

		//LookupTable containing multiword street aliases
		LookupTableRef streetnameMultiwordAliasesTable;

		// Lookup table from "numbers"
		LookupTableRef numberAliasTable;

		//Lexicon containing first word of multiword street name aliases
		LexiconRef streetNameMagnetWordsLexicon;

		// Is Puerto-rico specific?
		bool forPuertoRico;

		// Table indicating which characters are directionals.
		bool isDirectional[256];
	};
	
	typedef refcnt_ptr<AddressParserFirstLineImp> AddressParserFirstLineImpRef;
}

#endif

