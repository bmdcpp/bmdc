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

// Plugin main function
Bool DCAPI pluginMain(PluginState state, DCCorePtr core, dcptr_t /*pData*/) {
	switch(state) {
		case ON_INSTALL:
		case ON_LOAD: {
			Bool res = True;
			LuaPlugin::newInstance();
			LuaPlugin::getInstance()->onLoad(core, (state == ON_INSTALL), res);
			return res;
		}

		case ON_UNINSTALL:
		case ON_UNLOAD: {
			LuaPlugin::deleteInstance();
			return True;
		}

		default:
			return False;
	}
}

extern "C" {

	// Plugin loader
	DCEXP DCMAIN DCAPI pluginInit(MetaDataPtr info) {
		info->name = PLUGIN_NAME;
		info->author = PLUGIN_AUTHOR;
		info->description = PLUGIN_DESC;
		info->web = PLUGIN_WEB;
		info->version = PLUGIN_VERSION;
		info->apiVersion = DCAPI_CORE_VER;
		info->guid = PLUGIN_GUID;

		return &pluginMain;
	}

}

#ifdef _WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD /*reason*/, LPVOID /*lpReserved*/) {
	return TRUE;
}
#endif