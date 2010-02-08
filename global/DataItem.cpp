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

// DataItem.cpp: Classes that can represent an arbitrary hierarchy of key/value pairs.
#include <stdlib.h>
#include "Global_Headers.h"
#include "DataItem.h"
#include "Utility.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// DataItem: Base class for storing a hierarchy of key/value pairs.
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////////
	DataItem::~DataItem() {}

	///////////////////////////////////////////////////////////////////////////////
	// Does this represent a valid integer value?
	// Return Value:
	//	bool		true if this can be interpreted as an integer
	///////////////////////////////////////////////////////////////////////////////
	bool DataItem::ValidInt() { return false; }

	///////////////////////////////////////////////////////////////////////////////
	// Does this represent a valid float value?  Exponential not allowed.
	// Return Value:
	//	bool		true if this can be interpreted as a float
	///////////////////////////////////////////////////////////////////////////////
	bool DataItem::ValidFloat() { return false; }

	///////////////////////////////////////////////////////////////////////////////
	// True is this can be interpreted as a bool value.
	// Return Value:
	//	bool		true if this can be interpreted as a bool
	///////////////////////////////////////////////////////////////////////////////
	bool DataItem::ValidBool() { return false; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the text of the item.
	// Return Value:
	//	TsString			The text of the item.
	// Note: arrays and associations will be blank.
	///////////////////////////////////////////////////////////////////////////////
	DataItem::operator TsString() { return ""; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the integer value of the item.  
	// Return Value:
	//	int			The integer value of the item.
	///////////////////////////////////////////////////////////////////////////////
	DataItem::operator int() { return 0; }
	DataItem::operator unsigned int() { return 0; }
	DataItem::operator __int64() { return 0; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the float value of the item. 
	// Return Value:
	//	float		The float value of the item.
	///////////////////////////////////////////////////////////////////////////////
	DataItem::operator float() { return 0.0; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the bool value of the item.  
	// Return Value:
	//	bool		The value interpreted as a bool
	///////////////////////////////////////////////////////////////////////////////
	DataItem::operator bool() { return false; }

	///////////////////////////////////////////////////////////////////////////////
	// Lookup an item in the association.  
	// Inputs:
	//	TsString		key		The key to look up.
	// Return Value:
	//	DataItemRef	The value found, or zero if none found.
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef DataItem::operator[](const TsString& key) { return 0; }

	///////////////////////////////////////////////////////////////////////////////
	// Get the size of the array.  
	///////////////////////////////////////////////////////////////////////////////
	int DataItem::GetSize() { return 0; }

	///////////////////////////////////////////////////////////////////////////////
	// Get an item in the array.  
	// Inputs:
	//	TsString		idx		The index into the array
	// Return Value:
	//	DataItemRef	The value found, or zero if idx out-of-bounds
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef DataItem::operator[](int idx) { return 0; }

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// DataItemValue: DataItem subclass that implements value types:
	//		a string
	//		an identifier
	//		an integer
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Text Constructor
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::DataItemValue(
		TsString text_, 
		int line_
	) : 
		DataItem(line_), 
		text(text_)
	{}

	///////////////////////////////////////////////////////////////////////////////
	// Text Constructor
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::DataItemValue(
		const char* text_, 
		int line_
	) : 
		DataItem(line_), 
		text(text_)
	{}

	///////////////////////////////////////////////////////////////////////////////
	// Shortcut constructor for integer values
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::DataItemValue(
		int value, 
		int line_
	) :
		DataItem(line_),
		text(FormatInteger(value))
	{}

	DataItemValue::DataItemValue(
		unsigned int value, 
		int line_
	) :
		DataItem(line_),
		text(FormatInteger(value))
	{}

	DataItemValue::DataItemValue(
		__int64 value, 
		int line_
	) :
		DataItem(line_),
		text(FormatInteger(value))
	{}

	DataItemValue::DataItemValue(
		float value, 
		int line_
	) :
		DataItem(line_),
		text(FormatFloat(value))
	{}

	///////////////////////////////////////////////////////////////////////////////
	// Shortcut constructor for bool values
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::DataItemValue(
		bool value, 
		int line_
	) :
		DataItem(line_),
		text(value ? "Y" : "N")
	{}

	///////////////////////////////////////////////////////////////////////////////
	// Does this represent a valid integer value?
	// Return Value:
	//	bool		true if this can be interpreted as an integer
	///////////////////////////////////////////////////////////////////////////////
	bool DataItemValue::ValidInt()
	{
		if (text.empty()) {
			return false;
		}
		bool haveDigit = false;
		bool haveSign = false;
		for (unsigned idx = 0; idx < text.size(); idx++) {
			if (text[idx] == ' ') {
				if (haveDigit || haveSign) {
					// Trailing space.  The rest must be spaces.
					for (; idx < text.size(); idx++) {
						if (text[idx] != ' ') {
							return false;
						}
					}
				}
			} else if (ISDIGIT(text[idx])) {
				haveDigit = true;
			} else if (text[idx] == '-') {
				if (haveSign || haveDigit) {
					return false;
				}
				haveSign = true;
			} else {
				return false;
			}
		}
		return haveDigit;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Does this represent a valid float value?  Exponential not allowed.
	// Return Value:
	//	bool		true if this can be interpreted as a float
	///////////////////////////////////////////////////////////////////////////////
	bool DataItemValue::ValidFloat()
	{
		if (text.empty()) {
			return false;
		}
		bool haveDecimal = false;
		bool haveDigit = false;
		bool haveSign = false;
		for (unsigned idx = 0; idx < text.size(); idx++) {
			if (text[idx] == ' ') {
				if (haveDigit || haveSign || haveDecimal) {
					// Trailing space.  The rest must be spaces.
					for (; idx < text.size(); idx++) {
						if (text[idx] != ' ') {
							return false;
						}
					}
				}
			} else if (ISDIGIT(text[idx])) {
				haveDigit = true;
			} else if (text[idx] == '-') {
				if (haveSign || haveDigit || haveDecimal) {
					return false;
				}
				haveSign = true;
			} else if (text[idx] == '.') {
				if (haveDecimal) {
					return false;
				}
				haveDecimal = true;
			} else {
				return false;
			}
		}
		return haveDigit;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Is this a valid bool value?
	// Return Value:
	//	bool		true if this can be interpreted as a bool
	///////////////////////////////////////////////////////////////////////////////
	bool DataItemValue::ValidBool() 
	{
		TsString tmp = TsString(text);
		if (tmp.size() != 1) {
			return false;
		}
		switch (tmp[0]) {
		case 'y': case 'Y': case 't': case 'T':
		case 'f': case 'F': case 'n': case 'N':
			return true;
		default:
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the integer value of the element.
	// Return Value:
	//	int			The integer value of the element.
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::operator int()
	{ 
		int retval;
		return ScanInteger(text.c_str(), retval) ? retval : 0;
	}
	DataItemValue::operator unsigned int()
	{ 
		unsigned int retval;
		return ScanInteger(text.c_str(), retval) ? retval : 0;
	}
	DataItemValue::operator __int64()
	{
		__int64 retval;
		return ScanInteger(text.c_str(), retval) ? retval : 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the float value of the element.
	// Return Value:
	//	float		The float value of the element.
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::operator float()
	{ 
		return float(atof(text.c_str())); 
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the bool value of the element.
	// Return Value:
	//	bool		The value interpreted as a bool
	///////////////////////////////////////////////////////////////////////////////
	DataItemValue::operator bool() 
	{ 
		TsString tmp = TsString(text);
		if (tmp.empty()) {
			return false;
		}
		switch (tmp[0]) {
		case 'y': case 'Y': case 't': case 'T': case '1':
			return true;
		default:
			return false;
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// DataItemAssociation: DataItem subclass to implement dictionaries.
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Lookup an element in the association. 
	// Inputs:
	//	TsString		key		The key to look up.
	// Return Value:
	//	DataItemRef	The value found, or zero if none found.
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef DataItemAssociation::operator[](
		const TsString& key
	) {
		ElementTable::iterator iter = elements.find(key);
		return (iter == elements.end()) ? 0 : (*iter).second;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Add an item to the association
	// Inputs:
	//	TsString			key		The key under which item is added
	//	DataItemRef	item	The item to add under the key
	//
	// Note: if the given key already exists, then the new value item will
	// replace the existing one.
	///////////////////////////////////////////////////////////////////////////////
	void DataItemAssociation::AddItem(
		const TsString& key,
		DataItemRef item
	) {
	        RemoveItem(key);
		elements.insert(ElementTable::value_type(key, item));
		iter = elements.end();
	}
	void DataItemAssociation::AddItem(const TsString& key, DataItem* item)
	{
		AddItem(key, (DataItemRef)item);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Convenience methods for common data types
	///////////////////////////////////////////////////////////////////////////////
	void DataItemAssociation::AddItem(const TsString& key, int value) {
		AddItem(key, new DataItemValue(value));
	}
	void DataItemAssociation::AddItem(const TsString& key, unsigned int value) {
		AddItem(key, new DataItemValue(value));
	}
	void DataItemAssociation::AddItem(const TsString& key, __int64 value) {
		AddItem(key, new DataItemValue(value));
	}
	void DataItemAssociation::AddItem(const TsString& key, float value) {
		AddItem(key, new DataItemValue(value));
	}
	void DataItemAssociation::AddItem(const TsString& key, const TsString& value) {
		AddItem(key, new DataItemValue(value));
	}
	void DataItemAssociation::AddItem(const TsString& key, const char* value) {
		AddItem(key, new DataItemValue(value));
	}
	void DataItemAssociation::AddItem(const TsString& key, bool value) {
		AddItem(key, new DataItemValue(value));
	}

	///////////////////////////////////////////////////////////////////////////////
	// Remove an item from the association
	// Inputs:
	//	TsString			key		The key to remove
	///////////////////////////////////////////////////////////////////////////////
	void DataItemAssociation::RemoveItem(const TsString& key) 
	{
		ElementTable::iterator findIter = elements.find(key);
		if (findIter != elements.end()) {
			elements.erase(findIter);
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// Get the current item in the association.
	// Outputs:
	//	TsString&	keyReturn		The key under which the element is found
	// Return Value:
	//	DataItemRef		The element, or zero if there are no more elements.
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef DataItemAssociation::GetElement(
		TsString& keyReturn
	) {
		if (iter == elements.end()) {
			return 0;
		}
		keyReturn = (*iter).first;
		return (*iter).second;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Advance to the next element in the association
	///////////////////////////////////////////////////////////////////////////////
	void DataItemAssociation::NextElement() 
	{
		if (iter != elements.end()) {
			++iter;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Copy constructor.  Must do deep copy.
	///////////////////////////////////////////////////////////////////////////////
	DataItemAssociation::DataItemAssociation(
		const DataItemAssociation& rhs
	) :
		DataItem(rhs)
	{
		*this = rhs;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Assignemtn operator.  Must do deep copy.
	///////////////////////////////////////////////////////////////////////////////
	DataItemAssociation& DataItemAssociation::operator=(
		const DataItemAssociation& rhs
	) {
		if (this != &rhs) {
			elements.clear();
			for (
				ElementTable::const_iterator tmpIter = rhs.elements.begin();
				tmpIter != rhs.elements.end();
				++tmpIter
			) {
				AddItem((*tmpIter).first, (*tmpIter).second->Clone());
			}
		}
		return *this;
	}


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// DataItemArray: DataItem subclass that implements an array of elements.
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Get an element in the association.
	// Inputs:
	//	TsString		idx		The index into the array
	// Return Value:
	//	DataItemRef	The value found, or zero if idx out-of-bounds
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef DataItemArray::operator[](int idx) 
	{
		return (idx < (int)elements.size() && idx >= 0) ? elements[idx] : 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Append an item to the array
	// Inputs:
	//	DataItemRef		item	The item to add 
	///////////////////////////////////////////////////////////////////////////////
	void DataItemArray::AppendItem(DataItemRef item) 
	{
		elements.push_back(item);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Convenience methods for common data types
	///////////////////////////////////////////////////////////////////////////////
	void DataItemArray::AppendItem(int value) {
		AppendItem(new DataItemValue(value));
	}
	void DataItemArray::AppendItem(unsigned int value) {
		AppendItem(new DataItemValue(value));
	}
	void DataItemArray::AppendItem(__int64 value) {
		AppendItem(new DataItemValue(value));
	}
	void DataItemArray::AppendItem(float value) {
		AppendItem(new DataItemValue(value));
	}
	void DataItemArray::AppendItem(const TsString& value) {
		AppendItem(new DataItemValue(value));
	}
	void DataItemArray::AppendItem(const char* value) {
		AppendItem(new DataItemValue(value));
	}
	void DataItemArray::AppendItem(bool value) {
		AppendItem(new DataItemValue(value));
	}

	///////////////////////////////////////////////////////////////////////////////
	// Replace an item in the array
	// Inputs:
	//	int				idx			Index into the array
	//	DataItemRef		item		The replacement item 
	///////////////////////////////////////////////////////////////////////////////
	void DataItemArray::ReplaceItem(
		int idx,
		DataItemRef item
	) {
		if (idx < (int)elements.size() && idx >= 0) {
			elements[idx] = item;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Copy Constructor.  Must do deep copy.
	///////////////////////////////////////////////////////////////////////////////
	DataItemArray::DataItemArray(
		const DataItemArray& rhs
	) :
		DataItem(rhs)
	{
		*this = rhs;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Assignment operator.
	///////////////////////////////////////////////////////////////////////////////
	DataItemArray& DataItemArray::operator=(
		const DataItemArray& rhs
	) {
		if (this != &rhs) {
			elements.clear();
			// Deep copy all elements
			for (unsigned i = 0; i < rhs.elements.size(); i++) {
				AppendItem(rhs.elements[i]->Clone());
			}
		}
		return *this;
	}

} // namespace

