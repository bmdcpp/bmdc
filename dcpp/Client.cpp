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
#include "Client.h"
#include "BufferedSocket.h"

#include "FavoriteManager.h"
#include "TimerManager.h"
#include "ClientManager.h"

#include "RawManager.h"
#include "DebugManager.h"

namespace dcpp {

Client::Counts Client::counts;

Client::Client(const string& hubURL, char separator_, bool secure_) :
	myIdentity(ClientManager::getInstance()->getMe(), 0),
	reconnDelay(120), lastActivity(GET_TICK()), registered(false), autoReconnect(false),
	encoding(Text::systemCharset), state(STATE_DISCONNECTED), sock(0),
	hubUrl(hubURL), separator(separator_),
	secure(secure_), countType(COUNT_UNCOUNTED)
{
	string file, proto, query, fragment;
	Util::decodeUrl(hubURL, proto, address, port, file, query, fragment);
	
	if(!query.empty()) {
		auto q = Util::decodeQuery(query);
		auto kp = q.find("kp");
		if(kp != q.end()) {
			keyprint = kp->second;
		}	
	}		

	TimerManager::getInstance()->addListener(this);
	/* RSX++ */
	setCheckedAtConnect(false);
	cmdQueue.setClientPtr(this);
	/* RSX++ */
}

Client::~Client() throw() {
	dcassert(!sock);

	// In case we were deleted before we Failed
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	TimerManager::getInstance()->removeListener(this);
	updateCounts(true);
}

void Client::reconnect() {
	disconnect(true);
	setAutoReconnect(true);
	setReconnDelay(0);
}

void Client::shutdown() {
	//RSX++
	cmdQueue.clear();
	putDetectors();
	//END
	if(sock) {
		BufferedSocket::putSocket(sock);
		sock = 0;
	}
}

void Client::reloadSettings(bool updateNick) {
	FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());
	if(hub) {
		if(updateNick) {
			setCurrentNick(checkNick(hub->getNick(true)));
		}

		if(!hub->getUserDescription().empty()) {
			setCurrentDescription(hub->getUserDescription());
		} else {
			setCurrentDescription(SETTING(DESCRIPTION));
		}
		if(!hub->getPassword().empty())
			setPassword(hub->getPassword());
		///Hide
		setHideShare(hub->getHideShare());
		//RSX
		setCheckClients(hub->getCheckClients());
		setCheckFilelists(hub->getCheckFilelists());
		setCheckOnConnect(hub->getCheckOnConnect());

		setChatExtraInfo(hub->getChatExtraInfo());
		
		setFavIp(hub->getIp());

	} else {
		if(updateNick) {
			setCurrentNick(checkNick(SETTING(NICK)));
		}
		setCurrentDescription(SETTING(DESCRIPTION));
		///
		setHideShare(false);
		//RSX
		setCheckClients(false);
		setCheckFilelists(false);
		setCheckOnConnect(false);
		 
		setChatExtraInfo(Util::emptyString);
		setFavIp(Util::emptyString);

	}
}

void Client::connect() {
	if(sock)
		BufferedSocket::putSocket(sock);

	setAutoReconnect(true);
	setReconnDelay((uint32_t)(SETTING(TIME_RECCON)));//Time for Reconect
	reloadSettings(true);
	setRegistered(false);
	setMyIdentity(Identity(ClientManager::getInstance()->getMe(), 0));
	setHubIdentity(Identity());

	state = STATE_CONNECTING;

	try {
		sock = BufferedSocket::getSocket(separator);
		sock->addListener(this);
		sock->connect(address, port, secure, BOOLSETTING(ALLOW_UNTRUSTED_HUBS), true);
	} catch(const Exception& e) {
		if(sock) {
			BufferedSocket::putSocket(sock);
			sock = 0;
		}
		fire(ClientListener::Failed(), this, e.getError());
	}
	updateActivity();
}

void Client::send(const char* aMessage, size_t aLen) {
	dcassert(sock);
	if(!sock)
		return;
	updateActivity();
	sock->write(aMessage, aLen);
	COMMAND_DEBUG(aMessage, DebugManager::HUB_OUT, getIpPort());
}

void Client::on(Connected) throw() {
	updateActivity();
	ip = sock->getIp();
	localIp = sock->getLocalIp();
	if(sock->isSecure() && keyprint.compare(0, 7, "SHA256/") == 0) {
		auto kp = sock->getKeyprint();
		 if(!kp.empty()) {
			 vector<uint8_t> kp2v(kp.size());
			 Encoder::fromBase32(keyprint.c_str() + 7, &kp2v[0], kp2v.size());
			 if(!std::equal(kp.begin(), kp.end(), kp2v.begin())) {
				 state = STATE_DISCONNECTED;
				 sock->removeListener(this);
				 fire(ClientListener::Failed(), this, "Keyprint mismatch");
				 return;
		}
	}
}	
	fire(ClientListener::Connected(), this);
	state = STATE_PROTOCOL;
}

void Client::on(Failed, const string& aLine) throw() {
	state = STATE_DISCONNECTED;
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	sock->removeListener(this);
	fire(ClientListener::Failed(), this, aLine);
}

void Client::disconnect(bool graceLess) {
	if(sock)
		sock->disconnect(graceLess);
}

bool Client::isSecure() const {
	return sock && sock->isSecure();
}

bool Client::isTrusted() const {
	return sock && sock->isTrusted();
}

std::string Client::getCipherName() const {
	return sock ? sock->getCipherName() : Util::emptyString;
}

vector<uint8_t> Client::getKeyprint() const {
 return isReady() ? sock->getKeyprint() : vector<uint8_t>();	
}	

void Client::updateCounts(bool aRemove) {
	// We always remove the count and then add the correct one if requested...
	if(countType == COUNT_NORMAL) {
	    counts.normal.dec();
	}    
	else if (countType == COUNT_REGISTERED) {
			counts.registered.dec();
	} else if (countType == COUNT_OP) {
	   	counts.op.dec();
	}   	
		
	countType = COUNT_UNCOUNTED;	
	
	if(!aRemove) {
		if(getMyIdentity().isOp()) {
			counts.op.inc();
			countType = COUNT_OP;
		} else if(getMyIdentity().isRegistered()) {
			counts.registered.inc();
			countType = COUNT_REGISTERED;
		} else {
			counts.normal.inc();
			countType = COUNT_NORMAL;
		}
	}
}

string Client::getLocalIp() const {
	// Favorite hub Ip
	if(!getFavIp().empty())
		return Socket::resolve(getFavIp());
	
	// Best case - the server detected it
	if((!BOOLSETTING(NO_IP_OVERRIDE) || SETTING(EXTERNAL_IP).empty()) && !getMyIdentity().getIp().empty()) {
		return getMyIdentity().getIp();
	}

	if(!SETTING(EXTERNAL_IP).empty()) {
		return Socket::resolve(SETTING(EXTERNAL_IP), AF_INET);
	}

	if(localIp.empty()) {
		return Util::getLocalIp();
	}

	return localIp;
}

void Client::on(Line, const string& aLine) throw() {
	updateActivity();
	COMMAND_DEBUG(aLine, DebugManager::HUB_IN, getIpPort());
}

void Client::on(Second, uint64_t aTick) throw() {
	if(state == STATE_DISCONNECTED && getAutoReconnect() && (aTick > (getLastActivity() + getReconnDelay() * 1000)) ) {
		// Try to reconnect...
		connect();
	}
	
	if(isConnected()){
		cmdQueue.onSecond(aTick); //RSX+
	}
}

bool Client::isActive() const {
	return ClientManager::getInstance()->isActive(hubUrl);
}

#ifdef _USELUA
string ClientScriptInstance::formatChatMessage(const tstring& aLine) {
	Lock l(cs);
	// this string is probably in UTF-8.  Does lua want/need strings in the active code page?
	string processed = Text::fromT(aLine);
	MakeCall("dcpp", "FormatChatText", 1, (Client*)this, processed);

	if (lua_isstring(L, -1)) processed = lua_tostring(L, -1);

	lua_settop(L, 0);
	return Text::toT(processed);
}

bool ClientScriptInstance::onHubFrameEnter(Client* aClient, const string& aLine) {
	Lock l(cs);
	// ditto the comment above
	MakeCall("dcpp", "OnCommandEnter", 1, aClient, aLine);
	return GetLuaBool();
}
#endif

void Client::sendActionCommand(const OnlineUser& ou, int actionId) {
	if(!isConnected() /*|| (userCount < getUsersLimit())*/)
		return;
	cmdQueue.addCommand(ou, actionId);
}

bool Client::isActionActive(const int aAction) const {
	FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());

	return hub ? FavoriteManager::getInstance()->getEnabledAction(hub, aAction) : true;
}

} // namespace dcpp
