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

#include "stdinc.h"
#include "Client.h"
#include "BufferedSocket.h"
#include "FavoriteManager.h"
#include "TimerManager.h"
#include "ClientManager.h"
#include "DebugManager.h"
#include "PluginManager.h"

namespace dcpp {

atomic<long> Client::counts[COUNT_UNCOUNTED];

Client::Client(const string& hubURL, char separator_, bool secure_) :
	myIdentity(ClientManager::getInstance()->getMe(), 0),
	reconnDelay(120), lastActivity(GET_TICK()), registered(false), autoReconnect(false),
	encoding(Text::systemCharset), state(STATE_DISCONNECTED), sock(0),
	hubUrl(hubURL),separator(separator_),
	secure(secure_), countType(COUNT_UNCOUNTED)
{
	string file, proto, query, fragment;
	Util::decodeUrl(hubURL, proto, address, port, file, query, fragment);
	keyprint = Util::decodeQuery(query)["kp"];

	TimerManager::getInstance()->addListener(this);
	//RSX++
	setCheckAtConnect(false);
	cmdQueue.setClientPtr(this);
}

Client::~Client() {
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
	/// @todo update the nick in ADC hubs?
	string prevNick = Util::emptyString;
	if(!updateNick)
		prevNick = settings.getNick();
	settings = SettingsManager::getInstance()->getHubSettings();
	
	auto fav = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());
	if(fav)
	{
		FavoriteManager::getInstance()->mergeHubSettings(*fav, settings);
		if(!fav->getPassword().empty())
			setPassword(fav->getPassword());
		//[BMDC
		setHideShare(fav->getHideShare());
		setFavIp(fav->getIp());
		setChatExtraInfo(fav->getChatExtraInfo());
		setProtectUser(fav->getProtectUsers());
		setCheckAtConnect(fav->getCheckAtConn());
		setCheckClients(fav->getCheckClients());
		setCheckFilelists(fav->getCheckFilelists());
		setTabText(fav->getTabText());
		setTabIconStr(fav->getTabIconStr());
		//]
	}else{
		//[BMDC++
		setHideShare(false);
		setFavIp(Util::emptyString);
		setChatExtraInfo(Util::emptyString);
		setProtectUser(Util::emptyString);
		setCheckAtConnect(false);
		setCheckClients(false);
        setCheckFilelists(false);
        setTabText(Util::emptyString);
        setTabIconStr(Util::emptyString);
        //]
	}
	if(updateNick)
        checkNick(settings.nick);
    else
       settings.setNick(prevNick);
}

void Client::connect() {
	if(sock) {
		BufferedSocket::putSocket(sock);
		sock = 0;
	}

	setAutoReconnect(true);
	if((uint32_t)SETTING(TIME_RECCON) > 10)
        setReconnDelay((uint32_t)(SETTING(TIME_RECCON)));
	else
        setReconnDelay(20);
	reloadSettings(true);
	setRegistered(false);
	setMyIdentity(Identity(ClientManager::getInstance()->getMe(), 0));
	setHubIdentity(Identity());

	state = STATE_CONNECTING;

	try {
		sock = BufferedSocket::getSocket(separator, v4only());
		sock->addListener(this);
		sock->connect(address, port, secure, BOOLSETTING(ALLOW_UNTRUSTED_HUBS), true);
	} catch(const Exception& e) {
		state = STATE_DISCONNECTED;
		fire(ClientListener::Failed(), this, e.getError());
	}
	updateActivity();
}

void Client::send(const char* aMessage, size_t aLen) {
	if(!isReady()) {
		dcassert(0);
		return;
	}
	if(PluginManager::getInstance()->runHook(HOOK_NETWORK_HUB_OUT, (dcptr_t)this,(dcptr_t)&aMessage))
		return;
	
	updateActivity();
	sock->write(aMessage, aLen);
	{
		Lock l(cs);
		COMMAND_DEBUG(aMessage, DebugManager::HUB_OUT, getIpPort());
		
	}	
}

HubData* Client::getPluginObject() noexcept {
	resetEntity();

	pod.url = pluginString(hubUrl);
	pod.ip = pluginString(ip);
	pod.object = this;
	pod.port = Util::toInt(port);
	pod.protocol = isAdc() ? PROTOCOL_ADC : PROTOCOL_NMDC;
	pod.isOp = isOp() ? True : False;
	pod.isSecure = isSecure() ? True : False;

	return &pod;
}

void Client::on(Connected) noexcept {
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

void Client::on(Failed, const string& aLine) noexcept {
	state = STATE_DISCONNECTED;
	FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
	fire(ClientListener::Failed(), this, aLine);
}

void Client::disconnect(bool graceLess) {
	if(sock)
		sock->disconnect(graceLess);
}

bool Client::isSecure() const {
	return isReady() && sock->isSecure();
}

bool Client::isTrusted() const {
	return isReady() && sock->isTrusted();
}

std::string Client::getCipherName() const {
	return isReady() ? sock->getCipherName() : Util::emptyString;
}

vector<uint8_t> Client::getKeyprint() const {
	return isReady() ? sock->getKeyprint() : vector<uint8_t>();
}

bool Client::isActive() const {
	return ClientManager::getInstance()->isActive(hubUrl);
}

void Client::updateCounts(bool aRemove) {
	// We always remove the count and then add the correct one if requested...
	if(countType != COUNT_UNCOUNTED) {
		--counts[countType];
		countType = COUNT_UNCOUNTED;
	}

	if(!aRemove) {
		if(getMyIdentity().isOp()) {
			countType = COUNT_OP;
		} else if(getMyIdentity().isRegistered()) {
			countType = COUNT_REGISTERED;
		} else {
			countType = COUNT_NORMAL;
		}
		++counts[countType];
	}
}

string Client::getLocalIp() const {
    
    if(!getFavIp().empty()) {
        return Socket::resolve(getFavIp());
    }
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

string Client::getCounts() {
	char buf[128];
	return string(buf, snprintf(buf, sizeof(buf), "%ld/%ld/%ld",
		counts[COUNT_NORMAL].load(), counts[COUNT_REGISTERED].load(), counts[COUNT_OP].load()));
}

void Client::on(Line, const string& aLine) noexcept {
	updateActivity();
	
	{
		Lock l(cs);
		COMMAND_DEBUG(aLine, DebugManager::HUB_IN, getIpPort());
	}	
}

void Client::on(Second, uint64_t aTick) noexcept {
	if(state == STATE_DISCONNECTED && getAutoReconnect() && (aTick > (getLastActivity() + getReconnDelay() * 1000)) ) {
		// Try to reconnect...
		connect();
	}
	
	if(isConnected()){
		cmdQueue.onSecond(aTick); //RSX+
	}
}

#ifdef _USELUA
string ClientScriptInstance::formatChatMessage(const tstring& aLine) {
	Lock l(scs);
	// this string is probably in UTF-8.  Does lua want/need strings in the active code page?
	string processed = Text::fromT(aLine);
	MakeCall("dcpp", "FormatChatText", 1, (Client*)this, processed);

	if (lua_isstring(L, -1)) processed = lua_tostring(L, -1);

	lua_settop(L, 0);
	return Text::toT(processed);
}

bool ClientScriptInstance::onHubFrameEnter(Client* aClient, const string& aLine) {
	Lock l(scs);
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
