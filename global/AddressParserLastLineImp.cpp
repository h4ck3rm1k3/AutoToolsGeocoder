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
# $Rev: 79 $ 
# $Date: 2007-02-21 23:16:39 +0100 (Wed, 21 Feb 2007) $ 
*/

//////////////////////////////////////////////////////////////////////
// AddressParserLastLineImp.cpp:  Class that implements parsing of address lines
// into possible interpretations.
//////////////////////////////////////////////////////////////////////

#include "Global_Headers.h"
#include "AddressParserLastLineImp.h"

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Static variables
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Lookup tables
	//////////////////////////////////////////////////////////////////////

	// State aliases
	static const char* stateAliasFile = "address_parser_state_aliases.csv";
	// City component aliases
	static const char* cityComponentAliasFile = "address_parser_city_component_aliases.csv";

	//////////////////////////////////////////////////////////////////////
	// constructor
	//////////////////////////////////////////////////////////////////////
	AddressParserLastLineImp::AddressParserLastLineImp()
	{
	}

	//////////////////////////////////////////////////////////////////////
	// destructor in case it gets deleted by pointer-to-base
	//////////////////////////////////////////////////////////////////////
	AddressParserLastLineImp::~AddressParserLastLineImp()
	{
	}

	//////////////////////////////////////////////////////////////////////
	// Initialize the address parser.
	// Inputs:
	//	const TsString&	dataDir		The directory containing data files.
	// Outputs:
	//	std::String&		errorMsg	The message if an error occured.
	// Return value:
	//	bool	true on success, false on error.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserLastLineImp::Open(
		const TsString& dataDir_,
		TsString& errorMsg
	) {
		dataDir = dataDir_;

		bulkAllocator = new BulkAllocator;

		// Load alias translation tables and lexicons

		// state aliases
		stateAliasTable = new LookupTable;
		if (!stateAliasTable->LoadFromFile(dataDir + "/" + stateAliasFile, errorMsg)) {
			Close();
			return false;
		}

		// city component aliases.
		cityComponentAliasTable = new LookupTable;
		if (!cityComponentAliasTable->LoadFromFile(dataDir + "/" + cityComponentAliasFile, errorMsg)) {
			// This is OK,  This table is optional.
		}

		addressTokenizer = new AddressTokenizer();
		if (!addressTokenizer->Init(dataDir, bulkAllocator, errorMsg)) {
			Close();
			return false;
		}

		// All systems go...
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Release any resources that were in use during processing.
	///////////////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::Close()
	{
		stateAliasTable = 0;
		cityComponentAliasTable = 0;

		if (addressTokenizer != 0) {
			addressTokenizer->Cleanup();
			addressTokenizer = 0;
		}
		bulkAllocator = 0;
		inputTokenLists.clear();
	}

	//////////////////////////////////////////////////////////////////////
	// Read an address, generate tokens, and perform preprocessing of
	// address tokens.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserLastLineImp::Parse(
		const char* lastLine, 
		AddressParserLastLine::ParseCandidate& parseCandidate, 
		bool replaceAliases
	) {
		// Reset the allocator and free memory used by last parse
		bulkAllocator->Reset();
		// Clear other data structures
		inputTokenLists.clear();

		// First permutation candidate will start at 1, because the first parsing is
		// the one we will return now.
		nextCandidateIdx = 1;

		// Turn the raw input into tokens.
		TokenListAndFlags& newTokenList = inputTokenLists.UseExtraOnEnd();
		newTokenList.Clear();
		addressTokenizer->Tokenize(lastLine, newTokenList.tokenList);
		// Test each token against patterns and reference information.
		addressTokenizer->AssignAttributes(newTokenList.tokenList);

		// Perform permutations to the token lists before parsing.
		for (unsigned tokenListIdx = 0; tokenListIdx < inputTokenLists.size(); tokenListIdx++) {
			// **** Important:
			// Make sure we have enough extra space to avoid reallocation of the 
			// vector storage when we do an append, otherwise temp reference
			// variables will become invalid.

			// Make sufficient room in parsed tokens vector to avoid reallocation when
			// new entries are added.  But avoid reallocation on every loop by  forcing
			// a minimum increment.
			if (inputTokenLists.capacity() - inputTokenLists.size() < 4) {
				inputTokenLists.reserve(inputTokenLists.size() + 8);
			}

            /*
			const TokenListAndFlags& tokenListAndFlags = inputTokenLists[tokenListIdx];
			for (unsigned tokenIdx = 0; tokenIdx < tokenListAndFlags.tokenList.size(); tokenIdx++) {

				const Token& token = tokenListAndFlags.tokenList[tokenIdx];

				// 
				// TODO: Perform numeric-text conversions
				//

			}	// for token
            */
		}	// for token list

		// Clear the data structure used to detect duplicates
		hashedCandidates.Clear();

		// Parse each token list using a "best guess" heuristic.
		parsedTokens.clear();
		{for (unsigned i = 0; i < inputTokenLists.size(); i++) {
			parsedTokens.UseExtraOnEnd();
			ParseGuess(inputTokenLists[i], parsedTokens.Last());
			CheckNewParseCandidateForDuplication();
		}}

		if (parsedTokens.empty()) {
			return false;
		} else {
			ParsedTokensToParseCandidate(parseCandidate, parsedTokens[0], replaceAliases);
			return true;
		}
	}


	//////////////////////////////////////////////////////////////////////
	// Perform permutations on the address.
	// Inputs:
	//	int				permutationFlags Bits set in this flag describe the
	//                                   permutations desired.
	// Return value:
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::PermuteAddress(
		int permutationFlags
	) {
		// First permutation candidate will start at 1, because the first parsing is
		// the one we will return now.
		nextCandidateIdx = 1;

		//
		// Permutate each parse candidate using some standard and relatively innocuous
		// perturbations to each parse candidate.
		//
		for (unsigned parsedTokensIdx = 0; parsedTokensIdx < parsedTokens.size(); parsedTokensIdx++) {

			// **** Important:
			// Make sure we have enough extra space to avoid reallocation of the 
			// vector storage when we do an append, otherwise temp reference
			// variables will become invalid.

			// Make sufficient room in parsed tokens vector to avoid reallocation when
			// new entries are added.  But avoid reallocation on every loop by  forcing
			// a minimum increment.
			if (parsedTokens.capacity() - parsedTokens.size() < 15) {
				parsedTokens.reserve(parsedTokens.size() + 30);
			}

			const ParsedTokens& currentParsedTokens = parsedTokens[parsedTokensIdx];

			if ((permutationFlags & AddressParserLastLine::PermuteShortenLongCityName) != 0) {
				// Remove tokens from end of long city name.
				if (currentParsedTokens.city.size() >= 3) {
					ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
					newParsedTokens = currentParsedTokens;
					// Reduce it to at most four tokens.
					if (newParsedTokens.city.size() > 4) {
						newParsedTokens.city.resize(4);
					} else {
						newParsedTokens.city.pop_back();
					}
					newParsedTokens.flags |= AddressParserLastLine::ParseCandidate::ShortenedCity;
					newParsedTokens.numberOfMods++;
					CheckNewParseCandidateForDuplication();
				}

				// Remove tokens from front of long city name.
				if (currentParsedTokens.city.size() >= 3) {
					ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
					newParsedTokens = currentParsedTokens;
					// Reduce it to at most four tokens.
					if (newParsedTokens.city.size() > 4) {
						newParsedTokens.city.erase(
							newParsedTokens.city.begin(),
							newParsedTokens.city.begin() + newParsedTokens.city.size() - 4
						);
					} else {
						newParsedTokens.city.erase(newParsedTokens.city.begin());
					}
					newParsedTokens.flags |= AddressParserLastLine::ParseCandidate::ShortenedCity;
					newParsedTokens.numberOfMods++;
					CheckNewParseCandidateForDuplication();
				}
			}

			if ((permutationFlags & AddressParserLastLine::PermuteCityComponentAlias) != 0) {
				const char* ptr;
				const char* ptr2;
				const char* replacement;
				// Change components of city e.g. "ST" to "SAINT"
				if (currentParsedTokens.city.size() >= 2) {
					for (unsigned i = 0; i < currentParsedTokens.city.size(); i++) {
						if (cityComponentAliasTable->Find(currentParsedTokens.city[i].text, replacement)) {
							ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
							newParsedTokens = currentParsedTokens;
							SetTokenText(newParsedTokens.city[i], replacement);
							newParsedTokens.flags |= AddressParserLastLine::ParseCandidate::CityAlias;
							newParsedTokens.numberOfMods++;
							CheckNewParseCandidateForDuplication();
						}
					}
				} else if (
					currentParsedTokens.city.size() == 1 &&
					(ptr = strchr(currentParsedTokens.city[0].text, '-')) != 0
				) {
					tempStr1.assign(currentParsedTokens.city[0].text, ptr);
					if (cityComponentAliasTable->Find(tempStr1.c_str(), replacement)) {
						tempStr2 = replacement;
						tempStr2 += '-';
						tempStr2 += ptr + 1;
						ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
						newParsedTokens = currentParsedTokens;
						SetTokenText(newParsedTokens.city[0], tempStr2.c_str());
						newParsedTokens.flags |= AddressParserLastLine::ParseCandidate::CityAlias;
						newParsedTokens.numberOfMods++;
						CheckNewParseCandidateForDuplication();
					} else if (
						tempStr1.assign(ptr + 1, currentParsedTokens.city[0].text + currentParsedTokens.city[0].size),
						cityComponentAliasTable->Find(tempStr1.c_str(), replacement)
					) {
						tempStr2.assign(currentParsedTokens.city[0].text, ptr + 1);
						tempStr2 += replacement;
						ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
						newParsedTokens = currentParsedTokens;
						SetTokenText(newParsedTokens.city[0], tempStr2.c_str());
						newParsedTokens.flags |= AddressParserLastLine::ParseCandidate::CityAlias;
						newParsedTokens.numberOfMods++;
						CheckNewParseCandidateForDuplication();
					} else if (
						(ptr2 = strchr(ptr+1, '-')) != 0 &&
						(
							tempStr1.assign(ptr+1, ptr2),
							cityComponentAliasTable->Find(tempStr1.c_str(), replacement)
						)
					) {
						// Have two dashes with alias in the center
						tempStr2.assign(currentParsedTokens.city[0].text, ptr+1);
						tempStr2 += replacement;
						tempStr2 += ptr2;
						ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
						newParsedTokens = currentParsedTokens;
						SetTokenText(newParsedTokens.city[0], tempStr2.c_str());
						newParsedTokens.flags |= AddressParserLastLine::ParseCandidate::CityAlias;
						newParsedTokens.numberOfMods++;
						CheckNewParseCandidateForDuplication();
					}
				}
			}

		}	// candidate loop

	}


	//////////////////////////////////////////////////////////////////////
	// Retrieve the next address permutation.
	// Return value:
	//	bool	true if the next permutation is returned, false
	//			if there are no more permutations.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserLastLineImp::NextAddressPermutation(
		AddressParserLastLine::ParseCandidate& parseCandidate,
		bool replaceAliases
	) {
		if (nextCandidateIdx < int(parsedTokens.size())) {
			ParsedTokensToParseCandidate(
				parseCandidate, 
				parsedTokens[nextCandidateIdx],
				replaceAliases
			);
			nextCandidateIdx++;
			return true;
		} else {
			return false;
		}
	}


	//////////////////////////////////////////////////////////////////////
	// Perform a first-cut parse of the tokens using "rules of thumb" 
	// for splitting the address into components.
	// Inputs:
	//	const TokenListAndFlags&	tokenListAndFlags	The token list to parse
	// Outputs:
	//	ParsedTokens&				parsedTokens		The resulting list of token
	//													positions.
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::ParseGuess(
		const TokenListAndFlags& tokenListAndFlags,
		ParsedTokens& parsedTokens
	) {
		const TokenList& tokenList = tokenListAndFlags.tokenList;
		int numberOfTokens = int(tokenList.size());

		char tmpBuffer[100];

		// Start with an empty result.
		parsedTokens.Clear();
		parsedTokens.flags = tokenListAndFlags.flags;
		parsedTokens.numberOfMods = tokenListAndFlags.numberOfMods;

		// If empty, then nothing can be parsed
		if (numberOfTokens == 0) {
			return;
		}
		
		int nIndex = (int)tokenList.size() - 1;

		// Look for ZIP code
		bool foundZip = false;

		// First look for well-known ZIP formats
		while (!foundZip && nIndex >= 0) {
			// Full Zip
			// e.g. 12345 or 123456789 or A1B2 or A1B2C3
			if ((tokenList[nIndex].flags & Token::IsFullZip) != 0) {
				if (isdigit(tokenList[nIndex].text[0])) {
					// USA
					if ((tokenList[nIndex].flags & Token::Is9Digit) != 0) {
						// e.g. 123456789.  Must split token.
						// First 5 are Zip
						Token newToken(tokenList[nIndex]);
						SetTokenText(newToken, tokenList[nIndex].text, 5);
						newToken.flags = (Token::IsZip | Token::IsNumber | Token::HasDigit);
						parsedTokens.postcode.push_back(newToken);
						// Remainder is postcode extension.
						SetTokenText(newToken, tokenList[nIndex].text + 5);
						newToken.flags = (Token::IsNumber | Token::HasDigit);
						parsedTokens.postcodeExt.push_back(newToken);
					} else {
						parsedTokens.postcode.push_back(tokenList[nIndex]);
						if (nIndex < (int)tokenList.size() - 1 &&
							(tokenList[nIndex + 1].flags & Token::IsNumber) != 0 &&
							tokenList[nIndex + 1].size == 4
						) {
							// Has Zip code extension
							parsedTokens.postcodeExt.push_back(tokenList[nIndex + 1]);
						}
					}
				} else {
					// Canada.
					// First three are postcode
					Token newToken(tokenList[nIndex]);
					SetTokenText(newToken, tokenList[nIndex].text, 3);
					newToken.flags |= Token::IsZip;
					parsedTokens.postcode.push_back(newToken);
					if (tokenList[nIndex].size == 6) {
						// Remainder is postcode extension.
						SetTokenText(newToken, tokenList[nIndex].text + 3);
						parsedTokens.postcodeExt.push_back(newToken);
					}
				}
				foundZip = true;
			} else if ((tokenList[nIndex].flags & Token::IsCaZip1) != 0) {
				foundZip = true;
				if (nIndex < (int)tokenList.size() - 1 &&
					(tokenList[nIndex + 1].flags & Token::IsCaZip2) != 0
				) {
					// e.g. K1H 2N3
					// Second token
					Token newToken(tokenList[nIndex + 1]);
					parsedTokens.postcodeExt.push_back(newToken);
					// First token
					Token newToken2(tokenList[nIndex]);
					newToken2.flags |= Token::IsZip;
					parsedTokens.postcode.push_back(newToken2);
				} else {
					// e.g. K1H 
					parsedTokens.postcode.push_back(tokenList[nIndex]);
				}
			}
			nIndex--;
		}

		if (!foundZip) {
			// Try again, look for numeric zip codes that are not padded properly,
			// e.g 2164 should be 02164
			nIndex = (int)tokenList.size() - 1;
		}
		while (!foundZip && nIndex >= 0) {
			if ((tokenList[nIndex].flags & Token::IsNumber) != 0) {
				// Found number.  Make sure there is not another one preceding.
				if (nIndex > 0 && (tokenList[nIndex - 1].flags & Token::IsNumber) != 0) {
					// This must be postcode extension
					parsedTokens.postcodeExt.push_back(tokenList[nIndex]);
					// Use the previous number instead
					nIndex--;
				}
				char buf[10];
				strncpy(buf, tokenList[nIndex].text, sizeof(buf));
				buf[sizeof(buf) - 1] = 0;

				int len = int(strlen(buf));
				if (len > 5) {
					// Drop last four characters of the ZIP
					// Note we always subtact four because e.g. 21641234 should become 02164-1234
					// Dropped chars become extension.
					Token newToken(tokenList[nIndex]);
					SetTokenText(newToken, buf + len - 4);
					parsedTokens.postcodeExt.clear();
					parsedTokens.postcodeExt.push_back(newToken);
					// Remaining characters become postcode
					len -= 4;
				}

				// Pad with leading zeros up to length five
				char buf2[6];
				assert(len <= 5);
				memset(buf2, '0', 5 - len);
				memcpy(buf2 + 5 - len, buf, len);
				buf2[5] = 0;

				foundZip = true;
				Token newToken(tokenList[nIndex]);
				SetTokenText(newToken, buf2, 5);
				parsedTokens.postcode.push_back(newToken);
				nIndex--;
				break;
			}
			parsedTokens.discarded.push_back(tokenList[nIndex]);
			nIndex--;
		}

		if (!foundZip) {
			// No ZIP code; start over at end looking for state
			nIndex = (int)tokenList.size() - 1;
			parsedTokens.discarded.clear();
		}

		// Look for state name preceeding ZIP, using up to three tokens.
		const char* stateAlias = 0;
		int cityEndIndex = nIndex;
		bool exactStateMatch = false;
		{
			int endIndex = nIndex;
			int startIndex = (nIndex >= 2 ? nIndex - 2 : 0);
			while (stateAlias == 0 && startIndex >= 0 && startIndex <= endIndex) {
				// Construct the available state tokens (up to 3) into the temp buffer,
				// working from back to front.  Compare each construction against the
				// state alias table.
				char* ptr = tmpBuffer + sizeof(tmpBuffer) - 1;
				*ptr = 0;
				for (int tokenIndex = endIndex;
					tokenIndex >= startIndex && ptr - tmpBuffer > tokenList[tokenIndex].size;
					--tokenIndex
				) {
					ptr -= tokenList[tokenIndex].size;
					memcpy(ptr, tokenList[tokenIndex].text, tokenList[tokenIndex].size);
					// Compare to state alias table
					const char* tmp;
					if (stateAliasTable->Find(ptr, tmp)) {
						stateAlias = tmp;
						cityEndIndex = tokenIndex - 1;
						// Found an alias
						exactStateMatch = (strlen(ptr) == 2);
						if (exactStateMatch) {
							// Don't keep looking at preceding tokens if we matched an 
							// abbreviation directly.
							break;
						}
					}
					*(--ptr) = ' ';
				}

				if (stateAlias == 0) {
					// Not found yet -- track discarded tokens.
					parsedTokens.discarded.push_back(tokenList[endIndex]);
					// Back up token window
					endIndex--;
					if (startIndex > 0) {
						startIndex--;
					}
				}
			}
		}
		if (stateAlias != 0) {
			Token newToken(stateAlias, Token::IsState);
			parsedTokens.state.push_back(newToken);
			if (!exactStateMatch) {
				parsedTokens.flags |= AddressParserLastLine::ParseCandidate::StateAlias;
				parsedTokens.numberOfMods++;
			}
		}

		// Everything else should be city
		{for (
			int tokenIndex = 0;
			tokenIndex <= cityEndIndex;
			++tokenIndex
		) {
			parsedTokens.city.push_back(tokenList[tokenIndex]);
		}}

		// If nothing in city, use discarded tokens
		if (parsedTokens.city.empty() && !parsedTokens.discarded.empty()) {
			parsedTokens.flags |= AddressParserLastLine::ParseCandidate::Reorder;
			parsedTokens.numberOfMods++;
			parsedTokens.city = parsedTokens.discarded;
			parsedTokens.discarded.clear();
		}
	}


	//////////////////////////////////////////////////////////////////////
	// Check the last parse candidate added to the parsedTokens vector, and
	// remove it if it is duplicate.
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::CheckNewParseCandidateForDuplication()
	{
		TokenHashBuffer hashBuffer;
		parsedTokens.Last().Hash(hashBuffer);
		if (hashedCandidates.Find(hashBuffer)) {
			// Candidate is already in table
			parsedTokens.pop_back();
		} else {
			// Candidate is new; add to hash candidate table
			hashedCandidates.Add(hashBuffer);
		}
	}


	//////////////////////////////////////////////////////////////////////
	// Convert a ParsedTokens to a ParseCandidate
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::ParsedTokensToParseCandidate(
		AddressParserLastLine::ParseCandidate& parseCandidate, 
		const ParsedTokens& parsedTokens,
		bool replaceAliases
	) {
		parseCandidate.flags = parsedTokens.flags;
		parseCandidate.numberOfMods = parsedTokens.numberOfMods;

		parsedTokens.city.ToBuffer(parseCandidate.city, sizeof(parseCandidate.city));
		// Note: state does not need de-aliasing; that was done during parse.
		parsedTokens.state.ToBuffer(parseCandidate.state, sizeof(parseCandidate.state));
		parsedTokens.postcode.ToBuffer(parseCandidate.postcode, sizeof(parseCandidate.postcode));
		parsedTokens.postcodeExt.ToBuffer(parseCandidate.postcodeExt, sizeof(parseCandidate.postcodeExt));
	}


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ParsedTokens methods.
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Clear all of the token lists.
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::ParsedTokens::Clear() {
		// Use reset, because we want to mark everything as unchanged.
		city.clear();
		state.clear();
		postcode.clear();
		postcodeExt.clear();
		discarded.clear();
		flags = AddressParserLastLine::ParseCandidate::None;
		numberOfMods = 0;
	}

	//////////////////////////////////////////////////////////////////////
	// equality comparison
	//////////////////////////////////////////////////////////////////////
	bool AddressParserLastLineImp::ParsedTokens::operator==(const ParsedTokens& rhs) const {
		return 
			city == rhs.city &&
			state == rhs.state &&
			postcode == rhs.postcode;
	}

	//////////////////////////////////////////////////////////////////////
	// Hash the token lists into the given hash buffer.
	//////////////////////////////////////////////////////////////////////
	void AddressParserLastLineImp::ParsedTokens::Hash(TokenHashBuffer& hashBuffer) const {
		hashBuffer.Clear();
		hashBuffer.Hash(city, 1);
		hashBuffer.Hash(state, 10);
		hashBuffer.Hash(postcode, 15);
		hashBuffer.Hash(postcodeExt, 23);
	}

}	// namespace 




