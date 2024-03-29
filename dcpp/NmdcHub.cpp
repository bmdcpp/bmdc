/*
 * Copyright (C) 2001-2018 Jacek Sieka, arnetheduck on gmail point com
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
#include "NmdcHub.h"

#include "ChatMessage.h"
#include "ClientManager.h"
#include "SearchManager.h"
#include "ShareManager.h"
#include "ConnectivityManager.h"
#include "CryptoManager.h"
#include "ConnectionManager.h"
#include "ThrottleManager.h"
#include "UploadManager.h"
#include "version.h"
#include "DebugManager.h"
#include "Socket.h"
#include "UserCommand.h"
#include "StringTokenizer.h"
#include "format.h"
#include "AVManager.h"

namespace dcpp {

NmdcHub::NmdcHub(const string& aHubURL) :
Client(aHubURL, '|', false),
supportFlags(0), lastUpdate(0),lastProtectedIPsUpdate(0)
{
	
}

NmdcHub::~NmdcHub() {
	clearUsers();
}

#define checkstate() if(state != STATE_NORMAL) return

void NmdcHub::connect(const OnlineUser& aUser, const string&) {
	checkstate();
	Lock l(cs);
	dcdebug("NmdcHub::connect %s\n", aUser.getIdentity().getNick().c_str());
	if(ClientManager::getInstance()->isActive(getHubUrl()) || ( bIPv6 && !aUser.getIdentity().getIp6().empty() ) ) {
		connectToMe(aUser);
	} else {
		revConnectToMe(aUser);
	}
}

uint64_t NmdcHub::getAvailable() const {
	Lock l(cs);
	uint64_t x = 0;
	for(auto i = users.begin(); i != users.end(); ++i) {
		x += i->second->getIdentity().getBytesShared();
	}
	return x;
}

OnlineUser& NmdcHub::getUser(const string& aNick) {
	{
		Lock l(cs);
		NickIter i = users.find(aNick);
		if(i != users.end())
			return *i->second;
	}

	UserPtr p;
	if(aNick == HUBSETTING(NICK)) {
		p = ClientManager::getInstance()->getMe();
	} else {
		p = ClientManager::getInstance()->getUser(aNick, getHubUrl());
	}
	OnlineUser* u = nullptr;
	{
		Lock l(cs);
		u = users.emplace(aNick, new OnlineUser(p, *this, 0)).first->second;
		u->getIdentity().setNick(aNick);
		if(u->getUser() == getMyIdentity().getUser()) {
			setMyIdentity(u->getIdentity());
		}
	}
	if(u->getUser()->getCID())
		ClientManager::getInstance()->putOnline(u);
	return *u;
}

void NmdcHub::supports(const StringList& feat) {
	string x;
	for(auto i = feat.begin(); i != feat.end(); ++i) {
		x+= *i + ' ';
	}
	send("$Supports " + x + '|');
}

OnlineUser* NmdcHub::findUser(const string& aNick) {
	Lock l(cs);
	NickIter i = users.find(aNick);
	return i == users.end() ? (nullptr) : i->second;
}

void NmdcHub::putUser(const string& aNick) {
	OnlineUser* ou = nullptr;
	{
		Lock l(cs);
		NickIter i = users.find(aNick);
		if(i == users.end())
			return;
		ou = i->second;
		users.erase(i);
	}
	ClientManager::getInstance()->putOffline(ou);
	delete ou;
}

void NmdcHub::clearUsers() {
	NickMap u2;
	stopMyInfoCheck();//BMDC/RSX++
	{
		Lock l(cs);
		u2.swap(users);
	}

	for(NickIter i = u2.begin(); i != u2.end(); ++i) {
		ClientManager::getInstance()->putOffline(i->second);
		delete i->second;
	}
	u2.clear();
}

void NmdcHub::updateFromTag(Identity& id, const string& tag) {
	StringTokenizer<string> tok(tag, ',');
	for(vector<string>::iterator i = tok.getTokens().begin(); i != tok.getTokens().end(); ++i) {
		if(i->length() < 2)
			continue;

		if(i->compare(0, 2, "H:") == 0) {
			StringTokenizer<string> t(i->substr(2), '/');
			if(t.getTokens().size() != 3)
				continue;
			id.set("HN", t.getTokens()[0]);
			id.set("HR", t.getTokens()[1]);
			id.set("HO", t.getTokens()[2]);
		} else if(i->compare(0, 2, "S:") == 0) {
			id.set("SL", i->substr(2));
		} else if(i->find("V:") != string::npos) {
			string::size_type j = i->find("V:");
			i->erase(i->begin() + j, i->begin() + j + 2);
			id.set("VE", *i);
		} else if(i->compare(0, 2, "M:") == 0) {
			if(i->size() == 3) {
				if((*i)[2] == 'A') {
					id.getUser()->unsetFlag(User::PASSIVE);
				}
				else {
					id.getUser()->setFlag(User::PASSIVE);
				}
			}
			if(i->size() == 4) {

				if((*i)[2] == 'A') {
					id.getUser()->unsetFlag(User::PASSIVE);
				}
				else {
					id.getUser()->setFlag(User::PASSIVE);
				}
			}
		}
	}
	/// @todo Think about this
	id.set("TA", '<' + tag + '>');
}

void NmdcHub::onLine(const string& aLine) noexcept {
	if(aLine.empty())
		return;
	
	dcdebug("%s",aLine.c_str());	

	if(aLine[0] != '$') {
		// Check if we're being banned...
		if(state != STATE_NORMAL) {
			if(Util::findSubString(aLine, "banned") != string::npos) {
				setAutoReconnect(false);
			}
		}
				
		string line = toUtf8(aLine);
		if(line[0] != '<') {
			fire(ClientListener::StatusMessage(), this, unescape(line));
			return;
		}
		string::size_type i = line.find('>', 2);
		if(i == string::npos) {
			fire(ClientListener::StatusMessage(), this, unescape(line));
			return;
		}
		
		string nick = line.substr(1, i-1);
		string message;
		if((line.length()-1) > i) {
			message = line.substr(i+2);
		} else {
			fire(ClientListener::StatusMessage(), this, unescape(line));
			return;
		}

		if((line.find("Hub-Security") != string::npos) && (line.find("was kicked by") != string::npos)) {
			fire(ClientListener::StatusMessage(), this, unescape(line), ClientListener::FLAG_IS_SPAM);
			return;
		} else if((line.find("is kicking") != string::npos) && (line.find("because:") != string::npos)) {
			fire(ClientListener::StatusMessage(), this, unescape(line), ClientListener::FLAG_IS_SPAM);
			return;
		}

		OnlineUser* from = findUser(nick);

		if(!from) {
			OnlineUser& o = getUser(nick);
			// Assume that messages from unknown users come from the hub
			o.getIdentity().setHub(true);
			o.getIdentity().setHidden(true);
			updated(o);
			from = &o;
		}
		COMMAND_DEBUG(aLine,TYPE_HUB,INCOMING,getHubUrl());
		auto chatMessage = unescape(message);
		fire(ClientListener::Message(), this, ChatMessage(chatMessage, from));
		return;
	}

	string cmd;
	string param = Util::emptyString;
	string::size_type x = 0;

	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		if( (x+1) != string::npos)
			param = /*toUtf8(*/aLine.substr(x+1)/*)*/;
	}

	if(cmd == "$Search") {
		if(state != STATE_NORMAL || getHideShare()) {
			return;
		}
		string::size_type i = 0;
		string::size_type j = param.find(' ', i);
		if(j == string::npos || i == j)
			return;

		string seeker = param.substr(i, j-i);

		// Filter own searches
		if(ClientManager::getInstance()->isActive(getHubUrl())) {
			if(seeker == localIp + ":" + Util::toString(SearchManager::getInstance()->getPort())) {
				return;
			}
		} else {
			// Hub:seeker
			if( (seeker.size() == 4) &&  (Util::stricmp(seeker.c_str() + 4, getMyNick().c_str()) == 0)) {
				return;
			}
		}

		i = j + 1;

		uint64_t tick = GET_TICK();
		clearFlooders(tick);

		// First, check if it's a flooder
		for(auto fi = flooders.begin(); fi != flooders.end(); ++fi) {
			if(fi->first == seeker) {
				return;
			}
		}

		seekers.push_back(make_pair(seeker, tick));

		int count = 0;
		for(auto fi = seekers.begin(); fi != seekers.end(); ++fi) {
			if(fi->first == seeker) {
				count++;
			}	

			if(count > 7) {
				
				if(seeker.compare(0, 4, "Hub:") == 0) {
					fire(ClientListener::SearchFlood(), this, seeker.substr(4));
				} else {
					fire(ClientListener::SearchFlood(), this, string(seeker+F_(" (Nick unknown)")));
				}	

				flooders.push_back(make_pair(seeker, tick));
				return;
			}
		}

		int a = -1;
		if(param[i] == 'F') {
			a = SearchManager::SIZE_DONTCARE;
		} else if(param[i+2] == 'F') {
			a = SearchManager::SIZE_ATLEAST;
		} else {
			a = SearchManager::SIZE_ATMOST;
		}
		i += 4;
		j = param.find('?', i);
		if(j == string::npos || i == j)
			return;
		string size = param.substr(i, j-i);
		i = j + 1;
		j = param.find('?', i);
		if(j == string::npos || i == j)
			return;
		int type = Util::toInt(param.substr(i, j-i)) - 1;
		i = j + 1;
		string terms = unescape(param.substr(i));

		if(!terms.empty()) {
			if((seeker.size() > 4) &&  seeker.compare(0, 4, "Hub:") == 0) {
				OnlineUser* u = findUser(seeker.substr(4));

				if(u == NULL) {
					return;
				}

				if(!u->getUser()->isSet(User::PASSIVE)) {
					u->getUser()->setFlag(User::PASSIVE);
					updated(*u);
				}
			}

			fire(ClientListener::NmdcSearch(), this, seeker, a, Util::toInt64(size), type, terms);
		}
	} else if(cmd == "$MyINFO") {
//		$MyINFO $ALL <nick> <description>$ $<connection><magic byte>$<email>$<share size>$|
		string::size_type i = 5, j;
		j = param.find(' ', i);
		if( (j == string::npos) || (j == i) )
			return;
		string nick = toUtf8(unescape(param.substr(i, j-i)));

		if(nick.empty())
			return;

		i = j + 1;

		OnlineUser& u = getUser(nick);

		// If he is already considered to be the hub (thus hidden), probably should appear in the UserList
		if(u.getIdentity().isHidden()) {
			u.getIdentity().setHidden(false);
			u.getIdentity().setHub(false);
		}

		j = param.find('$', i);
		if(j == string::npos)
			return;

		string tmpDesc = unescape(param.substr(i, j-i));
		// Look for a tag...
		if(!tmpDesc.empty() && tmpDesc[tmpDesc.size()-1] == '>') {
			x = tmpDesc.rfind('<');
			if(x != string::npos) {
				// Hm, we have something...disassemble it...
				updateFromTag(u.getIdentity(), tmpDesc.substr(x + 1, tmpDesc.length() - x - 2));
				tmpDesc.erase(x);
			}
		}
		u.getIdentity().setDescription(tmpDesc);

		i = j + 3;
		j = param.find('$', i);
		if(j == string::npos)
			return;

		string connection = param.substr(i, j-i-1);

		u.getIdentity().setBot(connection.empty()); // No connection = bot...
		u.getIdentity().setHub(false);

		u.getIdentity().set("CO", connection);
		unsigned char aMode = param[j-1];
				
		if( (aMode & 0x02) == 0x02 ) {//@ if(x & y) is wrong beter this variant
			u.getIdentity().set("AW", "1");
		}else
		{
			u.getIdentity().set("AW", Util::emptyString);
		}
		//ipv6
		if( (aMode & 0x80) == 0x80) {//same as above
			u.getIdentity().set("IX", "1");
		}	
		else
			u.getIdentity().set("IX",Util::emptyString);
		//end
		i = j + 1;
		j = param.find('$', i);

		if(j == string::npos)
			return;

		u.getIdentity().setEmail(toUtf8(unescape(param.substr(i, j-i))));

		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u.getIdentity().setBytesShared(param.substr(i, j-i));

		if(u.getUser() == getMyIdentity().getUser()) {
			setMyIdentity(u.getIdentity());
		} else {

            if(getCheckAtConnect())
            {
                string report = u.getIdentity().myInfoDetect(u);
                if(!report.empty()) {
                    updated(u);
                    cheatMessage(report);
                }
            }
		}
		fire(ClientListener::UserUpdated(), this, u);
	} else if(cmd == "$Quit") {
		if(!param.empty()) {
			const string& nick = toUtf8(param);
			OnlineUser* u = findUser(nick);
			if(!u)
				return;

			fire(ClientListener::UserRemoved(), this, *u);

			putUser(nick);
			return;
		}
	} else if(cmd == "$ConnectToMe") {
		//$ConnectToMe User [::1]:1234|
		//or
		//$ConnectToMe User 10.0.0.34:1234|
		// And also port 0 isnt valid
		dcdebug("%s",param.c_str());
		string::size_type i = param.find(' ');
		string::size_type j;
		if( (i == string::npos) || ((i + 1) >= param.size()) ) {
			return;
		}
		j = param.rfind(':');
		if(j == string::npos) {
			return;
		}
		uint16_t p_port = 0;
		string server = string();
		
		string p = param.substr(++i);
		size_t y = p.find('[');
		bool b_ip6 = false;
		if(!p.empty() &&  p[0] == '[') {//demangle ip6
			size_t x = p.rfind(']');
			if(x == string::npos) return;
			server = p.substr(y+1,x-1);	
			p_port = Util::toInt(p.substr(x+1));
			b_ip6 = true;
		}		
		
		dcdebug("%s %d",server.c_str(),p_port);
		
		string nick = param.substr(0,i-1);
		
		dcdebug("%s",nick.c_str());
		
		if(nick != getMyNick())
			return;
			
		dcdebug("Port %d",p_port);
		
		if(bIPv6 && !server.empty())
		{
			dcdebug("%s",server.c_str());
		}
		else 
		{
			ClientManager::parsePortIp(p,server,p_port);
			if(!getMyIdentity().isOp() && AVManager::getInstance()->isIpVirused(param.substr(i,j-i))){
				fire(ClientListener::StatusMessage(), this, unescape("This user "+param.substr(i,j-i)+" has the viruses in share!"), ClientListener::FLAG_VIRUS);
				return;
			}				
		}		

		ConnectionManager::getInstance()->nmdcConnect(server, p_port, getMyNick(), getHubUrl(), getEncoding());
		// For simplicity, we make the assumption that users on a hub have the same character encoding
	} else if(cmd == "$RevConnectToMe") {
		if(state != STATE_NORMAL) {
			return;
		}

		string::size_type j = param.find(' ');
		if(j == string::npos) {
			return;
		}

		OnlineUser* u = findUser(toUtf8(param.substr(0, j)));
		if(u == NULL)
			return;

		if(ClientManager::getInstance()->isActive(getHubUrl()) || ( isActiveV6() && u->getIdentity().isSet("IX")) ) {
			connectToMe(*u);
			if(u->getUser()->isSet(User::PASSIVE)) {
        			u->getUser()->setFlag(User::PASSIVE);
        			updated(*u);
             }
		} else {
			if(!u->getUser()->isSet(User::PASSIVE)) {
				u->getUser()->setFlag(User::PASSIVE);
				// Notify the user that we're passive too...
				revConnectToMe(*u);
				updated(*u);

				return;
			}
		}
	} else if(cmd == "$SR") {
		SearchManager::getInstance()->onSearchResult(aLine);
	} else if(cmd == "$HubName") {
		// If " - " found, the first part goes to hub name, rest to description
		// If no " - " found, first word goes to hub name, rest to description

		string::size_type i = param.find(" - ");
		if(i == string::npos) {
			i = param.find(' ');
			if(i == string::npos) {
				getHubIdentity().setNick(toUtf8(unescape(param)));
				getHubIdentity().setDescription(Util::emptyString);
			} else {
				getHubIdentity().setNick(toUtf8(unescape(param.substr(0, i))));
				getHubIdentity().setDescription(toUtf8(unescape(param.substr(i+1))));
			}
		} else {
			getHubIdentity().setNick(toUtf8(unescape(param.substr(0, i))));
			getHubIdentity().setDescription(toUtf8(unescape(param.substr(i+3))));
		}
		fire(ClientListener::HubUpdated(), this);
	} else if(cmd == "$Supports") {
		
		StringTokenizer<string> st(param, ' ');
		StringList& sl = st.getTokens();
		
		for(auto i = sl.begin(); i != sl.end(); ++i) {
			if(*i == "UserCommand") {
				supportFlags |= SUPPORTS_USERCOMMAND;
			} else if(*i == "NoGetINFO") {
				supportFlags |= SUPPORTS_NOGETINFO;
			} else if(*i == "UserIP2") {
				supportFlags |= SUPPORTS_USERIP2;
			} else if (*i == "IP64") {
				supportFlags |= SUPPORTS_IP64;				
			}
		}
	} else if(cmd == "$UserCommand") {
		string::size_type i = 0;
		string::size_type j = param.find(' ');
		if(j == string::npos)
			return;

		int type = Util::toInt(param.substr(0, j));
		i = j+1;
 		if(type == UserCommand::TYPE_SEPARATOR || type == UserCommand::TYPE_CLEAR) {
			int ctx = Util::toInt(param.substr(i));
			fire(ClientListener::HubUserCommand(), this, type, ctx, Util::emptyString, Util::emptyString);
		} else if(type == UserCommand::TYPE_RAW || type == UserCommand::TYPE_RAW_ONCE) {
			j = param.find(' ', i);
			if(j == string::npos)
				return;
			int ctx = Util::toInt(param.substr(i));
			i = j+1;
			j = param.find('$');
			if(j == string::npos)
				return;
			string name = unescape(param.substr(i, j-i));
			// NMDC uses '\' as a separator but both ADC and our internal representation use '/'
			Util::replace("/", "//", name);
			Util::replace("\\", "/", name);
			i = j+1;
			string command = unescape(param.substr(i, param.length() - i));
			fire(ClientListener::HubUserCommand(), this, type, ctx, name, command);
		}
	} else if(cmd == "$Lock") {
		if(state != STATE_PROTOCOL) {
			return;
		}
		if(aLine.size() < 5) return;
		
		state = STATE_IDENTIFY;

		// Param must not be toUtf8'd...
		param = aLine.substr(6);

		if(!param.empty()) {
			string::size_type j = param.find(" Pk=");
			string lock;
			if( j != string::npos ) {
				lock = param.substr(0, j);
			} else {
				// Workaround for faulty linux hubs...
				j = param.find(" ");
				if(j != string::npos)
					lock = param.substr(0, j);
				else
					lock = param;
			}

			if(CryptoManager::getInstance()->isExtended(lock)) {
				StringList feat;
				feat.push_back("UserCommand");
				feat.push_back("NoGetINFO");
				feat.push_back("NoHello");
				feat.push_back("UserIP2");
				feat.push_back("TTHSearch");
				feat.push_back("ZPipe0");
				feat.push_back("SaltPass");
				feat.push_back("IPv4");
				if(bIPv6) {
					//@ Add to $Support IP64 only when we had IPv6 connectivity
					feat.push_back("IP64");
				}
				supports(feat);
			}
			key(CryptoManager::getInstance()->makeKey(lock));
			OnlineUser& ou = getUser( HUBSETTING(NICK));
			validateNick(ou.getIdentity().getNick());

		}
	} else if(cmd == "$Hello") {
		if(!param.empty()) {
			OnlineUser& u = getUser(toUtf8(param));

			if(u.getUser() == getMyIdentity().getUser()) {
				if(ClientManager::getInstance()->isActive(getHubUrl()))
					u.getUser()->unsetFlag(User::PASSIVE);
				else
					u.getUser()->setFlag(User::PASSIVE);
			}

			if(state == STATE_IDENTIFY && u.getUser() == getMyIdentity().getUser()) {
				state = STATE_NORMAL;
				updateCounts(false);

				version();
				getNickList();
				myInfo(true);
			}

			fire(ClientListener::UserUpdated(), this, u);
		}
	} else if(cmd == "$ForceMove") {
		disconnect(false);
		fire(ClientListener::Redirect(), this, param);
	} else if(cmd == "$HubIsFull") {
		fire(ClientListener::HubFull(), this);
	} else if(cmd == "$ValidateDenide") {		// Mind the spelling...
		disconnect(false);
		fire(ClientListener::NickTaken(), this);
	} else if(cmd == "$UserIP") {
		if(!param.empty()) {
			OnlineUserList v;
			StringTokenizer<string> t(param, "$$");
			StringList& l = t.getTokens();
			for(auto it = l.begin(); it != l.end(); ++it) {
				string::size_type j = 0;
				if((j = it->find(' ')) == string::npos)
					continue;
				if((j+1) == it->length())
					continue;

				OnlineUser* u = findUser(toUtf8(it->substr(0, j)));

				if(!u)
					continue;

				u->getIdentity().setIp4(it->substr(j+1));
				if(u->getUser() == getMyIdentity().getUser()) {
					setMyIdentity(u->getIdentity());
					refreshLocalIp();
				}
				v.push_back(u);
			}

			fire(ClientListener::UsersUpdated(), this, v);
		}
	} else if(cmd == "$NickList") {
		if(!param.empty()) {
			OnlineUserList v;
			StringTokenizer<string> t(param, "$$");
			StringList& sl = t.getTokens();

			for(auto it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;

				v.push_back(&getUser(toUtf8(*it)));
			}

			if(!( (supportFlags & SUPPORTS_NOGETINFO) == SUPPORTS_NOGETINFO) ) {
				string tmp;
				// Let's assume 10 characters per nick...
				tmp.reserve(v.size() * (11 + 10 + getMyNick().length()));
				string n = ' ' + fromUtf8(getMyNick()) + '|';
				for(auto i = v.begin(); i != v.end(); ++i) {
					tmp += "$GetINFO ";
					tmp += fromUtf8((*i)->getIdentity().getNick());
					tmp += n;
				}
				if(!tmp.empty()) {
					send(tmp);
				}
			}

			fire(ClientListener::UsersUpdated(), this, v);
		}
	} else if(cmd == "$OpList") {
		if(!param.empty()) {
			OnlineUserList v;
			StringTokenizer<string> t(param, "$$");
			StringList& sl = t.getTokens();
			for(auto it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				OnlineUser& ou = getUser(toUtf8(*it));
				ou.getIdentity().setOp(true);
				if(ou.getUser() == getMyIdentity().getUser()) {
					setMyIdentity(ou.getIdentity());
				}
				v.push_back(&ou);
			}

			fire(ClientListener::UsersUpdated(), this, v);
			updateCounts(false);

			// Special...to avoid op's complaining that their count is not correctly
			// updated when they log in (they'll be counted as registered first...)
			myInfo(false);
		}
	} else if(cmd == "$To:") {
		string::size_type i = param.find("From:");
		if(i == string::npos)
			return;

		i+=6;
		string::size_type j = param.find('$', i);
		if(j == string::npos)
			return;

		string rtNick = toUtf8(param.substr(i, j - 1 - i));
		if(rtNick.empty())
			return;
		i = j + 1;

		if(param.size() < i + 3 || param[i] != '<')
			return;

		j = param.find('>', i);
		if(j == string::npos)
			return;

		string fromNick = toUtf8(param.substr(i+1, j-i-1));
		if(fromNick.empty())
			return;

		if(param.size() < j + 2) {
			return;
		}

		auto from = findUser(fromNick);
		auto replyTo = findUser(rtNick);

		//if(!replyTo || !from) {
			if(!replyTo) {
				// Assume it's from the hub
				OnlineUser& ou = getUser(rtNick);
				ou.getIdentity().setHub(true);
				ou.getIdentity().setHidden(true);
				updated(ou);
				replyTo = &ou;
			}
			if(!from) {
				// Assume it's from the hub
				OnlineUser& ou = getUser(fromNick);
				ou.getIdentity().setHub(true);
				ou.getIdentity().setHidden(true);
				updated(ou);
				from = &ou;
			}
		//}

		auto chatMessage = unescape(toUtf8(param.substr(j + 2)));
		fire(ClientListener::Message(), this, ChatMessage(chatMessage, from, &getUser(getMyNick()), replyTo));
	} else if(cmd == "$GetPass") {
		OnlineUser& ou = getUser(getMyNick());
		ou.getIdentity().set("RG", "1");
		setMyIdentity(ou.getIdentity());
		if(!param.empty()) {
			salt = param;
		}
		fire(ClientListener::GetPassword(), this);
	} else if(cmd == "$BadPass") {
		setPassword(Util::emptyString);
	} else if(cmd == "$HubTopic") {
        string line = aLine;
        line.replace(0,9,Util::emptyString);
        fire(ClientListener::HubTopic(), this,toUtf8(unescape(line)) );
	} else if(cmd == "$ZOn") {
		try {
			sock->setMode(BufferedSocket::MODE_ZPIPE);
		} catch (const Exception& e) {
			dcdebug("NmdcHub::onLine %s failed with error: %s\n", cmd.c_str(), e.getError().c_str());
		}
	} else if (cmd == "$LogedIn") {
		//we dont interested in it...
	} else
	{
		dcassert(cmd[0] == '$');
		dcdebug("NmdcHub::onLine Unknown command %s\n", aLine.c_str());
	}
}

void NmdcHub::checkNick(string& nick) {
	
	for(size_t i = 0, n = nick.size(); i < n; ++i) {
          if(static_cast<uint8_t>(nick[i]) <= 32 || nick[i] == '|' || nick[i] == '$' || nick[i] == '<' || nick[i] == '>') {
                nick[i] = '_';
		}
	}
}

void NmdcHub::connectToMe(const OnlineUser& aUser) {
	checkstate();
	Lock l(cs);
	dcdebug("NmdcHub::connectToMe %s\n", aUser.getIdentity().getNick().c_str());
	string nick = fromUtf8(aUser.getIdentity().getNick());
	ConnectionManager::getInstance()->nmdcExpect(nick, getMyNick(), getHubUrl());
	
	if(!getMyIdentity().isOp() && AVManager::getInstance()->isNickVirused(aUser.getIdentity().getNick())){
			fire(ClientListener::StatusMessage(), this, unescape("This user "+aUser.getIdentity().getNick()+" possible had the viruses in share!"), ClientListener::FLAG_VIRUS);
			return;
	}	
	
	bool isOkIp6 = !aUser.getIdentity().get("IX").empty();
	//bool isOkIp6 = !aUser.getIdentity().getIp6().empty();
	dcdebug("%d - %d - %d - %d",(int)bIPv6,isActiveV6(),(int)((supportFlags & SUPPORTS_IP64) == SUPPORTS_IP64 ),isOkIp6);
	
	uint16_t cport = ConnectionManager::getInstance()->getPort();
	//we have IPv6, Hub Support that and User too
	if(bIPv6 && ((supportFlags & SUPPORTS_IP64) == SUPPORTS_IP64 ) && isOkIp6) {
		send("$ConnectToMe " + nick + " [" + getUserIp6() + "]:" + Util::toString(cport) + "|");
		dcdebug("\n%s",getUserIp6().c_str());
	} else
		send("$ConnectToMe " + nick + " " + localIp + ":" + Util::toString(cport) + "|");
}

void NmdcHub::revConnectToMe(const OnlineUser& aUser) {
	checkstate();
	if(!getMyIdentity().isOp() && AVManager::getInstance()->isNickVirused(aUser.getIdentity().getNick())){
		fire(ClientListener::StatusMessage(), this, unescape("This user"+aUser.getIdentity().getNick()+" has the viruses in share!"), ClientListener::FLAG_VIRUS);
		return;
	}	
	dcdebug("NmdcHub::revConnectToMe %s\n", aUser.getIdentity().getNick().c_str());
	send("$RevConnectToMe " + fromUtf8(getMyNick()) + " " + fromUtf8(aUser.getIdentity().getNick()) + "|");
}

void NmdcHub::hubMessage(const string& aMessage, bool thirdPerson) {
	checkstate();
	send(fromUtf8( "<" + getMyNick() + "> " + escape(thirdPerson ? "/me " + aMessage : aMessage) + "|" ) );
}

void NmdcHub::myInfo(bool alwaysSend) {
	checkstate();

	reloadSettings(false);

	string tmp1 = ";**\x1fU9";
	string tmp2 = "+L9";
	string tmp3 = "+G9";
	string tmp4 = "+R9";
	string tmp5 = "+N9";
	string::size_type i;

	for(i = 0; i < 6; i++) {
		tmp1[i]++;
	}
	for(i = 0; i < 3; i++) {
		tmp2[i]++; tmp3[i]++; tmp4[i]++; tmp5[i]++;
	}
	char modeChar[3];
	modeChar[0] = '?';
	modeChar[1] = '\0';
	//TODO per fav?
	if(CONNSETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5)
		modeChar[0] = '5';
	else if(ClientManager::getInstance()->isActive(getHubUrl())) {
		modeChar[0] = 'A';
		if(bIPv6)
			modeChar[1] = 'A';
	} else {
		modeChar[0] = 'P';
		if(bIPv6)
			modeChar[1] = 'P';
	}
	modeChar[2] =  '\0';
	
	
	string uploadSpeed;
	int upLimit = ThrottleManager::getInstance()->getUpLimit();
	if (upLimit > 0) {
		uploadSpeed = Util::toString(upLimit) + " KiB/s";
	} else {
		uploadSpeed = SETTING(UPLOAD_SPEED);
	}

	bool bFreeSlot = SETTING(SHOW_FREE_SLOTS_DESC);
	string sFreeSlot = "[" + Util::toString(UploadManager::getInstance()->getFreeSlots()) + "]";
	//away status
    char staFlag = Util::getAway() ? '\x02' : '\x01';
		
	string uMin = (SETTING(MIN_UPLOAD_SPEED) == 0) ? Util::emptyString : tmp5 + Util::toString(SETTING(MIN_UPLOAD_SPEED));
	
	string myInfoA =
		"$MyINFO $ALL " + fromUtf8(HUBSETTING(NICK)) + " " + fromUtf8(escape(bFreeSlot ? sFreeSlot + HUBSETTING(DESCRIPTION) : HUBSETTING(DESCRIPTION))) +
		"<"+APPNAME+" V:" + VERSIONSTRING + tmp2 + modeChar + tmp3 + getCounts();
		
	string myInfoB = tmp4 + Util::toString(SETTING(SLOTS));
	string myInfoC = uMin +
		">$ $" + uploadSpeed + staFlag + '$' + fromUtf8(escape(HUBSETTING(EMAIL))) + '$';
	ShareManager* sm = getShareManager();
	string share = getHideShare() ? "0" : sm->getShareSizeString(); //@Custom/Hide share NMDC
	string myInfoD = share + "$|";
	// we always send A and C; however, B (slots) and D (share size) can frequently change so we delay them if needed
 	if(lastMyInfoA != myInfoA || lastMyInfoC != myInfoC ||
		alwaysSend || ((lastMyInfoB != myInfoB || lastMyInfoD != myInfoD) && lastUpdate + 15*60*1000 < GET_TICK())) {
 		dcdebug("MyInfo %s...\n", getMyNick().c_str());
 		send(myInfoA + myInfoB + myInfoC + myInfoD);
 		lastMyInfoA = myInfoA;
 		lastMyInfoB = myInfoB;
		lastMyInfoC = myInfoC;
		lastMyInfoD = myInfoD;
 		lastUpdate = GET_TICK();
	}
}

void NmdcHub::search(int aSizeType, int64_t aSize, int aFileType, const string& aString, const string&, const StringList&) {
	checkstate();
	char c1 = (aSizeType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
	char c2 = (aSizeType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';
	string tmp = ((aFileType == SearchManager::TYPE_TTH) ? "TTH:" + aString : fromUtf8(escape(aString)));
	string::size_type i;
	while((i = tmp.find(' ')) != string::npos) {
		tmp[i] = '$';
	}
	string tmp2;
	if(ClientManager::getInstance()->isActive(getHubUrl())) {
		tmp2 = localIp + ':' + Util::toString(SearchManager::getInstance()->getPort());
	} else {
		tmp2 = "Hub:" + fromUtf8(getMyNick());
	}
	send("$Search " + tmp2 + ' ' + c1 + '?' + c2 + '?' + Util::toString(aSize) + '?' + Util::toString(aFileType+1) + '?' + tmp + '|');
}

string NmdcHub::validateMessage(string tmp, bool reverse) {
	string::size_type i = 0;

	if(reverse) {
		while( (i = tmp.find("&#36;", i)) != string::npos) {
			tmp.replace(i, 5, "$");
			i++;
		}
		i = 0;
		while( (i = tmp.find("&#124;", i)) != string::npos) {
			tmp.replace(i, 6, "|");
			i++;
		}
		i = 0;
		while( (i = tmp.find("&amp;", i)) != string::npos) {
			tmp.replace(i, 5, "&");
			i++;
		}
	} else {
		i = 0;
		while( (i = tmp.find("&amp;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find("&#36;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find("&#124;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find('$', i)) != string::npos) {
			tmp.replace(i, 1, "&#36;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find('|', i)) != string::npos) {
			tmp.replace(i, 1, "&#124;");
			i += 5;
		}
	}
	return tmp;
}

void NmdcHub::privateMessage(const string& nick, const string& message) {
	send("$To: " + fromUtf8(nick) + " From: " + fromUtf8(getMyNick()) + " $" + fromUtf8(escape("<" + getMyNick() + "> " + message)) + "|");
}

void NmdcHub::privateMessage(const OnlineUser& aUser, const string& aMessage, bool /*thirdPerson*/) {
	checkstate();
	privateMessage(aUser.getIdentity().getNick(), aMessage);
	// Emulate a returning message...
	Lock l(cs);
	OnlineUser* ou = findUser(getMyNick());
	if(ou) {
		ChatMessage message = { aMessage, ou, &aUser, ou };
		fire(ClientListener::Message(), this, message);
	}
}

void NmdcHub::sendUserCmd(const UserCommand& command, const ParamMap& params) {
	checkstate();
	string cmd = Util::formatParams(command.getCommand(), params, escape);
	if(command.isChat()) {
		if(command.getTo().empty()) {
			hubMessage(cmd);
		} else {
			privateMessage(command.getTo(), cmd);
		}
	} else {
		send(fromUtf8(cmd));
	}
}
// 5*60*1000 ms is 5 Minutes?
void NmdcHub::clearFlooders(uint64_t aTick) {
	while(!seekers.empty() && ( (seekers.front().second + (5 * 60 * 1000)) < aTick)) {
		seekers.pop_front();
	}

	while(!flooders.empty() && ((flooders.front().second + (120 * 60 * 1000)) < aTick)) {
		flooders.pop_front();
	}
}

void NmdcHub::refreshLocalIp() noexcept {
	if((!CONNSETTING(NO_IP_OVERRIDE) || getUserIp().empty()) && !getMyIdentity().getIp().empty()) {
		// Best case - the server detected it
		localIp = getMyIdentity().getIp();
	} else {
		localIp.clear();
	}
	if(localIp.empty()) {
		localIp = getUserIp();
		if(!localIp.empty()) {
			localIp = Socket::resolve(localIp);
		}
		if(localIp.empty()) {
			localIp = sock->getLocalIp();
			if(localIp.empty()) {
				localIp = Util::getLocalIp();
			}
		}
	}
}

void NmdcHub::on(Connected) noexcept {
	Client::on(Connected());

	if(state != STATE_PROTOCOL) {
		return;
	}

	supportFlags = 0;
	lastMyInfoA.clear();
	lastMyInfoB.clear();
	lastMyInfoC.clear();
	lastMyInfoD.clear();
	lastUpdate = 0;
	refreshLocalIp();
}

void NmdcHub::on(Line, const string& aLine) noexcept {
	Client::on(Line(), aLine);
	onLine(aLine);
}

void NmdcHub::on(Failed, const string& aLine) noexcept {
	clearUsers();
	Client::on(Failed(), aLine);
}

void NmdcHub::on(Second, uint64_t aTick) noexcept {
	Client::on(Second(), aTick);
}

void NmdcHub::on(Minute, uint64_t aTick) noexcept {
	refreshLocalIp();
	
	if( (state == STATE_NORMAL) && (aTick > (getLastActivity() + 120*1000)) ) {
		send("|", 1);
	}
}

void NmdcHub::password(const string& aPass) {
	if(!salt.empty()) {
		//$SaltPass in $Support
		string filteredPass = fromUtf8(aPass);
		size_t saltBytes = salt.size() * 5 / 8;
		uint8_t *buf = new uint8_t[saltBytes];
		Encoder::fromBase32(salt.c_str(), buf, saltBytes);
		TigerHash th;
		th.update(filteredPass.data(), filteredPass.length());
		th.update(buf, saltBytes);
		send("$MyPass " + Encoder::toBase32(th.finalize(), TigerHash::BYTES) + "|");
		salt.clear();
		delete [] buf;
	} else {//end
		send("$MyPass " + fromUtf8(aPass) + "|");
	}
}

//Refresh UL
void NmdcHub::refreshuserlist() {
		Lock l(cs);

		OnlineUserList v;
		for(NickIter i = users.begin(); i != users.end(); ++i) {
			v.push_back(i->second);
		}
		fire(ClientListener::UsersUpdated(), this, v);
}

} // namespace dcpp
