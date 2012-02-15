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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "PluginManager.h"
#include "ConnectionManager.h"
#include "FavoriteManager.h"

#include "AdcHub.h"
#include "UserConnection.h"

namespace dcpp {

Socket PluginApiImpl::apiSocket(Socket::TYPE_UDP);

hookHandle PluginApiImpl::initAPI(DCCore& dcCore) {
	dcCore.apiVersion = DCAPI_VER;

	// Hook creation
	dcCore.create_hook = &PluginApiImpl::createHook;
	dcCore.destroy_hook = &PluginApiImpl::destroyHook;

	// Hook interaction
	dcCore.set_hook = &PluginApiImpl::setHook;
	dcCore.call_hook = &PluginApiImpl::callHook;
	dcCore.un_hook = &PluginApiImpl::unHook;

	// Message regitster
	dcCore.register_message = &PluginApiImpl::registerMessage;
	dcCore.register_range = &PluginApiImpl::registerRange;
	dcCore.seek_message = &PluginApiImpl::seekMessage;

	// Setting management
	dcCore.set_cfg = &PluginApiImpl::setConfig;
	dcCore.get_cfg = &PluginApiImpl::getConfig;

	dcCore.memalloc = &PluginApiImpl::memalloc;
	dcCore.strconv = &PluginApiImpl::strconv;

	return PluginManager::getInstance()->createHook(CALLBACK_BASE, HOOK_CALLBACK, &PluginApiImpl::coreCallback);
}

void PluginApiImpl::releaseAPI(hookHandle hHook) {
	PluginManager::getInstance()->destroyHook(hHook);
	apiSocket.disconnect();
}

// Functions for DCCore
hookHandle PluginApiImpl::createHook(HookID hookId, HookType hookType, DCHOOK defProc) {
	return PluginManager::getInstance()->createHook(hookId, hookType, defProc);
}

void PluginApiImpl::destroyHook(hookHandle hHook) {
	PluginManager::getInstance()->destroyHook(hHook);
	hHook = NULL;
}

subsHandle PluginApiImpl::setHook(HookID hookId, DCHOOK hookProc, void* pCommon) {
	return PluginManager::getInstance()->setHook(hookId, hookProc, pCommon);
}

Bool PluginApiImpl::callHook(HookID hookId, uint32_t eventId, dcptr_t pData) {
	return PluginManager::getInstance()->callHook(hookId, eventId, pData) ? True : False;
}

size_t PluginApiImpl::unHook(subsHandle hHook) {
	return PluginManager::getInstance()->unHook(hHook);
}

uint32_t PluginApiImpl::registerMessage(HookType type, const char* name) {
	return PluginManager::getInstance()->registerMessage(type, name);
}

uint32_t PluginApiImpl::registerRange(HookType type, const char* name, uint32_t count) {
	return PluginManager::getInstance()->registerRange(type, name, count);
}

uint32_t PluginApiImpl::seekMessage(const char* name) {
	return PluginManager::getInstance()->seekMessage(name);
}

void PluginApiImpl::setConfig(const char* guid, const char* setting, ConfigValuePtr val) {
	PluginManager* pm = PluginManager::getInstance();
	switch(val->type) {
		case CFG_TYPE_REMOVE:
			pm->removeSetting(guid, setting);
			break;
		case CFG_TYPE_STRING:
			if(val->value.str)
				pm->setSetting(guid, setting, val->value.str);
			break;
		case CFG_TYPE_INT:
			pm->setSetting(guid, setting, Util::toString(val->value.int32));
			break;
		case CFG_TYPE_INT64:
			pm->setSetting(guid, setting, Util::toString(val->value.int64));
			break;
		default:
			dcassert(0);
	}
}

Bool PluginApiImpl::getConfig(const char* guid, const char* setting, ConfigValuePtr val) {
	// Attempt to retrieve core setting...
	if(strcmp(guid, "CoreSetup") == 0) {
		int n;
		SettingsManager::Types type;
		if(SettingsManager::getInstance()->getType(setting, n, type)) {
			switch(type) {
				case SettingsManager::TYPE_STRING:
					val->value.str = SettingsManager::getInstance()->get(static_cast<SettingsManager::StrSetting>(n)).c_str();
					break;
				case SettingsManager::TYPE_INT:
					val->value.int32 = SettingsManager::getInstance()->get(static_cast<SettingsManager::IntSetting>(n));
					break;
				case SettingsManager::TYPE_INT64:
					val->value.int64 = SettingsManager::getInstance()->get(static_cast<SettingsManager::Int64Setting>(n));
					break;
				default:
					dcassert(0);
			}
			val->type = static_cast<ConfigType>(type);
			return True;
		}
	}

	PluginManager* pm = PluginManager::getInstance();
	switch(val->type) {
		case CFG_TYPE_STRING:
			val->value.str = pm->getSetting(guid, setting).c_str();
			break;
		case CFG_TYPE_INT:
			val->value.int32 = Util::toInt(pm->getSetting(guid, setting));
			break;
		case CFG_TYPE_INT64:
			val->value.int64 = Util::toInt64(pm->getSetting(guid, setting));
			break;
		default:
			dcassert(0);
	}
	return True;
}

void* PluginApiImpl::memalloc(void* ptr, size_t bytes) {
	if(ptr != NULL && bytes > 0) {
		return realloc(ptr, bytes);
	} else if(ptr != NULL) {
		free(ptr);
	} else if(bytes > 0) {
		return malloc(bytes);
	}
	return NULL;
}

size_t PluginApiImpl::strconv(ConversionType type, void* dst, void* src, size_t len) {
	switch(type) {
		case CONV_TO_UTF8: {
			string sSrc(Text::toUtf8(reinterpret_cast<char*>(src)));
			char* cDst = static_cast<char*>(dst);
			len = (sSrc.size() < len) ? sSrc.size() : len;
			strncpy(cDst, sSrc.c_str(), len);
			return len;
		}
		case CONV_FROM_UTF8: {
			string sSrc(Text::fromUtf8(reinterpret_cast<char*>(src)));
			char* cDst = static_cast<char*>(dst);
			len = (sSrc.size() < len) ? sSrc.size() : len;
			strncpy(cDst, sSrc.c_str(), len);
			return len;
		}
		case CONV_UTF8_TO_WIDE: {
			wstring sSrc(Text::utf8ToWide(reinterpret_cast<char*>(src)));
			wchar_t* cDst = static_cast<wchar_t*>(dst);
			len = (sSrc.size() < len) ? sSrc.size() : len;
			wcsncpy(cDst, sSrc.c_str(), len);
			return len;
		}
		case CONV_WIDE_TO_UTF8: {
			string sSrc(Text::wideToUtf8(reinterpret_cast<wchar_t*>(src)));
			char* cDst = static_cast<char*>(dst);
			len = (sSrc.size() < len) ? sSrc.size() : len;
			strncpy(cDst, sSrc.c_str(), len);
			return len;
		}
		case CONV_TO_BASE32: {
			string sSrc(Encoder::toBase32(reinterpret_cast<uint8_t*>(src), len));
			char* cDst = static_cast<char*>(dst);
			len = (sSrc.size() < len) ? sSrc.size() : len;
			strncpy(cDst, sSrc.c_str(), len);
			return len;
		}
		case CONV_FROM_BASE32:
			Encoder::fromBase32(reinterpret_cast<char*>(src), reinterpret_cast<uint8_t*>(dst), len);
			return 0;
		default:
			// No value is better than the other
			return string::npos;
	}
}

// Default callback for hook CALLBACK_BASE
Bool PluginApiImpl::coreCallback(uint32_t eventId, dcptr_t pData) {
	switch(eventId) {
		case DBG_MESSAGE:
		case LOG_MESSAGE:
			LogManager::getInstance()->message(reinterpret_cast<const char*>(pData));
			break;
		case GET_PATHS: {
			TextDataCondPtr args = reinterpret_cast<TextDataCondPtr>(pData);
			args->res = const_cast<char*>(Util::getPath(static_cast<Util::Paths>(args->cond)).c_str());
			break;
		}
		case PROTOCOL_SEND_UDP: {
			TextDataCondPtr args = reinterpret_cast<TextDataCondPtr>(pData);
			try { apiSocket.writeTo(args->data, Util::toString(args->cond), args->res); }
			catch(const Exception& e) { dcdebug("CoreCallback, PROTOCOL_SEND_UDP: %s\n", e.getError().c_str()); }
			break;
		}
		case PROTOCOL_HUB_EMULATE_CMD: {
			TextArgsResPtr args = reinterpret_cast<TextArgsResPtr>(pData);
			if(args->object) reinterpret_cast<Client*>(args->object)->emulateCommand(args->data);
			break;
		}
		case PROTOCOL_HUB_SEND_CMD: {
			TextArgsResPtr args = reinterpret_cast<TextArgsResPtr>(pData);
			if(args->object) reinterpret_cast<Client*>(args->object)->send(args->data);
			break;
		}
		case PROTOCOL_CONN_SEND_CMD: {
			TextArgsResPtr args = reinterpret_cast<TextArgsResPtr>(pData);
			if(args->object) reinterpret_cast<UserConnection*>(args->object)->sendRaw(args->data);
			break;
		}
		case PROTOCOL_CONN_TERMINATE: {
			TextArgsCondPtr args = reinterpret_cast<TextArgsCondPtr>(pData);
			if(args->object) reinterpret_cast<UserConnection*>(args->object)->disconnect(args->cond != False);
			break;
		}
		case HUBS_CREATE_HUB: {
			TextArgsResPtr args = reinterpret_cast<TextArgsResPtr>(pData);
			return newClient(args->data, reinterpret_cast<ClientDataPtr>(args->res));
		}
		case HUBS_DESTROY_HUB:
			if(pData) ClientManager::getInstance()->putClient(reinterpret_cast<Client*>(pData));
			break;
		case HUBS_FIND_HUB: {
			TextArgsResPtr args = reinterpret_cast<TextArgsResPtr>(pData);
			return findOnlineHub(args->data, reinterpret_cast<ClientDataPtr>(args->res));
		}
		case HUBS_SEND_CHAT: {
			TextArgsCondPtr args = reinterpret_cast<TextArgsCondPtr>(pData);
			sendHubMessage(reinterpret_cast<Client*>(args->object), args->data, args->cond != False);
			break;
		}
		case HUBS_SEND_PM: {
			TextArgsCondPtr args = reinterpret_cast<TextArgsCondPtr>(pData);
			sendPrivateMessage(reinterpret_cast<OnlineUser*>(args->object), args->data, args->cond != False);
			break;
		}
		case HUBS_SEND_LOCAL: {
			TextArgsCondPtr args = reinterpret_cast<TextArgsCondPtr>(pData);
			Client* client = reinterpret_cast<Client*>(args->object);
			if(client) client->fire(ClientListener::ClientLine(), client, args->data, args->cond);
			break;
		}
		case QUEUE_ADD_DL: {
			QueueArgsPtr args = reinterpret_cast<QueueArgsPtr>(pData);
			return addDownload(args->target, args->size, args->hash, args->res);
		}
		case QUEUE_REMOVE_DL:
			QueueManager::getInstance()->remove(reinterpret_cast<const char*>(pData));
			break;
		case QUEUE_SET_PRIORITY: {
			TextArgsCondPtr args = reinterpret_cast<TextArgsCondPtr>(pData);
			if(args->object) reinterpret_cast<QueueItem*>(args->object)->setPriority(static_cast<QueueItem::Priority>(args->cond));
			break;
		}
		default:
			return False;
	}
	return True;
}

// Queue functions
Bool PluginApiImpl::addDownload(const string& fname, int64_t fsize, const string& fhash, QueueDataPtr data) {
	Bool bRes = False;

	try {
		string target = File::isAbsolute(fname) ? fname : SETTING(DOWNLOAD_DIRECTORY) + fname;
		QueueManager::getInstance()->add(target, fsize, TTHValue(fhash), HintedUser(UserPtr(), Util::emptyString));

		QueueManager::getInstance()->lockedOperation([data, &target, &bRes](const QueueItem::StringMap& lst) {
			QueueItem::StringMap::const_iterator i;
			if ((i = lst.find(&target)) != lst.end()) {
				QueueItem* qi = i->second;
				data->file = qi->getTargetFileName().c_str();
				data->target = qi->getTarget().c_str();
				data->location = qi->getTempTarget().c_str();
				data->hash = qi->getTTH().data;
				data->object = qi;
				data->size = qi->getSize();
				data->isFileList = qi->isSet(QueueItem::FLAG_USER_LIST) ? True : False;
				bRes = True;
			}
		});
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}

	return bRes;
}

// Hub functions
Bool PluginApiImpl::findOnlineHub(string hubUrl, ClientDataPtr data) {
	auto& clients = ClientManager::getInstance()->getClients();

	for(auto i = clients.begin(); i != clients.end(); ++i) {
		if(((*i)->getHubUrl() == hubUrl) && (*i)->isConnected()) {
			Client* client = *i;
			data->url = client->getHubUrl().c_str();
			data->ip = client->getIp().c_str();
			data->object = client;
			Util::toString(data->port) = client->getPort();
			data->protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
			data->isOp = client->isOp() ? True : False;
			data->isSecure = client->isSecure() ? True : False;
			return True;
		}
	}

	return False;
}

Bool PluginApiImpl::newClient(const string& hubUrl, ClientDataPtr data) {
	if(!SETTING(NICK).empty()) {
		// Note, it is no use specifying any other info here...
		Client* client = ClientManager::getInstance()->getClient(hubUrl);
		client->setPassword(Util::emptyString);
		client->connect();

		data->url = client->getHubUrl().c_str();
		data->ip = client->getIp().c_str();
		data->object = client;
		Util::toString(data->port) = client->getPort();
		data->protocol = dynamic_cast<AdcHub*>(client) ? PROTOCOL_ADC : PROTOCOL_NMDC;
		data->isOp = False;
		data->isSecure = client->isSecure() ? True : False;
		return True;
	}

	return False;
}

void PluginApiImpl::sendHubMessage(Client* client, const string& message, bool thirdPerson) {
	if(client) {
		// Lets make plugins life easier...
		ParamMap params;
		client->getMyIdentity().getParams(params, "my", true);
		client->getHubIdentity().getParams(params, "hub", false);

		client->hubMessage(Util::formatParams(message, params, false), thirdPerson);
	}
}

void PluginApiImpl::sendPrivateMessage(OnlineUserPtr ou, const string& message, bool thirdPerson) {
	if(ou) {
		// Lets make plugins life easier...
		ParamMap params;
		ou->getIdentity().getParams(params, "user", true);
		ou->getClient().getMyIdentity().getParams(params, "my", true);
		ou->getClient().getHubIdentity().getParams(params, "hub", false);

		ou->getClient().privateMessage(*ou, Util::formatParams(message, params, false), thirdPerson);
	}
}

} // namespace dcpp

/**
 * @file
 * $Id: PluginApiImpl.cpp 722 2010-10-09 11:51:36Z crise $
 */
