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
	
struct Raw {
	
	Raw() : id(0), name(Util::emptyString), raw(Util::emptyString), time(0), enabled(false) { }
	
	Raw(int _id, const std::string& _name, const std::string& _raw, int _time, bool _enabled)
		: id(_id), name(_name), raw(_raw), time(_time), enabled(_enabled) { }
	
	GETSET(int, id, Id);
	GETSET(std::string, name, Name);
	GETSET(std::string, raw, Raw);
	GETSET(int, time, Time);
	GETSET(bool, enabled, Enabled);

};

struct Action {
	typedef vector<Raw> RawsList;
	typedef std::vector<Action*> ActionList;

	Action() : id(0), name(Util::emptyString), enabled(false) { }
	Action(int _id, const std::string& _name, bool _enabled): id(_id), name(_name), enabled(_enabled) { }
	~Action() { raw.clear(); }

	GETSET(int, id, Id);
	GETSET(std::string, name, Name);
	GETSET(bool, enabled, Enabled);

	RawsList raw;
};

} //dcpp
#endif
