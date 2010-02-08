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

//////////////////////////////////////////////////////////////////////
// RegularExprWrapper.cpp:  Class that implements parsing of lines
// into possible interpretations.
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "Global_Headers.h"
#include "RegularExprWrapper.h"
#include "XmlToDataItem.h"
#include "DomHelper.h"
#include "ListenerNull.h"
#include "Filesys.h"
#include <xercesc/dom/DOMDocument.hpp>

XERCES_CPP_NAMESPACE_USE

namespace PortfolioExplorer {

	//////////////////////////////////////////////////////////////////////
	// Initialize the parser.
	// Inputs:
	//	const TsString&	dataDir		The directory containing data files.
	// Outputs:
	//	std::String&		errorMsg	The message if an error occured.
	// Return value:
	//	bool	true on success, false on error.
	//////////////////////////////////////////////////////////////////////
	bool RegularExprWrapper::ReadPatternToolConfiguration(
		const TsString& patternsConfigurationFile,
		TsString& errorMsg
	) {
		// Deserialize a configuration
		TsString configStr;
		TsString errorMsg2;
		if (!FileSys::ReadFileIntoString(patternsConfigurationFile, configStr, errorMsg2)) {
			errorMsg = errorMsg2;
			return false;
		}
		
		// New-style XML configuration
		DomHelper helper;
		DOMDocument* doc = helper.DomParseString(configStr.c_str(), errorMsg2);
		if (doc == 0) {
			errorMsg = errorMsg2;
			return false;
		}
		XMLToDataItem convertor(doc);
		DataItemRef config = convertor.MakeDataItem();
		if (config == 0) {
			errorMsg = "Error reading address pattern file.";
			return false;
		}

		// Get all process node configs and walk them
		DataItemRef processNodeConfigs = config["PROCESS_NODES"];
		if (processNodeConfigs == 0 || processNodeConfigs->GetType() != DataItem::Array) {
			// ProcessNodes config must exist even if it is empty
			errorMsg = "Error reading configuration file.";
			return false;
		}

		ListenerRef configListener = new ListenerNull;
		// Loop over all process nodes
		for (
			int nodeIdx = 0; 
			nodeIdx < processNodeConfigs.toArray()->GetSize(); 
			nodeIdx++
		) {
			DataItemRef nodeMetaConfig = (*processNodeConfigs.toArray())[nodeIdx];
			if (nodeMetaConfig->GetType() != DataItem::Association) {
				errorMsg = "Error reading configuration file.";
				return false;
			}
			DataItemRef nodeConfig = nodeMetaConfig["CONFIG"];
			if (nodeConfig == 0) {
				errorMsg = "Error reading configuration file.";
				return false;
			}

			DataItemRef nodeId = nodeMetaConfig["ID"];
			if (nodeId == 0) {
				errorMsg = "Error reading configuration file.";
				return false;
			}
			if( TsString(*nodeId) == "ProcessNodeTokenizer" ) {
				tokenizer = new Tokenizer();
				if (!tokenizer->Bind(nodeConfig , configListener)) {
					errorMsg = "Error reading configuration file.";
					return false;
				}
			}
			if( TsString(*nodeId) == "ProcessNodeSymbolizer" ) {
				symbolizer = new Symbolizer();
				if (!symbolizer->Bind(nodeConfig , configListener)) {
					errorMsg = "Error reading configuration file.";
					return false;
				}
			}
			if( TsString(*nodeId) == "ProcessNodePatternMatch" ) {
				patternMatcher = new PatternMatcher();
				if (!patternMatcher->Bind(nodeConfig , configListener)) {
					errorMsg = "Error reading configuration file.";
					return false;
				}
			}
		}

		if( tokenizer == 0 || symbolizer == 0 || patternMatcher == 0 ) {
			return false;
		}
		
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// Produce tokens and add them to the result vector
	//////////////////////////////////////////////////////////////////////
	void RegularExprWrapper::ProduceTokens(
		char const* value, 
		VectorNoDestruct<TokSymCls>& result, 
		BulkAllocatorRef bulkAllocator
	) {
		
		// Turn the raw input into tokens.
		assert(Open());
		tokens.clear();
		tokenizer->Process(value, tokens, bulkAllocator);

		result.clear();
		std::vector<const char*>::const_iterator it;
		for(it = tokens.begin(); it != tokens.end(); it++) {
			TokSymCls& tmp = result.UseExtraOnEnd();
			tmp.token = (*it);
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Produce symbols and add them to the result vector
	//////////////////////////////////////////////////////////////////////
	void RegularExprWrapper::ProduceSymbols(
		VectorNoDestruct<TokSymCls>& result, 
		BulkAllocatorRef bulkAllocator
	) {
		assert(Open());
		
		tokens.clear();
		symbols.clear();

		std::vector<TokSymCls>::iterator it;
		for(it = result.begin(); it < result.end(); it++) {
			tokens.push_back(
				(char*)bulkAllocator->NewString(
				(const char*)(*it).token.c_str())
			);
		}
		symbolizer->Process(tokens, symbols, bulkAllocator);
		assert(result.size() == symbols.size());

		for(unsigned i = 0; i < symbols.size(); i++) {
			result[i].symbol = symbols[i];
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Produce classes and add them to the result vector
	//////////////////////////////////////////////////////////////////////
	void RegularExprWrapper::ProduceClasses(
		VectorNoDestruct<TokSymCls>& result, 
		BulkAllocatorRef bulkAllocator
	) {
		assert(Open());

		uSymbols.clear();
		classes.clear();
		std::vector<TokSymCls>::iterator it;
		for(it = result.begin(); it < result.end(); it++) {
			uSymbols.push_back(
				(unsigned char*)bulkAllocator->NewString(
				(const char*)(*it).symbol.c_str())
			);
		}
		patternMatcher->Process(uSymbols, classes, bulkAllocator);
		assert(result.size() == classes.size());

		for(unsigned i = 0; i < classes.size(); i++) {
			result[i].token_class = classes[i];
		}
	}

} //namespace
