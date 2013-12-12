/*
 * Copyright (C) 2001-2013 Jacek Sieka, arnetheduck on gmail point com
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
#include "StringMatch.h"

#include "format.h"
#include "LogManager.h"
#include "StringTokenizer.h"

namespace dcpp {

StringMatch::Method StringMatch::getMethod() const {
	return !searchlist.empty() ? PARTIAL : (!search.empty()) ? EXACT : REGEX;
}

void StringMatch::setMethod(Method method) {
	switch(method) {
	case PARTIAL: searchlist = StringSearch::List(); break;
	case EXACT: search = string(); break;
	case REGEX: reg = string(); break;
	case METHOD_LAST: break;
	}
}

bool StringMatch::operator==(const StringMatch& rhs) const {
	return pattern == rhs.pattern && getMethod() == rhs.getMethod();
}

bool StringMatch::matchlist(string str) const {
	for(auto& i: searchlist) {
		if(!i.match(str)) {
			return false;
		}
	}
	return !searchlist.empty();
	}

bool StringMatch::matchstr(const string& s) const {
	return search == s;
}

bool StringMatch::matchreg(const string& str) const {
		return RegEx::match<string>(str,reg);
}

bool StringMatch::match(const string& str) const {
	if(str.empty())return false;
	if(matchstr(str))return true;
	if(matchlist(str)) return true;
	if(matchreg(str)) return true;
	return false;
}

} // namespace dcpp
