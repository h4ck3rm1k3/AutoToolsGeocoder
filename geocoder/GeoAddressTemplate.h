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

#ifndef INCL_ADDRESS_TEMPLATE_H
#define INCL_ADDRESS_TEMPLATE_H

#include "../geocommon/Geocoder_DllExport.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include "../global/TsString.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////
	// Class that deduces the "template" of an address, containing
	// up to two numeric parts and two alpha parts.
	///////////////////////////////////////////////////////////////////////
	class AddressRangeTemplate;

	class AddressTemplate {
		friend class AddressRangeTemplate;
	public:
		AddressTemplate();
		AddressTemplate(const char* addr);

		enum { 
			MaxSegments = 4,
			MaxAddrSize = 10
		};

		enum CharClass { CharClassLetter, CharClassDigit, CharClassOther };

		int GetNumberOfSegments() const { return nbrSegments; }
		struct Segment {
			CharClass charClass;
			char value[MaxAddrSize+1];	// the segment value
			double GetValue() const;
			bool IsEven() const {
				return 
					charClass == CharClassDigit &&
					(atoi(value) & 1) == 0;
			}
		};
		const Segment& GetSegment(int idx) const { 
			assert(idx >= 0 && idx < nbrSegments);
			return segments[idx];
		}
		Segment& GetSegment(int idx) { 
			assert(idx >= 0 && idx < nbrSegments);
			return segments[idx];
		}

		static inline CharClass ClassOf(unsigned char c) {
			return AddressRangeTemplateCharClass::ClassOf(c);
		}
		static inline CharClass ClassOf(char c) {
			return AddressRangeTemplateCharClass::ClassOf((unsigned char)c);
		}

		// Check compatibility of two address templates
		bool IsCompatibleWith(
			const AddressTemplate& rhs,
			bool matchEvenOdd = false
		) const;

		// Format to a string.
		void Format(TsString& outStr) const;

	private:
		int nbrSegments;
		Segment segments[MaxSegments];

		// Must have at least one static var of this type to cause initialization.
		// This is declared in AddressTemplate.cpp
		class AddressRangeTemplateCharClass {
		public:
			AddressRangeTemplateCharClass() {
				// Initialize the static data
				for (int i = 0; i < 256; i++) {
					charClass[i] = PrivateClassOf((unsigned char)i);
				}
			}
			static inline CharClass ClassOf(unsigned char c) {
				assert(charClass[c] == PrivateClassOf(c));
				return charClass[c];
			}
		private:
			static CharClass PrivateClassOf(unsigned char c) {
				if (isdigit(c) != 0) {
					return CharClassDigit;
				} else if (isalpha(c) != 0) {
					return CharClassLetter;
				} else {
					return CharClassOther;
				}
			}
			static CharClass charClass[256];
		};

		// Need this static variable to cause initialization of static charClass member.
		static AddressRangeTemplateCharClass addressRangeTemplateCharClass;
	};

	///////////////////////////////////////////////////////////////////////
	// Class that deduces the "template" of an address range, containing
	// up to two fixed parts and two variable parts.
	///////////////////////////////////////////////////////////////////////
	class AddressRangeTemplate {
	public:
		AddressRangeTemplate();
		AddressRangeTemplate(const char* addrLow, const char* addrHigh);
		bool IsValid() const { return isValid; }
		int GetNumberOfSegments() const { return addrTemplateLow.nbrSegments; }
		// Where is the address within the range?
		bool Interpolate(
			const AddressTemplate& addrTemplate,
			double& result,
			bool& inRangeReturn
		) const;
		// Score the adress range within the template
		double Score(
			const AddressTemplate& addrTemplate,
			bool& inRangeReturn,
			bool& evenOddMatchReturn
		) const;

		// Get the relative "Size" of the address range.
		double GetRangeSize() const;

		// Is a segment of this range even/odd?
		bool IsEvenOdd(int idx, bool& isEven) const;

		// Check compatibility of two address range templates
		bool IsCompatibleWith(
			const AddressRangeTemplate& rhs,
			bool matchEvenOdd = false
		) const;

		// Consolidate with another range template.  Should check compatibility first.
		void ConsolidateWith(
			const AddressRangeTemplate& rhs
		);

		const AddressTemplate& GetLow() const { return addrTemplateLow; }
		const AddressTemplate& GetHigh() const { return addrTemplateHigh; }

	private:
		bool isValid;
		bool isFixed[AddressTemplate::MaxSegments];
		AddressTemplate addrTemplateLow;
		AddressTemplate addrTemplateHigh;

		struct CompareRange { 
			double value; 
			const char* str; 
			bool operator<(const CompareRange& rhs) const {
				return value < rhs.value;
			}
		};
	};

}

#endif
