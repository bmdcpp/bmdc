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

#ifndef DCPLUSPLUS_DCPP_PLUGIN_MANAGER_H
#define DCPLUSPLUS_DCPP_PLUGIN_MANAGER_H

#include <vector>
#include <algorithm>

// for tstring
#include "DCPlusPlus.h"

#include "Singleton.h"
#include "SettingsManager.h"
#include "TimerManager.h"
#include "ClientManager.h"
#include "QueueManager.h"
#include "LogManager.h"
#include "UserConnection.h"

#include "PluginDefs.h"
#include "PluginApiImpl.h"

#ifdef _WIN32
typedef HMODULE pluginHandle;
#else
typedef void* pluginHandle;
#endif

namespace dcpp {

using std::vector;
using std::swap;

// Represents a plugin in hook context
struct HookSubscriber {
	union {
		DCHOOK hookProc;
		DCHOOKEX hookProcEx;
		DCHOOKCOMMON hookProcCommon;
		DCHOOKCOMMONEX hookProcCommonEx;
	};
	void* common;
	HookID owner;
};

// Hookable event (or event group)
struct PluginHook {
	typedef vector<HookSubscriber*> subsList;

	HookID id;
	HookType type;
	subsList subscribers;
	DCHOOK defProc;
};

// Holds a loaded plugin
class PluginInfo : private boost::noncopyable
{
public:
	typedef	DCHOOK	(DCAPI *PLUGIN_INIT)(MetaDataPtr info);

	PluginInfo(const string& aFile, pluginHandle hInst, MetaData aInfo, DCHOOK aHook)
		: mainHook(aHook), info(aInfo), file(aFile), handle(hInst) { };

	~PluginInfo();

	DCHOOK mainHook;
	const MetaData& getInfo() const { return info; }
	const string& getFile() const { return file; }

private:
	MetaData info;
	string file;
	pluginHandle handle;
};

class PluginManager : public Singleton<PluginManager>, private TimerManagerListener,
	private ClientManagerListener, private QueueManagerListener, private SettingsManagerListener
{
public:
	typedef vector<PluginInfo*> pluginList;

	PluginManager() : shutdown(false) {
		memset(&dcCore, 0, sizeof(DCCore));
		SettingsManager::getInstance()->addListener(this);
	}

	~PluginManager() throw() {
		SettingsManager::getInstance()->removeListener(this);
	}

	void loadPlugins(void (*f)(void*, const string&), void* p);
	bool loadPlugin(const string& fileName, bool isInstall = false);

	void unloadPlugins();
	void unloadPlugin(int index);

	bool addInactivePlugin(pluginHandle h);
	bool getShutdown() const { return shutdown; }

	void movePlugin(uint32_t index, int pos);
	const pluginList& getPluginList() { Lock l(cs); return plugins; };
	const PluginInfo* getPlugin(uint32_t index) { Lock l(cs); return plugins[index]; }

	// Functions that call the plugin
	bool onChatDisplay(Client* client, string line);
	bool onChatCommand(Client* client, const string& line);
	bool onChatCommandPM(const HintedUser& user, const string& line, bool isPriv);
	bool onOutgoingChat(Client* client, const string& message);
	bool onOutgoingPM(const OnlineUser& user, const string& message);
	bool onIncomingChat(Client* client, const string& message);
	bool onIncomingPM(const OnlineUser* user, const string& message);
	bool onIncomingHubData(Client* client, const string& message);
	bool onOutgoingHubData(Client* client, const string& message);
	bool onIncomingConnectionData(UserConnection* uc, const string& message);
	bool onOutgoingConnectionData(UserConnection* uc, const string& message);

	bool callHook(HookID hookId, uint32_t eventId, dcptr_t pData) {
		hookList::iterator i = hooks.find(hookId);
		if(i == hooks.end()) return false;
		return callHook(i->second, eventId, pData);
	}

private:
	friend class PluginApiImpl;

	typedef vector<pluginHandle> inactiveList;
	typedef map<HookID, PluginHook*> hookList;
	typedef map<string, StringMap> settingsMap;
	typedef map<string, uint32_t> messageMap;

	// Plugin finder
	struct PluginCompare {
		PluginCompare(const char* guid) : tmpGuid(guid) { }
		bool operator()(const PluginInfo* plugin) const { return strcmp(plugin->getInfo().guid, tmpGuid) == 0; }
	private:
		PluginCompare& operator=(const PluginCompare&);
		const char* tmpGuid;
	};

	// Check if plugin can be loaded
	bool checkPlugin(const MetaData& info);

	// Hook functions
	hookHandle createHook(HookID hookId, HookType hookType, DCHOOK defProc);
	void destroyHook(hookHandle hHook);

	subsHandle setHook(HookID hookId, DCHOOK hookProc, void* pCommon);
	bool callHook(PluginHook* hook, uint32_t eventId, dcptr_t pData);
	size_t unHook(subsHandle hHook);

	// Settings management
	void setSetting(const string& pluginName, const string& setting, const string& value) {
		settings[pluginName][setting] = value;
	}

	const string& getSetting(const string& pluginName, const string& setting) {
		settingsMap::const_iterator i;
		if((i = settings.find(pluginName)) != settings.end()) {
			StringMap::const_iterator j;
			if((j = i->second.find(setting)) != i->second.end())
				return j->second;
		}
		return Util::emptyString;
	}

	void removeSetting(const string& pluginName, const string& setting) {
		settingsMap::iterator i;
		if((i = settings.find(pluginName)) != settings.end()) {
			StringMap::iterator j;
			if((j = i->second.find(setting)) != i->second.end())
				i->second.erase(j);
		}
	}

	// Message register (to avoid collisions with plugin defined messages)
	uint32_t registerMessage(HookType type, const string& messageName) {
		messageMap::const_iterator i;
		if((i = messages.find(messageName)) == messages.end()) {
			messages[messageName] = ++(msgMaps[type]);
			return msgMaps[type];
		} else return i->second;
	}

	uint32_t registerRange(HookType type, const string& messageName, uint32_t count) {
		messageMap::const_iterator i;
		if((i = messages.find(messageName)) == messages.end()) {
			messages[messageName] = msgMaps[type] + 1;
			msgMaps[type] += count;
			return (msgMaps[type] - count) + 1;
		} else return i->second;
	}

	uint32_t seekMessage(const string& messageName) {
		messageMap::const_iterator i;
		if((i = messages.find(messageName)) != messages.end())
			return i->second;
		return 0; // always reserved
	}

	// Listeners
	void on(TimerManagerListener::Second, uint64_t ticks) throw();
	void on(TimerManagerListener::Minute, uint64_t ticks) throw();
	void on(ClientManagerListener::ClientConnected, Client* aClient) throw();
	void on(ClientManagerListener::ClientDisconnected, Client* aClient) throw();
	void on(QueueManagerListener::Added, QueueItem* qi) throw();
	void on(QueueManagerListener::Moved, QueueItem* qi, const string&) throw();
	void on(QueueManagerListener::Removed, QueueItem* qi) throw();
	void on(QueueManagerListener::Finished, QueueItem* qi, const string&, int64_t) throw();

	void on(SettingsManagerListener::Load, SimpleXML& /*xml*/) throw();
	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) throw();

	// Tracks plugin reserved messages
	static uint32_t msgMaps[3];

	pluginList plugins;
	inactiveList inactive;
	hookList hooks;
	settingsMap settings;
	messageMap messages;

	DCCore dcCore;
	CriticalSection cs, csHook, csCb;
	hookHandle coreHook;
	bool shutdown;
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_PLUGIN_MANAGER_H)

/**
 * @file
 * $Id: PluginManager.h 707 2010-09-03 15:40:16Z crise $
 */
