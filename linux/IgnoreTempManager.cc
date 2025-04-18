/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "IgnoreTempManager.hh"

using namespace std;
using namespace dcpp;

IgnoreTempManager::IgnoreTempManager()
{

}

IgnoreTempManager::~IgnoreTempManager()
{
	
}

void IgnoreTempManager::addNickIgnored(string nick, uint64_t uitime)
{
	Lock l(cs);
	uint64_t uitick = GET_TIME()+time_t(uitime*60);	//now+time in seconds
	nickIgnore.insert(make_pair(nick,uitick));
	
}
void IgnoreTempManager::addIpIgnored(string ip,uint64_t time)
{
	Lock l(cs);
	uint64_t tick = GET_TIME();	
	ipIgnore.insert(make_pair(ip,make_pair(tick,time)));
	
}
void IgnoreTempManager::addCidIgnored(string cid,uint64_t time)
{
	Lock l(cs);
	uint64_t tick = GET_TIME();	
	cidIgnore.insert(make_pair(cid,make_pair(tick,time)));
	
}

bool IgnoreTempManager::isCidIgnored(string cid) {Lock l(cs); return cidIgnore.find(cid) != cidIgnore.end();}
bool IgnoreTempManager::isIpIgnored(string ip){Lock l(cs); return ipIgnore.find(ip) != ipIgnore.end();}
bool IgnoreTempManager::isNickIgnored(string nick){Lock l(cs); return nickIgnore.find(nick) != nickIgnore.end();}

void IgnoreTempManager::removeNick(string nick){Lock l(cs); nickIgnore.erase(nick);}
void IgnoreTempManager::removeIp(string ip){Lock l(cs); ipIgnore.erase(ip);}
void IgnoreTempManager::removeCid(string cid){Lock l(cs); cidIgnore.erase(cid);}

void IgnoreTempManager::on(dcpp::TimerManagerListener::Minute, uint64_t aTick) noexcept
{
		time_t t_time = GET_TIME();//now
		Lock l(cs);
		if(!nickIgnore.empty())
		{
			vector<string> temp;
			for(auto i = nickIgnore.begin(); i!= nickIgnore.end();++i)
			{
				time_t timeRem = i->second;
				if(difftime(t_time,timeRem) > 0)
					temp.push_back(i->first);
			}
			for(auto ix:temp) {
				removeNick(ix);
			}	
		}
		/*this need check*/
		if(ipIgnore.size() >= 1)
		{
			
		
			for(auto i = ipIgnore.begin(); i!= ipIgnore.end();++i)
			{
				auto s = i->second;
				uint64_t timeRem = s.first + s.second;
				if(timeRem > aTick)	
						removeIp(i->first);
			}	
			
		}
		if(cidIgnore.size() >= 1)
		{
		
			for(auto i = cidIgnore.begin(); i!= cidIgnore.end();++i)
			{
				auto s = i->second;
				uint64_t timeRem = s.first + s.second;
				if(timeRem > aTick)	
						removeCid(i->first);
			}	
			
		}
}
