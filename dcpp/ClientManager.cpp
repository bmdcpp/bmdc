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

#include "stdinc.h"
#include "typedefs.h"
#include "ClientManager.h"

#include "ShareManager.h"
#include "SearchManager.h"
#include "ConnectionManager.h"
#include "CryptoManager.h"
#include "FavoriteManager.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "SearchResult.h"

#include "AdcHub.h"
#include "NmdcHub.h"

#include "FinishedManager.h"
#include "QueueManager.h"
#include "RawManager.h"
#include "LogManager.h"

namespace dcpp {

Client* ClientManager::getClient(const string& aHubURL) {
	Client* c;
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
	return getHubs(cid, hintUrl, FavoriteManager::getInstance()->isPrivate(hintUrl));
}

StringList ClientManager::getHubNames(const CID& cid, const string& hintUrl) {
	return getHubNames(cid, hintUrl, FavoriteManager::getInstance()->isPrivate(hintUrl));
}

StringList ClientManager::getNicks(const CID& cid, const string& hintUrl) {
	return getNicks(cid, hintUrl, FavoriteManager::getInstance()->isPrivate(hintUrl));
}

StringList ClientManager::getHubs(const CID& cid, const string& hintUrl, bool priv) {
	Lock l(cs);
	StringList lst;
	if(!priv) {
		OnlinePairC op = onlineUsers.equal_range(cid);
		for(OnlineIterC i = op.first; i != op.second; ++i) {
			lst.push_back(i->second->getClient().getHubUrl());
		}
	} else {
		OnlineUser* u = findOnlineUser_hint(cid, hintUrl);
		if(u)
			lst.push_back(u->getClient().getHubUrl());
	}
	return lst;
}

StringList ClientManager::getHubNames(const CID& cid, const string& hintUrl, bool priv) {
	Lock l(cs);
	StringList lst;
	if(!priv) {
		OnlinePairC op = onlineUsers.equal_range(cid);
		for(OnlineIterC i = op.first; i != op.second; ++i) {
			lst.push_back(i->second->getClient().getHubName());
		}
	} else {
		OnlineUser* u = findOnlineUser_hint(cid, hintUrl);
		if(u)
			lst.push_back(u->getClient().getHubName());
	}
	return lst;
}

StringList ClientManager::getNicks(const CID& cid, const string& hintUrl, bool priv) {
	Lock l(cs);
	StringSet ret;

	if(!priv) {
		OnlinePairC op = onlineUsers.equal_range(cid);
		for(OnlineIterC i = op.first; i != op.second; ++i) {
			ret.insert(i->second->getIdentity().getNick());
		}
	} else {
		OnlineUser* u = findOnlineUser_hint(cid, hintUrl);
		if(u)
			ret.insert(u->getIdentity().getNick());
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

string ClientManager::getConnection(const CID& cid) const {
	Lock l(cs);
	OnlineIterC i = onlineUsers.find(cid);
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

	for(/*Client::List::const_iterator*/auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return true;
		}
	}
	return false;
}

string ClientManager::findHub(const string& ipPort) const {
	Lock l(cs);

	string ip;
	uint16_t port = 411;
	string::size_type i = ipPort.find(':');
	if(i == string::npos) {
		ip = ipPort;
	} else {
		ip = ipPort.substr(0, i);
		port = static_cast<uint16_t>(Util::toInt(ipPort.substr(i+1)));
	}

	string url;
	for(/*Client::List::const_iterator*/auto i = clients.begin(); i != clients.end(); ++i) {
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

string ClientManager::findHubEncoding(const string& aUrl) const {
	Lock l(cs);

	for(/*Client::List::const_iterator*/auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return (*i)->getEncoding();
		}
	}
	return Text::systemCharset;
}

UserPtr ClientManager::findLegacyUser(const string& aNick) const throw() {
	if (aNick.empty())
		return UserPtr();

	Lock l(cs);

	for(OnlineMap::const_iterator i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
		const OnlineUser* ou = i->second;
		if(ou->getUser()->isSet(User::NMDC) && Util::stricmp(ou->getIdentity().getNick(), aNick) == 0)
			return ou->getUser();
	}
	return UserPtr();
}

UserPtr ClientManager::getUser(const string& aNick, const string& aHubUrl) throw() {
	CID cid = makeCid(aNick, aHubUrl);
	Lock l(cs);

	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		ui->second->setFlag(User::NMDC);
		return ui->second;
	}

	UserPtr p(new User(cid));
	p->setFlag(User::NMDC);
	users.insert(make_pair(cid, p));

	return p;
}

UserPtr ClientManager::getUser(const CID& cid) throw() {
	Lock l(cs);
	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}

	UserPtr p(new User(cid));
	users.insert(make_pair(cid, p));
	return p;
}

UserPtr ClientManager::findUser(const CID& cid) const throw() {
	Lock l(cs);
	UserMap::const_iterator ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}
	return 0;
}

bool ClientManager::isOp(const UserPtr& user, const string& aHubUrl) const {
	Lock l(cs);
	OnlinePairC p = onlineUsers.equal_range(user->getCID());
	for(OnlineIterC i = p.first; i != p.second; ++i) {
		if(i->second->getClient().getHubUrl() == aHubUrl) {
			return i->second->getIdentity().isOp();
		}
	}
	return false;
}

CID ClientManager::makeCid(const string& aNick, const string& aHubUrl) const throw() {
	string n = Text::toLower(aNick);
	TigerHash th;
	th.update(n.c_str(), n.length());
	th.update(Text::toLower(aHubUrl).c_str(), aHubUrl.length());
	// Construct hybrid CID from the bits of the tiger hash - should be
	// fairly random, and hopefully low-collision
	return CID(th.finalize());
}

void ClientManager::putOnline(OnlineUser* ou) throw() {
	{
		Lock l(cs);
		onlineUsers.insert(make_pair(ou->getUser()->getCID(), ou));
	}

	if(!ou->getUser()->isOnline()) {
		ou->getUser()->setFlag(User::ONLINE);
		ou->initializeData();//
		fire(ClientManagerListener::UserConnected(), ou->getUser());
	}
}

void ClientManager::putOffline(OnlineUser* ou, bool disconnect) throw() {
	bool lastUser = false;
	{
		Lock l(cs);
		OnlinePair op = onlineUsers.equal_range(ou->getUser()->getCID());
		dcassert(op.first != op.second);
		for(OnlineIter i = op.first; i != op.second; ++i) {
			OnlineUser* ou2 = i->second;
			if(ou == ou2) {
				lastUser = (distance(op.first, op.second) == 1);
				onlineUsers.erase(i);
				break;
			}
		}
	}

	if(lastUser) {
		UserPtr& u = ou->getUser();
		u->unsetFlag(User::ONLINE);
		u->unsetFlag(User::PROTECTED); //RSX++
		updateNick(*ou);
		if(disconnect)
			ConnectionManager::getInstance()->disconnect(u);
		fire(ClientManagerListener::UserDisconnected(), u);
	}
}

OnlineUser* ClientManager::findOnlineUser_hint(const CID& cid, const string& hintUrl, OnlinePair& p) throw() {
	p = onlineUsers.equal_range(cid);
	if(p.first == p.second) // no user found with the given CID.
		return 0;

	if(!hintUrl.empty()) {
		for(OnlineIter i = p.first; i != p.second; ++i) {
			OnlineUser* u = i->second;
			if(u->getClient().getHubUrl() == hintUrl) {
				return u;
			}
		}
	}

	return 0;
}

OnlineUser* ClientManager::findOnlineUser(const CID& cid, const string& hintUrl, bool priv) throw() {
	OnlinePair p;
	OnlineUser* u = findOnlineUser_hint(cid, hintUrl, p);
	if(u) // found an exact match (CID + hint).
		return u;

	if(p.first == p.second) // no user found with the given CID.
		return 0;

	// if the hint hub is private, don't allow connecting to the same user from another hub.
	if(priv)
		return 0;

	// ok, hub not private, return a random user that matches the given CID but not the hint.
	return p.first->second;
}

OnlineUser* ClientManager::findOnlineUser(const HintedUser& user, bool priv) {
	return findOnlineUser(user.user->getCID(), user.hint, priv);
}

void ClientManager::connect(const HintedUser& user, const string& token) {
	bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);

	Lock l(cs);
	OnlineUser* u = findOnlineUser(user.user->getCID(), user.hint, priv);

	if(u) {
		u->getClient().connect(*u, token);
	}
}

void ClientManager::privateMessage(const HintedUser& user, const string& msg, bool thirdPerson) {
	bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);

	Lock l(cs);
	OnlineUser* u = findOnlineUser(user.user->getCID(), user.hint, priv);

	if(u) {
		u->getClient().privateMessage(*u, msg, thirdPerson);
	}
}

void ClientManager::userCommand(const HintedUser& user, const UserCommand& uc, StringMap& params, bool compatibility) {
	Lock l(cs);
	/** @todo we allow wrong hints for now ("false" param of findOnlineUser) because users
	 * extracted from search results don't always have a correct hint; see
	 * SearchManager::onRES(const AdcCommand& cmd, ...). when that is done, and SearchResults are
	 * switched to storing only reliable HintedUsers (found with the token of the ADC command),
	 * change this call to findOnlineUser_hint. */
	OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint.empty() ? uc.getHub() : user.hint, false);
	if(!ou)
		return;

	ou->getIdentity().getParams(params, "user", compatibility);
	ou->getClient().getHubIdentity().getParams(params, "hub", false);
	ou->getClient().getMyIdentity().getParams(params, "my", compatibility);
	ou->getClient().escapeParams(params);
	ou->getClient().sendUserCmd(uc, params);
}

void ClientManager::send(AdcCommand& cmd, const CID& cid) {
	Lock l(cs);
	OnlineIter i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		OnlineUser& u = *i->second;
		if(cmd.getType() == AdcCommand::TYPE_UDP && !u.getIdentity().isUdpActive()) {
			cmd.setType(AdcCommand::TYPE_DIRECT);
			cmd.setTo(u.getIdentity().getSID());
			u.getClient().send(cmd);
		} else {
			try {
				udp.writeTo(u.getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(u.getIdentity().getUdpPort())), cmd.toString(getMe()->getCID()));
			} catch(const SocketException&) {
				dcdebug("Socket exception sending ADC UDP command\n");
			}
		}
	}
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(/*Client::Iter*/auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->info(false);
		}
	}
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
									int aFileType, const string& aString) throw()
{
	Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);

	bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);

	// We don't wan't to answer passive searches if we're in passive mode...
	if(isPassive && !ClientManager::getInstance()->isActive(aClient->getHubUrl())) {
		return;
	}

	SearchResultList l;
	ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(l.size() > 0) {
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

			if(str.size() > 0)
				aClient->send(str);

		} else {
			try {
				string ip, file, proto, query, fragment;
				uint16_t port = 0;
				Util::decodeUrl(aSeeker, proto, ip, port, file, query, fragment);
				ip = Socket::resolve(ip);
				if(static_cast<NmdcHub*>(aClient)->isProtectedIP(ip))
					return;
				if(port == 0)
					port = 412;
				for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
					const SearchResultPtr& sr = *i;
					udp.writeTo(ip, port, sr->toSR(*aClient));
				}
			} catch(const SocketException& /* e */) {
				dcdebug("Search caught error\n");
			}
		}
	}else if(!isPassive && (aFileType == SearchManager::TYPE_TTH) && (aString.compare(0, 4, "TTH:") == 0)) {
		PartsInfo partialInfo;
		TTHValue aTTH(aString.substr(4));
		if(!QueueManager::getInstance()->handlePartialSearch(aTTH, partialInfo)) {
			// if not found, try to find in finished list
			if(!FinishedManager::getInstance()->handlePartialRequest(aTTH, partialInfo)) {
				return;
			}
		}

		string ip, file, proto, query, fragment;
		uint16_t port = 0;
		Util::decodeUrl(aSeeker, proto, ip, port, file, query, fragment);

		try {
			AdcCommand cmd = SearchManager::getInstance()->toPSR(true, aClient->getMyNick(), aClient->getIpPort(), aTTH.toBase32(), partialInfo);
			Socket s;
			s.writeTo(Socket::resolve(ip), port, cmd.toString(ClientManager::getInstance()->getMe()->getCID()));
		} catch(...) {
			dcdebug("Partial search caught error\n");
		}
	}
}

void ClientManager::on(AdcSearch, Client* c, const AdcCommand& adc, const CID& from) throw() {
	bool isUdpActive = false;
	{
		Lock l(cs);

		OnlineIter i = onlineUsers.find(from);
		if(i != onlineUsers.end()) {
			OnlineUser& u = *i->second;
			isUdpActive = u.getIdentity().isUdpActive();
		}

	}
	SearchManager::getInstance()->respond(adc, from, isUdpActive,c->getHubUrl());
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	for(/*Client::Iter*/auto i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken, StringList() /*extlist*/);
		}
	}
}


void ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList) {
	Lock l(cs);

	for(StringIter it = who.begin(); it != who.end(); ++it) {
		string& client = *it;
		for(/*Client::Iter*/auto j = clients.begin(); j != clients.end(); ++j) {
			Client* c = *j;
			if(c->isConnected() && c->getHubUrl() == client) {
				c->search(aSizeMode, aSize, aFileType, aString, aToken, aExtList);
			}
		}
	}
}

void ClientManager::on(TimerManagerListener::Minute, uint64_t /* aTick */) throw() {
	Lock l(cs);

	// Collect some garbage...
	UserIter i = users.begin();
	while(i != users.end()) {
		if(i->second->unique()) {
			users.erase(i++);
		} else {
			++i;
		}
	}

	for(/*Client::Iter*/auto j = clients.begin(); j != clients.end(); ++j) {
		(*j)->info(false);
	}
}

UserPtr& ClientManager::getMe() {
	if(!me) {
		Lock l(cs);
		if(!me) {
			me = new User(getMyCID());
			users.insert(make_pair(me->getCID(), me));
		}
	}
	return me;
}

const CID& ClientManager::getMyPID() {
	if(pid.isZero())
		pid = CID(SETTING(PRIVATE_ID));
	return pid;
}

CID ClientManager::getMyCID() {
	TigerHash tiger;
	tiger.update(getMyPID().data(), CID::SIZE);
	return CID(tiger.finalize());
}

void ClientManager::updateNick(const OnlineUser& user) throw() {
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

void ClientManager::loadUsers() {
	try {
		SimpleXML xml;
		xml.fromXML(File(getUsersFile(), File::READ, File::OPEN).read());

		if(xml.findChild("Users")) {
			xml.stepIn();

			{
				Lock l(cs);
				while(xml.findChild("User")) {
					nicks[CID(xml.getChildAttrib("CID"))] = std::make_pair(xml.getChildAttrib("Nick"), false);
				}
			}

			xml.stepOut();
		}
	} catch(const Exception&) { }
}

void ClientManager::saveUsers() const {
	try {
		SimpleXML xml;
		xml.addTag("Users");
		xml.stepIn();

		{
			Lock l(cs);
			for(NickMap::const_iterator i = nicks.begin(), iend = nicks.end(); i != iend; ++i) {
				if(i->second.second) {
					xml.addTag("User");
					xml.addChildAttrib("CID", i->first.toBase32());
					xml.addChildAttrib("Nick", i->second.first);
				}
			}
		}

		xml.stepOut();

		const string fName = getUsersFile();
		File out(fName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(fName);
		File::renameFile(fName + ".tmp", fName);
	} catch(const Exception&) { }
}

void ClientManager::saveUser(const CID& cid) {
	Lock l(cs);
	NickMap::iterator i = nicks.find(cid);
	if(i != nicks.end())
		i->second.second = true;
}

void ClientManager::on(Connected, Client* c) throw() {
	fire(ClientManagerListener::ClientConnected(), c);
}

void ClientManager::on(UserUpdated, Client*, const OnlineUser& user) throw() {
	updateNick(user);
	fire(ClientManagerListener::UserUpdated(), user);
}

void ClientManager::on(UsersUpdated, Client* c, const OnlineUserList& l) throw() {
	for(OnlineUserList::const_iterator i = l.begin(), iend = l.end(); i != iend; ++i) {
		updateNick(*(*i));
		fire(ClientManagerListener::UserUpdated(), *(*i));
	}
}

void ClientManager::on(HubUpdated, Client* c) throw() {
	fire(ClientManagerListener::ClientUpdated(), c);
}

void ClientManager::on(Failed, Client* client, const string&) throw() {
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(HubUserCommand, Client* client, int aType, int ctx, const string& name, const string& command) throw() {
	if(BOOLSETTING(HUB_USER_COMMANDS)) {
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
//RSX
void ClientManager::setGenerator(const UserPtr& p, const string& aGenerator, const string& aCID, const string& aBase) {
	Client* c = 0;
	string report; //= Util::emptyString;
	{
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(p->getCID(),"",false);
		if(!ou)
			return;
		ou->getIdentity().set("GE", aGenerator);
		ou->getIdentity().set("FI", aCID);
		ou->getIdentity().set("FB", aBase);
		report = ou->getIdentity().checkFilelistGenerator(*ou);
		c = &ou->getClient();
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

void ClientManager::sendAction(const UserPtr& p, const int aAction) {
	if(/*p->isSet(User::DHT) ||*/ aAction < 1)
		return;
	{
		Lock l(cs);
		OnlineIterC i = onlineUsers.find(p->getCID());
		if(i == onlineUsers.end())
			return;

		if(i->second->getClient().isOp() && !i->second->isProtectedUser()) {
			i->second->getClient().sendActionCommand((*i->second), aAction);
		}
	}
}

void ClientManager::sendAction(OnlineUser& ou, const int aAction) {
	if(/*ou.getIdentity().getUser()->isSet(User::DHT) || */aAction < 1)
		return;

	if(ou.getClient().isOp() && !ou.isProtectedUser()) {
		ou.getClient().sendActionCommand(ou, aAction);
    }
}

void ClientManager::sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection/* = false*/) {
	if(!aRaw.empty()) {
		bool skipRaw = false;
		Lock l(cs);
		if(checkProtection) {
			OnlineUser* ou = findOnlineUser(user->getCID(), Util::emptyString, false);
			if(!ou) return;
			skipRaw = ou->isProtectedUser();
		}
		if(!skipRaw || !checkProtection) {
			StringMap ucParams;
			UserCommand uc = UserCommand(0, 0, 0, 0, "", aRaw, "", Util::emptyString);
			userCommand(HintedUser(user,Util::emptyString), uc, ucParams, true);
			//RSXPP_SET(TOTAL_RAW_COMMANDS_SENT, RSXPP_SETTING(TOTAL_RAW_COMMANDS_SENT)+1); //important stuff ;p
			if(SETTING(LOG_RAW_CMD)) {
				LOG(LogManager::RAW, ucParams);
			}
		}
	}
}

void ClientManager::setPkLock(const UserPtr& p, const string& aPk, const string& aLock) {
	Lock l(cs);
	OnlineUser* ou = findOnlineUser(p->getCID(), "", false);
	if(!ou)
		return;

	ou->getIdentity().set("PK", aPk);
	ou->getIdentity().set("LO", aLock);
}

//RSX++
void ClientManager::setListSize(const UserPtr& p, int64_t aFileLength, bool adc) {
	OnlineUser* ou = NULL;
	string report;// = Util::emptyString;
	{
		Lock l(cs);
		ou = findOnlineUser(p->getCID(),"", false);
		if(!ou)
			return;

		ou->getIdentity().set("LS", Util::toString(aFileLength));

		if(ou->getIdentity().getBytesShared() > 0) {
			if((SETTING(MAX_FILELIST_SIZE) > 0) && (aFileLength > SETTING(MAX_FILELIST_SIZE)) && BOOLSETTING(FILELIST_TOO_SMALL_BIG)) {
				report = ou->setCheat("Too large filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, BOOLSETTING(FILELIST_TOO_SMALL_BIG));
				sendAction(*ou, SETTING(FILELIST_TOO_SMALL_BIG_RAW));
			} else if((aFileLength < SETTING(MIN_FL_SIZE) && BOOLSETTING(FILELIST_TOO_SMALL_BIG)) || (aFileLength < 100)) {
				report = ou->setCheat("Too small filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, BOOLSETTING(FILELIST_TOO_SMALL_BIG));
				sendAction(*ou, SETTING(FILELIST_TOO_SMALL_BIG_RAW));
			}
		} else if(adc == false) {
			int64_t listLength = (!ou->getIdentity().get("LL").empty()) ? Util::toInt64(ou->getIdentity().get("LL")) : -1;
			if(p->isSet(User::DCPLUSPLUS) && (listLength != -1) && (listLength * 3 < aFileLength) && (ou->getIdentity().getBytesShared() > 0)) {
				report = ou->setCheat("Fake file list - ListLen = %[userLL], FileLength = %[userLS]", false, true, BOOLSETTING(LISTLEN_MISMATCH_SHOW));
				sendAction(*ou, SETTING(LISTLEN_MISMATCH));
			}
		}
	}
	 
	if(ou) {
		ou->getClient().updated(ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}
//END
void ClientManager::setSupports(const UserPtr& p, const string& aSupports) {
	Lock l(cs);
	OnlineUser* ou = findOnlineUser(p->getCID(),"", false);
	if(!ou)
		return;
	ou->getIdentity().set("SU", aSupports);
}

void ClientManager::setUnknownCommand(const UserPtr& p, const string& aUnknownCommand) {
	Lock l(cs);
	OnlineUser* ou = findOnlineUser(p->getCID(),"", false);
	if(!ou)
		return;
	ou->getIdentity().set("UC", aUnknownCommand);
}

void ClientManager::setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete) {
	OnlineUser* ou = NULL;
	string report;// = Util::emptyString;
	{
		Lock l(cs);
		ou = findOnlineUser(p->getCID(), "", false);
		if(!ou)
			return;

		if(!_ccResponse.empty()) {
			ou->setTestSURComplete();
			ou->getIdentity().set("TS", _ccResponse);
			report = ou->getIdentity().updateClientType(*ou);
		}
		if(_clientCheckComplete)
			ou->setTestSURComplete();
		if(_fileListCheckComplete)
			ou->setFileListComplete();
		if(!_cheatString.empty())
			report += ou->setCheat(_cheatString, _badClient, _badFileList, _displayCheat);
		sendAction(*ou, _actionId);
	}
	
	if(ou) {
		ou->getClient().updated(ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}

void ClientManager::fileListDisconnected(const UserPtr& p) {
	if(SETTING(MAX_DISCONNECTS) == 0)
		return;

	bool remove = false;
	string report = "Disconnected";// = Util::emptyString;
	Client* c = NULL;
	{
		Lock l(cs);
		OnlineUser* ou = findOnlineUser(p->getCID(),"", false);
		if(!ou) return;

		int fileListDisconnects = Util::toInt(ou->getIdentity().get("FD")) + 1;
		ou->getIdentity().set("FD", Util::toString(fileListDisconnects));

		if(fileListDisconnects == SETTING(MAX_DISCONNECTS)) {
			c = &ou->getClient();
			report += ou->setCheat("Disconnected file list %[userFD] times", false, true, BOOLSETTING(SHOW_DISCONNECT_RAW));
			if(ou->getIdentity().isFileListQueued()) {
				ou->setFileListComplete();
				ou->getIdentity().setFileListQueued(Util::emptyString);
				remove = true;
			}
			sendAction(*ou, SETTING(DISCONNECT_RAW));
		}
	}

	if(remove) {
		try {
			QueueManager::getInstance()->removeFileListCheck(p);
		} catch (...) {
			//...
		}
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

int ClientManager::getMode(const string& aHubUrl) const {

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

void ClientManager::setListLength(const UserPtr& p, const string& listLen) {
	Lock l(cs);
	OnlineUser* ou = findOnlineUser(p->getCID(), Util::emptyString, false);
	if(ou) {
		ou->getIdentity().set("LL", listLen);
	}
}

void ClientManager::checkCheating(const UserPtr& p, DirectoryListing* dl) {
	string report;// = Util::emptyString;
	OnlineUser* ou = NULL;

	{
		Lock l(cs);

		ou = findOnlineUser(p->getCID(), "", false);
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
				cheatStr = str(boost::format("Mismatched share size - filelist was inflated %1% times, stated size = %[userSSshort], real size = %[userRSshort]")
					% qwe);
			}
			report = ou->setCheat(cheatStr, false, true, BOOLSETTING(SHOW_FAKESHARE_RAW));
			sendAction(*ou, SETTING(FAKESHARE_RAW));
		} else {
			//RSX++ //ADLS Forbidden files
			const DirectoryListing::File::List forbiddenList = dl->getForbiddenFiles();
			const DirectoryListing::Directory::List forbiddenDirList = dl->getForbiddenDirs();

			if(forbiddenList.size() > 0 || forbiddenDirList.size() > 0) {
				int64_t fs = 0;
				string s, c, sz, tth, stringForKick, forbiddenFilesList;

				int actionCommand = 0, totalPoints = 0, point = 0;
				bool forFromFavs = false;
				bool forOverRide = false;

				if(forbiddenList.size() > 0) {
					for(DirectoryListing::File::Iter i = forbiddenList.begin() ; i != forbiddenList.end() ; i++) {
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
				if(forbiddenDirList.size() > 0) {
					for(DirectoryListing::Directory::Iter j = forbiddenDirList.begin() ; j != forbiddenDirList.end() ; j++) {
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
						sendRawCommand(ou->getUser(), stringForKick);
					}
				} else if(totalPoints > 0) {
					bool show = false;
					int rawToSend = 0;
					RawManager::getInstance()->calcADLAction(totalPoints, rawToSend, show);
					report = ou->setCheat(s, false, true, show);
					sendAction(*ou, rawToSend);
				} else {
					const string& isRMDC = ou->getIdentity().checkrmDC(*ou);
					if(!isRMDC.empty()) {
						report = isRMDC;
					}
				}
			}
		}
		//END
	}

	if(ou) {
		ou->setFileListComplete();
		ou->getIdentity().setFileListQueued(Util::emptyString);
		ou->getClient().updated(ou);
		if(!report.empty()) {
			ou->getClient().cheatMessage(report);
		}
	}
}

void ClientManager::addCheckToQueue(const HintedUser huser, bool filelist) {
	OnlineUser* ou = NULL;//0
	bool addCheck = false;
	{
		Lock l(cs);
		ou = findOnlineUser(huser,false);
		if(!ou) return;

		if(ou->isCheckable() /*&& ou->getClient().isOp()*/) {
			if(!ou->getChecked(filelist)) {
				if((filelist && ou->shouldCheckFileList()) || (!filelist && ou->shouldCheckClient())) {
					addCheck = true;
					ou->inc();
				}
			}
		}
	}

	if(addCheck) {
		try {
			if(filelist) {
				QueueManager::getInstance()->addFileListCheck(huser.user, /*ou->getClient().getHubUrl()*/huser.hint);
				ou->getIdentity().setFileListQueued("1");
			} else {
				QueueManager::getInstance()->addTestSUR(huser.user, huser.hint/*ou->getClient().getHubUrl()*/);
				ou->getIdentity().setTestSURQueued("1");
			}
		} catch(...) {
			//...
		}
		ou->dec();
	}
}
#ifdef _USELUA
bool ClientManager::ucExecuteLua(const string& cmd,StringMap& params)
{
	bool executelua = false;
	string::size_type i,j,k;
	i = j = k = 0;
	string tmp = cmd;
	 while( (i = tmp.find("%[lua:",i)) != string::npos)
	 {
		i+=6;
		j = tmp.find("]",i);
		if(j == string::npos)
			break;
		string chunk = tmp.substr(i,j-i);
		// for making possible using %[nick] and similar parameters too
		// !%!{nick!}
		k = 0;
		while(( k = chunk.find("!%")) != string::npos)
		{
				chunk.erase(k,2);
				chunk.insert(k,"%");
		}
		k = 0;
		while(( k = chunk.find("!{")) != string::npos)
		{
				chunk.erase(k,2);
				chunk.insert(k,"[");
		}
		k = 0;
		while(( k = chunk.find("!}")) != string::npos)
		{
				chunk.erase(k,2);
				chunk.insert(k,"]");
		}
		//@todo: use filter? I opted for no here, but this means Lua has to be careful about
		//filtering if it cares.

		ScriptManager::getInstance()->EvaluateChunk(Util::formatParams(chunk, params, false));
		executelua = true;
		i = j +1;

	 }
	 return executelua;
}
#endif

string ClientManager::getField(const CID& cid, const string& hint, const char* field) const {
    Lock l(cs);
    OnlinePair p;
    OnlineUser* u = findOnlineUser_hint(cid, hint,p);
    if(u) {
      string value = u->getIdentity().get(field);
       if(!value.empty()) {
                return value;
        }
    }

     for(OnlineIterC i = p.first; i != p.second; ++i) {
          string value = i->second->getIdentity().get(field);

           if(!value.empty()) {
               return value;
             }
    }
    return Util::emptyString;
}
//END

} // namespace dcpp
