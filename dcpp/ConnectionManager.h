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

#ifndef DCPLUSPLUS_DCPP_CONNECTION_MANAGER_H
#define DCPLUSPLUS_DCPP_CONNECTION_MANAGER_H

#include "TimerManager.h"

#include "UserConnectionListener.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "Util.h"
#include "BufferedSocket.h"
#include "ConnectionManagerListener.h"
#include "HintedUser.h"

namespace dcpp {

using std::unique_ptr;

class SocketException;

class ConnectionQueueItem  {
public:
	typedef ConnectionQueueItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum State {
		CONNECTING,					// Recently sent request to connect
		WAITING,					// Waiting to send request to connect
		NO_DOWNLOAD_SLOTS,			// Not needed right now
		ACTIVE						// In one up/downmanager
	};

	ConnectionQueueItem(const HintedUser& aUser, bool aDownload) : token(Util::toString(Util::rand())),
		lastAttempt(0), errors(0), state(WAITING), download(aDownload), user(aUser) { }

	GETSET(string, token, Token);
	GETSET(uint64_t, lastAttempt, LastAttempt);
	GETSET(int, errors, Errors); // Number of connection errors, or -1 after a protocol error
	GETSET(State, state, State);
	GETSET(bool, download, Download);

	const HintedUser& getUser() const { return user; }
	ConnectionQueueItem(const ConnectionQueueItem&) = delete;
	ConnectionQueueItem& operator=(const ConnectionQueueItem&) = delete;

private:
	HintedUser user;
};

class ExpectedMap {
public:
	void add(const string& aNick, const string& aMyNick, const string& aHubUrl) {
		Lock l(cs);
		expectedConnections.insert(make_pair(aNick, make_pair(aMyNick, aHubUrl)));
	}

	StringPair remove(const string& aNick) {
		Lock l(cs);
		auto i = expectedConnections.find(aNick);

		if(i == expectedConnections.end())
			return make_pair(Util::emptyString, Util::emptyString);

		StringPair tmp = i->second;
		expectedConnections.erase(i);

		return tmp;
	}

private:
	/** Nick -> myNick, hubUrl for expected NMDC incoming connections */
	typedef map<string, StringPair> ExpectMap;
	ExpectMap expectedConnections;

	CriticalSection cs;
};

// Comparing with a user...
inline bool operator==(ConnectionQueueItem::Ptr ptr, const UserPtr& aUser) { return ptr->getUser() == aUser; }

class ConnectionManager : public Speaker<ConnectionManagerListener>,
	public UserConnectionListener, TimerManagerListener,
	public Singleton<ConnectionManager>
{
	///'cos clang
private:
	using UserConnectionListener::on;
	using TimerManagerListener::on;	
public:
	void nmdcExpect(const string& aNick, const string& aMyNick, const string& aHubUrl) {
		expectedConnections.add(aNick, aMyNick, aHubUrl);
	}

	void nmdcConnect(const string& aServer, const uint16_t& aPort, const string& aMyNick, const string& hubUrl, const string& encoding);
	void adcConnect(const OnlineUser& aUser, const uint16_t& aPort, const string& aToken, bool secure);
	void adcConnect(const OnlineUser& aUser, const uint16_t& aPort, const uint16_t& localPort, BufferedSocket::NatRoles natRole, const string& aToken, bool secure);

	void getDownloadConnection(const HintedUser& aUser);
	void force(const UserPtr& aUser);

	void disconnect(const UserPtr& aUser); // disconnect downloads and uploads
	void disconnect(const UserPtr& aUser, bool isDownload);
	void disconnectAll(); // disconnect all transfers for all users
	
	void shutdown();

	/** Find a suitable port to listen on, and start doing it */
	void listen();
	void disconnect() noexcept;

	uint16_t getPort() const;
	uint16_t getSecurePort() const;

private:

	class Server : public Thread {
	public:
		Server(bool secure, const uint16_t& port_, const string& ip, const string& ipv6,bool nmdc_ = false);
		virtual ~Server() { die = true; join(); }

		uint16_t getPort() const { return port; }

	private:
		virtual int run() noexcept;

		Socket sock;
		uint16_t port;
		bool secure;
		bool die;
		bool nmdc;
	};
	friend class Server;

	CriticalSection cs;

	/** All ConnectionQueueItems */
	ConnectionQueueItem::List downloads;
	ConnectionQueueItem::List uploads;

	/** All active connections */
	UserConnectionList userConnections;
	
	typedef unordered_map<string, uint64_t> delayMap;
	typedef delayMap::iterator delayIter;
	delayMap delayedTokens;

	StringList features;
	StringList adcFeatures;

	ExpectedMap expectedConnections;

	uint32_t floodCounter;

	unique_ptr<Server> server;
	unique_ptr<Server> secureServer;
	
	unordered_set<string> ddosCTM2HUBs;

	bool shuttingDown;

	friend class Singleton<ConnectionManager>;
	ConnectionManager();

	virtual ~ConnectionManager() { shutdown(); }

	UserConnection* getConnection(bool aNmdc, bool secure) noexcept;
	void putConnection(UserConnection* aConn);

	void addUploadConnection(UserConnection* uc);
	void addDownloadConnection(UserConnection* uc);

	ConnectionQueueItem* getCQI(const HintedUser& aUser, bool download);
	void putCQI(ConnectionQueueItem* cqi);

	void accept(const Socket& sock, bool secure,bool nmdc = false) noexcept;

	bool checkKeyprint(UserConnection *aSource);
	
	/*https://bugs.launchpad.net/dcplusplus/+bug/1419478*/
	bool checkCTM2HUB(const string& aServer, const uint16_t& aPort, const string& aHubUrl);

	void failed(UserConnection* aSource, const string& aError, bool protocolError);

	// UserConnectionListener
	virtual void on(Connected, UserConnection*) noexcept;
	virtual void on(Failed, UserConnection*, const string&) noexcept;
	virtual void on(ProtocolError, UserConnection*, const string&) noexcept;
	virtual void on(CLock, UserConnection*, const string&, const string&) noexcept;
	virtual void on(Key, UserConnection*, const string&) noexcept;
	virtual void on(Direction, UserConnection*, const string&, const string&) noexcept;
	virtual void on(MyNick, UserConnection*, const string&) noexcept;
	virtual void on(Supports, UserConnection*, const StringList&) noexcept;

	virtual void on(AdcCommand::SUP, UserConnection*, const AdcCommand&) noexcept;
	virtual void on(AdcCommand::INF, UserConnection*, const AdcCommand&) noexcept;
	virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) noexcept;

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, uint64_t aTick) noexcept;
	virtual void on(TimerManagerListener::Minute, uint64_t aTick) noexcept;

};

} // namespace dcpp

#endif // !defined(CONNECTION_MANAGER_H)
