#include "AVManager.h"

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
#include "Text.h"

namespace dcpp {
//@ <nick>|<ip>|<share>|<time>\n
//@ parf of code is same as in Flylink
void AVManager::loadDb(const string& buf)

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
				string nick = Text::toLower(buf.substr(l_nick_pos, l_nick_len));
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

}

void AVManager::on(TimerManagerListener::Minute, uint64_t aTick) noexcept
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

}