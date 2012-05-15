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

#ifndef DCPLUSPLUS_DCPP_PLUGIN_MANAGER_H
#define DCPLUSPLUS_DCPP_PLUGIN_MANAGER_H

#include "stdinc.h"
#include "typedefs.h"
#include <map>
#include "Singleton.h"
#include "SettingsManager.h"
#include "TimerManager.h"
#include "LogManager.h"

#include "QueueManagerListener.h"
#include "ClientManagerListener.h"

#include "PluginEntity.h"
#include "PluginApiImpl.h"

#ifdef _WIN32
typedef HMODULE pluginHandle;
#else
typedef void* pluginHandle;
#endif

namespace dcpp {

using std::map;
using std::function;

// Represents a plugin in hook context
struct HookSubscriber {
	union {
		DCHOOK hookProc;
		DCHOOKCOMMON hookProcCommon;
	};
	void* common;
	string owner;
};

// Hookable event
struct PluginHook {
	string guid;
	DCHOOK defProc;

	vector<HookSubscriber*> subscribers;
	CriticalSection cs;
};

// Holds a loaded plugin
class PluginInfo : private boost::noncopyable
{
public:
	typedef	DCMAIN	(DCAPI *PLUGIN_INIT)(MetaDataPtr info);

	PluginInfo(const string& aFile, pluginHandle hInst, MetaData aInfo, DCMAIN aMain)
		: dcMain(aMain), info(aInfo), file(aFile), handle(hInst) { };

	~PluginInfo();

	DCMAIN dcMain;
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

	PluginManager() : shutdown(false), secNum(Util::rand()) {
		memset(&dcCore,0,sizeof(DCCore));
		SettingsManager::getInstance()->addListener(this);
	}

	~PluginManager() {
		SettingsManager::getInstance()->removeListener(this);
	}

	void loadPlugins(void (*f)(void*, const string&), void* p);
	bool loadPlugin(const string& fileName, bool isInstall = false);
	bool isLoaded(const string& guid);

	void unloadPlugins();
	void unloadPlugin(int index);

	bool addInactivePlugin(pluginHandle h);
	bool getShutdown() const { return shutdown; }

	void movePlugin(size_t index, int pos);
	const pluginList& getPluginList() { Lock l(cs); return plugins; };
	const PluginInfo* getPlugin(size_t index) { Lock l(cs); return plugins[index]; }

	DCCorePtr getCore() { return &dcCore; }

	// Functions that call the plugin
	bool onChatDisplay(Client* client, string& line);
	bool onChatCommand(Client* client, const string& line);
	bool onChatCommandPM(const HintedUser& user, const string& line, bool isPriv);

	// runHook wrappers for host calls
	bool runHook(const string& guid, dcptr_t pObject, dcptr_t pData) {
		auto i = hooks.find(guid);
		dcassert(i != hooks.end());
		if(shutdown) return false;
		return runHook(i->second, pObject, pData);
	}

	template<class T>
	bool runHook(const string& guid, PluginEntity<T>* entity, dcptr_t pData = NULL) {
		Lock l(entity->cs);
		return runHook(guid, entity->getPluginObject(), pData);
	}

	template<class T>
	bool runHook(const string& guid, PluginEntity<T>* entity, const string& data) {
		return runHook<T>(guid, entity, (dcptr_t)data.c_str());
	}

	// Plugin interface registry
	intfHandle registerInterface(const string& guid, dcptr_t funcs);
	dcptr_t queryInterface(const string& guid);
	bool releaseInterface(intfHandle hInterface);

	// Plugin Hook system
	PluginHook* createHook(const string& guid, DCHOOK defProc);
	bool destroyHook(PluginHook* hook);

	HookSubscriber* bindHook(const string& guid, DCHOOK hookProc, void* pCommon);
	bool runHook(PluginHook* hook, dcptr_t pObject, dcptr_t pData);
	size_t releaseHook(HookSubscriber* subscription);

	// Plugin configuration
	bool hasSettings(const string& pluginName);
	void processSettings(const string& pluginName, const function<void (StringMap&)>& currentSettings);

	void setPluginSetting(const string& pluginName, const string& setting, const string& value);
	const string& getPluginSetting(const string& pluginName, const string& setting);
	void removePluginSetting(const string& pluginName, const string& setting);

private:
	// Check if plugin can be loaded
	bool checkPlugin(const MetaData& info);

	// Listeners
	void on(TimerManagerListener::Second, uint64_t ticks) noexcept { runHook(HOOK_TIMER_SECOND, NULL, &ticks); }
	void on(TimerManagerListener::Minute, uint64_t ticks) noexcept { runHook(HOOK_TIMER_MINUTE, NULL, &ticks); }

	void on(ClientManagerListener::ClientConnected, Client* aClient) noexcept;
	void on(ClientManagerListener::ClientDisconnected, Client* aClient) noexcept;

	void on(QueueManagerListener::Added, QueueItem* qi) noexcept;
	void on(QueueManagerListener::Moved, QueueItem* qi, const string& /*aSource*/) noexcept;
	void on(QueueManagerListener::Removed, QueueItem* qi) noexcept;
	void on(QueueManagerListener::Finished, QueueItem* qi, const string& /*dir*/, int64_t /*speed*/) noexcept;

	void on(SettingsManagerListener::Load, SimpleXML& /*xml*/) noexcept;
	void on(SettingsManagerListener::Save, SimpleXML& /*xml*/) noexcept;

	pluginList plugins;
	vector<pluginHandle> inactive;

	map<string, PluginHook*> hooks;
	map<string, dcptr_t> interfaces;

	map<string, StringMap> settings;

	DCCore dcCore;
	CriticalSection cs, csHook;
	bool shutdown;

	uintptr_t secNum;
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_PLUGIN_MANAGER_H)

/**
 * @file
 * $Id: PluginManager.h 1245 2012-01-21 15:09:54Z crise $
 */
