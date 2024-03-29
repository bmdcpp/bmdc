// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
// 

#ifndef USER_MANAGER
#define USER_MANAGER
#include "format.h"
#include "Singleton.h"
#include "Speaker.h"
#include "ConnectionManager.h"
#include "Client.h"
#include <unordered_map>
#include <unordered_set>

namespace dcpp
{
    
class UsersManagerListener
{   
    public:
	virtual ~UsersManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> UserConnected;
	typedef X<1> UserUpdated;
	typedef X<2> UserDisconnected;

	/** User online in at least one hub */
	virtual void on(UserConnected, const UserPtr&) noexcept { }
	virtual void on(UserUpdated, const OnlineUser&) noexcept { }
	/** User offline in all hubs */
	virtual void on(UserDisconnected, const UserPtr&) noexcept { }
};

class UsersManager: public Singleton<UsersManager>, public Speaker<UsersManagerListener>
{
	typedef map<CID,string> OnlineHubUserMap;
	typedef std::unordered_multimap<CID, OnlineUser*> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;
	typedef pair<OnlineIter, OnlineIter> OnlinePair;
	typedef pair<OnlineIterC, OnlineIterC> OnlinePairC;
	typedef std::pair<std::string, bool> NickMapEntry; // the boolean being true means "save this".
	typedef unordered_map<CID, NickMapEntry> NickMap;
	
	public:
	
	StringList getHubs(const CID& cid, const string& hintUrl = string()) 
	{
		Lock l(cs);
		StringList lst;
		OnlinePairC op = onlineUsers.equal_range(cid);
		for(OnlineIterC i = op.first; i != op.second; ++i) {
			if(i->second->getClient().getHubUrl() == hintUrl)
				lst.push_back(i->second->getClient().getHubUrl());
		}
		return lst;
	}
	
	StringList getHubNames(const CID& cid, const string& x = string()) {
		Lock l(cs);
		(void)x;//hide warns...
		StringList lst;
		OnlinePairC op = onlineUsers.equal_range(cid);
		for(OnlineIterC i = op.first; i != op.second; ++i) {
			lst.push_back(i->second->getClient().getHubName());
		}
		return lst;
	}	
	
	StringList getNicks(const CID& cid, const string& hintUrl) 
	{
		Lock l(cs);
		StringList ret;

		OnlinePairC op = onlineUsers.equal_range(cid);
		for(auto i = op.first; i != op.second; ++i) {
			if(i->second->getClient().getHubUrl() == hintUrl)
				ret.push_back(i->second->getIdentity().getNick());
		}

		if(ret.empty()) {
			// offline
			NickMap::const_iterator i = nicks.find(cid);
			if(i != nicks.end()) {
				ret.push_back(i->second.first);
			} else {
				ret.push_back('{' + cid.toBase32() + '}');
			}
		}

		return ret;
	}

vector<Identity> getIdentities(const UserPtr &u) const {
	Lock l(cs);
	auto op = onlineUsers.equal_range(u->getCID());
	vector<Identity> ret;
	for(auto i = op.first; i != op.second; ++i) {
		ret.emplace_back(i->second->getIdentity());
	}

	return ret;
}


string getConnection(const CID& cid) const {
	Lock l(cs);
	auto i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		return i->second->getIdentity().getConnection();
	}
	return _("Offline");
}


UserPtr findLegacyUser(const string& aNick) const noexcept {
	if (aNick.empty())
		return UserPtr();

	Lock l(cs);

	for(auto i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
		const OnlineUser* ou = i->second;
		if(ou->getUser()->isSet(User::NMDC) && Util::stricmp(ou->getIdentity().getNick(), aNick) == 0)
			return ou->getUser();
	}
	return UserPtr();
}


bool isOp(const UserPtr& user, const string& aHubUrl) const {
	Lock l(cs);
	auto p = onlineUsers.equal_range(user->getCID());
	for(auto i = p.first; i != p.second; ++i) {
		if(i->second->getClient().getHubUrl() == aHubUrl) {
			return i->second->getIdentity().isOp();
		}
	}
	return false;
}


void putOnline(OnlineUser* ou) noexcept {
	{
		Lock l(cs);
		onlineUsers.emplace(ou->getUser()->getCID(),ou);
		onlineHubUsers.emplace(ou->getUser()->getCID(),ou->getClient().getHubUrl());
	}

	if(ou && !ou->getUser()->isOnline()) {
		ou->getUser()->setFlag(User::ONLINE);
		ou->initializeData(); //RSX++-like
		fire(UsersManagerListener::UserConnected(), ou->getUser());
	}
}

void putOffline(OnlineUser* ou, bool disconnect) noexcept {
	OnlineIter::difference_type diff = 0;
	{
		Lock l(cs);
		auto op = onlineUsers.equal_range(ou->getUser()->getCID());
		dcassert(op.first != op.second);
		OnlineUser* ou2 = nullptr;
		for(auto i = op.first; i != op.second; ++i) {
			ou2 = i->second;
			if(ou == ou2) {
				diff = distance(op.first, op.second);
				onlineUsers.erase(i);
				break;
			}
		}
	}
	
	auto z = onlineHubUsers.find(ou->getUser()->getCID());
	if(z != onlineHubUsers.end())
		onlineHubUsers.erase(z);

	if(diff == 1) { //last user
		UserPtr u = ou->getUser();
		u->unsetFlag(User::ONLINE);
		u->unsetFlag(User::PROTECT);
		if(disconnect)
			ConnectionManager::getInstance()->disconnect(u);

		fire(UsersManagerListener::UserDisconnected(), u);
	} else if(diff > 1) {
			fire(UsersManagerListener::UserUpdated(), *ou);
	}
}



OnlineUser* findOnlineUserHint(const CID& cid, const string& hintUrl, OnlinePairC& p) const {
	Lock l(cs);
    p = onlineUsers.equal_range(cid);
	if(p.first == p.second) // no user found with the given CID.
		return nullptr;

	if(!hintUrl.empty()) {
		for(auto i = p.first; i != p.second; ++i) {
			OnlineUser* u = i->second;
			if(u->getClient().getHubUrl() == hintUrl) {
				return u;
			}
		}
	}

	return nullptr;
}


void updateNick(const OnlineUser& user) noexcept {
	if(!user.getIdentity().getNick().empty()) {
		Lock l(cs);
		NickMap::iterator i = nicks.find(user.getUser()->getCID());
		if(i == nicks.end()) {
			nicks[user.getUser()->getCID()] = std::make_pair(user.getIdentity().getNick(), false);
		} else {
			i->second.first = user.getIdentity().getNick();
		}
	}
}

void setIpAddress(const UserPtr& p, const string& ip) {
    Lock l(cs);
	OnlineIterC i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		bool bIPv6 = false;	

		if(Util::isIp6(ip)) {
			i->second->getIdentity().set("I6", ip);
			bIPv6 = true;
		}
		
		if( bIPv6 == false) {
			i->second->getIdentity().set("I4", ip);
		}
		fire(UsersManagerListener::UserUpdated(),(dynamic_cast<const OnlineUser&>(*i->second)));
	}
}

void on(ClientListener::UserUpdated, dcpp::Client*, const OnlineUser& user) noexcept {
	updateNick(user);
	fire(UsersManagerListener::UserUpdated(), user);

}

void on(ClientListener::UsersUpdated, Client* , const OnlineUserList& l) noexcept {
	for(OnlineUserList::const_iterator i = l.begin(), iend = l.end(); i != iend; ++i) {
		updateNick(*(*i));
		fire(UsersManagerListener::UserUpdated(), *(*i));
	}
}

private:
	OnlineHubUserMap onlineHubUsers;
	OnlineMap onlineUsers;
	NickMap nicks;
	
	mutable CriticalSection cs;
	
};
}
#endif
