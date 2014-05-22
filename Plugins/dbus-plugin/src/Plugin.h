/*
 * Copyright (C) 2012-2014 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H
#include <gtk/gtk.h>
#include <map>
#include <vector>
#include <string>
#include <cstring>
typedef std::map<std::string, std::string> StringMap;
typedef StringMap::const_iterator StringMapIter;
typedef std::vector<std::string> StringList;
typedef StringList::const_iterator StringIter;
using std::string;

class Plugin
{
public:
	static Bool DCAPI main(PluginState state, DCCorePtr core, dcptr_t);
	bool parseCommand(const char* cmd, const char* param, string& result, bool& status);
private:
	Plugin();
	~Plugin();
	void addHooks();
	bool onLoad(DCCorePtr core, bool install, bool runtime);
	string getSpam(bool& status);
	bool onChatCommand(HubDataPtr hub, CommandDataPtr cmd);
	bool onChatCommandPM(UserDataPtr user, CommandDataPtr cmd);
	GDBusProxy *proxy = NULL;
	string parse(GVariant *var,string suffix);
	static Plugin* instance;
	string fixedftime(const string& format, struct tm* t) {
		string ret = format;
		const char codes[] = "aAbBcdHIjmMpSUwWxXyYzZ%";

	char tmp[4];
	tmp[0] = '%';
	tmp[1] = tmp[2] = tmp[3] = 0;

	StringMap sm;
	static const size_t BUF_SIZE = 1024;
	string buf;
	buf.resize(BUF_SIZE);
	for(size_t i = 0; i < strlen(codes); ++i) {
		tmp[1] = codes[i];
		tmp[2] = 0;
		strftime(&buf[0], BUF_SIZE-1, tmp, t);
		sm[tmp] = &buf[0];

		tmp[1] = '#';
		tmp[2] = codes[i];
		strftime(&buf[0], BUF_SIZE-1, tmp, t);
		sm[tmp] = &buf[0];
	}

	for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
		for(string::size_type j = ret.find(i->first); j != string::npos; j = ret.find(i->first, j)) {
			ret.replace(j, i->first.length(), i->second);
			j += i->second.length() - i->first.length();
		}
	}

	return ret;
}

string formatTime(const string &msg, const time_t t) {
	if (!msg.empty()) {
		size_t bufsize = msg.size() + 256;
		struct tm* loc = localtime(&t);

		if(!loc) {
			return "";
		}
		string buf(bufsize, 0);
		buf.resize(strftime(&buf[0], buf.size()-1, msg.c_str(), loc));

		if(buf.empty()) {
			return fixedftime(msg, loc);
		}

		return buf;
	}
	return "";
}

string formatParams(const string& format, StringMap& params) {
	string result = format;
	string::size_type i, j, k;
	i = 0;
	while (( j = result.find("%[", i)) != string::npos) {
		if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
			break;
		}
		string name = result.substr(j + 2, k - j - 2);
		StringMapIter smi = params.find(name);
		if(smi == params.end()) {
			result.erase(j, k-j + 1);
			i = j;
		} else {
			if(smi->second.find_first_of("%\\./") != string::npos) {
				string tmp = smi->second;	// replace all % in params with %% for strftime
				string::size_type m = 0;
				while(( m = tmp.find('%', m)) != string::npos) {
					tmp.replace(m, 1, "%%");
					m+=2;
				}

				result.replace(j, k-j + 1, tmp);
				i = j + tmp.size();
			} else {
				result.replace(j, k-j + 1, smi->second);
				i = j + smi->second.size();
			}
		}
	}

	result = formatTime(result, time(NULL));
	return result;
}
};

#endif
