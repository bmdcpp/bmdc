/*
 * Copyright (C) 2006-2013 Crise, crise<at>mail.berlios.de
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

#ifndef REG_EX_H
#define REG_EX_H

#include <pcre.h>     /* PCRE lib */
#include "StringTokenizer.h"
#include "debug.h"
namespace dcpp {

namespace RegEx {
#define OVECCOUNT 999
template<typename T>
bool match(const T& text, const T& pattern, bool ignoreCase = true) /*throw()*/ {
	if(pattern.empty())
		return false;

	try {
		const char *error;
		int   erroffset;
		pcre *re;
		int rc;
		int ovector[OVECCOUNT];
		re = pcre_compile (
             pattern.c_str(),       /* the pattern */
             ignoreCase ? PCRE_CASELESS : 0,           /* default options */
             &error,      /* for error message */
             &erroffset,   /* for error offset */
             0);           /* use default character tables */
		if (!re) {
			printf("pcre_compile failed (offset: %d), %s\n", erroffset, error);
			//throw("pcre_compile failed (offset: %d), %s\n", erroffset, error);
		}

		rc = pcre_exec (
        re,                   /* the compiled pattern */
        0,                    /* no extra data - pattern was not studied */
        text.c_str(),                  /* the string to match */
        text.length(),          /* the length of the string */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        ovector,              /* output vector for substring information */
        OVECCOUNT);           /* number of elements in the output vector */

		if (rc < 0) {
        switch (rc) {
            case PCRE_ERROR_NOMATCH:
                dcdebug("String didn't match");
                free(re);
				return false;
            default:
                printf("Error while matching: %d\n", rc);
                //throw("Error while matching: %d\n", rc);
        }
        free(re);
        return true;
    }
	} catch(...) { /* ... */ }
	return false;
}

template<typename T>
bool match(const T& text, const T& patternlist, const typename T::value_type delimiter, bool ignoreCase = false) {
	if(patternlist.empty())
		return false;

	StringTokenizer<T> st(patternlist, delimiter);
	for(auto i = st.getTokens().cbegin(); i != st.getTokens().cend(); ++i) {
		if(match(text, *i, ignoreCase))
			return true;
	}
	return false;
}

} // namespace RegEx

namespace Wildcard {

template<typename T>
T toRegEx(const T& expr, bool useSet) {
	if(expr.empty())
		return expr;

	T regex = expr;
	typename T::size_type i = 0;

	while((i = regex.find_first_of(useSet ? "().{}+|^$" : "[]().{}+|^$", i)) != T::npos) {
		regex.insert(i, "\\");
		i+=2;
	}

	i = 0;
	while((i = regex.find("?", i)) != T::npos) {
		if(i > 0 && regex[i-1] == '\\') {
			++i; continue;
		}

		regex.replace(i, 1, ".");
	}

	i = 0;
	while((i = regex.find("*", i)) != T::npos) {
		if(i > 0 && regex[i-1] == '\\') {
			++i; continue;
		}

		regex.replace(i, 1, ".*");
		i+=2;
	}

	return "^" + regex + "$";
}

template<typename T>
bool match(const T& text, const T& pattern, bool useSet = true, bool ignoreCase = false) {
	return RegEx::match(text, toRegEx(pattern, useSet), ignoreCase);
}

template<typename T>
bool match(const T& text, const T& patternlist, const typename T::value_type delimiter, bool useSet = true, bool ignoreCase = false) {
	if(patternlist.empty())
		return false;

	StringTokenizer<T> st(patternlist, delimiter);
	for(auto i = st.getTokens().cbegin(); i != st.getTokens().cend(); ++i) {
		if(match(text, *i, useSet, ignoreCase))
			return true;
	}
	return false;
}

} // namespace Wildcard

} // namespace dcpp

#endif // REG_EX_H
