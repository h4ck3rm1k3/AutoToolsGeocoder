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

// Listener.h:  Class that receives error messages

#ifndef INCL_LISTENER_H
#define INCL_LISTENER_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "TsString.h"
#include "RefPtr.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	class Listener;
	typedef refcnt_ptr<Listener> ListenerRef;

	///////////////////////////////////////////////////////////////////////////////
	// Interface for listener objects that are passed to various methods.
	// Subclass this and implement the "message" method to receive notification
	// of errors 
	///////////////////////////////////////////////////////////////////////////////
	class Listener : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Trace: Shortcut for informational notification
		// Inputs:
		//	const string&		message		Text of the message.  Do not alter or delete!
		///////////////////////////////////////////////////////////////////////////////
		void Trace(
			const TsString& message
		) {
			OutputMessage(message, InformationCategory);
			if (chainedListener != 0) {
				chainedListener->Trace(message);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Warning: Shortcut for warning notification
		// Inputs:
		//	const string&		message		Text of the message.  Do not alter or delete!
		///////////////////////////////////////////////////////////////////////////////
		void Warning(
			const TsString& message
		) {
			OutputMessage(message, WarningCategory);
			if (chainedListener != 0) {
				chainedListener->Warning(message);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Error: Shortcut for error notification
		// Inputs:
		//	const string&		message		Text of the message.  Do not alter or delete!
		///////////////////////////////////////////////////////////////////////////////
		void Error(
			const TsString& message
		) {
			OutputMessage(message, ErrorCategory);
			if (chainedListener != 0) {
				chainedListener->Error(message);
			}
		}

		// Message categories
		enum Category { InformationCategory, WarningCategory, ErrorCategory };

		///////////////////////////////////////////////////////////////////////////////
		// If this listener implementation keeps an error-list (bulliten-board
		// style instead of stream-style), then it should clear the list when this
		// is called.
		///////////////////////////////////////////////////////////////////////////////
		virtual void ClearErrors() {
			// default implementation is a NOP
		}

		///////////////////////////////////////////////////////////////////////////////
		// "Chain" a listener to this one, so as to propagate 
		// messages to more than one place.
		///////////////////////////////////////////////////////////////////////////////
		void ChainListener(ListenerRef chainedListener_) {
			chainedListener = chainedListener_;
		}

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
		) = 0;

		// You can "chain" a listener to this one, so as to propagate 
		// messages to more than one place.
		ListenerRef chainedListener;
	};


} // namespace

#endif

