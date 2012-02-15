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

/**
 * The PluginApiImpl class contains implementations of certain callback functions,
 * they are simply separated here.
 */

#ifndef DCPLUSPLUS_DCPP_PLUGIN_API_IMPL_H
#define DCPLUSPLUS_DCPP_PLUGIN_API_IMPL_H

namespace dcpp {

class PluginApiImpl
{
public:
	static hookHandle initAPI(DCCore& dcCore);
	static void releaseAPI(hookHandle hHook);

private:
	// Functions for DCCore
	static hookHandle DCAPI createHook(HookID hookId, HookType hookType, DCHOOK defProc);
	static void DCAPI destroyHook(hookHandle hHook);

	static subsHandle DCAPI setHook(HookID hookId, DCHOOK hookProc, void* pCommon);
	static Bool DCAPI callHook(HookID hookId, uint32_t eventId, dcptr_t pData);
	static size_t DCAPI unHook(subsHandle hHook);

	static uint32_t DCAPI registerMessage(HookType type, const char* name);
	static uint32_t DCAPI registerRange(HookType type, const char* name, uint32_t count);
	static uint32_t DCAPI seekMessage(const char* name);

	static void DCAPI setConfig(const char* guid, const char* setting, ConfigValuePtr val);
	static Bool DCAPI getConfig(const char* guid, const char* setting, ConfigValuePtr val);

	static void* DCAPI memalloc(void* ptr, size_t bytes);
	static size_t DCAPI strconv(ConversionType type, void* dst, void* src, size_t len);

	// Default callback for hook CALLBACK_BASE
	static Bool DCAPI coreCallback(uint32_t eventId, dcptr_t pData);

	// Queue functions
	static Bool addDownload(const string& fname, int64_t fsize, const string& fhash, QueueDataPtr data);

	// Hub functions
	static Bool findOnlineHub(string hubUrl, ClientDataPtr data);
	static Bool newClient(const string& hubUrl, ClientDataPtr data);
	static void sendHubMessage(Client* client, const string& message, bool thirdPerson);
	static void sendPrivateMessage(OnlineUserPtr ou, const string& message, bool thirdPerson);

	static Socket apiSocket;
};

} // namepsace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_PLUGIN_API_H)

/**
 * @file
 * $Id: PluginApiImpl.h 707 2010-09-03 15:40:16Z crise $
 */
