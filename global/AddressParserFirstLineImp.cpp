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
// AddressParserFirstLineImp.cpp:  Class that implements parsing of address lines
// into possible interpretations.
//////////////////////////////////////////////////////////////////////

#include "Global_Headers.h"
#include "AddressParserFirstLineImp.h"

#include <algorithm>

namespace PortfolioExplorer {


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Given a token list, and output buffer, and an alias table, 
	// convert the token list to the output buffer, transforming via
	// the alias table if possible.
	// Return value:
	//	bool		true if the tokens were replaced with a standard aliases,
	//				false if the tokens were output directly.
	//////////////////////////////////////////////////////////////////////
	static bool DeAliasToBuffer(
		char* buffer,
		int bufSize,
		LookupTableRef aliasTable,
		const TokenList& tokenList
	) {
		char tmpBuf[100];
		const char *tmpPtr;

		tokenList.ToBuffer(tmpBuf, sizeof(tmpBuf));
		if (aliasTable->Find(tmpBuf, tmpPtr)) {
			// Copy alias to output buffer
			bufSize--;		// Room for termination.
			int length = int(strlen(tmpPtr));
			length = (length > bufSize) ? bufSize : length;
			memcpy(buffer, tmpPtr, length);
			buffer[length] = 0;
			return true;
		} else {
			tokenList.ToBuffer(buffer, bufSize);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////
	// A useful function for reversing the text of a token
	//////////////////////////////////////////////////////////////////////
	static inline void ReverseTokenText(
		TsString& result,
		const Token& token
	) {
		result = "";
		for (int i = token.size - 1; i >= 0; i--) {
			result += (char)token.text[i];
		}
	}

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// Static variables
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////
	// Lookup tables and lexicons
	//////////////////////////////////////////////////////////////////////

	// File containing Lexicon for directionals
	static const char* directionalsFile = "address_parser_directionals.csv";

	// File containing Lexicon for reversed directionals
	static const char* reverseDirectionalsFile = "address_parser_reverse_full_directionals.csv";

	// File containing Lexicon for reversed suffixes
	static const char* reverseSuffixesFile = "address_parser_reverse_suffixes.csv";

	// File containing Lookup Table for suffix aliases
	static const char* suffixAliasFile = "address_parser_suffix_aliases.csv";

	// File containing Lookup table for directional aliases
	static const char* directionalAliasFile = "address_parser_directional_aliases.csv";

	// File containing Lookup table unit designator aliases
	static const char* unitDesignatorAliasFile = "address_parser_unit_designator_aliases.csv";

	// File containing pattern tools configuration
	static const char* patternsConfigurationFileNonPr = "address_parser_patterns.dlp";
	// PR-specific
	static const char* patternsConfigurationFilePr = "address_parser_patterns_pr.dlp";

	// File containing address tokens
	static const char* addressTokenFileNonPr = "address_parser_address_token_table.csv";
	// specific to PR
	static const char* addressTokenFilePr = "address_parser_address_token_table_pr.csv";

	// Following are streetname alias files

	// File containing single word streetname aliases
	static const char* streetnameAliasesFile = "address_parser_streetname_aliases.csv";

	// File containing first word lookup lexicon for multiword streetname aliases
	static const char* streetnameMultiwordSearchAliasesFile = "address_parser_streetname_multiword_search_aliases.csv";

	// File containing multiword lookup table
	static const char* streetnameMultiwordAliasesFile = "address_parser_streetname_multiword_aliases.csv";

	// File containing street prefix alias lookup table
	static const char* streetNamePrefixAliasesFile = "address_parser_streetname_prefix_dash_aliases.csv";

	// File containing Lookup Table for number aliases
	static const char* numberAliasFile = "address_parser_number_aliases.csv";

	// File containing magnet streetname words including PR
	static const char* streetNameMagnetWordsFile = "address_parser_magnet_street_words.csv";

	//////////////////////////////////////////////////////////////////////
	// constructor
	//////////////////////////////////////////////////////////////////////
	AddressParserFirstLineImp::AddressParserFirstLineImp() :
		forPuertoRico(false)
	{
		for (int i = 0; i < 256; i++) {
			switch (toupper(i)) {
			case 'N':
			case 'S':
			case 'E':
			case 'W':
				isDirectional[i] = true;
				break;
			default:
				isDirectional[i] = false;
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////
	// destructor in case it gets deleted by pointer-to-base
	//////////////////////////////////////////////////////////////////////
	AddressParserFirstLineImp::~AddressParserFirstLineImp()
	{
		Close();
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
	bool AddressParserFirstLineImp::Open(
		const TsString& dataDir_,
		TsString& errorMsg
	) {
		dataDir = dataDir_;

		bulkAllocator = new BulkAllocator;

		regularExprWrapper = new RegularExprWrapper;

		try {
			// Load alias translation tables and lexicons

			// unabbreviated directionals 
			directionalsLexicon = new Lexicon;
			if (!directionalsLexicon->LoadFromFile(dataDir + "/" + directionalsFile, errorMsg)) {
				throw 1;
			}

			// reversed unabbreviated directionals 
			reverseDirectionalsLexicon = new Lexicon;
			if (!reverseDirectionalsLexicon->LoadFromFile(dataDir + "/" + reverseDirectionalsFile, errorMsg)) {
				throw 1;
			}

			// reversed suffixes
			reverseSuffixesLexicon = new Lexicon;
			if (!reverseSuffixesLexicon->LoadFromFile(dataDir + "/" + reverseSuffixesFile, errorMsg)) {
				throw 1;
			}

			// suffix aliases
			suffixAliasTable = new LookupTable;
			if (!suffixAliasTable->LoadFromFile(dataDir + "/" + suffixAliasFile, errorMsg)) {
				throw 1;
			}

			// directional aliases
			directionalAliasTable = new LookupTable;
			if (!directionalAliasTable->LoadFromFile(dataDir + "/" + directionalAliasFile, errorMsg)) {
				throw 1;
			}

			// unit designator aliases
			unitDesignatorAliasTable = new LookupTable;
			if (!unitDesignatorAliasTable->LoadFromFile(dataDir + "/" + unitDesignatorAliasFile, errorMsg)) {
				throw 1;
			}

			// address token symbols
			{
				addressTokenTable = new LookupTable;
				TsString addressTokenFile = forPuertoRico ? addressTokenFilePr : addressTokenFileNonPr;
				if( !addressTokenTable->LoadFromFile(dataDir + "/" + addressTokenFile, errorMsg) ) {
					throw 1;
				}
			}

			// Streetname aliases
			streetnameAliasesTable = new LookupTable;
			if( !streetnameAliasesTable->LoadFromFile(dataDir + "/" + streetnameAliasesFile, errorMsg) ) {
				throw 1;
			}

			// Leading-token lexicon for multi-token streetname alias
			streetnameMultiwordAliasesLexicon = new Lexicon;
			if( !streetnameMultiwordAliasesLexicon->LoadFromFile(dataDir + "/" + streetnameMultiwordSearchAliasesFile, errorMsg) ) {
				throw 1;
			}

			// LookupTable containing multiword street aliases
			streetnameMultiwordAliasesTable = new LookupTable;
			if( !streetnameMultiwordAliasesTable->LoadFromFile(dataDir + "/" + streetnameMultiwordAliasesFile, errorMsg) ) {
				throw 1;
			}

			// LookupTable containing street name prefix aliases
			streetNamePrefixAliasesTable = new LookupTable;
			if (!streetNamePrefixAliasesTable->LoadFromFile(dataDir + "/" + streetNamePrefixAliasesFile, errorMsg) ) {
				throw 1;
			}

			// Create and bind the pattern matching tools
			{
				TsString patternsConfigurationFile = forPuertoRico ? patternsConfigurationFilePr : patternsConfigurationFileNonPr;
				if( !regularExprWrapper->ReadPatternToolConfiguration(dataDir_ + "/" + patternsConfigurationFile, errorMsg) ) {
					throw 1;
				}
			}

			// Number aliases
			numberAliasTable = new LookupTable;
			if (!numberAliasTable->LoadFromFile(dataDir + "/" + numberAliasFile, errorMsg)) {
				// Ignore this error; older distributions lack this file.
				// throw 1
			}

			// magnet street words
			streetNameMagnetWordsLexicon = new Lexicon;
			// Allow this load to fail.  Old address parsers did not ship with this lexicon so
			// we want to allow that.
			streetNameMagnetWordsLexicon->LoadFromFile(dataDir + "/" + streetNameMagnetWordsFile, errorMsg);


			// Build our lookup tries
			GenerateAssemblyTrie();
			GenerateSymbolTrie();

		} catch (int) {
			// Error occurred.
			return false;
		}

		// All systems go...
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Release any resources that were in use during processing.
	///////////////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::Close()
	{
		directionalsLexicon = 0;
		reverseDirectionalsLexicon = 0;
		reverseSuffixesLexicon = 0;
		numberAliasTable = 0;
		suffixAliasTable = 0;
		directionalAliasTable = 0;
		unitDesignatorAliasTable = 0;
		bulkAllocator = 0;
		assemblyTrie.Clear();
		symbolFlagTrie.Clear();
		streetnameAliasesTable = 0;
		streetNamePrefixAliasesTable = 0;
		streetnameMultiwordAliasesTable = 0;
		streetnameMultiwordAliasesLexicon = 0;
		streetNameMagnetWordsLexicon = 0;
		regularExprWrapper = 0;
		parsedTokens.clear();
	}

	//////////////////////////////////////////////////////////////////////
	// Read a first-line, generate tokens, and perform preprocessing of
	// address tokens.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserFirstLineImp::Parse(
		const char* addressLine, 
		AddressParserFirstLine::ParseCandidate& parseCandidate, 
		bool replaceAliases
	) {
		// Reset the allocator and free memory used by last parse
		bulkAllocator->Reset();
		hashedCandidates.Clear();
		addressParse.clear();

		// Next permutation candidate is always index==1 because index==0 
		// is the candidate returned from this call.
		nextCandidateIdx = 1;
		
		// Convert address line to upper case
		tempString1 = addressLine;
		{for (unsigned i = 0; i < tempString1.size(); i++) {
			tempString1[i] = TOUPPER(tempString1[i]);
		}}

		// Check for trivial case
		if (tempString1.empty()) {
			parsedTokens.clear();
			return false;
		}

		// Start with one parsed tokens entry -- the one we'll use for the initial parse.
		parsedTokens.resize(1);

		//Parse address line into tokens
		regularExprWrapper->ProduceTokens(tempString1.c_str(), addressParse, bulkAllocator);
		//Assign symbols to tokens
		regularExprWrapper->ProduceSymbols(addressParse, bulkAllocator);
		
		//Perform further symbol assignment by looking up tokens in the address token table
		std::vector<TokSymCls>::iterator it;
		const char *tmpPtr;
		for(it = addressParse.begin(); it != addressParse.end(); it++) {
			if (addressTokenTable->Find((*it).token.c_str(), tmpPtr)) {
				(*it).symbol = tmpPtr;
			}
		}

		//Assign classes to tokens and symbols
		regularExprWrapper->ProduceClasses(addressParse, bulkAllocator);
		
		//Put orginal tokens into proper token list
		GenerateTokenLists();

		// Perform some cleanup of initial parse that is hard for straight pattern-matcher
		CleanupInitialParse();
		
		ParsedTokens& returnParsedTokens = parsedTokens[0];

		//Fill the parsecandidate and return it
		ParsedTokensToParseCandidate(parseCandidate, returnParsedTokens, replaceAliases);

		// Enter into hash table
		CheckNewParseCandidateForDuplication();

		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// Fill the assembly trie with the proper assemBridging structs
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::GenerateAssemblyTrie()
	{
		assemblyTrie.Clear();

		AssemBridgingRef assemBridge;

		// This little bit of code is done to ensure that the vector of parsedTokens
		// is not reallocated, which would cause all of the pointers below to become
		// invalid.
		parsedTokens.clear();
		parsedTokens.reserve(MaxParseCandidates);
		
		//Make a new parsedtokens to contain the parsed address
		//This should always be parsedTokens[0]
		ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();

		//Create and assembridging for each of the possible class values
		//and add it to the trie
		assemBridge = new AssemBridging("NBR", &newParsedTokens.number);
		assemblyTrie.Insert("NBR", assemBridge);		
		assemBridge = new AssemBridging("PREDIR", &newParsedTokens.predir);
		assemblyTrie.Insert("PREDIR", assemBridge);		
		assemBridge = new AssemBridging("PREFIX", &newParsedTokens.prefix);
		assemblyTrie.Insert("PREFIX", assemBridge);		
		assemBridge = new AssemBridging("STREET", &newParsedTokens.street);
		assemblyTrie.Insert("STREET", assemBridge);		
		assemBridge = new AssemBridging("SUFFIX", &newParsedTokens.suffix);
		assemblyTrie.Insert("SUFFIX", assemBridge);		
		assemBridge = new AssemBridging("POSTDIR", &newParsedTokens.postdir);
		assemblyTrie.Insert("POSTDIR", assemBridge);		
		assemBridge = new AssemBridging("UNITDES", &newParsedTokens.unitDesignator);
		assemblyTrie.Insert("UNITDES", assemBridge);		
		assemBridge = new AssemBridging("UNITNBR", &newParsedTokens.unitNumber);
		assemblyTrie.Insert("UNITNBR", assemBridge);		
		assemBridge = new AssemBridging("PMBDES", &newParsedTokens.pmbDesignator);
		assemblyTrie.Insert("PMBDES", assemBridge);		
		assemBridge = new AssemBridging("PMBNBR", &newParsedTokens.pmbNumber);
		assemblyTrie.Insert("PMBNBR", assemBridge);		
		assemBridge = new AssemBridging("AND", &newParsedTokens.andTokens);
		assemblyTrie.Insert("AND", assemBridge);		
		assemBridge = new AssemBridging("PREDIR2", &newParsedTokens.predir2);
		assemblyTrie.Insert("PREDIR2", assemBridge);		
		assemBridge = new AssemBridging("STREET2", &newParsedTokens.street2);
		assemblyTrie.Insert("STREET2", assemBridge);		
		assemBridge = new AssemBridging("SUFFIX2", &newParsedTokens.suffix2);
		assemblyTrie.Insert("SUFFIX2", assemBridge);		
		assemBridge = new AssemBridging("POSTDIR2", &newParsedTokens.postdir2);
		assemblyTrie.Insert("POSTDIR2", assemBridge);		
		assemBridge = new AssemBridging("GARBAGE", &newParsedTokens.discarded);
		assemblyTrie.Insert("GARBAGE", assemBridge);		
	}

	//////////////////////////////////////////////////////////////////////
	// Fill the assembly trie with the proper assemBridging structs
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::GenerateSymbolTrie()
	{
		symbolFlagTrie.Clear();

		SymbolFlagMapRef symbolMap;
		
		//Create a trie that maps from class names to token types
		symbolMap = new SymbolFlagMap("SUFFIX1", Token::IsStreetSuffix);
		symbolFlagTrie.Insert("SUFFIX1", symbolMap);		
		symbolMap = new SymbolFlagMap("SUFFIX2", Token::IsStreetSuffix);
		symbolFlagTrie.Insert("SUFFIX2", symbolMap);		
		symbolMap = new SymbolFlagMap("DIR", Token::IsDirectional);
		symbolFlagTrie.Insert("DIR", symbolMap);		
		symbolMap = new SymbolFlagMap("NUM", Token::IsNumber);
		symbolFlagTrie.Insert("NUM", symbolMap);		
		symbolMap = new SymbolFlagMap("UNIT", Token::IsUnitDesignator);
		symbolFlagTrie.Insert("UNIT", symbolMap);		
		symbolMap = new SymbolFlagMap("UNIT0", Token::IsUnitDesignator);
		symbolFlagTrie.Insert("UNIT0", symbolMap);		
		symbolMap = new SymbolFlagMap("UNIT1", Token::IsUnitDesignator);
		symbolFlagTrie.Insert("UNIT1", symbolMap);		
		symbolMap = new SymbolFlagMap("TRAILER", Token::IsUnitDesignator);
		symbolFlagTrie.Insert("UNIT1", symbolMap);		
		symbolMap = new SymbolFlagMap("FRACT", Token::IsFraction);
		symbolFlagTrie.Insert("FRACT", symbolMap);		
	}

	//////////////////////////////////////////////////////////////////////
	// Create token lists out of parsed addresses
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::GenerateTokenLists()
	{
		// We should have set it this size from start of Parse() call.
		assert(parsedTokens.size() == 1);

		parsedTokens[0].Clear();
		std::vector<TokSymCls>::iterator it;
		AssemBridgingRef tmp;
		SymbolFlagMapRef symbol;
		int flags;
		for(it = addressParse.begin(); it != addressParse.end(); it++) {
			tmp = assemblyTrie.Find((*it).token_class.c_str());
			if( tmp != 0 ) {
				symbol = symbolFlagTrie.Find((*it).symbol.c_str());
				if( symbol != 0 ) {
					flags = symbol->tokenFlag;
				} else {
					flags = 0;
				}
				// Special-case the HasDigits flag, which can't be bound to a token type.
				for (const char* textPtr = (*it).token.c_str(); *textPtr != 0; textPtr++) {
					if (isdigit(*textPtr)) {
						flags |= Token::HasDigit;
						break;
					}
				}
				tmp->outputList->push_back(Token((*it).token.c_str(), flags));
			} else {
				// Invalid class.  This occurs for the PMB unit designator token.
				// Ignore it.
			}
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Convert a ParsedTokens to a AddressParserFirstLine::ParseCandidate
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::ParsedTokensToParseCandidate(
		AddressParserFirstLine::ParseCandidate& parseCandidate, 
		const ParsedTokens& tokens,
		bool replaceAliases
	) {
		parseCandidate.isIntersection = (!tokens.street2.empty());
		parseCandidate.permutations = tokens.permutations;
		parseCandidate.numberOfMods = tokens.numberOfMods;

		if (replaceAliases) {
			bool aliased = false;
			aliased |= DeAliasToBuffer(
				parseCandidate.predir, 
				sizeof(parseCandidate.predir),
				directionalAliasTable,
				tokens.predir
			);
			aliased |= DeAliasToBuffer(
				parseCandidate.suffix, 
				sizeof(parseCandidate.suffix),
				suffixAliasTable,
				tokens.suffix
			);
			aliased |= DeAliasToBuffer(
				parseCandidate.postdir, 
				sizeof(parseCandidate.postdir),
				directionalAliasTable,
				tokens.postdir
			);
			DeAliasToBuffer(
				parseCandidate.unitDesignator, 
				sizeof(parseCandidate.unitDesignator),
				unitDesignatorAliasTable,
				tokens.unitDesignator
			);

			// Replacements for second street of intersection.
			DeAliasToBuffer(
				parseCandidate.predir2, 
				sizeof(parseCandidate.predir2),
				directionalAliasTable,
				tokens.predir2
			);
			DeAliasToBuffer(
				parseCandidate.suffix2, 
				sizeof(parseCandidate.suffix2),
				suffixAliasTable,
				tokens.suffix2
			);
			DeAliasToBuffer(
				parseCandidate.postdir2, 
				sizeof(parseCandidate.postdir2),
				directionalAliasTable,
				tokens.postdir2
			);
			DeAliasToBuffer(
				parseCandidate.pmbDesignator, 
				sizeof(parseCandidate.pmbDesignator),
				unitDesignatorAliasTable,
				tokens.pmbDesignator
			);
			if (aliased) {
				parseCandidate.permutations |= AddressParserFirstLine::PermuteAlias;
			}
		} else {
			// Do not replace aliases.
			tokens.predir.ToBuffer(parseCandidate.predir, sizeof(parseCandidate.predir));
			tokens.suffix.ToBuffer(parseCandidate.suffix, sizeof(parseCandidate.suffix));
			tokens.postdir.ToBuffer(parseCandidate.postdir, sizeof(parseCandidate.postdir));
			tokens.unitDesignator.ToBuffer(parseCandidate.unitDesignator, sizeof(parseCandidate.unitDesignator));
			tokens.predir2.ToBuffer(parseCandidate.predir2, sizeof(parseCandidate.predir2));
			tokens.suffix2.ToBuffer(parseCandidate.suffix2, sizeof(parseCandidate.suffix2));
			tokens.postdir2.ToBuffer(parseCandidate.postdir2, sizeof(parseCandidate.postdir2));
			tokens.pmbDesignator.ToBuffer(parseCandidate.pmbDesignator, sizeof(parseCandidate.pmbDesignator));
		}
		
		// Remaining fields are never de-aliased.
		tokens.number.ToBuffer(parseCandidate.number, sizeof(parseCandidate.number));
		tokens.prefix.ToBuffer(parseCandidate.prefix, sizeof(parseCandidate.prefix));
		tokens.street.ToBuffer(parseCandidate.street, sizeof(parseCandidate.street));
		tokens.unitNumber.ToBuffer(parseCandidate.unitNumber, sizeof(parseCandidate.unitNumber));
		tokens.pmbNumber.ToBuffer(parseCandidate.pmbNumber, sizeof(parseCandidate.pmbNumber));
		tokens.street2.ToBuffer(parseCandidate.street2, sizeof(parseCandidate.street2));
	}

	//////////////////////////////////////////////////////////////////////
	// Perform some cleanup of initial parse that is hard for straight pattern-matcher
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::CleanupInitialParse()
	{	
		ParsedTokens& baseline = parsedTokens[0];

		// Check for problems where an initial parse will place a large word in the
		// unit number
		if (
			!baseline.unitDesignator.empty() &&
			!baseline.unitNumber.empty() &&
			baseline.unitNumber[0].size >= 5 &&
			(baseline.unitNumber[0].flags & Token::HasDigit) == 0 &&
			baseline.postdir.empty() &&
			baseline.suffix.empty()
		) {
			// Move the unit and unitdes to the street
			baseline.street.push_back(baseline.unitDesignator[0]);
			baseline.unitDesignator.clear();
			baseline.street.push_back(baseline.unitNumber[0]);
			baseline.unitNumber.clear();
		}
	}
		
	//////////////////////////////////////////////////////////////////////
	// Perform permutations on the address.
	// Inputs:
	//	int				permutationFlags	Bits set in this flag describe the
	//										permutations desired.
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
	void AddressParserFirstLineImp::PermuteAddress(
		int permutationFlags
	) {
		int parsedTokensSize = int(parsedTokens.size());
		for (int parsedTokensIdx = 0; parsedTokensIdx < parsedTokensSize; parsedTokensIdx++)
		{
			// Permute based on a specific baseline
			PermuteAddress(permutationFlags, parsedTokens[parsedTokensIdx]);
		}
	}


	//////////////////////////////////////////////////////////////////////
	// Perform permutations on the address.
	// Inputs:
	//	int				permutationFlags	Bits set in this flag describe the
	//										permutations desired.
	//	ParsedTokens&	baseline			The baseline parsedTokens to permute.
	//
	// Note: After NextAddressPermutation() returns false, you may call
	// PermuteAddress() again (perhaps using new flags) to generate
	// more permutations, which will become available through
	// subsequent calls to NextAddressPermutation().
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::PermuteAddress(
		int permutationFlags,
		const ParsedTokens& baseline
	) {
		// Permuting adds more candidates onto the existing candidate list,
		// so every time you call this method the list may grow.  However,
		// nextCandidateIdx is not reset, so only new permutations are
		// returned by NextAddressPermutation().

		// Just in case something really bad happens
		if (parsedTokens.size() > 0.8 * MaxParseCandidates) {
			// Something has gone wrong.  Don't generate any more permutations.
			return;
		}

		//
		// The following are necessary to deal with "magnet" sreets, where the 
		// street name may contain a suffix, directional, or both.
		//

		if ((permutationFlags & AddressParserFirstLine::PermuteGlomSuffix) != 0) {
			// Add street suffix to end of street name
			// 123 FIRST ROAD  -->  123 FIRST ROAD
			//       ^    ^             ^--------^
			//   street   suffix          street
			if (!baseline.suffix.empty()) {
				//Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.street.push_back(newTokens.suffix[0]);
				newTokens.suffix.erase(newTokens.suffix.begin());
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteGlomSuffix;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteCleaveSuffix) != 0) {
			// If multi-word street name contains a suffix, and there is no suffix,
			// then move last street token to suffix.
			//
			// 123 EAST LOOP -->  123 EAST  LOOP
			//     ^-------^           ^    ^
			//	     street        street  suffix
			//
			if (
				baseline.street.size() >= 2 &&
				baseline.suffix.empty() &&
				(
					(baseline.street.Last().flags & Token::IsStreetSuffix) != 0 ||
					strcmp(baseline.street.Last().text, "EXP") == 0 ||
					strcmp(baseline.street.Last().text, "EXPY") == 0 ||
					strcmp(baseline.street.Last().text, "EXPRESSWAY") == 0
				)
			) {
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.suffix.push_back(baseline.street.Last());
				newTokens.street.pop_back();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteCleaveSuffix;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteCleavePostdir) != 0) {
			// Move trailing directional in street to postdir
			// 123 CALLE E    -->   123 CALLE  E
			//     ^-----^                 ^   ^
			//      street             street  postdir
			if (
				baseline.street.size() >= 2 &&
				baseline.suffix.empty() &&
				baseline.postdir.empty() &&
				directionalsLexicon->Find(baseline.street.Last().text)
			) {
				//Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.postdir.push_back(newTokens.street.Last());
				newTokens.street.pop_back();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteCleavePostdir;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteGlomPostdir) != 0) {
			// Add post-directional to street name if there is no suffix in the way
			// 123 32ND   NORTH --> 123 32ND NORTH
			//       ^      ^           ^--------^ 
			//    street   postdir        street
			if (
				!baseline.postdir.empty() &&
				baseline.suffix.empty()
			) {
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.street.push_back(newTokens.postdir[0]);
				newTokens.postdir.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteGlomPostdir;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteGlomPredir) != 0) {
			// Add pre-directional to street name
			// 123 WEST 32ND ST --> 123 WEST 32ND ST
			//           ^              ^------^ 
			//         street            street
			if (!baseline.predir.empty()) {
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.street.insert(
					newTokens.street.begin(), 
					newTokens.predir[0]
				);
				newTokens.predir.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteGlomPredir;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteStreetSuffixToPredirStreet) != 0) {
			// If street name is a directional, and there is a suffix,
			// and there is no predirectional, 
			// then move street to predir and suffix to street.
			//
			// 123 WEST COURT  -->  123 WEST COURT
			//       ^    ^               ^    ^
			//   street  suffix       predir  street
			//
			if (
				baseline.street.size() == 1 &&
				(baseline.street[0].flags & Token::IsDirectional) != 0 &&
				!baseline.suffix.empty() &&
				baseline.predir.empty()
			) {
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.predir.push_back(baseline.street[0]);
				newTokens.street[0] = newTokens.suffix[0];
				newTokens.suffix.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteStreetSuffixToPredirStreet;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		//
		// End of "magnet" street cases
		//


		if ((permutationFlags & AddressParserFirstLine::PermuteCleaveUnitdes) != 0) {
			// If address has a unitdes in the street, try moving it to the
			// unitdes spot.  There are two approaches to this
			//
			// Cleave unit designator off street, moving suffix to unit
			//   123 HIGH OFF  ST  -->  123 HIGH   OFF      ST
			//       ^------^  ^              ^     ^       ^  
			//        street  suffix       street  unitdes  unitnum        
			//
			// Cleave unit designator off street, moving postdir to unit
			//   123 3 OFFICE   E      ->    123 3 OFFICE    E
			//       ^------^   ^                ^   ^       ^  
			//        street postdir         street unitdes  unitnum        
			//
			// Optionally we may do something like this:
			//   123 EAST OFFICE ST      ->    123 EAST OFFICE     ST
			//        ^      ^    ^                  ^     ^        ^
			//      predir street suffix          street unitdes unitnbr
			if (
				baseline.unitDesignator.empty() &&
				baseline.unitNumber.empty() &&
				!baseline.street.empty() &&
				(baseline.street.size() > 1 || !baseline.predir.empty()) &&
				(baseline.street.Last().flags & Token::IsUnitDesignator) != 0
			) {
				if (!baseline.suffix.empty() && baseline.suffix[0].size <= 2) {
					ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
					newTokens = baseline;
					// Move last street token to unitdes
					newTokens.unitDesignator.push_back(newTokens.street.Last());
					newTokens.street.pop_back();
					// Move suffix to unitnbr
					newTokens.unitNumber.push_back(newTokens.suffix[0]);
					newTokens.suffix.pop_back();
					// If the street is empty, we need to put the predir in the street
					if (newTokens.street.empty()) {
						assert(newTokens.predir.size() != 0);
						newTokens.street.push_back(newTokens.predir[0]);
						newTokens.predir.pop_back();
					}
					// Mark changes
					newTokens.permutations |= AddressParserFirstLine::PermuteCleaveUnitdes;
					newTokens.numberOfMods++;
					// Remove if this is a duplicate
					CheckNewParseCandidateForDuplication();
				} else if (!baseline.postdir.empty() && baseline.postdir[0].size <= 2) {
					ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
					newTokens = baseline;
					// Move last street token to unitdes
					newTokens.unitDesignator.push_back(newTokens.street.Last());
					newTokens.street.pop_back();
					// Move postdir to unit number
					newTokens.unitNumber.push_back(newTokens.postdir[0]);
					newTokens.postdir.pop_back();
					// If the street is empty, we need to put the predir in the street
					if (newTokens.street.empty()) {
						assert(newTokens.predir.size() != 0);
						newTokens.street.push_back(newTokens.predir[0]);
						newTokens.predir.pop_back();
					}
					// Mark changes
					newTokens.permutations |= AddressParserFirstLine::PermuteCleaveUnitdes;
					newTokens.numberOfMods++;
					// Remove if this is a duplicate
					CheckNewParseCandidateForDuplication();
				}
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteRemoveIntersection) != 0) {
			// If address is an intersection, move street2 to street and any of the trailing
			// street2 tokens to their first street counterparts. Also, add the AND token
			//
			// 123 HOLLY AND VINE    ST --> 123 HOLLY AND VINE ST
			//      ^          ^     ^          ^------------^ ^
			//   street1     street2 suffix2         street    suffix
			if( !baseline.street2.empty() ) {
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				//Put the AND token(s) in the street name
				{for (unsigned i = 0; i < baseline.andTokens.size(); i++) {
					newTokens.street.push_back(baseline.andTokens[i]);
					newTokens.andTokens.pop_back();
				}}
				//Now put the street2 tokens in the street name
				{for (unsigned i = 0; i < baseline.street2.size(); i++) {
					newTokens.street.push_back(baseline.street2[i]);
					newTokens.street2.pop_back();
				}}
				//Check for postdir2 and/or suffix2 and move them to the first
				//address-- NOTE:  Assuming first street doesn't have a suffix/postdir!
				if( !newTokens.suffix2.empty() ) {
					newTokens.suffix.push_back(newTokens.suffix2[0]);
					newTokens.suffix2.pop_back();
				}
				if( !newTokens.postdir2.empty() ) {
					newTokens.postdir.push_back(newTokens.postdir2[0]);
					newTokens.postdir2.pop_back();
				}
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteRemoveIntersection;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}


		if ((permutationFlags & AddressParserFirstLine::PermuteShufflePmbUnit) != 0) {
			// If address has a PMB designator, move the unitdes and unit to street, then
			// make PMB the unitdes and the PMB number the unitnum
			//
			// 123 HOLLY PIER   A    OFFICE 123    --> 123 HOLLY PIER A OFFICE 123
			//      ^      ^    ^      ^     ^             ^----------^    ^    ^
			//   street unitdes unit pmbdes pmbnum           street     unitdes unit
			if( !baseline.pmbDesignator.empty() || !baseline.pmbNumber.empty()) {
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				
				//Put the unitdes and unitnum into street
				if( !newTokens.unitDesignator.empty() ) {
					newTokens.street.push_back(newTokens.unitDesignator[0]);
					newTokens.unitDesignator.pop_back();
				}
				if( !newTokens.unitNumber.empty() ) {
					newTokens.street.push_back(newTokens.unitNumber[0]);
					newTokens.unitNumber.pop_back();
				}

				//Now move the pmb stuff
				if (!newTokens.pmbDesignator.empty()) {
					newTokens.unitDesignator.push_back(newTokens.pmbDesignator[0]);
					newTokens.pmbDesignator.pop_back();
				}
				if( !newTokens.pmbNumber.empty() ) {
					newTokens.unitNumber.push_back(newTokens.pmbNumber[0]);
					newTokens.pmbNumber.pop_back();
				}
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteShufflePmbUnit;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteMoveFractionalStreetName) != 0) {
			// Move a street name as fraction to the address number and unit designator to the street
			// 43     1/2    PIER -->  43 1/2  PIER
			//  ^      ^      ^          ^       ^
			// nbr   street unit        nbr  street
			//
			if (
				!baseline.unitDesignator.empty() && baseline.street.size() == 1 &&
				(baseline.street[0].flags & Token::IsFraction) != 0 
			) {
				//Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.number.push_back(newTokens.street[0]);
				newTokens.street.pop_back();
				newTokens.street.push_back(newTokens.unitDesignator[0]);
				newTokens.unitDesignator.pop_back();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteMoveFractionalStreetName;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteMoveStreetLetterToNbr) != 0) {
			// Move a leading single letter from the street name to the address number, e.g.:
			//  123  B MAIN  ST      -->   123B   MAIN     ST
			//   ^   ^----^  ^              ^     ^        ^
			//  nbr  street  suffix        nbr   street  suffix
			//
			//  A EL MIRADOR APTS    -->   A   EL MIRADOR APTS
			//  ^---------------^          ^   ^-------------^
			//      street                nbr       street
			//
			// But don't move directional letters, because that makes it possible to
			// match something like:
			//      123 E MAIN ST
			// with a record like:
			//      101-199 W MAIN ST
			if (
				baseline.street.size() >= 2 &&
				baseline.street[0].size == 1 &&
				baseline.predir.empty() &&
				(baseline.street[0].flags & Token::IsDirectional) == 0 &&
				(
					baseline.number.empty() ||
					(
						baseline.number.size() == 1 &&
						(baseline.number[0].flags & Token::IsNumber) != 0
					)
				)
			) {
				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				if (newTokens.number.empty()) {
					newTokens.number.push_back(newTokens.street[0]);
				} else {
					assert(newTokens.number.size() == 1);
					tempString1 = newTokens.number[0].text;
					tempString1 += newTokens.street[0].text;
					SetTokenText(newTokens.number[0], tempString1.c_str());
				}
				newTokens.street.erase(newTokens.street.begin());
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteMoveStreetLetterToNbr;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteMoveNbrWordToStreet) != 0) {
			// Move a non-numeric word from the NBR to the STREET:
			//  COND  STA ANNA   -->   COND STA ANNA
			//   ^    ^------^         ^-----------^
			//  nbr    street             street
			if (
				baseline.number.size() == 1 &&
				(baseline.number[0].flags & Token::HasDigit) == 0 &&
				baseline.predir.empty()
			) {
				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.street.insert(newTokens.street.begin(), newTokens.number[0]);
				newTokens.number.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteMoveNbrWordToStreet;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteSuffixUnitdesToStreet) != 0) {
			// Move suffix/unitdes to street:
			//	CRESCENT BEACH  BLDG      -->  CRESCENT BEACH BLDG
			//    ^        ^      ^            ^-----------------^
			//   street  suffix  unitdes             street
			if (
				baseline.unitDesignator.size() == 1 &&
				baseline.unitNumber.empty() &&
				baseline.suffix.size() == 1 &&
				baseline.postdir.empty()
			) {
				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.street.push_back(newTokens.suffix[0]);
				newTokens.street.push_back(newTokens.unitDesignator[0]);
				newTokens.suffix.clear();
				newTokens.unitDesignator.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteSuffixUnitdesToStreet;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteMoveStreetToUnitDes) != 0) {
			if (
				baseline.street.size() >= 2 &&
				(baseline.street.Last().flags & Token::IsUnitDesignator) != 0 &&
				baseline.unitNumber.empty() &&
				baseline.postdir.empty() &&
				baseline.suffix.empty()
			) {
				if (
					baseline.unitDesignator.size() == 1 &&
					baseline.unitDesignator[0].size <= 2
				) {
					// Shuffle street-last-token-that-is-unitdes into unitdes
					//	2158 AVE GILBERTO MONROEG APT PH   -->   2158 AVE GILBERTO MONROEG   APT     PH
					//    ^   ^--------------------^   ^           ^  ^------------------^    ^      ^
					//   nbr           street        unitdes       nbr      street         unitdes  unit

					// Create new parsed tokens based on the unpermuted address
					ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
					newTokens = baseline;
					newTokens.unitNumber.push_back(newTokens.unitDesignator[0]);
					newTokens.unitDesignator[0] = newTokens.street.Last();
					newTokens.street.pop_back();
					// Mark changes
					newTokens.permutations |= AddressParserFirstLine::PermuteMoveStreetToUnitDes;
					newTokens.numberOfMods++;
					// Remove if this is a duplicate
					CheckNewParseCandidateForDuplication();
				 } else if (baseline.unitDesignator.empty()) {
					// Move trailing unitdes from street to unitdes
					//	211 KERR ADMINISTRATION BLDG --> 211 KERR ADMINISTRATION BLDG
					//    ^ ^----------------------^      ^  ^-----------------^   ^
					//   nbr        street                nbr      street         unitdes

					// Create new parsed tokens based on the unpermuted address
					ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
					newTokens = baseline;
					newTokens.unitDesignator.push_back(newTokens.street.Last());
					newTokens.street.pop_back();
					// Mark changes
					newTokens.permutations |= AddressParserFirstLine::PermuteMoveStreetToUnitDes;
					newTokens.numberOfMods++;
					// Remove if this is a duplicate
					CheckNewParseCandidateForDuplication();
				 }
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteMoveUnitdesToStreet) != 0) {
			if (
				!baseline.unitDesignator.empty() &&
				baseline.unitNumber.empty() &&
				baseline.suffix.empty() &&
				baseline.postdir.empty()
			) {
				// Append unitdes to street
				//	2158 MONROE BLDG      -->   2158 MONROE BLDG
				//    ^     ^     ^              ^   ^---------^
				//   nbr  street  unitdes        nbr    street

				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.street.push_back(newTokens.unitDesignator[0]);
				newTokens.unitDesignator.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteMoveUnitdesToStreet;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermutePrPostdirToNbr) != 0) {
			if (
				forPuertoRico &&
				baseline.unitDesignator.empty() &&
				baseline.unitNumber.empty() &&
				baseline.postdir.size() == 1 &&
				baseline.postdir[0].size == 1 &&
				baseline.number.size() == 1 &&
				(baseline.number.Last().flags & Token::IsNumber) != 0
			) {
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

				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.number.insert(newTokens.number.begin(), newTokens.postdir[0]);
				newTokens.postdir.clear();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermutePrPostdirToNbr;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}
/*
		if ((permutationFlags & AddressParserFirstLine::PermutePmbUnit) != 0) {
			if (
				!baseline.pmbNumber.empty() &&
				!baseline.unitNumber.empty() &&
				(
					baseline.pmbDesignator.empty() ||
					strcmp(baseline.pmbDesignator[0].text, "#") == 0
				) &&
				(
					baseline.unitDesignator.empty() ||
					strcmp(baseline.unitDesignator[0].text, "#") == 0
				)
			) {
				// Swap unit/PMB, but only if both unit designator and PMB desigator
				// are blank or #
				// Input like:
				//  123 MAIN # 1 # 234
				// is normally parsed as:
				//  123 MAIN      # 1    # 234
				//   ^   ^          ^      ^
				//	nbr street     unit    pmb
				// change to:
				//  123 MAIN      # 1    # 234
				//   ^   ^          ^      ^
				//	nbr street     pmb    unit

				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				Token unitToken(newTokens.pmbNumber[0]);
				newTokens.pmbNumber[0] = newTokens.unitNumber[0];
				newTokens.unitNumber[0] = unitToken;
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermutePmbUnit;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}
*/

		if ((permutationFlags & AddressParserFirstLine::PermuteSaltlakeSyndrome) != 0) {
			if (
				baseline.street.size() == 1 &&
				isdigit(baseline.street[0].text[0]) &&
				IsDirectionalLetter(baseline.street[0].text[baseline.street[0].size-1]) &&
				baseline.number.size() == 1 &&
				isdigit(baseline.number[0].text[0]) &&
				IsDirectionalLetter(baseline.number[0].text[baseline.number[0].size-1]) &&
				baseline.predir.empty() &&
				baseline.postdir.empty()
			) {
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

				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				// Strip trailing directional from number and insert into predir
				{
					Token newToken(baseline.number[0]);
					SetTokenText(newToken, newToken.text + newToken.size - 1, 1);
					newTokens.predir.push_back(newToken);
					SetTokenText(newTokens.number[0], newTokens.number[0].text, newTokens.number[0].size-1);
				}
				// Strip trailing directional from street and insert into postdir
				{
					Token newToken(baseline.street[0]);
					SetTokenText(newToken, newToken.text + newToken.size - 1, 1);
					newTokens.postdir.push_back(newToken);
					SetTokenText(newTokens.street[0], newTokens.street[0].text, newTokens.street[0].size-1);
				}
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteSaltlakeSyndrome;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}

		if ((permutationFlags & AddressParserFirstLine::PermuteStreetNameAlias) != 0) {
			//
			// Perform substitutions of street name words to get variants
			//
			//   FIRST STREET <--> 1ST STREET
			//
			
			// Walk the street vector
			const char *tmpPtr;
			for (unsigned tokenIdx = 0; tokenIdx < baseline.street.size(); tokenIdx++) {
				const Token& streetToken = baseline.street[tokenIdx];
				// Look for single-token aliases
				if (streetnameAliasesTable->Find(streetToken.text, tmpPtr)) {
					// Make a new parse candidate with the replacement.
					ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
					newTokens = baseline;
					SetTokenText(newTokens.street[tokenIdx], tmpPtr);
					// Mark changes
					newTokens.permutations |= AddressParserFirstLine::PermuteStreetNameAlias;
					newTokens.numberOfMods++;
					// Remove if this is a duplicate
					CheckNewParseCandidateForDuplication();
				}
			}
		}
				
		if ((permutationFlags & AddressParserFirstLine::PermuteStreetNameMultiwordAlias) != 0) {
			//
			// Perform multiword substitutions of street names to get variants
			//
			//   NEW JERSEY ROUTE 1 <--> NJ ROUTE 1
			//
			
			// Walk the street vector
			const char *tmpPtr;
			for (int tokenIdx = 0; tokenIdx < (int)baseline.street.size() - 1; tokenIdx++) {
				const Token& streetToken = baseline.street[tokenIdx];
				if (streetnameMultiwordAliasesLexicon->Find(streetToken.text)) {
					// Construct two tokens into a buffer, separated by space, and
					// check that against the multi-token alias table.
					tempString1 = streetToken.text;
					tempString1 += ' ';
					tempString1 += baseline.street[tokenIdx+1].text;
					if (streetnameMultiwordAliasesTable->Find(tempString1.c_str(), tmpPtr)) {
						// Make a new parse candidate with the replacement.
						ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
						newTokens = baseline;
						SetTokenText(newTokens.street[tokenIdx], tmpPtr);
						// Remove the extra token
						newTokens.street.erase(newTokens.street.begin() + tokenIdx + 1);
						// Mark changes
						newTokens.permutations |= AddressParserFirstLine::PermuteStreetNameMultiwordAlias;
						newTokens.numberOfMods++;
						// Remove if this is a duplicate
						CheckNewParseCandidateForDuplication();
					}
				}
			}
		}

		if (
			(permutationFlags & AddressParserFirstLine::PermuteAddrNumberAlias) != 0 &&
			baseline.number.size() == 1
		) {
			//
			// Perform substitutions of numeric street names to get variants
			//
			//   ONE MAIN ST --> 1 MAIN ST
			//

			const char *tmpPtr;
			if (numberAliasTable->Find(baseline.number[0].text, tmpPtr)) {
				// Make a new parse candidate with the replacement.
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				SetTokenText(newTokens.number[0], tmpPtr);
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteAddrNumberAlias;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}


		if (
			(permutationFlags & AddressParserFirstLine::PermuteSplitSuffix) != 0 &&
			!baseline.street.empty() &&
			baseline.suffix.empty()
		) {
			// 
			// Fractionate trailing street suffix from street name
			// 
			//      123 MAINSTREET ---> 123 MAIN STREET
			//
			//  but fractionate two-letter suffix only from numeric names:
			//
			//      100 11ST --> 100 11 ST
			//
			//  and don't fractionate from digits if that would make a proper digit suffix:
			//       1ST    3RD   21ST   23RD

			// Reverse the street token text.
			const Token& streetToken = baseline.street.Last();
			ReverseTokenText(tempString1, streetToken);

			// Is a suffix of this token equal to a suffix without the
			// entire token being a suffix?  Also disallow splitting
			// the suffix if the next token is already a suffix.
			// Also, make sure not 
			int length;
			bool doReplace = false;
			if (
				reverseSuffixesLexicon->FindLongestKeyPrefix(tempString1.c_str(), length) &&
				length != streetToken.size
			) {
				doReplace = true;
				if (isdigit(tempString1[length])) {
					if (length == 2 &&
						(
							(
								tempString1[length] == '1' &&
								memcmp(tempString1.c_str(), "TS", 2) == 0		// ST reversed
							) ||
							(
								tempString1[length] == '3' &&
								memcmp(tempString1.c_str(), "DR", 2) == 0		// RD reversed
							)
						)
					) {
						doReplace = false;
					}
				} else {
					if (length <= 2) {
						doReplace = false;
					}
				}
			}
			if (doReplace) {
				// Make a new parse candidate with the replacement.
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				// Split the token into two halves.
				Token& tokenToSplit = newTokens.street.Last();
				assert(tokenToSplit.size == streetToken.size);
				// Make the new token consisting of the suffix
				Token newToken(tokenToSplit.text + tokenToSplit.size - length);
				// Truncate old token before suffix
				SetTokenText(tokenToSplit, tokenToSplit.text, tokenToSplit.size - length);
				// Set new token into suffix
				newTokens.suffix.push_back(newToken);
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteSplitSuffix;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}


		if (
			(permutationFlags & AddressParserFirstLine::PermuteSplitPredir) != 0 &&
			!baseline.street.empty() 
		) {
			// 
			// Fractionate leading directionals from tokens:
			//      100 WESTBAY ST ---> 123 WEST BAY ST
			//
			// ... but only fractionate short directionals from
			// numeric tokens:
			//
			//      100 W14 ST --> 100 W 14 ST
			//      10 N W MAIN ST --> 10 NW MAIN ST

			// Is a prefix of this token equal to a directional without the
			// entire token being a directional?
			const Token& streetToken = baseline.street[0];
			int length;
			if(	baseline.predir.empty() ) {
				if (
					directionalsLexicon->FindLongestKeyPrefix(streetToken.text, length) &&
					length != streetToken.size &&
					(
						length > 3 ||
						isdigit(streetToken.text[length])
					)
				) {
					// Make a new parse candidate with the replacement.
					ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
					newTokens = baseline;
					// Split the token into two halves.
					Token& tokenToSplit = newTokens.street[0];
					assert(tokenToSplit.size == streetToken.size);
					// Make the new token consisting of the directional
					Token newToken;
					SetTokenText(newToken, streetToken.text, length);
					// Set old token to text after directional
					SetTokenText(tokenToSplit, tokenToSplit.text + length, tokenToSplit.size - length);
					// Assign new token to predir
					newTokens.predir.push_back(newToken);
					// Mark changes
					newTokens.permutations|= AddressParserFirstLine::PermuteSplitPredir;
					newTokens.numberOfMods++;
					// Remove if this is a duplicate
					CheckNewParseCandidateForDuplication();
				}
			} else {
				//Check for a compatible primary dir in the predir (north or south)
				//Also make sure the street has at least two tokens
				Token predirToken(baseline.predir.Last());
				if (predirToken.size == 1 && baseline.street.size() > 1 ) {
					if( predirToken.text[0] == 'N' || 
						predirToken.text[0] == 'S' ) {
						tempString1 = predirToken.text[0];
						
						//Now we need the first word in the street to be E or W
						Token streetToken;
						if( baseline.street[0].size == 1 ) {
							if( baseline.street[0].text[0] == 'E' ||
							baseline.street[0].text[0] == 'W' ) {
							//Bingo!  
							tempString1 += baseline.street[0].text[0];

							// Make a new parse candidate with the replacement.
							ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
							newTokens = baseline;
							// Make the new token consisting of the directional
							Token newToken;
							SetTokenText(newToken, tempString1.c_str());
							newTokens.predir.pop_back();
							newTokens.predir.push_back(newToken);
							
							// Pull the first token out of the street
							newTokens.street.erase(newTokens.street.begin());
							// Mark changes
							newTokens.permutations|= AddressParserFirstLine::PermuteSplitPredir;
							newTokens.numberOfMods++;
							// Remove if this is a duplicate
							CheckNewParseCandidateForDuplication();
						}
					}
				}
			}
		}



		if (
			(permutationFlags & AddressParserFirstLine::PermuteSplitPostdir) != 0 &&
			!baseline.street.empty() &&
			baseline.postdir.empty()
		) {
			// 
			// Fractionate trailing (full) directionals from end of street:
			//      123 BAYWEST  ST --->  123   BAY      ST     WEST
			//       ^     ^      ^        ^     ^        ^       ^
			//      nbr  street  suffix   nbr  street  suffix  postdir
			// 
			//  but fractionate abbreviated directional only from digits
			//
			//      100 11W --> 100 11 W
			//

			// Reverse the street token text.
			const Token& streetToken = baseline.street.Last();
			ReverseTokenText(tempString1, streetToken);

			// Is a suffix of this token equal to a suffix without the
			// entire token being a suffix?  Also disallow splitting
			// the suffix if the next token is already a suffix.
			// Also, make sure not 
			int length;
			if (
				reverseDirectionalsLexicon->FindLongestKeyPrefix(tempString1.c_str(), length) &&
				length != streetToken.size &&
				(
					length > 2 ||
					isdigit(tempString1[length])
				)
			) {
				// Make a new parse candidate with the replacement.
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				// Split the token into two halves.
				Token& tokenToSplit = newTokens.street.Last();
				assert(tokenToSplit.size == streetToken.size);
				// Make the new token consisting of the suffix
				Token newToken(tokenToSplit.text + tokenToSplit.size - length);
				// Truncate old token before suffix
				SetTokenText(tokenToSplit, tokenToSplit.text, tokenToSplit.size - length);
				// Set new token into postdir
				newTokens.postdir.push_back(newToken);
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteSplitPostdir;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}


		if (
			(permutationFlags & AddressParserFirstLine::PermuteSplitAddrNbrToUnit) != 0 &&
			baseline.unitNumber.empty() &&
			baseline.number.size() == 1
		) {
			// If address number is like 123A and there is no unit#, then
			// make A the unit#, e.g.:
			//
			//     123A MAIN ST --> 123 MAIN ST # A
			//
			const Token& token = baseline.number[0];
			if (
				token.size > 1 &&
				isdigit(token.text[0]) &&
				isalpha(token.text[token.size - 1]) &&
				!isalpha(token.text[token.size - 2])
			) {
				ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
				newParsedTokens = baseline;
				Token& newToken = newParsedTokens.number[0];

				// Find the first alphabetic character
				int firstAlphaIdx = token.size - 1;
				while (firstAlphaIdx > 2 && isalpha(token.text[firstAlphaIdx - 1])) {
					firstAlphaIdx--;
				}
				// Set the unit designator if one does not exist
				if (newParsedTokens.unitDesignator.empty()) {
					newParsedTokens.unitDesignator.push_back(Token("#"));
				}
				// Add the alpha portion as the unit token
				newParsedTokens.unitNumber.push_back(Token(token.text + firstAlphaIdx));
				// Remove the alpha portion of the address token
				SetTokenText(newToken, newToken.text, firstAlphaIdx);
				newParsedTokens.permutations |= AddressParserFirstLine::PermuteSplitAddrNbrToUnit;
				newParsedTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}

			// If address number is like 111-222 and there is no unit#, then
			// make 222 the unit#
			const char* ptr = (const char*)strchr((const char*)token.text + 1, '-');
			if (ptr != 0 && ptr[1] != 0) {
				ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
				newParsedTokens = baseline;
				Token& newToken = newParsedTokens.number[0];
				// Set the unit designator if one does not exist
				if (newParsedTokens.unitDesignator.empty()) {
					newParsedTokens.unitDesignator.push_back(Token("#"));
				}
				// Add the after-dash text as the unit# token
				newParsedTokens.unitNumber.push_back(Token(ptr + 1));
				// Remove the alpha portion of the address token
				SetTokenText(newToken, newToken.text, int(ptr - token.text));
				// Mark changes
				newParsedTokens.permutations |= AddressParserFirstLine::PermuteSplitAddrNbrToUnit;
				newParsedTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}
	}

	{
		//
		// For street names containing dash-separated prefixes, replace the
		// prefix with an alias. This is necessary because the aliases 
		// do not have the same soundex as the original.
		//
		// ST-JOSEPH --> SAINT-JOSEPH
		//
		const char* separatorPtr;
		char tmpBuf[64];
		if (
			(permutationFlags & AddressParserFirstLine::PermuteStreetNamePrefixAlias) != 0 &&
			!baseline.street.empty() &&
			(separatorPtr = strchr(baseline.street[0].text, '-')) != 0 &&
			baseline.street[0].size < static_cast<int>(sizeof(tmpBuf) - 20)
		) {
			int honorificSize = int(separatorPtr - baseline.street[0].text);
			// Extract stuff before the '-'
			memcpy(tmpBuf, baseline.street[0].text, honorificSize);
			tmpBuf[honorificSize] = 0;
			// Look for the honorific in the alias table.
			const char *replacement;
			if (streetNamePrefixAliasesTable->Find(tmpBuf, replacement)) {
				ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
				newParsedTokens = baseline;
				// Replace honorific in street token
				int length = int(strlen(replacement));
				strcpy(tmpBuf, replacement);
				tmpBuf[length++] = '-';
				strcpy(tmpBuf + length, baseline.street[0].text + honorificSize + 1);
				SetTokenText(newParsedTokens.street[0], tmpBuf);
				newParsedTokens.numberOfMods++;
				// Mark changes
				newParsedTokens.permutations |= AddressParserFirstLine::PermuteStreetNamePrefixAlias;
				newParsedTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		}
	}


	// If a street ends with a number, and there is no postdir, suffix, or unit,
	// then move the street number to the unit.
	//		123 CALLE 4  -->  123 CALLE    4  
	//		 ^  ^-----^        ^  ^---^    ^
	//     nbr   street      nbr  street   unit
	//
	// However, do not cleave directional as unit
	//		123 MAIN ST N
	//		
	if (
		(permutationFlags & AddressParserFirstLine::PermuteStreetNumberToUnit) != 0 &&
		baseline.suffix.empty() &&
		baseline.postdir.empty() &&
		baseline.unitDesignator.empty() &&
		baseline.unitNumber.empty() &&
		baseline.street.size() >= 2 &&
		(
			(baseline.street.Last().flags & Token::IsNumber) != 0 ||
			(
				baseline.street.Last().size == 1 &&
				(baseline.street.Last().flags & Token::IsDirectional) == 0
			)
		)
	) {
		// Assemble street that will remain after cleaving unit 
		baseline.street.ToString(tempString1, 0, (int)baseline.street.size()-2);
		// Assemble street that will remain after cleaving suffix/unit
		baseline.street.ToString(tempString2, 0, (int)baseline.street.size()-3);
		if (!streetNameMagnetWordsLexicon->Find(tempString1.c_str())) {
			// Do not leave "magnet" street name.  

			// Cleave just unit.
			// Works for cases like:
			//		123 MAIN 8
			//		123 CARMEN HILL 6		(HILL is part of name)
			// Create new parsed tokens based on the unpermuted address
			ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
			newTokens = baseline;
			newTokens.unitNumber.push_back(newTokens.street.Last());
			newTokens.street.pop_back();
			// Mark changes
			newTokens.permutations |= AddressParserFirstLine::PermuteStreetNumberToUnit;
			newTokens.numberOfMods++;
			// Remove if this is a duplicate
			CheckNewParseCandidateForDuplication();

			if (
				baseline.street.size() >= 3 &&
				!streetNameMagnetWordsLexicon->Find(tempString2.c_str()) &&
				(baseline.street[baseline.street.size()-2].flags & Token::IsStreetSuffix) != 0
			) {
				// Cleave both suffix and unit .  Works for cases like:
				//		123 MAIN ST 8
				// Create new parsed tokens based on the unpermuted address
				ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
				newTokens = baseline;
				newTokens.unitNumber.push_back(newTokens.street.Last());
				newTokens.street.pop_back();
				newTokens.suffix.push_back(newTokens.street.Last());
				newTokens.street.pop_back();
				// Mark changes
				newTokens.permutations |= AddressParserFirstLine::PermuteStreetNumberToUnit;
				newTokens.numberOfMods++;
				// Remove if this is a duplicate
				CheckNewParseCandidateForDuplication();
			}
		} 
	}

	if (
		(permutationFlags & AddressParserFirstLine::PermuteUnitStreet) != 0 &&
		baseline.street.size() == 1 &&
		baseline.suffix.empty() && 
		strcmp(baseline.street[0].text, "UNIT") == 0 &&
		baseline.postdir.empty() &&
		baseline.predir.empty() &&
		baseline.unitDesignator.empty() &&
		baseline.unitNumber.empty() &&
		baseline.number.size() == 1
	) {
		// Permute unit/nbr to street for Military addresses
		// UNIT   123    -->    UNIT   123
		//   ^     ^             ^-------^
		// street  nbr            street
		ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
		newTokens = baseline;
		newTokens.street.push_back(newTokens.number[0]);
		newTokens.number.pop_back();
		// Mark changes
		newTokens.permutations |= AddressParserFirstLine::PermuteUnitStreet;
		newTokens.numberOfMods++;
		// Remove if this is a duplicate
		CheckNewParseCandidateForDuplication();
	}

	if (
		(permutationFlags & AddressParserFirstLine::PermuteShiftSuffixUnitdes) != 0 &&
		baseline.street.size() >= 3 &&
		baseline.suffix.empty() && 
		baseline.postdir.empty() && 
		baseline.unitDesignator.empty() && 
		(baseline.street[baseline.street.size()-2].flags & Token::IsStreetSuffix) != 0 &&
		(baseline.street.Last().flags & Token::IsUnitDesignator) != 0
	) {
		// Shift suffix and unitdes from street name
		//
		// 123 MAIN STREET SUITE  ->   123  MAIN   STREET SUITE
		//	^  ^---------------^        ^    ^       ^      ^
		// nbr         street          nbr street  suffix  unitdes
		ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
		newTokens = baseline;
		newTokens.unitDesignator.push_back(newTokens.street.Last());
		newTokens.street.pop_back();
		newTokens.suffix.push_back(newTokens.street.Last());
		newTokens.street.pop_back();
		// Mark changes
		newTokens.permutations |= AddressParserFirstLine::PermuteShiftSuffixUnitdes;
		newTokens.numberOfMods++;
		// Remove if this is a duplicate
		CheckNewParseCandidateForDuplication();
	}

	if (
		(permutationFlags & AddressParserFirstLine::PermuteShiftUnitdes) != 0 &&
		baseline.street.size() >= 2 &&
		baseline.suffix.empty() && 
		baseline.postdir.empty() && 
		baseline.unitDesignator.empty() && 
		(baseline.street.Last().flags & Token::IsUnitDesignator) != 0
	) {
		// Shift unitdes from street name
		//
		// 123 MAIN SUITE  ->   123  MAIN    SUITE
		//	^  ^--------^        ^    ^        ^
		// nbr   street	        nbr  street  unitdes
		ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
		newTokens = baseline;
		newTokens.unitDesignator.push_back(newTokens.street.Last());
		newTokens.street.pop_back();
		// Mark changes
		newTokens.permutations |= AddressParserFirstLine::PermuteShiftUnitdes;
		newTokens.numberOfMods++;
		// Remove if this is a duplicate
		CheckNewParseCandidateForDuplication();
	}

	if (
		(permutationFlags & AddressParserFirstLine::PermuteRemoveDoubleSuffix) != 0 &&
		baseline.street.size() >= 2 &&
		!baseline.suffix.empty() && 
		baseline.postdir.empty() && 
		(baseline.street.Last().flags & Token::IsStreetSuffix) != 0
	) {
		// Remove double suffix 
		//
		// 123 MAIN STREET COURT   --> 123  MAIN    STREET
		//	^  ^---------^   ^          ^     ^      ^     
		// nbr   street			       nbr  street  suffix
		ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
		newTokens = baseline;
		newTokens.suffix.clear();
		newTokens.suffix.push_back(newTokens.street.Last());
		newTokens.street.pop_back();
		// Mark changes
		newTokens.permutations |= AddressParserFirstLine::PermuteRemoveDoubleSuffix;
		newTokens.numberOfMods++;
		// Remove if this is a duplicate
		CheckNewParseCandidateForDuplication();
	}

	if (
		(permutationFlags & AddressParserFirstLine::PermuteRemoveDoubleUnit) != 0 &&
		!baseline.unitDesignator.empty() &&
		!baseline.unitNumber.empty() &&
		baseline.street.size() >= 2 &&
		baseline.suffix.empty() && 
		baseline.postdir.empty() && 
		(baseline.street.Last().flags & Token::IsUnitDesignator) != 0 &&
		baseline.unitDesignator[0].text[0] == '#'
	) {
		// Cleave unit from street and remove #
		// 123 MAIN APT   #       3      ->  123 MAIN  APT     3
		//     ^------^   ^       ^               ^     ^      ^
		//      street unitdes  unitnbr       street unitdes  unitnbr
		ParsedTokens& newTokens = parsedTokens.UseExtraOnEnd();
		newTokens = baseline;
		newTokens.unitDesignator[0] = newTokens.street.Last();
		newTokens.street.pop_back();
		// While we're at it, if the street now ends with a suffix do that as well.
		if (
			newTokens.street.size() >= 2 &&
			(newTokens.street.Last().flags & Token::IsStreetSuffix) != 0
		) {
			newTokens.suffix.push_back(newTokens.street.Last());
			newTokens.street.pop_back();
		}
		// Mark changes
		newTokens.permutations |= AddressParserFirstLine::PermuteRemoveDoubleUnit;
		newTokens.numberOfMods++;
		// Remove if this is a duplicate
		CheckNewParseCandidateForDuplication();
	}


	/*

		// Old permutation code.

		//
		// Replace two-part prefixes with single prefix token
		// e.g. DE L'   becomes one token
		//
		if (
			(token.flags & Token::IsPrefixToken) != 0 &&
			tokenIdx + 1 < tokenListAndFlags.tokenList.size() &&
			(tokenListAndFlags.tokenList[tokenIdx + 1].flags & Token::IsPrefixToken) != 0
		) {
			char tmpBuf[20];
			strcpy(tmpBuf, token.text);
			strcat(tmpBuf, " ");
			strcat(tmpBuf, tokenListAndFlags.tokenList[tokenIdx + 1].text);
			if (prefixesLexicon->Find(tmpBuf)) {
				// Make a new token list
				TokenListAndFlags& newTokenList = inputTokenLists.UseExtraOnEnd();
				newTokenList.tokenList.clear();
				newTokenList.flags = tokenListAndFlags.flags;
				// Copy tokens up to this one
				{for (int i = 0; i < tokenIdx; i++) {
					newTokenList.tokenList.push_back(tokenListAndFlags.tokenList[i]);
				}}
				// Append combined token
				Token newToken;
				SetTokenText(newToken, tmpBuf);
				newToken.flags |= Token::IsPrefix;
				newTokenList.tokenList.push_back(newToken);
				// Append remaining tokens
				{for (int i = tokenIdx + 2; i < tokenListAndFlags.tokenList.size(); i++) {
					newTokenList.tokenList.push_back(tokenListAndFlags.tokenList[i]);
				}}
				// We don't consider this to be a modification
				// Check for duplication
				CheckNewTokenListForDuplication();
			}
		}


		//
		// Permute each parse candidate using some standard and relatively innocuous
		// perturbations to each parse candidate.
		//
		for (int parsedTokensIdx = 0; parsedTokensIdx < parsedTokens.size(); parsedTokensIdx++) {

			const ParsedTokens& baseline = parsedTokens[parsedTokensIdx];

			if (!baseline.isIntersection) {
				// Permutations for non-intersection addresses

				// Swap suffix and postdir if possible
				// 123 ST-JOSEPH EST --> 123 ST-JOSEPH EST
				//     ^         ^           ^         ^
				//     street    suffix      street    postdir
				if (
					baseline.postdir.empty() &&
					baseline.suffix.size() == 1 &&
					directionalsLexicon->Find(baseline.suffix[0].text)
				) {
					ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
					newParsedTokens = baseline;
					newParsedTokens.postdir.push_back(newParsedTokens.suffix.Last());
					newParsedTokens.suffix.pop_back();
					// This is not considered to be a modification
					CheckNewParseCandidateForDuplication();
				}

				if (baseline.street.size() >= 2) {

					TokenList::const_iterator lastTokenIter = baseline.street.end() - 1;

					//
					// Remove suffixes from front of street name and place in suffix field
					//
					//     RUE SAINT GEORGE --> SAINT GEORGE RUE
					//     ^--------------^     ^----------^ ^
					//         street             street     suffix
					//
					if (
						(baseline.street[0].flags & Token::IsStreetSuffix) != 0 &&
						baseline.suffix.empty()
					) {
						ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
						newParsedTokens = baseline;
						TokenList::iterator streetTokenIter = newParsedTokens.street.begin();
						newParsedTokens.suffix.push_back(*streetTokenIter);
						newParsedTokens.street.erase(streetTokenIter);
						newParsedTokens.flags |= AddressParserFirstLine::ParseCandidate::SuffixWasPrefix;
						newParsedTokens.numberOfMods++;
						CheckNewParseCandidateForDuplication();
					}

					//
					// Remove prefix from front of street name and place in prefix field
					//
					//     DE L' ARGONNE --> DE L'   ARGONNE
					//     ^-----------^     ^---^   ^-----^
					//     street            prefix  street
					//
					if (
						(baseline.street[0].flags & Token::IsPrefix) != 0 &&
						baseline.prefix.empty()
					) {
						ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
						newParsedTokens = baseline;
						TokenList::iterator streetTokenIter = newParsedTokens.street.begin();
						newParsedTokens.prefix.push_back(*streetTokenIter);
						newParsedTokens.street.erase(streetTokenIter);
						// This is not considered a mod
						CheckNewParseCandidateForDuplication();
					}

					//
					// For street names containing leading "honorifics", replace the
					// honorific with an alias. This is necessary because the aliases 
					// do not have the same soundex as the original.
					//
					// ST JOSEPH --> SAINT JOSEPH
					//
					{
						const char *tmpPtr;
						if (streetHonorificAliasTable->Find(baseline.street[0].text, tmpPtr)) {
							ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
							newParsedTokens = baseline;
							SetTokenText(newParsedTokens.street[0], tmpPtr);
							newParsedTokens.numberOfMods++;
							CheckNewParseCandidateForDuplication();
						}
					}

#if 0
					//
					// For longer street names, replace first or second tokens of the street
					// name with common aliases.   This is necessary because the aliases 
					// do not have the same soundex as the original.
					//
					// AVENUE OF THE AMERICAS <--> AVE OF THE AMERICAS
					//
					{for (int i = 0; i < 2; i++) {
						const char* tmpPtr;
						if (streetComponentAliasTable->Find(baseline.street[i].text, tmpPtr)) {
							ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
							newParsedTokens = baseline;
							SetTokenText(newParsedTokens.street[i], tmpPtr);
							CheckNewParseCandidateForDuplication();
						}
					}}
#endif

				}


				//
				// Perform substitution of "SAINT" to "ST" in street name.
				//
				//   SAINT CLAIRE AVE --> ST CLAIRE AVE
				//
				{for (int i = 0; i < (int)baseline.street.size() - 1; i++) {
					if (strcmp(baseline.street[i].text, "SAINT") == 0) {
						ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
						newParsedTokens = baseline;
						// Replace the token with the new text
						SetTokenText(newParsedTokens.street[i], "ST");
						newParsedTokens.numberOfMods++;
						// Check for duplication
						CheckNewParseCandidateForDuplication();
					}
				}}


#if 0
				// If there is no addr#, but there is a pre-directional (or two),
				// and the first pre-directional is a single letter,
				// then move the first pre-directional to the addr#
				//      E 53RD ST #100  -->   E 53RD ST #100
				//      ^   ^                 ^   ^
				// predir  street         addr#  street
				if (
					baseline.number.empty() &&
					!baseline.predir.empty() &&
					baseline.predir[0].size == 1
				) {
					ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
					newParsedTokens = baseline;

					newParsedTokens.number.push_back(baseline.predir[0]);
					newParsedTokens.predir.erase(newParsedTokens.predir.begin());
					newParsedTokens.flags |= AddressParserFirstLine::ParseCandidate::SlideTokens;
					newParsedTokens.numberOfMods++;
					CheckNewParseCandidateForDuplication();
				}
#endif

				// 
				// If there is a non-digit addr number and no predir, then
				// move addr number to street.
				// 
				//      TOWN COURTHOUSE  -->  TOWN COURTHOUSE
				//       ^        ^            ^----------^
				//     number   street            street

				// Is a suffix of this token equal to a suffix without the
				// entire token being a suffix?
				if (
					baseline.number.size() == 1 &&
					baseline.predir.empty() &&
					!isdigit(baseline.number[0].text[0])
				) {
					ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
					newParsedTokens = baseline;

					newParsedTokens.street.insert(
						newParsedTokens.street.begin(), 
						baseline.number[0]
					);
					newParsedTokens.number.clear();
					newParsedTokens.flags |= AddressParserFirstLine::ParseCandidate::SlideTokens;
					newParsedTokens.numberOfMods++;
					CheckNewParseCandidateForDuplication();
				}

#if 0
				// 
				// If there is no addr#, no predir, the street has
				// two tokens, and the first one is a number, then move
				// the first street token to the address number
				// 
				//      1000 OAKS RD    -->    1000  OAKS   RD
				//      ^-------^                ^    ^
				//        street             number street

				// Is a suffix of this token equal to a suffix without the
				// entire token being a suffix?
				if (
					baseline.number.empty() &&
					baseline.predir.empty() &&
					baseline.street.size() >= 2 &&
					(baseline.street[0].flags & Token::HasDigit) != 0
				) {
					ParsedTokens& newParsedTokens = parsedTokens.UseExtraOnEnd();
					newParsedTokens = baseline;

					newParsedTokens.number.push_back(
						baseline.street[0]
					);
					newParsedTokens.street.erase(
						newParsedTokens.street.begin()
					);
					newParsedTokens.flags |= AddressParserFirstLine::ParseCandidate::SlideTokens;
					newParsedTokens.numberOfMods++;
					CheckNewParseCandidateForDuplication();
				}
#endif

			} // if (intersection) else

		}	// candidate loop
		*/
	}
	
	//////////////////////////////////////////////////////////////////////
	// Retrieve the next address permutation.
	// Return value:
	//	bool	true if the next permutation is returned, false
	//			if there are no more permutations.
	//////////////////////////////////////////////////////////////////////
	bool AddressParserFirstLineImp::NextAddressPermutation(
		AddressParserFirstLine::ParseCandidate& parseCandidate,
		bool replaceAliases
	) {
		if (nextCandidateIdx >= parsedTokens.size()) {
			return false;
		}

		//Fill the parsecandidate and return it
		ParsedTokensToParseCandidate(
			parseCandidate, 
			parsedTokens[nextCandidateIdx++],
			replaceAliases
		);

		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// Create a new, cleared out, ParsedTokens for use in parsing
	//////////////////////////////////////////////////////////////////////
	AddressParserFirstLineImp::ParsedTokens& AddressParserFirstLineImp::GetNewParsedTokens()
	{
		parsedTokens.UseExtraOnEnd();
		parsedTokens.Last().Clear();
		return parsedTokens.Last();
	}

	//////////////////////////////////////////////////////////////////////
	// Check the last parse candidate added to the parsedTokens vector, and
	// remove it if it is duplicate.
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::CheckNewParseCandidateForDuplication()
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
	//////////////////////////////////////////////////////////////////////
	// ParsedTokens methods.
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Clear all of the token lists.
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::ParsedTokens::Clear() 
	{
		number.clear();
		street.clear();
		predir.clear();
		prefix.clear();
		postdir.clear();
		suffix.clear();
		unitDesignator.clear();
		unitNumber.clear();
		discarded.clear();
		street2.clear();
		predir2.clear();
		suffix2.clear();
		pmbDesignator.clear();
		pmbNumber.clear();
		andTokens.clear();
		postdir2.clear();
		permutations = 0;
		numberOfMods = 0;
	}

	//////////////////////////////////////////////////////////////////////
	// equality comparison
	//////////////////////////////////////////////////////////////////////
	bool AddressParserFirstLineImp::ParsedTokens::operator==(const ParsedTokens& rhs) const {
		return 
			number == rhs.number &&
			street == rhs.street &&
			street2 == rhs.street2 &&
			predir == rhs.predir &&
			prefix == rhs.prefix &&
			postdir == rhs.postdir &&
			suffix == rhs.suffix &&
			unitDesignator == rhs.unitDesignator &&
			unitNumber == rhs.unitNumber &&
			pmbDesignator == rhs.pmbDesignator;
			pmbNumber == rhs.pmbNumber;
	}

	//////////////////////////////////////////////////////////////////////
	// Hash the token lists into the given hash buffer.
	//////////////////////////////////////////////////////////////////////
	void AddressParserFirstLineImp::ParsedTokens::Hash(TokenHashBuffer& hashBuffer) const {
		hashBuffer.Clear();
		hashBuffer.Hash(number, 1);
		hashBuffer.Hash(street, 10);
		hashBuffer.Hash(street2, 15);
		hashBuffer.Hash(predir, 176);
		hashBuffer.Hash(prefix, 84);
		hashBuffer.Hash(postdir, 53);
		hashBuffer.Hash(suffix, 19);
		hashBuffer.Hash(unitNumber, 131);
		hashBuffer.Hash(pmbDesignator, 201);
		hashBuffer.Hash(pmbNumber, 202);
	}

}	// namespace 


