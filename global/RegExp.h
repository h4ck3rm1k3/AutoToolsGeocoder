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

////////////////////////////////////////////////////////////////////////
// RegExp.h
//
// This code has been derived from work by Henry Spencer. 
// The main changes are
// 1. All char variables and functions have been changed to char
//    counterparts
// 2. Added GetFindLen() & GetReplaceString() to enable search
//    and replace operations.
// 3. And of course, added the C++ Wrapper
//
// The original copyright notice follows:
//
// Copyright (c) 1986, 1993, 1995 by University of Toronto.
// Written by Henry Spencer.  Not derived from licensed software.
//
// Permission is granted to anyone to use this software for any
// purpose on any computer system, and to redistribute it in any way,
// subject to the following restrictions:
//
// 1. The author is not responsible for the consequences of use of
// this software, no matter how awful, even if they arise
// from defects in it.
//
// 2. The origin of this software must not be misrepresented, either
// by explicit claim or by omission.
//
// 3. Altered versions must be plainly marked as such, and must not
// be misrepresented (by explicit claim or omission) as being
// the original software.
//
// 4. This notice must not be removed or altered.
/////////////////////////////////////////////////////////////////////////////

#ifndef INCL_REGEXP_H
#define INCL_REGEXP_H

#include "auto_ptr_array.h"
#include "RefPtr.h"
#include "TsString.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class RegExp : public VRefCount {
	public:

		/////////////////////////////////////////////////////////////////////////////
		// Default construction and destruction.
		/////////////////////////////////////////////////////////////////////////////
		RegExp();
		~RegExp();

		/////////////////////////////////////////////////////////////////////////////
		// Copy construction
		/////////////////////////////////////////////////////////////////////////////
		RegExp(const RegExp& rhs);

		/////////////////////////////////////////////////////////////////////////////
		// Compile a regular expression.
		// Inputs:
		//	const char*		re		The regular expression to compile.
		// Return value:
		//	bool		true on success, false on error.  If false is returned,
		//				call GetErrorMessage() for a textual description of the
		//				error.
		/////////////////////////////////////////////////////////////////////////////
		bool RegComp( const char *re );

		/////////////////////////////////////////////////////////////////////////////
		// Search a string for a regexp match.
		// Inputs:
		//	const char*		str		String to search
		// Return value:
		//	int				position of match in the string, or -1 if no match
		/////////////////////////////////////////////////////////////////////////////
		int RegFind(const char *str);

		/////////////////////////////////////////////////////////////////////////////
		// Get pointer to found-text buffer.  Do not free this pointer or
		// alter its contents.  Valid until next RegFind() or RegComp(), 
		// or until object is dsstructed.  This equivalent to 
		// GetReplaceString("&") but faster.
		/////////////////////////////////////////////////////////////////////////////
		const char* GetFoundText() const { return sFoundText; }

		/////////////////////////////////////////////////////////////////////////////
		// Build a replacement string from the replacement expression.
		// From the original documenation:
		// After a successful Match you can retrieve a replacement string as an 
		// alternative to building up the various substrings by hand. 
		// Each character in the source string will be copied to the return 
		// value except for the following special characters:
		//     &		The complete matched string (sub-string 0).
		//    \1		Sub-string 1
		// ...and so on until...
		//    \9		Sub-string 9
		// 
		// So, taking the example:
		//    TsString repl;
		//    re.GetReplacementString(repl, "\\2 == \\1" );
		// Will give:
		// repl == "Kelly == wyrdrune.com!kelly"; 
		/////////////////////////////////////////////////////////////////////////////
		void GetReplaceString(TsString& result, const char* sReplaceExp);

		/////////////////////////////////////////////////////////////////////////////
		// Get the length of the matched substring.
		// Return value:
		//	int		length of the matched substring.
		/////////////////////////////////////////////////////////////////////////////
		int GetFindLen() {
			if(startp[0] == 0 || endp[0] == 0) {
				return 0;
			}
			return int(endp[0] - startp[0]);
		}

		/////////////////////////////////////////////////////////////////////////////
		// Get the last error message.  Call this if an error was returned from
		// RegComp().
		/////////////////////////////////////////////////////////////////////////////
		const TsString& GetErrorMessage() const { return errorMessage; }

		/////////////////////////////////////////////////////////////////////////////
		// Maximum number of sub-expressions allowed.  A sub-expression is a 
		// top-level parenthesized expression.  The entire RE counts as one
		// sub-expression, so you effectively get one less.
		/////////////////////////////////////////////////////////////////////////////
		enum { NSUBEXP = 10 };

		/////////////////////////////////////////////////////////////////////////////
		// If there is a character that must be the first char in the string, 
		// then return it.
		// Outputs:
		//	char&		anchor		The anchor character
		// Return value:
		//	bool		true if there is an anchor, false otherwise
		/////////////////////////////////////////////////////////////////////////////
		bool GetAnchorChar(char& anchor) {
			anchor = anchorChar;
			return anchor != 0;
		}

		/////////////////////////////////////////////////////////////////////////////
		// Has this regexp been compiled with a valid expression?
		// Return value:
		//	bool		true if valid, false OW
		/////////////////////////////////////////////////////////////////////////////
		bool IsValid() const { return bCompiled; }

	private:
		// Initialize regexp members
		void Init();

		// Methods from original "C" regexp package.
		char *regnext(char *node);
		void reginsert(char op, char *opnd);
		int regtry(char *string);
		int regmatch(char *prog);
		size_t regrepeat(char *node);
		char *reg(int paren, int *flagp);
		char *regbranch(int *flagp);
		void regtail(char *p, char *val);
		void regoptail(char *p, char *val);
		char *regpiece(int *flagp);
		char *regatom(int *flagp);

		// Record an error.
		void Error(const TsString& msg) {
			errorMessage = msg;
		}

		// Allocate space in sFoundText
		void AllocateFoundText(int len) {
			if (foundTextLen < len) {
				foundTextLen = len;
				sFoundText = new char[len];
			}
		}

		// Inline functions
		char OP(char *p) {return *p;};
		char *OPERAND( char *p) {return (char*)((short *)(p+1)+1); };

		// regc - emit (if appropriate) a byte of code
		void regc(char b)
		{
			if (bEmitCode) {
				*regcode++ = b;
			} else {
				regsize++;
			}
		};

		// regnode - emit a node
		char * regnode(char op)
		{
			if (!bEmitCode) {
				regsize += 3;
				return regcode;
			}

			*regcode++ = op;
			*regcode++ = ('\0');          /* Null next pointer. */
			*regcode++ = ('\0');

			return regcode-3;
		}

		bool bEmitCode;
		bool bCompiled;

		auto_ptr_array<char> sFoundText;
		int foundTextLen;

		char *startp[NSUBEXP];
		char *endp[NSUBEXP];
		char regstart;
		bool reganch;			// true if anchored
		char anchorChar;		// non-zero if reganch and the next op is literal
		char *regmust;
		int regmlen;
		auto_ptr_array<char> program; 

		char *regparse;        // Input-scan pointer. 
		int regnpar;            // () count. 
		char *regcode;         // Code-emit pointer; &regdummy = don't. 
		char regdummy[3];      // NOTHING, 0 next ptr 
		long regsize;           // Code size. 

		char *reginput;        // String-input pointer. 
		char *regbol;          // Beginning of input, for ^ check. 

		// Last error message
		TsString errorMessage;

		// Copy of original RE string
		TsString originalRe;
	};

	// Reference-counted smart pointer to a RegExp.
	typedef refcnt_ptr<RegExp> RegExpRef;

}

#endif
