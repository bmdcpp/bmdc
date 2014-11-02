

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
			string ss;
			string ip;
		};
		AVEntry getEntryByNick(string nick) { Lock l(cs); return entries.find(Text::toLower(nick))->second; }
		AVEntry getEntryByIP(string ip) { Lock l(cs); return entip.find(ip)->second; }
		std::map<string /*nick*/,AVEntry> entries;
		std::map<string /*ip*/,AVEntry> entip;
		//@ <nick>|<ip>|<share>|<time>\n
		//@ parf of code is same as in Flylink
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
