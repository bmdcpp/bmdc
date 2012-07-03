/*
 * Copyright (C) 2007-2010 Crise, crise<at>mail.berlios.de
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

#include "stdafx.h"
#include "LuaPlugin.h"

#include "Util.h"

LuaPlugin* LuaPlugin::instance = NULL;

LuaPlugin::LuaPlugin() { L = NULL; }

LuaPlugin::~LuaPlugin() {
	if(L != NULL) {
		if(!hubs.empty()) {
			for(auto i = hubs.begin(); i != hubs.end(); ++i) {
				hub->remove_hub(*i);
				hub->release(*i);
			}

			Util::logMessage("Lua Warning: scripts do not correctly remove hubs they added!");
			hubs.clear();
		}

		for(auto i = events.begin(); i != events.end(); ++i)
			hooks->release_hook(i->second);

		events.clear();
		chatCache.clear();

		lua_close(L);
	}
}

void LuaPlugin::onLoad(DCCorePtr core, bool install, Bool& loadRes) {
	dcpp = core;
	hooks = (DCHooksPtr)core->query_interface(DCINTF_HOOKS, DCINTF_HOOKS_VER);

	hub = (DCHubPtr)core->query_interface(DCINTF_DCPP_HUBS, DCINTF_DCPP_HUBS_VER);
	connection = (DCConnectionPtr)core->query_interface(DCINTF_DCPP_CONNECTIONS, DCINTF_DCPP_CONNECTIONS_VER);

	auto utils = (DCUtilsPtr)core->query_interface(DCINTF_DCPP_UTILS, DCINTF_DCPP_UTILS_VER);
	auto config = (DCConfigPtr)core->query_interface(DCINTF_CONFIG, DCINTF_CONFIG_VER);
	auto logger = (DCLogPtr)core->query_interface(DCINTF_LOGGING, DCINTF_LOGGING_VER);

	if(!utils || !config || !logger || !hub || !connection) {
		loadRes = False;
		return;
	}

	Util::initialize(utils, config, logger);

	if(install) {
		// Default settings
		Util::setConfig("ScriptPath", Util::getPath(PATH_USER_CONFIG) + "scripts" PATH_SEPARATOR_STR);
		Util::setConfig("LuaDebug", false);
		Util::setConfig("FormatChat", true);

		Util::logMessage("LUA Plugin installed, please restart the client to begin using the plugin.");
		return;
	}

	L = lua_open();
	luaL_openlibs(L);

#ifdef WITH_LUAJIT
	// This just turns the whole JIT compiler on with default behaviour (could probably be improved)
	luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_ON);
#endif

	Lunar<LuaManager>::Register(L);
	lua_pop(L, lua_gettop(L));

	events[HOOK_CHAT_OUT]				= hooks->bind_hook(HOOK_CHAT_OUT, &LuaPlugin::chatOutEvent, NULL);

	events[HOOK_HUB_OFFLINE]			= hooks->bind_hook(HOOK_HUB_OFFLINE, &LuaPlugin::hubOfflineEvent, NULL);
	events[HOOK_HUB_ONLINE]				= hooks->bind_hook(HOOK_HUB_ONLINE, &LuaPlugin::hubOnlineEvent, NULL);

	events[HOOK_NETWORK_HUB_IN]			= hooks->bind_hook(HOOK_NETWORK_HUB_IN, &LuaPlugin::netHubInEvent, NULL);
	events[HOOK_NETWORK_HUB_OUT]		= hooks->bind_hook(HOOK_NETWORK_HUB_OUT, &LuaPlugin::netHubOutEvent, NULL);
	events[HOOK_NETWORK_CONN_IN]		= hooks->bind_hook(HOOK_NETWORK_CONN_IN, &LuaPlugin::netConnInEvent, NULL);
	events[HOOK_NETWORK_CONN_OUT]		= hooks->bind_hook(HOOK_NETWORK_CONN_OUT, &LuaPlugin::netConnOutEvent, NULL);

	events[HOOK_UI_PROCESS_CHAT_CMD]	= hooks->bind_hook(HOOK_UI_PROCESS_CHAT_CMD, &LuaPlugin::chatCmdEvent, NULL);

	const string& file = Util::fromUtf8(SETTING(ScriptPath)) + "startup.lua";
	if(Util::fileExists(file))
		ScriptInstance::EvaluateFile(file);

	// This ensures that FormatChatText is only called when present...
	if(CheckFunction("dcpp", "FormatChatText"))
		events[HOOK_UI_CHAT_DISPLAY] = hooks->bind_hook(HOOK_UI_CHAT_DISPLAY, &LuaPlugin::chatDisplayEvent, NULL);
}

Bool LuaPlugin::onHubEnter(HubDataPtr hHub, CommandDataPtr cmd) {
	if(cmd->isPrivate)
		return False;

	if(stricmp(cmd->command, "help") == 0) {
		if(stricmp(cmd->params, "plugins") == 0) {
			const char* help =
				"\t\t\t Help: LuaPlugin \t\t\t\n"
				"\t /lua <code> \t\t\t Evaluate parameter as lua code\n"
				"\t /luafile <file> \t\t\t Execute specified script file\n"
				"\t /luamem \t\t\t Return lua memory usage and version\n"
				"\t /luadebug \t\t\t Toggle non-fatal error messages (default: off)\n"
				"\t /formatchat \t\t\t Toggle lua chat formatting hook (default: on, if available)\n";

			hub->local_message(hHub, help, MSG_SYSTEM);
			return True;
		}
	} else if(stricmp(cmd->command, "lua") == 0) {
		if(strlen(cmd->params) != 0) {
			ScriptInstance::EvaluateChunk(cmd->params);
		} else {
			hub->local_message(hHub, "You must supply a parameter!", MSG_SYSTEM);
		}
		return True;
	} else if(stricmp(cmd->command, "luafile") == 0) {
		if(strlen(cmd->params) > 4) {
			ScriptInstance::EvaluateFile(cmd->params);
		} else {
			hub->local_message(hHub, "You must supply a valid parameter!", MSG_SYSTEM);
		}
		return True;
	} else if(stricmp(cmd->command, "meminfo") == 0 || stricmp(cmd->command, "luamem") == 0) {
		// Run GC and get LUA memory usage
		lua_gc(L, LUA_GCCOLLECT, 0);
		double mem = lua_gc(L, LUA_GCCOUNT, 0);
		const string& stats = "Scripts (" LUA_DIST ") are using " + Util::formatBytes(mem) + " of system memory";

		hub->local_message(hHub, stats.c_str(), MSG_SYSTEM);
		return True;
	} else if(stricmp(cmd->command, "luadebug") == 0) {
		bool state = BOOLSETTING(LuaDebug);
		const char* status = (state ? "Additional error messages disabled" : "Additional error messages enabled");

		Util::setConfig("LuaDebug", state);
		hub->local_message(hHub, status, MSG_SYSTEM);
		return True;
	} else if(stricmp(cmd->command, "formatchat") == 0) {
		bool state = BOOLSETTING(FormatChat);
		const char* status = (state ? "Lua chat formatting disabled" : "Lua chat formatting enabled");

		Util::setConfig("FormatChat", state);
		hub->local_message(hHub, status, MSG_SYSTEM);
		return True;
	}

	// This stupidity is because of the API
	return onOwnChatOut(hHub, string("/" + string(cmd->command) + " " + string(cmd->params)).c_str());
}

Bool LuaPlugin::onOwnChatOut(HubDataPtr hHub, const char* message) {
	Lock l(cs);
	return MakeCall("dcpp", "OnCommandEnter", 1, hHub, string(message)) ? GetLuaBool() : False;
}

Bool LuaPlugin::onHubDisconnected(HubDataPtr hHub) {
	// fixme: DC++ may trigger this incorrectly (for hubs where OnHubAdded was never invoked), if socket creation fails...
	MakeCall(GetHubType(hHub), "OnHubRemoved", 0, hHub);
	removeChatCache(hHub);

	return False;
}

Bool LuaPlugin::onHubConnected(HubDataPtr hHub) {
	MakeCall(GetHubType(hHub), "OnHubAdded", 0, hHub);

	return False;
}

Bool LuaPlugin::onHubDataIn(HubDataPtr hHub, const char* message) {
	Lock l(cs);
	return MakeCall(GetHubType(hHub), "DataArrival", 1, hHub, string(message)) ? GetLuaBool() : False;
}

Bool LuaPlugin::onHubDataOut(HubDataPtr hHub, const char* message, Bool* bBreak) {
	string sText = message;
	if(sText.find("LuaExec") != string::npos) {
		string delim = (hHub->protocol == PROTOCOL_ADC) ? "\n" : "|";
		string::size_type i = 0, j = 0;
		string out;
		while((i = sText.find(delim, i)) < sText.length()) {
			string cmd = sText.substr(j, i-j);
			if(cmd.find("LuaExec") != string::npos) {
				ScriptInstance::EvaluateChunk(cmd.substr(cmd.find(" ") + 1));
			} else if(cmd.length() > 1) {
				out += cmd + delim;
			}
			j = ++i;
		}
		if(out.length() > 1)
			hub->send_protocol_cmd(hHub, out.c_str());

		// We don't need to send this to other plugins
		*bBreak = True;
		return True;
	}

	return False;
}

Bool LuaPlugin::onConnectionDataIn(ConnectionDataPtr hConn, const char* message) {
	Lock l(cs);
	return MakeCall("dcpp", "UserDataIn", 1, hConn, string(message)) ? GetLuaBool() : False;
}

Bool LuaPlugin::onConnectionDataOut(ConnectionDataPtr hConn, const char* message) {
	Lock l(cs);
	return MakeCall("dcpp", "UserDataOut", 1, hConn, string(message)) ? GetLuaBool() : False;
}

Bool LuaPlugin::onFormatChat(HubDataPtr hHub, StringDataPtr line, Bool* bBreak) {
	Lock l(cs);
	if(!hHub || !BOOLSETTING(FormatChat) || !MakeCall("dcpp", "FormatChatText", 1, hHub, string(line->in)))
		return False;

	if(lua_isstring(L, -1)) {
		string text = lua_tostring(L, -1);
		if(strcmp(line->in, text.c_str()) != 0) {
			string& out = chatCache[hHub->url] = text;
			line->out = &out[0];
		}
	}

	lua_settop(L, 0);

	if(line->out != NULL) {
		// We don't send this to other plugins (users can control which formatting applies via plugin order)
		*bBreak = True;
		return True;
	} else return False;
}

Bool LuaPlugin::onTimer() {
	MakeCall("dcpp", "OnTimer", 0, 0);
	return False;
}
