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

// DataItem.h: Declarations of a class that can represent an arbitrary
// hierarchy of key/value pairs.

#ifndef INCL_DATAITEM_H
#define INCL_DATAITEM_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "TsString.h"
#include <vector>
#include <assert.h>
#include "RefPtr.h"
#include "Utility.h"
#include "StringTorefMap.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class DataItemRef;

	///////////////////////////////////////////////////////////////////////////////
	// DataItem: Data item describing a value in a hiararchical data structure.
	// A data item can be one of several things
	//		a value
	//		a association of elements
	//		an array of elements
	// When a config-file is parsed, then returned node is a association.
	// This class has several derived classes that implement specific subtypes.
	///////////////////////////////////////////////////////////////////////////////
	class DataItem : public VRefCount {
	public:
		enum Type { Value, Association, Array };

		DataItem(int line_) : line(line_) {}
		virtual ~DataItem();

		///////////////////////////////////////////////////////////////////////////////
		// Get the sub-type of this item
		///////////////////////////////////////////////////////////////////////////////
		virtual Type GetType() = 0;

		///////////////////////////////////////////////////////////////////////////////
		// Does this represent a valid integer value?
		// Return Value:
		//	bool		true if this can be interpreted as an integer
		///////////////////////////////////////////////////////////////////////////////
		virtual bool ValidInt();

		///////////////////////////////////////////////////////////////////////////////
		// Does this represent a valid float value?  Exponential not allowed.
		// Return Value:
		//	bool		true if this can be interpreted as a float
		///////////////////////////////////////////////////////////////////////////////
		virtual bool ValidFloat();

		///////////////////////////////////////////////////////////////////////////////
		// True is this can be interpreted as a bool value.
		// Return Value:
		//	bool		true if this can be interpreted as a bool
		///////////////////////////////////////////////////////////////////////////////
		virtual bool ValidBool();

		///////////////////////////////////////////////////////////////////////////////
		// Get the text of the item.
		// Return Value:
		//	TsString			The text of the item.
		// Note: arrays and associations will be blank.
		///////////////////////////////////////////////////////////////////////////////
		virtual operator TsString();

		///////////////////////////////////////////////////////////////////////////////
		// Get the integer value of the item.  
		// Return Value:
		//	int			The integer value of the item.
		///////////////////////////////////////////////////////////////////////////////
		virtual operator int();
		virtual operator unsigned int();
		virtual operator __int64();

		///////////////////////////////////////////////////////////////////////////////
		// Get the float value of the item. 
		// Return Value:
		//	float		The float value of the item.
		///////////////////////////////////////////////////////////////////////////////
		virtual operator float();

		///////////////////////////////////////////////////////////////////////////////
		// Get the bool value of the item.  
		// Return Value:
		//	bool		The value interpreted as a bool
		///////////////////////////////////////////////////////////////////////////////
		virtual operator bool();

		///////////////////////////////////////////////////////////////////////////////
		// Lookup an item in the association.  
		// Inputs:
		//	TsString		key		The key to look up.
		// Return Value:
		//	DataItemRef	The value found, or zero if none found.
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef operator[](const TsString& /*key*/);

		///////////////////////////////////////////////////////////////////////////////
		// Get the size of the array.  
		///////////////////////////////////////////////////////////////////////////////
		virtual int GetSize();

		///////////////////////////////////////////////////////////////////////////////
		// Get an item in the array.  
		// Inputs:
		//	TsString		idx		The index into the array
		// Return Value:
		//	DataItemRef	The value found, or zero if idx out-of-bounds
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef operator[](int /*idx*/);

		///////////////////////////////////////////////////////////////////////////////
		// Get the line number on which the item was found in the input
		///////////////////////////////////////////////////////////////////////////////
		int GetLine() { return line; }

		///////////////////////////////////////////////////////////////////////////////
		// Create a deep copy of this item, and allocate it from the heap.
		// Return Value:
		//	DataItemRef		A deep-copy of this item and its children
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef Clone() = 0;

	private:
		int line;
		// default copy constructor and assignment operator are OK
	};


	///////////////////////////////////////////////////////////////////////////////
	// Some forward declarations of smart-pointers
	///////////////////////////////////////////////////////////////////////////////
	class DataItemValueRef;
	class DataItemAssociationRef;
	class DataItemArrayRef;

	///////////////////////////////////////////////////////////////////////////////
	// DataItemRef is a class so we can implement automatic conversion
	///////////////////////////////////////////////////////////////////////////////
	typedef refcnt_ptr<DataItem> DataItemRef_;
	class DataItemRef : public DataItemRef_ {
	public:
		// Must implement constructor and assignment ops
		DataItemRef(DataItem* rhs = 0) : DataItemRef_(rhs) {}
		DataItemRef(const DataItemRef& rhs) : DataItemRef_(rhs) {}
		DataItemRef& operator=(DataItem* rhs) { DataItemRef_::operator=(rhs); return *this; }
		DataItemRef& operator=(const DataItemRef& rhs) { DataItemRef_::operator=(rhs); return *this; }

		// Downcast operations -- bodies follow later
		DataItemValueRef toValue();
		DataItemAssociationRef toAssociation();
		DataItemArrayRef toArray();

		// Shortcuts for subscripting operations.
		// Lets you use item[] instead of (*item)[] syntax
		DataItemRef operator[](const TsString& key) { return (**this)[key]; }
		DataItemRef operator[](int index) { return (**this)[index]; }
	};


	///////////////////////////////////////////////////////////////////////////////
	// DataItemValue: DataItem subclass that implements a single value.
	///////////////////////////////////////////////////////////////////////////////
	class DataItemValue : public DataItem {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Standard text constructor
		///////////////////////////////////////////////////////////////////////////////
		DataItemValue(TsString text_, int line_ = 0);

		///////////////////////////////////////////////////////////////////////////////
		// Standard text constructor
		///////////////////////////////////////////////////////////////////////////////
		DataItemValue(const char* text_, int line_ = 0);

		///////////////////////////////////////////////////////////////////////////////
		// Shortcut constructor for integer values
		///////////////////////////////////////////////////////////////////////////////
		DataItemValue(int value, int line_ = 0);
		DataItemValue(unsigned int value, int line_ = 0);
			
		///////////////////////////////////////////////////////////////////////////////
		// Shortcut constructor for integer values
		///////////////////////////////////////////////////////////////////////////////
		DataItemValue(__int64 value, int line_ = 0);
			
		///////////////////////////////////////////////////////////////////////////////
		// Shortcut constructor for float values
		///////////////////////////////////////////////////////////////////////////////
		DataItemValue(float value, int line_ = 0);

		///////////////////////////////////////////////////////////////////////////////
		// Shortcut constructor for bool values
		///////////////////////////////////////////////////////////////////////////////
		DataItemValue(bool value, int line_ = 0);
			
		///////////////////////////////////////////////////////////////////////////////
		// destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~DataItemValue() {}

		///////////////////////////////////////////////////////////////////////////////
		// Get the sub-type of this item
		///////////////////////////////////////////////////////////////////////////////
		virtual Type GetType() { return Value; }

		///////////////////////////////////////////////////////////////////////////////
		// Does this represent a valid integer value?
		// Return Value:
		//	bool		true if this can be interpreted as an integer
		///////////////////////////////////////////////////////////////////////////////
		virtual bool ValidInt();

		///////////////////////////////////////////////////////////////////////////////
		// Does this represent a valid float value?  Exponential not allowed.
		// Return Value:
		//	bool		true if this can be interpreted as a float
		///////////////////////////////////////////////////////////////////////////////
		virtual bool ValidFloat();

		///////////////////////////////////////////////////////////////////////////////
		// Does this represent a valid bool value?
		// Return Value:
		//	bool		true if this can be interpreted as a bool
		///////////////////////////////////////////////////////////////////////////////
		virtual bool ValidBool();

		///////////////////////////////////////////////////////////////////////////////
		// Get the text of the item.
		// Return Value:
		//	TsString			The text of the item.
		///////////////////////////////////////////////////////////////////////////////
		virtual operator TsString() { return text; }

		///////////////////////////////////////////////////////////////////////////////
		// Get the integer value of the item. 
		// Return Value:
		//	int			The integer value of the item.
		///////////////////////////////////////////////////////////////////////////////
		virtual operator int();
		virtual operator unsigned int();
		virtual operator __int64();

		///////////////////////////////////////////////////////////////////////////////
		// Get the float value of the item. 
		// Return Value:
		//	float		The float value of the item.
		///////////////////////////////////////////////////////////////////////////////
		virtual operator float();

		///////////////////////////////////////////////////////////////////////////////
		// Get the bool value of the item.  
		// Return Value:
		//	bool		The value interpreted as a bool
		///////////////////////////////////////////////////////////////////////////////
		virtual operator bool();

		///////////////////////////////////////////////////////////////////////////////
		// Create a deep copy of this item and allocate it from the heap
		// Return Value:
		//	DataItemRef		A deep-copy of this item and its children
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef Clone() {
			// use default copy constructor
			return new DataItemValue(*this);
		}

	private:
		TsString text;
		// default copy/assignment is OK
	};

	///////////////////////////////////////////////////////////////////////////////
	// DataItemValueRef is a class so we can implement automatic conversion
	///////////////////////////////////////////////////////////////////////////////
	typedef refcnt_ptr<DataItemValue> DataItemValueRef_;
	class DataItemValueRef : public DataItemValueRef_ {
	public:
		// Must implement constructor and assignment ops
		DataItemValueRef(DataItemValue* rhs = 0) : DataItemValueRef_(rhs) {}
		DataItemValueRef(const DataItemValueRef& rhs) : DataItemValueRef_(rhs) {}
		DataItemValueRef& operator=(DataItemValue* rhs) { DataItemValueRef_::operator=(rhs); return *this; }
		DataItemValueRef& operator=(const DataItemValueRef& rhs) { DataItemValueRef_::operator=(rhs); return *this; }
		operator DataItemRef() const { return get(); }
	};



	///////////////////////////////////////////////////////////////////////////////
	// DataItemAssociation: DataItem subclass to implement associations.
	///////////////////////////////////////////////////////////////////////////////
	class DataItemAssociation : public DataItem {
	private:
		typedef StringToRefMap<DataItemRef> ElementTable;
	public:
		DataItemAssociation(int line_ = 0) : DataItem(line_) {}
		virtual ~DataItemAssociation() {}

		///////////////////////////////////////////////////////////////////////////////
		// Get the sub-type of this item
		///////////////////////////////////////////////////////////////////////////////
		virtual Type GetType() { return Association; }

		///////////////////////////////////////////////////////////////////////////////
		// Lookup an item in the association.  
		// Inputs:
		//	TsString		key		The key to look up.
		// Return Value:
		//	DataItemRef	The value found, or zero if none found.
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef operator[](const TsString& key);

		///////////////////////////////////////////////////////////////////////////////
		// Add an item to the association
		// Inputs:
		//	TsString			key		The key under which item is added
		//	DataItemRef			item	The item to add under the key
		//								The item is not copied.  It will be
		//								owned by the association item.
		//
		// Note: if the given key already exists, then the new value item will
		// replace the existing one.
		///////////////////////////////////////////////////////////////////////////////
		void AddItem(const TsString& key, DataItemRef item);
		void AddItem(const TsString& key, DataItem* item);

		///////////////////////////////////////////////////////////////////////////////
		// Convenience methods for common data types
		///////////////////////////////////////////////////////////////////////////////
		void AddItem(const TsString& key, int value);
		void AddItem(const TsString& key, unsigned int value);
		void AddItem(const TsString& key, __int64 value);
		void AddItem(const TsString& key, float value);
		void AddItem(const TsString& key, const TsString& value);
		void AddItem(const TsString& key, const char* value);
		void AddItem(const TsString& key, bool value);

		///////////////////////////////////////////////////////////////////////////////
		// Remove an item from the association
		// Inputs:
		//	TsString			key		The key to remove
		///////////////////////////////////////////////////////////////////////////////
		void RemoveItem(const TsString& key);

		///////////////////////////////////////////////////////////////////////////////
		// Point to the first item in the association.
		///////////////////////////////////////////////////////////////////////////////
		void FirstElement() {
			iter = elements.begin();
		}

		///////////////////////////////////////////////////////////////////////////////
		// Get the current item in the association.
		// Outputs:
		//	TsString&	keyReturn		The key under which the item is found
		// Return Value:
		//	DataItemRef		The item, or zero if there are no more elements.
		///////////////////////////////////////////////////////////////////////////////
		DataItemRef GetElement(TsString& keyReturn);

		///////////////////////////////////////////////////////////////////////////////
		// Advance to the next item in the association
		///////////////////////////////////////////////////////////////////////////////
		void NextElement();

		///////////////////////////////////////////////////////////////////////////////
		// Create a deep copy of this item and allocate it from the heap
		// Return Value:
		//	DataItemRef		A deep-copy of this item and its children
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef Clone() {
			return new DataItemAssociation(*this);
		}

	private:
		ElementTable elements;
		ElementTable::iterator iter;
		// Must do deep copy for copy constructor and assignment
		DataItemAssociation(const DataItemAssociation&);
		DataItemAssociation& operator=(const DataItemAssociation&);
	};

	///////////////////////////////////////////////////////////////////////////////
	// DataItemAssociationRef is a class so we can implement automatic conversion
	///////////////////////////////////////////////////////////////////////////////
	typedef refcnt_ptr<DataItemAssociation> DataItemAssociationRef_;
	class DataItemAssociationRef : public DataItemAssociationRef_ {
	public:
		// Must implement constructor and assignment ops
		DataItemAssociationRef(DataItemAssociation* rhs = 0) : DataItemAssociationRef_(rhs) {}
		DataItemAssociationRef(const DataItemAssociationRef& rhs) : DataItemAssociationRef_(rhs) {}
		DataItemAssociationRef& operator=(DataItemAssociation* rhs) { DataItemAssociationRef_::operator=(rhs); return *this; }
		DataItemAssociationRef& operator=(const DataItemAssociationRef& rhs) { DataItemAssociationRef_::operator=(rhs); return *this; }
		operator DataItemRef() const { return get(); }
		// Shortcut for subscripting operations.
		// Lets you use item[] instead of (*item)[] syntax
		DataItemRef operator[](const TsString& key) { return (**this)[key]; }
	};


	///////////////////////////////////////////////////////////////////////////////
	// DataItemArray: DataItem subclass that implements an array of elements.
	///////////////////////////////////////////////////////////////////////////////
	class DataItemArray : public DataItem {
	public:
		DataItemArray(int line_ = 0) : DataItem(line_) {}
		virtual ~DataItemArray() {}

		///////////////////////////////////////////////////////////////////////////////
		// Get the sub-type of this item
		///////////////////////////////////////////////////////////////////////////////
		virtual Type GetType() { return Array; }

		///////////////////////////////////////////////////////////////////////////////
		// Get the size of the array.  
		///////////////////////////////////////////////////////////////////////////////
		virtual int GetSize() { return int(elements.size()); }

		///////////////////////////////////////////////////////////////////////////////
		// Get an item in the association.  
		// Inputs:
		//	TsString		idx		The index into the array
		// Return Value:
		//	DataItemRef	The value found, or zero if idx out-of-bounds
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef operator[](int idx);

		///////////////////////////////////////////////////////////////////////////////
		// Append an item to the array
		// Inputs:
		//	DataItemRef	item	The item to add 
		///////////////////////////////////////////////////////////////////////////////
		void AppendItem(DataItemRef item);
		void AppendItem(DataItem* item) { AppendItem((DataItemRef)item); }

		///////////////////////////////////////////////////////////////////////////////
		// Convenience methods for common data types
		///////////////////////////////////////////////////////////////////////////////
		void AppendItem(int value);
		void AppendItem(unsigned int value);
		void AppendItem(__int64 value);
		void AppendItem(float value);
		void AppendItem(const TsString& value);
		void AppendItem(const char* value);
		void AppendItem(bool value);

		///////////////////////////////////////////////////////////////////////////////
		// Replace an item in the array
		// Inputs:
		//	int				idx			Index into the array
		//	DataItemRef		item		The replacement item 
		///////////////////////////////////////////////////////////////////////////////
		void ReplaceItem(
			int idx,
			DataItemRef item
		);

		///////////////////////////////////////////////////////////////////////////////
		// Create a deep copy of this item and allocate it from the heap
		// Return Value:
		//	DataItemRef		A deep-copy of this item and its children
		///////////////////////////////////////////////////////////////////////////////
		virtual DataItemRef Clone() {
			return new DataItemArray(*this);
		}

	private:
		typedef std::vector<DataItemRef> ElementArray;
		ElementArray elements;
		// Must do deep copy for copy constructor and assignment
		DataItemArray(const DataItemArray&);
		DataItemArray& operator=(const DataItemArray&);
	};

	///////////////////////////////////////////////////////////////////////////////
	// DataItemArrayRef is a class so we can implement automatic conversion.
	// Otherwise, downcasting smart-pointers is a laborious process.  We implement
	// conversion methods here to shorten the code for downcasting from a
	// DataItemRef to DataItem subclass smart-pointers.
	///////////////////////////////////////////////////////////////////////////////
	typedef refcnt_ptr<DataItemArray> DataItemArrayRef_;
	class DataItemArrayRef : public DataItemArrayRef_ {
	public:
		// Must implement constructor and assignment ops
		DataItemArrayRef(DataItemArray* rhs = 0) : DataItemArrayRef_(rhs) {}
		DataItemArrayRef(const DataItemArrayRef& rhs) : DataItemArrayRef_(rhs) {}
		DataItemArrayRef& operator=(DataItemArray* rhs) { DataItemArrayRef_::operator=(rhs); return *this; }
		DataItemArrayRef& operator=(const DataItemArrayRef& rhs) { DataItemArrayRef_::operator=(rhs); return *this; }
		operator DataItemRef() const { return get(); }
		// Shortcut for subscripting operations.
		// Lets you use item[] instead of (*item)[] syntax
		DataItemRef operator[](int key) { return (**this)[key]; }
	};


	///////////////////////////////////////////////////////////////////////////////
	// Bodies of DataItemRef methods that were deferred to avoid forward references
	///////////////////////////////////////////////////////////////////////////////
	inline DataItemValueRef DataItemRef::toValue() {
		assert(get()->GetType() == DataItem::Value);
		return static_cast<DataItemValue*>(get());
	}
		
	inline DataItemAssociationRef DataItemRef::toAssociation() {
		assert(get()->GetType() == DataItem::Association);
		return static_cast<DataItemAssociation*>(get());
	}

	inline DataItemArrayRef DataItemRef::toArray() {
		assert(get()->GetType() == DataItem::Array);
		return static_cast<DataItemArray*>(get());
	}


} // namespace

#endif
