#ifndef _IGNORE_MANAGER
#define _IGNORE_MANAGER
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/Singleton.h>
#include <dcpp/TimerManager.h>
#include <map>
/*
struct IgnoreItem
{
	string nameItem;
	uint64_t time;
	int type;
};
*/

class IgnoreTempManager: private dcpp::TimerManagerListener
{
	public:
		IgnoreTempManager();
		~IgnoreTempManager();
		mutable dcpp::CriticalSection cs;

		std::map<std::string, std::pair<uint64_t,uint64_t> > nickIgnore;
		std::map<std::string, std::pair<uint64_t,uint64_t> > ipIgnore;
		std::map<std::string, std::pair<uint64_t,uint64_t> > cidIgnore;

		uint64_t lastTick;
void addNickIgnored(std::string nick, uint64_t time);
void addIpIgnored(std::string ip,uint64_t time);
void addCidIgnored(std::string cid,uint64_t time);
bool isCidIgnored(std::string cid);
bool isIpIgnored(std::string ip);
bool isNickIgnored(std::string nick);

void removeNick(std::string nick);
void removeIp(std::string ip);
void removeCid(std::string cid);

virtual void on(dcpp::TimerManagerListener::Minute, uint64_t aTick) noexcept;
};
#endif
