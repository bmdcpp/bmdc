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

// RegexpHandler.h Added by Sulan 2005-05-20
// Pothead had a little play with it
// adrian_007 have converted it to boost::regex and made everything inline 04/07/2008

#ifndef REGEX_UTIL_H
#define REGEX_UTIL_H

//#include "ResourceManager.h"
//#include <boost/regex.hpp>
#include <regex.h>

namespace dcpp {
using namespace std;
namespace RegexUtil {
	/*inline string splitVersion(const string& aExp, string aTag, size_t part) {
		try {
			const boost::regex reg(aExp);

			vector<string> out;
			boost::regex_split(std::back_inserter(out), aTag, reg, boost::regex_constants::match_default, 2);

			if(part >= out.size())
				return "";

			return out[part];
		} catch(...) {
			//
		}
		return "";
	}*/

	inline bool/*int*/ match(const std::string& strToMatch, const std::string& expression, bool caseSensative = true) {
		/*try {
			const boost::regex reg(expression, caseSensative ? 0 : boost::regex::icase);
			return boost::regex_search(strToMatch.begin(), strToMatch.end(), reg);
		} catch(...) {
			//...
		}
		return false;*/
			int    status;
			regex_t    re;
			const char *pattern=expression.c_str();
			const char *strings=strToMatch.c_str();
			    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
			        return /*0*/false;      /* Report error. */
    				}
			    status = regexec(&re, strings, (size_t) 0, NULL, 0);
			    regfree(&re);
			    if (status != 0) {
			        return /*0*/false;    /* Report error. */
				    }
    			return /*1*/true;
	}

	// Return the tags version number for %[version]
	/*inline string getVersion(const string& aExp, const string& aTag) {
		string::size_type i = aExp.find("%[version]");
		if (i == string::npos) {
			i = aExp.find("%[version2]");
			return splitVersion(aExp.substr(i + 11), splitVersion(aExp.substr(0, i), aTag, 1), 0);
		}
		return splitVersion(aExp.substr(i + 10), splitVersion(aExp.substr(0, i), aTag, 1), 0);
	}

	// Check if regexp is valid and return if it is a match or no match
	inline string matchExp(const string& expression, const string& strToMatch, const bool caseSensative = true) {
		try {
			const boost::regex reg(expression, caseSensative ? 0 : boost::regex::icase);
			return boost::regex_search(strToMatch.begin(), strToMatch.end(), reg) ? "TRUE"STRING(S_MATCH)// : STRING(S_MISSMATCH)//"FALSE";
		} catch(const boost::regex_error& e) {
			return STRING(S_INVALID) + " Error: " + e.what();
		}
		return STRING(S_INVALID)"Eror";
	}
*/
	//Format the params so we can view the regexp string
	inline std::string formatRegExp(const std::string& msg, dcpp::StringMap& params) {
		std::string result = msg;
		std::string::size_type i, j, k;
		i = 0;
		while (( j = result.find("%[", i)) != string::npos) {
			if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
				break;
			}
			std::string name = result.substr(j + 2, k - j - 2);
			StringMapIter smi = params.find(name);
			if(smi != params.end()) {
				result.replace(j, k-j + 1, smi->second);
				i = j + smi->second.size();
			} else {
				i = k + 1;
			}
		}
		return result;
	}
	// Check if string is an IP
	//inline bool isIp(const string& aString) {
	//	return match(aString, "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b") ? true : false;
	//}
} // namespace RegexUtil
} // namespace dcpp
#endif //REGEX_Util_H
