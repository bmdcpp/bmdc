/*
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
#ifndef _IGNORE_MANAGER
#define _IGNORE_MANAGER
#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/Singleton.h"
#include "../dcpp/TimerManager.h"
#include <map>
/*
struct IgnoreItem
{
	string nameItem;
	uint64_t time;
	int type;
};
*/

class IgnoreTempManager: public dcpp::Singleton<IgnoreTempManager>, private dcpp::TimerManagerListener
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
