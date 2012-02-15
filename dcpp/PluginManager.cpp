/* 
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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
#include "DCPlusPlus.h"

#include "PluginManager.h"
#include "ConnectionManager.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "AdcHub.h"

#ifdef _WIN32
# define PLUGIN_EXT "*.dll"

# define LOAD_LIBRARY(filename) ::LoadLibrary(Text::toT(filename).c_str())
# define FREE_LIBRARY(lib) ::FreeLibrary(lib)
# define GET_ADDRESS(lib, name) ::GetProcAddress(lib, name)
# define GET_ERROR() Util::translateError(GetLastError())
#else
# include "dlfcn.h"
# define PLUGIN_EXT "*.so"

# define LOAD_LIBRARY(filename) ::dlopen(filename.c_str(), RTLD_LAZY | RTLD_GLOBAL)
# define FREE_LIBRARY(lib) ::dlclose(lib)
# define GET_ADDRESS(lib, name) ::dlsym(lib, name)
# define GET_ERROR() ::dlerror()
#endif

namespace dcpp {

uint32_t PluginManager::msgMaps[3] = { HOOK_USER, CALLBACK_USER, EVENT_USER };

PluginInfo::~PluginInfo() {
	bool isSafe = true;
	if(mainHook((PluginManager::getInstance()->getShutdown() ? ON_UNLOAD : ON_UNINSTALL), NULL) == False) {
		// Plugin performs operation critical tasks (runtime unload not possible)
		isSafe = !PluginManager::getInstance()->addInactivePlugin(handle);
	} if(isSafe && handle != NULL) {
		FREE_LIBRARY(handle);
		handle = NULL;
	}
}

void PluginManager::loadPlugins(void (*f)(void*, const string&), void* p) {
	coreHook = PluginApiImpl::initAPI(dcCore);

	StringTokenizer<string> st(getSetting("CoreSetup", "Plugins"), ";");
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		if(!loadPlugin(*i) || !f) continue;
		(*f)(p, Util::getFileName(*i));
	}
}

bool PluginManager::loadPlugin(const string& fileName, bool isInstall /*= false*/) {
	Lock l(cs);
	pluginHandle hr = LOAD_LIBRARY(fileName);
	if(!hr) {
		LogManager::getInstance()->message("Error loading " + Util::getFileName(fileName) + ": " + GET_ERROR());
		return false;
	}

	PluginInfo::PLUGIN_INIT pluginInfo = reinterpret_cast<PluginInfo::PLUGIN_INIT>(GET_ADDRESS(hr, "pluginInit"));

	if(pluginInfo != NULL) {
		MetaData info;
		memset(&info, 0, sizeof(MetaData));

		DCHOOK mainHook;
		if((mainHook = pluginInfo(&info)) != NULL) {
			if(checkPlugin(info)) {
				if(mainHook((isInstall ? ON_INSTALL : ON_LOAD), &dcCore) != False) {
					plugins.push_back(new PluginInfo(fileName, hr, info, mainHook));
					return true;
				}
			}
		}
	} else LogManager::getInstance()->message(str(F_("%1% is not a valid plugin") % Util::getFileName(fileName)));

	FREE_LIBRARY(hr);
	return false;
}

bool PluginManager::checkPlugin(const MetaData& info) {
	// Check if user is trying to load a duplicate
	pluginList::const_iterator iter = std::find_if(plugins.begin(), plugins.end(), PluginCompare(info.guid));
	if(iter != plugins.end())
		return false;

	// Check API compatibility
	if(info.apiVersion < DCAPI_COMPATIBLE_VER || info.compatibleVersion > DCAPI_VER)
		return false;

	// Check that all dependencies are loaded
	if(info.numDependencies != 0) {
		for(size_t i = 0; i < info.numDependencies; ++i) {
			if((iter = std::find_if(plugins.begin(), plugins.end(), PluginCompare(info.dependencies[i]))) == plugins.end()) {
				// TODO: report to user
				return false;
			}
		}
	}

	return true;
}

void PluginManager::unloadPlugins() {
	Lock l(cs);
	shutdown = true;

	// Off we go...
	string installed;
	for(pluginList::reverse_iterator i = plugins.rbegin(); i != plugins.rend();) {
		PluginInfo* plugin = *i;
		installed.size() ? installed = plugin->getFile() + ";" + installed : installed = plugin->getFile(); 
		i = pluginList::reverse_iterator(plugins.erase(i.base()-1));
		delete plugin;
	}

	// Update plugin order
	setSetting("CoreSetup", "Plugins", installed);

	// Really unload plugins that have been flagged inactive (ON_UNLOAD returns False)
	for(inactiveList::iterator i = inactive.begin(); i != inactive.end(); ++i)
		FREE_LIBRARY(*i);

	if(hooks.size() > 1) {
		// Destroy all hooks not freed correctly
		for(hookList::iterator i = hooks.begin(); hooks.size() > 1;) {
			if(i->second != coreHook) {
				PluginHook* hook = i->second;
				for_each(hook->subscribers.begin(), hook->subscribers.end(), DeleteFunction());
				hook->subscribers.clear();
				hooks.erase(i++);
				delete hook;
			} else ++i;
		}

		TimerManager::getInstance()->removeListener(this);
		ClientManager::getInstance()->removeListener(this);
		QueueManager::getInstance()->removeListener(this);
	}

	PluginApiImpl::releaseAPI(coreHook);
}

void PluginManager::unloadPlugin(int index) {
	Lock l(cs);

	PluginInfo* plugin = plugins[index];
	plugins.erase(plugins.begin() + index);
	delete plugin;
}

bool PluginManager::addInactivePlugin(pluginHandle h) {
	if(std::find(inactive.begin(), inactive.end(), h) == inactive.end()) {
		inactive.push_back(h);
		return true;
	}
	return false;
}

void PluginManager::movePlugin(uint32_t index, int pos) {
	Lock l(cs);
	pluginList::iterator i = (plugins.begin() + index);
	swap(*i, *(i + pos));
}

// Functions that call the plugin
bool PluginManager::onChatDisplay(Client* client, string line) {
	hookList::iterator i = hooks.find(HOOK_UI);
	if(i == hooks.end()) return false;

	StringData data;
	memset(&data, 0, sizeof(StringData));

	string tmpLine;
	data.object = (client && client->isConnected()) ? client : NULL;
	data.in = Text::fromT(line, tmpLine).c_str();

	if(callHook(i->second, UI_CHAT_DISPLAY, &data) && data.out != NULL) {
		line.clear();
		Text::toT(data.out, line);

		// NOTE: Here we rely on plugins using DCCore::memalloc
		free(data.out);
		return true;
	}

	return false;
}

bool PluginManager::onChatCommand(Client* client, const string& line) {
	hookList::iterator i = hooks.find(HOOK_UI);
	if(i == hooks.end()) return false;

	CommandData data;
	memset(&data, 0, sizeof(CommandData));

	string cmd, param;
	string::size_type si = line.find(' ');
	if(si != string::npos) {
		param = line.substr(si + 1);
		cmd = line.substr(1, si - 1);
	} else {
		cmd = line.substr(1);
	}

	ClientData object;
	memset(&object, 0, sizeof(ClientData));

	object.data = line.c_str();
	object.url = client->getHubUrl().c_str();
	object.ip = client->getIp().c_str();
	object.object = client;
	Util::toString(object.port) = client->getPort();
	object.protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
	object.isOp = client->isOp() ? True : False;
	object.isSecure = client->isSecure() ? True : False;

	data.object = &object;
	data.command = cmd.c_str();
	data.params = param.c_str();
	data.isPrivate = False;

	return callHook(i->second, UI_PROCESS_CHAT_CMD, &data);
}

bool PluginManager::onChatCommandPM(const HintedUser& user, const string& line, bool isPriv) {
	hookList::iterator i = hooks.find(HOOK_UI);
	if(i == hooks.end()) return false;

	// Hopefully this is safe...
	bool res = false;
	auto lock = ClientManager::getInstance()->lock();
	OnlineUser* ou = ClientManager::getInstance()->findOnlineUser(user.user->getCID(), user.hint, isPriv);

	if(ou) {
		CommandData data;
		memset(&data, 0, sizeof(CommandData));

		string cmd, param;
		string::size_type si = line.find(' ');
		if(si != string::npos) {
			param = line.substr(si + 1);
			cmd = line.substr(1, si - 1);
		} else {
			cmd = line.substr(1);
		}

		UserData object;
		memset(&object, 0, sizeof(UserData));

		object.data = line.c_str();
		object.hubHint = ou->getClient().getHubUrl().c_str();
		object.cid = ou->getUser()->getCID().data();
		object.object = (dcptr_t)ou;
		object.isOp = ou->getIdentity().isOp() ? True : False;
		if(ou->getUser()->isNMDC()) {
			object.protocol = PROTOCOL_NMDC;
			strncpy(object.uid.nick, ou->getIdentity().getNick().c_str(), 35);
		} else {
			object.protocol = PROTOCOL_ADC;
			object.uid.sid = ou->getIdentity().getSID();
		}

		data.object = &object;
		data.command = cmd.c_str();
		data.params = param.c_str();
		data.isPrivate = True;

		res = callHook(i->second, UI_PROCESS_CHAT_CMD, &data);
	}

	return res;
}

bool PluginManager::onOutgoingChat(Client* client, const string& message) {
	hookList::iterator i = hooks.find(HOOK_CHAT);
	if(i == hooks.end()) return false;

	ClientData data;
	memset(&data, 0, sizeof(ClientData));

	data.data = message.c_str();
	data.url = client->getHubUrl().c_str();
	data.ip = client->getIp().c_str();
	data.object = client;
	Util::toString(data.port) = client->getPort();
	data.protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
	data.isOp = client->isOp() ? True : False;
	data.isSecure = client->isSecure() ? True : False;

	return callHook(i->second, CHAT_OUT, &data);
}

bool PluginManager::onOutgoingPM(const OnlineUser& user, const string& message) {
	hookList::iterator i = hooks.find(HOOK_CHAT);
	if(i == hooks.end()) return false;

	UserData data;
	memset(&data, 0, sizeof(UserData));

	data.data = message.c_str();
	data.hubHint = user.getClient().getHubUrl().c_str();
	data.cid = user.getUser()->getCID().data();
	data.object = (dcptr_t)&user;
	data.isOp = user.getIdentity().isOp() ? True : False;
	if(user.getUser()->isNMDC()) {
		data.protocol = PROTOCOL_NMDC;
		strncpy(data.uid.nick, user.getIdentity().getNick().c_str(), 35);
	} else {
		data.protocol = PROTOCOL_ADC;
		data.uid.sid = user.getIdentity().getSID();
	}

	return callHook(i->second, CHAT_PM_OUT, &data);
}

bool PluginManager::onIncomingChat(Client* client, const string& message) {
	hookList::iterator i = hooks.find(HOOK_CHAT);
	if(i == hooks.end()) return false;

	ClientData data;
	memset(&data, 0, sizeof(ClientData));

	data.data = message.c_str();
	data.url = client->getHubUrl().c_str();
	data.ip = client->getIp().c_str();
	data.object = client;
	Util::toString(data.port) = client->getPort();
	data.protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
	data.isOp = client->isOp() ? True : False;
	data.isSecure = client->isSecure() ? True : False;

	return callHook(i->second, CHAT_IN, &data);
}

bool PluginManager::onIncomingPM(const OnlineUser* user, const string& message) {
	hookList::iterator i = hooks.find(HOOK_CHAT);
	if(i == hooks.end()) return false;

	UserData data;
	memset(&data, 0, sizeof(UserData));

	data.data = message.c_str();
	data.hubHint = user->getClient().getHubUrl().c_str();
	data.cid = user->getUser()->getCID().data();
	data.object = (dcptr_t)user;
	data.isOp = user->getIdentity().isOp() ? True : False;
	if(user->getUser()->isNMDC()) {
		data.protocol = PROTOCOL_NMDC;
		strncpy(data.uid.nick, user->getIdentity().getNick().c_str(), 35);
	} else {
		data.protocol = PROTOCOL_ADC;
		data.uid.sid = user->getIdentity().getSID();
	}

	return callHook(i->second, CHAT_PM_IN, &data);
}

bool PluginManager::onIncomingHubData(Client* client, const string& message) {
	hookList::iterator i = hooks.find(HOOK_PROTOCOL);
	if(i == hooks.end()) return false;

	ClientData data;
	memset(&data, 0, sizeof(ClientData));

	data.data = message.c_str();
	data.url = client->getHubUrl().c_str();
	data.ip = client->getIp().c_str();
	data.object = client;
	Util::toString(data.port) = client->getPort();
	data.protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
	data.isOp = client->isOp() ? True : False;
	data.isSecure = client->isSecure() ? True : False;

	return callHook(i->second, HUB_IN, &data);
}

bool PluginManager::onOutgoingHubData(Client* client, const string& message) {
	hookList::iterator i =  hooks.find(HOOK_PROTOCOL);
	if(i == hooks.end()) return false;

	ClientData data;
	memset(&data, 0, sizeof(ClientData));

	data.data = message.c_str();
	data.url = client->getHubUrl().c_str();
	data.ip = client->getIp().c_str();
	data.object = client;
	Util::toString(data.port) = client->getPort();
	data.protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
	data.isOp = client->isOp() ? True : False;
	data.isSecure = client->isSecure() ? True : False;

	return callHook(i->second, HUB_OUT, &data);
}

bool PluginManager::onIncomingConnectionData(UserConnection* uc, const string& message) {
	hookList::iterator i =  hooks.find(HOOK_PROTOCOL);
	if(i == hooks.end()) return false;

	ConnectionData data;
	memset(&data, 0, sizeof(ConnectionData));

	data.data = message.c_str();
	data.ip = uc->getRemoteIp().c_str();
	data.object = uc;
	Util::toString(data.port) = uc->getPort();
	data.protocol = (message[0] == '$') ? PROTOCOL_NMDC : PROTOCOL_ADC;
	data.isOp = uc->isSet(UserConnection::FLAG_OP) ? True : False;
	data.isSecure = uc->isSecure() ? True : False;

	return callHook(i->second, CONN_IN, &data);
}

bool PluginManager::onOutgoingConnectionData(UserConnection* uc, const string& message) {
	hookList::iterator i = hooks.find(HOOK_PROTOCOL);
	if(i == hooks.end()) return false;

	ConnectionData data;
	memset(&data, 0, sizeof(ConnectionData));

	data.data = message.c_str();
	data.ip = uc->getRemoteIp().c_str();
	data.object = uc;
	Util::toString(data.port) = uc->getPort();
	data.protocol = (message[0] == '$') ? PROTOCOL_NMDC : PROTOCOL_ADC;
	data.isOp = uc->isSet(UserConnection::FLAG_OP) ? True : False;
	data.isSecure = uc->isSecure() ? True : False;

	return callHook(i->second, CONN_OUT, &data);
}

// Hook functions
hookHandle PluginManager::createHook(HookID hookId, HookType hookType, DCHOOK defProc) {
	Lock l((hookType == HOOK_EVENT) ? csHook : csCb);

	hookList::iterator i = hooks.find(hookId);
	if(i != hooks.end()) return NULL;

	PluginHook* hook = new PluginHook();
	hook->id = hookId;
	hook->type = hookType;
	hook->defProc = defProc;
	hooks[hookId] = hook;
	return hook;
}

void PluginManager::destroyHook(hookHandle hHook) {
	PluginHook* hook = (PluginHook*)hHook;
	hookList::iterator i;

	Lock l((hook->type == HOOK_EVENT) ? csHook : csCb);

	if((i = hooks.find(hook->id)) != hooks.end()) {
		for_each(hook->subscribers.begin(), hook->subscribers.end(), DeleteFunction());
		hook->subscribers.clear();
		hooks.erase(i);
		delete hook;
	}
}

subsHandle PluginManager::setHook(HookID hookId, DCHOOK hookProc, void* pCommon) {
	PluginHook* hook;
	hookList::iterator i = hooks.find(hookId);

	// Handle "common" hooks
	if(i == hooks.end()) {
		if(hookId < HOOK_USER) {
			hook = (PluginHook*)createHook(hookId, HOOK_EVENT, NULL);
			switch(hookId) {
				case HOOK_TIMER: TimerManager::getInstance()->addListener(this); break;
				case HOOK_HUBS: ClientManager::getInstance()->addListener(this); break;
				case HOOK_QUEUE: QueueManager::getInstance()->addListener(this); break;
				default: /* do nothing - except silence mingw */ break;
			}
		} else return NULL;
	} else hook = i->second;

	Lock l((hook->type == HOOK_EVENT) ? csHook : csCb);

	HookSubscriber* subscribtion = new HookSubscriber();
	subscribtion->hookProc = hookProc;
	subscribtion->common = pCommon;
	subscribtion->owner = hookId;
	hook->subscribers.push_back(subscribtion);

	return subscribtion;
}

bool PluginManager::callHook(PluginHook* hook, uint32_t eventId, dcptr_t pData) {
	Lock l((hook->type == HOOK_EVENT) ? csHook : csCb);

	Bool bBreak = False;
	Bool bRes = False;
	for(PluginHook::subsList::const_iterator i = hook->subscribers.begin(); i != hook->subscribers.end(); ++i) {
		HookSubscriber* sub = *i;
		switch(hook->type) {
			case HOOK_CALLBACK:
				bRes = (sub->common ? sub->hookProcCommonEx(eventId, pData, sub->common, &bBreak) : sub->hookProcEx(eventId, pData, &bBreak));
				if(bBreak)
					return (bRes != False);
				break;
			case HOOK_EVENT:
				if(sub->common ? sub->hookProcCommon(eventId, pData, sub->common) : sub->hookProc(eventId, pData))
					bRes = True;
				break;
			default:
				dcassert(0);
		}
	}

	// Call default hook procedure for all unused hooks and overloadables
	if(hook->defProc && (hook->subscribers.empty() || hook->type == HOOK_CALLBACK)) {
		if(hook->defProc(eventId, pData))
			bRes = True;
	}

	return (bRes != False);
}

size_t PluginManager::unHook(subsHandle hHook) {
	if(hHook == NULL)
		return 0;

	hookList::iterator i;
	PluginHook* hook = NULL;
	HookSubscriber* subscription = reinterpret_cast<HookSubscriber*>(hHook);

	if((i = hooks.find(subscription->owner)) != hooks.end())
		hook = i->second;

	if(hook == NULL)
		return 0;

	Lock l((hook->type == HOOK_EVENT) ? csHook : csCb);

	hook->subscribers.erase(std::remove(hook->subscribers.begin(), hook->subscribers.end(), subscription), hook->subscribers.end());
	delete subscription;

	// Handle "common" hooks
	if(hook->subscribers.empty() && hook->id < HOOK_USER) {
		switch(hook->id) {
			case HOOK_TIMER: TimerManager::getInstance()->removeListener(this); break;
			case HOOK_HUBS: ClientManager::getInstance()->removeListener(this); break;
			case HOOK_QUEUE: QueueManager::getInstance()->removeListener(this); break;
			default: /* do nothing - except silence mingw */ break;
		}
		hooks.erase(i);
		delete hook; return 0;
	}

	return hook->subscribers.size();
}

// Listeners
void PluginManager::on(TimerManagerListener::Second, uint64_t ticks) throw() {
	callHook(hooks.find(HOOK_TIMER)->second, TIMER_SECOND, &ticks);
}

void PluginManager::on(TimerManagerListener::Minute, uint64_t ticks) throw() {
	callHook(hooks.find(HOOK_TIMER)->second, TIMER_MINUTE, &ticks);
}

void PluginManager::on(ClientManagerListener::ClientDisconnected, Client* aClient) throw() {
	ClientData data;
	memset(&data, 0, sizeof(ClientData));

	data.url = aClient->getHubUrl().c_str();
	data.object = aClient;
	data.protocol = dynamic_cast<AdcHub*>(aClient) ? PROTOCOL_ADC : PROTOCOL_NMDC;

	callHook(hooks.find(HOOK_HUBS)->second, HUB_OFFLINE, &data);
}

void PluginManager::on(ClientManagerListener::ClientConnected, Client* aClient) throw() {
	ClientData data;
	memset(&data, 0, sizeof(ClientData));

	data.url = aClient->getHubUrl().c_str();
	data.ip = aClient->getIp().c_str();
	data.object = aClient;
	Util::toString(data.port) = aClient->getPort();
	data.protocol = dynamic_cast<AdcHub*>(aClient) ? PROTOCOL_ADC : PROTOCOL_NMDC;
	data.isOp = aClient->isOp() ? True : False;
	data.isSecure = aClient->isSecure() ? True : False;

	callHook(hooks.find(HOOK_HUBS)->second, HUB_ONLINE, &data);
}

void PluginManager::on(QueueManagerListener::Added, QueueItem* qi) throw() {
	QueueData data;
	memset(&data, 0, sizeof(QueueData));

	data.file = qi->getTargetFileName().c_str();
	data.target = qi->getTarget().c_str();
	data.location = qi->isFinished() ? data.target : qi->getTempTarget().c_str();
	data.hash = qi->getTTH().data;
	data.object = qi;
	data.size = qi->getSize();
	data.isFileList = qi->isSet(QueueItem::FLAG_USER_LIST) ? True : False;

	callHook(hooks.find(HOOK_QUEUE)->second, QUEUE_ADD, &data);
}

void PluginManager::on(QueueManagerListener::Moved, QueueItem* qi, const string&) throw() {
	QueueData data;
	memset(&data, 0, sizeof(QueueData));

	data.file = qi->getTargetFileName().c_str();
	data.target = qi->getTarget().c_str();
	data.location = qi->isFinished() ? data.target : qi->getTempTarget().c_str();
	data.hash = qi->getTTH().data;
	data.object = qi;
	data.size = qi->getSize();
	data.isFileList = qi->isSet(QueueItem::FLAG_USER_LIST) ? True : False;

	callHook(hooks.find(HOOK_QUEUE)->second, QUEUE_MOVE, &data);
}

void PluginManager::on(QueueManagerListener::Removed, QueueItem* qi) throw() {
	QueueData data;
	memset(&data, 0, sizeof(QueueData));

	data.file = qi->getTargetFileName().c_str();
	data.target = qi->getTarget().c_str();
	data.location = qi->isFinished() ? data.target : qi->getTempTarget().c_str();
	data.hash = qi->getTTH().data;
	data.object = qi;
	data.size = qi->getSize();
	data.isFileList = qi->isSet(QueueItem::FLAG_USER_LIST) ? True : False;

	callHook(hooks.find(HOOK_QUEUE)->second, QUEUE_REMOVE, &data);
}

// Finished items are no longer quaranteed to be removed right away
void PluginManager::on(QueueManagerListener::Finished, QueueItem* qi, const string&, int64_t) throw() {
	QueueData data;
	memset(&data, 0, sizeof(QueueData));

	data.file = qi->getTargetFileName().c_str();
	data.target = qi->getTarget().c_str();
	data.location = qi->isFinished() ? data.target : qi->getTempTarget().c_str();
	data.hash = qi->getTTH().data;
	data.object = qi;
	data.size = qi->getSize();
	data.isFileList = qi->isSet(QueueItem::FLAG_USER_LIST) ? True : False;

	callHook(hooks.find(HOOK_QUEUE)->second, QUEUE_FINISHED, &data);
}

// Load / Save settings
void PluginManager::on(SettingsManagerListener::Load, SimpleXML& /*xml*/) throw() {
	Lock l(cs);

	try {
		Util::migrate(Util::getPath(Util::PATH_USER_LOCAL) + "Plugins.xml");

		SimpleXML xml;
		xml.fromXML(File(Util::getPath(Util::PATH_USER_LOCAL) + "Plugins.xml", File::READ, File::OPEN).read());
		if(xml.findChild("Plugins")) {
			xml.stepIn();
			while(xml.findChild("Plugin")) {
				const string& pluginGuid = xml.getChildAttrib("Guid");
				xml.stepIn();
				StringMap settings = xml.getCurrentChildren();
				for(StringMapIter j = settings.begin(); j != settings.end(); ++j) {
					setSetting(pluginGuid, j->first, j->second);
				}
				xml.stepOut();
			}
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("PluginManager::loadSettings: %s\n", e.getError().c_str());
	}
 }

void PluginManager::on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw() {
	Lock l(cs);

	try {
		SimpleXML xml;
		xml.addTag("Plugins");
		xml.stepIn();
		for(settingsMap::const_iterator i = settings.begin(); i != settings.end(); ++i){
			xml.addTag("Plugin");
			xml.stepIn();
			for(StringMap::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				xml.addTag(j->first, j->second);			
			}
			xml.stepOut();
			xml.addChildAttrib("Guid", i->first);
		}
		xml.stepOut();

		string fname = Util::getPath(Util::PATH_USER_LOCAL) + "Plugins.xml";
		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("PluginManager::saveSettings: %s\n", e.getError().c_str());
	}
}

} // namespace dcpp

/**
 * @file
 * $Id: PluginManager.cpp 712 2010-09-07 14:46:45Z crise $
 */
