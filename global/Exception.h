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

// Exception.h: Error objects

#ifndef INCL_Exception_H
#define INCL_Exception_H

#if _MSC_VER >= 1000
#pragma once
#endif

// Disable that pesky long-mangled-name warning
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "TsString.h"
#include "Global_DllExport.h"

namespace PortfolioExplorer {

	// Definition of non-fatal-error object (used by such things as file
	// write failure).  This is caught internally by the JH library.
	class  ErrorException {
	public:
		ErrorException(const TsString& message_) : message(message_) {}
		TsString message;
	};


	// Definition of abort-job-exception object.
	// This is caught internally by the JH library.
	class  AbortException {
	public:
		AbortException() {}
	};

}

#endif

