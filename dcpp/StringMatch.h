/*
 * Copyright (C) 2001-2016 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_STRING_MATCH_H
#define DCPLUSPLUS_DCPP_STRING_MATCH_H

#include "forward.h"
#include "StringSearch.h"
#include "RegEx.h"
#include <string>
#include "Util.h"

namespace dcpp {

using std::string;

/** Provides ways of matching a pattern against strings. */
struct StringMatch {
	
	enum Method {
		PARTIAL, /// case-insensitive pattern matching (multiple patterns separated with spaces)
		EXACT, /// case-sensitive, character-for-character equality
		REGEX /// regular expression
	};
	StringMatch(): i_method(-1),
	search(Util::emptyString),reg(Util::emptyString)
	{ } 
	
	string pattern;

	Method getMethod() const;
	void setMethod(Method method);

	bool operator==(const StringMatch& rhs) const;
	void prepare();
	bool match(const string& str) const;

private:
	int i_method;
	StringSearch::List searchlist;
	string search;
	string reg;
	bool matchlist(string str) const;
	bool matchstr(const string& str) const;
	bool matchreg(const string& str) const;
};

} // namespace dcpp

#endif
