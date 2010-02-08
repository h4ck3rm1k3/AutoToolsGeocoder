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
# $Rev: 55 $ 
# $Date: 2006-11-28 21:51:48 +0100 (Tue, 28 Nov 2006) $ 
*/

// GeoUtil.cpp: Geocoder utility methods
#include <stdlib.h>
#include "Geocoder_Headers.h"
#include "GeoUtil.h"
#include "../global/Utility.h"
#include "../global/Exception.h"

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif


#include <algorithm>

namespace PortfolioExplorer {

	// Data files
	const char* GeoUtil::CITY_STATE_POSTCODE_FILE = "CityStatePostcode.dat";
	const char* GeoUtil::CITY_STATE_POSTCODE_FA_INDEX_FILE = "CityStatePostcodeFaIndex.dat";
	const char* GeoUtil::POSTCODE_ALIAS_BY_POSTCODE_FILE = "PostcodeAliasByPostcode.dat";
	const char* GeoUtil::POSTCODE_ALIAS_BY_GROUP_FILE = "PostcodeAliasByGroup.dat";
	const char* GeoUtil::CITY_SOUNDEX_FILE = "CitySoundex.dat";
	const char* GeoUtil::STREET_NAME_FILE = "StreetName.dat";
	const char* GeoUtil::STREET_NAME_POSITION_INDEX_FILE = "StreetNamePositionIndex.dat";
	const char* GeoUtil::STREET_NAME_SOUNDEX_FILE = "StreetNameSoundex.dat";
	const char* GeoUtil::STREET_SEGMENT_FILE = "StreetSegment.dat";
	const char* GeoUtil::STREET_SEGMENT_POSITION_INDEX_FILE = "StreetSegmentPositionIndex.dat";
	const char* GeoUtil::COORDINATE_FILE = "CoordinatePoint.dat";
	const char* GeoUtil::COORDINATE_POSITION_INDEX_FILE = "CoordinatePointPositionIndex.dat";
	const char* GeoUtil::STREET_INTERSECTION_SOUNDEX_FILE = "StreetIntersectionSoundex.dat";
	const char* GeoUtil::STREET_INTERSECTION_SOUNDEX_POSITION_INDEX_FILE = "StreetIntersectionSoundexPostitionIndex.dat";
	const char* GeoUtil::POSTCODE_CENTROID_FILE = "PostcodeCentroid.dat";

	// Huffman frequency tables
	const char* GeoUtil::STREET_NAME_CITY_STATE_POSTCODE_ID_HUFF_FILE = "StreetNameCityStatePostcodeIDHuff.txt";
	const char* GeoUtil::STREET_NAME_PREFIX_HUFF_FILE = "StreetNamePrefixHuff.txt";
	const char* GeoUtil::STREET_NAME_PREDIR_HUFF_FILE = "StreetNamePredirHuff.txt";
	const char* GeoUtil::STREET_NAME_NAME_HUFF_FILE = "StreetNameNameHuff.txt";
	const char* GeoUtil::STREET_NAME_SUFFIX_HUFF_FILE = "StreetNameSuffixHuff.txt";
	const char* GeoUtil::STREET_NAME_POSTDIR_HUFF_FILE = "StreetNamePostdirHuff.txt";
	const char* GeoUtil::STREET_NAME_STREET_SEGMENT_ID_FIRST_HUFF_FILE = "StreetNameStreetSegmentIDFirstHuff.txt";
	const char* GeoUtil::STREET_NAME_STREET_SEGMENT_COUNT_HUFF_FILE = "StreetNameStreetSegmentCountHuff.txt";

	const char* GeoUtil::STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE1 = "StreetSegmentAddrLowKeyHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE2 = "StreetSegmentAddrLowKeyHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE1 = "StreetSegmentAddrLowNonkeyHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE2 = "StreetSegmentAddrLowNonkeyHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_ADDR_HIGH_HUFF_FILE1 = "StreetSegmentAddrHighHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_ADDR_HIGH_HUFF_FILE2 = "StreetSegmentAddrHighHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_COUNTY_KEY_HUFF_FILE = "StreetSegmentCountyKeyHuff.txt";
	const char* GeoUtil::STREET_SEGMENT_COUNTY_NONKEY_HUFF_FILE = "StreetSegmentCountyNonkeyHuff.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE1 = "StreetSegmentCensusTractKeyHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE2 = "StreetSegmentCensusTractKeyHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE1 = "StreetSegmentCensusTractNonkeyHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE2 = "StreetSegmentCensusTractNonkeyHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE1 = "StreetSegmentCensusBlockKeyHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE2 = "StreetSegmentCensusBlockKeyHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE1 = "StreetSegmentCensusBlockNonkeyHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE2 = "StreetSegmentCensusBlockNonkeyHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_POSTCODE_EXT_KEY_HUFF_FILE = "StreetSegmentPostcodeExtKeyHuff.txt";
	const char* GeoUtil::STREET_SEGMENT_POSTCODE_EXT_NONKEY_HUFF_FILE = "StreetSegmentPostcodeExtNonKeyHuff.txt";
	const char* GeoUtil::STREET_SEGMENT_COORDINATE_ID_HUFF_FILE1 = "StreetSegmentCoordinateIDHuff1.txt";
	const char* GeoUtil::STREET_SEGMENT_COORDINATE_ID_HUFF_FILE2 = "StreetSegmentCoordinateIDHuff2.txt";
	const char* GeoUtil::STREET_SEGMENT_COORDINATE_COUNT_HUFF_FILE = "StreetSegmentCoordinateCountHuff.txt";

	const char* GeoUtil::COORDINATE_LATITUDE_HUFF_FILE1 = "CoordinateLatitudeHuff1.txt";
	const char* GeoUtil::COORDINATE_LATITUDE_HUFF_FILE2 = "CoordinateLatitudeHuff2.txt";
	const char* GeoUtil::COORDINATE_LONGITUDE_HUFF_FILE1 = "CoordinateLongitudeHuff1.txt";
	const char* GeoUtil::COORDINATE_LONGITUDE_HUFF_FILE2 = "CoordinateLongitudeHuff2.txt";

	const char* GeoUtil::STREET_INTERSECTION_STATE_HUFF_FILE = "StreetIntersectionStateHuff.txt";
	const char* GeoUtil::STREET_INTERSECTION_SOUNDEX1_HUFF_FILE = "StreetIntersectionSoundex1Huff.txt";
	const char* GeoUtil::STREET_INTERSECTION_STREET_NAME_ID1_HUFF_FILE = "StreetIntersectionStreetNameID1Huff.txt";
	const char* GeoUtil::STREET_INTERSECTION_STREET_SEGMENT_OFFSET1_HUFF_FILE = "StreetIntersectionStreetSegmentOffset1Huff.txt";
	const char* GeoUtil::STREET_INTERSECTION_SOUNDEX2_HUFF_FILE = "StreetIntersectionSoundex2Huff.txt";
	const char* GeoUtil::STREET_INTERSECTION_STREET_NAME_ID2_HUFF_FILE = "StreetIntersectionStreetNameID2Huff.txt";
	const char* GeoUtil::STREET_INTERSECTION_STREET_SEGMENT_OFFSET2_HUFF_FILE = "StreetIntersectionStreetSegmentOffset2Huff.txt";

#if defined(WIN32)
	HINSTANCE GeoUtil::dllModuleInstance = NULL;
#endif // WIND32

	///////////////////////////////////////////////////////////////////////////
	// PackSoundex: Convert a modified soundex to an integer.
	// Inputs:
	//	const unsigned char*	buf		Four-byte buffer containing soundex.
	// Return value:
	//	unsigned int		Soundex value compacted into 16 bits.
	///////////////////////////////////////////////////////////////////////////
	unsigned int GeoUtil::PackSoundex(const unsigned char* buf)
	{
		unsigned int soundexValue;
		if (isdigit(buf[0])) {
			soundexValue = buf[0] - '0';
		} else if (buf[0] == 0) {
			// Short buffer
			assert(0);
			return 0;
		} else {
			assert(toupper(buf[0]) >= 'A' && toupper(buf[0]) <= 'Z');
			soundexValue = toupper(buf[0]) - 'A' + 10;
		}
		// Remaining digits or spaces
		for (int i = 1; i <= 3; i++) {
			if (isdigit(buf[i])) {
				soundexValue = soundexValue * 11 + buf[i] - '0';
			} else if (buf[i] == 0) {
				// Short buffer
				assert(0);
				break;
			} else {
				assert(buf[i] == ' ');
				soundexValue = soundexValue * 11 + 10;
			}
		}
		#ifndef NDEBUG
			char tmp[5];
			UnpackSoundex(tmp, soundexValue);
			assert(memcmp(tmp, buf, 4) == 0);
		#endif
		return soundexValue;
	}

	///////////////////////////////////////////////////////////////////////////
	// UnpackSoundex: Convert a compacted modified soundex back to a buffer.
	// Inputs:
	//	unsigned int	value		Compacted soundex value.
	// Outputs:
	//	unsigned char*	bufReturn	Five-byte buffer to receive (terminated) soundex.
	///////////////////////////////////////////////////////////////////////////
	void GeoUtil::UnpackSoundex(
		unsigned char* bufReturn,
		unsigned int value
	) {
		// terminate
		bufReturn[4] = 0;	
		// Trailing digits or spaces
		for (int i = 3; i >= 1; i--) {
			int c = value % 11;
			value /= 11;
			if (c == 10) {
				bufReturn[i] = ' ';
			} else {
				bufReturn[i] = (unsigned char)(c + '0');
			}
		}
		if (value < 10) {
			bufReturn[0] = (unsigned char)(value + '0');
		} else {
			bufReturn[0] = (unsigned char)(value - 10 + 'A');
		}

	}

	///////////////////////////////////////////////////////////////////////////
	// PackFa: Convert a finance area to a 24-bit integer.
	// Inputs:
	//	const unsigned char*	buf		Four-byte buffer containing Fa.
	// Return value:
	//	unsigned int	Fa value compacted into 24 bits.
	///////////////////////////////////////////////////////////////////////////
	unsigned int GeoUtil::PackFa(const unsigned char* buf)
	{
		if (isdigit(buf[0])) {
			// US numeric Fa
			return atoi((const char*)buf) | 0x800000;
		} else {
			// CA letter/digit/letter
			return ((int)buf[0] << 14) | ((int)buf[1] << 7) | (int)buf[2];
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// UnpackFa: Convert a compacted finance are back to a buffer.
	// Inputs:
	//	unsigned int	value		Compacted Fa value.
	// Outputs:
	//	unsigned char*	bufReturn	seven-byte buffer containing 
	//								null-terminated finance number
	///////////////////////////////////////////////////////////////////////////
	void GeoUtil::UnpackFa(
		unsigned char* bufReturn,
		unsigned int value
	) {
		if ((value & 0x800000) != 0) {
			// US numeric
			value &= ~0x800000;
			unsigned char* ptr = bufReturn + 5; 
			for (;;) {
				*ptr = (unsigned char)('0' + (value % 10));
				value /= 10;
				if (ptr == bufReturn) {
					break;
				}
				ptr--;
			}
			bufReturn[6] = 0;
		} else {
			// CA letter/digit/letter
			bufReturn[0] = (unsigned char)((value >> 14) & 0x7f); 
			bufReturn[1] = (unsigned char)((value >> 7) & 0x7f); 
			bufReturn[2] = (unsigned char)(value & 0x7f);
			bufReturn[3] = 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Compare two string values.
	//	const char*	s1			First value
	//	const char*	s2			Second value
	// Return value:
	//	int		The number of errors encountered in the comparison.
	///////////////////////////////////////////////////////////////////////////////
	int GeoUtil::EditDistance(
		const char* s1,
		const char* s2
	) {
		int length1 = int(strlen(s1));
		int length2 = int(strlen(s2));

		int minLength, maxLength;
		if (length1 > length2) {
			minLength = length2;
			maxLength = length1;
		} else {
			minLength = length1;
			maxLength = length2;
		}

		int lastPosition = int(minLength - 1);
		int errorCount = int(maxLength - minLength);

		// We give deletion two chances.  Otherwise is is too expensive.
		int deletionChances = (abs(length1 - length2) == 1) ? 2 : 0;

		// Some notes on the algorithm:
		// If we can transpose, that is best.  In order to do this:
		//   -- It must not be the last character.
		//   -- Transposing must fix errors in both this and the next position.
		// If we cannot transpose, then try s1 deletion.  This works if 
		// s1[j+1] == s2[j] for all remaining j.  An extra error is
		// added if the last s2 char is non-blank.
		// If s1 deletion fails, then try s2 deletion.
		// We only try deletion once.
		int i;
		for (i = 0; i < lastPosition; i++) {
			// Equal?
			char c1 = char(toupper(s1[i]));
			char c2 = char(toupper(s2[i]));

			if (c1 == c2) {
				// Current char is identical.
				continue;
			}

			// Fetch the following characters
			char c12 = char(toupper(s1[i+1]));
			char c22 = char(toupper(s2[i+1]));


			// Check for transposition.  This is best because it fixes two errors
			// with one error count.
			if (c12 == c2 && c1 == c22) {
				// Transposition works.  Record one error and skip an extra character.
				errorCount++;
				i++;
				continue;
			}

			if (c12 == c22) {
				// Don't try deletion when next chars are equal; substitution works.
				errorCount++;
				// Skip next char since they are equal
				i++;
				continue;
			}

			if (deletionChances > 0) {
				if (length1 > length2) {
					// Check for s1 delete
					assert(length1 == length2 + 1);
					if (MEMICMP(s1 + i + 1, s2 + i, length2 - i) == 0) {
						//Successful deletion
						// the error was already acounted for in the length diff
						//errorCount++;
						// We are done.  Signal to code past loop end not the check last char.
						i = minLength;
						break;
					}
				} else {
					// Check for s2 delete
					assert(length2 == length1 + 1);
					if (MEMICMP(s1 + i, s2 + i + 1, length1 - i) == 0) {
						// Successful deletion
						// the error was already acounted for in the length diff
						//errorCount++;
						// We are done.  Signal to code past loop end not the check last char.
						i = minLength;
						break;
					}
				}
				// Reduce the number of chances left for deletion.
				deletionChances--;
			}

			// Failing that, we do substitution
			errorCount++;
		}

		// If the final character has not been considered, test for substitution now
		if (i == lastPosition && s1[i] != s2[i]) {
			errorCount++;
		}

		return errorCount;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Run-length compress zeros of the vector, assuming that the
	// input values are in the range 0-255.  Code run-lengths as values of the
	// form 0x1nnn, where nnn is the count.
	// Inputs:
	//	std:;string&		str		The input string.  Chars will be interpreted
	//								as unsigned.
	//	int					offset	Amount to subtract from each string char.
	// Outputs:
	//	std::vector<int>&	vec		The output vector.
	///////////////////////////////////////////////////////////////////////////////
	void GeoUtil::RLECompressVec(
		std::vector<int>& vec
	) {
		int size = 0;
		for (unsigned i = 0; i < vec.size();) {
			if (vec[i] == 0) {
				unsigned start = i;
				unsigned limit = JHMIN(unsigned(start + 0xFFF), unsigned(vec.size()));
				do {
					i++;
				} while (i < limit && vec[i] == 0);
				// RLE compress sequences of zeros are repesented by an int
				// of the form 0x10nn, where nn is the count.
				vec[size++] = (i - start + 0x1000);
			} else {
				vec[size++] = vec[i++];
			}
		}
		vec.resize(size);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Run-length de-compress zeros of the vector, performing the opposite
	// transform of RLECompressVec.
	// Inputs:
	//	std::vector<int>&	vec			The input vector to compress.
	// Outputs:
	//	std::vector<int>&	vecResult	The output vector.
	///////////////////////////////////////////////////////////////////////////////
	void GeoUtil::RLEDecompressVec(
		std::vector<int>& vecResult,
		const std::vector<int>& vec
	) {
		vecResult.clear();
		for (unsigned i = 0; i < vec.size(); i++) {
			if (vec[i] >= 0x1000) {
				// Run-length zeros
				int count = vec[i] - 0x1000;
				for (int j = 0; j < count; j++) {
					vecResult.push_back(0);
				}
			} else {
				vecResult.push_back(vec[i]);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Given an integer value, fill a buffer with the required number of
	// bytes to represent it as a variable-length sequence.
	// Inputs:
	//	int				value	The value.
	// Outputs:
	//	unsigned char*	buf		The buffer to fill.  Must be at least six bytes long.
	// Return value:
	//	int			The number of bytes filled.
	///////////////////////////////////////////////////////////////////////////
	int GeoUtil::IntToVarLengthBuf(
		int value,
		unsigned char* buf
	) {
		// Construct an unsigned integer by moving the sign bit down to bit 0.
		if (value < 0) {
			return IntToVarLengthBuf(
				(unsigned int)( ((-value) << 1) | 1),
				buf
			);
		} else {
			return IntToVarLengthBuf(
				(unsigned int)(value << 1),
				buf
			);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Given an integer value, fill a buffer with the required number of
	// bytes to represent it as a variable-length sequence.
	// Inputs:
	//	unsigned int	value	The value.
	// Outputs:
	//	unsigned char*	buf		The buffer to fill.  Must be at least five bytes long.
	// Return value:
	//	int			The number of bytes filled.
	///////////////////////////////////////////////////////////////////////////
	int GeoUtil::IntToVarLengthBuf(
		unsigned int value,
		unsigned char* buf
	) {
		// Count bytes needed
		int nbrBytes = 0;
		unsigned int tmp = value;
		do {
			nbrBytes++;
			tmp >>= 7;
		} while (tmp != 0);

		// Do last byte -- it has no extension bit set
		buf[nbrBytes - 1] = (unsigned char)(value & 0x7F);
		value >>= 7;
		// Other bytes have extension bit set.
		for (int i = nbrBytes - 2; i >= 0; i--) {
			buf[i] = (unsigned char)(value | 0x80);
			value >>= 7;
		}
		return nbrBytes;
	}

	void GeoUtil::AddArrayToFreqTable(
		FreqTable<int>& freqTable,
		const unsigned char* array,
		int arraySize
	) {
		for (int i = 0; i < arraySize; i++) {
			freqTable.Count(array[i]);
		}
	}


	void GeoUtil::CodeVarLengthIntToBitStream(
		int value,
		HuffmanCoder<int, std::less<int> >& coder,
		BitStreamWrite& bitStream
	) {
		unsigned char tmpBuf[8];
		int varIntLength = IntToVarLengthBuf(value, tmpBuf);
		for (int i = 0; i < varIntLength; i++) {
			if (!coder.WriteCode(tmpBuf[i], bitStream)) {
				throw TsString("Error in Huffman code table");
			}
		}
	}

	void GeoUtil::CodeVarLengthIntToBitStream(
		unsigned int value,
		HuffmanCoder<int, std::less<int> >& coder,
		BitStreamWrite& bitStream
	) {
		unsigned char tmpBuf[8];
		int varIntLength = IntToVarLengthBuf(value, tmpBuf);
		for (int i = 0; i < varIntLength; i++) {
			if (!coder.WriteCode(tmpBuf[i], bitStream)) {
				throw TsString("Error in Huffman code table");
			}
		}
	}


}
