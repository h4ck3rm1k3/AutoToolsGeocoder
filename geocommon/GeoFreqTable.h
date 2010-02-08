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

// FreqTable.h:  Helper class for generating frequency tables for Huffman coding

#ifndef INCL_FREQTABLE_H
#define INCL_FREQTABLE_H

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "Geocoder_DllExport.h"

#include <map>
#include <iostream>
#include <sstream>
#include "../global/TsString.h"

namespace PortfolioExplorer {

	template<class T> class FreqTable {
	public:
		typedef typename std::map<T, int>::const_iterator const_iterator;
		const_iterator begin() const { return freqTable.begin(); }
		const_iterator end() const { return freqTable.end(); }

		typedef typename std::map<T, int>::iterator iterator;
		iterator begin() { return freqTable.begin(); }
		iterator end() { return freqTable.end(); }

		// Count another item
		void Count(const T& key, int count = 1) {
			iterator iter = freqTable.find(key);
			if (iter == freqTable.end()) {
			  // new entry
			  freqTable.insert( std::make_pair( key, count ) );
			} else {
				// increment existing entry
				(*iter).second += count;
			}
		}

		// Get a specific item's count
		int GetCount(const T& t) {
			iterator iter = freqTable.find(t);
			if (iter == freqTable.end()) {
				return 0;
			} else {
				return (*iter).second;
			}
		}

		// Save the freq table to a file in "human-readable" form.
		bool Save(std::ostream& os);

		// Load the freq table from a file in "human-readable" form.
		bool Load(std::istream& is);

	protected:
		std::map<T, int> freqTable;
	};

	// Out-of-line functions

	// Save the freq table to a file in "human-readable" form.
	template<class T> bool FreqTable<T>::Save(std::ostream& os)
	{
		TsString str;
		for (iterator iter = begin(); iter != end(); ++iter) {
			// Encapsulate the token with single quotes, and double embedded quotes.
			os << "'";
			std::stringstream ss;
			ss << (*iter).first;
			ss << '\0';
			str = ss.str();
			for (const char* p = str.c_str(); *p != 0; p++) {
				if (*p == '\'') {
					os << '\'';
				}
				os << *p;
			}
			os << "' " << (*iter).second << "\n";
			if (!os.good()) {
				return false;
			}
		}
		return true;
	}

	// Load the freq table from a file in "human-readable" form.
	template<class T> bool FreqTable<T>::Load(std::istream& is)
	{
		freqTable.clear();
		T t;
		int i;
		while (true) {
			// Skip leading '
			while (is.peek() != '\'' && is.good()) {
				is.get();
			}
			is.get();
			// Read token
			std::stringstream ss;
			while (is.good()) {
				char c = is.get();
				if (c == '\'') {
					if (is.peek() == '\'') {
						// Doubled quote char
						is.get();
					} else {
						// End of token
						break;
					}
				}
				ss << c;
			}
			ss.seekg(0);
			ss >> t;
			// Read value
			is >> i;
			if (!is.good()) {
				break;
			}
			freqTable.insert( std::make_pair(t, i));
		}
		return is.eof();
	}


	// Special-case for TsString

	// Load the freq table from a file in "human-readable" form.
	// This is a specialization for TsString
	class StringFreqTable : public FreqTable<TsString> {
	public:
		bool Load(std::istream& is)
		{
			freqTable.clear();
			TsString t;
			int i;
			for(;;) {
				t = "";
				// Skip leading '
				while (is.peek() != '\'' && is.good()) {
					is.get();
				}
				is.get();
				// Read token
				while (is.good()) {
					char c = char(is.get());
					if (c == '\'') {
						if (is.peek() == '\'') {
							// Doubled quote char
							is.get();
						} else {
							// End of token
							break;
						}
					}
					t += c;
				}
				// Read value
				is >> i;
				if (!is.good()) {
					break;
				}
				freqTable.insert( std::make_pair(t, i) );
			}
			return is.eof();
		}
	};


}

#endif
