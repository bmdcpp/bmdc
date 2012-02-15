/*
 * Copyright (C) 2012 Jacek Sieka, arnetheduck on gmail point com
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
#include "Plugin.h"

#ifdef _WIN32
HINSTANCE hInst = NULL;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID lpReserved) {
	UNREFERENCED_PARAMETER(reason);
	UNREFERENCED_PARAMETER(lpReserved);
	hInst = (HINSTANCE)hModule;
	return TRUE;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/* Plugin loader */
	DCEXP DCHOOK DCAPI pluginInit(MetaDataPtr info) {
		info->name = PLUGIN_NAME;
		info->author = PLUGIN_AUTHOR;
		info->description = PLUGIN_DESC;
		info->version = PLUGIN_VERSION;
		info->web = PLUGIN_WEB;
		info->apiVersion = DCAPI_VER;
		info->guid = PLUGIN_GUID;
		/* Modify icon if you wish to change it */

		return &pluginProc;
	}

#ifdef __cplusplus
}
#endif