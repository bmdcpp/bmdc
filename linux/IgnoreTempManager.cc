#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "IgnoreTempManager.hh"

using namespace std;
using namespace dcpp;

IgnoreTempManager::IgnoreTempManager()
{}

IgnoreTempManager::~IgnoreTempManager()
{}

void IgnoreTempManager::addNickIgnored(string nick, uint64_t time)
{
	Lock l(cs);
	uint64_t tick = GET_TICK();	
	nickIgnore.insert(make_pair(nick,make_pair(tick,time)));
	
}
void IgnoreTempManager::addIpIgnored(string ip,uint64_t time)
{
	Lock l(cs);
	uint64_t tick = GET_TICK();	
	ipIgnore.insert(make_pair(ip,make_pair(tick,time)));
	
}
void IgnoreTempManager::addCidIgnored(string cid,uint64_t time)
{
	Lock l(cs);
	uint64_t tick = GET_TICK();	
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
		//lastTick = aTick;
		Lock l(cs);
		if(nickIgnore.size() >= 1)
		{
			for(auto i = nickIgnore.begin(); i!= nickIgnore.end();++i)
			{
				auto s = i->second;
				uint64_t timeRem = s.first + s.second;
				if(timeRem > aTick)	
						removeNick(i->first);
			}
			
		}
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
