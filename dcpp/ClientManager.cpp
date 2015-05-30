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

#include "stdinc.h"
#include "ClientManager.h"

#include "ShareManager.h"
#include "SearchManager.h"
#include "ConnectionManager.h"
#include "CryptoManager.h"
#include "FavoriteManager.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "SearchResult.h"
#include "File.h"
#include "AdcHub.h"
#include "NmdcHub.h"
#include "nullptr.h"
#include "RawManager.h"
#include "LogManager.h"

namespace dcpp {

ClientManager::ClientManager() : FakeChecker(this), udp(Socket::TYPE_UDP) {
	TimerManager::getInstance()->addListener(this);
}

ClientManager::~ClientManager() {
	TimerManager::getInstance()->removeListener(this);
}

Client* ClientManager::getClient(const string& aHubURL) {
	Client* c = nullptr;
	if(Util::strnicmp("adc://", aHubURL.c_str(), 6) == 0) {
		c = new AdcHub(aHubURL, false);
	} else if(Util::strnicmp("adcs://", aHubURL.c_str(), 7) == 0) {
		c = new AdcHub(aHubURL, true);
	} else {
		c = new NmdcHub(aHubURL);
	}

	{
		Lock l(cs);
		clients.insert(c);
	}

	c->addListener(this);

	return c;
}

void ClientManager::putClient(Client* aClient) {
	fire(ClientManagerListener::ClientDisconnected(), aClient);
	aClient->removeListeners();

	{
		Lock l(cs);
		clients.erase(aClient);
	}
	aClient->shutdown();
	delete aClient;
}

size_t ClientManager::getUserCount() const {
	Lock l(cs);
	return onlineUsers.size();
}

StringList ClientManager::getHubs(const CID& cid, const string& hintUrl) {
	Lock l(cs);
	StringList lst;
	OnlinePairC op = onlineUsers.equal_range(cid);
	for(auto i = op.first; i != op.second; ++i) {
		if(i->second->getClient().getHubUrl() == hintUrl)
				lst.push_back(i->second->getClient().getHubUrl());
	}
	return lst;
}

StringList ClientManager::getHubNames(const CID& cid, const string& hintUrl) {
	Lock l(cs);
	StringList lst;
	OnlinePairC op = onlineUsers.equal_range(cid);
	for(auto i = op.first; i != op.second; ++i) {
		//if(i->second->getClient().getHubUrl() == hintUrl)
			lst.push_back(i->second->getClient().getHubName());
	}
	return lst;
}

StringList ClientManager::getNicks(const CID& cid, const string& hintUrl) {
	Lock l(cs);
	StringSet ret;

	OnlinePairC op = onlineUsers.equal_range(cid);
	for(auto i = op.first; i != op.second; ++i) {
		if(i->second->getClient().getHubUrl() == hintUrl)
				ret.insert(i->second->getIdentity().getNick());
	}

	if(ret.empty()) {
		// offline
		NickMap::const_iterator i = nicks.find(cid);
		if(i != nicks.end()) {
			ret.insert(i->second.first);
		} else {
			ret.insert('{' + cid.toBase32() + '}');
		}
	}

	return StringList(ret.begin(), ret.end());
}

vector<Identity> ClientManager::getIdentities(const UserPtr &u) const {
	Lock l(cs);
	auto op = onlineUsers.equal_range(u->getCID());
	vector<Identity> ret;
	for(auto i = op.first; i != op.second; ++i) {
		ret.emplace_back(i->second->getIdentity());
	}

	return ret;
}

string ClientManager::getField(const CID& cid, const string& hint, const char* field) const {
	Lock l(cs);

	OnlinePairC p;
	OnlineUser* u = findOnlineUserHint(cid, hint, p);
	if(u) {
		string value = u->getIdentity().get(field);
		if(!value.empty()) {
			return value;
		}
	}

	for(auto i = p.first; i != p.second; ++i) {
		string value = i->second->getIdentity().get(field);
		if(!value.empty()) {
			return value;
		}
	}

	return Util::emptyString;
}

string ClientManager::getConnection(const CID& cid) const {
	Lock l(cs);
	auto i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		return i->second->getIdentity().getConnection();
	}
	return _("Offline");
}

int64_t ClientManager::getAvailable() const {
	Lock l(cs);
	int64_t bytes = 0;
	for(OnlineIterC i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
		bytes += i->second->getIdentity().getBytesShared();
	}

	return bytes;
}

bool ClientManager::isConnected(const string& aUrl) const {
	Lock l(cs);

	for(auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return true;
		}
	}
	return false;
}

bool ClientManager::isHubConnected(const string& aUrl) const {
	Lock l(cs);

	for(auto i: clients) {
		if(i->getHubUrl() == aUrl) {
			return i->isConnected();
		}
	}
	return false;
}

string ClientManager::findHub(const string& ipPort) const {
	Lock l(cs);

	string ip;
	uint16_t port = 411;

	parsePortIp(ipPort,ip, port);

	if( port < 1 || port > 65535)
			return Util::emptyString;//good idea?
	bool ok = false;
	if(Util::isIp6(ip) == true)
		ok = true;
	else
		ok = (inet_addr(ip.c_str()) != INADDR_NONE);
				
	if(ok == true) {

		string url;
		for(auto i = clients.begin(); i != clients.end(); ++i) {
			const Client* c = *i;
			if(c->getIp() == ip) {
				// If exact match is found, return it
				if(c->getPort() == port)
					return c->getHubUrl();

				// Port is not always correct, so use this as a best guess...
				url = c->getHubUrl();
		}
	}

		return url;
	}
	return Util::emptyString;
}

string ClientManager::findHubEncoding(const string& aUrl) const {
	Lock l(cs);

	for(auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return (*i)->getEncoding();
		}
	}
	return Text::systemCharset;
}

UserPtr ClientManager::findLegacyUser(const string& aNick) const noexcept {
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

UserPtr ClientManager::getUser(const string& aNick, const string& aHubUrl) noexcept {
	CID cid = makeCid(aNick, aHubUrl);
	Lock l(cs);

	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		if(ui->second) {
			ui->second->setFlag(User::NMDC);
			return ui->second;
		}
	}

	UserPtr p = make_shared<User>(cid);
	p->setFlag(User::NMDC);
	users.emplace(cid,p);

	return p;
}

UserPtr ClientManager::getUser(const CID& cid) noexcept {
	Lock l(cs);
	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}
	if(cid == getMe()->getCID()) { //should create only one instance of yourself
		return getMe();
	}

	UserPtr p = make_shared<User>(cid);
	users.emplace(cid,p);
	return p;
}

UserPtr ClientManager::findUser(const CID& cid,const string& hubUrl /*=empty*/) const noexcept {
	{
		Lock l(cs);
		UserMap::const_iterator ui = users.find(cid);
		if(ui != users.end()) {
			return ui->second;
		}
	}
	return nullptr;
}

bool ClientManager::isOp(const UserPtr& user, const string& aHubUrl) const {
	Lock l(cs);
	auto p = onlineUsers.equal_range(user->getCID());
	for(auto i = p.first; i != p.second; ++i) {
		if(i->second->getClient().getHubUrl() == aHubUrl) {
			return i->second->getIdentity().isOp();
		}
	}
	return false;
}

CID ClientManager::makeCid(const string& aNick, const string& aHubUrl) const noexcept {
	string n = Text::toLower(aNick);
	TigerHash th;
	th.update(n.c_str(), n.length());
	th.update(Text::toLower(aHubUrl).c_str(), aHubUrl.length());
	// Construct hybrid CID from the bits of the tiger hash - should be
	// fairly random, and hopefully low-collision
	return CID(th.finalize());
}

void ClientManager::putOnline(OnlineUser* ou) noexcept {
	{
		Lock l(cs);
		onlineUsers.emplace(ou->getUser()->getCID(),ou);
	}

	if(ou &&  !ou->getUser()->isOnline()) {
		ou->getUser()->setFlag(User::ONLINE);
		ou->initializeData(); //RSX++-like
		fire(ClientManagerListener::UserConnected(), ou->getUser());
	}
}

void ClientManager::putOffline(OnlineUser* ou, bool disconnect) noexcept {
	OnlineIter::difference_type diff = 0;
	{
		Lock l(cs);
		auto op = onlineUsers.equal_range(ou->getUser()->getCID());
		dcassert(op.first != op.second);
		for(auto i = op.first; i != op.second; ++i) {
			OnlineUser* ou2 = i->second;
			if(ou == ou2) {
				diff = distance(op.first, op.second);
				onlineUsers.erase(i);
				break;
			}
		}
	}

	if(diff == 1) { //last user
		UserPtr u = ou->getUser();
		u->unsetFlag(User::ONLINE);
		u->unsetFlag(User::PROTECT);
		if(disconnect)
			ConnectionManager::getInstance()->disconnect(u);

		fire(ClientManagerListener::UserDisconnected(), u);
	} else if(diff > 1) {
			fire(ClientManagerListener::UserUpdated(), *ou);
	}
}

OnlineUser* ClientManager::findOnlineUserHint(const CID& cid, const string& hintUrl, OnlinePairC& p) const {
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

OnlineUser* ClientManager::findOnlineUser(const HintedUser& user) {
	return findOnlineUser(user.user->getCID(), user.hint);
}

OnlineUser* ClientManager::findOnlineUser(const CID& cid, const string& hintUrl) {
	OnlinePairC p;
	OnlineUser* u = findOnlineUserHint(cid, hintUrl, p);
	if(u) // found an exact match (CID + hint).
		return u;

	if(p.first == p.second) // no user found with the given CID.
		return nullptr;

	// return a random user that matches the given CID but not the hint.
	return p.first->second;
}
//TODO ? needed....
string ClientManager::findMySID(const HintedUser& p) {
	//this could also be done by just finding in the client list... better?
	if(p.hint.empty()) // we cannot find the correct SID without a hubUrl
		return Util::emptyString;

	OnlineUser* u = findOnlineUser(p.user->getCID(), p.hint);
	if(u)
		return (&u->getClient())->getMyIdentity().getSIDString();

	return Util::emptyString;
}
//
void ClientManager::connect(const HintedUser& user, const string& token) {
	Lock l(cs);
	OnlineUser* u = findOnlineUser(user);

	if(u) {
		u->getClient().connect(*u, token);
	}
}

void ClientManager::privateMessage(const HintedUser& user, const string& msg, bool thirdPerson) {
	Lock l(cs);
	OnlineUser* u = findOnlineUser(user);

	if(u) {
		u->getClient().privateMessage(*u, msg, thirdPerson);
	}
}

void ClientManager::userCommand(const HintedUser& user, const UserCommand& uc, ParamMap& params, bool compatibility) {
	Lock l(cs);
	/** @todo we allow wrong hints for now ("false" param of findOnlineUser) because users
	 * extracted from search results don't always have a correct hint; see
	 * SearchManager::onRES(const AdcCommand& cmd, ...). when that is done, and SearchResults are
	 * switched to storing only reliable HintedUsers (found with the token of the ADC command),
	 * change this call to findOnlineUserHint. */
	OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint.empty() ? uc.getHub() : user.hint);
	if(!ou)
		return;

	ou->getIdentity().getParams(params, "user", compatibility);
	ou->getClient().getHubIdentity().getParams(params, "hub", false);
	ou->getClient().getMyIdentity().getParams(params, "my", compatibility);
	ou->getClient().sendUserCmd(uc, params);
}

void ClientManager::send(AdcCommand& cmd, const CID& cid) {
	Lock l(cs);
	auto i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		OnlineUser& u = *i->second;
		if(cmd.getType() == AdcCommand::TYPE_UDP && !u.getIdentity().isUdpActive()) {
			cmd.setType(AdcCommand::TYPE_DIRECT);
			cmd.setTo(u.getIdentity().getSID());
			u.getClient().send(cmd);
		} else {
			try {
				string ip = u.getIdentity().getIp();
				string port = u.getIdentity().getUdpPort();
				bool ok = false;
				if(Util::isIp6(ip) == true)
					ok = true;
				else
					ok = (inet_addr(ip.c_str()) != INADDR_NONE);
				
				if(ok == true) {
					udp.writeTo(ip, port, cmd.toString(getMe()->getCID()));
				}	
			} catch(const SocketException&) {
				dcdebug("Socket exception sending ADC UDP command\n");
			}
		}
	}
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->info();
		}
	}
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
									int aFileType, const string& aString) noexcept
{
	Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);
	if(aSeeker.empty()) return;
	if(aSeeker.length() < 4) return;


	bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);

	// We don't wan't to answer passive searches if we're in passive mode...
	if(isPassive && !ClientManager::getInstance()->isActive()) {
		return;
	}

	SearchResultList l = ShareManager::getInstance()->search(aString, aSearchType, aSize, aFileType, isPassive ? 5 : 10);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(!l.empty()) {
		if(isPassive) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
				const SearchResultPtr& sr = *i;
				str += sr->toSR(*aClient);
				str[str.length()-1] = 5;
				str += name;
				str += '|';
			}

			if(!aClient && !str.empty())
				aClient->send(str);

		} else {
			try {
				string ip;
				uint16_t port = 0 ;
				string seek = Util::trimUrl(aSeeker);
				
				parsePortIp(seek,ip,port);
				
				if( (aClient ) || static_cast<NmdcHub*>(aClient)->isProtectedIP(ip))
					return;
						
				bool isOk = false;
				if(Util::isIp6(ip) == true)
					isOk = true;
				else
					isOk = (inet_addr(ip.c_str()) != INADDR_NONE);
				
				if(port == -1)
					return;
				//port should be number	
				if( port < 1 || port > 65535)
							return;
						
				if(isOk == true) {
					
				for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
					const SearchResultPtr& sr = *i;
					udp.writeTo(ip, Util::toString(port), sr->toSR(*aClient));
				}
			  }
			} catch(const SocketException& /* e */) {
				dcdebug("Search caught error\n");
			}
		}
	}
}

void ClientManager::on(AdcSearch, Client*, const AdcCommand& adc, const CID& from) noexcept {
	bool isUdpActive = false;
	{
		Lock l(cs);

		auto i = onlineUsers.find(from);
		if(i != onlineUsers.end()) {
			OnlineUser& u = *i->second;
			isUdpActive = u.getIdentity().isUdpActive();
		}

	}
	SearchManager::getInstance()->respond(adc, from, isUdpActive);
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	for(auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken, StringList() /*ExtList*/);
		}
	}
}

void ClientManager::search(string& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList) {
	Lock l(cs);

	for(auto i = clients.begin(); i != clients.end(); ++i) { //change clients set to map<string*(hubUrl), Client*> for better lookup with .find
		if(((*i)->getHubUrl() == who) && (*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken, aExtList);
			break;
		}
	}
}

void ClientManager::getOnlineClients(StringList& onlineClients) {
	Lock l (cs);
	for(auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected())
			onlineClients.push_back((*i)->getHubUrl());
	}
}

void ClientManager::on(TimerManagerListener::Minute, uint64_t /* aTick */) noexcept {
	Lock l(cs);

	// Collect some garbage...
	auto i = users.begin();
	while(i != users.end()) {
		if(i->second.unique()) {
			unordered_map<CID, NickMapEntry>::const_iterator n = nicks.find(i->second->getCID());//should also remove from nicks...
			if(n != nicks.end()) nicks.erase(n);
			users.erase(i++);
		} else {
			++i;
		}
	}

	for(auto j = clients.begin(); j != clients.end(); ++j) {
		if((*j)->isConnected())
			(*j)->info();
	}
}

UserPtr& ClientManager::getMe() {
	if(!me) {
		Lock l(cs);
		if(!me) {
			me = make_shared<User>(getMyCID());
			users.emplace(me->getCID(), me);
		}
	}
	return me;
}

const CID& ClientManager::getMyPID() {
	if(!pid)
		pid = CID(SETTING(PRIVATE_ID));
	return pid;
}

CID ClientManager::getMyCID() {
	TigerHash tiger;
	tiger.update(getMyPID().data(), CID::SIZE);
	return CID(tiger.finalize());
}

void ClientManager::updateNick(const OnlineUser& user) noexcept {
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
int ClientManager::getMode(const string& aHubUrl) const {//todo ipv6?

	if(aHubUrl.empty())
		return SETTING(INCOMING_CONNECTIONS);

	const FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(aHubUrl);
	if(hub) {
		switch(hub->getMode()) {
			case 1 : return SettingsManager::INCOMING_DIRECT;
			case 2 : return SettingsManager::INCOMING_FIREWALL_PASSIVE;
			default: return SETTING(INCOMING_CONNECTIONS);
		}
	}
	return SETTING(INCOMING_CONNECTIONS);
}

bool ClientManager::isActive(const string& aHubUrl /*= Util::emptyString*/) const
{
	return ( (getMode(aHubUrl) != SettingsManager::INCOMING_FIREWALL_PASSIVE) );
}
//TODO:check ip6 if ok
void ClientManager::setIpAddress(const UserPtr& p, const string& ip) {
    Lock l(cs);
	OnlineIterC i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		bool ipv6 = false;	
		if(ip.find_first_of(':') != ip.find_last_of(':')) {//ipv6
			i->second->getIdentity().set("I6", ip);
			ipv6 = true;
		}
		if( ipv6 == false) {
			i->second->getIdentity().set("I4", ip);
		}
		fire(ClientManagerListener::UserUpdated(),(dynamic_cast<const OnlineUser&>(*i->second)));
	}
}

void ClientManager::sendAction(OnlineUser& ou, const int aAction) {
	if(aAction < 1)
		return;

	if(ou.getClient().isOp() && !ou.isProtectedUser()) {
		ou.getClient().sendActionCommand(ou, aAction);
    }
}
//TODO is this Suite to FakeChecker?
void ClientManager::addCheckToQueue(const HintedUser hintedUser, bool filelist) {
	OnlineUser* ou = nullptr;
	bool addCheck = false;
	{
		Lock l(cs);
		ou = findOnlineUser(hintedUser);
		if(!ou) return;

		if(ou->isCheckable()) {
			if(!ou->getChecked(filelist)) {
				if((filelist && ou->shouldCheckFileList()) || (!filelist && ou->shouldCheckClient())) {
					addCheck = true;
				}
			}
		}
	}

	if(addCheck) {
		try {
			if(filelist) {
				QueueManager::getInstance()->addFileListCheck(hintedUser);
				ou->getIdentity().setFileListQueued("1");
			} else {
				QueueManager::getInstance()->addClientCheck(hintedUser);
				ou->getIdentity().setTestSURQueued("1");
			}
		} catch(...) {
			//...
		}
	}
}

void ClientManager::on(Connected, Client* c) noexcept {
	fire(ClientManagerListener::ClientConnected(), c);
}

void ClientManager::on(UserUpdated, Client*, const OnlineUser& user) noexcept {
	updateNick(user);
	fire(ClientManagerListener::UserUpdated(), user);
}

void ClientManager::on(UsersUpdated, Client* c, const OnlineUserList& l) noexcept {
	for(OnlineUserList::const_iterator i = l.begin(), iend = l.end(); i != iend; ++i) {
		updateNick(*(*i));
		fire(ClientManagerListener::UserUpdated(), *(*i));
	}
}

void ClientManager::on(HubUpdated, Client* c) noexcept {
	fire(ClientManagerListener::ClientUpdated(), c);
}

void ClientManager::on(Failed, Client* client, const string&) noexcept {
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(HubUserCommand, Client* client, int aType, int ctx, const string& name, const string& command) noexcept {
	if(SETTING(HUB_USER_COMMANDS)) {
		if(aType == UserCommand::TYPE_REMOVE) {
			int cmd = FavoriteManager::getInstance()->findUserCommand(name, client->getHubUrl());
			if(cmd != -1)
				FavoriteManager::getInstance()->removeUserCommand(cmd);
		} else if(aType == UserCommand::TYPE_CLEAR) {
 			FavoriteManager::getInstance()->removeHubUserCommands(ctx, client->getHubUrl());
 		} else {
 			FavoriteManager::getInstance()->addUserCommand(aType, ctx, UserCommand::FLAG_NOSAVE, name, command, "", client->getHubUrl());
 		}
	}
}
//..suite of FakeChecker ?
void ClientManager::checkCheating(const HintedUser& p, DirectoryListing* dl) {
	string report;
	OnlineUser* ou = nullptr;

	{
		Lock l(cs);

		ou = findOnlineUser(p);
		if(!ou) return;

		int64_t statedSize = ou->getIdentity().getBytesShared();
		int64_t realSize = dl->getTotalSize();

		double multiplier = ((100+(double)SETTING(PERCENT_FAKE_SHARE_TOLERATED))/100);

		int64_t sizeTolerated = (int64_t)(realSize*multiplier);

		ou->getIdentity().set("RS", Util::toString(realSize));
		ou->getIdentity().set("SF", Util::toString(dl->getTotalFileCount(true)));

		bool isFakeSharing = false;
		if(statedSize > sizeTolerated) {
			isFakeSharing = true;
		}

		if(isFakeSharing) {
			string cheatStr;
			if(realSize == 0) {
				cheatStr = "Mismatched share size - zero bytes real size";
			} else {
				double qwe = (double)((double)statedSize / (double)realSize);
				cheatStr = "Mismatched share size - filelist was inflated "+Util::toString(qwe)+" times, stated size = %[userSSshort], real size = %[userRSshort]";
			}
			report = ou->setCheat(cheatStr, false, true, SETTING(SHOW_FAKESHARE_RAW));
			sendAction(*ou, SETTING(FAKESHARE_RAW));
		} else {
			//RSX++ //ADLS Forbidden files
			const DirectoryListing::File::List forbiddenList = dl->getForbiddenFiles();
			const DirectoryListing::Directory::List forbiddenDirList = dl->getForbiddenDirs();

			if(!forbiddenList.empty() || !forbiddenDirList.empty() ) {
				int64_t fs = 0;
				string s, c, sz, tth, stringForKick; //forbiddenFilesList;

				int actionCommand = 0, totalPoints = 0, point = 0;
				bool forFromFavs = false;
				bool forOverRide = false;

				if(!forbiddenList.empty()) {
					for(auto i = forbiddenList.begin() ; i != forbiddenList.end() ; i++) {
						fs += (*i)->getSize();
						totalPoints += (*i)->getPoints();
						if(((*i)->getPoints() >= point) || (*i)->getOverRidePoints()) {
							point = (*i)->getPoints();
							s = (*i)->getFullFileName();
							c = (*i)->getAdlsComment();
							tth = (*i)->getTTH().toBase32();
							sz = Util::toString((*i)->getSize());
							if((*i)->getOverRidePoints()) {
								forOverRide = true;
								if ((*i)->getFromFavs()) {
									actionCommand = (*i)->getAdlsRaw();
									forFromFavs = true;
								} else {
									stringForKick = (*i)->getKickString();
									forFromFavs = false;
								}
							}
						}
					}
				}
				if(!forbiddenDirList.empty()) {
					for(auto j = forbiddenDirList.begin() ; j != forbiddenDirList.end() ; j++) {
						fs += (*j)->getTotalSize();
						totalPoints += (*j)->getPoints();
						if(((*j)->getPoints() >= point) || (*j)->getOverRidePoints()) {
							point = (*j)->getPoints();
							s = (*j)->getName();
							c = (*j)->getAdlsComment();
							sz = Util::toString((*j)->getTotalSize());
							if((*j)->getOverRidePoints()) {
								forOverRide = true;
								if ((*j)->getFromFavs()) {
									actionCommand = (*j)->getAdlsRaw();
									forFromFavs = true;
								} else {
									stringForKick = (*j)->getKickString();
									forFromFavs = false;
								}
							}
						}
					}
				}
				ou->getIdentity().set("A1", s);								//file
				ou->getIdentity().set("A2", c);								//comment
				ou->getIdentity().set("A3", sz);							//file size
				ou->getIdentity().set("A4", tth);							//tth
				ou->getIdentity().set("A5", Util::toString(fs));			//forbidden size
				ou->getIdentity().set("A6", Util::toString(totalPoints));	//total points
				ou->getIdentity().set("A7", Util::toString((int)forbiddenList.size() + (int)forbiddenDirList.size())); //no. of forbidden files&dirs

				s = "[%[adlTotalPoints]] Is sharing %[adlFilesCount] forbidden files/directories including: %[adlFile]";

            if(forOverRide) {
					report = ou->setCheat(s, false, true, true);
					if(forFromFavs) {
						sendAction(*ou, actionCommand);
					} else {
						sendRawCommand(*ou, stringForKick);
					}
				} else if(totalPoints > 0) {
					bool show = false;
					int rawToSend = 0;
					CalcADLAction::getInstance()->calcADLAction(totalPoints, rawToSend, show);
					report = ou->setCheat(s, false, true, show);
					sendAction(*ou, rawToSend);
				}
			}
		}
		//END
	}

	if(ou) {
		ou->getIdentity().setFileListComplete("1");
		ou->getIdentity().setFileListQueued("0");
		ou->getClient().updated(*ou);
		if(!report.empty()) {
			ou->getClient().cheatMessage(report);
		}
	}
}

void ClientManager::sendRawCommand(OnlineUser& ou, const string& aRaw, bool checkProtection/* = false*/) {
	if(!aRaw.empty()) {
		bool skipRaw = false;
		Lock l(cs);
		if(checkProtection) {
			skipRaw = ou.isProtectedUser();
		}
		if(!skipRaw || !checkProtection) {
			ParamMap ucParams;
			UserCommand uc = UserCommand(0, 0, 0, 0, "", aRaw, "", Util::emptyString);
			userCommand(HintedUser(ou.getUser(),Util::emptyString), uc, ucParams, true);
			if(SETTING(LOG_RAW_CMD)) {
				LOG(LogManager::RAW, ucParams);
			}
		}
	}
}

} // namespace dcpp
