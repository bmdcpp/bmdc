/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#include "ADLSearch.h"
#include "ClientManager.h"
#include "ConnectionManager.h"
#include "ConnectivityManager.h"
#include "CryptoManager.h"
#include "DownloadManager.h"
#include "FavoriteManager.h"
#include "FinishedManager.h"
#include "GeoManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "MappingManager.h"
#include "QueueManager.h"
#include "SearchManager.h"
#include "SettingsManager.h"
#include "ShareManager.h"
#include "ThrottleManager.h"
#include "UploadManager.h"

#include "DetectionManager.h"
#include "RawManager.h"
#include "CalcADLAction.h"
#include "PluginApiImpl.h"
#ifdef HAVE_LIBTAR
	#include "BackupManager.h"
	#include "ExportManager.h"
#endif
#include "DebugManager.h"
#include "HighlightManager.h"
#if 0
#include "PluginManager.h"
#endif
#include "BMDCUtil.h"

#include "AVManager.h"

#include "UserManager.h"

#include "format.h"

#ifdef _WIN32
extern "C" int _nl_msg_cat_cntr;
#endif

namespace dcpp {

void startup() {
#ifdef _WIN32
	// "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
	// Nev's great contribution to dc++
	while(1) break;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	
	SettingsManager::newInstance();
	TimerManager::newInstance();
	LogManager::newInstance();
	HashManager::newInstance();
	CryptoManager::newInstance();
	DebugManager::newInstance();
	SearchManager::newInstance();
	UsersManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	ThrottleManager::newInstance();
	RawManager::newInstance();//
	CalcADLAction::newInstance();//
	QueueManager::newInstance();
	ShareManager::newInstance();
	FavoriteManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();
	ConnectivityManager::newInstance();
	MappingManager::newInstance();
	GeoManager::newInstance();
#if 0	
	PluginManager::newInstance();
    PluginApiImpl::init();
#endif    
#ifdef HAVE_LIBTAR
	ExportManager::newInstance();
#endif
	DetectionManager::newInstance();
	HighlightManager::newInstance();
#ifdef HAVE_LIBTAR
	BackupManager::newInstance();
	RestoreManager::newInstance();
#endif
	AVManager::newInstance();
}
void load(function<void (const string&)> stepF, function<void (float)> progressF){

	SettingsManager::getInstance()->load();

#ifdef _WIN32
	if(!SETTING(LANGUAGE).empty()) {
		string language = "LANGUAGE=" + SETTING(LANGUAGE);
		putenv(language.c_str());

		// Apparently this is supposted to make gettext reload the message catalog...
		_nl_msg_cat_cntr++;
	}
#endif

	auto announce = [&stepF](const string& str) {
		if(stepF) {
			stepF(str);
		}
	};

	announce(_("Users"));
	FavoriteManager::getInstance()->load();

	announce(_("Security certificates"));
	CryptoManager::getInstance()->loadCertificates();

	announce(_("Hash database"));
	HashManager::getInstance()->startup(progressF);

	announce(_("Shared Files"));
	ShareManager::getInstance()->refresh(true, false, true,progressF);

	announce(_("Download Queue"));
	QueueManager::getInstance()->loadQueue();
#if 0
	PluginManager::getInstance()->loadPlugins(stepF);
#endif
	if(SETTING(GET_USER_COUNTRY)) {
		announce(_("Country information"));
		GeoManager::getInstance()->init();
	}

	announce(_("Detections"));
	DetectionManager::getInstance()->load();

	bmUtil::init();
#ifdef HAVE_LIBTAR
	if(SETTING(ENABLE_AUTOBACKUP)) {
		BackupManager::getInstance()->createBackup();
	}
#endif
}

void shutdown() {
#if 0	
    PluginApiImpl::shutdown();
#endif
#ifdef HAVE_LIBTAR
	ExportManager::deleteInstance();
	RestoreManager::deleteInstance();
	BackupManager::deleteInstance();
#endif
#if 0	
	PluginManager::getInstance()->unloadPlugins();
#endif
	TimerManager::getInstance()->shutdown();
	ThrottleManager::getInstance()->shutdown();//..

	HashManager::getInstance()->shutdown();

	ConnectionManager::getInstance()->shutdown();
	MappingManager::getInstance()->close();
	GeoManager::getInstance()->close();
	BufferedSocket::waitShutdown();

	QueueManager::getInstance()->saveQueue(true);
	SettingsManager::getInstance()->save();


	AVManager::deleteInstance();
	HighlightManager::deleteInstance();
	DetectionManager::deleteInstance();
	#if 0
	PluginManager::deleteInstance();//
	#endif
	GeoManager::deleteInstance();
	MappingManager::deleteInstance();
	ConnectivityManager::deleteInstance();
	ADLSearchManager::deleteInstance();
	CalcADLAction::deleteInstance();//
	RawManager::deleteInstance();//.
	FinishedManager::deleteInstance();
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	ThrottleManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	QueueManager::deleteInstance();
	ConnectionManager::deleteInstance();
	SearchManager::deleteInstance();
	FavoriteManager::deleteInstance();
	ClientManager::deleteInstance();
	UsersManager::deleteInstance();//
	DebugManager::deleteInstance();
	HashManager::deleteInstance();
	LogManager::deleteInstance();
	SettingsManager::deleteInstance();
	TimerManager::deleteInstance();

#ifdef _WIN32
	::WSACleanup();
#endif
}

} // namespace dcpp
