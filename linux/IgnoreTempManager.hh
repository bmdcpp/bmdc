/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _IGNORE_MANAGER
#define _IGNORE_MANAGER
#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/Singleton.h"
#include "../dcpp/TimerManager.h"
#include <map>

class IgnoreTempManager: public dcpp::Singleton<IgnoreTempManager>, private dcpp::TimerManagerListener
{
	public:
		IgnoreTempManager();
		~IgnoreTempManager();

		uint64_t lastTick;//not used?

		void addNickIgnored(std::string nick, uint64_t time);
		void addIpIgnored(std::string ip,uint64_t time);
		void addCidIgnored(std::string cid,uint64_t time);
		bool isCidIgnored(std::string cid);
		bool isIpIgnored(std::string ip);
		bool isNickIgnored(std::string nick);

		void removeNick(std::string nick);
		void removeIp(std::string ip);
		void removeCid(std::string cid);

		std::map<std::string, time_t > nickIgnore;
		std::map<std::string, std::pair<uint64_t,uint64_t> > ipIgnore;
		std::map<std::string, std::pair<uint64_t,uint64_t> > cidIgnore;
	
		virtual void on(dcpp::TimerManagerListener::Minute, uint64_t aTick) noexcept;
		mutable dcpp::CriticalSection cs;
};
#endif
