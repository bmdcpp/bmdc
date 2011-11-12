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

#ifndef DCPLUSPLUS_DCPP_ONLINEUSER_H_
#define DCPLUSPLUS_DCPP_ONLINEUSER_H_

#include <map>

#include <boost/noncopyable.hpp>

#include "forward.h"
#include "Flags.h"
#include "FastAlloc.h"
#include "Util.h"
#include "User.h"

#include "TimerManager.h"

namespace dcpp {

/** One of possibly many identities of a user, mainly for UI purposes */
class Identity : public Flags {
public:
	enum IdentityFlagBits {
		GOT_INF_BIT,
		NMDC_PASSIVE_BIT
	};
	enum IdentityFlags {
		GOT_INF = 1 << GOT_INF_BIT,
		NMDC_PASSIVE = 1 << NMDC_PASSIVE_BIT
	};
	enum ClientType {
		CT_BOT = 1,
		CT_REGGED = 2,
		CT_OP = 4,
		CT_SU = 8,
		CT_OWNER = 16,
		CT_HUB = 32,
		CT_HIDDEN = 64
	};

	Identity() : sid(0) { }
	Identity(const UserPtr& ptr, uint32_t aSID) : user(ptr), sid(aSID) { }
	Identity(const Identity& rhs) : Flags(), sid(0) { *this = rhs; } // Use operator= since we have to lock before reading...
	Identity& operator=(const Identity& rhs) { FastLock l(cs); *static_cast<Flags*>(this) = rhs; user = rhs.user; sid = rhs.sid; info = rhs.info; return *this; }

// GS is already defined on some systems (e.g. OpenSolaris)
#ifdef GS
#undef GS
#endif

#define GS(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
	GS(Nick, "NI")
	GS(Description, "DE")
	GS(Ip4, "I4")
	GS(Ip6, "I6")
	GS(Udp4Port, "U4")
	GS(Udp6Port, "U6")
	GS(Email, "EM")
	GS(ClientType, "CL")
	GS(MyInfoType, "MT")
	GS(cheats, "CS")
	GS(TestSURChecked, "TC")
	GS(FileListChecked, "FC")
	GS(FileListQueued, "FQ")
	GS(TestSURQueued, "TQ")
	GS(FileListComplete, "CF")
	GS(TestSURComplete, "TK")

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
	bool isBot() const { return isClientType(CT_BOT) || isSet("BO"); }
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
    void checkTagState(OnlineUser& ou);
    string getDetectionField(const string& aName) const;
    string myInfoDetect(OnlineUser& ou);
    string updateClientType(OnlineUser& ou);
    map<string, string> getReport() const;
    
    bool isFileListQueued() const { return isSet("FQ"); }
	bool isClientChecked() const { return isSet("TC"); }
	bool isFileListChecked() const { return isSet("FC"); }
	bool isClientQueued() const { return isSet("TQ"); }
    
	void getParams(ParamMap& params, const string& prefix, bool compatibility) const;
	UserPtr& getUser() { return user; }
	GETSET(UserPtr, user, User);
	GETSET(uint32_t, sid, SID);
	GETSET(uint64_t, loggedIn, LoggedIn); 
private:
	typedef std::unordered_map<short, string> InfMap;
	typedef InfMap::iterator InfIter;
	InfMap info;

	static FastCriticalSection cs;

	string getFilelistGeneratorVer() const;
	string getPkVersion() const;
	void getDetectionParams(ParamMap& p);
};

class OnlineUser : public FastAlloc<OnlineUser>, public intrusive_ptr_base<OnlineUser>, private boost::noncopyable {
public:
	typedef vector<OnlineUser*> List;
	typedef List::iterator Iter;

	OnlineUser(const UserPtr& ptr, Client& client_, uint32_t sid_);

	operator UserPtr&() { return getUser(); }
	operator const UserPtr&() const { return getUser(); }

	UserPtr& getUser() { return getIdentity().getUser(); }
	const UserPtr& getUser() const { return getIdentity().getUser(); }
	Identity& getIdentity() { return identity; }
	Client& getClient() { return client; }
	const Client& getClient() const { return client; }
	//CMD
	string setCheat(const string& aCheat, bool aBadClient, bool aBadFilelist = false, bool aDisplayCheat = true)
	{
        return identity.setCheat(getClient(),aCheat,aBadClient, aBadFilelist, aDisplayCheat);
	}
	inline bool isProtectedUser(bool checkOp = true) const {
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


	GETSET(Identity, identity, Identity);
private:
	friend class NmdcHub;

	Client& client;
};

}

#endif /* ONLINEUSER_H_ */
