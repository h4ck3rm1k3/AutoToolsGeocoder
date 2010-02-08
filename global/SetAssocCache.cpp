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

// SetAssocCache.cpp: N-way set-associative cache template

#include "Global_Headers.h"
#include "SetAssocCache.h"

namespace PortfolioExplorer {

	bool SetAssocCacheBase::IsPrime(int value)
	{
		for (int test = 2; test * test <= value; test++)
		{
			if (value % test == 0)
			{
				return false;
			}
		}
		return true;
	}

	int SetAssocCacheBase::NextPrime(int value)
	{
		if (value % 2 == 0)
		{
			value++;
		}
		while (!IsPrime(value))
		{
			value += 2;
		}
		return value;
	}

}
