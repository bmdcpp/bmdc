/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_FAVORITE_MANAGER_H
#define DCPLUSPLUS_DCPP_FAVORITE_MANAGER_H


#include "SettingsManager.h"

#include "CriticalSection.h"
#include "HttpConnectionListener.h"
#include "UserCommand.h"
#include "FavoriteUser.h"
#include "Singleton.h"
#include "ClientManagerListener.h"
#include "FavoriteManagerListener.h"
#include "HubEntry.h"
#include "FavHubGroup.h"
#include "User.h"
#include "OnlineUser.h"
#include "UserManager.h"


namespace dcpp {


class SimpleXML;

/**
 * Public hub list, favorites (hub&user). Assumed to be called only by UI thread.
 */
class FavoriteManager : public Speaker<FavoriteManagerListener>, private HttpConnectionListener, public Singleton<FavoriteManager>,
	private SettingsManagerListener, private UsersManagerListener
{
private:
	using SettingsManagerListener::on;
	using UsersManagerListener::on;
public:
// Public Hubs
	enum HubTypes {
		TYPE_NORMAL,
		TYPE_BZIP2
	};

	StringList getHubLists();
	void setHubList(int aHubList);
	int getSelectedHubList() { return lastServer; }
	void refresh(bool forceDownload = false);
	HubTypes getHubListType() { return listType; }
	HubEntryList getPublicHubs() {
		Lock l(cs);
		return publicListMatrix[publicListServer];
	}
	bool isDownloading() { return (useHttp && running); }
	const string& getCurrentHubList() const { return publicListServer; }
	/// @return ref to the reason string the current list is blacklisted for; or empty string otherwise
	const string& blacklisted() const;
	/// @param domain a domain name with 2 words max, such as "example.com"
	void addBlacklist(const string& domain, const string& reason);
// Favorite Users
	typedef unordered_map<CID, FavoriteUser> FavoriteMap;
	FavoriteMap getFavoriteUsers() { Lock l(cs); return users; }

	void addFavoriteUser(const UserPtr& aUser);
	bool isFavoriteUser(const UserPtr& aUser) const { Lock l(cs); return users.find(aUser->getCID()) != users.end(); }
	void removeFavoriteUser(const UserPtr& aUser);

	const FavoriteUser* getFavoriteUser(const UserPtr& aUser) const;
	bool hasSlot(const UserPtr& aUser) const;
	void setUserDescription(const UserPtr& aUser, const string& description);
	void setAutoGrant(const UserPtr& aUser, bool grant);
	void userUpdated(const OnlineUser& info);
	time_t getLastSeen(const UserPtr& aUser) const;
	string getUserURL(const UserPtr& aUser) const;
	void setIgnore(const UserPtr& aUser, bool ignore);
// Indepent Favorites on CID
	typedef map<string, FavoriteUser*> FavoriteNoCid;
	FavoriteNoCid favoritesNoCid;

	FavoriteNoCid getFavoritesIndepentOnCid() { Lock l(cs); return favoritesNoCid; }
	FavoriteUser* getIndepentFavorite(const string& nick)
	{
		Lock l(cs);
		auto fit = favoritesNoCid.find(nick);
		if(fit!= favoritesNoCid.end())
		{
			return fit->second;
		}
		return NULL;
	}

	bool hasSlotI(const string& nick) {
		Lock l(cs);
		FavoriteUser* u = getIndepentFavorite(nick);
		return (u != NULL) ? (u->isSet(FavoriteUser::FLAG_GRANTSLOT)) : false;
	}
	bool isFavoriteIUser(string nick) { Lock l(cs); return favoritesNoCid.find(nick) != favoritesNoCid.end(); }

	void addFavoriteIUser(const string& nick, const time_t lastSeen = 0, const string& desc = Util::emptyString)
	{
		Lock l(cs);
		if ( favoritesNoCid.find(nick) == favoritesNoCid.end() )
		{
			FavoriteUser *fav = new FavoriteUser(FavoriteUser::Flags::FLAG_NICK);
			fav->setDescription(desc);
			fav->setLastSeen(lastSeen);
			fire(FavoriteManagerListener::FavoriteIAdded(),nick,fav);
			favoritesNoCid.insert(make_pair(nick,fav));
			save();
		}
	}
	void removeFavoriteIUser(const string& nick)
	{
		Lock l(cs);
		auto it = favoritesNoCid.find(nick);
		if(it!= favoritesNoCid.end())
		{
			fire(FavoriteManagerListener::FavoriteIRemoved(),nick, it->second);
			favoritesNoCid.erase(it);
			save();
		}
	}
	typedef map<string /*IP*/,FavoriteUser*> iplist;
	iplist ips;

	iplist getListIp() {Lock l(cs); return ips;}

	void addFavoriteIp(const string& ip, time_t lastSeen = time(NULL), int type = FavoriteUser::Flags::FLAG_IP)
	{
		Lock l(cs);

		if(ips.find(ip) == ips.end())
		{
			FavoriteUser* fav = new FavoriteUser(type);
			fav->setIp(ip);
			fav->setLastSeen(lastSeen);
			ips.insert(make_pair(ip,fav));
			save();
		}
	}
	void remFavoriteIp(const string& iporange)
	{
		iplist::const_iterator it = ips.find(iporange);
		if(it == ips.end() || it == ips.begin())
		ips.erase(it);
		save();
	}


// Favorite Hubs
	const FavoriteHubEntryList& getFavoriteHubs() const { return favoriteHubs; }
	FavoriteHubEntryList& getFavoriteHubs() { return favoriteHubs; }

	void addFavorite(const FavoriteHubEntry& aEntry);
	void removeFavorite(FavoriteHubEntry* entry);
	bool isFavoriteHub(const std::string& aUrl);
	FavoriteHubEntryPtr getFavoriteHubEntry(const string& aServer) const;
	string getAwayMessage(const string& aServer, ParamMap& params);

// Favorite hub groups
	const FavHubGroups& getFavHubGroups() const { return favHubGroups; }
	void setFavHubGroups(const FavHubGroups& favHubGroups_) { favHubGroups = favHubGroups_; }

	FavoriteHubEntryList getFavoriteHubs(const string& group) const;
// Recent Hubs
	void addRecent(const RecentHubEntry& aEntry);
	void removeRecent(const RecentHubEntry* entry);
	void updateRecent(const RecentHubEntry* entry);
	RecentHubEntry* getRecentHubEntry(const string& aServer) {
		for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
			RecentHubEntry* r = *i;
			if(Util::stricmp(r->getServer(), aServer) == 0) {
				return r;
			}
		}
		return (nullptr);
	}
	RecentHubEntry::List getRecentHubs() { return recentHubs;}
// Favorite Directories
	bool addFavoriteDir(const string& aDirectory, const string& aName);
	bool removeFavoriteDir(const string& aName);
	bool renameFavoriteDir(const string& aName, const string& anotherName);
	StringPairList getFavoriteDirs() { return favoriteDirs; }

// User Commands
	UserCommand addUserCommand(int type, int ctx, int flags, const string& name, const string& command, const string& to, const string& hub);
	bool getUserCommand(int cid, UserCommand& uc);
	int findUserCommand(const string& aName, const string& aUrl);
	bool moveUserCommand(int cid, int pos);
	void updateUserCommand(const UserCommand& uc);
	void removeUserCommand(int cid);
	void removeUserCommand(const string& srv);
	void removeHubUserCommands(int ctx, const string& hub);

	UserCommand::List getUserCommands() { Lock l(cs); return userCommands; }
	UserCommand::List getUserCommands(int ctx, const StringList& hub);
    //Raw Manager
	bool getEnabledAction(FavoriteHubEntry* entry, int actionId);
	void setEnabledAction(FavoriteHubEntry* entry, int actionId, bool enabled);
	bool getEnabledRaw(FavoriteHubEntry* entry, int actionId, unsigned int rawId);
	void setEnabledRaw(FavoriteHubEntry* entry, int actionId, unsigned int rawId, bool enabled);

	void load();
	void save();

	void removeallRecent() {
		recentHubs.clear();
		recentsave();
	}

	void mergeHubSettings(const FavoriteHubEntry& entry, HubSettings& settings) const;

private:
	FavoriteHubEntryList favoriteHubs;
	RecentHubEntry::List recentHubs;
	FavHubGroups favHubGroups;
	StringPairList favoriteDirs;
	UserCommand::List userCommands;
	FavoriteMap users;

	mutable CriticalSection cs;

	// Public Hubs
	typedef unordered_map<string, HubEntryList> PubListMap;
	PubListMap publicListMatrix;
	HttpConnection* c;
	HubTypes listType;
	StringMap blacklist;
	string downloadBuf;
	string publicListServer;
	int lastServer;
	int lastId;
	bool useHttp, running;
	/** Used during loading to prevent saving. */
	bool dontSave;

	friend class Singleton<FavoriteManager>;

	FavoriteManager();
	virtual ~FavoriteManager();

	FavoriteHubEntryList::iterator getFavoriteHub(const string& aServer);
	RecentHubEntry::Iter getRecentHub(const string& aServer) const;

	// ClientManagerListener
	void on(UserUpdated, const OnlineUser& user) noexcept;
	void on(UserConnected, const UserPtr& user) noexcept;
	void on(UserDisconnected, const UserPtr& user) noexcept;

	// HttpConnectionListener
	void on(Data, HttpConnection*, const uint8_t*, size_t) noexcept;
	void on(Failed, HttpConnection*, const string&) noexcept;
	void on(Complete, HttpConnection*, const string&, bool) noexcept;
	void on(Redirected, HttpConnection*, const string&) noexcept;
	void on(Retried, HttpConnection*, bool) noexcept;

	bool onHttpFinished(bool fromHttp) noexcept;

	// SettingsManagerListener
	void on(SettingsManagerListener::Load, SimpleXML& xml) {
		load(xml);
	}

	void load(SimpleXML& aXml);
	void recentload(SimpleXML& aXml);
	void recentsave();

	static string getConfigFile() { return Util::getPath(Util::PATH_USER_CONFIG) + "Favorites.xml"; }
};

} // namespace dcpp

#endif // !defined(FAVORITE_MANAGER_H)
