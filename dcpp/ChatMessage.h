/*
 * Copyright (C)  Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CHAT_MESSAGE_H
#define DCPLUSPLUS_DCPP_CHAT_MESSAGE_H

#include "forward.h"
#include <string>
#include "OnlineUser.h"

namespace dcpp {

using std::string;

struct ChatMessage {
	ChatMessage(const string& _text, OnlineUser* from,
		const OnlineUser* to = nullptr, const OnlineUser* replyTo = nullptr,
		bool _thirdPerson = false, time_t _messageTimestamp = 0):
	from(from->getUser()), to(to ? to->getUser() : nullptr),
	replyTo(replyTo ? replyTo->getUser() : nullptr),
	text(_text),
	timestamp(time(0)),
	messageTimestamp(_messageTimestamp),
	thirdPerson(_thirdPerson) 
	{ 

	}

	UserPtr from;
	UserPtr to;
	UserPtr replyTo;
	string text;
	time_t timestamp; //[ADC-only]
	time_t messageTimestamp;
	bool thirdPerson;	

};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_CHAT_MESSAGE_H)
