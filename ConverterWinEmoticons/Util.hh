/*
 * Util.hh
 *
 * Copyright (C) 2012 - 2013 - Mank 
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UTIL_HH
#define _UTIL_HH
#include <stdio.h>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <locale.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32

# define PATH_SEPARATOR '\\'
# define PATH_SEPARATOR_STR "\\"

#else

# define PATH_SEPARATOR '/'
# define PATH_SEPARATOR_STR "/"
#endif

using std::string;
using std::wstring;
using std::pair;


/** Evaluates op(pair<T1, T2>.first, compareTo) */
template<class T1, class T2, class op = std::equal_to<T1> >
class CompareFirst {
public:
	CompareFirst(const T1& compareTo) : a(compareTo) { }
	bool operator() (const pair<T1, T2>& p) const { return op()(p.first, a); }
private:
	CompareFirst& operator=(const CompareFirst&);
	const T1& a;
};


class Util
{
	public:

	static string emptyString;
	static wstring emptyStringW;

	static std::string toString(int val) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d", val);
		return buf;
	}

	static int toInt(const std::string& aString) {
		return atoi(aString.c_str());
	}
	
	static std::string formatBytes(int64_t aBytes);

	static std::vector<int> splitString(const std::string &str, const std::string &delimiter);	
    static bool fileExists(const std::string aFile); 
    
    
	static int64_t toInt64(const string& aString) {
#ifdef _WIN32
		return _atoi64(aString.c_str());
#else
		return strtoll(aString.c_str(), (char **)NULL, 10);
#endif
	} 
	
	/* Utf-8 versions of strnicmp and stricmp, unicode char code order (!) */
	static int stricmp(const char* a, const char* b);
	static int strnicmp(const char* a, const char* b, size_t n);
	static int stricmp(const string& a, const string& b) { return stricmp(a.c_str(), b.c_str()); }
	static int strnicmp(const string& a, const string& b, size_t n) { return strnicmp(a.c_str(), b.c_str(), n); }
	 static string getFileName(const string& path, char separator = PATH_SEPARATOR) {
		string::size_type i = path.rfind(separator);
		return (i != string::npos) ? path.substr(i + 1) : path;
	}  
};
#else
class Util;
#endif

