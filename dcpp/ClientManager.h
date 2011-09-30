/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#include "TimerManager.h"

#include "Client.h"
#include "Singleton.h"
#include "SettingsManager.h"
#include "User.h"
#include "Socket.h"
#include "DirectoryListing.h"
#include <unordered_map>
#include <unordered_set>
#include "ClientManagerListener.h"

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

	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);
	///RSX++
	virtual ~ClientManager() throw() {
		TimerManager::getInstance()->removeListener(this);
	}

	typedef unordered_multimap<CID, OnlineUser*> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;
	///END

	size_t getUserCount() const;
	int64_t getAvailable() const;

	StringList getHubs(const CID& cid, const string& hintUrl);
	StringList getHubNames(const CID& cid, const string& hintUrl);
	StringList getNicks(const CID& cid, const string& hintUrl);

	StringList getHubs(const CID& cid, const string& hintUrl, bool priv);
	StringList getHubNames(const CID& cid, const string& hintUrl, bool priv);
	StringList getNicks(const CID& cid, const string& hintUrl, bool priv);
	string getField(const CID& cid, const string& hintUrl, const char* field) const;

	StringList getNicks(const HintedUser& user) { return getNicks(user.user->getCID(), user.hint); }
	StringList getHubNames(const HintedUser& user) { return getHubNames(user.user->getCID(), user.hint); }
	StringList getHubs(const HintedUser& user) { return getHubs(user.user->getCID(), user.hint); }
	
	string getConnection(const CID& cid) const;

	bool isConnected(const string& aUrl) const;

	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList);
	void infoUpdated();

	UserPtr getUser(const string& aNick, const string& aHubUrl) throw();
	UserPtr getUser(const CID& cid) throw();

	string findHub(const string& ipPort) const;
	string findHubEncoding(const string& aUrl) const;

	/**
	* @param priv discard any user that doesn't match the hint.
	* @return OnlineUser* found by CID and hint; might be only by CID if priv is false.
	*/
	OnlineUser* findOnlineUser(const HintedUser& user, bool priv);
	OnlineUser* findOnlineUser(const CID& cid, const string& hintUrl, bool priv) throw();

	UserPtr findUser(const string& aNick, const string& aHubUrl) const throw() { return findUser(makeCid(aNick, aHubUrl)); }
	UserPtr findUser(const CID& cid) const throw();
	UserPtr findLegacyUser(const string& aNick) const throw();

	bool isOnline(const UserPtr& aUser) const {
		Lock l(cs);
		return onlineUsers.find(aUser->getCID()) != onlineUsers.end();
	}

	bool getSharingHub(const UserPtr& p, const string& hubHint = Util::emptyString) {
		Client* c = 0;
		{
			Lock l(cs);
			OnlineUser* ou = findOnlineUser(p->getCID(), hubHint,false);
			if(!ou)
				return false;
			c = &ou->getClient();
		}
		return c ? c->getHideShare() : false;
	}

	bool isOp(const UserPtr& aUser, const string& aHubUrl) const;

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) const throw();

	void putOnline(OnlineUser* ou) throw();
	void putOffline(OnlineUser* ou, bool disconnect = false) throw();

	UserPtr& getMe();

	void send(AdcCommand& c, const CID& to);

	void connect(const HintedUser& user, const string& token);
	void privateMessage(const HintedUser& user, const string& msg, bool thirdPerson);
	void userCommand(const HintedUser& user, const UserCommand& uc, StringMap& params, bool compatibility);
	#ifdef _USELUA
		static bool ucExecuteLua(const string& cmd,StringMap& params);//add
	#endif
	int getMode(const string& aHubUrl) const;
	bool isActive(const string& aHubUrl = Util::emptyString) const { return getMode(aHubUrl) != SettingsManager::INCOMING_FIREWALL_PASSIVE; }

	void lock() throw() { cs.enter(); }
	void unlock() throw() { cs.leave(); }

	ClientList& getClients() { return clients; }

	CID getMyCID();
	const CID& getMyPID();

	void loadUsers();
	void saveUsers() const;
	void saveUser(const CID& cid);
	//RSX
	void setGenerator(const UserPtr& p, const string& aGenerator, const string& aCID, const string& aBase);
	// =P
	void sendAction(const UserPtr& p, const int aAction);
	void sendAction(OnlineUser& ou, const int aAction);
	void setPkLock(const UserPtr& p, const string& aPk, const string& aLock);
	void setListSize(const UserPtr& p, int64_t aFileLength, bool adc);
	void setSupports(const UserPtr& p, const string& aSupports);
	void setUnknownCommand(const UserPtr& p, const string& aUnknownCommand);
	void setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete);
	void fileListDisconnected(const UserPtr& p);
	void setListLength(const UserPtr& p, const string& listLen);
	void checkCheating(const UserPtr& p, DirectoryListing* dl);
	void sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection = false);
	void addCheckToQueue(const HintedUser huser, bool filelist);

private:
	typedef unordered_map<string, UserPtr> LegacyMap;
	typedef LegacyMap::iterator LegacyIter;

	typedef unordered_map<CID, UserPtr> UserMap;
	typedef UserMap::iterator UserIter;

	typedef std::pair<std::string, bool> NickMapEntry; // the boolean being true means "save this".
	typedef unordered_map<CID, NickMapEntry> NickMap;

	//typedef unordered_multimap<CID, OnlineUser*> OnlineMap;
	//typedef OnlineMap::iterator OnlineIter;
	//typedef OnlineMap::const_iterator OnlineIterC;
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

	ClientManager() : udp(Socket::TYPE_UDP) {
		TimerManager::getInstance()->addListener(this);
	}

	void updateNick(const OnlineUser& user) throw();

	/// @return OnlineUser* found by CID and hint; discard any user that doesn't match the hint.
	OnlineUser* findOnlineUser_hint(const CID& cid, const string& hintUrl) throw() {
		OnlinePair p;
		return findOnlineUser_hint(cid, hintUrl, p);
	}
	/**
	* @param p OnlinePair of all the users found by CID, even those who don't match the hint.
	* @return OnlineUser* found by CID and hint; discard any user that doesn't match the hint.
	*/
	OnlineUser* findOnlineUser_hint(const CID& cid, const string& hintUrl, OnlinePair& p) throw();

	string getUsersFile() const { return Util::getPath(Util::PATH_USER_LOCAL) + "Users.xml"; }

	// ClientListener
	virtual void on(Connected, Client* c) throw();
	virtual void on(UserUpdated, Client*, const OnlineUser& user) throw();
	virtual void on(UsersUpdated, Client* c, const OnlineUserList&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(HubUpdated, Client* c) throw();
	virtual void on(HubUserCommand, Client*, int, int, const string&, const string&) throw();
	virtual void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
		int aFileType, const string& aString) throw();
	virtual void on(AdcSearch, Client* c, const AdcCommand& adc, const CID& from) throw();
	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, uint64_t aTick) throw();
};

} // namespace dcpp

#endif // !defined(CLIENT_MANAGER_H)
