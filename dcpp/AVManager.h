/*
 * Copyright (C) 2014-2015 Mank, freedcpp on seznam point cz
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

#ifndef _BMDC_A_MANAGER_H
#define _BMDC_A_MANAGER_H

#include <utility>
#include <map>
#include "Exception.h"
#include "File.h"
#include "LogManager.h"
#include "HttpDownload.h"
#include "Singleton.h"
#include "Thread.h"
#include "TimerManager.h"
#include "Util.h"

namespace dcpp {

class AVManager: public Singleton<AVManager>, private TimerManagerListener
{
	public:
		AVManager(): timestamp_db(0), temp_tick(GET_TICK())
		{	
			TimerManager::getInstance()->addListener(this);
		}
		virtual ~AVManager() {	TimerManager::getInstance()->removeListener(this); }
		bool isNickVirused(string nick) { Lock l(cs); return entries.find(Text::toLower(nick)) != entries.end(); }
		bool isIpVirused(string ip) { Lock l(cs); return entip.find(ip) != entip.end(); }
		struct AVEntry
		{
			string nick;
			uint64_t share;
		};
		void addItemNick(const string& nick,const AVEntry& entry);
		void addItemIp(const string& ip,const AVEntry& entry);
		AVEntry getEntryByNick(string nick) { Lock l(cs); return entries.find(Text::toLower(nick))->second; }
		AVEntry getEntryByIP(string ip) { Lock l(cs); return entip.find(ip)->second; }
		std::map<string /*nick*/,AVEntry> entries;
		std::map<string /*ip*/,AVEntry> entip;
		//@ <nick>|<ip>|<share>|<time>\n
		void loadDb(const string& buf);
		std::unique_ptr<dcpp::HttpDownload> conn;
		time_t timestamp_db;
		uint64_t temp_tick;
		virtual void on(TimerManagerListener::Minute, uint64_t aTick) noexcept;
		CriticalSection cs;
		friend class Singleton<AVManager>;
};

}
#endif
