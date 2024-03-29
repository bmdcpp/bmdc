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
#include "Mapper_MiniUPnPc.h"

#include "debug.h"

extern "C" {
#ifndef MINIUPNP_STATICLIB
#define MINIUPNP_STATICLIB
#endif
//#define HAVE_LOCAL_MINIUPNPC 1
#ifdef HAVE_LOCAL_MINIUPNPC
#include "../miniupnpc/miniupnpc.h"
#include "../miniupnpc/upnpcommands.h"
#else
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#endif
}

namespace dcpp {

const string Mapper_MiniUPnPc::name = "MiniUPnP";

Mapper_MiniUPnPc::Mapper_MiniUPnPc(string&& localIp, bool v6) :
Mapper(move(localIp), v6)
{
}

bool Mapper_MiniUPnPc::init() {
	if(!url.empty())
		return true;
	
	UPNPDev* devices = upnpDiscover(2000, localIp.empty() ? nullptr : localIp.c_str(), nullptr, 0, v6, 2, nullptr);
	
	if(!devices)
		return false;

	UPNPUrls urls;
	IGDdatas data;

	int ret = UPNP_GetValidIGD(devices, &urls, &data, 0, 0);

	bool ok = ret != 0;
	if(ok) {
		url = urls.controlURL;
		service = data.first.servicetype;
		device = data.cureltname;
	}

	if(ret > 0) {
		FreeUPNPUrls(&urls);
		freeUPNPDevlist(devices);
	}

	return ok;
}

void Mapper_MiniUPnPc::uninit() {
}

bool Mapper_MiniUPnPc::add(const string& port, const Protocol protocol, const string& description) {
	dcdebug("\nMINIUPPNC income %s , %s\n",port.c_str() ,protocols[protocol] );
	
	int x = UPNP_AddPortMapping(url.c_str(), service.c_str(), port.c_str(), port.c_str(),
		localIp.c_str(), description.c_str(), protocols[protocol], 0, 0);
		dcdebug("\nMINIUPPNC %d\n",x );
	return (x == UPNPCOMMAND_SUCCESS);
}

bool Mapper_MiniUPnPc::remove(const string& port, const Protocol protocol) {
	return UPNP_DeletePortMapping(url.c_str(), service.c_str(), port.c_str(), protocols[protocol], 0) == UPNPCOMMAND_SUCCESS;
}

string Mapper_MiniUPnPc::getDeviceName() {
	return device;
}

string Mapper_MiniUPnPc::getExternalIP() {
	char buf[16] = { 0 };
	if(UPNP_GetExternalIPAddress(url.c_str(), service.c_str(), buf) == UPNPCOMMAND_SUCCESS)
		return buf;
	return string();
}

} // dcpp namespace
