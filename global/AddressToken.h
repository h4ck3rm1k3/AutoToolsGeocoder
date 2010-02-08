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
// AddressToken.h: Tokens and lists thereof, and operations to assist
// AddressTokenizer.
//////////////////////////////////////////////////////////////////////

#ifndef INCL_AddressToken_H
#define INCL_AddressToken_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <vector>
#include <assert.h>
#include "Global_DllExport.h"
#include "TsString.h"

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Data structure used when "tokenizing" the input.
	//////////////////////////////////////////////////////////////////////
	struct Token {
		Token(const char* text_, int flags_ = 0) : 
			text(text_), size(int(strlen((const char*)text_))), flags(flags_)
		{}
		Token() : 
			text(0), size(0), flags(0)
		{}

		bool operator==(const Token& rhs) const {
			return
				size == rhs.size &&
				strcmp((const char*)text, (const char*)rhs.text) == 0;

		}

		// Convert token to buffer
		void ToBuffer(char* buf, int bufSize) const {
			bufSize--;	// room for termination
			int length = (bufSize < size) ? bufSize : size;
			memcpy(buf, text, length);
			buf[length] = 0;
		}

		// The null-terminated token text.
		// Points to either input-record text or a scratch buffer.
		const char* text;
		// Size of the token text (not including null termination)
		int size;
		// Flags indicating what has been determined about the token.
		int flags;
		enum { 
			HasDigit =					0x1,
			IsState =					0x4,
			IsNumber =					0x8,
			IsDirectional =				0x20,
			IsStreetSuffix =			0x40, 
			IsUnitDesignator =			0x80,
			IsZip =						0x100,
			IsFraction =				0x200,
			IsNumberDashNumber =		0x400,
			IsNumberDotNumber =			0x800,
			HasLeading5Digits =			0x1000,
			IsCaZip1 =					0x2000,		// e.g. K1H
			IsCaZip2 =					0x4000,		// e.g. 4F8
			IsCaZip3 =					0x8000,		// e.g. K1N9
			IsCaZip =					0x10000,	// e.g. K1N9F4
			Is9Digit =					0x20000,	// e.g. 123456789
			IsPrefixToken =				0x40000,	// e.g. ALA
			IsPrefix =					0x80000,	// e.g. ALA
			Is5Dash4Digit =				0x100000,
			IsReservedWord = IsDirectional | IsStreetSuffix | IsUnitDesignator,
			IsAnyZip = IsZip | Is9Digit | IsCaZip1 | IsCaZip2 | IsCaZip3 | IsCaZip,
			IsFullZip = IsZip | IsCaZip | IsCaZip3 | Is9Digit
		};
	};

	//////////////////////////////////////////////////////////////////////
	// Tokens from parsed inputs
	//////////////////////////////////////////////////////////////////////
	typedef std::vector<Token> TokenListBase_;
	class TokenList : public TokenListBase_ {
	public:
		TokenList() {}
		TokenList(const TokenList& rhs) : TokenListBase_(rhs) {}
		TokenList& operator=(const TokenList& rhs) {
			TokenListBase_::operator=(rhs);
			return *this;
		}

		//////////////////////////////////////////////////////////////////////
		// Get the last token in the list
		//////////////////////////////////////////////////////////////////////
		const Token& Last() const {
			assert(!empty());
			return at(size() - 1);
		}

		//////////////////////////////////////////////////////////////////////
		// Get the last token in the list
		//////////////////////////////////////////////////////////////////////
		Token& Last() {
			assert(!empty());
			return at(size() - 1);
		}

		//////////////////////////////////////////////////////////////////////
		// Find a token containing the given flag(s), starting at the given position.
		//////////////////////////////////////////////////////////////////////
		bool FindTokenByFlag(
			int flag,
			int& positionReturn,
			int startPosition = 0
		) const {
			for (const_iterator iter = begin() + startPosition; iter != end(); ++iter) {
				if (((*iter).flags & flag) != 0) {
					positionReturn = int(iter - begin());
					return true;
				}
			}
			return false;
		}

		//////////////////////////////////////////////////////////////////////
		// Find a token containing equal to the given string, 
		// starting at the given position.
		//////////////////////////////////////////////////////////////////////
		bool FindTokenByText(
			const char* text,
			int& positionReturn,
			int startPosition = 0
		) const {
			for (const_iterator iter = begin() + startPosition; iter != end(); ++iter) {
				if (strcmp((*iter).text, text) == 0) {
					positionReturn = int(iter - begin());
					return true;
				}
			}
			return false;
		}

		//////////////////////////////////////////////////////////////////////
		// Convert token list to buffer
		//////////////////////////////////////////////////////////////////////
		void ToBuffer(char* buf, int bufSize) const {
			assert(bufSize > 0);

			if (empty()) {
				*buf = 0;
				return;
			}

			char* ptr = buf;
			int availSize = bufSize - 1;

			// First token
			int length = (availSize > at(0).size) ? at(0).size : availSize;
			memcpy(ptr, at(0).text, length);
			ptr += length;
			availSize -= length;

			// Remaining tokens
			for (int i = 1; availSize > 1 && i < int(size()); i++) {
				availSize--;
				*ptr++ = ' ';
				length = (availSize > at(i).size) ? at(i).size : availSize;
				memcpy(ptr, at(i).text, length);
				ptr += length;
				availSize -= length;
			}
			*ptr = 0;
		}

		//////////////////////////////////////////////////////////////////////
		// Convert token list to a string
		//////////////////////////////////////////////////////////////////////
		void ToString(
			TsString& str,
			int minIdx,
			int maxIdx
		) const {
			str = "";
			for (int i = minIdx; i <= maxIdx && i < int(size()); i++) {
				if (i != 0) {
					str += " ";
				}
				str += at(i).text;
			}
		}
		void ToString(TsString& str) const {
			ToString(str, 0, int(size())-1);
		}


	};
	typedef TokenList::iterator TokenListIterator;


	//////////////////////////////////////////////////////////////////////
	// Data structures used to compared hashed versions of each candidate parse.
	// This is used to eliminate duplicates.  Very simple, but fast and effective
	// because of the normally-small number of candidates.
	//////////////////////////////////////////////////////////////////////
	class TokenHashBuffer {
	public:
		TokenHashBuffer() { Clear(); }
		bool operator==(const TokenHashBuffer& rhs) const {
			return memcmp(buffer, rhs.buffer, HashSize) == 0;
		}
		// Clear the hash value.
		void Clear() {
			memset(buffer, 0, sizeof(buffer));
			offset = 0;
		}
		// Hash a token and add it to the hash value.
		void Hash(const Token& token, unsigned char salt) {
			for (const char* ptr = token.text; *ptr != 0; ptr++) {
				HashChar(*ptr);
			}
			// Add a trailing salt after each token
			HashChar(salt);
		}
		// Hash a token list and add it to the hash value.
		void Hash(const TokenList& tokenList, unsigned char salt) {
			for (TokenList::const_iterator iter = tokenList.begin(); iter != tokenList.end(); ++iter) {
				Hash(*iter, salt);
			}
		}
	private:
		void HashChar(unsigned char c) {
			buffer[offset] += c;
			offset = (offset == HashSize-1) ? 0 : offset+1;
		}
		enum { HashSize = 5 };
		char buffer[HashSize];
		int offset;
	};

	//////////////////////////////////////////////////////////////////////
	// Tracks a list of candidates via their hashed values, and can
	// determine if a candidate has been previously generated.
	//////////////////////////////////////////////////////////////////////
	class HashedCandidateTable {
	public:
		// This is fast as implemented when the number of candidates
		// is less than 12 or so, which is normally the case.
		bool Find(const TokenHashBuffer& hashBuffer) const {
			for (unsigned i = 0; i < hashVector.size(); i++) {
				if (hashBuffer == hashVector[i]) {
					return true;
				}
			}
			return false;
		}

		void Add(const TokenHashBuffer& hashBuffer) {
			hashVector.push_back(hashBuffer);
		}

		void Clear() { hashVector.clear(); }
	private:
		std::vector<TokenHashBuffer> hashVector;
	};

}

#endif
