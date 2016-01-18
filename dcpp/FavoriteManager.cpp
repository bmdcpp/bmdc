/*
 * Copyright (C) 2001-2016 Jacek Sieka, arnetheduck on gmail point com
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
#include "FavoriteManager.h"
#include "Pointer.h"
#include "ClientManager.h"
#include "CryptoManager.h"
//[BMDC
#include "RawManager.h"
//]
#include "HttpConnection.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "File.h"
#include "BZUtils.h"
#include "FilteredFile.h"

namespace dcpp {

using std::make_pair;
using std::swap;

//RSX++//BMDC++
FavoriteHubEntry::FavAction::FavAction(bool _enabled, string _raw /*= Util::emptyString*/, int64_t id /*=0*/) noexcept: enabled(_enabled) {
	if(_raw.empty()) return;
	StringTokenizer<string> tok(_raw, ',');
	const Action* a = RawManager::getInstance()->findAction(id);
	if(a != NULL) {
		for(auto j = tok.getTokens().begin(); j != tok.getTokens().end(); ++j) {
			int rId = Util::toInt(*j);
			for(auto i = a->raw.begin(); i != a->raw.end(); ++i) {
				if(rId == i->getId()) {
					raws.push_back(rId);
					break;
				}
			}
		}
	}
}
//END

FavoriteManager::FavoriteManager() : lastId(0), useHttp(false), running(false), c(NULL), lastServer(0), listType(TYPE_NORMAL), dontSave(false) {
	SettingsManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);

	File::ensureDirectory(Util::getHubListsPath());

	/* after the release of the first version including this blacklist (DC++ 0.780), remember to
	also update version.xml when a domain has to be added to this list, using the following format:
	<Blacklist>
		<Blacklisted Domain="example1.com" Reason="Domain used for spam purposes."/>
		<Blacklisted Domain="example2.com" Reason="Domain used for spam purposes."/>
	</Blacklist>
	(the "Blacklist" tag should be under the main "DCUpdate" tag.) */
	addBlacklist("adchublist.com", "Domain used for spam purposes.");
	addBlacklist("hublist.org", "Domain used for spam purposes.");
	addBlacklist("hubtracker.com", "Domain lost to unknown owners advertising dubious pharmaceuticals.");
	addBlacklist("openhublist.org", "Domain used for spam purposes.");
}

FavoriteManager::~FavoriteManager() {
	ClientManager::getInstance()->removeListener(this);
	SettingsManager::getInstance()->removeListener(this);
	if(c) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	for_each(favoriteHubs.begin(), favoriteHubs.end(), DeleteFunction());
	for_each(recentHubs.begin(), recentHubs.end(), DeleteFunction());
}

UserCommand FavoriteManager::addUserCommand(int type, int ctx, int flags, const string& name, const string& command, const string& to, const string& hub) {
	// No dupes, add it...
	Lock l(cs);
	userCommands.push_back(UserCommand(lastId++, type, ctx, flags, name, command, to, hub));
	UserCommand& uc = userCommands.back();
	if(!uc.isSet(UserCommand::FLAG_NOSAVE))
		save();
	return userCommands.back();
}

bool FavoriteManager::getUserCommand(int cid, UserCommand& uc) {
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			uc = *i;
			return true;
		}
	}
	return false;
}

bool FavoriteManager::moveUserCommand(int cid, int pos) {
	dcassert(pos == -1 || pos == 1);
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

void FavoriteManager::updateUserCommand(const UserCommand& uc) {
	bool nosave = true;
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == uc.getId()) {
			*i = uc;
			nosave = uc.isSet(UserCommand::FLAG_NOSAVE);
			break;
		}
	}
	if(!nosave)
		save();
}

int FavoriteManager::findUserCommand(const string& aName, const string& aUrl) {
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getName() == aName && i->getHub() == aUrl) {
			return i->getId();
		}
	}
	return -1;
}

void FavoriteManager::removeUserCommand(int cid) {
	bool nosave = true;
	Lock l(cs);
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			nosave = i->isSet(UserCommand::FLAG_NOSAVE);
			userCommands.erase(i);
			break;
		}
	}
	if(!nosave) {
		l.unlock();
		save();
	}
}
void FavoriteManager::removeUserCommand(const string& srv) {
	Lock l(cs);
	std::remove_if(userCommands.begin(), userCommands.end(), [&](const UserCommand& uc) -> bool {
                 return uc.getHub() == srv && uc.isSet(UserCommand::FLAG_NOSAVE);
        });
}

void FavoriteManager::removeHubUserCommands(int ctx, const string& hub) {
	Lock l(cs);
	std::remove_if(userCommands.begin(), userCommands.end(), [&](const UserCommand& uc) -> bool {
                 return uc.getHub() == hub && uc.isSet(UserCommand::FLAG_NOSAVE) && uc.getCtx() == ctx;
        });
}

void FavoriteManager::addFavoriteUser(const UserPtr& aUser) {
	{
		Lock l(cs);
		if(users.find(aUser->getCID()) == users.end()) {
			StringList urls = ClientManager::getInstance()->getHubs(aUser->getCID(), Util::emptyString);
			StringList nicks = ClientManager::getInstance()->getNicks(aUser->getCID(), Util::emptyString);

			/// @todo make this an error probably...
			if(urls.empty())
				urls.push_back(Util::emptyString);
			if(nicks.empty())
				nicks.push_back(Util::emptyString);

			FavoriteMap::iterator i = users.insert(make_pair(aUser->getCID(), FavoriteUser(aUser, nicks[0], urls[0]))).first;
			fire(FavoriteManagerListener::UserAdded(), i->second);
			save();
		}
	}
}

void FavoriteManager::removeFavoriteUser(const UserPtr& aUser) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i != users.end()) {
		fire(FavoriteManagerListener::UserRemoved(), i->second);
		users.erase(i);
		save();
	}
}

string FavoriteManager::getUserURL(const UserPtr& aUser) const {
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i != users.end()) {
		const FavoriteUser& fu = i->second;
		return fu.getUrl();
	}
	return Util::emptyString;
}

void FavoriteManager::addFavorite(const FavoriteHubEntry& aEntry) {
	FavoriteHubEntry* f = nullptr;

	FavoriteHubEntryList::iterator i = getFavoriteHub(aEntry.getServer());
	if(i != favoriteHubs.end()) {
		return;
	}
	f = new FavoriteHubEntry(aEntry);
	favoriteHubs.push_back(f);
	fire(FavoriteManagerListener::FavoriteAdded(), f);
	save();
}

void FavoriteManager::removeFavorite(FavoriteHubEntry* entry) {
	FavoriteHubEntryList::iterator i = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(i == favoriteHubs.end()) {
		return;
	}

	fire(FavoriteManagerListener::FavoriteRemoved(), entry);
	favoriteHubs.erase(i);
	delete entry;
	save();
}

bool FavoriteManager::isFavoriteHub(const std::string& url) {
	FavoriteHubEntryList::iterator i = getFavoriteHub(url);
	if(i != favoriteHubs.end()) {
		return true;
	}
	return false;
}

bool FavoriteManager::addFavoriteDir(const string& aDirectory, const string & aName){
	string path = aDirectory;

	if( path[ path.length() -1 ] != PATH_SEPARATOR )
		path += PATH_SEPARATOR;

	for(StringPairIter i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i) {
		if((Util::strnicmp(path, i->first, i->first.length()) == 0) && (Util::strnicmp(path, i->first, path.length()) == 0)) {
			return false;
		}
		if(Util::stricmp(aName, i->second) == 0) {
			return false;
		}
	}
	favoriteDirs.push_back(make_pair(aDirectory, aName));
	save();
	return true;
}

bool FavoriteManager::removeFavoriteDir(const string& aName) {
	string d(aName);

	if(d[d.length() - 1] != PATH_SEPARATOR)
		d += PATH_SEPARATOR;

	for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
		if(Util::stricmp(j->first.c_str(), d.c_str()) == 0) {
			favoriteDirs.erase(j);
			save();
			return true;
		}
	}
	return false;
}

bool FavoriteManager::renameFavoriteDir(const string& aName, const string& anotherName) {

	for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
		if(Util::stricmp(j->second.c_str(), aName.c_str()) == 0) {
			j->second = anotherName;
			save();
			return true;
		}
	}
	return false;
}
//[Recent Hubs
void FavoriteManager::addRecent(const RecentHubEntry& aEntry) {
	RecentHubEntry::Iter i = getRecentHub(aEntry.getServer());
	if(i != recentHubs.end()) {
		return;
	}
	RecentHubEntry* f = new RecentHubEntry(aEntry);
	recentHubs.push_back(f);
	fire(FavoriteManagerListener::RecentAdded(), f);
	recentsave();
}

void FavoriteManager::removeRecent(const RecentHubEntry* entry) {
	RecentHubEntry::List::iterator i = find(recentHubs.begin(), recentHubs.end(), entry);
	if(i == recentHubs.end()) {
		return;
	}

	fire(FavoriteManagerListener::RecentRemoved(), entry);
	recentHubs.erase(i);
	delete entry;
	recentsave();
}

void FavoriteManager::updateRecent(const RecentHubEntry* entry) {
	RecentHubEntry::Iter i = find(recentHubs.begin(), recentHubs.end(), entry);
	if(i == recentHubs.end()) {
		return;
	}

	fire(FavoriteManagerListener::RecentUpdated(), entry);
	recentsave();
}

RecentHubEntry::Iter FavoriteManager::getRecentHub(const string& aServer) const {
	for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
		if(Util::stricmp((*i)->getServer(), aServer) == 0) {
			return i;
		}
	}
	return recentHubs.end();
}
//end
class XmlListLoader : public SimpleXMLReader::CallBack {
public:
	XmlListLoader(HubEntryList& lst) : publicHubs(lst) { }
	virtual ~XmlListLoader() { }
	virtual void startTag(const string& name, StringPairList& attribs, bool) {
		if(name == "Hub") {
			const string& name = getAttrib(attribs, "Name", 0);
			const string& server = getAttrib(attribs, "Address", 1);
			const string& description = getAttrib(attribs, "Description", 2);
			const string& users = getAttrib(attribs, "Users", 3);
			const string& country = getAttrib(attribs, "Country", 4);
			const string& shared = getAttrib(attribs, "Shared", 5);
			const string& minShare = getAttrib(attribs, "Minshare", 5);
			const string& minSlots = getAttrib(attribs, "Minslots", 5);
			const string& maxHubs = getAttrib(attribs, "Maxhubs", 5);
			const string& maxUsers = getAttrib(attribs, "Maxusers", 5);
			const string& reliability = getAttrib(attribs, "Reliability", 5);
			const string& rating = getAttrib(attribs, "Rating", 5);
			publicHubs.push_back(HubEntry(name, server, description, users, country, shared, minShare, minSlots, maxHubs, maxUsers, reliability, rating));
		}
	}
	virtual void endTag(const string&) {

	}
private:
	//made clang happy
	using SimpleXMLReader::CallBack::endTag;
	HubEntryList& publicHubs;
};

bool FavoriteManager::onHttpFinished(bool fromHttp) noexcept {
	MemoryInputStream mis(downloadBuf);
	bool success = true;

	Lock l(cs);
	HubEntryList& list = publicListMatrix[publicListServer];
	list.clear();

	try {
		XmlListLoader loader(list);

		if((listType == TYPE_BZIP2) && (!downloadBuf.empty())) {
			FilteredInputStream<UnBZFilter, false> f(&mis);
			SimpleXMLReader(&loader).parse(f);
		} else {
			SimpleXMLReader(&loader).parse(mis);
		}
	} catch(const Exception&) {
		success = false;
		fire(FavoriteManagerListener::Corrupted(), fromHttp ? publicListServer : Util::emptyString);
	}

	if(fromHttp) {
		try {
			File f(Util::getHubListsPath() + Util::validateFileName(publicListServer), File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(downloadBuf);
			f.close();
		} catch(const FileException&) { }
	}

	downloadBuf = Util::emptyString;

	return success;
}

void FavoriteManager::save() {
	if(dontSave)
		return;

	Lock l(cs);
	try {
		SimpleXML xml;

		xml.addTag("Favorites");
		xml.stepIn();

		xml.addTag("Hubs");
		xml.stepIn();

		for(FavHubGroups::const_iterator i = favHubGroups.begin(), iend = favHubGroups.end(); i != iend; ++i) {
			xml.addTag("Group");
			xml.addChildAttrib("Name", i->first);
			i->second.save(xml);
		}

		for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i) {
			xml.addTag("Hub");
			xml.addChildAttrib("Name", (*i)->getName());
			xml.addChildAttrib("Description", (*i)->getHubDescription());
			xml.addChildAttrib("Password", (*i)->getPassword());
			xml.addChildAttrib("Server", (*i)->getServer());
			xml.addChildAttrib("Encoding", (*i)->getEncoding());
			//BMDC++
			xml.addChildAttrib("HideShare", (*i)->getHideShare());
			//BMDC++
			xml.addChildAttrib("Group", (*i)->getGroup());
			//BMDC++
			xml.addChildAttrib("Mode", (*i)->getMode());
			xml.addChildAttrib("ChatExtraInfo", (*i)->getChatExtraInfo());
			xml.addChildAttrib("onConnect", (*i)->getCheckAtConn());
			xml.addChildAttrib("CheckClients", (*i)->getCheckClients());
			xml.addChildAttrib("CheckFilelists", (*i)->getCheckFilelists());
			xml.addChildAttrib("Protects", (*i)->getProtectUsers());
			xml.addChildAttrib("UserListToggle", (*i)->getShowUserList());
			xml.addChildAttrib("NotifyToggle", (*i)->getNotify());
			xml.addChildAttrib("Order",(*i)->getHubOrder());
			xml.addChildAttrib("Visible",(*i)->getHubVisible());
			xml.addChildAttrib("Width",(*i)->getHubWidth());
			xml.addChildAttrib("Private", (*i)->getPrivate());

			(*i)->save(xml);
			//RSX++
			xml.stepIn();
			for(auto a = (*i)->action.begin(); a != (*i)->action.end(); ++a) {
				if(RawManager::getInstance()->getValidAction(a->first)) {
					string raw = Util::emptyString;
					for(auto j = a->second->raws.begin(); j != a->second->raws.end(); ++j) {
						if(!raw.empty())
							raw += ",";
						raw += Util::toString(*j);
					}
					if(!raw.empty() || a->second->getEnabled()) {
						xml.addTag("Action");
						xml.addChildAttrib("ID", a->first);
						xml.addChildAttrib("Active", Util::toString(a->second->getEnabled()));
						xml.addChildAttrib("Raw", raw);
					}
				}
			}
			xml.stepOut();
		}

		xml.stepOut();

		xml.addTag("Users");
		xml.stepIn();
		for(FavoriteMap::const_iterator i = users.begin(), iend = users.end(); i != iend; ++i) {
			xml.addTag("User");
			xml.addChildAttrib("LastSeen", i->second.getLastSeen());
			xml.addChildAttrib("GrantSlot", i->second.isSet(FavoriteUser::FLAG_GRANTSLOT));
			xml.addChildAttrib("Ignore", i->second.isSet(FavoriteUser::FLAG_IGNORE));
			xml.addChildAttrib("UserDescription", i->second.getDescription());
			xml.addChildAttrib("Nick", i->second.getNick());
			xml.addChildAttrib("URL", i->second.getUrl());
			xml.addChildAttrib("CID", i->first.toBase32());
			xml.addChildAttrib("HistoryNicks", i->second.getNicks());
		}

		xml.stepOut();
		xml.addTag("UserCommands");
		xml.stepIn();
		for(UserCommand::List::const_iterator i = userCommands.begin(), iend = userCommands.end(); i != iend; ++i) {
			if(!i->isSet(UserCommand::FLAG_NOSAVE)) {
				xml.addTag("UserCommand");
				xml.addChildAttrib("Type", i->getType());
				xml.addChildAttrib("Context", i->getCtx());
				xml.addChildAttrib("Name", i->getName());
				xml.addChildAttrib("Command", i->getCommand());
				xml.addChildAttrib("To", i->getTo());
				xml.addChildAttrib("Hub", i->getHub());
			}
		}
		xml.stepOut();

		//Favorite download to dirs
		xml.addTag("FavoriteDirs");
		xml.stepIn();
		StringPairList spl = getFavoriteDirs();
		for(StringPairIter i = spl.begin(), iend = spl.end(); i != iend; ++i) {
			xml.addTag("Directory", i->first);
			xml.addChildAttrib("Name", i->second);
		}
		xml.stepOut();

		xml.addTag("FavoriteIUsers");
		xml.stepIn();
		for(auto it = favoritesNoCid.begin();it!= favoritesNoCid.end();++it)
		{
			xml.addTag("FavoriteIUser");
			xml.addChildAttrib("Nick",it->first);
			xml.addChildAttrib("LastSeen",it->second->getLastSeen());
			xml.addChildAttrib("Description",it->second->getDescription());
			xml.addChildAttrib("GrantSlot", it->second->isSet(FavoriteUser::FLAG_GRANTSLOT));
		}
		
		xml.stepOut();
		xml.addTag("FavoriteIpAddress");
		xml.stepIn();
		for(auto it:ips)
		{
			xml.addTag("FavoriteIP");
			xml.addChildAttrib("IP",it.second->getIp());
			xml.addChildAttrib("LastSeen",it.second->getLastSeen());
			xml.addChildAttrib("Range",it.second->isSet(FavoriteUser::FLAG_IP_RANGE) );
			
		}
		xml.stepOut();

		xml.stepOut();

		string fname = getConfigFile();

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("FavoriteManager::save: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::recentsave() {
	try {
		SimpleXML xml;

		xml.addTag("Recents");
		xml.stepIn();

		xml.addTag("Hubs");
		xml.stepIn();

		for(RecentHubEntry::Iter i = recentHubs.begin(); i != recentHubs.end(); ++i) {
			xml.addTag("Hub");
			xml.addChildAttrib("Name", (*i)->getName());
			xml.addChildAttrib("Description", (*i)->getDescription());
			xml.addChildAttrib("Users", (*i)->getUsers());
			xml.addChildAttrib("Shared", (*i)->getShared());
			xml.addChildAttrib("Server", (*i)->getServer());
		}

		xml.stepOut();
		xml.stepOut();

		string fname = Util::getPath(Util::PATH_USER_CONFIG) + "Recents.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentsave: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::load() {

	// Add NMDC standard op commands
	static const char kickstr[] =
		"$To: %[userNI] From: %[myNI] $<%[myNI]> You are being kicked because: %[line:Reason]|<%[myNI]> %[myNI] is kicking %[userNI] because: %[line:Reason]|$Kick %[userNI]|";
	addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_USER | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
		_("Kick user(s)"), kickstr, "", "op");
	static const char redirstr[] =
		"$OpForceMove $Who:%[userNI]$Where:%[line:Target Server]$Msg:%[line:Message]|";
	addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_USER | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
		_("Redirect user(s)"), redirstr, "", "op");

	try {
		SimpleXML xml;
		Util::migrate(getConfigFile());
		File f(getConfigFile(), File::READ, File::OPEN);
		xml.fromXML(f.read());
		f.close();
		if(xml.findChild("Favorites")) {
			xml.stepIn();
			load(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::load: %s\n", e.getError().c_str());
	}

	try {
		Util::migrate(Util::getPath(Util::PATH_USER_CONFIG) + "Recents.xml");

		SimpleXML xml;
		File f(Util::getPath(Util::PATH_USER_CONFIG) + "Recents.xml", File::READ, File::OPEN);
		xml.fromXML(f.read());
		f.close();
		if(xml.findChild("Recents")) {
			xml.stepIn();
			recentload(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentload: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::load(SimpleXML& aXml) {
	dontSave = true;
	bool needSave = false;

	aXml.resetCurrentChild();
	if(aXml.findChild("Hubs")) {
		aXml.stepIn();

		while(aXml.findChild("Group")) {
			string name = aXml.getChildAttrib("Name");
			if(name.empty())
				continue;
			HubSettings settings;
            		settings.load(aXml);
			favHubGroups[name] = std::move(settings);
		}

		aXml.resetCurrentChild();
		while(aXml.findChild("Hub")) {
			FavoriteHubEntry* e = new FavoriteHubEntry();
			e->setName(aXml.getChildAttrib("Name"));
			e->setHubDescription(aXml.getChildAttrib("Description"));
			e->setPassword(aXml.getChildAttrib("Password"));
			e->setServer(aXml.getChildAttrib("Server"));
			e->setEncoding(aXml.getChildAttrib("Encoding"));
			//BMDC++
			e->setHideShare(Util::toInt(aXml.getChildAttrib("HideShare")));
			//BMDC++
			e->setGroup(aXml.getChildAttrib("Group"));
			e->setMode(aXml.getIntChildAttrib("Mode"));
			e->setChatExtraInfo(aXml.getChildAttrib("ChatExtraInfo"));
			e->setCheckAtConn(Util::toInt(aXml.getChildAttrib("onConnect")));
			e->setCheckClients(Util::toInt(aXml.getChildAttrib("CheckClients")));
			e->setCheckFilelists(Util::toInt(aXml.getChildAttrib("CheckFilelists")));
			e->setProtectUsers(aXml.getChildAttrib("Protects"));
			e->setShowUserList(Util::toInt(aXml.getChildAttrib("UserListToggle")));
			e->setNotify(Util::toInt(aXml.getChildAttrib("NotifyToggle")));
			e->setHubOrder(aXml.getChildAttrib("Order"));
			e->setHubVisible(aXml.getChildAttrib("Visible"));
			e->setHubWidth(aXml.getChildAttrib("Width"));
			e->setPrivate(Util::toInt(aXml.getChildAttrib("Private")));//add
			e->load(aXml);
			favoriteHubs.push_back(e);

			aXml.stepIn();
			while(aXml.findChild("Action")) {
				int actionId = aXml.getIntChildAttrib("ID");
				bool enabled = aXml.getBoolChildAttrib("Active");
				const string& raw = aXml.getChildAttrib("Raw");
				if(RawManager::getInstance()->getValidAction(actionId))
					e->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(enabled, raw, actionId)));
			}
			aXml.stepOut();
		}

		aXml.stepOut();
	}

	aXml.resetCurrentChild();
	if(aXml.findChild("Users")) {
		aXml.stepIn();
		while(aXml.findChild("User")) {
			UserPtr u;
			const string& cid = aXml.getChildAttrib("CID");
			const string& nick = aXml.getChildAttrib("Nick");
			const string& hubUrl = aXml.getChildAttrib("URL");

			if(cid.length() != 39) {
				if(nick.empty() || hubUrl.empty())
					continue;
				u = ClientManager::getInstance()->getUser(nick, hubUrl);
			} else {
				u = ClientManager::getInstance()->getUser(CID(cid));
			}

			//ClientManager::getInstance()->saveUser(u->getCID());
			FavoriteMap::iterator i = users.insert(make_pair(u->getCID(), FavoriteUser(u, nick, hubUrl))).first;

			if(aXml.getBoolChildAttrib("GrantSlot"))
				i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);
			if(aXml.getBoolChildAttrib("Ignore"))
				i->second.setFlag(FavoriteUser::FLAG_IGNORE);

			i->second.setLastSeen((uint32_t)aXml.getIntChildAttrib("LastSeen"));
			i->second.setDescription(aXml.getChildAttrib("UserDescription"));
			string f = aXml.getChildAttrib("HistoryNicks");
			StringTokenizer<string> sl(f, ";");
			i->second.setNicks(sl.getTokens());

		}
		aXml.stepOut();
	}
	aXml.resetCurrentChild();

	aXml.resetCurrentChild();
	if(aXml.findChild("UserCommands")) {
		aXml.stepIn();
		while(aXml.findChild("UserCommand")) {
			addUserCommand(aXml.getIntChildAttrib("Type"), aXml.getIntChildAttrib("Context"), 0, aXml.getChildAttrib("Name"),
				aXml.getChildAttrib("Command"), aXml.getChildAttrib("To"), aXml.getChildAttrib("Hub"));
		}
		aXml.stepOut();
	}

	//Favorite download to dirs
	aXml.resetCurrentChild();
	if(aXml.findChild("FavoriteDirs")) {
		aXml.stepIn();
		while(aXml.findChild("Directory")) {
			string virt = aXml.getChildAttrib("Name");
			string d(aXml.getChildData());
			FavoriteManager::getInstance()->addFavoriteDir(d, virt);
		}
		aXml.stepOut();
	}
	// Indepent Fav
	aXml.resetCurrentChild();
	if(aXml.findChild("FavoriteIUsers"))
	{
		aXml.stepIn();
		while(aXml.findChild("FavoriteIUser")) {
			string nick = aXml.getChildAttrib("Nick");
			time_t lastSeen = (time_t)aXml.getIntChildAttrib("LastSeen");
			string desc = aXml.getChildAttrib("Description");
			addFavoriteIUser(nick,lastSeen, desc);
			auto iif = favoritesNoCid.find(nick);
			if(aXml.getBoolChildAttrib("GrantSlot"))
			{
				if(iif != favoritesNoCid.end())
					iif->second->setFlag(FavoriteUser::FLAG_GRANTSLOT);
			}
		}
		aXml.stepOut();
	}
	aXml.resetCurrentChild();
	if(aXml.findChild("FavoriteIpAddress"))
	{
			aXml.stepIn();
			while(aXml.findChild("FavoriteIP"))
			{
				string ip = aXml.getChildAttrib("IP");
				time_t lastSeen = (time_t)aXml.getIntChildAttrib("LastSeen");
				bool type = aXml.getBoolChildAttrib("Range");
				addFavoriteIp(ip,lastSeen, type ? FavoriteUser::Flags::FLAG_IP_RANGE : FavoriteUser::Flags::FLAG_IP);
			}
			aXml.stepOut();
	}

	dontSave = false;
	if(needSave)
		save();
}

void FavoriteManager::recentload(SimpleXML& aXml) {
	aXml.resetCurrentChild();
	if(aXml.findChild("Hubs")) {
		aXml.stepIn();
		while(aXml.findChild("Hub")) {
			RecentHubEntry* e = new RecentHubEntry();
			e->setName(aXml.getChildAttrib("Name"));
			e->setDescription(aXml.getChildAttrib("Description"));
			e->setUsers(aXml.getChildAttrib("Users"));
			e->setShared(aXml.getChildAttrib("Shared"));
			e->setServer(aXml.getChildAttrib("Server"));
			recentHubs.push_back(e);
		}
		aXml.stepOut();
	}
}

void FavoriteManager::userUpdated(const OnlineUser& info) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(info.getUser()->getCID());
	bool isToSave = false;
	if(i != users.end()) {
		FavoriteUser& fu = i->second;
		fu.update(info);
		fire(FavoriteManagerListener::UserUpdated(), i->second);
		isToSave = true;
	}
	//Indepent Fav
	auto it = favoritesNoCid.find(info.getIdentity().getNick());
	if(it != favoritesNoCid.end())
	{
		FavoriteUser& fiu = *(it->second);
		fiu.update(info);
		fire(FavoriteManagerListener::FavoriteIUpdate(), info.getIdentity().getNick(), fiu);
		isToSave = true;
	}
	if(isToSave)
		save();
}

FavoriteHubEntryPtr FavoriteManager::getFavoriteHubEntry(const string& aServer) const {
	for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i) {
		FavoriteHubEntry* hub = *i;
		if(Util::stricmp(hub->getServer(), aServer) == 0) {
			return hub;
		}
	}
	return nullptr;
}

FavoriteHubEntryList FavoriteManager::getFavoriteHubs(const string& group) const {
	FavoriteHubEntryList ret;
	for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i)
		if((*i)->getGroup() == group)
			ret.push_back(*i);
	return ret;
}

const FavoriteUser* FavoriteManager::getFavoriteUser(const UserPtr &aUser) const {
	Lock l(cs);
	auto i = users.find(aUser->getCID());
	return i == users.end() ? (nullptr) : &i->second;
}

bool FavoriteManager::hasSlot(const UserPtr& aUser) const {
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return false;
	return i->second.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

time_t FavoriteManager::getLastSeen(const UserPtr& aUser) const {
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return 0;
	return i->second.getLastSeen();
}

void FavoriteManager::setAutoGrant(const UserPtr& aUser, bool grant) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	if(grant)
		i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);
	else
		i->second.unsetFlag(FavoriteUser::FLAG_GRANTSLOT);

	fire(FavoriteManagerListener::UserUpdated(), i->second);

	save();
}

void FavoriteManager::setIgnore(const UserPtr& aUser, bool ignore) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	if(ignore)
		i->second.setFlag(FavoriteUser::FLAG_IGNORE);
	else
		i->second.unsetFlag(FavoriteUser::FLAG_IGNORE);

	fire(FavoriteManagerListener::UserUpdated(), i->second);

	save();
}

void FavoriteManager::setUserDescription(const UserPtr& aUser, const string& description) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	i->second.setDescription(description);

	fire(FavoriteManagerListener::UserUpdated(), i->second);

	save();
}

StringList FavoriteManager::getHubLists() {
	StringTokenizer<string> lists(SETTING(HUBLIST_SERVERS), ';');
	return lists.getTokens();
}

const string& FavoriteManager::blacklisted() const {
	if(publicListServer.empty())
		return Util::emptyString;

	// get the host
	uint16_t port = 0;
	string server, file, query, proto, fragment;
	Util::decodeUrl(publicListServer, proto, server, port, file, query, fragment);
	// only keep the last 2 words (example.com)
	size_t pos = server.rfind('.');
	if(pos == string::npos || pos == 0 || pos >= server.size() - 2)
		return Util::emptyString;
	pos = server.rfind('.', pos - 1);
	if(pos != string::npos)
		server = server.substr(pos + 1);

	StringMap::const_iterator i = blacklist.find(server);
	if(i == blacklist.end())
		return Util::emptyString;
	return i->second;
}

void FavoriteManager::addBlacklist(const string& domain, const string& reason) {
	if(domain.empty() || reason.empty())
		return;
	blacklist[domain] = reason;
}

FavoriteHubEntryList::iterator FavoriteManager::getFavoriteHub(const string& aServer) {
	for(FavoriteHubEntryList::iterator i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
		if(Util::stricmp((*i)->getServer(), aServer) == 0) {
			return i;
		}
	}
	return favoriteHubs.end();
}

void FavoriteManager::setHubList(int aHubList) {
	lastServer = aHubList;
	refresh();
}

void FavoriteManager::refresh(bool forceDownload /* = false */) {
	StringList sl = getHubLists();
	if(sl.empty()) {
		fire(FavoriteManagerListener::DownloadFailed(), Util::emptyString);
		return;
	}
	publicListServer = sl[(lastServer) % sl.size()];
	if(Util::strnicmp(publicListServer.c_str(), "http://", 7) != 0) {
		lastServer++;
		fire(FavoriteManagerListener::DownloadFailed(), string(F_("Invalid URL: ") + Util::addBrackets(publicListServer)));
		return;
	}

	if(!forceDownload) {
		string path = Util::getHubListsPath() + Util::validateFileName(publicListServer);
		if(File::getSize(path) > 0) {
			useHttp = false;
			string fileDate;
			{
				Lock l(cs);
				publicListMatrix[publicListServer].clear();
			}
			listType = (Util::stricmp(path.substr(path.size() - 4), ".bz2") == 0) ? TYPE_BZIP2 : TYPE_NORMAL;
			try {
				File cached(path, File::READ, File::OPEN);
				downloadBuf = cached.read();
				char buf[20];
				time_t fd = cached.getLastModified();
				if (strftime(buf, 20, "%x", localtime(&fd))) {
					fileDate = string(buf);
				}
			} catch(const FileException&) {
				downloadBuf = Util::emptyString;
			}
			if(!downloadBuf.empty()) {
				if (onHttpFinished(false)) {
					fire(FavoriteManagerListener::LoadedFromCache(), publicListServer, fileDate);
				}
				return;
			}
		}
	}

	if(!running) {
		useHttp = true;
		{
			Lock l(cs);
			publicListMatrix[publicListServer].clear();
		}
		fire(FavoriteManagerListener::DownloadStarting(), publicListServer);
		if(c == NULL)
			c = new HttpConnection();
		c->addListener(this);
		c->downloadFile(publicListServer);
		running = true;
	}
}

UserCommand::List FavoriteManager::getUserCommands(int ctx, const StringList& hubs) {
	vector<bool> isOp(hubs.size());

	for(size_t i = 0; i < hubs.size(); ++i) {
		if(ClientManager::getInstance()->isOp(ClientManager::getInstance()->getMe(), hubs[i])) {
			isOp[i] = true;
		}
	}

	Lock l(cs);
	UserCommand::List lst;
	for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
		UserCommand& uc = *i;
		if(!(uc.getCtx() & ctx)) {
			continue;
		}

		for(size_t j = 0; j < hubs.size(); ++j) {
			const string& hub = hubs[j];
			bool hubAdc = hub.compare(0, 6, "adc://") == 0 || hub.compare(0, 7, "adcs://") == 0;
			bool commandAdc = uc.adc();
			if(hubAdc && commandAdc) {//its already know is it adc's no reason check again
			{
					lst.push_back(*i);
					break;
			}
			} else if((!hubAdc && !commandAdc) || uc.isChat()) {
				if((uc.getHub().length() == 0) ||
					(uc.getHub() == "op" && isOp[j]) ||
					(uc.getHub() == hub) )
				{
					lst.push_back(*i);
					break;
				}
			}
		}
	}
	return lst;
}

//Raw Manager
bool FavoriteManager::getEnabledAction(FavoriteHubEntry* entry, int64_t actionId) {
	auto h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return false;

	FavoriteHubEntry::FavAction::List::const_iterator i = (*h)->action.find(actionId);
	if(i != (*h)->action.end()) {
		return i->second->getEnabled();
	} else {
		(*h)->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(false)));
		return false;
	}
}

void FavoriteManager::setEnabledAction(FavoriteHubEntry* entry, int64_t actionId, bool enabled) {
	auto h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return;

	FavoriteHubEntry::FavAction::List::iterator i = (*h)->action.find(actionId);
	if(i != (*h)->action.end()) {
		i->second->setEnabled(enabled);
		return;
	}
	if(enabled)
		(*h)->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(true)));
}

bool FavoriteManager::getEnabledRaw(FavoriteHubEntry* entry, int64_t actionId, int64_t rawId) {
	auto h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return false;

	FavoriteHubEntry::FavAction::List::const_iterator i = (*h)->action.find(actionId);
	if(i == (*h)->action.end())
		return false;
	for(std::list<int64_t>::const_iterator j = i->second->raws.begin(); j != i->second->raws.end(); ++j) {
		if(*j == rawId) {
			return true;
		}
	}
	return false;
}

void FavoriteManager::setEnabledRaw(FavoriteHubEntry* entry, int64_t actionId, int64_t rawId, bool enabled) {
	auto h = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(h == favoriteHubs.end())
		return;

	FavoriteHubEntry::FavAction::List::const_iterator i = (*h)->action.find(actionId);
	if(i != (*h)->action.end()) {
		for(std::list<int64_t>::iterator j = i->second->raws.begin(); j != i->second->raws.end(); ++j) {
			if(*j == rawId) {
				if(!enabled)
					i->second->raws.erase(j);
				return;
			}
		}
		if(enabled)
			i->second->raws.push_back(rawId);
		return;
	}

	if(enabled) {
		FavoriteHubEntry::FavAction* act = (*h)->action.insert(make_pair(actionId, new FavoriteHubEntry::FavAction(true))).first->second;
		act->raws.push_back(rawId);
	}
}
//RSX++

// HttpConnectionListener
void FavoriteManager::on(Data, HttpConnection*, const uint8_t* buf, size_t len) noexcept {
	if(useHttp)
		downloadBuf.append((const char*)buf, len);
}

void FavoriteManager::on(Failed, HttpConnection*, const string& aLine) noexcept {
	c->removeListener(this);
	lastServer++;
	running = false;
	if(useHttp){
		downloadBuf = Util::emptyString;
		fire(FavoriteManagerListener::DownloadFailed(), aLine);
	}
}
void FavoriteManager::on(Complete, HttpConnection*, const string& aLine, bool fromCoral) noexcept {
	bool parseSuccess = false;
	c->removeListener(this);
	if(useHttp) {
		if(c->getMimeType() == "application/x-bzip2")
			listType = TYPE_BZIP2;
		parseSuccess = onHttpFinished(true);
	}
	running = false;
	if(parseSuccess) {
		fire(FavoriteManagerListener::DownloadFinished(), aLine, fromCoral);
	}
}
void FavoriteManager::on(Redirected, HttpConnection*, const string& aLine) noexcept {
	if(useHttp)
		fire(FavoriteManagerListener::DownloadStarting(), aLine);
}

void FavoriteManager::on(Retried, HttpConnection*, bool connected) noexcept {
	if(connected)
		downloadBuf.clear();
}

void FavoriteManager::on(UserUpdated, const OnlineUser& user) noexcept {
	userUpdated(user);
}
void FavoriteManager::on(UserDisconnected, const UserPtr& user) noexcept {
	{
		Lock l(cs);
		FavoriteMap::iterator i = users.find(user->getCID());
		if(i != users.end()) {
			i->second.setLastSeen(GET_TIME());
			fire(FavoriteManagerListener::StatusChanged(), i->second);
			save();
		}
	}
}

void FavoriteManager::on(UserConnected, const UserPtr& user) noexcept {
	{
		Lock l(cs);
		FavoriteMap::iterator i = users.find(user->getCID());
		if(i != users.end()) {
			fire(FavoriteManagerListener::StatusChanged(), i->second);
		}
		//Idedetn Favorites
		ClientManager::getInstance()->lock();
		OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(HintedUser(user,Util::emptyString));
		Identity id = ou->getIdentity();
		string nick = id.getNick();
		auto idt = favoritesNoCid.find(nick);
		if(idt != favoritesNoCid.end())
		{
			idt->second->update(*ou);
			fire(FavoriteManagerListener::FavoriteIUpdate(), nick, *(idt->second));
		}
	}
}

void FavoriteManager::mergeHubSettings(const FavoriteHubEntry& entry, HubSettings& settings) const {
        // apply group settings first.
        const string& name = entry.getGroup();
        if(!name.empty()) {
             auto group = favHubGroups.find(name);
             if(group != favHubGroups.end())
              settings.merge(group->second);
        }
        // apply fav entry settings next.
        settings.merge(entry);
}

string FavoriteManager::getAwayMessage(const string& aServer, ParamMap& params) {
	FavoriteHubEntry* hub = getFavoriteHubEntry(aServer);
	string name = hub->getGroup();
	auto group = favHubGroups.find(name);
	if(group != favHubGroups.end())
		return (group->second.get(SettingsManager::DEFAULT_AWAY_MESSAGE,SETTING(DEFAULT_AWAY_MESSAGE)).empty() ?
			(hub ?
			( hub->get(SettingsManager::DEFAULT_AWAY_MESSAGE,SETTING(DEFAULT_AWAY_MESSAGE)).empty() ?
				Util::getAwayMessage(params) : Util::formatParams(hub->get(SettingsManager::DEFAULT_AWAY_MESSAGE,SETTING(DEFAULT_AWAY_MESSAGE)), params) ) : Util::getAwayMessage(params) ) : Util::formatParams(group->second.get(SettingsManager::DEFAULT_AWAY_MESSAGE,SETTING(DEFAULT_AWAY_MESSAGE)), params));

	return Util::getAwayMessage(params);
}

} // namespace dcpp
