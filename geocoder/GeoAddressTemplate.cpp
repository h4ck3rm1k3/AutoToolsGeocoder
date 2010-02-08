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

#include "../geocommon/Geocoder_Headers.h"
#include "GeoAddressTemplate.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace PortfolioExplorer {

	// Need this static variable to cause initialization of static charClass member.
	AddressTemplate::AddressRangeTemplateCharClass AddressTemplate::addressRangeTemplateCharClass;

	// Static char class array
	AddressTemplate::CharClass AddressTemplate::AddressRangeTemplateCharClass::charClass[256];

	///////////////////////////////////////////////////////////////////////
	// Convert an alphabetic string to a float
	///////////////////////////////////////////////////////////////////////
	static inline double StringToFloat(const char* ptr) {
		double value = 0.0;
		if (isalpha(*ptr)) {
			// Alpha case, relative to 'A'
			while (*ptr != 0) {
				value = value * 26.0 + *ptr++ - 'A';
			}
		} else if (isdigit(*ptr)) {
			return atoi(ptr);
		} else {
			// Usually "other" is a single separate character like '-'
			while (*ptr != 0) {
				value = value * 128.0 + *ptr++;
			}
		}
		return value;
	}

	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	// Class that deduces the "template" of an address, containing
	// up to two numeric parts and two alpha parts.
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	AddressTemplate::AddressTemplate() :
		nbrSegments(0)
	{
	}

	AddressTemplate::AddressTemplate(
		const char* addr
	) :
		nbrSegments(0)
	{
		// Ignore trailing fractions; they are not really part of the address segment.
		const char* endPtr = 0;
		if (strchr(addr, '/') != 0) {
			endPtr = strchr(addr, ' ');
		}
		if (endPtr == 0) {
			endPtr = addr + strlen(addr);
		}

		const char* ptr = addr;
		while (nbrSegments < MaxSegments && ptr < endPtr) {
			Segment& segment = segments[nbrSegments++];
			char* valuePtr = segment.value;
			segment.charClass = ClassOf(*ptr);
			while (
				ptr < endPtr && 
				valuePtr < segment.value + MaxAddrSize &&
				segment.charClass == ClassOf(*ptr)
			) {
				*valuePtr++ = *ptr++;
			}
			*valuePtr = 0;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Check compatibility of two address templates
	///////////////////////////////////////////////////////////////////////
	bool AddressTemplate::IsCompatibleWith(
		const AddressTemplate& rhs,
		bool matchEvenOdd
	) const {
		if (nbrSegments != rhs.nbrSegments) {
			return false;
		}
		for (int i = 0; i < nbrSegments; i++) {
			if (segments[i].charClass != rhs.segments[i].charClass) {
				return false;
			}
			if (
				matchEvenOdd &&
				segments[i].IsEven() != rhs.segments[i].IsEven()
			) {
				return false;
			}
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	// Format to a string.
	// Outputs:
	//	TsString&	outStr		Buffer to receive formatted address segment.
	///////////////////////////////////////////////////////////////////////
	void AddressTemplate::Format(TsString& outStr) const
	{
		outStr = "";
		for (int i = 0; i < nbrSegments; i++) {
			for (const char* ptr = segments[i].value; *ptr != 0; ptr++) {
				outStr += *ptr;
			}
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Return the value of the address segment as a double, where conversion
	// is performed from either numeric or text representation to produce
	// a result that is "proportional" to the value.
	///////////////////////////////////////////////////////////////////////
	double AddressTemplate::Segment::GetValue() const
	{
		return StringToFloat(value);
	}


	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	// Class that deduces the "template" of an address range, containing
	// up to two fixed parts and two variable parts.
	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	// Constructor
	///////////////////////////////////////////////////////////////////////
	AddressRangeTemplate::AddressRangeTemplate() :
		isValid(false)
	{
	}

	AddressRangeTemplate::AddressRangeTemplate(
		const char* addrLow, 
		const char* addrHigh
	) :
		isValid(false),
		addrTemplateLow(addrLow),
		addrTemplateHigh(addrHigh)
	{
		if (addrTemplateLow.nbrSegments == 0) {
			return;
		}
		if (addrTemplateLow.nbrSegments != addrTemplateHigh.nbrSegments) {
			// The two segments should be very similar.  The only exception
			// allowed is that one high address may have an extra segment that is
			// missing from the other address.
			if (addrTemplateLow.GetNumberOfSegments() + 1 == addrTemplateHigh.GetNumberOfSegments()) {
				// Add an extra blank segment to the low address
				addrTemplateLow.segments[addrTemplateLow.nbrSegments].charClass = 
					addrTemplateHigh.segments[addrTemplateLow.nbrSegments].charClass;
				addrTemplateLow.segments[addrTemplateLow.nbrSegments].value[0] = 0;
				addrTemplateLow.nbrSegments++;
			} else if (addrTemplateLow.GetNumberOfSegments() == addrTemplateHigh.GetNumberOfSegments() + 1) {
				// Add an extra blank segment to the high address
				addrTemplateHigh.segments[addrTemplateHigh.nbrSegments].charClass = 
					addrTemplateLow.segments[addrTemplateHigh.nbrSegments].charClass;
				addrTemplateHigh.segments[addrTemplateHigh.nbrSegments].value[0] = 0;
				addrTemplateHigh.nbrSegments++;
			} else {
				return;
			}
		}
		for (int i = 0; i < GetNumberOfSegments(); i++) {
			if (addrTemplateLow.segments[i].charClass != addrTemplateHigh.segments[i].charClass) {
				// Segment class is different
				return;
			}
			isFixed[i] = strcmp(addrTemplateLow.segments[i].value, addrTemplateHigh.segments[i].value) == 0;
		}

		if (GetNumberOfSegments() == 1) {
			// Special case: do not count a segment as fixed if there is only one segment,
			// otherwise interpolation will fail for out-of-range values.
			isFixed[0] = false;
		}

		// Note it is acceptable to have multiple non-fixed segments.
		isValid = true;
	}


	///////////////////////////////////////////////////////////////////////
	// Interpolate the address template within the address range template.
	// Inputs:
	//	const AddressTemplate&	
	//					addrTemplate	The address template
	// Outputs:
	//	double&			result			0.0:		Address matches addrLow
	//									1.0:		Address matches addrHigh
	//									0.0 - 1.0:	Address between addrLow and addrHigh
	//									< 0.0:		Address off addrLow end of range
	//									> 1.0:		Address off addrHigh end of range
	//	bool&			inRangeReturn	true if address is in range, false if outside of range
	// Return value:
	//	bool			true if successfully interpolated, false o/w.
	//
	// Notes: Remember that addrLow can be "higher" than addrHigh.
	///////////////////////////////////////////////////////////////////////
	bool AddressRangeTemplate::Interpolate(
		const AddressTemplate& addrTemplate,
		double& result,
		bool& inRangeReturn
	) const {
		// Default return values
		inRangeReturn = true;
		// Default result in case all segments are fixed
		result = 0.5;

		if (!isValid || addrTemplate.GetNumberOfSegments() == 0) {
			// Empty address
			return false;
		}


		double totalValue = 0.0;
		double totalRange = 0.0;
		double prevRange = 0.0;

		for (int i = 0; i < GetNumberOfSegments(); i++) {
			if (i >= addrTemplate.GetNumberOfSegments()) {
				// More segments in the address range than the address
				if (
					i > addrTemplate.GetNumberOfSegments() ||
					addrTemplateLow.GetSegment(i).charClass != AddressTemplate::CharClassDigit
				) {
					// Reject address with more than one missing segment,
					// or with missing segment that is numeric.
					return false;
				} else {
					// Follow through with comparison, skipping this segment.
					continue;
				}
			}

			double low = addrTemplateLow.GetSegment(i).GetValue();
			double high = addrTemplateHigh.GetSegment(i).GetValue();
			double value = addrTemplate.GetSegment(i).GetValue();

			if (addrTemplateLow.GetSegment(i).charClass != addrTemplate.GetSegment(i).charClass) {
				// Segment class mismatch
				return false;
			}

			if (isFixed[i]) {
				if (value != low) {
					// Fixed segments must be matched exactly.
					return false;
				}
				// Fixed entries don't factor into the range calculations.
				continue;
			}

			if (low > high) {
				// Inverted address range
				totalValue = totalValue * prevRange + (low - value);
				totalRange = totalRange * prevRange + (low - high);
				prevRange = low - high;
				if (value > low || value < high) {
					inRangeReturn = false;
				}
			} else {
				// Normal address range
				totalValue = totalValue * prevRange + (value - low);
				totalRange = totalRange * prevRange + (high - low);
				prevRange = high - low;
				if (value < low || value > high) {
					inRangeReturn = false;
				}
			}
		}

		// Interpolate halfway in segment when low==high
		result = (totalRange == 0.0) ? 0.5 : (totalValue / totalRange);

		return true;
	}

	///////////////////////////////////////////////////////////////////////
	// Score the address template against address range template.
	// Inputs:
	//	const AddressTemplate&	addrTemplate	The address template
	// Outputs:
	//	bool					inRangeReturn	true if in address range, false if
	//											interpolated from outside of range
	//	bool					evenOddMatchReturn
	//											true if the even/odd nature of the range
	//											matches the even/odd of the address.
	// Return value:
	//	int				a score between 0.0 and 1.0
	//
	// Notes: Remember that addrLow can be "higher" than addrHigh.
	///////////////////////////////////////////////////////////////////////
	double AddressRangeTemplate::Score(
		const AddressTemplate& addrTemplate,
		bool& inRangeReturn,
		bool& evenOddMatchReturn
	) const {
		// Default result is no match
		double score = 0.0;

		// Default return values
		inRangeReturn = true;
		evenOddMatchReturn = true;

		if (!isValid) {
			// Empty address
			return 0.0;
		}

		int minSegments, maxSegments;
		if (GetNumberOfSegments() > addrTemplate.GetNumberOfSegments()) {
			minSegments = addrTemplate.GetNumberOfSegments();
			maxSegments = GetNumberOfSegments();

			// Discount penalty for a single missing non-number segment
			if (
				GetNumberOfSegments() - 1 == addrTemplate.GetNumberOfSegments() &&
				addrTemplateLow.GetSegment(addrTemplate.GetNumberOfSegments()).charClass != AddressTemplate::CharClassDigit
			) {
				score += 0.7;
			}
		} else {
			minSegments = GetNumberOfSegments();
			maxSegments = addrTemplate.GetNumberOfSegments();

			// Discount penalty for a single missing non-number segment
			if (
				GetNumberOfSegments() == addrTemplate.GetNumberOfSegments() - 1 &&
				addrTemplate.GetSegment(GetNumberOfSegments()).charClass != AddressTemplate::CharClassDigit
			) {
				score += 0.7;
			}
		}

		if (minSegments <= 0) {
			return 0.0;
		}

		for (int i = 0; i < minSegments; i++) {
			if (addrTemplateLow.GetSegment(i).charClass != addrTemplate.GetSegment(i).charClass) {
				// Segment classes must match
				continue;
			}
			double low, high, value;
			if (addrTemplateLow.GetSegment(i).charClass == AddressTemplate::CharClassDigit) {
				int intValue = atoi(addrTemplate.GetSegment(i).value);
				int intLow = atoi(addrTemplateLow.GetSegment(i).value);
				int intHigh = atoi(addrTemplateHigh.GetSegment(i).value);
				if ((intLow & 1) == (intHigh & 1)) {
					// Segment is even/odd
					if ((intLow & 1) != (intValue & 1)) {
						// 100-point Penalty for even/odd mismatch.  Since address-number score counts
						// for only 30% of street score, penalty has to be steep to exceed default
						// 25-point multiple threshold.  This 100 points translates to 30 points in final.
						score -= 0.10;
						evenOddMatchReturn = false;
					}
				} else {
					// Segment is not even/odd.  Prefer even/odd segments through a very slight penatly.
					score -= 0.01;
				}
				value = intValue;
				low = intLow;
				high = intHigh;
			} else {
				value = addrTemplate.GetSegment(i).GetValue();
				low = addrTemplateLow.GetSegment(i).GetValue();
				high = addrTemplateHigh.GetSegment(i).GetValue();
			}
			if (low > high) {
				double tmp = low;
				low = high;
				high = tmp;
			}
			if (low <= value && value <= high) {
				score += 1.0;
			} else {
				double denom = high - low;
				// ... but don't let it be too small.
				if (denom < 50) {
					denom = 50;
				}
				double error = (value < low) ? low - value : value - high;
				inRangeReturn = false;
				double segmentScore = 0.9 - error / denom;
				// Minimum 10% hit if even slightly outside range.  This prevents multiple-matches
				// with default multiple threshold of 25.
				if (segmentScore > 0.9) {
					segmentScore = 0.9;
				} else if (segmentScore < 0.0) {
					segmentScore = 0.0;
				}
				score += segmentScore;
				// e.g. address number of 220 for range 100-200 will score 700
			}
		}

		if (score < 0.0) {
			score = 0.0;
		}
		score /= maxSegments;

		if (score < 0.5) {
			// Check for special case of hyphenated vs non-hyphenated comparison
			// This is a very small penalty
			if (
				GetNumberOfSegments() == 1 && 
				addrTemplate.GetNumberOfSegments() == 3 &&
				addrTemplate.GetSegment(1).charClass == AddressTemplate::CharClassOther
			) {
				// Hyphenated address, non-hyphenated template
				char tempBuf[AddressTemplate::MaxAddrSize * 2 + 1];
				strcpy(tempBuf, addrTemplate.GetSegment(0).value);
				strcat(tempBuf, addrTemplate.GetSegment(2).value);
				AddressTemplate tmpAddrTemplate(tempBuf);
				score = Score(tmpAddrTemplate, inRangeReturn, evenOddMatchReturn) * .97;
			} else if (
				addrTemplate.GetNumberOfSegments() == 1 && 
				GetNumberOfSegments() == 3 &&
				addrTemplateLow.GetSegment(1).charClass == AddressTemplate::CharClassOther
			) {
				// Non-hyphenated address, hyphenated template
				char tempBufLow[AddressTemplate::MaxAddrSize * 2 + 1];
				char tempBufHigh[AddressTemplate::MaxAddrSize * 2 + 1];
				strcpy(tempBufLow, addrTemplateLow.GetSegment(0).value);
				strcat(tempBufLow, addrTemplateLow.GetSegment(2).value);
				strcpy(tempBufHigh, addrTemplateHigh.GetSegment(0).value);
				strcat(tempBufHigh, addrTemplateHigh.GetSegment(2).value);
				AddressRangeTemplate tmpAddrRangeTemplate(tempBufLow, tempBufHigh);
				score = tmpAddrRangeTemplate.Score(addrTemplate, inRangeReturn, evenOddMatchReturn) * .97;
			}
		}

		return score;
	}

	///////////////////////////////////////////////////////////////////////
	// Check compatibility of two address range templates
	///////////////////////////////////////////////////////////////////////
	bool AddressRangeTemplate::IsCompatibleWith(
		const AddressRangeTemplate& rhs,
		bool matchEvenOdd
	) const {
		if (!isValid || !rhs.isValid) {
			return false;
		}
		if (GetNumberOfSegments() != rhs.GetNumberOfSegments()) {
			return false;
		}

		// First check compatibility independent of even/odd considerations
		if (!addrTemplateLow.IsCompatibleWith(rhs.addrTemplateLow)) {
			return false;
		}

		if (matchEvenOdd) {
			// Also check even/odd attributes
			for (int i = 0; i < GetNumberOfSegments(); i++) {
				bool leftEven, rightEven;
				if (
					IsEvenOdd(i, leftEven) != rhs.IsEvenOdd(i, rightEven) ||
					leftEven != rightEven
				) {
					return false;
				}
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////
	// Consolidate with another range template.  Should check compatibility first.
	///////////////////////////////////////////////////////////////////////
	void AddressRangeTemplate::ConsolidateWith(
		const AddressRangeTemplate& rhs
	) {
		for (int i = 0; i < GetNumberOfSegments() && i < rhs.GetNumberOfSegments(); i++) {
			if (addrTemplateLow.GetSegment(i).charClass != rhs.addrTemplateLow.GetSegment(i).charClass) {
				assert(0);
				continue;
			}

			struct CompareRange order[4];

			order[0].value = addrTemplateLow.GetSegment(i).GetValue();
			order[0].str = addrTemplateLow.GetSegment(i).value;
			order[1].value = addrTemplateHigh.GetSegment(i).GetValue();
			order[1].str = addrTemplateHigh.GetSegment(i).value;
			order[2].value = rhs.addrTemplateLow.GetSegment(i).GetValue();
			order[2].str = rhs.addrTemplateLow.GetSegment(i).value;
			order[3].value = rhs.addrTemplateHigh.GetSegment(i).GetValue();
			order[3].str = rhs.addrTemplateHigh.GetSegment(i).value;

			std::sort(order, order + 4);

			if (addrTemplateLow.GetSegment(i).GetValue() < addrTemplateHigh.GetSegment(i).GetValue()) {
				// Normal low/high order
				strcpy(addrTemplateLow.GetSegment(i).value, order[0].str);
				strcpy(addrTemplateHigh.GetSegment(i).value, order[3].str);
			} else {
				// Preserve inverted low/high order
				strcpy(addrTemplateLow.GetSegment(i).value, order[3].str);
				strcpy(addrTemplateHigh.GetSegment(i).value, order[0].str);
			}
		}
	}



	///////////////////////////////////////////////////////////////////////
	// Is this range even/odd?
	// Return value:
	//	bool		if true, then range segment has even/odd property.
	///////////////////////////////////////////////////////////////////////
	bool AddressRangeTemplate::IsEvenOdd(
		int idx,
		bool& isEven
	) const {
		if (!isValid) {
			return false;
		}

		if (
			addrTemplateLow.GetSegment(idx).charClass == AddressTemplate::CharClassDigit &&
			addrTemplateLow.GetSegment(idx).IsEven() == addrTemplateHigh.GetSegment(idx).IsEven()
		) {
			isEven = addrTemplateLow.GetSegment(idx).IsEven();
			return true;
		} else {
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Get the relative "Size" of the address range.
	// Return value:
	//	double		The "size" of the address range.  This is an arbitrary
	//				number used to compare address ranges to each other.
	///////////////////////////////////////////////////////////////////////
	double AddressRangeTemplate::GetRangeSize() const
	{
		if (!isValid) {
			// Empty address
			return 1.0;
		}

		double totalRange = 1.0;
		double prevRange = 0.0;

		for (int i = 0; i < GetNumberOfSegments(); i++) {
			double low = addrTemplateLow.GetSegment(i).GetValue();
			double high = addrTemplateHigh.GetSegment(i).GetValue();

			if (low > high) {
				// Inverted address range
				totalRange = totalRange * prevRange + (low - high);
				prevRange = low - high;
			} else {
				// Normal address range
				totalRange = totalRange * prevRange + (high - low);
				prevRange = high - low;
			}
		}

		return totalRange;
	}

}
