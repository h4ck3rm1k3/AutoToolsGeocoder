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
# $Rev: 40 $ 
# $Date: 2006-08-02 18:40:51 +0200 (Wed, 02 Aug 2006) $ 
*/

// DomHelper.h: Helper module augments Xerces DOM, adding useful utility functions.

#ifndef INCL_DOM_HELPER_H
#define INCL_DOM_HELPER_H

#include "auto_ptr_array.h"
#include "TsString.h"
#include "RefPtr.h"
#include "Global_DllExport.h"

#ifdef WIN32
# pragma once
#endif 


#include <xercesc/util/XMLString.hpp>
// Include these to get implicit conversion of DOMDocument* to DOMNode*
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_USE


XERCES_CPP_NAMESPACE_BEGIN
	class DOMElement;
	class DOMImplementation;
	class XercesDOMParser;
XERCES_CPP_NAMESPACE_END


#include <vector>

namespace PortfolioExplorer {

	// We make this a class instead of a collection of methods, because we
	// use an expandable array of XMLCh to handle conversions.

	class DomHelper : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////////
		DomHelper();

		///////////////////////////////////////////////////////////////////////////
		// Destructor
		///////////////////////////////////////////////////////////////////////////
		virtual ~DomHelper();

		///////////////////////////////////////////////////////////////////////////
		// Get the underlying DOM implementation.
		///////////////////////////////////////////////////////////////////////////
	    DOMImplementation* GetDomImplementation();

		///////////////////////////////////////////////////////////////////////////
		// Helper method to add a tag to a DOM node.
		// Returns the added tag
		///////////////////////////////////////////////////////////////////////////
		DOMElement* DomAddElement(
			DOMNode* node,
			const TsString& tag
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to add text to an existing DOM tag
		///////////////////////////////////////////////////////////////////////////
		void DomAddTextToElement(
			DOMNode* node,
			const TsString& value
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to add a tag and text value to a DOM node
		///////////////////////////////////////////////////////////////////////////
		void DomAddElement(
			DOMNode* node,
			const TsString& tag,
			const TsString& value
		);
		// Shortcut to add char*
		void DomAddElement(
			DOMNode* node,
			const TsString& tag,
			const char* value
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to add a tag and integer value to a DOM node
		///////////////////////////////////////////////////////////////////////////
		void DomAddElement(
			DOMNode* node,
			const TsString& tag,
			int value
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to add a tag and boolean value to a DOM document
		///////////////////////////////////////////////////////////////////////////
		void DomAddElement(
			DOMNode* node,
			const TsString& tag,
			bool value
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to add a tag and struct tm value to a DOM document
		///////////////////////////////////////////////////////////////////////////
		void DomAddElement(
			DOMNode* node,
			const TsString& tag,
			const struct tm& value
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper methods to get the first direct child element with tag
		// equal to the one specified.
		// If there is no such tag then a Null DOMNode is returned.
		///////////////////////////////////////////////////////////////////////////
		DOMNode* DomGetFirstChildByTagName(
			DOMNode* node,
			const char* tag
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper methods to get the text of the first direct child element 
		// with tag equal to the one specified.
		// If there is no such tag then the empty string is returned.
		///////////////////////////////////////////////////////////////////////////
		TsString DomGetFirstChildTextByTagName(
			DOMNode* node,
			const char* tag
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper methods to get the integer value of the first direct child element 
		// with tag equal to the one specified.
		// If there is no such tag then 0 is returned.
		///////////////////////////////////////////////////////////////////////////
		int DomGetFirstChildIntegerByTagName(
			DOMNode* node,
			const char* tag
		);


		///////////////////////////////////////////////////////////////////////////
		// Helper methods to get the boolean value of the first direct child element 
		// with tag equal to the one specified.
		// If there is no such tag then false is returned.
		///////////////////////////////////////////////////////////////////////////
		bool DomGetFirstChildBooleanByTagName(
			DOMNode* node,
			const char* tag
		);


		///////////////////////////////////////////////////////////////////////////
		// Helper methods to get the struct tm value of the first direct child element 
		// with tag equal to the one specified.
		// If there is no such tag then an all-zero struct is returned.
		///////////////////////////////////////////////////////////////////////////
		struct tm DomGetFirstChildTimestampByTagName(
			DOMNode* node,
			const char* tag
		);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to get the text of an element.
		// Returns an empty string if no text is present.
		///////////////////////////////////////////////////////////////////////////
		TsString DomGetElementText(DOMNode* node);

		///////////////////////////////////////////////////////////////////////////
		// Helper method to get the tag of an element.
		///////////////////////////////////////////////////////////////////////////
		TsString DomGetElementTag(DOMNode* node);

		///////////////////////////////////////////////////////////////////////////
		// DOM parser method to assist with conversion of text to DOM doc 
		// Returns a null document on failure.
		// DO NOT delete the resulting pointer, it is owned by the helper.
		///////////////////////////////////////////////////////////////////////////
		DOMDocument* DomParseString(
			const char* str,
			TsString& errorMsgReturn
		);

		///////////////////////////////////////////////////////////////////////////
		// Formatting flags for the DomFormatString method
		///////////////////////////////////////////////////////////////////////////
		enum DomFormatFlags {
			DomFormatNone = 0x0,
			DomFormatHeader = 0x1,
			DomFormatPretty = 0x2
		};

		///////////////////////////////////////////////////////////////////////////
		// DOM parser method to assist with conversion of DOM doc to text.
		// Returns an empty string on failure.
		// Inputs:
		//	DOMNode*		node		The dom node to format
		//	int				flags		Flags to control the formatting
		///////////////////////////////////////////////////////////////////////////
		TsString DomFormatString(
			DOMNode* node,
			int flags = DomFormatPretty | DomFormatHeader
		);

		///////////////////////////////////////////////////////////////////////////
		// Convert char to XMLCh.  Returns internal pointer which is
		// valid until next call, or desctruction of DomHelper.
		///////////////////////////////////////////////////////////////////////////
		const XMLCh* ToXMLCh(const char* charStr);

		///////////////////////////////////////////////////////////////////////////
		// Convert TsString to XMLCh.  Returns internal pointer which is
		// valid until next call, or desctruction of DomHelper.
		///////////////////////////////////////////////////////////////////////////
		const XMLCh* ToXMLCh(const TsString& str) {
			return ToXMLCh(str.c_str());
		}

		///////////////////////////////////////////////////////////////////////////
		// Convert XMLCh to char.  Returns internal pointer which is
		// valid until next call, or desctruction of DomHelper.
		///////////////////////////////////////////////////////////////////////////
		const char* ToChar(const XMLCh* xmlChStr);

		///////////////////////////////////////////////////////////////////////////////
		// Get the first DOM node child, ignoring extraneous whitespace.
		// Returns zero if there are no children.
		///////////////////////////////////////////////////////////////////////////////
		DOMNode* DomGetFirstChild(DOMNode* node);

		///////////////////////////////////////////////////////////////////////////////
		// Get the next sibling, ignoring extraneous whitespace.
		// Returns zero if there is no sibling.
		///////////////////////////////////////////////////////////////////////////////
		DOMNode* DomGetNextSibling(DOMNode* node);

		///////////////////////////////////////////////////////////////////////////////
		// Is this node ignorable whitespace?  It seems that the flag is not set by
		// Xerces when we want it to be, so we use the convention that we call a node
		// ignorable if it is a DOMText node with a sibling.  This is not correct
		// in general, but good enough for our purposes.
		///////////////////////////////////////////////////////////////////////////////
		bool DomNodeIsIgnorableWhitespace(DOMNode* node);

		///////////////////////////////////////////////////////////////////////////////
		// Escape newlines in a text string so as to encode it independently of newline
		// convention used in XML storage format.
		///////////////////////////////////////////////////////////////////////////////
		TsString EscapeNewlines(const TsString& str);

		///////////////////////////////////////////////////////////////////////////////
		// Opposite of EscapeNewlines()
		///////////////////////////////////////////////////////////////////////////////
		TsString UnescapeNewlines(const TsString& str);

		///////////////////////////////////////////////////f////////////////////////
		// DOM Node method to remove a child node
		// Returns a null node on failure.
		///////////////////////////////////////////////////////////////////////////
		DOMNode* RemoveFirstChildByTagName(
			DOMNode* node,
			const char* str,
			TsString& errorMsgReturn
		);

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Add whitespace text to the DOM tree so its formats nicely.  This will also
		// remove any existing whitespace.
		// Inputs:
		//	DOMNode*				node		The node to prettify
		///////////////////////////////////////////////////////////////////////////////
		void InsertPrettyNodes(
			DOMNode* node,
			int indentLevel = 0
		);

		///////////////////////////////////////////////////////////////////////////////
		// File the charBuf with a newline and indentation
		///////////////////////////////////////////////////////////////////////////////
		void FillCharBufWithNewlineAndIndent(int indentLevel);

		// Used for conversion from char to XMLCh
		auto_ptr_array<XMLCh> xmlChBuf;
		unsigned xmlChBufSize;

		// Used for conversion from XMLCh to char
		auto_ptr_array<char> charBuf;
		unsigned charBufSize;

		// Implementation object
		DOMImplementation* impl;

		// Parser object
		XercesDOMParser *parser;

		// The number of spaces to use when formatting each indent level
		enum {
			IndentSize = 2
		};

		// The newline sequence
		static const char *newlineSeq;

	};	
}

#endif
