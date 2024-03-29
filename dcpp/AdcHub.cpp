/*
 * Copyright (C) Jacek Sieka, arnetheduck on gmail point com
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
#include "AdcHub.h"
#include "format.h"

#include "ChatMessage.h"
#include "ClientManager.h"
#include "ShareManager.h"
#include "StringTokenizer.h"
#include "AdcCommand.h"
#include "ConnectionManager.h"
#include "version.h"
#include "Util.h"
#include "UserCommand.h"
#include "CryptoManager.h"
#include "LogManager.h"
#include "ThrottleManager.h"
#include "UploadManager.h"
#include "format.h"
#include "DebugManager.h"
#include <cmath>
#include "BufferedSocket.h"
#include "ConnectivityManager.h"
#include "nullptr.h"

namespace dcpp {

using std::make_pair;

const string AdcHub::CLIENT_PROTOCOL("ADC/1.0");
const string AdcHub::SECURE_CLIENT_PROTOCOL_TEST("ADCS/0.10");
const string AdcHub::ADCS_FEATURE("ADC0");
const string AdcHub::TCP4_FEATURE("TCP4");
const string AdcHub::TCP6_FEATURE("TCP6");
const string AdcHub::UDP4_FEATURE("UDP4");
const string AdcHub::UDP6_FEATURE("UDP6");
const string AdcHub::NAT0_FEATURE("NAT0");
const string AdcHub::SEGA_FEATURE("SEGA");

const string AdcHub::BASE_SUPPORT("ADBASE");
const string AdcHub::BAS0_SUPPORT("ADBAS0");
const string AdcHub::TIGR_SUPPORT("ADTIGR");
const string AdcHub::UCM0_SUPPORT("ADUCM0");
const string AdcHub::BLO0_SUPPORT("ADBLO0");
const string AdcHub::ZLIF_SUPPORT("ADZLIF");

const vector<StringList> AdcHub::searchExts;

AdcHub::AdcHub(const string& aHubURL, bool secure) :
	Client(aHubURL, '\n', secure), oldPassword(false), udp(Socket::TYPE_UDP), sid(0) {
}

AdcHub::~AdcHub() {
	clearUsers();
}

OnlineUser& AdcHub::getUser(const uint32_t aSID, const CID& aCID) {
	OnlineUser* pou = findUser(aSID);
	if(pou) {
		return *pou;
	}
	UserPtr pu = ClientManager::getInstance()->getUser(aCID);

	{
		Lock l(cs);
		pou = users.emplace(aSID, new OnlineUser(pu, *this, aSID)).first->second;
	}
	ClientManager::getInstance()->putOnline(pou);
	return *pou;
}

OnlineUser* AdcHub::findUser(const uint32_t aSID) const {
	Lock l(cs);
	SIDIter i = users.find(aSID);
	return i == users.end() ? nullptr : i->second;
}

OnlineUser* AdcHub::findUser(const CID& aCID) const {
	Lock l(cs);
	OnlineUser* pou = nullptr;
	for(SIDIter i = users.begin(); i != users.end(); ++i) {
		if(i->second->getUser()->getCID() == aCID) {
				pou =	i->second;
				break;
		}
	}
	return pou;
}

void AdcHub::putUser(const uint32_t aSID, bool disconnect) {
	OnlineUser* pou = nullptr;
	{
		Lock l(cs);
		SIDIter i = users.find(aSID);
		if(i == users.end())
			return;
		pou = i->second;
		users.erase(i);
	}

	ClientManager::getInstance()->putOffline(pou, disconnect);

	fire(ClientListener::UserRemoved(), this, *pou);
	delete pou;
}

void AdcHub::clearUsers() {
	SIDMap tmp;
	stopMyInfoCheck(); //BMDC++//RSX-like
	{
		Lock l(cs);
		users.swap(tmp);
	}

	for(SIDIter i = tmp.begin(); i != tmp.end(); ++i) {
		if(i->first != AdcCommand::HUB_SID) {
			ClientManager::getInstance()->putOffline(i->second);
			delete i->second;
		}
	}
	tmp.clear();
}

void AdcHub::handle(AdcCommand::INF, AdcCommand& c) noexcept {
	if(c.getParameters().empty())
		return;

	string sCid;

	OnlineUser* u = nullptr;
	if(c.getParam("ID", 0, sCid)) {
		u = findUser(CID(sCid));
		if(u) {
			if(u->getIdentity().getSID() != c.getFrom()) {
				// Same CID but different SID not allowed - buggy hub?
				string nick;
				if(!c.getParam("NI", 0, nick)) {
					nick = "[nick unknown]";
				}
				fire(ClientListener::StatusMessage(), this, autosprintf(_("%s has same CID %s as %s %s,%s, ignoring"), u->getIdentity().getNick().c_str()
				, u->getIdentity().getSIDString().c_str(), sCid.c_str(), nick.c_str(),AdcCommand::fromSID(c.getFrom()).c_str()).c_str()
					,ClientListener::FLAG_IS_SPAM);
				return;
			}
		} else {
			u = &getUser(c.getFrom(), CID(sCid));
		}
	} else if(c.getFrom() == AdcCommand::HUB_SID) {
		u = &getUser(c.getFrom(), CID());
	} else {
		u = findUser(c.getFrom());
	}

	if(!u) {
		LOG(PROTO, "AdcHub::INF Unknown user / no ID");
		DebugManager::getInstance()->SendCommandMessage("AdcHub::INF Unknown user / no ID\n", DebugManager::TYPE_HUB, DebugManager::INCOMING, "Unknow");
		return;
	}

	for(auto i = c.getParameters().begin(); i != c.getParameters().end(); ++i) {
		if(i->length() < 2)
			continue;
			
		if(*i == "U4") { u->getIdentity().setUdp4Port(Util::toInt(i->substr(2))); continue; }
		if(*i == "U6") { u->getIdentity().setUdp6Port(Util::toInt(i->substr(2))); continue; }

		u->getIdentity().set(i->c_str(), i->substr(2));
	}

	if(u->getIdentity().supports(ADCS_FEATURE)) {
		u->getUser()->setFlag(User::TLS);
	}

	if(getCheckAtConnect())
	{
		string sReport = u->getIdentity().myInfoDetect(*u);
		if(!sReport.empty()) {
			cheatMessage(sReport);
			updated(*u);
		}
	}

	if(u->getUser() == getMyIdentity().getUser()) {
		state = STATE_NORMAL;
		setAutoReconnect(true);
		setMyIdentity(u->getIdentity());
		updateCounts(false);
	}

	if(u->getIdentity().isHub()) {
		setHubIdentity(u->getIdentity());
		u->getIdentity().setBot(true);
		u->getIdentity().setOp(true);//@fix for the visible at top for HUB_SID
		fire(ClientListener::HubUpdated(), this);
		fire(ClientListener::UserUpdated(), this, *u);
	} else {
		fire(ClientListener::UserUpdated(), this, *u);
	}
}

void AdcHub::handle(AdcCommand::SUP, AdcCommand& c) noexcept {
	if(state != STATE_PROTOCOL) /** @todo SUP changes */
		return;
	bool baseOk = false;
	bool tigrOk = false;
	for(auto i = c.getParameters().begin(); i != c.getParameters().end(); ++i) {
		if(*i == BAS0_SUPPORT) {
			tigrOk = baseOk = true;
		} else if(*i == BASE_SUPPORT) {
			baseOk = true;
		} else if(*i == TIGR_SUPPORT) {
			tigrOk = true;
		}
	}

	if(!baseOk) {
		fire(ClientListener::StatusMessage(), this, _("Failed to negotiate base protocol"));
		disconnect(false);
		return;
	} else if(!tigrOk) {
		oldPassword = true;
		// Some hubs fake BASE support without TIGR support =/
		fire(ClientListener::StatusMessage(), this, _("Hub probably uses an old version of ADC, please encourage the owner to upgrade"));
	}
}

void AdcHub::handle(AdcCommand::SID, AdcCommand& c) noexcept {
	if(state != STATE_PROTOCOL) {
		dcdebug("Invalid state for SID\n");
		LOG(PROTO,"Invalid state for SID");
		return;
	}

	if(c.getParameters().empty())
		return;

	sid = AdcCommand::toSID(c.getParam(0));

	state = STATE_IDENTIFY;
	info();
}

void AdcHub::handle(AdcCommand::MSG, AdcCommand& c) noexcept {
	if(c.getParameters().empty())
		return;

	OnlineUser* from = findUser(c.getFrom());
	if(!from )
		return;

	OnlineUser* to = nullptr, *replyTo = nullptr;

	string temp;
	string chatMessage = c.getParam(0);
	if(c.getParam("PM", 1, temp)) { // add PM<group-cid> as well

		to = findUser(c.getTo());
		if(!to)
			return;

		replyTo = findUser(AdcCommand::toSID(temp));
		if(!replyTo)
			return;
	}
	fire(ClientListener::Message(), this, ChatMessage(chatMessage, from, to, replyTo, c.hasFlag("ME", 1),
		c.getParam("TS", 1, temp) ? Util::toInt64(temp) : 0));
}

void AdcHub::handle(AdcCommand::GPA, AdcCommand& c) noexcept {
	if(c.getParameters().empty())
		return;

	salt = c.getParam(0);
	state = STATE_VERIFY;

	fire(ClientListener::GetPassword(), this);
}

void AdcHub::handle(AdcCommand::QUI, AdcCommand& c) noexcept {
	uint32_t s = AdcCommand::toSID(c.getParam(0));

	OnlineUser* victim = findUser(s);
	string tmp;
	if(victim) {

		string dMessage;
		if(c.getParam("MS", 1, dMessage)) {
			OnlineUser* source = nullptr;
			string tmp2;
			if(c.getParam("ID", 1, tmp2)) {
				source = findUser(AdcCommand::toSID(tmp2));
			}

			tmp = victim->getIdentity().getNick();

			if(source) {
				tmp += " was kicked by " + source->getIdentity().getNick() + ": " + dMessage;
			} else {
				tmp += " was kicked: "  + dMessage;
			}
			fire(ClientListener::StatusMessage(), this, tmp, ClientListener::FLAG_IS_SPAM);
		}

		putUser(s, c.getParam("DI", 1, tmp));
	}

	if(s == sid) {
		// this QUI is directed to us
		string tmp;
		if(c.getParam("TL", 1, tmp)) {
			if(tmp == "-1") {
				setAutoReconnect(false);
			} else {
				setAutoReconnect(true);
				setReconnDelay(Util::toUInt32(tmp));
			}
		}
		if(!victim && c.getParam("MS", 1, tmp)) {
			fire(ClientListener::StatusMessage(), this, tmp, ClientListener::FLAG_NORMAL);
		}
		if(c.getParam("RD", 1, tmp)) {
			disconnect(false);
			fire(ClientListener::Redirect(), this, tmp);
		}
	}
}

void AdcHub::handle(AdcCommand::CTM, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 3)
		return;

	OnlineUser* u = findUser(c.getFrom());
	if(!u || u->getUser() == ClientManager::getInstance()->getMe())
		return;

	const string& protocol = c.getParam(0);
	const string& port = c.getParam(1);
	const string& token = c.getParam(2);

	bool secure = false;
	if(protocol == CLIENT_PROTOCOL) {
		// Nothing special
	} else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
		secure = true;
	} else {
		unknownProtocol(c.getFrom(), protocol, token);
		return;
	}

	if(!u->getIdentity().isTcpActive(this)) {
		send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "IP unknown", AdcCommand::TYPE_DIRECT).setTo(c.getFrom()));
		return;
	}

	ConnectionManager::getInstance()->adcConnect(*u, Util::toInt(port), token, secure);
}

void AdcHub::handle(AdcCommand::RCM, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 2) {
		return;
	}

	OnlineUser* u = findUser(c.getFrom());
	if(!u || u->getUser() == ClientManager::getInstance()->getMe())
		return;

	const string& protocol = c.getParam(0);
	const string& token = c.getParam(1);

	bool secure = false;
	if(protocol == CLIENT_PROTOCOL) {
		//Nothink
	} else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
		secure = true;
	} else {
		unknownProtocol(c.getFrom(), protocol, token);
		return;
	}

	if(ClientManager::getInstance()->isActive(getHubUrl())) {
		connect(*u, token, secure);
		return;
	}

	if (!u->getIdentity().supports(NAT0_FEATURE))
		return;

	// Attempt to traverse NATs and/or firewalls with TCP.
	// If they respond with their own, symmetric, RNT command, both
	// clients call ConnectionManager::adcConnect.
	send(AdcCommand(AdcCommand::CMD_NAT, u->getIdentity().getSID(), AdcCommand::TYPE_DIRECT).
		addParam(protocol).addParam(Util::toString(sock->getLocalPort())).addParam(token));
}

void AdcHub::handle(AdcCommand::CMD, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 1)
		return;
	/*
		Did we need this?
	if(c.getFrom() != AdcCommand::HUB_SID)
		return;
	*/
	const string& name = c.getParam(0);
	bool rem = c.hasFlag("RM", 1);
	if(rem) {
		fire(ClientListener::HubUserCommand(), this, (int)UserCommand::TYPE_REMOVE, 0, name, string());
		return;
	}
	bool sep = c.hasFlag("SP", 1);
	string sctx;
	if(!c.getParam("CT", 1, sctx))
		return;
	int ctx = Util::toInt(sctx);
	if(ctx <= 0)
		return;
	if(sep) {
		fire(ClientListener::HubUserCommand(), this, (int)UserCommand::TYPE_SEPARATOR, ctx, name, string());
		return;
	}
	bool once = c.hasFlag("CO", 1);
	string txt;
	if(!c.getParam("TT", 1, txt))
		return;
	fire(ClientListener::HubUserCommand(), this, (int)(once ? UserCommand::TYPE_RAW_ONCE : UserCommand::TYPE_RAW), ctx, name, txt);
}

void AdcHub::sendUDP(const AdcCommand& cmd) noexcept {
	string command;
	string ip;
	uint16_t port;
	{
		Lock l(cs);
		SIDIter i = users.find(cmd.getTo());
		if(i == users.end()) {
			dcdebug("AdcHub::sendUDP: invalid user\n");//Possible alow logging?
			LOG(PROTO,"AdcHub::sendUDP: invalid user");
			return;
		}
		OnlineUser& ou = *i->second;
		if(!ou.getIdentity().isUdpActive()) {
			return;
		}
		ip = ou.getIdentity().getIp();
		port = ou.getIdentity().getUdpPort();
		command = cmd.toString(ou.getUser()->getCID());
	}
	try {
		udp.writeTo(ip, port, command);
	} catch(const SocketException& e) {
		dcdebug("AdcHub::sendUDP: write failed: %s\n", e.getError().c_str());
		udp.close();
	}
}

void AdcHub::handle(AdcCommand::STA, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 2)
		return;

	OnlineUser* u = c.getFrom() == AdcCommand::HUB_SID ? &getUser(c.getFrom(), CID()) : findUser(c.getFrom());
	if(!u)
		return;

	if(c.getParam(0).size() != 3) {
		return;
	}

	switch(Util::toInt(c.getParam(0).substr(1))) {

	case AdcCommand::ERROR_BAD_PASSWORD:
		{
			if(c.getType() == AdcCommand::TYPE_INFO) {
				setPassword(Util::emptyString);
			}
			break;
		}

	case AdcCommand::ERROR_COMMAND_ACCESS:
		{
			if(c.getType() == AdcCommand::TYPE_INFO) {
				string tmp;
				if(c.getParam("FC", 1, tmp) && tmp.size() == 4)
					forbiddenCommands.insert(AdcCommand::toFourCC(tmp.c_str()));
			}
			break;
		}

	case AdcCommand::ERROR_PROTOCOL_UNSUPPORTED:
		{
			string tmp;
			if(c.getParam("PR", 1, tmp)) {
				if(tmp == CLIENT_PROTOCOL) {
					u->getUser()->setFlag(User::NO_ADC_1_0_PROTOCOL);
				} else if(tmp == SECURE_CLIENT_PROTOCOL_TEST) {
					u->getUser()->setFlag(User::NO_ADCS_0_10_PROTOCOL);
					u->getUser()->unsetFlag(User::TLS);
				}
				// Try again...
				ConnectionManager::getInstance()->force(u->getUser());
			}
			return;
		}
	}

	ChatMessage message = { c.getParam(1), u };
	fire(ClientListener::Message(), this, message);
}

void AdcHub::handle(AdcCommand::SCH, AdcCommand& c) noexcept {
	if(getHideShare()) // Hide Share
		return;

	OnlineUser* ou = findUser(c.getFrom());
	if(!ou) {
		dcdebug("Invalid user in AdcHub::onSCH\n");//should be logged ?
		return;
	}

	fire(ClientListener::AdcSearch(), this, c, ou->getUser()->getCID());
}

void AdcHub::handle(AdcCommand::RES, AdcCommand& c) noexcept {
	OnlineUser* ou = findUser(c.getFrom());
	if(!ou) {
		dcdebug("Invalid user in AdcHub::onRES\n");//logging?
		return;
	}
	SearchManager::getInstance()->onRES(c, ou->getUser());
}

void AdcHub::handle(AdcCommand::GET, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 5) {
		if(!c.getParameters().empty()) {
			if(c.getParam(0) == "blom") {
				send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC,
					"Too few parameters for blom", AdcCommand::TYPE_HUB));
			} else {
				send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
					"Unknown transfer type", AdcCommand::TYPE_HUB));
			}
		} else {
			send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC,
				"Too few parameters for GET", AdcCommand::TYPE_HUB));
		}
		return;
	}

	const string& type = c.getParam(0);
	string sk, sh;
	if(type == "blom" && c.getParam("BK", 4, sk) && c.getParam("BH", 4, sh))  {
		ByteVector v;
		size_t m = Util::toUInt32(c.getParam(3)) * 8;
		size_t k = Util::toUInt32(sk);
		size_t h = Util::toUInt32(sh);

		if(k > 8 || k < 1) {
			send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
				"Unsupported k", AdcCommand::TYPE_HUB));
			return;
		}
		if(h > 64 || h < 1) {
			send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
				"Unsupported h", AdcCommand::TYPE_HUB));
			return;
		}

		ShareManager* sm = getShareManager();

		size_t n = sm->getSharedFiles();

		// When h >= 32, m can't go above 2^h anyway since it's stored in a size_t.
        if(m > (5. * (size_t)Util::roundUp((int64_t)(n * k / log(2.)), (int64_t)64.)) || (h < 32. && m > static_cast<size_t>(1U << h))) {
			send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
				"Unsupported m", AdcCommand::TYPE_HUB));
			return;
		}

		AdcCommand cmd(AdcCommand::CMD_SND, AdcCommand::TYPE_HUB);
		cmd.addParam(c.getParam(0));
		cmd.addParam(c.getParam(1));
		cmd.addParam(c.getParam(2));
		cmd.addParam(c.getParam(3));
		cmd.addParam(c.getParam(4));
		send(cmd);
		
		if (m > 0) {
			sm->getBloom(v, k, m, h);
			send((char*)&v[0], v.size());
		}
	}
}

void AdcHub::handle(AdcCommand::NAT, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 3)
		return;

	OnlineUser* u = findUser(c.getFrom());
	if(!u || u->getUser() == ClientManager::getInstance()->getMe() || c.getParameters().size() < 3)
		return;

	const string& protocol = c.getParam(0);
	const string& port = c.getParam(1);
	const string& token = c.getParam(2);

	bool secure = false;
	if(protocol == CLIENT_PROTOCOL) {
		// Nothing special
	} else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
		secure = true;
	} else {
		unknownProtocol(c.getFrom(), protocol, token);
		return;
	}

	// Trigger connection attempt sequence locally ...
	uint16_t localPort = sock->getLocalPort();
	dcdebug("triggering connecting attempt in NAT: remote port = %s, local port = %d\n", port.c_str(), sock->getLocalPort());
	ConnectionManager::getInstance()->adcConnect(*u, Util::toInt(port), localPort, BufferedSocket::NAT_CLIENT, token, secure);

	// ... and signal other client to do likewise.
	send(AdcCommand(AdcCommand::CMD_RNT, u->getIdentity().getSID(), AdcCommand::TYPE_DIRECT).addParam(protocol).
		addParam(Util::toString(localPort)).addParam(token));
}

void AdcHub::handle(AdcCommand::RNT, AdcCommand& c) noexcept {
	if(c.getParameters().size() < 3)
		return;

	// Sent request for NAT traversal cooperation, which
	// was acknowledged (with requisite local port information).
	OnlineUser* u = findUser(c.getFrom());
	if(!u || u->getUser() == ClientManager::getInstance()->getMe())
		return;

	const string& protocol = c.getParam(0);
	const string& port = c.getParam(1);
	const string& token = c.getParam(2);

	bool secure = false;
	if(protocol == CLIENT_PROTOCOL) {
		// Nothing special
	} else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
		secure = true;
	} else {
		unknownProtocol(c.getFrom(), protocol, token);
		return;
	}

	// Trigger connection attempt sequence locally
	dcdebug("triggering connecting attempt in RNT: remote port = %s, local port = %d\n", port.c_str(), sock->getLocalPort());
	ConnectionManager::getInstance()->adcConnect(*u, Util::toInt(port), sock->getLocalPort(), BufferedSocket::NAT_SERVER, token, secure);
}

void AdcHub::handle(AdcCommand::ZON, AdcCommand& c) noexcept {
	if(c.getType() == AdcCommand::TYPE_INFO) {
		try {
			sock->setMode(BufferedSocket::MODE_ZPIPE);
		} catch (const Exception& e) {
			dcdebug("AdcHub::handleZON failed with error: %s\n", e.getError().c_str());
		}
	}
}

void AdcHub::handle(AdcCommand::ZOF, AdcCommand& c) noexcept {
	if(c.getType() == AdcCommand::TYPE_INFO) {
		try {
			sock->setMode(BufferedSocket::MODE_LINE);
		} catch (const Exception& e) {
			dcdebug("AdcHub::handleZOF failed with error: %s\n", e.getError().c_str());
		}
	}
}

void AdcHub::connect(const OnlineUser& user, const string& token) {
	connect(user, token, CryptoManager::getInstance()->TLSOk() && user.getUser()->isSet(User::TLS));
}

void AdcHub::connect(const OnlineUser& user, string const& token, bool secure) {
	if(state != STATE_NORMAL)
		return;
	Lock l(cs);
	string proto;
	if(secure) {
		if(user.getUser()->isSet(User::NO_ADCS_0_10_PROTOCOL)) {
			/// @todo log
			return;
		}
		proto = SECURE_CLIENT_PROTOCOL_TEST;
	} else {
		if(user.getUser()->isSet(User::NO_ADC_1_0_PROTOCOL)) {
			/// @todo log
			return;
		}
		proto = CLIENT_PROTOCOL;
	}

	if(ClientManager::getInstance()->isActive(getHubUrl())) {
		const string& port = secure ? Util::toString(ConnectionManager::getInstance()->getSecurePort()) : Util::toString(ConnectionManager::getInstance()->getPort());
		if(port.empty()) {
			// Oops?
			LogManager::getInstance()->message((F_("Not listening for connections - please restart ") + string(APPNAME)) , LogManager::Sev::HIGH);
			return;
		}
		send(AdcCommand(AdcCommand::CMD_CTM, user.getIdentity().getSID(), AdcCommand::TYPE_DIRECT).addParam(proto).addParam(port).addParam(token));
	} else {
		send(AdcCommand(AdcCommand::CMD_RCM, user.getIdentity().getSID(), AdcCommand::TYPE_DIRECT).addParam(proto).addParam(token));
	}
}

void AdcHub::hubMessage(const string& aMessage, bool thirdPerson) {
	if(state != STATE_NORMAL)
		return;
	AdcCommand c(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST);
	c.addParam(aMessage);
	if(thirdPerson)
		c.addParam("ME", "1");
	send(c);
}

void AdcHub::privateMessage(const OnlineUser& user, const string& aMessage, bool thirdPerson) {
	if(state != STATE_NORMAL)
		return;

	AdcCommand c(AdcCommand::CMD_MSG, user.getIdentity().getSID(), AdcCommand::TYPE_ECHO);
	c.addParam(aMessage);
	if(thirdPerson)
		c.addParam("ME", "1");
	c.addParam("PM", getMySID());
	send(c);
}

void AdcHub::sendUserCmd(const UserCommand& command, const ParamMap& params) {
	if(state != STATE_NORMAL)
		return;

	string cmd = Util::formatParams(command.getCommand(), params, escape);
	if(command.isChat()) {
		if(command.getTo().empty()) {
			hubMessage(cmd);
		} else {
			const string& to = command.getTo();
			Lock l(cs);
			for(SIDIter i = users.begin(); i != users.end(); ++i) {
				if(i->second->getIdentity().getNick() == to) {
					privateMessage(*i->second, cmd);
					return;
				}
			}
		}
	} else {
		send(cmd);
	}
}

const vector<StringList>& AdcHub::getSearchExts() {
	if(!searchExts.empty())
		return searchExts;

	// the list is always immutable except for this function where it is initially being filled.
	auto& xSearchExts = const_cast<vector<StringList>&>(searchExts);

	xSearchExts.resize(6);

	/// @todo simplify this as searchExts[0] = { "mp3", "etc" } when VC++ supports initializer lists

	// these extensions *must* be sorted alphabetically!

	{
		StringList& l = xSearchExts[0];
		l.push_back("ape"); l.push_back("flac"); l.push_back("m4a"); l.push_back("mid");
		l.push_back("mp3"); l.push_back("mpc"); l.push_back("ogg"); l.push_back("ra");
		l.push_back("wav"); l.push_back("wma");
	}

	{
		StringList& l = xSearchExts[1];
		l.push_back("7z"); l.push_back("ace"); l.push_back("arj"); l.push_back("bz2");
		l.push_back("gz"); l.push_back("lha"); l.push_back("lzh"); l.push_back("rar");
		l.push_back("tar"); l.push_back("z"); l.push_back("zip");
	}

	{
		StringList& l = xSearchExts[2];
		l.push_back("doc"); l.push_back("docx"); l.push_back("htm"); l.push_back("html");
		l.push_back("nfo"); l.push_back("odf"); l.push_back("odp"); l.push_back("ods");
		l.push_back("odt"); l.push_back("pdf"); l.push_back("ppt"); l.push_back("pptx");
		l.push_back("rtf"); l.push_back("txt"); l.push_back("xls"); l.push_back("xlsx");
		l.push_back("xml"); l.push_back("xps");
	}

	{
		StringList& l = xSearchExts[3];
		l.push_back("app"); l.push_back("bat"); l.push_back("cmd"); l.push_back("com");
		l.push_back("dll"); l.push_back("exe"); l.push_back("jar"); l.push_back("msi");
		l.push_back("ps1"); l.push_back("vbs"); l.push_back("wsf");
	}

	{
		StringList& l = xSearchExts[4];
		l.push_back("bmp"); l.push_back("cdr"); l.push_back("eps"); l.push_back("gif");
		l.push_back("ico"); l.push_back("img"); l.push_back("jpeg"); l.push_back("jpg");
		l.push_back("png"); l.push_back("ps"); l.push_back("psd"); l.push_back("sfw");
		l.push_back("tga"); l.push_back("tif"); l.push_back("webp");
	}

	{
		StringList& l = xSearchExts[5];
		l.push_back("3gp"); l.push_back("asf"); l.push_back("asx"); l.push_back("avi");
		l.push_back("divx"); l.push_back("flv"); l.push_back("mkv"); l.push_back("mov");
		l.push_back("mp4"); l.push_back("mpeg"); l.push_back("mpg"); l.push_back("ogm");
		l.push_back("pxp"); l.push_back("qt"); l.push_back("rm"); l.push_back("rmvb");
		l.push_back("swf"); l.push_back("vob"); l.push_back("webm"); l.push_back("wmv");
	}

	return searchExts;
}

StringList AdcHub::parseSearchExts(int flag) {
	StringList ret;
	const auto& searchExts = getSearchExts();
	for(auto i = searchExts.cbegin(), iend = searchExts.cend(); i != iend; ++i) {
		if(flag & (1 << (i - searchExts.cbegin()))) {
			ret.insert(ret.begin(), i->begin(), i->end());
		}
	}
	return ret;
}

void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList) {
	if(state != STATE_NORMAL)
		return;

	AdcCommand c(AdcCommand::CMD_SCH, AdcCommand::TYPE_BROADCAST);

	/* token format: [per-hub unique id] "/" [per-search actual token]
	this allows easily knowing which hub a search was sent on when parsing a search result,
	whithout having to bother maintaining a list of sent tokens. */
	c.addParam("TO", std::to_string(getUniqueId()) + "/" + aToken);

	if(aFileType == SearchManager::TYPE_TTH) {
		c.addParam("TR", aString);

	} else {
		if(aSizeMode == SearchManager::SIZE_ATLEAST) {
			c.addParam("GE", Util::toString(aSize));
		} else if(aSizeMode == SearchManager::SIZE_ATMOST) {
			c.addParam("LE", Util::toString(aSize));
		}

		StringTokenizer<string> st(aString, ' ');
		for(auto i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
			c.addParam("AN", *i);
		}

		if(aFileType == SearchManager::TYPE_DIRECTORY) {
			c.addParam("TY", "2");
		}

		if(aExtList.size() > 2) {
			StringList exts = aExtList;
			sort(exts.begin(), exts.end());

			uint8_t gr = 0;
			StringList rx;

			const auto& searchExts = getSearchExts();
			for(auto i = searchExts.cbegin(), iend = searchExts.cend(); i != iend; ++i) {
				const StringList& def = *i;

				// gather the exts not present in any of the lists
				StringList temp(def.size() + exts.size());
				temp = StringList(temp.begin(), set_symmetric_difference(def.begin(), def.end(),
					exts.begin(), exts.end(), temp.begin()));

				// figure out whether the remaining exts have to be added or removed from the set
				StringList rx_;
				bool ok = true;
				for(auto diff = temp.begin(); diff != temp.end();) {
					if(find(def.cbegin(), def.cend(), *diff) == def.cend()) {
						++diff; // will be added further below as an "EX"
					} else {
						if(rx_.size() == 2) {
							ok = false;
							break;
						}
						rx_.push_back(*diff);
						diff = temp.erase(diff);
					}
				}
				if(!ok) // too many "RX"s necessary - disregard this group
					continue;

				// let's include this group!
				gr += 1 << (i - searchExts.cbegin());

				exts = temp; // the exts to still add (that were not defined in the group)

				rx.insert(rx.begin(), rx_.begin(), rx_.end());

				if(exts.size() <= 2)
					break;
				// keep looping to see if there are more exts that can be grouped
			}

			if(gr) {
				// some extensions can be grouped; let's send a command with grouped exts.
				AdcCommand c_gr(AdcCommand::CMD_SCH, AdcCommand::TYPE_FEATURE);
				c_gr.setFeatures('+' + SEGA_FEATURE);

				const auto& params = c.getParameters();
				for(auto i = params.cbegin(), iend = params.cend(); i != iend; ++i)
					c_gr.addParam(*i);

				for(auto i = exts.cbegin(), iend = exts.cend(); i != iend; ++i)
					c_gr.addParam("EX", *i);
				c_gr.addParam("GR", Util::toString(gr));
				for(auto i = rx.cbegin(), iend = rx.cend(); i != iend; ++i)
					c_gr.addParam("RX", *i);

				sendSearch(c_gr);

				// make sure users with the feature don't receive the search twice.
				c.setType(AdcCommand::TYPE_FEATURE);
				c.setFeatures('-' + SEGA_FEATURE);
			}
		}

		for(auto i = aExtList.cbegin(), iend = aExtList.cend(); i != iend; ++i)
			c.addParam("EX", *i);
	}

	sendSearch(c);
}

void AdcHub::sendSearch(AdcCommand& c) {
	if(ClientManager::getInstance()->isActive(getHubUrl())) {
		send(c);
	} else {
		c.setType(AdcCommand::TYPE_FEATURE);
		string features = c.getFeatures();
		c.setFeatures(features + '+' + TCP4_FEATURE + '-' + NAT0_FEATURE);
		send(c);
	}
}

void AdcHub::password(const string& pwd) {
	if(state != STATE_VERIFY)
		return;

	if(!salt.empty()) {
		size_t saltBytes = salt.size() * 5 / 8;
		uint8_t *buf = new uint8_t[saltBytes];
		Encoder::fromBase32(salt.c_str(), buf, saltBytes);
		TigerHash th;
		if(oldPassword) {
			CID cid = getMyIdentity().getUser()->getCID();
			th.update(cid.data(), CID::SIZE);
		}
		th.update(pwd.data(), pwd.length());
		th.update(buf, saltBytes);
		delete [] buf;
		send(AdcCommand(AdcCommand::CMD_PAS, AdcCommand::TYPE_HUB).addParam(Encoder::toBase32(th.finalize(), TigerHash::BYTES)));
		salt.clear();
	}
}

void AdcHub::addParam(StringMap& lastInfoMap, AdcCommand& c, const string& var, const string& value) {
	StringMap::iterator i = lastInfoMap.find(var);

	if(i != lastInfoMap.end()) {
		if(i->second != value) {
			if(value.empty()) {
				lastInfoMap.erase(i);
			} else {
				i->second = value;
			}
			c.addParam(var, value);
		}
	} else if(!value.empty()) {
		lastInfoMap.insert(make_pair(var, value));
		c.addParam(var, value);
	}
}

void AdcHub::infoImpl() 
{
	if((state != STATE_IDENTIFY) && (state != STATE_NORMAL))
				return;

	reloadSettings(false);

	AdcCommand c(AdcCommand::CMD_INF, AdcCommand::TYPE_BROADCAST);

	if (state == STATE_NORMAL) {
		updateCounts(false);
	}

	bool bIsFreeSlots = SETTING(SHOW_FREE_SLOTS_DESC);//TODO: is good per Fav ?
	string freeslots = "[" + Util::toString(UploadManager::getInstance()->getFreeSlots()) + "]";
	ShareManager* sm = getShareManager();
	
	addParam(lastInfoMap, c, "ID", ClientManager::getInstance()->getMyCID().toBase32());
	addParam(lastInfoMap, c, "PD", ClientManager::getInstance()->getMyPID().toBase32());
	addParam(lastInfoMap, c, "NI", HUBSETTING(NICK));
	addParam(lastInfoMap, c, "DE", bIsFreeSlots ? freeslots + " " + HUBSETTING(DESCRIPTION) : HUBSETTING(DESCRIPTION) );
	addParam(lastInfoMap, c, "SL", Util::toString(SETTING(SLOTS)));
	addParam(lastInfoMap, c, "FS", Util::toString(UploadManager::getInstance()->getFreeSlots()));
	addParam(lastInfoMap, c, "SS", getHideShare() ? "0" : sm->getShareSizeString());
	addParam(lastInfoMap, c, "SF", getHideShare() ? "0" : std::to_string(sm->getSharedFiles()));
	addParam(lastInfoMap, c, "EM", HUBSETTING(EMAIL));
	addParam(lastInfoMap, c, "HN", std::to_string(counts[COUNT_NORMAL]));
	addParam(lastInfoMap, c, "HR", std::to_string(counts[COUNT_REGISTERED]));
	addParam(lastInfoMap, c, "HO", std::to_string(counts[COUNT_OP]));
	addParam(lastInfoMap, c, "AP", APPNAME);
	addParam(lastInfoMap, c, "VE", VERSIONSTRING);
	addParam(lastInfoMap, c, "AW", Util::getAway() ? "1" : Util::emptyString);
	// RF = ref address from connected...
	addParam(lastInfoMap, c, "RF", getHubUrl());

	int64_t iLimit = ThrottleManager::getInstance()->getDownLimit();
	if (iLimit > 0) {
		addParam(lastInfoMap, c, "DS", Util::toString(iLimit * 1024));
	} else {
		addParam(lastInfoMap, c, "DS", Util::emptyString);
	}

	iLimit = ThrottleManager::getInstance()->getUpLimit();
	if (iLimit > 0) {
		addParam(lastInfoMap, c, "US", Util::toString(iLimit * 1024));
	} else {
		addParam(lastInfoMap, c, "US", std::to_string((long)(Util::toDouble(SETTING(UPLOAD_SPEED))*1024*1024/8)));
	}

	string su(SEGA_FEATURE);

	if(CryptoManager::getInstance()->TLSOk()) {
		su += "," + ADCS_FEATURE;
		const vector<uint8_t> kp = CryptoManager::getInstance()->getKeyprint();
		addParam(lastInfoMap, c, "KP", "SHA256/" + Encoder::toBase32(&kp[0], kp.size()));
	}

	addParam(lastInfoMap, c, "LC", Util::getIETFLang());

	bool addV4 = !sock->isV6Valid() || ( ((int)HUBSETTING(INCOMING_CONNECTIONS)) <= 2);
	bool addV6 = sock->isV6Valid() || ( !((string)HUBSETTING(EXTERNAL_IP6)).empty()) ;

	if(addV4 && isActiveV4()) {
		su += "," + TCP4_FEATURE;
		su += "," + UDP4_FEATURE;
	}

	if(addV6 && isActiveV6()) {
		su += "," + TCP6_FEATURE;
		su += "," + UDP6_FEATURE;
	}

	if ( (addV6 && !isActiveV6() && (!HUBSETTING(EXTERNAL_IP6).empty()))
		|| ( (addV4 && !isActiveV4()) && ((int)HUBSETTING(INCOMING_CONNECTIONS) <= 2) ))
	{
		su += "," + NAT0_FEATURE;
	}

	addParam(lastInfoMap, c, "SU", su);

	appendConnectivity(lastInfoMap, c, addV4, addV6);

	if(!c.getParameters().empty()) {
		send(c);
	}
}

uint64_t AdcHub::getAvailable() const {
	Lock l(cs);
	uint64_t x = 0;
	for(SIDIter i = users.begin(); i != users.end(); ++i) {
		x += i->second->getIdentity().getBytesShared();
	}
	return x;
}

void AdcHub::checkNick(string& nick) {
	for(size_t i = 0, n = nick.size(); i < n; ++i) {
          if(static_cast<uint8_t>(nick[i]) <= 32) {
                nick[i] = '_';
           }
	}
}

void AdcHub::send(const AdcCommand& cmd) {
	if(forbiddenCommands.find(AdcCommand::toFourCC(cmd.getFourCC().c_str())) == forbiddenCommands.end()) {
		if(cmd.getType() == AdcCommand::TYPE_UDP)
			sendUDP(cmd);
		send(cmd.toString(sid));
	}
}

void AdcHub::unknownProtocol(uint32_t target, const string& protocol, const string& token) {
	AdcCommand cmd(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_DIRECT);
	cmd.setTo(target);
	cmd.addParam("PR", protocol);
	cmd.addParam("TO", token);

	send(cmd);
}

void AdcHub::on(Connected c) noexcept {
	Client::on(c);

	if(state != STATE_PROTOCOL) {
		return;
	}

	lastInfoMap.clear();
	sid = 0;
	forbiddenCommands.clear();

	AdcCommand cmd(AdcCommand::CMD_SUP, AdcCommand::TYPE_HUB);
	cmd.addParam(BAS0_SUPPORT).addParam(BASE_SUPPORT).addParam(TIGR_SUPPORT);

	if(SETTING(HUB_USER_COMMANDS)) {
		cmd.addParam(UCM0_SUPPORT);
	}
	if(SETTING(SEND_BLOOM)) {
		cmd.addParam(BLO0_SUPPORT);
	}

	cmd.addParam(ZLIF_SUPPORT);

	send(cmd);

}

void AdcHub::on(Line l, const string& aLine) noexcept {
	Client::on(l, aLine);

	if(!Text::validateUtf8(aLine)) {
		// @todo report to user?
		dcdebug("non-valid utf-8");
		return;
	}

	COMMAND_DEBUG(aLine,TYPE_HUB,INCOMING,getHubUrl());

	dispatch(aLine);
}

void AdcHub::on(Failed f, const string& aLine) noexcept {
	clearUsers();
	Client::on(f, aLine);
}

void AdcHub::on(Second s, uint64_t aTick) noexcept {
	Client::on(s, aTick);
	if(state == STATE_NORMAL && (aTick > (getLastActivity() + 120*1000))) {
		send("\n", 1);
	}
}
//Refresh UL
void AdcHub::refreshuserlist() {
	Lock l(cs);

	OnlineUserList v;
	for(SIDIter i = users.begin(); i != users.end(); ++i) {
		if(i->first != AdcCommand::HUB_SID) {
			v.push_back(i->second);
		}
	}
	fire(ClientListener::UsersUpdated(), this, v);
}


void AdcHub::appendConnectivity(StringMap& lastInfoMap, AdcCommand& c, bool v4, bool v6) {
	if (v4) {
		if(CONNSETTING(NO_IP_OVERRIDE) && !getUserIp4().empty()) {
			addParam(lastInfoMap, c, "I4", Socket::resolve(getUserIp4(), AF_INET));
		} else {
			addParam(lastInfoMap, c, "I4", "0.0.0.0");
		}

		if(isActiveV4()) {
			addParam(lastInfoMap, c, "U4", Util::toString(SearchManager::getInstance()->getPort()));
		} else {
			addParam(lastInfoMap, c, "U4", "");
		}
	} else {
		addParam(lastInfoMap, c, "I4", "");
		addParam(lastInfoMap, c, "U4", "");
	}

	if (v6) {
		if (CONNSETTING(NO_IP_OVERRIDE) && !getUserIp6().empty()) {
			addParam(lastInfoMap, c, "I6", Socket::resolve(getUserIp6(), AF_INET6));
		} else {
			addParam(lastInfoMap, c, "I6", "::");
		}

		if(isActiveV6()) {
			addParam(lastInfoMap, c, "U6", Util::toString(SearchManager::getInstance()->getPort()));
		} else {
			addParam(lastInfoMap, c, "U6", "");
		}
	} else {
		addParam(lastInfoMap, c, "I6", "");
		addParam(lastInfoMap, c, "U6", "");
	}
}


} // namespace dcpp
