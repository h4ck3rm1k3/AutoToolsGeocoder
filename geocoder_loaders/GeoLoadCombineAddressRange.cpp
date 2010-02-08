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
# $Rev: 41 $ 
# $Date: 2006-08-02 18:53:16 +0200 (Wed, 02 Aug 2006) $ 
*/

// GeoLoadCombineAddressRange.cpp:  Object that rearranges fields.

#define _WIN32_WINNT 0x5000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>


// this loader seems to not be used in the Geocoder Load process...
#if 0
#include "GeoLoadCombineAddressRange.h"
#include "DataSourceGeoLoadCombineAddressRange.h"
#include "BitStreamAdaptor.h"
namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// GeoLoadCombineAddressRange: Class that GeoLoadCombineAddressRanges multiple inputs.
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// constructor
	// Inputs:
	//	ProcessSystemBase*	processSystem	The ProcessSystem object to which
	//										this  belongs.
	//	ListenerRef		listener			The object that will receive messages for
	//										the .
	///////////////////////////////////////////////////////////////////////////////
	GeoLoadCombineAddressRange::GeoLoadCombineAddressRange(
		ProcessSystemBase* processSystem_,
		ListenerRef listener_
	) :
		SingleOutputProxy(
			processSystem_, 
			listener_
		)
	{
		SetDataSource(new DataSourceGeoLoadCombineAddressRange(this, "Output", listener_));
		configChanged = true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Initialize the process node in whatever manner is appropriate
	// for the specific implementation.
	// Return value:
	//	bool	true on success, false on error.  On success, state is
	//			set to StateRunning.  On failure, state is set to StateInitError.
	///////////////////////////////////////////////////////////////////////////////
	bool GeoLoadCombineAddressRange::Init()
	{
		// dataSource will be initialized MultiDataSource::Init()

		// Process the configuration settings, which will set up the recordCopier.
		ProcessConfiguration();
		if (haveConfigError) {
			state = StateInitError;
			return false;
		}

		// All systems go.
		state = StateRunning;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Release any resources that were in use during processing.
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadCombineAddressRange::Cleanup()
	{
		if (state == StateRunning) {
			state = StateFinishOK;
		}

		// DataSource will be cleaned up by MultiDataSource::Cleanup()

		// Don't delete these; they are created by ProcessConfiguration()
		// outputRecord = 0;
		// recordCopier = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// List the kinds of inputs that this node accepts.
	// Return value:
	//	InputOutputKindList		A list of InputKinds.  These are not very descriptive,
	//						but provide a means by which the UI can query and choose
	//						the available inputs for a node.
	///////////////////////////////////////////////////////////////////////////////
	::InputOutputKindList GeoLoadCombineAddressRange::GetInputKinds() const
	{
		InputOutputKindList inputKinds;
		// Multiple inputs are not allowed
		inputKinds.push_back(InputOutputKind(true, "Input"));
		return inputKinds;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Process the configuration settings.  This will set up the recordCopier.
	// Note: Check HaveConfigurationError() for the result.
	///////////////////////////////////////////////////////////////////////////////
	void GeoLoadCombineAddressRange::ProcessConfiguration()
	{
		if (!configChanged) {
			return;
		}
		::ProcessConfiguration();

		// Output record starts out empty
		outputRecord = new Record;

		// Make a new record copier
		recordCopier = new RecordCopier;

		// Must have an output
		DataSourceList outputs = GetOutputs();
		if (outputs.size() == 0) {
			ConfigError("Must have at least one output attached");
		}

		// Get references to all inputs.
		DataSourceRef input = GetFirstInput();
		if (input == 0) {
			ConfigError("Must have at least one input attached");
			return;
		}

		// Output is always copy of input record schema
		outputRecord = new Record(*input->GetRecord());

		// Copy entire record; this is only very slightly wasteful of CPU.
		recordCopier->AddRecordTransfers(outputRecord);

		// Configuration processing.  Walk the DataItem hierarchy and
		// transform that into the data-file, file format, and record layout.

		DataItemRef tmp;

		///////////////////////////////////////////////////////////////////////////////
		// Specified fields
		///////////////////////////////////////////////////////////////////////////////

		postcodeFieldName = "";
		tlidFieldName = "";
		leftRightFieldName = "";
		fraddrFieldName = "";
		toaddrFieldName = "";

		tmp = config["ZIP"];
		if (tmp != 0) {
			postcodeFieldName = TsString(*tmp);
		}
		if (outputRecord->GetField(postcodeFieldName) == 0) {
			ConfigError("ZIP field '" + postcodeFieldName + "' does not exist on input record");
		}

		tmp = config["TLID"];
		if (tmp != 0) {
			tlidFieldName = TsString(*tmp);
		}
		if (outputRecord->GetField(tlidFieldName) == 0) {
			ConfigError("TLID field '" + tlidFieldName + "' does not exist on input record");
		}

		tmp = config["LEFTRIGHT"];
		if (tmp != 0) {
			leftRightFieldName = TsString(*tmp);
		}
		if (outputRecord->GetField(leftRightFieldName) == 0) {
			ConfigError("LEFTRIGHT field '" + leftRightFieldName + "' does not exist on input record");
		}

		tmp = config["FRADDR"];
		if (tmp != 0) {
			fraddrFieldName = TsString(*tmp);
		}
		if (outputRecord->GetField(fraddrFieldName) == 0) {
			ConfigError("FRADDR field '" + fraddrFieldName + "' does not exist on input record");
		}

		tmp = config["TOADDR"];
		if (tmp != 0) {
			toaddrFieldName = TsString(*tmp);
		}
		if (outputRecord->GetField(toaddrFieldName) == 0) {
			ConfigError("TOADDR field '" + toaddrFieldName + "' does not exist on input record");
		}
	}

}	// namespace
#endif
