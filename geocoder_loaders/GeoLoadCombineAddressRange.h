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

// GeoLoadCombineAddressRange.h:  Object that rearranges fields.

#ifndef INCL_GeoLoadCombineAddressRange_H
#define INCL_GeoLoadCombineAddressRange_H

#if _MSC_VER >= 1000
#pragma once
#endif


#include <vector>
#include "RecordCopier.h"
#include "MultiDataSource.h"
#include "SingleOutputProxy.h"

namespace PortfolioExplorer {


	///////////////////////////////////////////////////////////////////////////////
	// GeoLoadCombineAddressRange: Class that GeoLoadCombineAddressRanges multiple inputs.
	///////////////////////////////////////////////////////////////////////////////
	class GeoLoadCombineAddressRange : public SingleOutputProxy {
		friend class DataSourceGeoLoadCombineAddressRange;
	public:
		///////////////////////////////////////////////////////////////////////////////
		// constructor
		// Inputs:
		//	ProcessSystemBase*	processSystem	The ProcessSystem object to which
		//										this  belongs.
		//	ListenerRef		listener			The object that will receive messages for
		//										the .
		///////////////////////////////////////////////////////////////////////////////
		GeoLoadCombineAddressRange(
			ProcessSystemBase* processSystem_,
			ListenerRef listener_
		);

		///////////////////////////////////////////////////////////////////////////////
		// Initialize the process node in whatever manner is appropriate
		// for the specific implementation.
		// Return value:
		//	bool	true on success, false on error.  On success, state is
		//			set to StateRunning.  On failure, state is set to StateInitError.
		///////////////////////////////////////////////////////////////////////////////
		virtual bool Init();

		///////////////////////////////////////////////////////////////////////////////
		// Release any resources that were in use during processing.
		///////////////////////////////////////////////////////////////////////////////
		virtual void Cleanup();

		///////////////////////////////////////////////////////////////////////////////
		// Get the identifier of this node kind.  This should be unique among all
		//  classes.
		// Return value;
		//	string		An identifier describing the kind of this node.  This can
		//				be passed to ProcessSystem::Make to create a 
		//				of the given kind, thus supporting "virtual construction".
		//				When used in conjunction with serialized DataItem, it
		//				supports save/load and cut/copy/paste operations.
		///////////////////////////////////////////////////////////////////////////////
		virtual TsString GetNodeId() const
		{ 
			return "GeoLoadCombineAddressRange"; 
		}

		///////////////////////////////////////////////////////////////////////////////
		// List the kinds of inputs that this node accepts.
		// Return value:
		//	InputOutputKindList		A list of InputKinds.  These are not very descriptive,
		//						but provide a means by which the UI can query and choose
		//						the available inputs for a node.
		///////////////////////////////////////////////////////////////////////////////
		virtual InputOutputKindList GetInputKinds() const;

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Process the configuration settings
		// Note: Check HaveConfigurationError() for the result.
		///////////////////////////////////////////////////////////////////////////////
		virtual void ProcessConfiguration();

		// Output record.  Set by ProcessConfiguration().
		// Accessed directly by DataSourceGeoLoadCombineAddressRange.
		RecordRef outputRecord;

		// Performs field-copying from input record to output record.
		// Set by ProcessConfiguration().
		RecordCopierRef recordCopier;

		// Fields to be processed
		TsString postcodeFieldName;
		TsString tlidFieldName;
		TsString leftRightFieldName;
		TsString fraddrFieldName;
		TsString toaddrFieldName;

	};
	typedef refcnt_ptr<GeoLoadCombineAddressRange> GeoLoadCombineAddressRangeRef;

}

#endif
