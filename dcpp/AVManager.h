

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
		bool isNickVirused(string nick) { Lock l(cs); return entries.find(nick) != entries.end(); }
		bool isIpVirused(string ip) { Lock l(cs); return entip.find(ip) != entip.end(); }
		struct AVEntry
		{
			string nick;
			string ss;
			string ip;
		};
		AVEntry getEntryByNick(string nick) { Lock l(cs); return entries.find(nick)->second; }
		AVEntry getEntryByIP(string ip) { Lock l(cs); return entip.find(ip)->second; }
		std::map<string /*nick*/,AVEntry> entries;
		std::map<string /*ip*/,AVEntry> entip;
		//@ <nick>|<ip>|<share>|<time>\n
		//@ parf of code is same as in Flylink
		void loadDb(const string& buf)
		{
			if((!buf.length()) || ((buf.length() == 1) && (buf == "0") )) return;
			if (buf.length() < 12) return;
			size_t l_pos = 0;
			int l_nick_pos = 0;
			int l_nick_len = 0;
			int l_count_new_user = 0;
			while (true)
			{
				l_nick_pos = l_pos;
				size_t l_sep_pos = buf.find('|', l_pos);
				if (l_sep_pos == string::npos)
					break;
				l_nick_len = l_sep_pos - l_nick_pos;
				string nick = Text::toUtf8(buf.substr(l_nick_pos, l_nick_len));
				l_pos = l_sep_pos + 1;
				l_sep_pos = buf.find('|', l_pos);
				if (l_sep_pos == string::npos)
					break;
				string ip = buf.substr(l_pos, l_sep_pos - l_pos);

				l_pos = l_sep_pos + 1;
				const string& share = buf.c_str() + l_pos;
				l_pos = l_sep_pos + 1;
				l_sep_pos = buf.find('\n', l_pos);
				if (l_sep_pos == string::npos)
					break;
				l_pos = l_sep_pos + 1;
				++l_count_new_user;
				
				if(nick.empty() || ip.empty() || share.empty()){
						continue;
				}
				
				AVEntry entry;
				entry.nick = nick;
				entry.ss = share;
				entry.ip = ip;
				entries.insert(make_pair(nick,entry));
				entip.insert(make_pair(ip,entry));
			}	

		};
		std::unique_ptr<dcpp::HttpDownload> conn;
		time_t timestamp_db;
		uint64_t temp_tick;
		virtual void on(TimerManagerListener::Minute, uint64_t aTick) noexcept
		{
			if(SETTING(USE_AV_FILTER)) {

			if(aTick >= temp_tick) {
			string address =
			 ("http://te-home.net/?do=tools&action=avdbload&time="+Util::toString(timestamp_db)+"&notime=1");
				conn.reset( new HttpDownload(address,[this](bool s,const string& b) { if(s) loadDb(b); }, false));
				timestamp_db = time(NULL);
				LogManager::getInstance()->message(_("[AVDB] load ")+Util::toString(temp_tick)+" - "+Util::toString(timestamp_db)+" - "+Util::toString(aTick));	
				temp_tick = aTick+(60*60*1000);
			}
			LogManager::getInstance()->message(_("[AVDB] next ")+Util::toString(temp_tick)+" - "+Util::toString(timestamp_db)+" - "+Util::toString(aTick));
		  }					
		}
		CriticalSection cs;
		friend class Singleton<AVManager>;
};

}
#endif
