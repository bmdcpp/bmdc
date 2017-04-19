// Util.cpp
//
// Copyright (C) 2012 - 2014 
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
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
#include <ctime>
#include "Util.hh"
#include "Text.h"
using namespace std;
using namespace dcpp;
string Util::emptyString;
wstring Util::emptyStringW;
//..others miscelenous ...
vector<int> Util::splitString(const string &str, const string &delimiter)
{
	string::size_type loc, len, pos = 0;
	vector<int> array;

	if (!str.empty() && !delimiter.empty())
	{
		while ((loc = str.find(delimiter, pos)) != string::npos)
		{
			len = loc - pos;
			array.push_back(toInt(str.substr(pos, len)));
			pos = loc + delimiter.size();
		}
		len = str.size() - pos;
		array.push_back(Util::toInt(str.substr(pos, len)));
	}
	return array;
}

bool Util::fileExists(const string aFile) {
    struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(aFile.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  } else {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. If you
    // need to do that level of checking, lookup the
    // return values of stat which will give you
    // more details on why stat failed.
    blnReturn = false;
  }

  return blnReturn;
}

string Util::formatBytes(int64_t aBytes) {
	char buf[128];
	if(aBytes < 1024) {
		snprintf(buf, sizeof(buf), "%d B", (int)(aBytes&0xffffffff));
	} else if(aBytes < 1024*1024) {
		snprintf(buf, sizeof(buf), "%.02f KiB", (double)aBytes/(1024.0));
	} else if(aBytes < 1024*1024*1024) {
		snprintf(buf, sizeof(buf), "%.02f MiB", (double)aBytes/(1024.0*1024.0));
	} else if(aBytes < (int64_t)1024*1024*1024*1024) {
		snprintf(buf, sizeof(buf), "%.02f GiB", (double)aBytes/(1024.0*1024.0*1024.0));
	} else if(aBytes < (int64_t)1024*1024*1024*1024*1024) {
		snprintf(buf, sizeof(buf), "%.02f TiB", (double)aBytes/(1024.0*1024.0*1024.0*1024.0));
	} else {
		snprintf(buf, sizeof(buf), "%.02f PiB", (double)aBytes/(1024.0*1024.0*1024.0*1024.0*1024.0));
	}

	return buf;
}

int Util::stricmp(const char* a, const char* b) {
	wchar_t ca = 0, cb = 0;
	while(*a) {
		ca = cb = 0;
		int na = Text::utf8ToWc(a, ca);
		int nb = Text::utf8ToWc(b, cb);
		ca = Text::toLower(ca);
		cb = Text::toLower(cb);
		if(ca != cb) {
			return (int)ca - (int)cb;
		}
		a += abs(na);
		b += abs(nb);
	}
	ca = cb = 0;
	Text::utf8ToWc(a, ca);
	Text::utf8ToWc(b, cb);

	return (int)Text::toLower(ca) - (int)Text::toLower(cb);
}

int Util::strnicmp(const char* a, const char* b, size_t n) {
	const char* end = a + n;
	wchar_t ca = 0, cb = 0;
	while(*a && a < end) {
		ca = cb = 0;
		int na = Text::utf8ToWc(a, ca);
		int nb = Text::utf8ToWc(b, cb);
		ca = Text::toLower(ca);
		cb = Text::toLower(cb);
		if(ca != cb) {
			return (int)ca - (int)cb;
		}
		a += abs(na);
		b += abs(nb);
	}
	ca = cb = 0;
	Text::utf8ToWc(a, ca);
	Text::utf8ToWc(b, cb);
	return (a >= end) ? 0 : ((int)Text::toLower(ca) - (int)Text::toLower(cb));
}



