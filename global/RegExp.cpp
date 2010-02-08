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
# $Rev: 49 $ 
# $Date: 2006-09-25 20:00:58 +0200 (Mon, 25 Sep 2006) $ 
*/

////////////////////////////////////////////////////////////////////////////////
// RegExp.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Global_Headers.h"
#include "RegExp.h"
#include <ctype.h>
#include <string.h>

namespace PortfolioExplorer {

	// Operator definitions
	// definition   number  opnd?   meaning 
	enum Operators {
		Op_END     =0,               // no   End of program. 
		Op_BOL     =1,               // no   Match beginning of line. 
		Op_EOL     =2,               // no   Match end of line. 
		Op_ANY     =3,               // no   Match any character. 
		Op_ANYOF   =4,               // str  Match any of these. 
		Op_ANYBUT  =5,               // str  Match any but one of these. 
		Op_BRANCH  =6,               // node Match this, or the next..\&. 
		Op_BACK    =7,               // no   "next" ptr points backward. 
		Op_EXACTLY =8,               // str  Match this string. 
		Op_NOTHING =9,               // no   Match empty string. 
		Op_STAR    =10,              // node Match this 0 or more times. 
		Op_PLUS    =11,              // node Match this 1 or more times. 
		Op_OPEN    =20,              // no   Sub-RE starts here. 
									 //      Op_OPEN+1 is number 1, etc. 
		Op_CLOSE   =Op_OPEN+RegExp::NSUBEXP     // no   Analogous to Op_OPEN. 
	};

	// Utility definitions.

	#define ISREPN(c)       ((c) == ('*') || (c) == ('+') || (c) == ('?'))
	#define META            "^$.[()|?+*\\"

	// Flags to be passed up and down.
	#define HASWIDTH        01      // Known never to match null string. 
	#define SIMPLE          02      // Simple enough to be Op_STAR/Op_PLUS operand. 
	#define SPSTART         04      // Starts with * or +. 
	#define WORST           0       // Worst case. 


	/////////////////////////////////////////////////////////////////////////////
	// Default construction and destruction.
	/////////////////////////////////////////////////////////////////////////////
	RegExp::RegExp() :
		foundTextLen(0)
	{
		Init();
	}

	RegExp::~RegExp()
	{
	}

	/////////////////////////////////////////////////////////////////////////////
	// Copy construction
	/////////////////////////////////////////////////////////////////////////////
	RegExp::RegExp(const RegExp& rhs)
	{
		Init();
		if (!rhs.originalRe.empty()) {
			RegComp(rhs.originalRe.c_str());
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// Initialize the regexp.
	/////////////////////////////////////////////////////////////////////////////
	void RegExp::Init()
	{
		bEmitCode = false;
		bCompiled = false;
		errorMessage = "";
		originalRe = "";
		for (int i = 0; i < NSUBEXP; i++)
		{
			startp[i] = 0;
			endp[i] = 0;
		}

		regstart = 0;
		reganch = 0;
		regmust = 0;
		regmlen = 0;
		program = 0; 
		regparse = 0;
		regnpar = 0;
		regcode = 0;
		strcpy(regdummy, "");
		regsize = 0;
		reginput = 0;
		regbol = 0;
	}


	/////////////////////////////////////////////////////////////////////////////
	// Compile a regular expression.
	/////////////////////////////////////////////////////////////////////////////
	bool RegExp::RegComp(const char *exp)
	{
		char *scan;
		int flags;

		if (exp == 0) {
			return false;
		}

		// Save original expression
		originalRe = exp;

		// Reset various pointers
		Init();

		// First pass: determine size, legality. 
		bEmitCode = false;
		regparse = (char *)exp;
		regnpar = 1;
		regsize = 0L;
		regdummy[0] = Op_NOTHING;
		regdummy[1] = regdummy[2] = 0;
		regcode = regdummy;
		if (reg(0, &flags) == 0) {
			return false;
		}

		// Allocate space. 
		program = new char[regsize];
		memset(program, 0, regsize);

		if (program == 0) {
			return false;
		}

		// Second pass: emit code. 
		bEmitCode = true;
		regparse = (char *)exp;
		regnpar = 1;
		regcode = program;
		if (reg(0, &flags) == 0) {
			return false;
		}

		// Dig out information for optimizations. 
		regstart = ('\0');            // Worst-case defaults. 
		reganch = false;
		anchorChar = 0;
		regmust = 0;
		regmlen = 0;
		scan = program;         // First Op_BRANCH. 

		if (OP(regnext(scan)) == Op_END) {       
			// Only one top-level choice. 
			scan = OPERAND(scan);

			// Starting-point info. 
			if (OP(scan) == Op_EXACTLY) {
				regstart = *OPERAND(scan);
			} else if (OP(scan) == Op_BOL) {
				reganch = true;
				// Look for an anchor char.
				char* firstop = regnext(scan);
				if (firstop != 0 && OP(firstop) == Op_EXACTLY) {
					anchorChar = *OPERAND(scan);
				}
			}

			// If there's something expensive in the r.e., find the
			// longest literal string that must appear and make it the
			// regmust.  Resolve ties in favor of later strings, since
			// the regstart check works with the beginning of the r.e.
			// and avoiding duplication strengthens checking.  Not a
			// strong reason, but sufficient in the absence of others.
			if (flags & SPSTART) {
				char *longest = 0;
				size_t len = 0;

				for (; scan != 0; scan = regnext(scan)) {
					if (OP(scan) == Op_EXACTLY && strlen(OPERAND(scan)) >= len) {
						longest = OPERAND(scan);
						len = strlen(OPERAND(scan));
					}
				}
				regmust = longest;
				regmlen = (int)len;
			}
		}

		bCompiled = true;
		return true;
	}

	// reg - regular expression, i.e. main body or parenthesized thing
	//
	// Caller must absorb opening parenthesis.
	//
	// Combining parenthesis handling with the base level of regular expression
	// is a trifle forced, but the need to tie the tails of the branches to what
	// follows makes it hard to avoid.



	char *RegExp::reg(int paren, int *flagp)
	{
		char *ret;
		char *br;
		char *ender;
		int parno;
		int flags;

		*flagp = HASWIDTH;      // Tentatively. 

		if (paren) 
		{
			// Make an Op_OPEN node. 
			if (regnpar >= NSUBEXP)
			{
				Error("Too many subexpressions");
				return 0;
			}
			parno = regnpar;
			regnpar++;
			ret = regnode(Op_OPEN+parno);
		}

		// Pick up the branches, linking them together. 
		br = regbranch(&flags);
		if (br == 0) {
			return 0;
		}
		if (paren) {
			regtail(ret, br);       // Op_OPEN -> first. 
		} else {
			ret = br;
		}
		*flagp &= ~(~flags&HASWIDTH);   // Clear bit if bit 0. 
		*flagp |= flags&SPSTART;
		while (*regparse == ('|')) {
			regparse++;
			br = regbranch(&flags);
			if (br == 0) {
				return 0;
			}
			regtail(ret, br);       // Op_BRANCH -> Op_BRANCH. 
			*flagp &= ~(~flags&HASWIDTH);
			*flagp |= flags&SPSTART;
		}

		// Make a closing node, and hook it on the end. 
		ender = regnode((paren) ? Op_CLOSE+parno : Op_END);
		regtail(ret, ender);

		// Hook the tails of the branches to the closing node. 
		for (br = ret; br != 0; br = regnext(br)) {
			regoptail(br, ender);
		}

		// Check for proper termination. 
		if (paren && *regparse++ != (')')) {
			Error("unterminated ()\n");
			return 0;
		} else if (!paren && *regparse != ('\0')) {
			if (*regparse == (')')) {
				Error("unmatched ()\n");
				return 0;
			} else {
				Error("internal error: junk after end of expression\n");
				return 0;
			}
		}

		return(ret);
	}




	//
	// regbranch - one alternative of an | operator
	//
	// Implements the concatenation operator.

	char *RegExp::regbranch(int *flagp)
	{
		char *ret;
		char *chain;
		char *latest;
		int flags;
		int c;

		*flagp = WORST;                         // Tentatively. 

		ret = regnode(Op_BRANCH);
		chain = 0;

		while ((c = *regparse) != ('\0') && c != ('|') && c != (')')) {
			latest = regpiece(&flags);
			if (latest == 0) {
				return 0;
			}
			*flagp |= flags&HASWIDTH;
			if (chain == 0) {
				// First piece. 
				*flagp |= flags & SPSTART;
			} else {
				regtail(chain, latest);
			}
			chain = latest;
		}
		if (chain == 0) {
			// Loop ran zero times. 
			(void) regnode(Op_NOTHING);
		}

		return(ret);
	}

	//
	// regpiece - something followed by possible [*+?]
	//
	// Note that the branching code sequences used for ? and the general cases
	// of * and + are somewhat optimized:  they use the same Op_NOTHING node as
	// both the endmarker for their branch list and the body of the last branch.
	// It might seem that this node could be dispensed with entirely, but the
	// endmarker role is not redundant.

	char *RegExp::regpiece(int *flagp)
	{
		char *ret;
		char op;
		char *next;
		int flags;

		ret = regatom(&flags);
		if (ret == 0) {
			return 0;
		}

		op = *regparse;
		if (!ISREPN(op)) {
			*flagp = flags;
			return(ret);
		}

		if (!(flags&HASWIDTH) && op != ('?'))
		{
			Error("*+ operand could be empty\n");
			return 0;
		}

		switch (op) {
			case ('*'):
				*flagp = WORST|SPSTART;
				break;
			case ('+'):
				*flagp = WORST|SPSTART|HASWIDTH;
				break;
			case ('?'):
				*flagp = WORST;
				break;
		}

		if (op == ('*') && (flags&SIMPLE)) {
			reginsert(Op_STAR, ret);
		} else if (op == ('*')) {
			// Emit x* as (x&|), where & means "self". 
			reginsert(Op_BRANCH, ret);         // Either x 
			regoptail(ret, regnode(Op_BACK));  // and loop 
			regoptail(ret, ret);            // back 
			regtail(ret, regnode(Op_BRANCH));  // or 
			regtail(ret, regnode(Op_NOTHING)); // null. 
		} else if (op == ('+') && (flags&SIMPLE)) {
			reginsert(Op_PLUS, ret);
		} else if (op == ('+')) {
			// Emit x+ as x(&|), where & means "self". 
			next = regnode(Op_BRANCH);         // Either 
			regtail(ret, next);
			regtail(regnode(Op_BACK), ret);    // loop back 
			regtail(next, regnode(Op_BRANCH)); // or 
			regtail(ret, regnode(Op_NOTHING)); // null. 
		} else if (op == ('?')) {
			// Emit x? as (x|) 
			reginsert(Op_BRANCH, ret);         // Either x 
			regtail(ret, regnode(Op_BRANCH));  // or 
			next = regnode(Op_NOTHING);                // null. 
			regtail(ret, next);
			regoptail(ret, next);
		}

		regparse++;
		if (ISREPN(*regparse))
		{
			Error("nested *?+\n");
			return 0;
		}

		 return(ret);
	}

	//
	// regatom - the lowest level
	//
	// Optimization:  gobbles an entire sequence of ordinary characters so that
	// it can turn them into a single node, which is smaller to store and
	// faster to run.  Backslashed characters are exceptions, each becoming a
	// separate node; the code is simpler that way and it's not worth fixing.

	char *RegExp::regatom(int *flagp)
	{
		char *ret;
		int flags;
		*flagp = WORST;         // Tentatively. 

		switch (*regparse++) {
			case ('^'):
				ret = regnode(Op_BOL);
				break;
			case ('$'):
				ret = regnode(Op_EOL);
				break;
			case ('.'):
				ret = regnode(Op_ANY);
				*flagp |= HASWIDTH|SIMPLE;
				break;
			case ('['): {
				int range;
				int rangeend;
				int c;

				if (*regparse == ('^')) {     // Complement of range. 
					ret = regnode(Op_ANYBUT);
					regparse++;
				} else {
					ret = regnode(Op_ANYOF);
				}
				if ((c = *regparse) == (']') || c == ('-')) {
					regc(c);
					regparse++;
				}
				while ((c = *regparse++) != ('\0') && c != (']')) {
					if (c != ('-')) {
						regc(c);
					} else if ((c = *regparse) == (']') || c == ('\0')) {
						regc(('-'));
					} else {
						range = (unsigned) (char)*(regparse-2);
						rangeend = (unsigned) (char)c;
						if (range > rangeend)
						{
						Error("invalid [] range\n");
						return 0;
						}
						for (range++; range <= rangeend; range++)
						regc(range);
						regparse++;
					}
				}
				regc(('\0'));
				if (c != (']')) {
					Error("unmatched []\n");
					return 0;
				}
				*flagp |= HASWIDTH|SIMPLE;
				break;
			}
			case ('('):
				ret = reg(1, &flags);
				if (ret == 0) {
					return 0;
				}
				*flagp |= flags&(HASWIDTH|SPSTART);
				break;
			case ('\0'):
			case ('|'):
			case (')'):
				// supposed to be caught earlier 
				Error("internal error: \\0|) unexpected\n");
				return 0;
				break;
			case ('?'):
			case ('+'):
			case ('*'):
				Error("?+* follows nothing\n");
				return 0;
				break;
			case ('\\'):
				if (*regparse == ('\0')) {
					Error("trailing \\\n");
					return 0;
				}
				ret = regnode(Op_EXACTLY);
				regc(*regparse++);
				regc(('\0'));
				*flagp |= HASWIDTH|SIMPLE;
				break;
			default: {
				size_t len;
				char ender;

				regparse--;
				len = strcspn(regparse, META);
				if (len == 0) {
					Error("internal error: strcspn 0\n");
					return 0;
				}
				ender = *(regparse+len);
				if (len > 1 && ISREPN(ender)) {
					len--;          // Back off clear of ?+* operand. 
				}
				*flagp |= HASWIDTH;
				if (len == 1) {
					*flagp |= SIMPLE;
				}
				ret = regnode(Op_EXACTLY);
				for (; len > 0; len--) {
					regc(*regparse++);
				}
				regc(('\0'));
				break;
			}
		}

		return(ret);
	}



	// reginsert - insert an operator in front of already-emitted operand
	//
	// Means relocating the operand.

	void RegExp::reginsert(char op, char *opnd)
	{
		 char *place;

		 if (!bEmitCode) {
			 regsize += 3;
			 return;
		 }

		 (void) memmove(opnd+3, opnd, (size_t)((regcode - opnd)*sizeof(char)));
		 regcode += 3;

		 place = opnd;           // Op node, where operand used to be. 
		 *place++ = op;
		 *place++ = ('\0');
		 *place++ = ('\0');
	}

	//
	// regtail - set the next-pointer at the end of a node chain

	void RegExp::regtail(char *p, char *val)
	{
		 char *scan;
		 char *temp;
	//      int offset;

		 if (!bEmitCode)
			 return;

		 // Find last node. 
		 for (scan = p; (temp = regnext(scan)) != 0; scan = temp)
			 continue;

		 *((short *)(scan+1)) = (OP(scan) == Op_BACK) ? short(scan - val) : short(val - scan);
	}


	// regoptail - regtail on operand of first argument; nop if operandless

	void RegExp::regoptail(char *p, char *val)
	{
		 // "Operandless" and "op != Op_BRANCH" are synonymous in practice. 
		 if (!bEmitCode || OP(p) != Op_BRANCH)
			 return;
		 regtail(OPERAND(p), val);
	}


	////////////////////////////////////////////////////////////////////////////
	// RegFind      - match a regexp against a string
	// Returns      - Returns position of regexp or -1
	//                        if regular expression not found
	// Note         - The regular expression should have been
	//                        previously compiled using RegComp
	////////////////////////////////////////////////////////////////////////////
	int RegExp::RegFind(const char *str)
	{
		char *tmpStr = (char *)str;   // avert const poisoning 
		char *s;

		// Be paranoid. 
		if(tmpStr == 0) {
			Error("Null argument to RegFind\n");
			return(-1);
		}

		// Check validity of regex
		if (!bCompiled) {
			Error("No regular expression provided yet.\n");
			return(-1);
		}

		// If there is a "must appear" string, look for it. 
		if (regmust != 0 && strstr(tmpStr, regmust) == 0) {
			return(-1);
		}

		// Mark beginning of line for ^
		regbol = tmpStr;

		// Simplest case:  anchored match need be tried only once. 
		if (reganch) {
			if( regtry(tmpStr)) {
				// Save the found substring in case we need it
				AllocateFoundText(GetFindLen() + 1);
				sFoundText[GetFindLen()] = ('\0');
				strncpy(sFoundText, tmpStr, GetFindLen() );
				return 0;
			}
			// String not found
			return -1;
		}

		 // Messy cases:  unanchored match. 
		if (regstart != ('\0')) {
			// We know what char it must start with. 
			for (s = tmpStr; s != 0; s = strchr(s+1, regstart)) {
				if (regtry(s)) {
					int nPos = int(s - str);
					// Save the found substring in case we need it later
					AllocateFoundText(GetFindLen() + 1);
					sFoundText[GetFindLen()] = ('\0');
					strncpy(sFoundText, s, GetFindLen() );
					return nPos;
				}
			}
			return -1;
		} else {
			// We don't know starting char -- general case
			for (s = tmpStr; !regtry(s); s++) {
				if (*s == ('\0')) {
					return(-1);
				}
			}

			int nPos = int(s-str);

			// Save the found substring in case we need it later
			AllocateFoundText(GetFindLen() + 1);
			sFoundText[GetFindLen()] = ('\0');
			strncpy(sFoundText, s, GetFindLen() );

			return nPos;
		}
	}


	// regtry - try match at specific point

	int RegExp::regtry(char *tmpStr)
	{
		int i;
		char **stp;
		char **enp;

		reginput = tmpStr;

		stp = startp;
		enp = endp;
		for (i = NSUBEXP; i > 0; i--) {
			*stp++ = 0;
			*enp++ = 0;
		}
		if (regmatch(program)) {
			startp[0] = tmpStr;
			endp[0] = reginput;
			return 1;
		} else {
			return 0;
		}
	}

	// regmatch - main matching routine
	//
	// Conceptually the strategy is simple:  check to see whether the current
	// node matches, call self recursively to see whether the rest matches,
	// and then act accordingly.  In practice we make some effort to avoid
	// recursion, in particular by going through "ordinary" nodes (that don't
	// need to know whether the rest of the match failed) by a loop instead of
	// by recursion.

	int RegExp::regmatch(char *prog)
	{
		char *scan;    // Current node. 
		char *next;            // Next node. 

		for (scan = prog; scan != 0; scan = next) {
			next = regnext(scan);

			switch (OP(scan)) {
			case Op_BOL:
				if (reginput != regbol) {
					return 0;
				}
				break;
			case Op_EOL:
				if (*reginput != ('\0')) {
					return 0;
				}
				break;
			case Op_ANY:
				if (*reginput == ('\0')) {
					return 0;
				}
				reginput++;
				break;
			case Op_EXACTLY:
			{
				size_t len;
				char *const opnd = OPERAND(scan);
				// Inline the first character, for speed. 
				if (*opnd != *reginput) {
					return 0;
				}
				len = strlen(opnd);
				if (len > 1 && strncmp(opnd, reginput, len) != 0) {
					return 0;
				}
				reginput += len;
				break;
			}
			case Op_ANYOF:
				if (*reginput == ('\0') || strchr(OPERAND(scan), *reginput) == 0) {
					return 0;
				}
				reginput++;
				break;
			case Op_ANYBUT:
				if (*reginput == ('\0') || strchr(OPERAND(scan), *reginput) != 0) {
					return 0;
				}
				reginput++;
				break;
			case Op_NOTHING:
				break;
			case Op_BACK:
				break;
			case Op_BRANCH: {
				char *const save = reginput;
				if (OP(next) != Op_BRANCH) {
					// No choice. 
					next = OPERAND(scan);   // Avoid recursion. 
				} else {
					while (OP(scan) == Op_BRANCH) {
						if (regmatch(OPERAND(scan))) {
							return 1;
						}
						reginput = save;
						scan = regnext(scan);
					}
					return 0;
				}
				break;
			}
			case Op_STAR: 
			case Op_PLUS: {
				const char nextch = (OP(next) == Op_EXACTLY) ? *OPERAND(next) : ('\0');
				size_t no;
				char *const save = reginput;
				const size_t min = (OP(scan) == Op_STAR) ? 0 : 1;
				for (no = regrepeat(OPERAND(scan)) + 1; no > min; no--) {
					reginput = save + no - 1;
					// If it could work, try it. 
					if (nextch == ('\0') || *reginput == nextch) {
						if (regmatch(next)) {
							return 1;
						}
					}
				}
				return 0;
				break;
			}
			case Op_END:
				return 1;      // Success! 
				break;
			default:
				if (OP(scan) >= Op_OPEN+1 && OP(scan) < Op_OPEN+NSUBEXP) {
					const int no = OP(scan) - Op_OPEN;
					char *const input = reginput;
					if (regmatch(next)) {
						// Don't set startp if some later
						// invocation of the same parentheses
						// already has.

						if (startp[no] == 0)
						startp[no] = input;
						return 1;
					} else {
						return 0;
					}
				} else if (OP(scan) >= Op_CLOSE+1 && OP(scan) < Op_CLOSE+NSUBEXP) {
					const int no = OP(scan) - Op_CLOSE;
					char *const input = reginput;
					if (regmatch(next)) {
						// Don't set endp if some later
						// invocation of the same parentheses
						// already has.

						if (endp[no] == 0)
						endp[no] = input;
						return 1;
					} else {
						return 0;
					}
				} else {
					Error("regexp corruption\n");
					return 0;
				}
				break;
			}
		}

		// We get here only if there's trouble -- normally "case Op_END" is
		// the terminating point.

		Error("corrupted pointers\n");
		return 0;
	}


	// regrepeat - report how many times something simple would match
	size_t RegExp::regrepeat(char *node)
	{
		 size_t count;
		 char *scan;
		 char ch;

		 switch (OP(node)) 
		 {
		 case Op_ANY:
			 return(strlen(reginput));
			 break;
		 case Op_EXACTLY:
			 ch = *OPERAND(node);
			 count = 0;
			 for (scan = reginput; *scan == ch; scan++)
				 count++;
			 return(count);
			 break;
		 case Op_ANYOF:
			 return(strspn(reginput, OPERAND(node)));
			 break;
		 case Op_ANYBUT:
			 return(strcspn(reginput, OPERAND(node)));
			 break;
		 default:                // Oh dear.  Called inappropriately. 
			 Error("internal error: bad call of regrepeat\n");
			 return 0;      // Best compromise. 
			 break;
		 }
		 // NOTREACHED 
	}

	// regnext - dig the "next" pointer out of a node

	char *RegExp::regnext(char *p)
	{
		const short &offset = *((short*)(p+1));
		if (offset == 0) {
			return 0;
		}
		return((OP(p) == Op_BACK) ? p-offset : p+offset);
	}

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
	void RegExp::GetReplaceString(TsString& result, const char* sReplaceExp)
	{
		char *src = (char *)sReplaceExp;
		char c;
		int no;

		result = "";

		if (sReplaceExp == 0 || sFoundText == 0) {
			return;
		}

		// Now we can create the string
		src = (char *)sReplaceExp;
		while ((c = *src++) != ('\0')) {
			if (c == ('&')) {
				no = 0;
			} else if (c == ('\\') && isdigit(*src)) {
				no = *src++ - ('0');
			} else {
				no = -1;
			}

			if (no < 0) {       
				// Ordinary character. 
				if (c == ('\\') && (*src == ('\\') || *src == ('&'))) {
					// Escape \\ and \&
					c = *src++;
					if (c == 0) {
						break;
					}
				}
				result += c;
			} else if (startp[no] != 0 && endp[no] != 0 && endp[no] > startp[no]) {
				// Get tagged expression
				result.append(
					sFoundText + (startp[no] - startp[0]), 
					endp[no] - startp[no]
				);
			}
		}
	}

}
