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
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

// DomHelper.cpp: Helper module augments Xerces DOM, adding useful utility functions.

#include "Global_Headers.h"
#include "DomHelper.h"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <time.h>
#include <stdio.h>

XERCES_CPP_NAMESPACE_USE

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////
	// Formatter class used by DomFormatString()
	///////////////////////////////////////////////////////////////////////////
	class StringFormatter : public XMLFormatTarget {
	public:
		virtual void writeChars(
			const XMLByte* const toWrite,
			const unsigned int count,
			XMLFormatter * const formatter
		) {
			// Decrease the number of re-allocations.
			if (result.capacity() < result.size() + count + 2) {
				result.reserve((int)(result.capacity() * 1.5) + 10);
			}
			// Append text to result.
			result.append((char*)toWrite, count);
		}

		TsString GetResult() const { return result; }
	private:
		TsString result;
	};



	///////////////////////////////////////////////////////////////////////////
	// Node filter class used by DomFormatString()
	///////////////////////////////////////////////////////////////////////////
	class FormatFilter : public DOMWriterFilter {
	public:
		// Is the node acceptable for output?
		virtual short acceptNode(const DOMNode *node) const {
			switch (node->getNodeType()) {
			case DOMNode::TEXT_NODE:
				// Discard text siblings of elements.
				// They are never used in this application, and are whitespace.

				//if (static_cast<const DOMText*>(node)->isIgnorableWhitespace()) {
				if (			
					node->getNodeType() == DOMNode::TEXT_NODE &&
					node->getParentNode() != 0 &&
					node->getParentNode()->getFirstChild()->getNextSibling() != 0
				) {
					return FILTER_REJECT;
				}
				break;

			}
			return FILTER_ACCEPT;
		}

		// Tells the DOMWriter what types of nodes to show to the filter.
		virtual unsigned long getWhatToShow() const {
			return SHOW_ALL;
		}
 
		// Set what types of nodes are to be presented. No-op..
		virtual void setWhatToShow(unsigned long) {}
 	private:
	};

	///////////////////////////////////////////////////////////////////////////
	// Constructor
	///////////////////////////////////////////////////////////////////////////
	DomHelper::DomHelper() :
		xmlChBufSize(0),
		charBufSize(0),
		impl(0),
		parser(0)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////
	DomHelper::~DomHelper()
	{
		if (parser != 0) {
			delete parser;
			parser = 0;
		}
	}


	///////////////////////////////////////////////////////////////////////////
	// Get the underlying DOM implementation.
	///////////////////////////////////////////////////////////////////////////
	DOMImplementation* DomHelper::GetDomImplementation()
	{
		if (impl == 0) {
			impl = DOMImplementationRegistry::getDOMImplementation(ToXMLCh("LS"));
		}
		return impl; 
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper method to add a tag to a DOM node.
	// Returns the added tag
	///////////////////////////////////////////////////////////////////////////
	DOMElement* DomHelper::DomAddElement(
		DOMNode* node,
		const TsString& tag
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return 0;
		}
		DOMDocument* doc = node->getOwnerDocument();
		DOMElement* child = doc->createElement(ToXMLCh(tag));
		node->appendChild(child);
		return child;
	}


	///////////////////////////////////////////////////////////////////////////
	// Helper method to add text to an existing DOM tag
	///////////////////////////////////////////////////////////////////////////
	void DomHelper::DomAddTextToElement(
		DOMNode* node,
		const TsString& value
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return;
		}
		DOMDocument* doc = node->getOwnerDocument();
		node->appendChild(doc->createTextNode(ToXMLCh(value)));
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper method to add a tag and text value to a DOM document
	///////////////////////////////////////////////////////////////////////////
	void DomHelper::DomAddElement(
		DOMNode* node,
		const TsString& tag,
		const TsString& value
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return;
		}
		DOMDocument* doc = node->getOwnerDocument();
		DOMElement* child = doc->createElement(ToXMLCh(tag));
		node->appendChild(child);
		child->appendChild(doc->createTextNode(ToXMLCh(value)));
	}


	///////////////////////////////////////////////////////////////////////////
	// Shortcut to add char*
	///////////////////////////////////////////////////////////////////////////
	void DomHelper::DomAddElement(
		DOMNode* node,
		const TsString& tag,
		const char* value
	) {
		DomAddElement(node, tag, TsString(value));
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper method to add a tag and integer value to a DOM document
	///////////////////////////////////////////////////////////////////////////
	void DomHelper::DomAddElement(
		DOMNode* node,
		const TsString& tag,
		int value
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return;
		}
		DOMDocument* doc = node->getOwnerDocument();
		DOMElement* child = doc->createElement(ToXMLCh(tag));
		node->appendChild(child);
		char buf[32];
		sprintf(buf, "%d", value);
		child->appendChild(doc->createTextNode(ToXMLCh(buf)));
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper method to add a tag and boolean value to a DOM document
	///////////////////////////////////////////////////////////////////////////
	void DomHelper::DomAddElement(
		DOMNode* node,
		const TsString& tag,
		bool value
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return;
		}
		DOMDocument* doc = node->getOwnerDocument();
		DOMElement* child = doc->createElement(ToXMLCh(tag));
		node->appendChild(child);
		child->appendChild(doc->createTextNode(ToXMLCh(value ? "YES" : "NO" )));
	}


	///////////////////////////////////////////////////////////////////////////
	// Helper method to add a tag and struct tm value to a DOM document
	///////////////////////////////////////////////////////////////////////////
	void DomHelper::DomAddElement(
		DOMNode* node,
		const TsString& tag,
		const struct tm& value
	) {
		struct tm temp(value);
		DomAddElement(node, tag, (int)mktime(&temp));
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper methods to get the first direct child element with tag
	// equal to the one specified.
	///////////////////////////////////////////////////////////////////////////
	DOMNode *DomHelper::DomGetFirstChildByTagName(
		DOMNode* node,
		const char* nodeName
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return 0;
		}
        for (
			DOMNode* child = node->getFirstChild();
			child != 0;
			child = child->getNextSibling()
		) {
			if (
				child->getNodeType() == DOMNode::ELEMENT_NODE &&
				XMLString::compareString(child->getNodeName(), ToXMLCh(nodeName)) == 0
			) {
				return child;
			}
		}
		return 0;
	}


	///////////////////////////////////////////////////////////////////////////
	// Helper methods to get the text of the first direct child element 
	// with tag equal to the one specified.
	// If there is no such tag then the empty string is returned.
	///////////////////////////////////////////////////////////////////////////
	TsString DomHelper::DomGetFirstChildTextByTagName(
		DOMNode* node,
		const char* tag
	) {
		DOMNode* child = DomGetFirstChildByTagName(node, tag);
		return child == 0 ? "" : DomGetElementText(child);
	}


	///////////////////////////////////////////////////////////////////////////
	// Helper methods to get the integer value of the first direct child element 
	// with tag equal to the one specified.
	// If there is no such tag then 0 is returned.
	///////////////////////////////////////////////////////////////////////////
	int DomHelper::DomGetFirstChildIntegerByTagName(
		DOMNode* node,
		const char* tag
	) {
		DOMNode* child = DomGetFirstChildByTagName(node, tag);
		return child == 0 ? 0 : atoi(DomGetElementText(child).c_str());
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper methods to get the boolean value of the first direct child element 
	// with tag equal to the one specified.
	// If there is no such tag then 0 is returned.
	///////////////////////////////////////////////////////////////////////////
	bool DomHelper::DomGetFirstChildBooleanByTagName(
		DOMNode* node,
		const char* tag
	) {
		DOMNode* child = DomGetFirstChildByTagName(node, tag);
		if (child == 0) {
			return false;
		}
		TsString tmp = DomGetElementText(child);
		return 
			tmp[0] == 'Y' ||
			tmp[0] == 'y' ||
			tmp[0] == '1' ||
			tmp[0] == 'T' ||
			tmp[0] == 't';
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper methods to get the struct tm value of the first direct child element 
	// with tag equal to the one specified.
	// If there is no such tag then an all-zero struct is returned.
	///////////////////////////////////////////////////////////////////////////
	struct tm DomHelper::DomGetFirstChildTimestampByTagName(
		DOMNode* node,
		const char* tag
	) {
		struct tm returnValue;
		time_t tval = DomGetFirstChildIntegerByTagName(node, tag);
		if (tval == 0) {
			memset(&returnValue, 0, sizeof(returnValue));
		} else {
			struct tm *timePtr = localtime(&tval);
			if (timePtr == 0) {
				memset(&returnValue, 0, sizeof(returnValue));
			} else {
				returnValue = *timePtr;
			}
		}
		return returnValue;
	}

	///////////////////////////////////////////////////////////////////////////
	// Helper method to get the text of an element.
	// Returns an empty string if no text is present.
	///////////////////////////////////////////////////////////////////////////
	TsString DomHelper::DomGetElementText(DOMNode* node)
	{
		if (node == 0) {
			return 0;
		}
        for (
			DOMNode* child = node->getFirstChild();
			child != 0;
			child = child->getNextSibling()
		) {
			if (child->getNodeType() == DOMNode::TEXT_NODE) {
				return ToChar(child->getNodeValue());
			}
		}
		return "";
	}


	///////////////////////////////////////////////////////////////////////////
	// Helper method to get the tag of an element.
	///////////////////////////////////////////////////////////////////////////
	TsString DomHelper::DomGetElementTag(DOMNode* node)
	{
		if (node == 0) {
			return 0;
		}
		return ToChar(node->getNodeName());
	}

	///////////////////////////////////////////////////////////////////////////
	// DOM parser method to assist with conversion of text to DOM doc 
	// Returns a null document on failure.
	// DO NOT delete the returned DOMDocument.
	///////////////////////////////////////////////////////////////////////////
	DOMDocument* DomHelper::DomParseString(
		const char* str,
		TsString& errorMsgReturn
	) {
		if (str == 0) {
			return 0;
		}
		if (parser == 0) {
			// Make a parser if we don't already have one.
			parser = new XercesDOMParser;
			parser->setIncludeIgnorableWhitespace(false);
		}

		// Adapt buffer to a form that can be read by the parser.
		MemBufInputSource inputSource(
			(const XMLByte *)str,
			unsigned(strlen(str)),
			"DataLever"		// fake "system ID"
		);

		bool haveError = false;
		TsString errorMsg;

		try {
			parser->parse(inputSource);
		}
        catch (const SAXException& e) {
			auto_ptr_array<char> ptr(XMLString::transcode(e.getMessage()));
			errorMsgReturn = ptr;
            haveError = true;
		}
        catch (const XMLException& e) {
			auto_ptr_array<char> ptr(XMLString::transcode(e.getMessage()));
			errorMsgReturn = ptr;
            haveError = true;
        }
        catch (const DOMException&)
        {
			errorMsgReturn= "DOM Parsing Error";
            haveError = true;
        }

		if (haveError) {
			// Null document
			return 0;
		} else {
			return parser->getDocument();
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// DOM parser method to assist with conversion of DOM doc to text.
	// Returns an empty string on failure.
	// Inputs:
	//	DOMNode*		node		The dom node to format
	//	int				flags		Flags to control the formatting
	///////////////////////////////////////////////////////////////////////////
	TsString DomHelper::DomFormatString(
		DOMNode* node,
		int flags
	) {
		DOMWriter *domWriter = GetDomImplementation()->createDOMWriter();

		domWriter->setNewLine(ToXMLCh("\n"));

		if ((flags & DomFormatPretty) != 0) {
			// Insert whitespace nodes to make it pretty
			InsertPrettyNodes(node);
			// Also do this to get a newline after the header.
			// On second thought, it seems that xerces in 2.2.0 this makes text-only
			// nodes go away.
//			if (domWriter->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
//				domWriter->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
//			}
		}

/*
		if (domWriter->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, true)) {
			domWriter->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, true);
		}
*/

		if ((flags & DomFormatHeader) == 0) {
			// Header is not wanted.
			// The only way JEL knows to get rid of this is to format the 
			// root element of the document instead of the document itself.
			if (node->getNodeType() == DOMNode::DOCUMENT_NODE) {
				 node = ((DOMDocument*)node)->getDocumentElement();
			}
		}

		FormatFilter filter;
		if ((flags & DomFormatPretty) == 0) {
			// Filter out whitespace if not prettifying
			domWriter->setFilter(&filter);
		}

		StringFormatter formatter;
		domWriter->writeNode(&formatter, *node);

		TsString retval = formatter.GetResult();

		delete domWriter;
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////
	// Convert XMLCh to char.  Returns internal pointer which is
	// valid until next call, or desctruction of DomHelper.
	///////////////////////////////////////////////////////////////////////////
	const XMLCh* DomHelper::ToXMLCh(const char* charStr)
	{
		// Make enough room in buffer
		if (strlen(charStr)+1 > xmlChBufSize) {
			xmlChBufSize = unsigned(strlen(charStr)+1);
			xmlChBuf = new XMLCh[xmlChBufSize];
		}
		XMLCh* xmlChPtr = xmlChBuf.get();

		// TODO: This may not be exactly right, but XMLString does not
		// document any transcode() going from native code page to
		// Unicode.  This will work for ASCII char range of course.
		while (*charStr != 0) {
			*xmlChPtr++ = *charStr++;
		}
		*xmlChPtr = 0;
		return xmlChBuf.get();
	}

	///////////////////////////////////////////////////////////////////////////
	// Convert char to XMLCh.  Returns internal pointer which is
	// valid until next call, or desctruction of DomHelper.
	///////////////////////////////////////////////////////////////////////////
	const char* DomHelper::ToChar(const XMLCh* xmlChStr)
	{
		// Leave lots of room in the char buffer.
		if (XMLString::stringLen(xmlChStr)*4 + 1 > charBufSize) {
			charBufSize = XMLString::stringLen(xmlChStr)*4 + 1;
			charBuf = new char[charBufSize];
		}
		if (XMLString::transcode(xmlChStr, charBuf.get(), charBufSize-1)) {
			return charBuf.get();
		} else {
			return "";
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// Get the first DOM node child, ignoring extraneous whitespace.
	// Returns zero if there are no children.
	///////////////////////////////////////////////////////////////////////////////
	DOMNode* DomHelper::DomGetFirstChild(DOMNode* node)
	{
		DOMNode* child;
		for (
			child = node->getFirstChild();
			child != 0;
			child = child->getNextSibling()
		) {
			if (!DomNodeIsIgnorableWhitespace(child)) {
				break;
			}
		}
		return child;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the next sibling , ignoring extraneous whitespace.
	// Returns zero if there is no sibling.
	///////////////////////////////////////////////////////////////////////////////
	DOMNode* DomHelper::DomGetNextSibling(DOMNode* node)
	{
		DOMNode* child;
		for (
			child = node->getNextSibling();
			child != 0;
			child = child->getNextSibling()
		) {
			if (!DomNodeIsIgnorableWhitespace(child)) {
				break;
			}
		}
		return child;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Is this node ignorable whitespace?  It seems that the flag is not set by
	// Xerces when we want it to be, so we use the convention that we call a node
	// ignorable if it is a DOMText node with a sibling.  This is not correct
	// in general, but good enough for our purposes.
	///////////////////////////////////////////////////////////////////////////////
	bool DomHelper::DomNodeIsIgnorableWhitespace(DOMNode* node)
	{
		return
			node != 0 &&
			node->getNodeType() == DOMNode::TEXT_NODE &&
			node->getParentNode() != 0 &&
			node->getParentNode()->getFirstChild()->getNextSibling() != 0;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Escape newlines in a text string so as to encode it independently of newline
	// convention used in XML storage format.
	///////////////////////////////////////////////////////////////////////////////
	TsString DomHelper::EscapeNewlines(const TsString& str) {
		TsString retval;
		retval.reserve(str.size() + 5);	// for performance
		for (unsigned i = 0; i < str.size(); i++) {
			switch (str[i]) {
			case '\r':
				retval += "\\r";
				break;
			case '\n':
				retval += "\\n";
				break;
			case '\\':
				retval += "\\\\";
				break;
			default:
				retval += str[i];
				break;
			}
		}
		return retval;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Opposite of EscapeNewlines()
	///////////////////////////////////////////////////////////////////////////////
	TsString DomHelper::UnescapeNewlines(const TsString& str) {
		TsString retval;
		retval.reserve(str.size());	// for performance
		for (unsigned i = 0; i < str.size(); i++) {
			if (str[i] == '\\' && i < str.size()-1) {
				// Unescape the char
				switch (str[i+1]) {
				case 'r':
					retval += '\r';
					i++;
					break;
				case 'n':
					retval += '\n';
					i++;
					break;
				case '\\':
					retval += '\\';
					i++;
					break;
				default:
					retval += str[i];
					break;
				}
			} else {
				retval += str[i];
			}
		}
		return retval;
	
	}

	///////////////////////////////////////////////////////////////////////////////
	// Add whitespace text to the DOM tree so its formats nicely.  This will also
	// remove any existing whitespace.
	// Inputs:
	//	DOMNode*				node		The node to prettify
	///////////////////////////////////////////////////////////////////////////////
	void DomHelper::InsertPrettyNodes(
		DOMNode* node,
		int indentLevel
	) {
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0 || node->getParentNode() == 0) {
			return;
		}

		// Remove the node if it is ignorable.
		while (DomNodeIsIgnorableWhitespace(node)) {
			DOMNode* sibling = node->getNextSibling();
			node->getParentNode()->removeChild(node);
			node = sibling;
		}

		if (node == 0) {
			return;
		}

		// Before an element node, insert a newline and indentation.
		if (
			node->getNodeType() == DOMNode::ELEMENT_NODE &&
			indentLevel != 0
		) {
			FillCharBufWithNewlineAndIndent(indentLevel);
			DOMNode* newNode = node->getOwnerDocument()->createTextNode(ToXMLCh(charBuf));
			node->getParentNode()->insertBefore(newNode, node);
		}

		// Walk child nodes (will recursively walk siblings of child)
		InsertPrettyNodes(node->getFirstChild(), indentLevel+1);

		// Insert a newline after the last child (before the close of this element tag),
		// but only if the node contains an element

		if (
			node->getNodeType() == DOMNode::ELEMENT_NODE
		) {
			for (
				DOMNode* child = node->getFirstChild();
				child != 0;
				child = child->getNextSibling()
			) {
				if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
					FillCharBufWithNewlineAndIndent(indentLevel);
					DOMNode* newNode = node->getOwnerDocument()->createTextNode(ToXMLCh(charBuf));
					node->appendChild(newNode);
					break;
				}
			}
		}

		// Walk the sibling nodes
		InsertPrettyNodes(node->getNextSibling(), indentLevel);
	}


	///////////////////////////////////////////////////////////////////////////////
	// File the charBuf with a newline and indentation
	///////////////////////////////////////////////////////////////////////////////
	void DomHelper::FillCharBufWithNewlineAndIndent(int indentLevel) {
		if (charBufSize < unsigned(indentLevel*IndentSize + 3)) {
			charBufSize = indentLevel*IndentSize + 3;
			charBuf = new char[charBufSize];
		}
		strcpy(charBuf, "\n");
		int i;
		for (i = 1; i <= indentLevel*IndentSize; i++) {
			charBuf[i] = ' ';
		}
		charBuf[i] = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// DOM Node method to remove a child node
	// Returns a null node on failure.
	///////////////////////////////////////////////////////////////////////////
	DOMNode* DomHelper::RemoveFirstChildByTagName(
		DOMNode* node,
		const char* str,
		TsString& errorMsgReturn
	) {
		if (str == 0) {
			return 0;
		}
		if (node != 0 && node->getNodeType() == DOMNode::DOCUMENT_NODE) {
			node = static_cast<DOMDocument*>(node)->getDocumentElement();
		}
		if (node == 0) {
			return 0;
		}

		TsString errorMsg;
		DOMNode* childNode = DomGetFirstChildByTagName(node, str);
		if( childNode == 0 ) {
			return 0;
		}
		DOMNode* returnNode = NULL;
		try {
			returnNode = node->removeChild(childNode);
		}
        catch (const DOMException&)
        {
			errorMsgReturn= "DOM Parsing Error";
        }
        return returnNode;
	}

}
