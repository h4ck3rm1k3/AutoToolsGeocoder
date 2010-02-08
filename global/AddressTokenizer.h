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
// AddressTokenizer.h: Base class used for common implementations
// involving parsing of addresses.
//////////////////////////////////////////////////////////////////////

#ifndef INCL_AddressTokenizer_H
#define INCL_AddressTokenizer_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "LookupTable.h"
#include "Lexicon.h"
#include "BulkAllocator.h"
#include "AddressToken.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class AddressTokenizer : public VRefCount {
	public:
		//////////////////////////////////////////////////////////////////////
		// constructor
		//////////////////////////////////////////////////////////////////////
		AddressTokenizer();

		//////////////////////////////////////////////////////////////////////
		// destructor 
		//////////////////////////////////////////////////////////////////////
		~AddressTokenizer();

		//////////////////////////////////////////////////////////////////////
		// Initialize data structures.
		// Inputs:
		//	const TsString&	dataDir		The directory in which reference data
		//									files are found such as lexicons and 
		//									replacement tables.
		//	BulkAllocatorRef	builkAllocator
		//									Object used to quickly allocate variable-
		//									length memory blocks during record processing.
		// Outputs:
		//	TsString&		errorMsg	If an error occurs during Init(), this
		//									string will hold the error message.
		// Return value:
		//	bool				true on success, false on failure.
		//////////////////////////////////////////////////////////////////////
		bool Init(
			const TsString& dataDir,
			BulkAllocatorRef bulkAllocator,
			TsString& errorMsg
		);

		//////////////////////////////////////////////////////////////////////
		// Cleanup data structures
		//////////////////////////////////////////////////////////////////////
		void Cleanup();

		//////////////////////////////////////////////////////////////////////
		// Reset the bulk allocator in preparation for another parse.
		// This must be separate from Tokenize() because first line and
		// list line are both tokenized at once.
		//////////////////////////////////////////////////////////////////////
		void PrepareForNext() {
			bulkAllocator->Reset();
		}

		//////////////////////////////////////////////////////////////////////
		// Tokenize an input field
		//////////////////////////////////////////////////////////////////////
		void Tokenize(const char* fieldPtr, TokenList& tokenList);

		//////////////////////////////////////////////////////////////////////
		// Reassemble a token list back into a string.  Allocate the string
		// using the bulk allocator.
		// Inputs:
		//  const TokenList&	tokenList		The token list.
		// Return value:
		//	const char*			A string reassambled form the tokens.  Do not
		//						alter or free this string; it will be freed
		//						when the bulk allocator is reset.
		//////////////////////////////////////////////////////////////////////
		const char* Detokenize(const TokenList& tokenList);

		//////////////////////////////////////////////////////////////////////
		// Reassemble a token list back into a string.
		// Inputs:
		//  const TokenList&	tokenList	The token list.
		//	int					resultSize	Size of the results buffer.
		// Outputs:
		//	char*				result		A string reassambled from the tokens.  This
		//									buffer must be large enough to handle the
		//									total text size of all tokens plus the
		//									separating space between each token.
		//									The result will be terminated.
		//////////////////////////////////////////////////////////////////////
		void Detokenize(
			const TokenList& tokenList,
			int resultSize,
			char* result
		) {
			tokenList.ToBuffer(result, resultSize);
		}

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

		//////////////////////////////////////////////////////////////////////
		// Are two directionals compatible?
		//////////////////////////////////////////////////////////////////////
		static bool CompatibleDirectionals(
			const char* tokenText1,
			const char* tokenText2
		) {
			return
				(tokenText1[0] == 'N' || tokenText1[0] == 'S') &&
				(tokenText2[0] == 'E' || tokenText2[0] == 'W');
		}

		//////////////////////////////////////////////////////////////////////
		// Assign basic attributes to a token.
		//////////////////////////////////////////////////////////////////////
		void AssignAttributes(Token& token);

		//////////////////////////////////////////////////////////////////////
		// Assign basic attributes to a list of tokens.
		//////////////////////////////////////////////////////////////////////
		void AssignAttributes(TokenList& tokenList);

	private:
		// Fast allocator for synthesized token text.
		// This will be used to allocate new tokens that have a lifetime
		// of a single record process.  Normally this is a shared reference
		// to an allocator used by the address parser.
		BulkAllocatorRef bulkAllocator;

		// Lookup table for secondary address designators
		//LookupTableRef unitDesignatorAliasTable;

		// Lookup table for state aliases
		LookupTableRef stateAliasTable;

		// Lexicon for directionals
		LexiconRef directionalsLexicon;

		// Lexicon for street suffixes
		//LexiconRef streetSuffixesLexicon;

		// Lexicon for unit designators
		//LexiconRef unitDesignatorsLexicon;

		// Lexicon for single-token street prefixes
		//LexiconRef prefixesLexicon;

		// Lexicon for components of street prefixes
		//LexiconRef prefixTokensLexicon;

		// Lexicon for state names
		LexiconRef stateNamesLexicon;

		// enum denoting the dispoisiton of token break characters
		enum BreakCharAction {
			RemoveBreakChar,		// break char is deleted
			TokenizeBreakChar,		// break char becomes its own token
			ReprocessBreakChar		// backup pointer and process break char again
		};
	};
	typedef refcnt_ptr<AddressTokenizer> AddressTokenizerRef;
}

#endif
