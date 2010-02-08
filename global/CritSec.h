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

///////////////////////////////////////////////////////////////////////////////
//
// (C) 2001 SRC, LLC  -   All rights reserved
//
///////////////////////////////////////////////////////////////////////////////
//
// Module: MUTEX.H
//
// class SrcCriticalSection
// class TMutex<TCriticalSection>
// typdef TMutex<SrcCriticalSection> SrcMutex
// typdef TMutex<CCriticalSection> Mutex
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MUTEX_H__
#define __MUTEX_H__

namespace PortfolioExplorer
{

//*****************************************************************************
//*
//*	class SrcCriticalSection
//*
//* A wrapper for a Windows CRITICAL_SECTION object, handling initialization 
//* and deletion automatically in the ctor and dtor.  
//*
//*****************************************************************************
class CritSecInfo
{
private:
	// Copy ctor and op= are not implemented.
	CritSecInfo(const CritSecInfo & );
	CritSecInfo & operator=(const CritSecInfo & );

public:
	inline CritSecInfo() { 
#if defined(WIN32)
	  ::InitializeCriticalSection( &m_criticalSection); 
#endif
	}

	inline ~CritSecInfo() { 
#if defined(WIN32)
	  ::DeleteCriticalSection(&m_criticalSection);
#endif
	}
	inline void Lock() { 

#if defined(WIN32)
	  ::EnterCriticalSection( &m_criticalSection);
#endif
	}

	inline void Unlock() { 
#if defined(WIN32)
	  ::LeaveCriticalSection(&m_criticalSection);
#endif
	}

private:

#if defined(WIN32)
	CRITICAL_SECTION m_criticalSection;
#endif

}; // SrcCriticalSection

//*****************************************************************************
//*
//*	class TMutex
//*
//* A class for locking and unlocking a critical section automatically.  The
//* dtor will automatically unlock the critical section, making this critical
//* section thread safe.
//*
//* NOTE: Despite the name, this class is NOT a mutex.  It is simply a wrapper
//* for handling critical sections.  As such, it will protect resources across
//* threads, but not across processes.  For applications that require resources
//* to be protected across processes, use the Window CMutex.
//*
//*****************************************************************************
template <class TCriticalSection> class TMutex
{
private:
	// Copy ctor and op= are not implemented.
	TMutex(const TMutex & );
	TMutex & operator=(const TMutex & );

public:
	inline TMutex(TCriticalSection & criticalSection) : m_criticalSection(criticalSection) { this->Lock();  m_bLocked = true; }
	inline ~TMutex() { if (m_bLocked) this->Unlock(); }

	inline void Lock() { m_criticalSection.Lock();  m_bLocked=true; }
	inline void Unlock() { m_criticalSection.Unlock();  m_bLocked = false; }

private:
	TCriticalSection & m_criticalSection;
	bool m_bLocked;

}; // TMutex

//*****************************************************************************
typedef TMutex<CritSecInfo> CritSec;

} // namespace PortfolioExplorer


#endif // __MUTEX_H__
