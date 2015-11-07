/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * This is reduced version of old CriticalSection.h from dcpp, the reason for
 * inclusion being that LuaPlugin being one of the first plugins in existense
 * I have no desire to couple it with boost at this point in time. You do have
 * the option, to use boost, though by defining WITH_BOOST_CS during compilation
 * via commandline.
 *
 *	- Crise
 */

#ifndef DCPLUSPLUS_DCPP_CRITICAL_SECTION_H
#define DCPLUSPLUS_DCPP_CRITICAL_SECTION_H

#include <debug.h>

#ifdef WITH_BOOST_CS
# include <boost/thread/recursive_mutex.hpp>
#endif

namespace dcpp {

#ifdef WITH_BOOST_CS

typedef boost::recursive_mutex CriticalSection;
typedef boost::unique_lock<boost::recursive_mutex> Lock;

#else

class CriticalSection
{
#ifdef _WIN32
public:
	void lock() throw() {
		EnterCriticalSection(&cs);
		dcdrun(counter++);
	}
	void unlock() throw() {
		dcassert(--counter >= 0);
		LeaveCriticalSection(&cs);
	}
	CriticalSection() throw() {
		dcdrun(counter = 0;);
		InitializeCriticalSection(&cs);
	}
	~CriticalSection() throw() {
		dcassert(counter==0);
		DeleteCriticalSection(&cs);
	}
private:
	dcdrun(long counter);
	CRITICAL_SECTION cs;
#else
public:
	CriticalSection() throw() {
		pthread_mutexattr_init(&ma);
		pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&mtx, &ma);
	}
	~CriticalSection() throw() {
		pthread_mutex_destroy(&mtx);
		pthread_mutexattr_destroy(&ma);
	}
	void lock() throw() { pthread_mutex_lock(&mtx); }
	void unlock() throw() { pthread_mutex_unlock(&mtx); }
	pthread_mutex_t& getMutex() { return mtx; }
private:
	pthread_mutex_t mtx;
	pthread_mutexattr_t ma;
#endif
	CriticalSection(const CriticalSection&);
	CriticalSection& operator=(const CriticalSection&);
};

template<class T>
class LockBase {
public:
	LockBase(T& aCs) throw() : cs(aCs) { cs.lock(); }
	~LockBase() throw() { cs.unlock(); }
private:
	LockBase& operator=(const LockBase&);
	T& cs;
};

typedef LockBase<CriticalSection> Lock;

#endif

} // namespace dcpp

#endif // DCPLUSPLUS_DCPP_CRITICAL_SECTION_H
