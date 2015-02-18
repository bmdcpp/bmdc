/*
 * FakeChecker.cpp
 * This file is part of BMDC++ 
 *
 * Copyright (C) 2012 - 2015 - Mank
 *
 * BMDC++ is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * BMDC++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
#include "stdinc.h"
#include "Client.h"
#include "ClientManager.h"
#include "QueueManager.h"
#include "OnlineUser.h"
#include "SettingsManager.h"
#include "ADLSearch.h"
#include "FakeChecker.h"

namespace dcpp {


void FakeChecker::setCheating(const HintedUser& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete) {

	OnlineUser* ou = nullptr;
	string report;
	{
		cm->lock();
		ou = cm->findOnlineUser(p);
		if(!ou)
			return;

		if(!_ccResponse.empty()) {
			ou->getIdentity().setTestSURComplete("1");
			ou->getIdentity().set("TS", _ccResponse);
			report = ou->getIdentity().updateClientType(*ou);
		}
		if(_clientCheckComplete)
			ou->getIdentity().setTestSURComplete("1");
		if(_fileListCheckComplete)
			ou->getIdentity().setFileListComplete("1");
		if(!_cheatString.empty())
			report += ou->setCheat(_cheatString, _badClient, _badFileList, _displayCheat);
		cm->sendAction(*ou, _actionId);
	}

	if(ou) {
		ou->getClient().updated(*ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}

void FakeChecker::fileListDisconnected(const HintedUser& p) {
	if(SETTING(MAX_DISCONNECTS) == 0)
		return;

	bool remove = false;
	string report = Util::emptyString;
	Client* c = nullptr;
	{
		cm->lock();
		OnlineUser* ou = cm->findOnlineUser(p);
		if(!ou) return;

		int fileListDisconnects = Util::toInt(ou->getIdentity().get("FD")) + 1;
		ou->getIdentity().set("FD", Util::toString(fileListDisconnects));

		if(fileListDisconnects == SETTING(MAX_DISCONNECTS)) {
			c = &ou->getClient();
			report += ou->setCheat("Disconnected file list %[userFD] times", false, true, SETTING(SHOW_DISCONNECT));
			if(ou->getIdentity().isFileListQueued()) {
				ou->getIdentity().setFileListComplete("1");
				ou->getIdentity().setFileListQueued("0");
				remove = true;
			}
			cm->sendAction(*ou, SETTING(DISCONNECT_RAW));
		}
	}

	if(remove) {
		try {
			QueueManager::getInstance()->removeFileListCheck(p);
		} catch (...) {
			//...
		}
	}
	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

void FakeChecker::setUnknownCommand(const HintedUser& p, const string& aUnknownCommand) {
	cm->lock();
	OnlineUser* ou = cm->findOnlineUser(p);
	if(!ou)
		return;
	ou->getIdentity().set("UC", aUnknownCommand);
}

void FakeChecker::setListLength(const HintedUser& p, const string& listLen) {
	cm->lock();
	OnlineUser* ou = cm->findOnlineUser(p);
	if(ou) {
		ou->getIdentity().set("LL", listLen);
	}
}

void FakeChecker::setListSize(const HintedUser& p, int64_t aFileLength, bool adc) {
	OnlineUser* ou = nullptr;
	string report;
	{
		cm->lock();
		ou = cm->findOnlineUser(p);
		if(!ou)
			return;

		ou->getIdentity().set("LS", Util::toString(aFileLength));

		if(ou->getIdentity().getBytesShared() > 0) {
			if((SETTING(MAX_FILELIST_SIZE) > 0) && (aFileLength > SETTING(MAX_FILELIST_SIZE)) && SETTING(FILELIST_TOO_SMALL_BIG)) {
				report = ou->setCheat("Too large filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, SETTING(FILELIST_TOO_SMALL_BIG));
				cm->sendAction(*ou, SETTING(FILELIST_TOO_SMALL_BIG_RAW));
			} else if((aFileLength < SETTING(MIN_FL_SIZE) && SETTING(FILELIST_TOO_SMALL_BIG)) || (aFileLength < 100)) {
				report = ou->setCheat("Too small filelist - %[userLSshort] for the specified share of %[userSSshort]", false, true, SETTING(FILELIST_TOO_SMALL_BIG));
				cm->sendAction(*ou, SETTING(FILELIST_TOO_SMALL_BIG_RAW));
			}
		} else if(!adc) {
			int64_t listLength = (!ou->getIdentity().get("LL").empty()) ? Util::toInt64(ou->getIdentity().get("LL")) : -1;
			if((listLength != -1) && (listLength * 3 < aFileLength) && (ou->getIdentity().getBytesShared() > 0)) {
				report = ou->setCheat("Fake file list - ListLen = %[userLL], FileLength = %[userLS]", false, true, SETTING(LISTLEN_MISMATCH_SHOW));
				cm->sendAction(*ou, SETTING(LISTLEN_MISMATCH));
			}
		}
	}

	if(ou) {
		ou->getClient().updated(*ou);
		if(!report.empty())
			ou->getClient().cheatMessage(report);
	}
}

void FakeChecker::setPkLock(const HintedUser& p, const string& aPk, const string& aLock) {
	cm->lock();
	OnlineUser* ou = cm->findOnlineUser(p);
	if(!ou)
		return;

	ou->getIdentity().set("PK", aPk);
	ou->getIdentity().set("LO", aLock);
}

void FakeChecker::setSupports(const HintedUser& p, const string& aSupports) {
	cm->lock();
	OnlineUser* ou = cm->findOnlineUser(p);
	if(!ou)
		return;
	ou->getIdentity().set("SU", aSupports);
}

void FakeChecker::setGenerator(const HintedUser& p, const string& aGenerator, const string& aCID, const string& aBase) {
	Client* c  = nullptr;
	string report;
	{
		cm->lock();
		OnlineUser* ou = cm->findOnlineUser(p);
		if(!ou)
			return;
		ou->getIdentity().set("GE", aGenerator);
		ou->getIdentity().set("FI", aCID);
		ou->getIdentity().set("FB", aBase);
		report = ou->getIdentity().checkFilelistGenerator(*ou);
		c = &ou->getClient();
	}

	if(c && !report.empty()) {
		c->cheatMessage(report);
	}
}

}//namespace dcpp

