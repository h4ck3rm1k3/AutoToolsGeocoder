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

// ListenerFIFO: Object that places messages in a thread-safe queue for later retrieval.

#include "Global_Headers.h"
#include "ListenerFIFO.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// OutputMessage: Notification of messages
	// Inputs:
	//	const string&	message			Text of the message.
	//	Category		category		Category of the message.
	///////////////////////////////////////////////////////////////////////////////
	void ListenerFIFO::OutputMessage(
		const TsString& message,
		Category category
	) {
		if (fifo.count_nolock() < MaxStoredMessages) {
			fifo.add(Message(category, message));
		} else if (fifo.count_nolock() == MaxStoredMessages) {
			fifo.add(Message(InformationCategory, "Message storage capacity reached.  No more messages will be stored."));
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Are there any messages in the queue?
	///////////////////////////////////////////////////////////////////////////////
	bool ListenerFIFO::HaveMessage() {
		return fifo.count() != 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Are there any messages in the queue?
	///////////////////////////////////////////////////////////////////////////////
	bool ListenerFIFO::HaveMessage(Category category) {
		if (fifo.count() == 0) {
			return false;
		}
		// Not very efficient...
		std::list<ListenerFIFO::Message> msgList = fifo.getAll();
		for (
			std::list<ListenerFIFO::Message>::iterator iter = msgList.begin();
			iter != msgList.end();
			++iter
		) {
			if ((*iter).category == category) {
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Copy all the messages in the queue, but leave them there.
	// Return value:
	//	std::vector<Message>		List of all messages
	///////////////////////////////////////////////////////////////////////////////
	std::vector<ListenerFIFO::Message> ListenerFIFO::GetMessages() const
	{
		// Not very efficient...
		std::list<ListenerFIFO::Message> msgList = fifo.getAll();
		std::vector<ListenerFIFO::Message> retval;
		for (
			std::list<ListenerFIFO::Message>::iterator iter = msgList.begin();
			iter != msgList.end();
			++iter
		) {
			retval.push_back(*iter);
		}
		return retval;			
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get the next message
	///////////////////////////////////////////////////////////////////////////////
	ListenerFIFO::Message ListenerFIFO::GetNextMessage() { 
		if (fifo.count() == 0) {
			return Message(ErrorCategory, "Reading empty listener message queue.");
		} else {
			return fifo.remove(); 
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Clear out the error messages.
	// Overide of base class method.
	///////////////////////////////////////////////////////////////////////////////
	void ListenerFIFO::ClearErrors() { 
		fifo.clear();
	}

}	// namespace 

