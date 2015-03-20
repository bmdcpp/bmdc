/*
 * Copyright (C) 2001-2015 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CLIENT_H
#define DCPLUSPLUS_DCPP_CLIENT_H

#include "compiler.h"

#include <atomic>

#include "forward.h"
#include "Speaker.h"

#include "BufferedSocketListener.h"
#include "TimerManager.h"
#include "ClientListener.h"
#include "OnlineUser.h"
#include "CommandQueue.h"
#include "PluginEntity.h"
#include "HubSettings.h"



namespace dcpp {

using std::atomic;
/** Yes, this should probably be called a Hub */
class Client :
	public PluginEntity<HubData>,
	public Speaker<ClientListener>,
	public BufferedSocketListener,
	protected TimerManagerListener,
	public HubSettings
{
public:
	virtual void connect();
	virtual void disconnect(bool graceless);

	virtual void connect(const OnlineUser& user, const string& token) = 0;
	virtual void hubMessage(const string& aMessage, bool thirdPerson = false) = 0;
	virtual void privateMessage(const OnlineUser& user, const string& aMessage, bool thirdPerson = false) = 0;
	virtual void sendUserCmd(const UserCommand& command, const ParamMap& params) = 0;
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList) = 0;
	virtual void password(const string& pwd) = 0;
	
	/** Send new information about oneself. Thread-safe. */
	void info();

	virtual void emulateCommand(const string& cmd) = 0;
	virtual size_t getUserCount() const = 0;
	virtual int64_t getAvailable() const = 0;
	virtual void getUserList(OnlineUserList& list) const = 0;
	virtual void refreshuserlist() = 0;

	virtual string startCheck(const string &params) = 0;
	virtual void startMyInfoCheck() = 0;
	virtual void stopMyInfoCheck() = 0;
	virtual void stopChecking() = 0;

	virtual void send(const AdcCommand& command) = 0;

	bool isConnected() const { return state != STATE_CONNECTING && state != STATE_DISCONNECTED; }
	bool isSecure() const;
	bool isTrusted() const;
	string getCipherName() const;
	vector<uint8_t> getKeyprint() const;

	bool isOp() const { return getMyIdentity().isOp(); }

	const uint16_t& getPort() const { return port; }
	const string& getAddress() const { return address; }

	const string& getIp() const { return ip; }
	string getIpPort() const { return getIp() + ':' + Util::toString(port); }

	void updated(const OnlineUser& aUser) { fire(ClientListener::UserUpdated(), this, aUser); }

	void cheatMessage(const string& message) { fire(ClientListener::CheatMessage(), this, message); }

	static string getCounts();
	static int getTotalCounts() {
		return counts[COUNT_NORMAL].load() + counts[COUNT_REGISTERED].load() + counts[COUNT_OP].load();
	}

	void reconnect();
	void shutdown();

	void send(const string& aMessage) { send(aMessage.c_str(), aMessage.length()); }
	void send(const char* aMessage, size_t aLen);
	void sendActionCommand(const OnlineUser& ou, int actionId);
	bool isActionActive(const int aAction) const;

	string getMyNick() const { return getMyIdentity().getNick(); }
	string getHubName() const { return getHubIdentity().getNick().empty() ? getHubUrl() : getHubIdentity().getNick(); }
	string getHubDescription() const { return getHubIdentity().getDescription(); }

	Identity& getHubIdentity() { return hubIdentity; }
	
	virtual UserPtr findUserWithCID(const CID& cid) = 0;

	const string& getHubUrl() const { return hubUrl; }

	bool isActive() const;
	bool isActiveV4() const;
	bool isActiveV6() const; 
	void putDetectors() { stopMyInfoCheck(); stopChecking();  }

	HubData* getPluginObject() noexcept;

	GETSET(Identity, myIdentity, MyIdentity);
	GETSET(Identity, hubIdentity, HubIdentity);

	GETSET(string, defpassword, Password);
	GETSET(uint32_t, reconnDelay, ReconnDelay);
	GETSET(uint64_t, lastActivity, LastActivity);
	GETSET(bool, registered, Registered);
	GETSET(bool, autoReconnect, AutoReconnect);
	GETSET(string, encoding, Encoding);

	//BMDC++
	GETSET(bool, hideShare, HideShare);
	GETSET(string, chatExtraInfo, ChatExtraInfo);
	GETSET(string, protectUser, ProtectUser);
	GETSET(string, tabText, TabText);
	GETSET(string, tabIconStr, TabIconStr);

	GETSET(bool, checkAtConnect, CheckAtConnect);
	GETSET(bool, checkClients, CheckClients);
	GETSET(bool, checkFilelists, CheckFilelists);
	mutable CriticalSection cs; //BMDC++//RSX++
protected:
	friend class ClientManager;
	Client(const string& hubURL, char separator, bool secure_);
	virtual ~Client();

	enum CountType {
		COUNT_NORMAL,
		COUNT_REGISTERED,
		COUNT_OP,
		COUNT_UNCOUNTED,
	};

	static atomic<long> counts[COUNT_UNCOUNTED];

	enum States {
		STATE_CONNECTING,	///< Waiting for socket to connect
		STATE_PROTOCOL,		///< Protocol setup
		STATE_IDENTIFY,		///< Nick setup
		STATE_VERIFY,		///< Checking password
		STATE_NORMAL,		///< Running
		STATE_DISCONNECTED,	///< Nothing in particular
	} state;

	BufferedSocket *sock;

	void updateCounts(bool aRemove);
	void updateActivity() { lastActivity = GET_TICK(); }

	/** Reload details from favmanager or settings */
	void reloadSettings(bool updateNick);
	/// Get the external IP the user has defined for this hub, if any.
	const string& getUserIp() const;
	const string& getUserIp4() const;
	const string getUserIp6() const; 

	virtual void checkNick(string& nick) = 0;

	// TimerManagerListener
	virtual void on(Second, uint64_t aTick) noexcept;
	// BufferedSocketListener
	virtual void on(Connecting) noexcept { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) noexcept;
	virtual void on(Line, const string& aLine) noexcept;
	virtual void on(Failed, const string&) noexcept;

	virtual bool v4only() const = 0;
private:
	virtual void infoImpl() = 0;

	string hubUrl;
	string address;
	string ip;
	string keyprint;
	uint16_t port;
	char separator;
	bool secure;
	CountType countType;

	CommandQueue cmdQueue;

	Client(const Client&);
	Client& operator=(const Client&);
	
};

} // namespace dcpp

#endif // !defined(CLIENT_H)
