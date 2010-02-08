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

///////////////////////////////////////////////////////////////////////////////
// XMLToDataItem.h: Conversion of XML to DataItem hierarchy
///////////////////////////////////////////////////////////////////////////////

#ifndef INCL_XMLToDataItem_H
#define INCL_XMLToDataItem_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "Global_DllExport.h"
#include "DataItem.h"
#include "DomHelper.h"
#include "Global_DllExport.h"

#include <xercesc/util/XercesDefs.hpp>
XERCES_CPP_NAMESPACE_BEGIN
	class DOMDocument;
	class DOMNode;
XERCES_CPP_NAMESPACE_END
XERCES_CPP_NAMESPACE_USE


namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Conversion of XML to DataItem hierarchy
	///////////////////////////////////////////////////////////////////////////////
	class XMLToDataItem : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor
		// Inputs:
		//	DOM_Node*		domNode		The DOM node to convert from XML.
		//
		// include <xercesc/dom/DOM.hpp> to get the full class definition.
		// include "DomHelper.h" for useful Dom utility functions including formatting.
		///////////////////////////////////////////////////////////////////////////////
		XMLToDataItem(DOMNode* domNode_) : theDomNode(domNode_) {}

		///////////////////////////////////////////////////////////////////////////////
		// Create a DataItem hierarchy from the DOM node.
		///////////////////////////////////////////////////////////////////////////////
		DataItemRef MakeDataItem();

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Walk the hierarchy starting at this node
		// Inputs:
		//	DOMNode*		domNode			The DOM node to convert
		// Return value:
		//	DataItemRef		The resulting dataItem element
		///////////////////////////////////////////////////////////////////////////////
		DataItemRef WalkNode(
			DOMNode* domNode
		);

		// The DOM node to convert
		DOMNode* theDomNode;

		// Helper object
		DomHelper helper;
	};
	typedef refcnt_ptr<XMLToDataItem> XMLToDataItemRef;
	
} // namespace 

#endif
