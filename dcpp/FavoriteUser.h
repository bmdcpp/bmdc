/*
 * Copyright (C) 2001-2014 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_FAVORITE_USER_H
#define DCPLUSPLUS_DCPP_FAVORITE_USER_H

#include "Flags.h"

namespace dcpp {

class FavoriteUser : public Flags {
public:
	FavoriteUser() : lastSeen(time(NULL)) { }
	FavoriteUser(const UserPtr& user_, const string& nick_, const string& hubUrl_) : user(user_), url(hubUrl_), lastSeen(0), nick(nick_) 
	{ 
		nicks.push_back(nick);
	}

	enum Flags {
		FLAG_GRANTSLOT = 1 << 0,
		FLAG_IGNORE = 2 << 0
	};

	UserPtr& getUser() { return user; }

	void update(const OnlineUser& info);

	GETSET(UserPtr, user, User);
	GETSET(string, url, Url);
	GETSET(time_t, lastSeen, LastSeen);
	GETSET(string, description, Description);
	GETSET(string, cid, Cid);
	
	void setNick(string _nick)
	{
		uint64_t count = 0;
		for(auto& i:nicks){
				if(i==_nick) ++count;
		}
		if(count > 1)		
			nicks.push_back(_nick);
		nick = _nick;
	}
	string getNick() const
	{ return nick; }

	string getNicks() const {
		string _nicks = Util::emptyString;
		int num = 0;
		for(vector<string>::const_iterator it = nicks.begin();it!= nicks.end();++it)
		{
			if(num == 0) {
				_nicks+= (*it);
			} else {
				_nicks+= ";"+(*it);
			}
			num++;
		 }

		return _nicks+string( num == 0 ? "" : ";");
	}
	void setNicks(vector<string> _nicks) {
			nicks = _nicks;
	}
private:
	vector<string> nicks;
	string nick;
};

} // namespace dcpp

#endif // !defined(FAVORITE_USER_H)
