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

#ifndef DCPLUSPLUS_DCPP_ONLINEUSER_H_
#define DCPLUSPLUS_DCPP_ONLINEUSER_H_

#include <map>
#include "forward.h"
#include "Flags.h"
#include "Util.h"
#include "User.h"
#include "PluginEntity.h"
#include "TimerManager.h"

namespace dcpp {

/** One of possibly many identities of a user, mainly for UI purposes */
class Identity : public Flags {
public:
	 
	enum ClientType {
		CT_BOT = 1,
		CT_REGGED = 2,
		CT_OP = 4,
		CT_SU = 8,
		CT_OWNER = 16,
		CT_HUB = 32,
		CT_HIDDEN = 64
	};

	Identity() : sid(0) , loggedIn(0) { }
	Identity(const UserPtr& ptr, uint32_t aSID) : user(ptr), sid(aSID), loggedIn(0) { }
	Identity(const Identity& rhs) : Flags(), sid(0), loggedIn(0) { *this = rhs; } // Use operator= since we have to lock before reading...
	Identity& operator=(const Identity& rhs) { FastLock l(cs); *static_cast<Flags*>(this) = rhs; user = rhs.user; sid = rhs.sid; info = rhs.info; return *this; }

#define GESET(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }

	GESET(Nick, "NI")
	GESET(Description, "DE")
	GESET(Ip4, "I4")
	GESET(Ip6, "I6")
	GESET(Udp4Port, "U4")
	GESET(Udp6Port, "U6")
	GESET(Email, "EM")
	GESET(ClientType, "CL")
	GESET(MyInfoType, "MT")
	GESET(cheats, "CS")
	GESET(TestSURChecked, "TC")
	GESET(FileListChecked, "FC")
	GESET(FileListQueued, "FQ")
	GESET(TestSURQueued, "TQ")
	GESET(FileListComplete, "CF")
	GESET(TestSURComplete, "TK")

	void setBytesShared(const string& bs) { set("SS", bs); }
	int64_t getBytesShared() const { return Util::toInt64(get("SS")); }

	void setOp(bool op) { set("OP", op ? "1" : Util::emptyString); }
	void setHub(bool hub) { set("HU", hub ? "1" : Util::emptyString); }
	void setBot(bool bot) { set("BO", bot ? "1" : Util::emptyString); }
	void setHidden(bool hidden) { set("HI", hidden ? "1" : Util::emptyString); }
	string getTag() const;
	string getApplication() const;
	string getConnection() const;
	const string& getCountry() const;
	bool supports(const string& name) const;
	bool isHub() const { return isClientType(CT_HUB) || isSet("HU"); }
	bool isOp() const { return isClientType(CT_OP) || isClientType(CT_SU) || isClientType(CT_OWNER) || isSet("OP"); }
	bool isRegistered() const { return isClientType(CT_REGGED) || isSet("RG"); }
	bool isHidden() const { return isClientType(CT_HIDDEN) || isSet("HI"); }
	bool isBot() const { return isClientType(CT_HUB) ||isClientType(CT_BOT) || isSet("BO"); }//bot fix
	bool isAway() const { return isSet("AW"); }
	bool isTcpActive(const Client *c = NULL) const;
	bool isTcp4Active() const;
	bool isTcp6Active() const;
	bool isUdpActive() const;
	bool isUdp4Active() const;
	bool isUdp6Active() const;
	string getIp() const;
	string getUdpPort() const;

	std::map<string, string> getInfo() const;
	string get(const char* name) const;
	void set(const char* name, const string& val);
	bool isSet(const char* name) const;
	string getSIDString() const { return string((const char*)&sid, 4); }

	bool isClientType(ClientType ct) const;

	void logDetection(bool successful);
	string setCheat(const Client& c,const string& aCheat, bool aBadClient, bool aBadFilelist = false, bool aDisplayCheat = true);
	string checkFilelistGenerator(OnlineUser& ou);
	bool isProtectedUser(const Client& c, bool OpBotHubCheck) const;
	void checkTagState(OnlineUser& ou) const;
	string getDetectionField(const string& aName) const;
	string myInfoDetect(OnlineUser& ou);
	string updateClientType(OnlineUser& ou);
	map<string, string> getReport() const;

	bool isFileListQueued() const { return isSet("FQ"); }
	bool isClientChecked() const { return isSet("TC"); }
	bool isFileListChecked() const { return isSet("FC"); }
	bool isClientQueued() const { return isSet("TQ"); }

	void getParams(ParamMap& params, const string& prefix, bool compatibility) const;
	//UserPtr& getUser() { return user; }
	GETSET(UserPtr, user, User);
	GETSET(uint64_t, loggedIn, LoggedIn);
	GETSET(uint32_t, sid, SID);
private:
	typedef std::unordered_map<short, string> InfMap;
	typedef InfMap::iterator InfIter;
	InfMap info;

	static FastCriticalSection cs;

	string getFilelistGeneratorVer() const;
	string getPkVersion() const;
	void getDetectionParams(ParamMap& p);
};

class OnlineUser : public PluginEntity<UserData> {
public:
	typedef vector<OnlineUser*> List;
	typedef List::iterator Iter;

	OnlineUser(const UserPtr& ptr, Client& client_, uint32_t sid_);

	operator UserPtr/*&*/() { return getUser(); }
	operator const UserPtr/*&*/() const { return getUser(); }

	UserPtr/*&*/ getUser() { return identity.getUser(); }
	const UserPtr/*&*/ getUser() const { return identity.getUser(); }
	Identity& getIdentity() { return identity; }
	Client& getClient() { return client; }
	const Client& getClient() const { return client; }
	//CMD
	string setCheat(const string& aCheat, bool aBadClient, bool aBadFilelist = false, bool aDisplayCheat = true)
	{
        return identity.setCheat(client,aCheat, aBadClient, aBadFilelist, aDisplayCheat);
	}

	bool isProtectedUser(bool checkOp = true) const {
		return identity.isProtectedUser(getClient(), checkOp);
	}

	bool isCheckable(uint32_t delay = 0);

	inline bool shouldCheckClient() const {
		if(identity.isClientChecked() || identity.isClientQueued())
			return false;
		return true;
	}
	inline bool shouldCheckFileList() const {
		if(identity.isFileListQueued() || identity.isFileListChecked() || identity.isClientQueued())
			return false;
		return (GET_TIME() - Util::toInt64(identity.getTestSURChecked()) > 10);
	}

	inline void initializeData() {
		identity.setLoggedIn(GET_TICK());
		identity.set("LT", Util::formatTime("%d-%m %H:%M", GET_TIME()));
	}

	bool getChecked(bool filelist = false, bool checkComplete = true);

	UserData* getPluginObject() noexcept;
	GETSET(Identity, identity, Identity);
private:

	Client& client;
	OnlineUser(OnlineUser&);
	OnlineUser operator=(OnlineUser&);
};

}

#endif /* ONLINEUSER_H_ */
