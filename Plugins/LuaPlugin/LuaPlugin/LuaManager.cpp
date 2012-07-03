/*
 * Copyright (C) 2008 cologic, cologic@parsoma.net
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
#include "LuaManager.h"

#include "Util.h"
#include "LuaPlugin.h"
#include <stdlib.h>

const char LuaManager::className[] = "DC";
Lunar<LuaManager>::RegType LuaManager::methods[] = {
	{"SendHubMessage", &LuaManager::SendHubMessage },
	{"SendClientMessage", &LuaManager::SendClientMessage },
	{"SendUDP", &LuaManager::SendUDPPacket},
	{"PrintDebug", &LuaManager::GenerateDebugMessage},
	{"GetClientIp", &LuaManager::GetClientIp},
	{"GetHubIpPort", &LuaManager::GetHubIpPort},
	{"GetHubUrl", &LuaManager::GetHubUrl},
	{"InjectHubMessage", &LuaManager::InjectHubMessage},
	{"InjectHubMessageADC", &LuaManager::InjectHubMessage},
	{"CreateClient", &LuaManager::CreateClient},
	{"DeleteClient", &LuaManager::DeleteClient},
	{"RunTimer", &LuaManager::RunTimer},
	{"GetSetting", &LuaManager::GetSetting},
	{"ToUtf8", &LuaManager::ToUtf8},
	{"FromUtf8", &LuaManager::FromUtf8},
	{"GetAppPath", &LuaManager::GetAppPath},
	{"GetConfigPath", &LuaManager::GetConfigPath},
	{"GetScriptPath", &LuaManager::GetScriptPath},
	{"DropUserConnection", &LuaManager::DropUserConnection},

	{"HubWindowAttention", &LuaManager::NotImpl},
	{"FindWindowHandle", &LuaManager::NotImpl},
	{"SendWindowMessage", &LuaManager::NotImpl},
	{0}
};

int LuaManager::SendHubMessage(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		LuaPlugin::getInstance()->sendHubCommand(reinterpret_cast<HubDataPtr>(lua_touserdata(L, -2)), lua_tostring(L, -1));
	}

	return 0;
}

int LuaManager::SendClientMessage(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		LuaPlugin::getInstance()->sendClientCommand(static_cast<ConnectionDataPtr>(lua_touserdata(L, -2)), lua_tostring(L, -1));
	}

	return 0;
}

int LuaManager::SendUDPPacket(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		// TODO: ipv6 awareness
		string dest = lua_tostring(L, -2);
		string::size_type i = dest.find(":");
		LuaPlugin::getInstance()->sendUdpPacket(dest.substr(0, i-1).c_str(), static_cast<uint32_t>(atoi(dest.substr(i+1).c_str())), lua_tostring(L, -1));
	}

	return 0;
}

int LuaManager::GenerateDebugMessage(lua_State* L) {
	/* arguments: string */
	if (lua_gettop(L) == 1 && lua_isstring(L, -1))
		Util::logMessage(lua_tostring(L, -1));

	return 0;
}

int LuaManager::GetClientIp(lua_State* L) {
	/* arguments: client */
	ConnectionData* hConn = (ConnectionDataPtr)lua_touserdata(L, 1);
	if(hConn == NULL) {
		LuaError(L, "GetClientIpPort: missing client pointer");
		return 0;
	}

	lua_pushstring(L, hConn->ip);
	return 1;
}

int LuaManager::GetHubIpPort(lua_State* L) {
	/* arguments: client */
	HubDataPtr client = reinterpret_cast<HubDataPtr>(lua_touserdata(L, 1));
	if(client == NULL) {
		LuaError(L, "GetHubIpPort: missing hub pointer");
		return 0;
	}

	// TODO: ipv6 awareness
	lua_pushstring(L, (string(client->ip) + ":" + Util::toString(client->port)).c_str());
	return 1;
}

int LuaManager::GetHubUrl(lua_State* L) {
	/* arguments: client */
	HubDataPtr client = reinterpret_cast<HubDataPtr>(lua_touserdata(L, 1));
	if(client == NULL) {
		LuaError(L, "GetHubUrl: missing hub pointer");
		return 0;
	}

	lua_pushstring(L, client->url);
	return 1;
}

int LuaManager::InjectHubMessage(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		string message = string(lua_tostring(L, -1));
		if(message.find("$UserCommand") != string::npos) {
			message = Util::ucLuaConvert(message, false);
		} else if(message.find("CMD") != string::npos) {
			message = Util::ucLuaConvert(message, true);
		}
		LuaPlugin::getInstance()->injectHubCommand(reinterpret_cast<HubDataPtr>(lua_touserdata(L, -2)), message.c_str());
	}

	return 0;
}

int LuaManager::CreateClient(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)){
		HubDataPtr client = NULL;
		if((client = LuaPlugin::getInstance()->createHub(lua_tostring(L, -2), lua_tostring(L, -1), "")) != NULL) {
			lua_pushlightuserdata(L, client);
			return 1;
		}

		LuaError(L, "CreateClient: hub creation failed, check settings (nick)");
	}

	return 0;
}

int LuaManager::DeleteClient(lua_State* L) {
	if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)){
		LuaPlugin::getInstance()->destroyHub(reinterpret_cast<HubDataPtr>(lua_touserdata(L, -1)));
	}

	return 0;
}

int LuaManager::RunTimer(lua_State* L) {
	/* arguments: bool: on/off */
	if(lua_gettop(L) == 1 && lua_isnumber(L, -1)) {
		LuaPlugin::getInstance()->setTimer(lua_tonumber(L, 1) != 0);
		return 1;
	}

	LuaError(L, "RunTimer: missing integer (0=off,!0=on)");
	return 0;
}

int LuaManager::GetSetting(lua_State* L) {
	/* arguments: string */
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		auto setting = Util::getCoreConfig(lua_tostring(L, -1));
		switch(setting->type) {
			case CFG_TYPE_STRING: {
				auto str = (ConfigStrPtr)setting;
				lua_pushstring(L, str->value);
				break;
			}
			case CFG_TYPE_INT: {
				auto num = (ConfigIntPtr)setting;
				lua_pushnumber(L, num->value);
				break;
			}
			case CFG_TYPE_INT64: {
				auto num = (ConfigInt64Ptr)setting;
				lua_pushnumber(L, static_cast<lua_Number>(num->value));
				break;
			}
			default:
				Util::freeCoreConfig(setting);
				LuaError(L, "GetSetting: setting not found", false);
				return 0;
		}

		Util::freeCoreConfig(setting);
		return 1;
	}

	LuaError(L, "GetSetting: invalid arguments");
	return 0;
}

int LuaManager::ToUtf8(lua_State* L) {
	/* arguments: string */
	if(lua_gettop(L) == 1 && lua_isstring(L, -1) ) {
		lua_pushstring(L, Util::toUtf8(lua_tostring(L, -1)).c_str());
		return 1;
	}

	LuaError(L, "ToUtf8: string needed as argument");
	return 0;
}

int LuaManager::FromUtf8(lua_State* L) {
	/* arguments: string */
	if(lua_gettop(L) == 1 && lua_isstring(L, -1) ) {
		lua_pushstring(L, Util::fromUtf8(lua_tostring(L, -1)).c_str());
		return 1;
	}

	LuaError(L, "FromUtf8: string needed as argument");
	return 0;
}

int LuaManager::GetAppPath(lua_State* L) {
	lua_pushstring(L, Util::fromUtf8(Util::getPath(PATH_RESOURCES)).c_str());
	return 1;
}

int LuaManager::GetConfigPath(lua_State* L) {
	lua_pushstring(L, Util::fromUtf8(Util::getPath(PATH_USER_CONFIG)).c_str());
	return 1;
}

int LuaManager::GetScriptPath(lua_State* L) {
	lua_pushstring(L, Util::fromUtf8(SETTING(ScriptPath)).c_str());
	return 1;
}

int LuaManager::DropUserConnection(lua_State* L) {
	/* arguments: userconnection to drop */
	if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
		LuaPlugin::getInstance()->dropClientConnection(static_cast<ConnectionDataPtr>(lua_touserdata(L, -1)), False);
	}

	return 0;
}

void LuaManager::LuaError(lua_State* L, const char* fmt, bool fatal /*= true*/, ...) {
	if(fatal || BOOLSETTING(LuaDebug)) {
		va_list argp;
		va_start(argp, fatal);
		luaL_error(L, fmt, argp);
		va_end(argp);
	}
}