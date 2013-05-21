/*
 * FakeChecker.h
 * This file is part of BMDC++
 *
 * Copyright (C) 2012 - 2013 - Mank
 *
 * BMDC++ is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * <library name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef _FAKE_CHECKER_
#define _FAKE_CHECKER_
#include "stdinc.h"
#include "DCPlusPlus.h"
#include "HintedUser.h"

namespace dcpp {
class FakeChecker
{
	public:
		FakeChecker(ClientManager *_cm): cm(_cm) { }
		virtual ~FakeChecker() {};

	void setSupports(const HintedUser& user, const string& aSupport);
	void setGenerator(const HintedUser& user, const string& aGenerator, const string& cid, const string& aBase);
	void setPkLock(const HintedUser& user, const string& aPk, const string& aLock);
	void setUnknownCommand(const HintedUser& user, const string& cmd);
	void setListSize(const HintedUser& user, int64_t listSize, bool adc);
	void setListLength(const HintedUser& user, const string& listLen);
	void fileListDisconnected(const HintedUser& user);

	void setCheating(const HintedUser& user, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete);

	//friend class ClientManager;//is this needed ??
	ClientManager *cm;
};
}//namespace dcpp
#endif
