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
# $Rev: 54 $ 
# $Date: 2006-10-07 12:53:22 +0200 (Sat, 07 Oct 2006) $ 
*/

// RegularExprLiteralRange.cpp: Defines the regular expression literal object

#include "Global_Headers.h"
#include "RegularExprEngineBase.h"
#include "RegularExprLiteralRange.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// SetupBind: Check if this expressions ordinals are represented in the map
	//            and add them if need be.
	// Inputs:
	//  OrdinalMap&		map				map of token->ordinal
	//  ListenerRef 	listener		reports any errors encountered
	//
	// Return value:
	//	bool		true if add succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprLiteralRange::SetupBind(OrdinalMap& map, ListenerRef listener)
	{
		OrdinalMap::iterator iterator = map.find(token.text);
		if( iterator == map.end() ) {
			std::pair<OrdinalMap::iterator, bool> insertPair = 
				map.insert(OrdinalMap::value_type(token.text, int(map.size())));
			if( insertPair.second == false ) {
			listener->Error("Couldn't insert literal into binding map");
				return false;
			}
			iterator = insertPair.first;
		}

		OrdinalMap::iterator highIterator = map.find(highToken.text);
		if( highIterator == map.end() ) {
			std::pair<OrdinalMap::iterator, bool> insertPair = 
				map.insert(OrdinalMap::value_type(token.text, int(map.size())));
			if( insertPair.second == false ) {
			listener->Error("Couldn't insert literal into binding map");
				return false;
			}
			highIterator = insertPair.first;
			return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Bind: Bind this expression to a map of ordinals, replacing token values with
	//       the appropriate ordinal.
	// Inputs:
	//  OrdinalMap&		map				map of token->ordinal
	//  ListenerRef 	listener		reports any errors encountered
	//
	// Return value:
	//	bool		true if bind succeeded, else false
	///////////////////////////////////////////////////////////////////////////////
	bool RegularExprLiteralRange::Bind(OrdinalMap& map, ListenerRef listener)
	{
		OrdinalMap::iterator iterator = map.find(token.text);
		if( iterator == map.end() ) {
			listener->Error("Couldn't find literal in binding map");
			return false;
		}

		OrdinalMap::iterator highIterator = map.find(highToken.text);
		if( highIterator == map.end() ) {
			listener->Error("Couldn't find literal in binding map");
			return false;
		}
		
		if( (*highIterator).first <= (*iterator).first ) {
			TsString errorMsg = "Literal " + token.text + " is not less than literal " +
				highToken.text + " .";
			listener->Error(errorMsg);
			return false;
		}
		//Make the ordinal bitset 1 greater than map size... the last value is
		//for unknown		
		
		ordinals = new Bitset(int(map.size() + 1));
		++highIterator;
		for(; iterator != highIterator; ++iterator) {
			ordinals->Set((*iterator).second);
		}

		return (bound = true);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Dump: output this regular expression to the ostream provided.
	// Inputs:
	//  ostream&		os				stream to output into
	//  TsString		prefix			string to prepend to expression info
	//
	// Return value:
	///////////////////////////////////////////////////////////////////////////////
	void RegularExprLiteralRange::Dump(std::ostream& os, TsString prefix)
	{
		os << prefix.c_str() << "LiteralRange: " << token.text.c_str() << " - " 
			<< highToken.text.c_str() << '\n';
	}

} //namespace
