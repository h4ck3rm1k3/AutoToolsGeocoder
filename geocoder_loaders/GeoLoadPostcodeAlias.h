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

// GeoLoadPostcodeAlias.h:  

#ifndef INCL_GeoLoadPostcodeAlias_H
#define INCL_GeoLoadPostcodeAlias_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "GeoLoadBase.h"

namespace PortfolioExplorer {


	///////////////////////////////////////////////////////////////////////////////
	// : abstract base class for processing nodes
	///////////////////////////////////////////////////////////////////////////////
	class GeoLoadPostcodeAlias : public GeoLoadBase {
	public:


		///////////////////////////////////////////////////////////////////////////////
		// Process the records for a terminal node.
		// Return value:
		//	bool		true on success, false on error or abort
		///////////////////////////////////////////////////////////////////////////////
		virtual void Process();

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Get a static array of FieldParameter entries, which will be used to
		// load up the fieldParameters vector.  The terminating element must
		// have an empty paramName.
		///////////////////////////////////////////////////////////////////////////////
		virtual std::vector<TsString> GetFieldParameters();

		struct PostcodeGroup {
			char postcode[7];
			char group[7];
		};

		struct PostcodeCmp {
			bool operator()(const PostcodeGroup& lhs, const PostcodeGroup& rhs) const {
				return strcmp(lhs.postcode, rhs.postcode) < 0;
			}
		};
		struct GroupCmp {
			bool operator()(const PostcodeGroup& lhs, const PostcodeGroup& rhs) const {
				return strcmp(lhs.group, rhs.group) < 0;
			}
		};

	};
}

#endif

