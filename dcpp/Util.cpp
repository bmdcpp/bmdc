/*
 * Copyright (C) Jacek Sieka, arnetheduck on gmail point com
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
#include "Util.h"
#include "nullptr.h"
#include "Encoder.h"
#include "CID.h"
#include "ClientManager.h"
#include "ConnectivityManager.h"
#include "File.h"
#include "LogManager.h"
#include "SettingsManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "version.h"

#ifndef _WIN32
	#include <ifaddrs.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/utsname.h>
	#include <cctype>
	#include <cstring>
#else
#include "Text.h"
#include "w.h"
#include <shlobj.h>
#endif
#include <clocale>

#define __STDC_LIMIT_MACROS
#include <stdint.h>
//Some old glibc didnt follow that much C++11 see stdinc.h
#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

namespace dcpp {

using std::make_pair;

string Util::emptyString;
wstring Util::emptyStringW;
tstring Util::emptyStringT;
bool Util::away = false;
bool Util::manualAway = false;
string Util::awayMsg;
time_t Util::awayTime;
uint64_t Util::uptime = time(NULL);
string Util::paths[Util::PATH_LAST];

bool Util::localMode = true;

static void sgenrand(unsigned long seed);

extern "C" void bz_internal_error(int errcode) {
	dcdebug("bzip2 internal error: %d\n", errcode);
}

#ifdef _WIN32

typedef HRESULT (WINAPI* _SHGetKnownFolderPath)(GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);

static string getDownloadsPath(const string& def) {
	// Try Vista downloads path
	static _SHGetKnownFolderPath getKnownFolderPath = 0;
	static HINSTANCE shell32 = NULL;

	if(!shell32) {
	    shell32 = ::LoadLibrary(_T("Shell32.dll"));
	    if(shell32)
	    {
	    	getKnownFolderPath = (_SHGetKnownFolderPath)::GetProcAddress(shell32, "SHGetKnownFolderPath");

	    	if(getKnownFolderPath) {
	    		 PWSTR path = NULL;
	             // Defined in KnownFolders.h.
	             static GUID downloads = {0x374de290, 0x123f, 0x4565, {0x91, 0x64, 0x39, 0xc4, 0x92, 0x5e, 0x46, 0x7b}};
	    		 if(getKnownFolderPath(downloads, 0, NULL, &path) == S_OK) {
	    			 string ret = std::string();
	    			 Text::wcToUtf8(*path,ret);
	    			 //string ret = Text::fromT(path) + "\\";
	    			 ::CoTaskMemFree(path);
	    			 return ret;
	    		 }
	    	}
	    }
	}

	return def + "Downloads\\";
}

#endif

void Util::initialize(PathsMap pathOverrides) {
	Text::initialize();

	sgenrand((unsigned long)time(NULL));

#ifdef _WIN32
	TCHAR buf[MAX_PATH+1] = { 0 };
	::GetModuleFileName(NULL, buf, MAX_PATH);

	string exePath = Util::getFilePath(Text::fromT(buf));

	// Global config path is DC++ executable path...
	paths[PATH_GLOBAL_CONFIG] = exePath;

	paths[PATH_USER_CONFIG] = paths[PATH_GLOBAL_CONFIG];

	loadBootConfig();

	if(!File::isAbsolute(paths[PATH_USER_CONFIG])) {
		paths[PATH_USER_CONFIG] = paths[PATH_GLOBAL_CONFIG] + paths[PATH_USER_CONFIG];
	}

	paths[PATH_USER_CONFIG] = validateFileName(paths[PATH_USER_CONFIG]);

	if(localMode) {
		paths[PATH_USER_LOCAL] = paths[PATH_USER_CONFIG];

		paths[PATH_DOWNLOADS] = paths[PATH_USER_CONFIG] + "Downloads\\";

	} else {
		if(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
			paths[PATH_USER_CONFIG] = Text::fromT(buf) + "\\DC++\\";
		}

		paths[PATH_DOWNLOADS] = getDownloadsPath(paths[PATH_USER_CONFIG]);

		paths[PATH_USER_LOCAL] = ::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK ? Text::fromT(buf) + "\\DC++\\" : paths[PATH_USER_CONFIG];
	}

	paths[PATH_RESOURCES] = exePath;

	// libintl doesn't support wide path names so we use the short (8.3) format.
	// https://sourceforge.net/forum/message.php?msg_id=4882703
	tstring localePath_ = Text::toT(exePath) + _T("locale\\");
	memset(buf, 0, sizeof(buf));
	::GetShortPathName(localePath_.c_str(), buf, sizeof(buf)/sizeof(TCHAR));

	paths[PATH_LOCALE] = Text::fromT(buf);

#else
	string home = g_get_home_dir ();
	#ifndef _DEBUG
	paths[PATH_GLOBAL_CONFIG] = paths[PATH_USER_CONFIG] = home + "/.bmdc++/";
	#else
	paths[PATH_GLOBAL_CONFIG] = paths[PATH_USER_CONFIG] = home + "/.bmdc++-debug/";
	#endif
	#ifdef _WIN32
        loadBootConfig();
	#endif
	if(!File::isAbsolute(paths[PATH_USER_CONFIG])) {
		paths[PATH_USER_CONFIG] = paths[PATH_GLOBAL_CONFIG] + paths[PATH_USER_CONFIG];
	}

	paths[PATH_USER_CONFIG] = validateFileName(paths[PATH_USER_CONFIG]);

	if(localMode) {
		// @todo implement...
	}

	paths[PATH_USER_LOCAL] = paths[PATH_USER_CONFIG];
	paths[PATH_RESOURCES] = "/usr/share/";
	paths[PATH_LOCALE] = paths[PATH_RESOURCES] + "locale/";
	paths[PATH_DOWNLOADS] = home + "/Downloads/";

#endif

	paths[PATH_FILE_LISTS] = paths[PATH_USER_LOCAL] + "FileLists" PATH_SEPARATOR_STR;
	paths[PATH_HUB_LISTS] = paths[PATH_USER_LOCAL] + "HubLists" PATH_SEPARATOR_STR;
	paths[PATH_NOTEPAD] = paths[PATH_USER_CONFIG] + "Notepad.txt";
	paths[PATH_BACKUP] = paths[PATH_USER_CONFIG] + "Backups" PATH_SEPARATOR_STR;

	// Override core generated paths
	for (PathsMap::const_iterator it = pathOverrides.begin(); it != pathOverrides.end(); ++it)
	{
		if (!it->second.empty())
			paths[it->first] = it->second;
	}

	File::ensureDirectory(paths[PATH_USER_CONFIG]);
	File::ensureDirectory(paths[PATH_USER_LOCAL]);
}

void Util::migrate(const string& file) {
	if(localMode) {
		return;
	}

	if(File::getSize(file) != -1) {
		return;
	}

	string fname = getFileName(file);
	string old = paths[PATH_GLOBAL_CONFIG] + fname;
	if(File::getSize(old) == -1) {
		return;
	}

	File::renameFile(old, file);
}
#ifdef _WIN32
void Util::loadBootConfig() {

	// Load boot settings
	try {
		SimpleXML boot;
		boot.fromXML(File(getPath(PATH_GLOBAL_CONFIG) + "dcppboot.xml", File::READ, File::OPEN).read());
		boot.stepIn();

		if(boot.findChild("LocalMode")) {
			localMode = boot.getChildData() != "0";
		}

		if(boot.findChild("ConfigPath")) {
			ParamMap params;
			/// @todo load environment variables instead? would make it more useful on *nix
			/*params["APPDATA"] = []() -> string {
				TCHAR path[MAX_PATH];
				return Text::fromT((::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path), path));
			};
			params["PERSONAL"] = []() -> string {
				TCHAR path[MAX_PATH];
				return Text::fromT((::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path), path));
			};*/
			paths[PATH_USER_CONFIG] = Util::formatParams(boot.getChildData(), params);
		}
	} catch(const Exception& ) {
		// Unable to load boot settings...
	}

}
#endif
#ifdef _WIN32
static const char badChars[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, '<', '>', '/', '"', '|', '?', '*', 0
};
#else

static const char badChars[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, '<', '>', '\\', '"', '|', '?', '*', 0
};
#endif

/**
 * Replaces all strange characters in a file with '_'
 * @todo Check for invalid names such as nul and aux...
 */
string Util::validateFileName(string tmp) {
	string::size_type i = 0;

	// First, eliminate forbidden chars
	while( (i = tmp.find_first_of(badChars, i)) != string::npos) {
		tmp[i] = '_';
		i++;
	}

	// Then, eliminate all ':' that are not the second letter ("c:\...")
	i = 0;
	while( (i = tmp.find(':', i)) != string::npos) {
		if(i == 1) {
			i++;
			continue;
		}
		tmp[i] = '_';
		i++;
	}

	// Remove the .\ that doesn't serve any purpose
	i = 0;
	while( (i = tmp.find("\\.\\", i)) != string::npos) {
		tmp.erase(i+1, 2);
	}
	i = 0;
	while( (i = tmp.find("/./", i)) != string::npos) {
		tmp.erase(i+1, 2);
	}

	// Remove any double \\ that are not at the beginning of the path...
	i = 1;
	while( (i = tmp.find("\\\\", i)) != string::npos) {
		tmp.erase(i+1, 1);
	}
	i = 1;
	while( (i = tmp.find("//", i)) != string::npos) {
		tmp.erase(i+1, 1);
	}

	// And last, but not least, the infamous ..\! ...
	i = 0;
	while( ((i = tmp.find("\\..\\", i)) != string::npos) ) {
		tmp[i + 1] = '_';
		tmp[i + 2] = '_';
		tmp[i + 3] = '_';
		i += 2;
	}
	i = 0;
	while( ((i = tmp.find("/../", i)) != string::npos) ) {
		tmp[i + 1] = '_';
		tmp[i + 2] = '_';
		tmp[i + 3] = '_';
		i += 2;
	}

	// Dots at the end of path names aren't popular
	i = 0;
	while( ((i = tmp.find(".\\", i)) != string::npos) ) {
		tmp[i] = '_';
		i += 1;
	}
	i = 0;
	while( ((i = tmp.find("./", i)) != string::npos) ) {
		tmp[i] = '_';
		i += 1;
	}


	return tmp;
}

bool Util::checkExtension(const string& tmp) {
	for(size_t i = 0, n = tmp.size(); i < n; ++i) {
		if (tmp[i] < 0 || tmp[i] == 32 || tmp[i] == ':') {
			return false;
		}
	}
	if(tmp.find_first_of(badChars, 0) != string::npos) {
		return false;
	}
	return true;
}

string Util::cleanPathChars(const string& str) {
	string ret(str);
	string::size_type i = 0;
	while((i = ret.find_first_of("/.\\", i)) != string::npos) {
		ret[i] = '_';
	}
	return ret;
}

string Util::addBrackets(const string& s) {
	return '<' + s + '>';
}

string Util::getShortTimeString(time_t t) {
	char buf[255];
	tm* _tm = localtime(&t);
	if(_tm == NULL) {
		strcpy(buf, "xx:xx");
	} else {
		strftime(buf, 254, SETTING(TIME_STAMPS_FORMAT).c_str(), _tm);
	}
	return buf;
}

/**
 * Decodes a URL the best it can...
 * Default ports:
 * http:// -> port 80
 * https:// -> port 443
 * dchub:// -> port 411
 */
void Util::decodeUrl(const string& url, string& protocol, string& host, uint16_t& port, string& path, string& query, string& fragment) {
	size_t fragmentEnd = url.size();
	size_t fragmentStart = url.rfind('#');

	size_t queryEnd;
	if(fragmentStart == string::npos) {
		queryEnd = fragmentStart = fragmentEnd;
	} else {
		dcdebug("f");
		queryEnd = fragmentStart;
		fragmentStart++;
	}

	size_t queryStart = url.rfind('?', queryEnd);
	size_t fileEnd;

	if(queryStart == string::npos) {
		fileEnd = queryStart = queryEnd;
	} else {
		dcdebug("q");
		fileEnd = queryStart;
		queryStart++;
	}

	size_t protoStart = 0;
	size_t protoEnd = url.find("://", protoStart);

	size_t authorityStart = protoEnd == string::npos ? protoStart : protoEnd + 3;
	size_t authorityEnd = url.find_first_of("/#?", authorityStart);

	size_t fileStart;
	if(authorityEnd == string::npos) {
		authorityEnd = fileStart = fileEnd;
	} else {
		dcdebug("a");
		fileStart = authorityEnd;
	}

	protocol = (protoEnd == string::npos ? string() : url.substr(protoStart, protoEnd - protoStart));

	if(authorityEnd > authorityStart) {
		dcdebug("x");
		size_t portStart = string::npos;
		if(url[authorityStart] == '[') {
			// IPv6?
			size_t hostEnd = url.find(']');
			if(hostEnd == string::npos) {
				return;
			}

			host = url.substr(authorityStart + 1, hostEnd - authorityStart - 1);
			if(hostEnd + 1 < url.size() && url[hostEnd + 1] == ':') {
				portStart = hostEnd + 2;
			}
		} else {
			size_t hostEnd;
			portStart = url.find(':', authorityStart);
			if(portStart != string::npos && portStart > authorityEnd) {
				portStart = string::npos;
			}

			if(portStart == string::npos) {
				hostEnd = authorityEnd;
			} else {
				hostEnd = portStart;
				portStart++;
			}

			dcdebug("h");
			host = url.substr(authorityStart, hostEnd - authorityStart);
		}

		if(portStart == string::npos) {
			if(protocol == "http") {
				port = 80;
			} else if(protocol == "https") {
				port = 443;
			} else if(protocol == "dchub"  || protocol.empty()) {
				port = 411;
			}
		} else {
			dcdebug("p");
			port = Util::toInt(url.substr(portStart, authorityEnd - portStart));

			if(port == UINT16_MAX )
				port = 0;

		}
	}

	dcdebug("\n");
	path = url.substr(fileStart, fileEnd - fileStart);
	query = url.substr(queryStart, queryEnd - queryStart);
	fragment = url.substr(fragmentStart, fragmentEnd - fragmentStart);
}

map<string, string> Util::decodeQuery(const string& query) {
	map<string, string> ret;
	size_t start = 0;
	while(start < query.size()) {
		size_t eq = query.find('=', start);
		if(eq == string::npos) {
			break;
		}

		size_t param = eq + 1;
		size_t end = query.find('&', param);

		if(end == string::npos) {
			end = query.size();
		}

		if(eq > start && end > param) {
			ret[query.substr(start, eq-start)] = query.substr(param, end - param);
		}

		start = end + 1;
	}

	return ret;
}

string Util::getAwayMessage(ParamMap& params) {
	params["idleTI"] = Text::fromT(formatSeconds(time(NULL) - awayTime));
	params["time"] = Util::toString(awayTime);
	return formatParams((awayMsg.empty() ? SETTING(DEFAULT_AWAY_MESSAGE) : awayMsg), params);
}

string Util::formatBytes(const int64_t aBytes) {
	char buf[128];
	if(aBytes < 1024) {
		snprintf(buf, sizeof(buf), ("%d B"), (int)(aBytes&0xffffffff));
	} else if(aBytes < 1024*1024) {
		snprintf(buf, sizeof(buf), ("%.02f KiB"), (double)aBytes/(1024.0));
	} else if(aBytes < 1024*1024*1024) {
		snprintf(buf, sizeof(buf), ("%.02f MiB"), (double)aBytes/(1024.0*1024.0));
	} else if(aBytes < (int64_t)1024*1024*1024*1024) {
		snprintf(buf, sizeof(buf), ("%.02f GiB"), (double)aBytes/(1024.0*1024.0*1024.0));
	} else if(aBytes < (int64_t)1024*1024*1024*1024*1024) {
		snprintf(buf, sizeof(buf), ("%.02f TiB"), (double)aBytes/(1024.0*1024.0*1024.0*1024.0));
	} else {
		snprintf(buf, sizeof(buf), ("%.02f PiB"), (double)aBytes/(1024.0*1024.0*1024.0*1024.0*1024.0));
	}

	return buf;
}

string Util::formatExactSize(const int64_t aBytes) {
#ifdef _WIN32
		TCHAR tbuf[128];
		TCHAR number[64];
		NUMBERFMT nf;
		_sntprintf(number, 64, _T("%I64d"), aBytes);
		TCHAR Dummy[16];
		TCHAR sep[2] = _T(",");

		/*No need to read these values from the system because they are not
		used to format the exact size*/
		nf.NumDigits = 0;
		nf.LeadingZero = 0;
		nf.NegativeOrder = 0;
		nf.lpDecimalSep = sep;

		GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_SGROUPING, Dummy, 16 );
		nf.Grouping = Util::toInt(Text::fromT(Dummy));
		GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, Dummy, 16 );
		nf.lpThousandSep = Dummy;

		GetNumberFormat(LOCALE_USER_DEFAULT, 0, number, &nf, tbuf, sizeof(tbuf)/sizeof(tbuf[0]));

		char buf[128];
		_snprintf(buf, sizeof(buf), _("%s B"), Text::fromT(tbuf).c_str());
		return buf;
#else
		char buf[128];
		snprintf(buf, sizeof(buf), ("%'lld B"), (long long int)aBytes);
		return string(buf);
#endif
}
//todo win code?

string Util::getLocalIp(bool IsIPv6) {
#ifndef _WIN32
		struct ifaddrs *ifaddr= NULL,*ifa = NULL;
		void *tmp = NULL;
		string mAddrIP = string() , mAddrIP6 = string();
		getifaddrs(&ifaddr);
		for(ifa = ifaddr;ifa!= NULL;ifa = ifa->ifa_next)
		{
			if(!ifa->ifa_addr)
				continue;
				
			if(ifa->ifa_addr->sa_family == AF_INET)
			{
				tmp = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
				char address[INET_ADDRSTRLEN];
				inet_ntop(AF_INET,tmp,address, INET_ADDRSTRLEN);
				mAddrIP = address;
			}
			else if(ifa->ifa_addr->sa_family == AF_INET6)
			{
				tmp = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;
				char address[INET_ADDRSTRLEN];
				inet_ntop(AF_INET6,tmp,address, INET_ADDRSTRLEN);
				mAddrIP6 = address;
			}
			
			
		}
		if(IsIPv6)
				mAddrIP = mAddrIP6;
		return mAddrIP;
/*	
	char buf[256];
	gethostname(buf, 255);
	hostent* he = gethostbyname(buf);
	if(he == NULL || he->h_addr_list[0] == 0)
		return Util::emptyString;
	sockaddr_in dest;
	int i = 0;

	// We take the first ip as default, but if we can find a better one, use it instead...
	memcpy(&(dest.sin_addr), he->h_addr_list[i++], he->h_length);
	tmp = inet_ntoa(dest.sin_addr);
	if(Util::isPrivateIp(tmp) || strncmp(tmp.c_str(), "169.254", 7) == 0) {
		while(he->h_addr_list[i]) {
			memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
			string tmp2 = inet_ntoa(dest.sin_addr);
			if(!Util::isPrivateIp(tmp2) && strncmp(tmp2.c_str(), "169.254", 7) != 0) {
				tmp = tmp2;
			}
			i++;
		}
	}*/
#else
return string();
#endif	
}

bool Util::isPrivateIp(string const& ip) {
	struct in_addr addr;

	addr.s_addr = inet_addr(ip.c_str());

	if (addr.s_addr != INADDR_NONE) {
		unsigned long haddr = ntohl(addr.s_addr);
		return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
				(haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
				(haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
				(haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
	}
	return false;
}

typedef const uint8_t* ccp;
static wchar_t utf8ToLC(ccp& str) {
	wchar_t c = 0;
	if(str[0] & 0x80) {
		if(str[0] & 0x40) {
			if(str[0] & 0x20) {
				if(str[1] == 0 || str[2] == 0 ||
					!((((unsigned char)str[1]) & ~0x3f) == 0x80) ||
					!((((unsigned char)str[2]) & ~0x3f) == 0x80))
				{
					str++;
					return 0;
				}
				c = ((wchar_t)(unsigned char)str[0] & 0xf) << 12 |
					((wchar_t)(unsigned char)str[1] & 0x3f) << 6 |
					((wchar_t)(unsigned char)str[2] & 0x3f);
				str += 3;
			} else {
				if(str[1] == 0 ||
					!((((unsigned char)str[1]) & ~0x3f) == 0x80))
				{
					str++;
					return 0;
				}
				c = ((wchar_t)(unsigned char)str[0] & 0x1f) << 6 |
					((wchar_t)(unsigned char)str[1] & 0x3f);
				str += 2;
			}
		} else {
			str++;
			return 0;
		}
	} else {
		wchar_t c = Text::asciiToLower((char)str[0]);
		str++;
		return c;
	}

	return Text::toLower(c);
}

string Util::toString(const StringList& lst) {
	if(lst.empty())
		return string();
	if(lst.size() == 1)
		return lst[0];
	return '[' + toString(",", lst) + ']';
}

string::size_type Util::findSubString(const string& aString, const string& aSubString, string::size_type start) noexcept {
	if(aString.length() < start)
		return (string::size_type)string::npos;

	if(aString.length() - start < aSubString.length())
		return (string::size_type)string::npos;

	if(aSubString.empty())
		return 0;

	// Hm, should start measure in characters or in bytes? bytes for now...
	const uint8_t* tx = (const uint8_t*)aString.c_str() + start;
	const uint8_t* px = (const uint8_t*)aSubString.c_str();

	const uint8_t* end = tx + aString.length() - start - aSubString.length() + 1;

	wchar_t wp = utf8ToLC(px);

	while(tx < end) {
		const uint8_t* otx = tx;
		if(wp == utf8ToLC(tx)) {
			const uint8_t* px2 = px;
			const uint8_t* tx2 = tx;

			for(;;) {
				if(*px2 == 0)
					return otx - (uint8_t*)aString.c_str();

				if(utf8ToLC(px2) != utf8ToLC(tx2))
					break;
			}
		}
	}
	return (string::size_type)string::npos;
}

wstring::size_type Util::findSubString(const wstring& aString, const wstring& aSubString, wstring::size_type pos) noexcept {
	if(aString.length() < pos)
		return static_cast<wstring::size_type>(wstring::npos);

	if(aString.length() - pos < aSubString.length())
		return static_cast<wstring::size_type>(wstring::npos);

	if(aSubString.empty())
		return 0;

	wstring::size_type j = 0;
	wstring::size_type end = aString.length() - aSubString.length() + 1;

	for(; pos < end; ++pos) {
		if(Text::toLower(aString[pos]) == Text::toLower(aSubString[j])) {
			wstring::size_type tmp = pos+1;
			bool found = true;
			for(++j; j < aSubString.length(); ++j, ++tmp) {
				if(Text::toLower(aString[tmp]) != Text::toLower(aSubString[j])) {
					j = 0;
					found = false;
					break;
				}
			}

			if(found)
				return pos;
		}
	}
	return static_cast<wstring::size_type>(wstring::npos);
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

int compare(const std::string& a, const std::string& b) {
	return compare(a.c_str(), b.c_str());
}
int compare(const std::wstring& a, const std::wstring& b) {
	return compare(a.c_str(), b.c_str());
}
int compare(const char* a, const char* b) {
	// compare wide chars because the locale is usually not *.utf8 (never on Win)
	wchar_t ca[2] = { 0 }, cb[2] = { 0 };
	while(*a) {
		ca[0] = cb[0] = 0;
		int na = Text::utf8ToWc(a, ca[0]);
		int nb = Text::utf8ToWc(b, cb[0]);
		auto comp = compare(const_cast<const wchar_t*>(ca), const_cast<const wchar_t*>(cb));
		if(comp) {
			return comp;
		}
		a += abs(na);
		b += abs(nb);
	}
	ca[0] = cb[0] = 0;
	Text::utf8ToWc(a, ca[0]);
	Text::utf8ToWc(b, cb[0]);
	return compare(const_cast<const wchar_t*>(ca), const_cast<const wchar_t*>(cb));
}
int compare(const wchar_t* a, const wchar_t* b) {
	return wcscoll(a, b);
}

string Util::encodeURI(const string& aString, bool reverse) {
	// reference: rfc2396
	string tmp = aString;
	if(reverse) {
		string::size_type idx;
		for(idx = 0; idx < tmp.length(); ++idx) {
			if(tmp.length() > idx + 2 && tmp[idx] == '%' && isxdigit(tmp[idx+1]) && isxdigit(tmp[idx+2])) {
				tmp[idx] = fromHexEscape(tmp.substr(idx+1,2));
				tmp.erase(idx+1, 2);
			} else { // reference: rfc1630, magnet-uri draft
				if(tmp[idx] == '+')
					tmp[idx] = ' ';
			}
		}
	} else {
		const string disallowed = ";/?:@&=+$," // reserved
								  "<>#%\" "    // delimiters
								  "{}|\\^[]`"; // unwise
		string::size_type idx;
		for(idx = 0; idx < tmp.length(); ++idx) {
			if(tmp[idx] == ' ') {
				tmp[idx] = '+';
			} else {
				if(tmp[idx] <= 0x1F || tmp[idx] >= 0x7f || (disallowed.find_first_of(tmp[idx])) != string::npos) {
					tmp.replace(idx, 1, toHexEscape(tmp[idx]));
					idx+=2;
				}
			}
		}
	}
	return tmp;
}
/**
 * This function takes a string and a set of parameters and transforms them according to
 * a simple formatting rule, similar to strftime. In the message, every parameter should be
 * represented by %[name]. It will then be replaced by the corresponding item in
 * the params stringmap. After that, the string is passed through strftime with the current
 * date/time and then finally written to the log file. If the parameter is not present at all,
 * it is removed from the string completely...
 */
string Util::formatParams(const string& msg, const ParamMap& params, FilterF filter) {
	string result = msg;

	string::size_type i, j, k;
	i = 0;
	while (( j = result.find("%[", i)) != string::npos) {
		if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
			break;
		}

		auto param = params.find(result.substr(j + 2, k - j - 2));

		if(param == params.end()) {
			result.erase(j, k - j + 1);
			i = j;

		} else {
			auto replacement = param->second;

			// replace all % in params with %% for strftime
			replace("%", "%%", replacement);

			if(filter) {
				replacement = filter(replacement);
			}

			result.replace(j, k - j + 1, replacement);
			i = j + replacement.size();
		}
	}

	result = formatTime(result, time(NULL));

	return result;
}

string Util::formatTime(const string &msg, const time_t t) {
	if(!msg.empty()) {
		tm* loc = localtime(&t);
		if(!loc) {
			return string();
		}

		size_t bufsize = msg.size() + 256;
		string buf(bufsize, 0);

		errno = 0;

		buf.resize(strftime(&buf[0], bufsize-1, msg.c_str(), loc));

		while(buf.empty()) {
			if(errno == EINVAL)
				return string();

			bufsize+=64;
			buf.resize(bufsize);
			buf.resize(strftime(&buf[0], bufsize-1, msg.c_str(), loc));
		}
/*
#ifdef _WIN32
		if(!Text::validateUtf8(buf))
#endif
		{
			buf = Text::toUtf8(buf);
		}*/
		if(!g_utf8_validate(buf.c_str(),-1,NULL))
			return string();
		//gsize oread,owrite;
		buf = g_filename_to_utf8(buf.c_str(),-1,NULL,NULL,NULL);

		return buf;
	}
	return string();
}

/* Below is a high-speed random number generator with much
   better granularity than the CRT one in msvc...(no, I didn't
   write it...see copyright) */
/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.
   Any feedback is very welcome. For any question, comments,
   see http://www.math.keio.ac.jp/matumoto/emt.html or email
   matumoto@math.keio.ac.jp */
/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializing the array with a NONZERO seed */
static void sgenrand(unsigned long seed) {
	/* setting initial seeds to mt[N] using         */
	/* the generator Line 25 of Table 1 in          */
	/* [KNUTH 1981, The Art of Computer Programming */
	/*    Vol. 2 (2nd Ed.), pp102]                  */
	mt[0]= seed & 0xffffffff;
	for (mti=1; mti<N; mti++)
		mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

uint32_t Util::rand() {
	unsigned long y;

	if (mti >= N) { /* generate N words at one time */
		//...
		static unsigned long mag01[2]={0x0, MATRIX_A};
		/* mag01[x] = x * MATRIX_A  for x=0,1 */

		int kk;

		if (mti == N+1)   /* if sgenrand() has not been called, */
			sgenrand(4357); /* a default initial seed is used   */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

		mti = 0;
	}

	y = mt[mti++];
	y ^= TEMPERING_SHIFT_U(y);
	y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
	y ^= TEMPERING_SHIFT_L(y);

	return y;
}

string Util::getTimeString() {
	char buf[64];
	time_t _tt;
	time(&_tt);
	tm* _tm = localtime(&_tt);
	if(_tm == NULL) {
		strcpy(buf, "xx:xx:xx");
	} else {
		strftime(buf, 64, "%X", _tm);
	}
	return buf;
}

string Util::toAdcFile(const string& file) {
	if(file == "files.xml.bz2" || file == "files.xml")
		return file;

	string ret;
	ret.reserve(file.length() + 1);
	ret += '/';
	ret += file;
	for(string::size_type i = 0; i < ret.length(); ++i) {
		if(ret[i] == '\\') {
			ret[i] = '/';
		}
	}
	return ret;
}
string Util::toNmdcFile(const string& file) {
	if(file.empty())
		return string();

	string ret(file.substr(1));
	for(string::size_type i = 0; i < ret.length(); ++i) {
		if(ret[i] == '/') {
			ret[i] = '\\';
		}
	}
	return ret;
}

string Util::translateError(int aError) {
#ifdef _WIN32
	LPTSTR lpMsgBuf;
	DWORD chars = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		aError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);
	if(chars == 0) {
		return string();
	}
	string tmp = Text::fromT(lpMsgBuf);
	// Free the buffer.
	LocalFree( lpMsgBuf );
	string::size_type i = 0;

	while( (i = tmp.find_first_of("\r\n", i)) != string::npos) {
		tmp.erase(i, 1);
	}
	return tmp;
#else // _WIN32
	return Text::toUtf8(strerror(aError));
#endif // _WIN32
}

bool Util::getAway() {
	return away;
}

void Util::setAway(bool aAway) {
	bool changed = aAway != away;

	away = aAway;
	if(away)
		awayTime = time(NULL);

	if(changed)
		ClientManager::getInstance()->infoUpdated();
}

void Util::switchAway() {
	setAway(!away);
}

string Util::getTempPath() {
#ifdef _WIN32
	TCHAR buf[MAX_PATH + 1];
	DWORD x = GetTempPath(MAX_PATH, buf);
	return Text::fromT(tstring(buf, x));
#else
	return "/tmp/";
#endif
}

string Util::formatRegExp(const string& msg, ParamMap& params) {
		string result = msg;
		string::size_type i, j, k;
		i = 0;
		while (( j = result.find("%[", i)) != string::npos) {
			if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
				break;
			}
			string name = result.substr(j + 2, k - j - 2);
			ParamMap::iterator smi = params.find(name);
			if(smi != params.end()) {
				result.replace(j, k-j + 1, (smi->second));
				i = j + (smi->second).size();
			} else {
				i = k + 1;
			}
		}
		return result;
}

bool Util::fileExists(const string& aFile) {
    #ifndef _WIN32
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
  #else
  //TODO: find correct ver for WINdoze
  return !(File::getSize(aFile) == -1);
  #endif
}

string Util::getBackupTimeString(time_t t /*= time(NULL) */ ) {
 	char buf[255];
 	tm* _tm = localtime(&t);
 	if(_tm == NULL) {
 		strcpy(buf, "xx:xx");
 	} else {
 		strftime(buf, 254, SETTING(BACKUP_TIMESTAMP).c_str(), _tm);
 	}
 	return buf;
}

string Util::convertCEscapes(string tmp)
{
	string::size_type i = 0;
	while( (i = tmp.find('\\', i)) != string::npos) {
		switch(tmp[i + 1]) {
			case '\0':
			{
				return tmp;
			}
			case 'a': tmp.replace(i, 2, "\a"); break;
			case 'b': tmp.replace(i, 2, "\b"); break;
			case 'e': tmp.replace(i, 2, "\033"); break;
			case 'f': tmp.replace(i, 2, "\f"); break;
			case 'n': tmp.replace(i, 2, "\n"); break;
			case 'r': tmp.replace(i, 2, "\r"); break;
			case 't': tmp.replace(i, 2, "\t"); break;
			case 'v': tmp.replace(i, 2, "\v"); break;
			case '\\': tmp.replace(i, 2, "\\"); break;
			case 'x':
			{
				if(i < tmp.length() - 3) {
					int num = strtol(tmp.substr(i + 2, 2).c_str(), NULL, 16);
					tmp.replace(i, 4, string(1, (char)num));
				}
				break;
			}
			default:
				if(tmp[i + 1] >= '0' && tmp[i + 1] <= '7') {
					int c = 1;
					if(tmp[i + 2] >= '0' && tmp[i + 2] <= '7') {
						++c;
						if(tmp[i + 1] <= '3' && tmp[i + 3] >= '0' && tmp[i + 3] <= '7')
							++c;
					}
					int num = strtol(tmp.substr(i + 1, c).c_str(), NULL, 8);
					tmp.replace(i, c + 1, string(1, (char)num));
				}
		}
		i += 1;
	}
	return tmp;
}


string Util::getIETFLang() {
#ifdef _WIN32
	string lang = SETTING(LANGUAGE);
	/*if(lang.empty()) {
		string lang = _nl_locale_name_default();
	}*/
	if(lang.empty() || lang == "C") {
		lang = "en-US";
	}

	// replace separation signs by hyphens.
	size_t i = 0;
	while((i = lang.find_first_of("_@.", i)) != string::npos) {
		lang[i] = '-';
		++i;
	}

	return lang;
#else
	return string(g_get_language_names()[0]);
#endif
}

bool Util::isIp6(const string& name)
{
	//5 = :xxxx:xxxx:
	if(name.empty()) return false;
	size_t n = std::count(name.begin(), name.end(), ':');
	if( (n==2) && (name.size() == 2) ) return true;//Fix for "::"
	if( n < 2)
			return false;

	bool ok = false;
	for(auto i = name.begin();i!=name.end();++i) {
			if(*i==':') {//cechk this
				for(int j = 5; j>0;--j){
						if(isxdigit(name[j])){ok = true;}
				}
		}
			if(ok){break;}
	}
	bool ok2 = false;
	for(auto i = name.end();i!=name.begin();--i) {
			if(*i==':') {//check
				for(int q = 0; q<5;++q){
						if(isxdigit(name[q])){ok2 = true;}
				}
			}
		if(ok2) {break;}
	}
	bool isOkIpV6 = false;
	if( (ok == true ) || (ok2 == true)) {
		struct sockaddr_in sa;
		#ifdef _WIN32
		int result = Socket::inet_pton(name.c_str() , &(sa.sin_addr));//6
		#else
		int result = inet_pton(AF_INET6,name.c_str() , &(sa.sin_addr));//6
		#endif
		isOkIpV6 = result == 1;
	}

	if(isOkIpV6)
	{
		return isOkIpV6;
	}
	return false;
}

string Util::trimUrl(string url)
{
	string currentUrl = url;
		// Trim spaces
		while(currentUrl[0] == ' ')
			currentUrl.erase(0, 1);
		while(currentUrl[currentUrl.length() - 1] == ' ') {
			currentUrl.erase(currentUrl.length()-1);
		}
		return currentUrl;
}



} // namespace dcpp
