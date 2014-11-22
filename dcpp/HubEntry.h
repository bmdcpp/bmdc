/*
 * Copyright (C) 2001-2014 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_HUBENTRY_H_
#define DCPLUSPLUS_DCPP_HUBENTRY_H_

#include <string>

#include "SettingsManager.h"
#include "Util.h"
#include "ActionRaw.h"
#include <list>
#include "HubSettings.h"

namespace dcpp {

using std::string;

class HubEntry {
public:
	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) :
	name(aName), server(aServer), description(aDescription), country(Util::emptyString),
	rating(Util::emptyString), reliability(0.0), shared(0), minShare(0), users(Util::toInt(aUsers)), minSlots(0), maxHubs(0), maxUsers(0) { }

	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers, const string& aCountry,
		const string& aShared, const string& aMinShare, const string& aMinSlots, const string& aMaxHubs, const string& aMaxUsers,
		const string& aReliability, const string& aRating) : name(aName), server(aServer), description(aDescription), country(aCountry),
		rating(aRating), reliability((float)(Util::toFloat(aReliability) / 100.0)), shared(Util::toInt64(aShared)), minShare(Util::toInt64(aMinShare)),
		users(Util::toInt(aUsers)), minSlots(Util::toInt(aMinSlots)), maxHubs(Util::toInt(aMaxHubs)), maxUsers(Util::toInt(aMaxUsers))
	{

	}

	HubEntry() { }
	HubEntry(const HubEntry& rhs) : name(rhs.name), server(rhs.server), description(rhs.description), country(rhs.country),
		rating(rhs.rating), reliability(rhs.reliability), shared(rhs.shared), minShare(rhs.minShare), users(rhs.users), minSlots(rhs.minSlots),
		maxHubs(rhs.maxHubs), maxUsers(rhs.maxUsers) { }

	~HubEntry() { }

	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, country, Country);
	GETSET(string, rating, Rating);
	GETSET(float, reliability, Reliability);
	GETSET(int64_t, shared, Shared);
	GETSET(int64_t, minShare, MinShare);
	GETSET(int, users, Users);
	GETSET(int, minSlots, MinSlots);
	GETSET(int, maxHubs, MaxHubs);
	GETSET(int, maxUsers, MaxUsers);
};

class FavoriteHubEntry: public HubSettings {
public:
	FavoriteHubEntry() :
	name(Util::emptyString), server(Util::emptyString), hubDescription(Util::emptyString), password(Util::emptyString),
	encoding(Text::systemCharset), group(Util::emptyString), hideShare(false), autoConnect(false), mode(0),
	chatExtraInfo(SETTING(CHAT_EXTRA_INFO)), protectUsers(Util::emptyString), checkAtConn(false), checkClients(false),
	checkFilelists(false), checkMyInfo(false), showUserList(true), 
	order(SETTING(HUB_UL_ORDER)), visible(SETTING(HUB_UL_VISIBLE)), width(SETTING(HUB_UL_SIZE)), 
	tabText(SETTING(HUB_TEXT_STR)) , tabIconStr(SETTING(HUB_ICON_STR)) , notify(false) { }

	FavoriteHubEntry(const HubEntry& rhs) : name(rhs.getName()), server(rhs.getServer()),
		hubDescription(rhs.getDescription()), password(Util::emptyString), encoding(Text::systemCharset), group(Util::emptyString),
	hideShare(false), autoConnect(false), mode(0), chatExtraInfo(SETTING(CHAT_EXTRA_INFO)), protectUsers(Util::emptyString), 
	checkAtConn(false), checkClients(false), checkFilelists(false),  checkMyInfo(false), showUserList(true),
	order(SETTING(HUB_UL_ORDER)),visible(SETTING(HUB_UL_VISIBLE)),width(SETTING(HUB_UL_SIZE)),
	tabText(SETTING(HUB_TEXT_STR)) , tabIconStr(SETTING(HUB_ICON_STR)) , notify(false)
		{ }

	FavoriteHubEntry(const FavoriteHubEntry& rhs) :
		HubSettings(rhs),
		name(rhs.getName()), server(rhs.getServer()), hubDescription(rhs.getHubDescription()),
		password(rhs.getPassword()), encoding(rhs.getEncoding()), group(rhs.getGroup()), hideShare(rhs.hideShare),
		autoConnect(rhs.autoConnect),  mode(rhs.mode), chatExtraInfo(rhs.chatExtraInfo),
		protectUsers(rhs.protectUsers),	checkAtConn(rhs.checkAtConn), checkClients(rhs.checkClients), checkFilelists(rhs.checkFilelists),  checkMyInfo(rhs.checkMyInfo),
		showUserList(rhs.showUserList), order(rhs.order), visible(rhs.visible), width(rhs.width), tabText(rhs.tabText), tabIconStr(rhs.tabIconStr), notify(rhs.notify)
		{ }

	~FavoriteHubEntry() { }

	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, hubDescription, HubDescription);
	GETSET(string, password, Password);
	GETSET(string, encoding, Encoding);
	GETSET(string, group, Group);
	//BMDC++
	GETSET(bool, hideShare, HideShare);
	GETSET(bool, autoConnect, AutoConnect);
	GETSET(int, mode, Mode);
	
	GETSET(string, chatExtraInfo, ChatExtraInfo);//HubGroup?
	GETSET(string, protectUsers, ProtectUsers);//Hub Group?
	GETSET(bool, checkAtConn, CheckAtConn);
	GETSET(bool, checkClients, CheckClients);
	GETSET(bool, checkFilelists, CheckFilelists);
	GETSET(bool, checkMyInfo,CheckMyInfo);

	GETSET(bool, showUserList , ShowUserList);//TODO Hub Group?
	GETSET(string, order, HubOrder);
	GETSET(string, visible, HubVisible);
	GETSET(string, width, HubWidth);
	
	GETSET(string, tabText, TabText);//bad idea 
	GETSET(string, tabIconStr, TabIconStr);//TODO HubGroup
	GETSET(bool, notify , Notify );//TODO HubGroup

	//Raw Manager
	struct FavAction {
		typedef unordered_map<int, FavAction*> List;
		FavAction(): enabled(false) { }
		FavAction(bool _enabled, string _raw = Util::emptyString, int id = 0) noexcept;

		GETSET(bool, enabled, Enabled);
		std::list<int> raws;
	};

	FavAction::List action;
};

class RecentHubEntry {
public:
		typedef RecentHubEntry* Ptr;
		typedef vector<Ptr> List;
		typedef List::const_iterator Iter;

		~RecentHubEntry() noexcept { }

		GETSET(string, name, Name);
		GETSET(string, server, Server);
		GETSET(string, description, Description);
		GETSET(string, users, Users);
		GETSET(string, shared, Shared);
};

}

#endif /*HUBENTRY_H_*/
