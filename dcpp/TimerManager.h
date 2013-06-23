/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include "Thread.h"
#include "Semaphore.h"
#include "Speaker.h"
#include "Singleton.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

namespace dcpp {

class TimerManagerListener {
public:
	virtual ~TimerManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Second;
	typedef X<1> Minute;

	// We expect everyone to implement this...
	virtual void on(Second, uint64_t) throw() { }
	virtual void on(Minute, uint64_t) throw() { }
};

class TimerManager : public Speaker<TimerManagerListener>, public Singleton<TimerManager>, public Thread {
public:
	TimerManager();
	virtual ~TimerManager() throw();

	void shutdown() {
		s.signal();
		join();
	}

	static time_t getTime() {
		return (time_t)time(NULL);
	}
	static uint64_t getTick();
private:
	Semaphore s;

	int run();

#ifdef _WIN32
	static DWORD lastTick;
	static uint32_t cycles;
#else
	static timeval tv;
#endif
};
};//namespace dcpp
#define GET_TICK() TimerManager::getTick()
#define GET_TIME() TimerManager::getTime()

#endif // TIMERMANAGER_H
