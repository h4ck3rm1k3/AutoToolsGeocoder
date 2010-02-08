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
// AddressTokenizer.cpp:  Class that implements splicing fields from
// multiple inputs.
//////////////////////////////////////////////////////////////////////

#include "Global_Headers.h"
#include "AddressTokenizer.h"
#include <ctype.h>

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Tables and Lexicons
	//////////////////////////////////////////////////////////////////////

	// File containing Lexicon for directionals
	static const char* directionalsFile = "address_parser_directionals.csv";

	// File containing Lexicon for street suffixes
	//static const char* streetSuffixesFile = "suffixes.csv";

	// File containing for secondary address designators
	//static const char* unitDesignatorsFile = "unit_designators.csv";

	// File containing prefix tokens
	//static const char* prefixTokensFile = "prefix_tokens.csv";

	// File containing prefixes
	///static const char* prefixesFile = "prefixes.csv";

	// File containing state names
	static const char* stateNamesFile = "states.csv";

	//////////////////////////////////////////////////////////////////////
	// constructor
	//////////////////////////////////////////////////////////////////////
	AddressTokenizer::AddressTokenizer()
	{
	}

	//////////////////////////////////////////////////////////////////////
	// destructor in case it gets deleted by pointer-to-base
	//////////////////////////////////////////////////////////////////////
	AddressTokenizer::~AddressTokenizer()
	{
	}

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
	bool AddressTokenizer::Init(
		const TsString& dataDir,
		BulkAllocatorRef bulkAllocator_,
		TsString& errorMsg
	) {
		// Get the bulk-allocator to be used for token fragments.
		bulkAllocator = bulkAllocator_;

		// Load lexicons for each token type.
		try {
			TsString tmpStr;

			// Lexicon for directionals
			directionalsLexicon = new Lexicon;
			if (!directionalsLexicon->LoadFromFile(dataDir + "/" + directionalsFile, tmpStr)) {
				throw directionalsFile;
			}

			// Lexicon for street suffixes
			//streetSuffixesLexicon = new Lexicon;
			//if (!streetSuffixesLexicon->LoadFromFile(dataDir + "/" + streetSuffixesFile, tmpStr)) {
			//	throw streetSuffixesFile;
			//}

			// Lexicon for prefix tokens
			//prefixTokensLexicon = new Lexicon;
			//if (!prefixTokensLexicon->LoadFromFile(dataDir + "/" + prefixTokensFile, tmpStr)) {
			//	throw prefixTokensFile;
			//}

			// Lexicon for prefixes
			//prefixesLexicon = new Lexicon;
			//if (!prefixesLexicon->LoadFromFile(dataDir + "/" + prefixesFile, tmpStr)) {
			//	throw prefixesFile;
			//}

			// Lexicon for unit designators
			//unitDesignatorsLexicon = new Lexicon;
			//if (!unitDesignatorsLexicon->LoadFromFile(dataDir + "/" + unitDesignatorsFile, tmpStr)) {
			//	throw unitDesignatorsFile;
			//}

			// Lexicon for state names
			stateNamesLexicon = new Lexicon;
			if (!stateNamesLexicon->LoadFromFile(dataDir + "/" + stateNamesFile, tmpStr)) {
				throw stateNamesFile;
			}
		} catch (const char* badFileName) {
			errorMsg = "Error reading data file " + dataDir + "/" + badFileName;
			Cleanup();
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Release any resources that were in use during processing.
	///////////////////////////////////////////////////////////////////////////////
	void AddressTokenizer::Cleanup()
	{
		bulkAllocator = 0;
		directionalsLexicon = 0;
		//streetSuffixesLexicon = 0;
		//unitDesignatorsLexicon = 0;
		//prefixTokensLexicon = 0;
		//prefixesLexicon = 0;
		stateNamesLexicon = 0;
	}

	//////////////////////////////////////////////////////////////////////
	// Tokenize an input field
	//////////////////////////////////////////////////////////////////////
	void AddressTokenizer::Tokenize(
		const char* fieldPtr_, 
		TokenList& tokenList
	) {

		char* fieldPtr = (char*)bulkAllocator->New(strlen(fieldPtr_) + 1);
		// Change everything to uppercase.
		{
			const char* ptr = fieldPtr_;
			char* ptr2 = fieldPtr;
			while (*ptr != 0) {
				*ptr2++ = toupper(*ptr++);
			}
			*ptr2 = 0;
		}

		tokenList.clear();

		Token token;

		const char* tokenBegin = fieldPtr;
		bool tokenBreak = false;
		BreakCharAction breakCharAction = RemoveBreakChar;
		{for (const char* ptr = fieldPtr; true; ptr++) {
			// Pretend that we have a space at the end of each input field to
			// make tokenizing easier.
			switch (*ptr) {
			case '.':
			case '/':
			case ';':
			case ':':
			case ',':
			case ' ':
			case 0:
				tokenBreak = true;
				break;
			case '-':
				// Break digits for Zip+4, but not anything else.
				tokenBreak = (isdigit(ptr[1]) != 0);
				break;
			case '#':
			case '&':
				tokenBreak = true;
				breakCharAction = TokenizeBreakChar;
				break;
			case '\'':
				// Prefix separator like L'ARGONNE.  Should only break when
				// token will be D' or L'.  Avoid breaking e.g. O'CONNER
				if (
					ptr - tokenBegin == 1 &&
					(*tokenBegin == 'D' || *tokenBegin == 'L')
				) {
					// In this case we want to include the ' in the first token and
					// split from the second token:  L' ARGONNE
					tokenBreak = true;
					// But break at the following character...
					ptr++;
					// ... and arrange to reprocess the following character
					breakCharAction = ReprocessBreakChar;
				}
				break;
			default:
				break;
			}

			if (tokenBreak) {
				if (ptr == tokenBegin) {
					// Still skipping whitespace before the token.
					tokenBegin++;
				} else {
					// End of token.  Record the token without the trailing break.
					SetTokenText(token, tokenBegin, int(ptr - tokenBegin));
					tokenList.push_back(token);
					// Start of next token
					tokenBegin = ptr + 1;
				}
				if (breakCharAction == RemoveBreakChar) {
					// Do nothing.
				} else if (breakCharAction == TokenizeBreakChar) {
					// Create a token from the just-skipped punctuation char.
					SetTokenText(token, ptr, 1);
					tokenList.push_back(token);
				} else {
					// ReprocessBreakChar
					// Back up to process this char again.
					tokenBegin--;
					ptr--;
				}

				tokenBreak = false;
				breakCharAction = RemoveBreakChar;

				// Put test here instead of loop, because we want to process the null-termination.
				if (*ptr == 0) {
					break;
				}
			}
		}}
	}

	//////////////////////////////////////////////////////////////////////
	// Reassemble a token list back into a string.  Allocate the string
	// using the bulk allocator.
	// Inputs:
	//  const TokenList&	tokenList		The token list.
	// Return value:
	//	const char*			A terminated string reassambled form the tokens.
	//						Do not alter or free this string; it will be 
	//						freed when the bulk allocator is reset.
	//////////////////////////////////////////////////////////////////////
	const char* AddressTokenizer::Detokenize(
		const TokenList& tokenList
	) {
		int bufSize = 1;
		{for (unsigned i = 0; i < tokenList.size(); i++) {
			bufSize += tokenList[i].size + 1;
		}}
		char* buf = (char*)bulkAllocator->New(bufSize);
		Detokenize(tokenList, bufSize, buf);
		return buf;
	}

	//////////////////////////////////////////////////////////////////////
	// Assign basic attributes to a token.  This includes all attributes
	// except IsStreetPrefix, IsStreetComponent, IsStreetName, 
	// IsCityPrefix, IsCityComponent, IsCityName
	//////////////////////////////////////////////////////////////////////
	void AddressTokenizer::AssignAttributes(Token& token)
	{
		// Test for directionals
		if (directionalsLexicon->Find(token.text)) {
			token.flags |= Token::IsDirectional;
		}

		// Test for street suffix
		//if (streetSuffixesLexicon->Find(token.text)) {
		//	token.flags |= Token::IsStreetSuffix;
		//}

		// Test for unit designators
		//if (unitDesignatorsLexicon->Find(token.text)) {
		//	token.flags |= Token::IsUnitDesignator;
		//}

		// Test for state names
		if (stateNamesLexicon->Find(token.text)) {
			token.flags |= Token::IsState;
		}

		// Test for prefix components
		//if (prefixTokensLexicon->Find(token.text)) {
		//	token.flags |= Token::IsPrefixToken;
		//}

		// Test for prefix
		//if (prefixesLexicon->Find(token.text)) {
		//	token.flags |= Token::IsPrefix;
		//}

		// Test for digit sequences
		token.flags |= Token::IsNumber;
		{for (int i = 0; i < token.size; i++) {
			if (!isdigit(token.text[i])) {
				token.flags &= ~Token::IsNumber;
			} else {
				token.flags |= Token::HasDigit;
				if (i == 4 && (token.flags & Token::IsNumber) != 0) {
					token.flags |= Token::HasLeading5Digits;
				}
			}
		}}

		switch (token.size) {
		case 2:
			// Half-Fraction 
			if (token.text[0] == '/' && isdigit(token.text[1])) {
				token.flags |= Token::IsFraction;
			}
			break;

		case 3:
			// Canadian half-zip
			if (
				isalpha(token.text[0]) && 
				isdigit(token.text[1]) && 
				isalpha(token.text[2])
			) {
				// First part of CA ZIP e.g. A1B
				token.flags |= Token::IsCaZip1;
			} else if (
				isdigit(token.text[0]) && 
				isalpha(token.text[1]) && 
				isdigit(token.text[2])
			) {
				// Second part of CA ZIP e.g. 3D4
				token.flags |= Token::IsCaZip2;
			}
			break;
		case 4:
			if (
				isalpha(token.text[0]) && 
				isdigit(token.text[1]) && 
				isalpha(token.text[2]) &&
				isdigit(token.text[3])
			) {
				// First part of CA ZIP
				token.flags |= Token::IsCaZip3;
			}
			break;
		case 5:
			// US ZIP code
			if ((token.flags & Token::IsNumber) != 0) {
				token.flags |= Token::IsZip;
			}
			break;
		case 6:
			// Canadian Full ZIP code
			if (
				isalpha(token.text[0]) && 
				isdigit(token.text[1]) && 
				isalpha(token.text[2]) && 
				isdigit(token.text[3]) && 
				isalpha(token.text[4]) && 
				isdigit(token.text[5])
			) {
				token.flags |= Token::IsCaZip;
			}
			break;
		// We don't need this because last-line tokens are split at '-'
//		case 7:
//			// Canadian Full ZIP code
//			if (
//				isalpha(token.text[0]) && 
//				isdigit(token.text[1]) && 
//				isalpha(token.text[2]) && 
//				token.text[3] == '-' && 
//				isdigit(token.text[4]) && 
//				isalpha(token.text[5]) && 
//				isdigit(token.text[6])
//			) {
//				token.flags |= Token::IsCaZip;
//			}
//			break;
		case 9:
			if ((token.flags & Token::IsNumber) != 0) {
				token.flags |= Token::Is9Digit;
			}
			break;

		case 10:
			// Test for ZIP+4 format
			if (
				token.text[5] == '-' &&
				(token.flags & Token::HasLeading5Digits) != 0
			) {
				token.flags |= Token::Is5Dash4Digit;
				for (int i = 6; i < 10; i++) {
					if (!isdigit(token.text[i])) {
						token.flags &= ~Token::Is5Dash4Digit;
						break;
					}
				}
			}
			break;
		}


		// Test for fraction like 1/2
		// or hyphenated numbers
		if (token.size >= 3 && isdigit(token.text[0])) {
			int i = 1;
			while (i < token.size && isdigit(token.text[i])) {
				i++;
			}
			if (i < token.size - 1) {
				switch (token.text[i]) {
				case '/':
					i++;
					while (i < token.size && isdigit(token.text[i])) {
						i++;
					}
					if (i == token.size) {
						// Found one!
						token.flags |= Token::IsFraction;
					}
					break;
				case '-':
					i++;
					while (i < token.size && isdigit(token.text[i])) {
						i++;
					}
					if (i == token.size) {
						// Found one!
						token.flags |= Token::IsNumberDashNumber;
					}
					break;
				case '.':
					i++;
					while (i < token.size && isdigit(token.text[i])) {
						i++;
					}
					if (i == token.size) {
						// Found one!
						token.flags |= Token::IsNumberDotNumber;
					}
					break;
				default:
					break;
				}
			}
		} 

		// We don't try to identify address ranges with patterns, because
		// addresses ranges are so varied that an encompassing pattern would
		// match everything.
	}

	//////////////////////////////////////////////////////////////////////
	// Assign basic attributes to a list of tokens.
	//////////////////////////////////////////////////////////////////////
	void AddressTokenizer::AssignAttributes(TokenList& tokenList)
	{
		for (unsigned i = 0; i < tokenList.size(); i++) {
			AssignAttributes(tokenList[i]);
		}
	}

}
