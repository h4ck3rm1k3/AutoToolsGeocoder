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

// DataInput.h: Class used to read data from a file and present it as a bitstream.

#ifndef INCL_DATAINPUT_H
#define  INCL_DATAINPUT_H

#include <stdio.h>
#include "GeoAbstractByteIO.h"
#include "GeoUtil.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////
	// Read adaptor between FILE* and BitStream
	///////////////////////////////////////////////////////////////////////
	class FileByteReader : public ByteReader {
	public:
		FileByteReader() : fp(0) {}
		~FileByteReader() {}
		void SetFile(FILE* fp_) { fp = fp_; }
		// Returns the number of bytes actually read
		virtual int Read(int size, unsigned char* buffer) {
			return int(fread(buffer, 1, size, fp));
		}
		// Returns true on success, false on failure
		virtual bool Seek(int pos) { return fseek(fp, pos, SEEK_SET) == 0; }
		// Return the current position in the file.
		virtual int GetPosition() { return ftell(fp); }
	private:
		FILE* fp;
	};
	typedef refcnt_ptr<FileByteReader> FileByteReaderRef;

	///////////////////////////////////////////////////////////////////////
	// Class used to read data from a file and present it as a bitstream.
	// TODO: When caching is running, use non-buffered input.
	///////////////////////////////////////////////////////////////////////
	class DataInput {
		DataInput(DataInput &);
		DataInput & operator =(DataInput &);
	public:
		DataInput() : 
			fp(0),
			reader(new FileByteReader),
			bitStream(reader.get())
		{
		}
		~DataInput() { Close(); }
		bool IsOpen() { return fp != 0; }
		bool Open(const TsString& filename_) {
			if (IsOpen() && filename == filename_) {
				return true;
			} else {
				Close();
			}
			filename = filename_;
			fp = fopen(filename.c_str(), "rb");
			reader->SetFile(fp);
			if (fp == 0) {
				return false;
			}
			// Find the size of the file.
			fseek(fp, 0, SEEK_END);
			fileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			return true;
		}
		void Close() {
			if (IsOpen()) {
				reader->SetFile(0);
				fclose(fp);
			}
			fp = 0;
			filename = "";
		}

		// Use for bit-oriented I/O
		BitStreamRead& GetBitStream() { return bitStream; }

		// Byte-oriented I/O:
		// Returns the number of bytes actually read
		int Read(int size, unsigned char* buffer) {
			return reader->Read(size, buffer);
		}
		int Read(int size, char* buffer) {
			return reader->Read(size, (unsigned char*)buffer);
		}

		// Byte-oriented I/O:
		// Seek to given position in the file.
		// Returns true on success, false on failure
		bool Seek(int pos) {
			return reader->Seek(pos);
		}

		int GetFileSize() { return fileSize; }

		///////////////////////////////////////////////////////////////////////////
		// Byte-oriented I/O for reading unsigned integers.
		///////////////////////////////////////////////////////////////////////////
		bool ReadThreeByteInt(unsigned int &value) {
			unsigned char buf[3];
			if (Read(3, buf) != 3) {
				return false;
			}
			value = buf[0] | (buf[1] << 8) | (buf[2] << 16);
			return true;
		}
		bool ReadTwoByteInt(unsigned int& value) {
			unsigned char buf[2];
			if (Read(2, buf) != 2) {
				return false;
			}
			value = buf[0] | (buf[1] << 8);
			return true;
		}
		bool ReadOneByteInt(unsigned int& value) {
			unsigned char c;
			if (Read(1, &c) != 1) {
				return false;
			}
			value = c;
			return true;
		}
		bool ReadTwoByteInt(unsigned short& value) {
			unsigned char buf[2];
			if (Read(2, buf) != 2) {
				return false;
			}
			value = buf[0] | (buf[1] << 8);
			return true;
		}
		bool ReadOneByteInt(unsigned short& value) {
			unsigned char c;
			if (Read(1, &c) != 1) {
				return false;
			}
			value = c;
			return true;
		}
		bool ReadThreeByteInt( int &value) {
			unsigned char buf[3];
			if (Read(3, buf) != 3) {
				return false;
			}
			value = buf[0] | (buf[1] << 8) | (buf[2] << 16);
			if (buf[2] & 0x80) {
				// sign-extend
				value |= 0xFF000000;
			}
			return true;
		}
		bool ReadTwoByteInt( int& value) {
			unsigned char buf[2];
			if (Read(2, buf) != 2) {
				return false;
			}
			value = buf[0] | (buf[1] << 8);
			if (buf[1] & 0x80) {
				// sign-extend
				value |= 0xFFFF0000;
			}
			return true;
		}
		bool ReadOneByteInt( int& value) {
			unsigned char c;
			if (Read(1, &c) != 1) {
				return false;
			}
			value = c;
			if (c & 0x80) {
				// sign-extend
				value |= 0xFFFFFF00;
			}
			return true;
		}
		bool ReadTwoByteInt( short& value) {
			unsigned char buf[2];
			if (Read(2, buf) != 2) {
				return false;
			}
			value = buf[0] | (buf[1] << 8);
			if (buf[1] & 0x80) {
				// sign-extend
				value |= 0xFFFF0000;
			}
			return true;
		}
		bool ReadOneByteInt( short& value) {
			unsigned char c;
			if (Read(1, &c) != 1) {
				return false;
			}
			value = c;
			if (c & 0x80) {
				// sign-extend
				value |= 0xFFFFFF00;
			}
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Read a string value using a string coder, and place it into
		// a fixed-length char buffer with termination.
		///////////////////////////////////////////////////////////////////////////
		bool ReadStringFromCoder(
			char* buf,
			int bufSize,
			HuffmanCoder<TsString, std::less<TsString> >& coder
		) {
			const TsString* strPtr;
			if (!coder.ReadCode(bitStream, strPtr)) {
				return false;
			}
			strncpy(buf, (*strPtr).c_str(), bufSize - 1);
			buf[bufSize - 1] = 0;
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Read a variable-length string value using a per-character 
		// coder, and place it into a fixed-length buffer.
		///////////////////////////////////////////////////////////////////////////
		bool ReadStringFromCoder(
			char* buf,
			int bufSize,
			HuffmanCoder<int, std::less<int> >& coder
		) {
			char* ptr = buf;
			char* endPtr = buf + bufSize - 1;
			const int* codePtr;
			do {
				if (!coder.ReadCode(bitStream, codePtr)) {
					return false;
				}
				if (ptr < endPtr) {
					*ptr++ = char(*codePtr);
				}
			} while (*codePtr != 0);
			*ptr = 0;
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Read an integer value using an integer Huffman coder.
		///////////////////////////////////////////////////////////////////////////
		bool ReadIntFromCoder(
			int &result,
			HuffmanCoder<int, std::less<int> >& coder
		) {
			const int *codePtr;
			if (!coder.ReadCode(bitStream, codePtr)) {
				return false;
			}
			result = *codePtr;
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Read a variable-length integer via Huffman coder
		///////////////////////////////////////////////////////////////////////////
		bool ReadVarLengthCodedInt(
			unsigned int& valueReturn, 
			HuffmanCoder<int, std::less<int> >& coder
		) {
			// Call two-coder version
			return ReadVarLengthCodedInt(valueReturn, coder, coder);
		}
		bool ReadVarLengthCodedInt(
			int& valueReturn, 
			HuffmanCoder<int, std::less<int> >& coder
		) {
			// Call two-coder version
			return ReadVarLengthCodedInt(valueReturn, coder, coder);
		}

		///////////////////////////////////////////////////////////////////////////
		// Read a variable-length integer using a separate Huffman coder
		// for the first byte than for subsequent bytes
		///////////////////////////////////////////////////////////////////////////
		bool ReadVarLengthCodedInt(
			unsigned int& valueReturn, 
			HuffmanCoder<int, std::less<int> >& coder1,
			HuffmanCoder<int, std::less<int> >& coder2
		) {
			if (!ProcessVarLengthCodedInt(coder1, coder2)) {
				return false;
			}
			valueReturn = varLengthBufToInt.GetUnsignedValue();
			return true;
		}
		bool ReadVarLengthCodedInt(
			int& valueReturn, 
			HuffmanCoder<int, std::less<int> >& coder1,
			HuffmanCoder<int, std::less<int> >& coder2
		) {
			if (!ProcessVarLengthCodedInt(coder1, coder2)) {
				return false;
			}
			valueReturn = varLengthBufToInt.GetSignedValue();
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Read N bits into an integer. See BitStream::ReadBitsIntoInt
		///////////////////////////////////////////////////////////////////////////
		bool ReadBitsIntoInt(int nbrBits, int& value) {
			return bitStream.ReadBitsIntoInt(nbrBits, value);
		}
		bool ReadBitsIntoInt(int nbrBits, unsigned int& value) {
			return bitStream.ReadBitsIntoInt(nbrBits, value);
		}

		///////////////////////////////////////////////////////////////////////////
		// Read an RLE-compressed string via huffman coders, and add the given
		// offset to each element after decoding.  Also copy the result
		// string to a vector, which may be used in subsequent calls to
		// ReadRLECompressedStrDiff().
		// The first coder decodes the leading size byte, and the second
		// coder decodes the bytes of the string content.
		///////////////////////////////////////////////////////////////////////////
		bool ReadRLECompressedStr(
			char* buf,
			int bufSize,
			HuffmanCoder<int, std::less<int> >& coderSize,
			HuffmanCoder<int, std::less<int> >& coderBytes,
			int offset
		) {
			// Read leading length value
			const int *codePtr;
			if (!coderSize.ReadCode(bitStream, codePtr)) {
				return false;
			}
			int length = *codePtr;

			// Read bytes into vector via coder
			tmpVec1.resize(length);
			for (int i = 0; i < length; i++) {
				if (!coderBytes.ReadCode(bitStream, codePtr)) {
					return false;
				}
				tmpVec1[i] = *codePtr;
			}

			// Decompress bytes
			GeoUtil::RLEDecompressVec(tmpVec2, tmpVec1);

			// Transfer to buffer
			int limit = JHMIN(bufSize - 1, (int)tmpVec2.size());
			int j;
			for (j = 0; j < limit; j++) {
				buf[j] = char(tmpVec2[j] + offset);
			}
			buf[j] = 0;
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Above with a single coder for both size and contents of string.
		///////////////////////////////////////////////////////////////////////////
		bool ReadRLECompressedStr(
			char* buf,
			int bufSize,
			HuffmanCoder<int, std::less<int> >& coder,
			int offset
		) {
			return ReadRLECompressedStr(buf, bufSize, coder, coder, offset);
		}

		///////////////////////////////////////////////////////////////////////////
		// Read an RLE-compressed string via a coder, and interpret as a 
		// differrence from the given previous vector value.  Also copy the result
		// string to a vector, which may be used in subsequent calls to
		// ReadRLECompressedStrDiff().
		///////////////////////////////////////////////////////////////////////////
		bool ReadRLECompressedStrDiff(
			char* buf,
			int bufSize,
			HuffmanCoder<int, std::less<int> >& coderSize,
			HuffmanCoder<int, std::less<int> >& coderBytes,
			const char* previousValue,
			int offset
		) {
			// Read leading length value
			const int *codePtr;
			if (!coderSize.ReadCode(bitStream, codePtr)) {
				return false;
			}
			int length = *codePtr;
			assert(length >= 0 && length < bufSize);

			// Read bytes into vector via coder
			tmpVec1.resize(length);
			for (int i = 0; i < length; i++) {
				if (!coderBytes.ReadCode(bitStream, codePtr)) {
					return false;
				}
				tmpVec1[i] = *codePtr;
			}

			// Decompress bytes
			GeoUtil::RLEDecompressVec(tmpVec2, tmpVec1);

			// Add difference vector
			GeoUtil::StrToVec(tmpVec3, previousValue, offset);
			GeoUtil::VecAdd(tmpVec2, tmpVec3);

			// Transfer to buffer
			int limit = JHMIN(bufSize - 1, (int)tmpVec2.size());
			int j;
			for (j = 0; j < limit; j++) {
				buf[j] = char(tmpVec2[j] + offset);
			}
			buf[j] = 0;
			return true;
		}

		///////////////////////////////////////////////////////////////////////////
		// Above with single coder for size and bytes
		///////////////////////////////////////////////////////////////////////////
		bool ReadRLECompressedStrDiff(
			char* buf,
			int bufSize,
			HuffmanCoder<int, std::less<int> >& coder,
			const char* previousValue,
			int offset
		) {
			return ReadRLECompressedStrDiff(buf, bufSize, coder, coder, previousValue, offset);
		}

	private:
		///////////////////////////////////////////////////////////////////////////
		// Read a variable-length coded integer into a byte buffer, and prepare 
		//  it for conversion to an integer.
		///////////////////////////////////////////////////////////////////////////
		bool ProcessVarLengthCodedInt(
			HuffmanCoder<int, std::less<int> >& coder
		) {
			// Call two-coder version
			return ProcessVarLengthCodedInt(coder, coder);
		}

		///////////////////////////////////////////////////////////////////////////
		// Read a variable-length coded integer into a byte buffer, and prepare 
		//  it for conversion to an integer, where a separate coder is used for
		// the leading byte than for subsequent bytes
		///////////////////////////////////////////////////////////////////////////
		bool ProcessVarLengthCodedInt(
			HuffmanCoder<int, std::less<int> >& coder1,
			HuffmanCoder<int, std::less<int> >& coder2
		) {
			const int *codePtr;
			if (!coder1.ReadCode(bitStream, codePtr)) {
				return false;
			}
			if (varLengthBufToInt.FirstByte((unsigned char)(*codePtr))) {
				bool needMore;
				do {
					if (!coder2.ReadCode(bitStream, codePtr)) {
						return false;
					}
					needMore = varLengthBufToInt.NextByte((unsigned char)(*codePtr));
				} while (needMore);
			}
			return true;
		}

		TsString filename;
		FILE* fp;
		FileByteReaderRef reader;
		BitStreamRead bitStream;
		int fileSize;
		GeoUtil::VarLengthBufToInt varLengthBufToInt;
		std::vector<int> tmpVec1;
		std::vector<int> tmpVec2;
		std::vector<int> tmpVec3;
	};

}

#endif
