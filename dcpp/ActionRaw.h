/*
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

#ifndef _BMDC_ACTION_RAW_
#define _BMDC_ACTION_RAW_

#include "compiler.h"
#include "Util.h"
#include "Text.h"
#include "debug.h"
#include "typedefs.h" 
#include "forward.h"
#include "noexcept.h"
#include "GetSet.h"

namespace dcpp {
	using namespace std;
	
struct Raw {
	
	Raw() : name(), raw(),id(0),time(0), enabled(false) { }
	
	Raw(int _id, const string& _name, const string& _raw, uint64_t _time, bool _enabled)
		: name(_name), raw(_raw),id(_id), time(_time), enabled(_enabled) { }
	
	GETSET(string, name, Name);
	GETSET(string, raw, Raw);
	GETSET(int, id, Id);
	GETSET(uint64_t, time, Time);
	GETSET(bool, enabled, Enabled);

};

struct Action {
	typedef vector<Raw> RawsList;
	typedef std::vector<Action*> ActionList;

	Action() : name(),id(0), enabled(false) { }
	Action(int _id, const string& _name, bool _enabled): name(_name), id(_id), enabled(_enabled) { }
	~Action() { raw.clear(); }

	GETSET(string, name, Name);
	GETSET(int, id, Id);
	GETSET(bool, enabled, Enabled);

	RawsList raw;
};

} //dcpp
#endif
