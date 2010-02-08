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

// ListenerNull.h:  NOP listener object

#ifndef INCL_LISTENERNULL_H
#define INCL_LISTENERNULL_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "Listener.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Interface for listener objects that are passed to various methods.
	// Subclass this and implement the "message" method to receive notification
	// of errors 
	///////////////////////////////////////////////////////////////////////////////
	class ListenerNull : public Listener {
	public:
		ListenerNull() {}
		virtual ~ListenerNull() {}
	private:
		///////////////////////////////////////////////////////////////////////////////
		// OutputMessage: Override this method to receive messages
		// Inputs:
		//	const string&	message			Text of the message.
		//	Category		category		Category of the message.
		///////////////////////////////////////////////////////////////////////////////
		virtual void OutputMessage(
			const TsString& message,
			Category category
		) {
			// NOP
		}
	};


} // namespace

#endif

