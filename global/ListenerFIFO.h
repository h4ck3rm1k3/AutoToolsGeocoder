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
# $Rev: 49 $ 
# $Date: 2006-09-25 20:00:58 +0200 (Mon, 25 Sep 2006) $ 
*/

// ListenerFIFO: Object that places messages in a thread-safe queue for later retrieval.

#ifndef INCL_JHLISTENERFIFO_H
#define INCL_JHLISTENERFIFO_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "Listener.h"
#include "ThreadSafeFifo.h"
#include "Global_DllExport.h"
#include <vector>

namespace PortfolioExplorer {

	class ListenerFIFO : public Listener {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// constructor
		///////////////////////////////////////////////////////////////////////////////
		ListenerFIFO() { }

		///////////////////////////////////////////////////////////////////////////////
		// destructor
		///////////////////////////////////////////////////////////////////////////////
		virtual ~ListenerFIFO() { }

		///////////////////////////////////////////////////////////////////////////////
		// struct that contains an Info, Warning, or Error message
		///////////////////////////////////////////////////////////////////////////////
		struct Message {
			Message(
				Category category_, 
				const TsString& str_
			) : 
				category(category_), str(str_)
			{}
			Category category;
			TsString str;
		};

		///////////////////////////////////////////////////////////////////////////////
		// Are there any messages in the queue?
		///////////////////////////////////////////////////////////////////////////////
		bool HaveMessage();

		///////////////////////////////////////////////////////////////////////////////
		// Are there any messages in the queue of the given category?
		///////////////////////////////////////////////////////////////////////////////
		bool HaveMessage(Category category);

		///////////////////////////////////////////////////////////////////////////////
		// Copy all queued messages but leave them in the queue.
		// Return value:
		//	std::vector<Message>		List of messages
		///////////////////////////////////////////////////////////////////////////////
		std::vector<Message> GetMessages() const;

		///////////////////////////////////////////////////////////////////////////////
		// Get the next message
		///////////////////////////////////////////////////////////////////////////////
		Message GetNextMessage();

		///////////////////////////////////////////////////////////////////////////////
		// OutputMessage: Notification of messages
		// Inputs:
		//	const string&	message			Text of the message.
		//	Category		category		Category of the message.
		///////////////////////////////////////////////////////////////////////////////
		void OutputMessage(
			const TsString& message,
			Category category
		);

		///////////////////////////////////////////////////////////////////////////////
		// Clear out the error messages.
		// Overide of base class method.
		///////////////////////////////////////////////////////////////////////////////
		virtual void ClearErrors();

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Message queue
		///////////////////////////////////////////////////////////////////////////////
		ThreadSafeFIFO<Message> fifo;

		// Maximum number of stored messages
		enum { MaxStoredMessages = 1000 };
	};

	typedef refcnt_ptr<ListenerFIFO> ListenerFIFORef;

}

#endif
