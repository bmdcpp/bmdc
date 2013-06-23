/*
 * Copyright (C) 2007-2013 adrian_007, adrian-007 on o2 point pl
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
#include "CommandQueue.h"
#include "Client.h"
#include "FavoriteManager.h"
#include "RawManager.h"
#include "LogManager.h"
#include "SettingsManager.h"

namespace dcpp {

void CommandQueue::onSecond(uint64_t tick) {
	Lock l(cs);
	if(clientPtr == NULL)
		return;
	for(auto i = queue.begin(); i != queue.end(); ++i) {
		const pair<uint64_t, CommandItem>& p = *i;
		if(tick >= p.first) {
			execCommand(p.second);
			i = queue.erase(i);
		}
	}
}

void CommandQueue::addCommandDelayed(uint64_t delay, const CommandItem& item)  {
	Lock l(cs);
	queue.push_back(make_pair(delay, item));
}

void CommandQueue::execCommand(const CommandItem& item)  {
	ParamMap params;
	item.ou->getIdentity().getParams(params, "user", true);
	clientPtr->getHubIdentity().getParams(params, "hub", false);
	clientPtr->getMyIdentity().getParams(params, "my", true);
	Util::formatParams(item.uc.getCommand(), params);
	clientPtr->sendUserCmd(item.uc, params);
}

void CommandQueue::addCommand(const OnlineUser& ou, int actionId) {
	FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(clientPtr->getHubUrl());
	if(hub) {
		Action* a = RawManager::getInstance()->findAction(actionId);

		if(a != NULL) {
			if(FavoriteManager::getInstance()->getEnabledAction(hub, actionId)) {

				uint64_t delayTime = GET_TICK();
				for(auto i = a->raw.begin(); i != a->raw.end(); ++i) {
					if(i->getEnabled() && !(i->getRaw().empty())) {
						if(FavoriteManager::getInstance()->getEnabledRaw(hub, actionId, i->getId())) {
							ParamMap params;
							const UserCommand uc = UserCommand(0, 0, 0, 0, "", i->getRaw(),"", "");

							ou.getIdentity().getParams(params, "user", true);
							clientPtr->getHubIdentity().getParams(params, "hub", false);
							clientPtr->getMyIdentity().getParams(params, "my", true);
							string formattedCmd = Util::formatParams(uc.getCommand(), params);

							CommandItem item;
							item.name = i->getName();
							item.uc = uc;
							item.ou = &ou;

							if(SETTING(USE_SEND_DELAYED_RAW)) {
								delayTime += (i->getTime() * 1000) + 1;
								addCommandDelayed(delayTime, item);
							} else {
								execCommand(item);
							}
							if(SETTING(LOG_RAW_CMD)) {
								params["rawCommand"] = formattedCmd;
								LOG(LogManager::RAW, params);
							}
						}
					}
				}

			}
		}
	}
}

} // namespace dcpp
