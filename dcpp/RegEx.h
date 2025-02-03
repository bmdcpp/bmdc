/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef REG_EX_H
#define REG_EX_H

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>  /* PCRE lib */
#include "StringTokenizer.h"
#include "debug.h"

namespace dcpp {

namespace RegEx {

template<typename T>
bool match(const T& text, const T& pattern, bool ignoreCase = true)  {
	if(pattern.empty())
		return false;
	if(text.empty())
		return false;	
	pcre2_match_data *match_data;
	int error;
	PCRE2_SIZE erroffset;
	pcre2_code *re;
	int rc;
	PCRE2_SIZE *ovector;

	re = pcre2_compile (
		pattern.c_str(),		/* the pattern */
		PCRE2_ZERO_TERMINATED,	/* default options */
		0,
		&error,							/* for error message */
		&erroffset,						/* for error offset */
		NULL);									/* use default character tables */
			 
		if (!re)
  		{
  			PCRE2_UCHAR buffer[256];
  			pcre2_get_error_message(error, buffer, sizeof(buffer));
  			dcdebug("PCRE2 compilation failed at offset %d: %s\n", (int)erroffset,
    		buffer);
  			return false;
  		}
		match_data = pcre2_match_data_create_from_pattern(re, NULL);
	
	/* Now run the match. */

		rc = pcre2_match(
  			re,                   /* the compiled pattern */
  			text.c_str(),              /* the subject string */
  			text.length(),       /* the length of the subject */
  			0,                    /* start at offset 0 in the subject */
  			0,                    /* default options */
  			match_data,           /* block for storing the result */
  			NULL);                /* use default match context */

	if (rc < 0) {
       switch (rc) {
            case PCRE2_ERROR_NOMATCH:
                dcdebug("String didn't match");
				return false;
            default:
                dcdebug("Error while matching: %d\n", rc);
                return false;
        }
        return true;
    }
    if( rc == 0)
    {	
  	 		dcdebug("ovector was not big enough for all the captured substrings\n");
  	 		return true;
	}
    if(rc > 1)
		return true;

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
