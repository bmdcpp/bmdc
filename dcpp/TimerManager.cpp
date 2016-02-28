/*
 * Copyright (C) 2001-2015 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "TimerManager.h"
#include "Util.h"

namespace dcpp {
#ifdef _WIN32
    DWORD TimerManager::lastTick = 0;
    uint32_t TimerManager::cycles = 0;

    typedef ULONGLONG (WINAPI *GTC64)(void);
    GTC64 pGTC64 = NULL;
#else
    timeval TimerManager::tv;
#endif
TimerManager::TimerManager() {
#ifdef _WIN32
    if(Util::OsMajor >= 6) {
        pGTC64 = (GTC64)::GetProcAddress(::GetModuleHandle(_T("Kernel32.dll")), "GetTickCount64");
    }
#else
	gettimeofday(&tv, NULL);
#endif
	dcdebug("TimerManager \n %d , %p ", (int)listeners.size(),this);

}

TimerManager::~TimerManager() throw() {
	dcassert(listeners.empty());
	shutdown();
}

int TimerManager::run() {
	int nextMin = 0;

	uint64_t x = getTick();
	uint64_t nextTick = x + 1000;

	while(!s.wait(nextTick > x ? (nextTick - x) : 0)) {
		uint64_t z = getTick();
		nextTick = z + 1000;
		fire(TimerManagerListener::Second(), z);
		if(nextMin++ >= 60) {
			fire(TimerManagerListener::Minute(), z);
			nextMin = 0;
		}
		x = getTick();
	}

	return 0;
}
//@this is in Milisecunds
uint64_t TimerManager::getTick() {
#ifdef _WIN32
    static volatile long state = 0;

    while(Thread::safeExchange(state, 1) == 1) {
		::Sleep(1);
	}

    if(pGTC64 != NULL) {
        uint64_t ui64Ret = pGTC64();

        Thread::safeDec(state);

        return ui64Ret;
    } else {
        DWORD tick = ::GetTickCount();
        if(tick < lastTick) {
            cycles++;
        }
        lastTick = tick;

        uint64_t ui64Ret = uint64_t(cycles) * (uint64_t(std::numeric_limits<DWORD>::max()) + 1) + tick;

        Thread::safeDec(state);

        return ui64Ret;
    }
#else
	timeval tv2;
	gettimeofday(&tv2, NULL);
	/// @todo check conversions to use uint64_t fully
	return static_cast<uint64_t>(((tv2.tv_sec - tv.tv_sec) * static_cast<uint64_t>(1000) ) + ( (tv2.tv_usec - tv.tv_usec) / static_cast<uint64_t>(1000) ));
#endif
}
}//namespace dcpp
