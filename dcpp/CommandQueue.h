/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#ifndef COMMAND_QUEUE
#define COMMAND_QUEUE

#include "UserCommand.h"
#include "CriticalSection.h"
#include <list>

namespace dcpp {
using namespace std;
class Client;
class OnlineUser;
class CommandQueue {
public:
	CommandQueue() throw() : clientPtr(NULL) { }

	void setClientPtr(Client* c) {
		clientPtr = c;
	}

	void onSecond(uint64_t tick) throw();
	void clear() throw() {
		Lock l(cs);
		queue.clear();
		clientPtr = NULL;
	}
	void addCommand(const OnlineUser& ou, int actionId);
private:
	struct CommandItem {
		string name;
		UserCommand uc;
		const OnlineUser *ou;
	};

	typedef list<pair<uint64_t, CommandItem> > Commands;

	void addCommandDelayed(uint64_t delay, const CommandItem& item) throw();
	void execCommand(const CommandItem& item) throw();

	Client* clientPtr;
	CriticalSection cs;
	Commands queue;
};
} // namespace dcpp
#endif
