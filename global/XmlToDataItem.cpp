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
// XMLToDataItem.cpp: Conversion of XML to DataItem hierarchy
///////////////////////////////////////////////////////////////////////////////

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "Global_Headers.h"
#include "XmlToDataItem.h"
#include "DomHelper.h"
#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_USE

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Create a DataItem hierarchy from the DOM node.
	// Return value:
	//	DataItemRef		The resulting dataItem element, or zero if the XML does
	//					not define a valid DataItem hierarchy
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef XMLToDataItem::MakeDataItem()
	{
		return WalkNode(theDomNode);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Walk the hierarchy starting at this node
	// Inputs:
	//	DOMNode*		domNode			The DOM node to convert
	// Return value:
	//	DataItemRef		The resulting dataItem element, or zero if the XML does
	//					not define a valid DataItem hierarchy
	///////////////////////////////////////////////////////////////////////////////
	DataItemRef XMLToDataItem::WalkNode(
		DOMNode* domNode
	) {
		if (domNode == 0) {
			return 0;
		}
		// Determine what kind of element this is.  This will result in
		// either a DataItemAssociation, DataItemArray, or DataItemValue.
		if (domNode->getNodeType() == DOMNode::DOCUMENT_NODE) {
			return WalkNode(
				static_cast<DOMDocument*>(domNode)->getDocumentElement()
			);
		} else if (domNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			// The contents of the element determine what kind of thing it is:
//TsString tag = helper.DomGetElementTag(domNode);
			DOMNode* child = helper.DomGetFirstChild(domNode);
			if (child == 0) {
				// Contains no children.  Treat as a blank text value.
				return new DataItemValue("");
			}
			if (child->getNodeType() == DOMNode::TEXT_NODE) {
				// Contains text; this must be a value node.
				TsString textStr = helper.UnescapeNewlines(
					helper.ToChar(child->getNodeValue())
				);
				return new DataItemValue(textStr);
			}
			if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
				TsString tag = helper.ToChar(child->getNodeName());
				if (tag == "ELEMENT" || tag == "NOELEMENT") {
					// If it contains <ELEMENT> or <NOELEMENT> tags, it is an array...
					DataItemArrayRef retval = new DataItemArray;
					if (tag == "ELEMENT") {
						// ... and the array has elements
						// Walk the children and add them to the array.
						for (; child != 0; child = helper.DomGetNextSibling(child)) {
							DataItemRef childItem = WalkNode(child);
							if (childItem != 0) {
								retval->AppendItem(childItem);
							}
						}
					}
					return retval.get();
				} else {
					// If it contains other tags, it is a dictionary
					DataItemAssociationRef retval = new DataItemAssociation;
					// Walk the children and add them to the association
					for (; child != 0; child = helper.DomGetNextSibling(child)) {
						DataItemRef childItem = WalkNode(child);
						if (childItem != 0) {
							retval->AddItem(
								helper.ToChar(child->getNodeName()),
								childItem
							);
						}
					}
					return retval.get();
				}
			}
		}
		// Nothing else is valid in the context of DataItem conversion.
		// Who knows what this is?
		return 0;
	}


} // namespace 


