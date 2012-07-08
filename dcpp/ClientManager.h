/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CLIENT_MANAGER_H
#define DCPLUSPLUS_DCPP_CLIENT_MANAGER_H

#include <unordered_map>
#include <unordered_set>

#include "TimerManager.h"

#include "Singleton.h"
#include "OnlineUser.h"
#include "Socket.h"
#include "CID.h"
#include "ClientListener.h"
#include "ClientManagerListener.h"
#include "HintedUser.h"
#include "Client.h"
#include "DirectoryListing.h"

namespace dcpp {

using std::pair;
using std::unordered_map;
using std::unordered_multimap;
using std::unordered_set;

class UserCommand;

class ClientManager : public Speaker<ClientManagerListener>,
	private ClientListener, public Singleton<ClientManager>,
	private TimerManagerListener
{
public:
	typedef unordered_set<Client*> ClientList;
	typedef unordered_map<CID, UserPtr> UserMap;

	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	size_t getUserCount() const;
	int64_t getAvailable() const;

	StringList getHubs(const CID& cid, const string& hintUrl);
	StringList getHubNames(const CID& cid, const string& hintUrl);
	StringList getNicks(const CID& cid, const string& hintUrl);
	string getField(const CID& cid, const string& hintUrl, const char* field) const;

	StringList getHubs(const CID& cid, const string& hintUrl, bool priv);
	StringList getHubNames(const CID& cid, const string& hintUrl, bool priv);
	StringList getNicks(const CID& cid, const string& hintUrl, bool priv);

	StringList getNicks(const HintedUser& user) { return getNicks(user.user->getCID(), user.hint); }
	StringList getHubNames(const HintedUser& user) { return getHubNames(user.user->getCID(), user.hint); }
	StringList getHubs(const HintedUser& user) { return getHubs(user.user->getCID(), user.hint); }

	vector<Identity> getIdentities(const UserPtr &u) const;

	string getConnection(const CID& cid) const;

	bool isConnected(const string& aUrl) const;

	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(string& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList);
	void infoUpdated();

	UserPtr getUser(const string& aNick, const string& aHubUrl) noexcept;
	UserPtr getUser(const CID& cid) noexcept;
	string findMySID(const HintedUser& p);

	string findHub(const string& ipPort) const;
	string findHubEncoding(const string& aUrl) const;

	/**
	* @param priv discard any user that doesn't match the hint.
	* @return OnlineUser* found by CID and hint; might be only by CID if priv is false.
	*/
	OnlineUser* findOnlineUser(const HintedUser& user);
	OnlineUser* findOnlineUser(const CID& cid, const string& hintUrl);

	UserPtr findUser(const string& aNick, const string& aHubUrl) const noexcept { return findUser(makeCid(aNick, aHubUrl)); }
	UserPtr findUser(const CID& cid) const noexcept;
	UserPtr findLegacyUser(const string& aNick) const noexcept;

	bool isOnline(const UserPtr& aUser) const {
		Lock l(cs);
		return onlineUsers.find(aUser->getCID()) != onlineUsers.end();
	}

	bool isOp(const UserPtr& aUser, const string& aHubUrl) const;

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) const noexcept;

	void putOnline(OnlineUser* ou) noexcept;
	void putOffline(OnlineUser* ou, bool disconnect = false) noexcept;

	UserPtr& getMe();

	void send(AdcCommand& c, const CID& to);

	void connect(const HintedUser& user, const string& token);
	void privateMessage(const HintedUser& user, const string& msg, bool thirdPerson);
	void userCommand(const HintedUser& user, const UserCommand& uc, ParamMap& params, bool compatibility);
	Lock lock() { return Lock(cs); }

	const ClientList& getClients() const { return clients; }
	const UserMap& getUsers() const { return users; }
	void getOnlineClients(StringList& onlineClients);

	CID getMyCID();
	const CID& getMyPID();

	void loadUsers();
	void saveUsers() const;
	void saveUser(const CID& cid);

	bool getSharingHub(const HintedUser& p) {
		Lock l(cs);
		OnlineUser* ou = findOnlineUserHint(p.user->getCID(), p.hint);
		if(ou)
			return (!ou->getClient().getHideShare());
		return true;
	}
	
	
	int getMode(const string& aHubUrl) const;
	bool isActive(const string& aHubUrl = Util::emptyString) const;
	//CMD
	void setIpAddress(const UserPtr& p, const string& ip);
	void setSupports(const UserPtr& p, const string& aSupports);
	void setGenerator(const HintedUser& p, const string& aGenerator, const string& aCID, const string& aBase);
	void setPkLock(const HintedUser& p,const string& aPk, const string& aLock);
	void setUnknownCommand(const UserPtr& p, const string& aUnknownCommand);
	void setListSize(const UserPtr& p, int64_t aFileLength, bool adc);
	void setListLength(const UserPtr& p, const string& listLen);

	void sendAction(const UserPtr& p, const int aAction);
	void sendAction(OnlineUser& ou, const int aAction);
	void sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection = false);
	
	void addCheckToQueue(const HintedUser hintedUser, bool filelist);
	void checkCheating(const HintedUser& p, DirectoryListing* dl);
	void setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete);
	void fileListDisconnected(const UserPtr& p);
	
private:
	typedef unordered_map<string, UserPtr> LegacyMap;
	typedef LegacyMap::iterator LegacyIter;

	typedef UserMap::iterator UserIter;

	typedef std::pair<std::string, bool> NickMapEntry; // the boolean being true means "save this".
	typedef unordered_map<CID, NickMapEntry> NickMap;

	typedef unordered_multimap<CID, OnlineUser*> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;
	typedef pair<OnlineIter, OnlineIter> OnlinePair;
	typedef pair<OnlineIterC, OnlineIterC> OnlinePairC;

	ClientList clients;
	mutable CriticalSection cs;

	UserMap users;
	OnlineMap onlineUsers;
	NickMap nicks;

	UserPtr me;

	Socket udp;

	CID pid;

	friend class Singleton<ClientManager>;

	ClientManager();
	virtual ~ClientManager();

	void updateNick(const OnlineUser& user) noexcept;

	/// @return OnlineUser* found by CID and hint; discard any user that doesn't match the hint.
	OnlineUser* findOnlineUserHint(const CID& cid, const string& hintUrl) const {
		OnlinePairC p;
		return findOnlineUserHint(cid, hintUrl, p);
	}
	/**
	* @param p OnlinePair of all the users found by CID, even those who don't match the hint.
	* @return OnlineUser* found by CID and hint; discard any user that doesn't match the hint.
	*/
	OnlineUser* findOnlineUserHint(const CID& cid, const string& hintUrl, OnlinePairC& p) const;

	string getUsersFile() const { return Util::getPath(Util::PATH_USER_LOCAL) + "Users.xml"; }

	// ClientListener
	virtual void on(Connected, Client* c) noexcept;
	virtual void on(UserUpdated, Client*, const OnlineUser& user) noexcept;
	virtual void on(UsersUpdated, Client* c, const OnlineUserList&) noexcept;
	virtual void on(Failed, Client*, const string&) noexcept;
	virtual void on(HubUpdated, Client* c) noexcept;
	virtual void on(HubUserCommand, Client*, int, int, const string&, const string&) noexcept;
	virtual void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
		int aFileType, const string& aString) noexcept;
	virtual void on(AdcSearch, Client* c, const AdcCommand& adc, const CID& from) noexcept;
	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, uint64_t aTick) noexcept;
};

} // namespace dcpp

#endif // !defined(CLIENT_MANAGER_H)
