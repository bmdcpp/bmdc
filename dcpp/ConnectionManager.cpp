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

#include "stdinc.h"
#include "ConnectionManager.h"

#include "Client.h"
#include "ClientManager.h"
#include "ConnectivityManager.h"
#include "CryptoManager.h"
#include "DownloadManager.h"
#include "LogManager.h"
#include "QueueManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ClientManager.h"
#include "QueueManager.h"
#include "LogManager.h"
#include "Client.h"
#include "UserConnection.h"

namespace dcpp {

ConnectionManager::ConnectionManager() : floodCounter(0), shuttingDown(false) {
	TimerManager::getInstance()->addListener(this);

	features.push_back(UserConnection::FEATURE_MINISLOTS);
	features.push_back(UserConnection::FEATURE_XML_BZLIST);
	features.push_back(UserConnection::FEATURE_ADCGET);
	features.push_back(UserConnection::FEATURE_TTHL);
	features.push_back(UserConnection::FEATURE_TTHF);

	adcFeatures.push_back("AD" + UserConnection::FEATURE_ADC_BAS0);
	adcFeatures.push_back("AD" + UserConnection::FEATURE_ADC_BASE);
	adcFeatures.push_back("AD" + UserConnection::FEATURE_ADC_TIGR);
	adcFeatures.push_back("AD" + UserConnection::FEATURE_ADC_BZIP);
}

void ConnectionManager::listen() {
	server.reset(new Server(false, CONNSETTING(TCP_PORT), CONNSETTING(BIND_ADDRESS),CONNSETTING(BIND_ADDRESS6)));

	if(!CryptoManager::getInstance()->TLSOk()) {
		dcdebug("Skipping secure port: %d\n", CONNSETTING(TLS_PORT));
		return;
	}
	secureServer.reset(new Server(true, CONNSETTING(TLS_PORT), CONNSETTING(BIND_ADDRESS),CONNSETTING(BIND_ADDRESS6)));
}

/**
 * Request a connection for downloading.
 * DownloadManager::addConnection will be called as soon as the connection is ready
 * for downloading.
 * @param aUser The user to connect to.
 */
void ConnectionManager::getDownloadConnection(const HintedUser& aUser) {
	dcassert(aUser.user != NULL);
	{
		Lock l(cs);
		auto i = find(downloads.begin(), downloads.end(), aUser.user);
		if(i == downloads.end()) {
			getCQI(aUser, true);
		} else {
			DownloadManager::getInstance()->checkIdle(aUser);
		}
	}
}

ConnectionQueueItem* ConnectionManager::getCQI(const HintedUser& aUser, bool download) {
	ConnectionQueueItem* cqi = new ConnectionQueueItem(aUser, download);
	if(download) {
		dcassert(find(downloads.begin(), downloads.end(), aUser.user) == downloads.end());
		downloads.push_back(cqi);
	} else {
		dcassert(find(uploads.begin(), uploads.end(), aUser.user) == uploads.end());
		uploads.push_back(cqi);
	}

	fire(ConnectionManagerListener::Added(), cqi);
	return cqi;
}

void ConnectionManager::putCQI(ConnectionQueueItem* cqi) {
	fire(ConnectionManagerListener::Removed(), cqi);
	if(cqi->getDownload()) {
		dcassert(find(downloads.begin(), downloads.end(), cqi) != downloads.end());
		downloads.erase(remove(downloads.begin(), downloads.end(), cqi), downloads.end());
		delayedTokens[cqi->getToken()] = GET_TICK();
	} else {
		dcassert(find(uploads.begin(), uploads.end(), cqi) != uploads.end());
		uploads.erase(remove(uploads.begin(), uploads.end(), cqi), uploads.end());
	}
	delete cqi;
}

UserConnection* ConnectionManager::getConnection(bool aNmdc, bool secure) noexcept {
	UserConnection* uc = new UserConnection(secure);
	uc->addListener(this);
	{
		Lock l(cs);
		userConnections.push_back(uc);
	}
	if(aNmdc)
		uc->setFlag(UserConnection::FLAG_NMDC);
	return uc;
}

void ConnectionManager::putConnection(UserConnection* aConn) {
	aConn->removeListener(this);
	aConn->disconnect();

	Lock l(cs);
	userConnections.erase(remove(userConnections.begin(), userConnections.end(), aConn), userConnections.end());
}

void ConnectionManager::on(TimerManagerListener::Second, uint64_t aTick) noexcept {
	UserList passiveUsers;
	ConnectionQueueItem::List removed;

	{
		Lock l(cs);

		for(auto i = delayedTokens.begin(); i != delayedTokens.end();) {
           if((i->second + (90 * 1000)) < aTick) {
                 delayedTokens.erase(i++);
           } else {
                  ++i;
           }
        }

		bool attemptDone = false;

		for(auto i = downloads.begin(); i != downloads.end(); ++i) {
			ConnectionQueueItem* cqi = *i;

			if(cqi->getState() != ConnectionQueueItem::ACTIVE) {
				if(!cqi->getUser().user->isOnline()) {
					// Not online anymore...remove it from the pending...
					removed.push_back(cqi);
					continue;
				}

				if(cqi->getUser().user->isSet(User::PASSIVE) && !ClientManager::getInstance()->isActive()) {
					passiveUsers.push_back(cqi->getUser());
					removed.push_back(cqi);
					continue;
				}

				if(cqi->getErrors() == -1 && cqi->getLastAttempt() != 0) {
					// protocol error, don't reconnect except after a forced attempt
					continue;
				}

				//if(cqi->getLastAttempt() == 0 || (!attemptDone &&
				//	cqi->getLastAttempt() + 60 * 1000 * max(1, cqi->getErrors()) < aTick))
				if(cqi->getLastAttempt() == 0 || (((cqi->getLastAttempt() + 60*1000) < aTick) && !attemptDone))
				{
					cqi->setLastAttempt(aTick);

					QueueItem::Priority prio = QueueManager::getInstance()->hasDownload(cqi->getUser());

					if(prio == QueueItem::PAUSED) {
						removed.push_back(cqi);
						continue;
					}

					bool startDown = DownloadManager::getInstance()->startDownload(prio);

					if(cqi->getState() == ConnectionQueueItem::WAITING) {
						if(startDown) {
							cqi->setState(ConnectionQueueItem::CONNECTING);
							ClientManager::getInstance()->connect(cqi->getUser(), cqi->getToken());
							fire(ConnectionManagerListener::StatusChanged(), cqi);
							attemptDone = true;
						} else {
							cqi->setState(ConnectionQueueItem::NO_DOWNLOAD_SLOTS);
							fire(ConnectionManagerListener::Failed(), cqi, _("All download slots taken"));
						}
					} else if(cqi->getState() == ConnectionQueueItem::NO_DOWNLOAD_SLOTS && startDown) {
						cqi->setState(ConnectionQueueItem::WAITING);
					}
				} else if(cqi->getState() == ConnectionQueueItem::CONNECTING && cqi->getLastAttempt() + 50 * 1000 < aTick) {
					//cqi->setErrors(cqi->getErrors() + 1);
					fire(ConnectionManagerListener::Failed(), cqi, _("Connection timeout"));
					cqi->setState(ConnectionQueueItem::WAITING);
				}
			}
		}

		for(auto m = removed.begin(); m != removed.end(); ++m) {
			putCQI(*m);
		}

	}

	for(auto ui = passiveUsers.begin(); ui != passiveUsers.end(); ++ui) {
		QueueManager::getInstance()->removeSource(*ui, QueueItem::Source::FLAG_PASSIVE);
	}
}

void ConnectionManager::on(TimerManagerListener::Minute, uint64_t aTick) noexcept {
	Lock l(cs);

	for(auto j = userConnections.begin(); j != userConnections.end(); ++j) {
		if(((*j)->getLastActivity() + 180*1000) < aTick) {
			(*j)->disconnect(true);
		}
	}
}

uint16_t ConnectionManager::getPort() const {
	if(server.get())
		return server->getPort();
	return 0;	
}

uint16_t ConnectionManager::getSecurePort() const {
	if(secureServer.get())
		return secureServer->getPort();
	return 0;	
}

static const uint32_t FLOOD_TRIGGER = 20000;
static const uint32_t FLOOD_ADD = 2000;

ConnectionManager::Server::Server(bool secure, const uint16_t& port_, const string& ip, const string& ipv6,bool nmdc_) :
sock(Socket::TYPE_TCP), secure(secure), die(false) , nmdc(nmdc_)
{
	sock.setLocalIp4(ip);
	sock.setLocalIp6(ipv6);

	port = sock.listen(port_);
	start();
}

static const uint32_t POLL_TIMEOUT = 250;

int ConnectionManager::Server::run() noexcept {
	while(!die) {
		try {
			while(!die) {
				auto ret = sock.wait(POLL_TIMEOUT, true, false);
				if(ret.first) {
					ConnectionManager::getInstance()->accept(sock, secure,nmdc);
				}
			}
		} catch(const Exception& e) {
			dcdebug("ConnectionManager::Server::run Error: %s\n", e.getError().c_str());
		}

		bool failed = false;
		while(!die) {
			try {
				sock.disconnect();
				port = sock.listen(port);

				if(failed) {
					LogManager::getInstance()->message(_("Connectivity restored"));
					failed = false;
				}
				break;
			} catch(const SocketException& e) {
				dcdebug("ConnectionManager::Server::run Stopped listening: %s\n", e.getError().c_str());

				if(!failed) {
					LogManager::getInstance()->message(string(F_("Connectivity error: ") + e.getError()));
					failed = true;
				}

				// Spin for 60 seconds
				for(int i = 0; i < 60 && !die; ++i) {
					Thread::sleep(1000);
				}
			}
		}
	}
	return 0;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::accept(const Socket& sock, bool secure,bool nmdc) noexcept {
	uint64_t now = GET_TICK();

	if(now > floodCounter) {
		floodCounter = now + FLOOD_ADD;
	} else {
		if(false && now + FLOOD_TRIGGER < floodCounter) {
			Socket s(Socket::TYPE_TCP);
			try {
				s.accept(sock);
			} catch(const SocketException&) {
				// ...
			}
			dcdebug("Connection flood detected!\n");
			return;
		} else {
			floodCounter += FLOOD_ADD;
		}
	}
	UserConnection* uc = getConnection(nmdc, secure);
	uc->setFlag(UserConnection::FLAG_INCOMING);
	uc->setState(UserConnection::STATE_SUPNICK);
	uc->setLastActivity(GET_TICK());
	try {
		uc->accept(sock);
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::nmdcConnect(const string& aServer, const uint16_t& aPort, const string& aNick, const string& hubUrl, const string& encoding) {
	if(shuttingDown)
		return;

	UserConnection* uc = getConnection(true, false);
	
	
	if (checkCTM2HUB(aServer, aPort, hubUrl))
		return;
	
	
	uc->setToken(aNick);
	uc->setHubUrl(hubUrl);
	uc->setEncoding(encoding);
	uc->setState(UserConnection::STATE_CONNECT);
	uc->setFlag(UserConnection::FLAG_NMDC);

	try {
		uc->connect(aServer, aPort, 0, BufferedSocket::NAT_NONE);
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::adcConnect(const OnlineUser& aUser, const uint16_t& aPort, const string& aToken, bool secure) {
	adcConnect(aUser, aPort, 0, BufferedSocket::NAT_NONE, aToken, secure);
}

void ConnectionManager::adcConnect(const OnlineUser& aUser, const uint16_t& aPort, const uint16_t& localPort, BufferedSocket::NatRoles natRole, const string& aToken, bool secure) {
	if(shuttingDown)
		return;

	UserConnection* uc = getConnection(false, secure);
	uc->setToken(aToken);
	uc->setEncoding(Text::utf8);
	uc->setState(UserConnection::STATE_CONNECT);
	uc->setHubUrl(aUser.getClient().getHubUrl());//this need proper way?
	
	if(aUser.getIdentity().isOp()) {
		uc->setFlag(UserConnection::FLAG_OP);
	}
	try {
		uc->connect(aUser.getIdentity().getIp(), aPort, localPort, natRole);
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::disconnect() noexcept {
	server.reset();
	secureServer.reset();
}

void ConnectionManager::on(AdcCommand::SUP, UserConnection* aSource, const AdcCommand& cmd) noexcept {
	if(aSource->getState() != UserConnection::STATE_SUPNICK) {
		// Already got this once, ignore...@todo fix support updates
		dcdebug("CM::onSUP %p sent sup twice\n", (void*)aSource);
		return;
	}

	bool baseOk = false;

	for(auto i = cmd.getParameters().begin(); i != cmd.getParameters().end(); ++i) {
		if(i->compare(0, 2, "AD") == 0) {
			string feat = i->substr(2);
			if(feat == UserConnection::FEATURE_ADC_BASE || feat == UserConnection::FEATURE_ADC_BAS0) {
				baseOk = true;
				// ADC clients must support all these...
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_TTHF);
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
				// For compatibility with older clients...
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
			} else if(feat == UserConnection::FEATURE_ZLIB_GET) {
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
			} else if(feat == UserConnection::FEATURE_ADC_BZIP) {
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
			}
		}
	}

	if(!baseOk) {
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Invalid SUP"));
		aSource->disconnect();
		return;
	}

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		StringList defFeatures = adcFeatures;
		if(SETTING(COMPRESS_TRANSFERS)) {
			defFeatures.push_back("AD" + UserConnection::FEATURE_ZLIB_GET);
		}
		aSource->sup(defFeatures);
		aSource->inf(false);
	} else {
		aSource->inf(true);
	}
	aSource->setState(UserConnection::STATE_INF);
}

void ConnectionManager::on(AdcCommand::STA, UserConnection* , const AdcCommand& ) noexcept {
}

void ConnectionManager::on(UserConnectionListener::Connected, UserConnection* aSource) noexcept {
	if(SETTING(REQUIRE_TLS) && !aSource->isSet(UserConnection::FLAG_NMDC) && !aSource->isSecure()) {
		putConnection(aSource);
		QueueManager::getInstance()->removeSource(aSource->getUser(), QueueItem::Source::FLAG_UNENCRYPTED);
		return;
	}

	if(aSource->isSecure() && !aSource->isTrusted() && !SETTING(ALLOW_UNTRUSTED_CLIENTS)) {
		putConnection(aSource);
		QueueManager::getInstance()->removeSource(aSource->getUser(), QueueItem::Source::FLAG_UNTRUSTED);
		return;
	}

	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	if(aSource->isSet(UserConnection::FLAG_NMDC)) {
		aSource->myNick(aSource->getToken());
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk() + "Ref=" + aSource->getHubUrl());
	} else {
		StringList defFeatures = adcFeatures;
		if(SETTING(COMPRESS_TRANSFERS)) {
			defFeatures.push_back("AD" + UserConnection::FEATURE_ZLIB_GET);
		}
		aSource->sup(defFeatures);
		aSource->send(AdcCommand(AdcCommand::SEV_SUCCESS, AdcCommand::SUCCESS, Util::emptyString).addParam("RF", aSource->getHubUrl()));
	}
	aSource->setState(UserConnection::STATE_SUPNICK);
}

void ConnectionManager::on(UserConnectionListener::MyNick, UserConnection* aSource, const string& aNick) noexcept {
	if(aSource->getState() != UserConnection::STATE_SUPNICK) {
		// Already got this once, ignore...
		dcdebug("CM::onMyNick %p sent nick twice\n", (void*)aSource);
		return;
	}

	dcassert(!aNick.empty());
	dcdebug("ConnectionManager::onMyNick %p, %s\n", (void*)aSource, aNick.c_str());
	dcassert(!aSource->getUser());

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		// Try to guess where this came from...
		pair<string, string> i = expectedConnections.remove(aNick);
		if(i.second.empty()) {
			dcassert(i.first.empty());
			dcdebug("Unknown incoming connection from %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}
		aSource->setToken(i.first);
		aSource->setHubUrl(i.second);
		aSource->setEncoding(ClientManager::getInstance()->findHubEncoding(i.second));
	}

	string nick = Text::toUtf8(aNick, aSource->getEncoding());
	CID cid = ClientManager::getInstance()->makeCid(nick, aSource->getHubUrl());

	// First, we try looking in the pending downloads...hopefully it's one of them...
	{
		Lock l(cs);
		for(auto i = downloads.begin(); i != downloads.end(); ++i) {
			ConnectionQueueItem* cqi = *i;
			cqi->setErrors(0);
			if((cqi->getState() == ConnectionQueueItem::CONNECTING || cqi->getState() == ConnectionQueueItem::WAITING) &&
				cqi->getUser().user->getCID() == cid)
			{
				aSource->setUser(cqi->getUser());
				// Indicate that we're interested in this file...
				aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
				break;
			}
		}
	}

	if(!aSource->getUser()) {
		// Make sure we know who it is, i e that he/she is connected...
        ClientManager::getInstance()->lock();
        OnlineUser* ou = ClientManager::getInstance()->findOnlineUser(CID(cid),aSource->getHubUrl());
		if(ou)
            aSource->setUser(ou->getUser());
        //aSource->setUser(ClientManager::getInstance()->findUser(cid,aSource->getHubUrl()));
		//if(!ClientManager::getInstance()->isOnline(aSource->getUser())) {
        if(!ou) {
			dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", nick.c_str());
			putConnection(aSource);
			return;
		}
		// We don't need this connection for downloading...make it an upload connection instead...
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
	}

	ClientManager::getInstance()->setIpAddress(aSource->getUser(),aSource->getRemoteIp());

	if(UsersManager::getInstance()->isOp(aSource->getUser(), aSource->getHubUrl()))
		aSource->setFlag(UserConnection::FLAG_OP);

	if( aSource->isSet(UserConnection::FLAG_INCOMING) ) {
		aSource->myNick(aSource->getToken());
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	}

	aSource->setState(UserConnection::STATE_LOCK);
}

void ConnectionManager::on(UserConnectionListener::CLock, UserConnection* aSource, const string& aLock, const string& aPk) noexcept {
	if(aSource->getState() != UserConnection::STATE_LOCK) {
		dcdebug("CM::onLock %p received lock twice, ignoring\n", (void*)aSource);
		return;
	}

	if( CryptoManager::getInstance()->isExtended(aLock) ) {
		StringList defFeatures = features;
		if(SETTING(COMPRESS_TRANSFERS)) {
			defFeatures.push_back(UserConnection::FEATURE_ZLIB_GET);
		}

		aSource->supports(defFeatures);
	}

	aSource->setState(UserConnection::STATE_DIRECTION);
	aSource->direction(aSource->getDirectionString(), aSource->getNumber());
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));
//Detection ..Pk String/Lock string
	if(aSource->getHintedUser().user)
		ClientManager::getInstance()->setPkLock(aSource->getHintedUser(), aPk, aLock);

}

void ConnectionManager::on(UserConnectionListener::Direction, UserConnection* aSource, const string& dir, const string& num) noexcept {
	if(aSource->getState() != UserConnection::STATE_DIRECTION) {
		dcdebug("CM::onDirection %p received direction twice, ignoring\n", (void*)aSource);
		return;
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));
	if(dir == "Upload") {
		// Fine, the other fellow want's to send us data...make sure we really want that...
		if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
			// Huh? Strange...disconnect...
			putConnection(aSource);
			return;
		}
	} else {
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			int number = Util::toInt(num);
			// Damn, both want to download...the one with the highest number wins...
			if(aSource->getNumber() < number) {
				// Damn! We lost!
				aSource->unsetFlag(UserConnection::FLAG_DOWNLOAD);
				aSource->setFlag(UserConnection::FLAG_UPLOAD);
			} else if(aSource->getNumber() == number) {
				putConnection(aSource);
				return;
			}
		}
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));

	aSource->setState(UserConnection::STATE_KEY);
}

void ConnectionManager::addDownloadConnection(UserConnection* uc) {
	dcassert(uc->isSet(UserConnection::FLAG_DOWNLOAD));
	bool addConn = false;
	{
		Lock l(cs);

		auto i = find(downloads.begin(), downloads.end(), uc->getUser());
		if(i != downloads.end()) {
			ConnectionQueueItem* cqi = *i;
			if(cqi->getState() == ConnectionQueueItem::WAITING || cqi->getState() == ConnectionQueueItem::CONNECTING) {
				cqi->setState(ConnectionQueueItem::ACTIVE);
				uc->setFlag(UserConnection::FLAG_ASSOCIATED);
				uc->setHubUrl(cqi->getUser().hint); //set the correct hint for the uc, it might not even have a hint at first.

				fire(ConnectionManagerListener::Connected(), cqi);

				dcdebug("ConnectionManager::addDownloadConnection, leaving to downloadmanager\n");
				addConn = true;
			}
		}
	}

	if(addConn) {
		DownloadManager::getInstance()->addConnection(uc);
	} else {
		putConnection(uc);
	}
}

void ConnectionManager::addUploadConnection(UserConnection* uc) {
	dcassert(uc->isSet(UserConnection::FLAG_UPLOAD));

	bool addConn = false;
	{
		Lock l(cs);

		auto i = find(uploads.begin(), uploads.end(), uc->getUser());
		if(i == uploads.end()) {
			ConnectionQueueItem* cqi = getCQI(uc->getHintedUser(), false);

			cqi->setState(ConnectionQueueItem::ACTIVE);
			uc->setFlag(UserConnection::FLAG_ASSOCIATED);

			fire(ConnectionManagerListener::Connected(), cqi);

			dcdebug("ConnectionManager::addUploadConnection, leaving to uploadmanager\n");
			addConn = true;
		}
	}

	if(addConn) {
		UploadManager::getInstance()->addConnection(uc);
	} else {
		putConnection(uc);
	}
}

void ConnectionManager::on(UserConnectionListener::Key, UserConnection* aSource, const string&/* aKey*/) noexcept {
	if(aSource->getState() != UserConnection::STATE_KEY) {
		dcdebug("CM::onKey Bad state, ignoring");
		return;
	}

	dcassert(aSource->getUser());

	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		addDownloadConnection(aSource);
	} else {
		addUploadConnection(aSource);
	}
}

void ConnectionManager::on(AdcCommand::INF, UserConnection* aSource, const AdcCommand& cmd) noexcept {
	if(aSource->getState() != UserConnection::STATE_INF) {
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Expecting INF"));
		aSource->disconnect();
		return;
	}

	string cid;
	if(!cmd.getParam("ID", 0, cid)) {
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_INF_MISSING, "ID missing").addParam("FL", "ID"));
		dcdebug("CM::onINF missing ID\n");
		aSource->disconnect();
		return;
	}

	aSource->setUser(ClientManager::getInstance()->findUser(CID(cid),aSource->getHubUrl()));

	ClientManager::getInstance()->setIpAddress(aSource->getUser(),aSource->getRemoteIp());

	if(!aSource->getUser()) {
		dcdebug("CM::onINF: User not found");
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_GENERIC, "User not found"));
		putConnection(aSource);
		return;
	}

	if(!checkKeyprint(aSource)) {
		putConnection(aSource);
		return;
	}

	string token;
	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		if(!cmd.getParam("TO", 0, token)) {
			aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_GENERIC, "TO missing"));
			putConnection(aSource);
			return;
		}
	} else {
		token = aSource->getToken();
	}

	dcassert(!token.empty());
	{
		Lock l(cs);
		auto i = find(downloads.begin(), downloads.end(), aSource->getUser());

		if(i != downloads.end()) {
			ConnectionQueueItem* cqi = *i;
			cqi->setErrors(0);
			aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
        } else if (delayedTokens.find(token) == delayedTokens.end()) {
         aSource->setFlag(UserConnection::FLAG_UPLOAD);
		}
	}
	aSource->setToken(token);

	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		addDownloadConnection(aSource);
	} else if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
		addUploadConnection(aSource);
	}
}

void ConnectionManager::force(const UserPtr& aUser) {
	Lock l(cs);

	auto i = find(downloads.begin(), downloads.end(), aUser);
	if(i == downloads.end()) {
		return;
	}

	(*i)->setLastAttempt(0);
}

bool ConnectionManager::checkKeyprint(UserConnection *aSource) {
	if(!aSource->isSecure() || aSource->isTrusted())
         return true;
    string kp = ClientManager::getInstance()->getField(aSource->getUser()->getCID(), aSource->getHubUrl(), "KP");
    return aSource->verifyKeyprint(kp, SETTING(ALLOW_UNTRUSTED_CLIENTS));
}

void ConnectionManager::failed(UserConnection* aSource, const string& aError, bool protocolError) {
	Lock l(cs);

	if(aSource->isSet(UserConnection::FLAG_ASSOCIATED)) {
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			auto i = find(downloads.begin(), downloads.end(), aSource->getUser());
			dcassert(i != downloads.end());
			ConnectionQueueItem* cqi = *i;
			cqi->setState(ConnectionQueueItem::WAITING);
			cqi->setLastAttempt(GET_TICK());
			cqi->setErrors(protocolError ? -1 : (cqi->getErrors() + 1));
			fire(ConnectionManagerListener::Failed(), cqi, aError);
		} else if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
			auto i = find(uploads.begin(), uploads.end(), aSource->getUser());
			dcassert(i != uploads.end());
			ConnectionQueueItem* cqi = *i;
			putCQI(cqi);
		}
	}
	putConnection(aSource);
}

void ConnectionManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) noexcept {
	failed(aSource, aError, false);
}

void ConnectionManager::on(UserConnectionListener::ProtocolError, UserConnection* aSource, const string& aError) noexcept {
	if(aError.compare(0, 7, "CTM2HUB", 7) == 0) {

		{
			Lock l(cs);
			ddosCTM2HUBs.insert(Text::toLower(aSource->getRemoteIp()));
		}

		string aServerPort = aSource->getRemoteIp() + ":" + aSource->getPort();
		LogManager::getInstance()->message(autosprintf(_("Blocking '%s', potential DDoS detected (originating hub '%s')"), aServerPort.c_str(), aSource->getHubUrl().c_str() ));
	}

 	failed(aSource, aError, true);
}

void ConnectionManager::disconnect(const UserPtr& aUser) {
	Lock l(cs);
	for(auto i = userConnections.begin(); i != userConnections.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == aUser)
			uc->disconnect(true);
	}
}

void ConnectionManager::disconnect(const UserPtr& aUser, bool isDownload) {
	Lock l(cs);
	for(UserConnectionList::iterator i = userConnections.begin(); i != userConnections.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == aUser && uc->isSet(isDownload ? UserConnection::FLAG_DOWNLOAD : UserConnection::FLAG_UPLOAD)) {
			uc->disconnect(true);
			break;
		}
	}
}

void ConnectionManager::disconnectAll() {
	Lock l(cs);
	for(auto j: userConnections) {
		j->disconnect(true);
	}
}


void ConnectionManager::shutdown() {
	TimerManager::getInstance()->removeListener(this);
	shuttingDown = true;
	disconnect();
	disconnectAll();
	// Wait until all connections have died out...
	while(true) {
		{
			Lock l(cs);
			if(userConnections.empty()) {
				break;
			}
		}
		Thread::sleep(50);
	}
}

// UserConnectionListener
void ConnectionManager::on(UserConnectionListener::Supports, UserConnection* conn, const StringList& feat) noexcept {
	string sup = Util::emptyString;
	for(auto i = feat.begin(); i != feat.end(); ++i) {
	    sup += " " +*i;
		if(*i == UserConnection::FEATURE_MINISLOTS) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
		} else if(*i == UserConnection::FEATURE_XML_BZLIST) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
		} else if(*i == UserConnection::FEATURE_ADCGET) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
		} else if(*i == UserConnection::FEATURE_ZLIB_GET) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
		} else if(*i == UserConnection::FEATURE_TTHL) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
		} else if(*i == UserConnection::FEATURE_TTHF) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHF);
		}
	}
	ClientManager::getInstance()->setSupports(conn->getHintedUser(),sup);
}

 bool ConnectionManager::checkCTM2HUB(const string& aServer, const uint16_t& aPort, const string& aHubUrl)
 {
	const string server_lower = Text::toLower(aServer);
	dcassert(server_lower == aServer);

	bool is_ctm2hub = false;

	{
		Lock l(cs);
		is_ctm2hub = !ddosCTM2HUBs.empty() && ddosCTM2HUBs.find(server_lower) != ddosCTM2HUBs.end();
	}

    if(is_ctm2hub)
	{
		LogManager::getInstance()->message(autosprintf(_("Block CTM2HUB = '%s:%d' on hub '%s'"), aServer.c_str(), aPort, aHubUrl.c_str()));
		return true;
	}

	return false;
}
 

} // namespace dcpp
