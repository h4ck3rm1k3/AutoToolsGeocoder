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

// GeoUtil.h: Geocoder utility methods

#ifndef INCL_GEOUTIL_H
#define INCL_GEOUTIL_H

#include "Geocoder_DllExport.h"
#include "GeoHuffman.h"
#include "GeoFreqTable.h"

#include <vector>
#include <assert.h>
#include <string.h>

namespace PortfolioExplorer {

	class GeoUtil {
	public:

		// Data files
		static const char* CITY_STATE_POSTCODE_FILE;
		static const char* CITY_STATE_POSTCODE_FA_INDEX_FILE;
		static const char* POSTCODE_ALIAS_BY_POSTCODE_FILE;
		static const char* POSTCODE_ALIAS_BY_GROUP_FILE;
		static const char* CITY_SOUNDEX_FILE;
		static const char* STREET_NAME_FILE;
		static const char* STREET_NAME_POSITION_INDEX_FILE;
		static const char* STREET_NAME_SOUNDEX_FILE;
		static const char* STREET_SEGMENT_FILE;
		static const char* STREET_SEGMENT_POSITION_INDEX_FILE;
		static const char* COORDINATE_FILE;
		static const char* COORDINATE_POSITION_INDEX_FILE;
		static const char* STREET_INTERSECTION_SOUNDEX_FILE;
		static const char* STREET_INTERSECTION_SOUNDEX_POSITION_INDEX_FILE;
		static const char* POSTCODE_CENTROID_FILE;

		// Huffman frequency tables
		static const char* STREET_NAME_CITY_STATE_POSTCODE_ID_HUFF_FILE;
		static const char* STREET_NAME_PREFIX_HUFF_FILE;
		static const char* STREET_NAME_PREDIR_HUFF_FILE;
		static const char* STREET_NAME_NAME_HUFF_FILE;
		static const char* STREET_NAME_SUFFIX_HUFF_FILE;
		static const char* STREET_NAME_POSTDIR_HUFF_FILE;
		static const char* STREET_NAME_STREET_SEGMENT_ID_FIRST_HUFF_FILE;
		static const char* STREET_NAME_STREET_SEGMENT_COUNT_HUFF_FILE;

		static const char* STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE1;
		static const char* STREET_SEGMENT_ADDR_LOW_KEY_HUFF_FILE2;
		static const char* STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE1;
		static const char* STREET_SEGMENT_ADDR_LOW_NONKEY_HUFF_FILE2;
		static const char* STREET_SEGMENT_ADDR_HIGH_HUFF_FILE1;
		static const char* STREET_SEGMENT_ADDR_HIGH_HUFF_FILE2;
		static const char* STREET_SEGMENT_COUNTY_KEY_HUFF_FILE;
		static const char* STREET_SEGMENT_COUNTY_NONKEY_HUFF_FILE;
		static const char* STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE1;
		static const char* STREET_SEGMENT_CENSUS_TRACT_KEY_HUFF_FILE2;
		static const char* STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE1;
		static const char* STREET_SEGMENT_CENSUS_TRACT_NONKEY_HUFF_FILE2;
		static const char* STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE1;
		static const char* STREET_SEGMENT_CENSUS_BLOCK_KEY_HUFF_FILE2;
		static const char* STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE1;
		static const char* STREET_SEGMENT_CENSUS_BLOCK_NONKEY_HUFF_FILE2;
		static const char* STREET_SEGMENT_POSTCODE_EXT_KEY_HUFF_FILE;
		static const char* STREET_SEGMENT_POSTCODE_EXT_NONKEY_HUFF_FILE;
		static const char* STREET_SEGMENT_COORDINATE_ID_HUFF_FILE1;
		static const char* STREET_SEGMENT_COORDINATE_ID_HUFF_FILE2;
		static const char* STREET_SEGMENT_COORDINATE_COUNT_HUFF_FILE;

		static const char* COORDINATE_LATITUDE_HUFF_FILE1;
		static const char* COORDINATE_LATITUDE_HUFF_FILE2;
		static const char* COORDINATE_LONGITUDE_HUFF_FILE1;
		static const char* COORDINATE_LONGITUDE_HUFF_FILE2;

		static const char* STREET_INTERSECTION_STATE_HUFF_FILE;
		static const char* STREET_INTERSECTION_SOUNDEX1_HUFF_FILE;
		static const char* STREET_INTERSECTION_STREET_NAME_ID1_HUFF_FILE;
		static const char* STREET_INTERSECTION_STREET_SEGMENT_OFFSET1_HUFF_FILE;
		static const char* STREET_INTERSECTION_SOUNDEX2_HUFF_FILE;
		static const char* STREET_INTERSECTION_STREET_NAME_ID2_HUFF_FILE;
		static const char* STREET_INTERSECTION_STREET_SEGMENT_OFFSET2_HUFF_FILE;

#if defined(WIN32)
		static HINSTANCE dllModuleInstance;
#endif // WIND32

		enum {
			CityStatePostcodeCountryFieldLength = 2,	// in bytes
			CityStatePostcodeStateFieldLength = 1,		// in bytes
			CityStatePostcodePostcodeFieldLength = 6,	// in bytes
			CityStatePostcodeCityNameFieldLength = 40,	// in bytes
			CityStatePostcodeFinanceFieldLength = 6,	// in bytes
			CityStatePostcodeStreetNameIDFirstFieldLength = 3,	// in bytes
			CityStatePostcodeStreetNameIDLastFieldLength = 3,	// in bytes
			CityStatePostcodeRecordLength = (
				CityStatePostcodeCountryFieldLength +
				CityStatePostcodeStateFieldLength +
				CityStatePostcodePostcodeFieldLength +
				CityStatePostcodeCityNameFieldLength +
				CityStatePostcodeFinanceFieldLength +
				CityStatePostcodeStreetNameIDFirstFieldLength +
				CityStatePostcodeStreetNameIDLastFieldLength
			),

			CityStatePostcodeFaIndexFinanceFieldLength = 6,		// in bytes
			CityStatePostcodeFaIndexCityStatePostcodeIDFieldLength = 3,		// in bytes
			CityStatePostcodeFaIndexRecordLength = (
				CityStatePostcodeFaIndexFinanceFieldLength + 
				CityStatePostcodeFaIndexCityStatePostcodeIDFieldLength
			),

			CitySoundexFieldLength = 4,		// length of unpacked soundex, in bytes
			CitySoundexRecordLength = 6,	// in bytes

			StreetNamePositionIndexBitSize = 29,
			StreetNameCityStatePostcodeIDBitSize = 18,
			StreetNameStreetSegmentIDFirstBitSize = 27,
			StreetNameNameFieldLength = 40,
			StreetNameChunkSize = 10,

			StreetNameSoundexFaBitSize = 24,	// packed FA, in bits
			StreetNameSoundexBitSize = 16,			// length of packed soundex in bits
			StreetNameSoundexStreetNameIDBitSize = 23,
			StreetNameSoundexRecordBitSize = (StreetNameSoundexFaBitSize + StreetNameSoundexBitSize + StreetNameSoundexStreetNameIDBitSize),

			StreetSegmentPositionIndexBitSize = 32,
			StreetSegmentLeftRightBitSize = 1,
//			StreetSegmentCoordinateIDBitSize = 27,
			StreetSegmentCoordinateIDBitSize = 28,
			StreetSegmentChunkSize = 10,

			CoordinatePositionIndexBitSize = 32,
			CoordinateLatitudeBitSize = 25,
			CoordinateLongitudeBitSize = 26,
			CoordinateChunkSize = 40,

			StreetIntersectionStateBitSize = 8, 
			StreetIntersectionSoundexBitSize = 16,
			StreetIntersectionStreetNameIDBitSize = 23,
			StreetIntersectionPositionIndexBitSize = 32,
			StreetIntersectionSoundexChunkSize = 20,

			PostcodeAliasPostcodeFieldLength = 6,	// in bytes
			PostcodeAliasGroupFieldLength = 6,		// in bytes
			PostcodeAliasRecordLength = 12,			// in bytes

			PostcodeCentroidPostcodeFieldLength = 6,		// in bytes
			PostcodeCentroidLongitudeFieldLength = 4,		// in bytes
			PostcodeCentroidLatitudeFieldLength = 4,		// in bytes
			PostcodeCentroidRecordLength = 14				// in bytes
		};


		///////////////////////////////////////////////////////////////////////////
		// PackSoundex: Compact a soundex to a 16-bit integer.
		// Inputs:
		//	const unsigned char*	buf		Four-byte buffer containing soundex.
		// Return value:
		//	unsigned int	Soundex value compacted into 16 bits.
		///////////////////////////////////////////////////////////////////////////
		static unsigned int PackSoundex(const unsigned char* buf);
		static inline unsigned int PackSoundex(const char* buf) {
			return PackSoundex((const unsigned char*) buf);
		}

		///////////////////////////////////////////////////////////////////////////
		// UnpackSoundex: Convert a compacted soundex back to a buffer.
		// Inputs:
		//	unsigned int	value		Compacted soundex value.
		// Outputs:
		//	unsigned char*	bufReturn	Four-byte buffer containing soundex.
		///////////////////////////////////////////////////////////////////////////
		static void UnpackSoundex(
			unsigned char* bufReturn,
			unsigned int value
		);

		static inline void UnpackSoundex(
			char* bufReturn,
			unsigned int value
		) {
			UnpackSoundex((unsigned char*)bufReturn, value);
		}


		///////////////////////////////////////////////////////////////////////////
		// PackFa: Convert a finance area to a 24-bit integer.
		// Inputs:
		//	const unsigned char*	buf		Four-byte buffer containing Fa.
		// Return value:
		//	unsigned int	Fa value compacted into 16 bits.
		///////////////////////////////////////////////////////////////////////////
		static unsigned int PackFa(const unsigned char* buf);
		static inline unsigned int PackFa(const char* buf) {
			return PackFa((const unsigned char*) buf);
		}

		///////////////////////////////////////////////////////////////////////////
		// UnpackFa: Convert a compacted finance are back to a buffer.
		// Inputs:
		//	unsigned int	value		Compacted Fa value.
		// Outputs:
		//	unsigned char*	bufReturn	seven-byte buffer containing 
		//								null-terminated finance number
		///////////////////////////////////////////////////////////////////////////
		static void UnpackFa(
			unsigned char* bufReturn,
			unsigned int value
		);

		static inline void UnpackFa(
			char* bufReturn,
			unsigned int value
		) {
			UnpackFa((unsigned char*)bufReturn, value);
		}

		// Compare two strings and return the number of errors.
		static int EditDistance(
			const char* s1,
			const char* s2
		);


		///////////////////////////////////////////////////////////////////////////////
		// Convert a string to a vector, treating the string elements like
		// unsigned chars.  Subtract the given offset from each string element.
		///////////////////////////////////////////////////////////////////////////////
		static inline void StrToVec(
			std::vector<int>& vec,
			const TsString& str,
			int offset = 0
		) {
			vec.resize(str.size());
			for (unsigned i = 0; i < str.size(); i++) {
				vec[i] = (int)(unsigned char)str[i] - offset;
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Convert a string to a vector, treating the string elements like
		// unsigned chars.  Subtract the given offset from each string element.
		///////////////////////////////////////////////////////////////////////////////
		static inline void StrToVec(
			std::vector<int>& vec,
			const char* str,
			int offset = 0
		) {
			size_t length = strlen(str);
			vec.resize(length);
			for (unsigned i = 0; i < length; i++) {
				vec[i] = (int)(unsigned char)str[i] - offset;
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Convert a vector to a string, treating the string elements like
		// unsigned chars.  Add the given offset to each vector element.
		///////////////////////////////////////////////////////////////////////////////
		static inline void VecToStr(
			TsString& str,
			const std::vector<int>& vec,
			int offset = 0
		) {
			str.resize(vec.size());
			for (unsigned i = 0; i < vec.size(); i++) {
				str[i] = (unsigned char)(vec[i] + offset);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Run-length compress zeros of the vector, assuming that the
		// input values are in the range 0-255.  Code run-lengths as values of the
		// form 0x1nnn, where nnn is the count.
		// Inputs:
		//	std::vector<int>&	vec		The input vector to compress.
		// Outputs:
		//	std::vector<int>&	vec		The output vector.
		///////////////////////////////////////////////////////////////////////////////
		static void RLECompressVec(
			std::vector<int>& vec
		);

		///////////////////////////////////////////////////////////////////////////////
		// Run-length de-compress zeros of the vector, performing the opposite
		// transform of RLECompressVec.
		// Inputs:
		//	std::vector<int>&	vec			The input vector to compress.
		// Outputs:
		//	std::vector<int>&	vecResult	The output vector.
		///////////////////////////////////////////////////////////////////////////////
		static void RLEDecompressVec(
			std::vector<int>& vecResult,
			const std::vector<int>& vec
		);

		///////////////////////////////////////////////////////////////////////////////
		// Convert a string to an integer vector, offsetting each char by the
		// given offset.  Run-length compress the resulting zeros, assuming that the
		// input values are in the range 0-255.  Code run-lengths as values of the
		// form 0x1nnn, where nnn is the count.
		// Inputs:
		//	std:;string&		str		The input string.  Chars will be interpreted
		//								as unsigned.
		//	int					offset	Amount to subtract from each string char.
		// Outputs:
		//	std::vector<int>&	vec		The output vector.
		///////////////////////////////////////////////////////////////////////////////
		static inline void RLECompressStrToVec(
			std::vector<int>& vec,
			const TsString& str,
			int offset
		) {
			StrToVec(vec, str, offset);
			RLECompressVec(vec);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Perform the opposite of RLECompressStrToVec.
		// Inputs:
		//	std::vector<int>&	vec		The input vector.
		//	int					offset	Amount to subtract from each string char.
		//	std::vector<int>&	tmpVec	A vector used for temp space while processing.
		// Outputs:
		//	TsString&		str		The input string.  Chars will be interpreted
		//								as unsigned.
		///////////////////////////////////////////////////////////////////////////////
		static inline void RLEDecompressVecToStr(
			TsString& str,
			const std::vector<int>& vec,
			int offset,
			std::vector<int>& tmpVec
		) {
			RLEDecompressVec(tmpVec, vec);
			VecToStr(str, tmpVec, offset);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Substract one vector of ints from antother.  If the vectors are not the
		// same length, only the overlapping trailing portion of the destination is
		// affected.
		// Inputs:
		//	std::vector<int>&	dest	The vector to be operated upon
		//	std::vector<int>&	src		The vector to subtract
		// Outputs:
		//	std::vector<int>&	dest	The resulting vector
		///////////////////////////////////////////////////////////////////////////////
		static inline void VecDiff(
			std::vector<int>& dest,
			const std::vector<int>& src
		) {
			int sizeDiff = int(dest.size() - src.size());
			if (sizeDiff >= 0) {
				// Dest longer than src
				for (unsigned i = 0; i < src.size(); i++) {
					dest[sizeDiff + i] -= src[i];
				}
			} else {
				// Src longer than dest
				sizeDiff = -sizeDiff;
				for (unsigned i = 0; i < dest.size(); i++) {
					dest[i] -= src[sizeDiff + i];
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Add one vector of ints from antother.  If the vectors are not the
		// same length, only the overlapping trailing portion of the destination is
		// affected.
		// Inputs:
		//	std::vector<int>&	dest	The vector to be operated upon
		//	std::vector<int>&	src		The vector to add
		// Outputs:
		//	std::vector<int>&	dest	The resulting vector
		///////////////////////////////////////////////////////////////////////////////
		static inline void VecAdd(
			std::vector<int>& dest,
			const std::vector<int>& src
		) {
			int sizeDiff = int(dest.size() - src.size());
			if (sizeDiff >= 0) {
				// Dest longer than src
				for (unsigned i = 0; i < src.size(); i++) {
					dest[sizeDiff + i] += src[i];
				}
			} else {
				// Src longer than dest
				sizeDiff = -sizeDiff;
				for (unsigned i = 0; i < dest.size(); i++) {
					dest[i] += src[sizeDiff + i];
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
		static int IntToVarLengthBuf(
			int value,
			unsigned char* buf
		);

		///////////////////////////////////////////////////////////////////////////
		// Given an integer value, fill a buffer with the required number of
		// bytes to represent it as a variable-length sequence.
		// Inputs:
		//	unsigned int	value	The value.
		// Outputs:
		//	unsigned char*	buf		The buffer to fill.  Must be at least six bytes long.
		// Return value:
		//	int			The number of bytes filled.
		///////////////////////////////////////////////////////////////////////////
		static int IntToVarLengthBuf(
			unsigned int value,
			unsigned char* buf
		);

		static void AddArrayToFreqTable(
			FreqTable<int>& freqTable,
			const unsigned char* array,
			int arraySize
		);

		static void CodeVarLengthIntToBitStream(
			int value,
			HuffmanCoder<int, std::less<int> >& coder,
			BitStreamWrite& bitStream
		);

		static void CodeVarLengthIntToBitStream(
			unsigned int value,
			HuffmanCoder<int, std::less<int> >& coder,
			BitStreamWrite& bitStream
		);

		///////////////////////////////////////////////////////////////////////////
		// Class to convert variable-length buffers to signed/unsigned integers.
		///////////////////////////////////////////////////////////////////////////
		class VarLengthBufToInt {
		public:
			///////////////////////////////////////////////////////////////////////////
			// Start the conversion with the first byte.
			// Inputs:
			//	unsigned char	byte		The next byte to add
			// Return value:
			//	bool		true if another byte is needed.
			///////////////////////////////////////////////////////////////////////////
			bool FirstByte(unsigned char byte) {
				value = byte & 0x7f;
				return (byte & 0x80) != 0;
			}

			///////////////////////////////////////////////////////////////////////////
			// Add another byte to the value.
			// Inputs:
			//	unsigned char	byte		The next byte to add
			// Return value:
			//	bool		true if another byte is needed.
			///////////////////////////////////////////////////////////////////////////
			bool NextByte(unsigned char byte) {
				value = (value << 7) | (byte & 0x7f);
				return (byte & 0x80) != 0;
			}

			///////////////////////////////////////////////////////////////////////////
			// When FirstByte() or NextByte() returns false, call this to get the value.
			///////////////////////////////////////////////////////////////////////////
			unsigned int GetUnsignedValue() const { return value; }
			int GetSignedValue() const { 
				// Process the sign bit stored in LSB
				return (value & 1) ? -(int)(value >> 1) : (int)(value >> 1);
			}

		protected:
			unsigned int value;
		};

	};


}

#endif
