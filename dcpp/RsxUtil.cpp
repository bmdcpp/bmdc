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

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "version.h"

#include "RsxUtil.h"
#include "Util.h"
#include "Text.h"
#include "ResourceManager.h"
//#include "rsxppSettingsManager.h"

namespace dcpp {

string RsxUtil::tmpTestSur;

const string defaultTestSURName = "TestSUR";

void RsxUtil::init() {
	generateTestSURString();
}

void RsxUtil::uinit() {
	//...
}

bool RsxUtil::checkVersion(const string& tag) {
	const char* aTag = tag.c_str();
	if(strncmp(aTag, "<++ V:0.69", 10) == 0) {
		return true;
	} else if(strncmp(aTag, "<++ V:0.7", 9) == 0) {
		return true;
	}
	return false;
}

/*
string RsxUtil::toIP(const uint32_t ipnum) {
	return Util::toString((ipnum / 16777216) % 256) + '.' + Util::toString((ipnum / 65536) % 256) + '.' + Util::toString((ipnum / 256) % 256) + '.' + Util::toString(ipnum % 256);
}
uint32_t RsxUtil::toIpNumber(const string& aIp) {
	// you must supply a valid ip!!
	string::size_type a = aIp.find('.');
	string::size_type b = aIp.find('.', a+1);
	string::size_type c = aIp.find('.', b+2);

	return (Util::toUInt32(aIp.c_str()) << 24) |
		(Util::toUInt32(aIp.c_str() + a + 1) << 16) |
		(Util::toUInt32(aIp.c_str() + b + 1) << 8) |
		(Util::toUInt32(aIp.c_str() + c + 1) );
}
bool RsxUtil::isIpInRange(const string& aIp, const string& aRange) {
	string::size_type j = aRange.find('-') + 1;
	if(j == string::npos)
		return false;
	uint32_t upper = toIpNumber(aRange.substr(j, aRange.size() - j));
	uint32_t lower = toIpNumber(aRange.substr(0, aRange.find('-')));
	uint32_t ip = toIpNumber(aIp);
	if(lower <= ip && ip <= upper)
		return true;
	return false;
}
uint32_t RsxUtil::getUpperRange(const string& aRange) {
	string::size_type j = aRange.find('-') + 1;
	return toIpNumber(aRange.substr(j, aRange.size() - j));
}

uint32_t RsxUtil::getLowerRange(const string& aRange) {
	return toIpNumber(aRange.substr(0, aRange.find('-')));
}
*/
void RsxUtil::generateTestSURString() {
	string tmp;
	tmp.reserve(3);
	for(int i = 0; i < 3; i++) {
		tmp.append(1, (char)Util::rand('a', 'z'));
	}
	tmp = tmp + Util::toString(Util::rand(1, 99));
	tmpTestSur = tmp;
}

const string& RsxUtil::getTestSURString() {
	return tmpTestSur.empty() ? defaultTestSURName : tmpTestSur;
}

string RsxUtil::getUpdateFileNames(const int number) {
	switch(number) {
		case 1: return "UserInfoProfiles.xml";
		case 2: return "IPWatch.xml";
		default: return "Profiles.xml";
	}
}
/*
bool RsxUtil::toBool(const string& aString) {
	return (Util::toInt(aString) >= 1 ? true : false);
}

bool RsxUtil::toBool(const int aNumber) {
	return (aNumber > 0 ? true : false);
}

void RsxUtil::trim(string& source, const string& delims = " \t\r\n") {
	string result(source);
	string::size_type index = result.find_last_not_of(delims);

	if(index != std::string::npos) {
		result.erase(++index);
	}

	index = result.find_first_not_of(delims);
	if(index != std::string::npos) {
		result.erase(0, index);
	} else {
		result.erase();
	}
	source = result;
}

tstring RsxUtil::replace(const tstring& aString, const tstring& fStr, const tstring& rStr) {
	tstring tmp = aString;
	tstring::size_type pos = 0;
	while( (pos = tmp.find(fStr, pos)) != tstring::npos ) {
		tmp.replace(pos, fStr.length(), rStr);
		pos += rStr.length() - fStr.length();
	}

	return tmp;
}

string RsxUtil::replace(const string& aString, const string& fStr, const string& rStr) {
	string tmp = aString;
	string::size_type pos;
	while( (pos = tmp.find(fStr)) != string::npos ) {
		tmp.replace(pos, fStr.length(), rStr);
		pos += rStr.length() - fStr.length();
	}

	return tmp;
}

bool RsxUtil::compareLower(const string& firstStr, const string& secondStr) {
	if(Text::toLower(firstStr).compare(Text::toLower(secondStr)) == 0) {
		return true;
	} else {
		return false;
	}
}

bool RsxUtil::compareLower(const tstring& firstStr, const tstring& secondStr) {
	if(Text::toLower(firstStr).compare(Text::toLower(secondStr)) == 0) {
		return true;
	} else {
		return false;
	}
}
//http://www.codeproject.com/tips/JbColorContrast.asp
#define abs(x) ((x) < 0 ? (-(x)) : (x))
#define calc_col(x) abs(((x) & 0xFF) - 0x80) <= TOLERANCE
#define TOLERANCE 0x40

int RsxUtil::CalcContrastColor(int crBg) {
    if(calc_col(crBg) && calc_col(crBg >> 8) && calc_col(crBg >> 16))
		return (0x7F7F7F + crBg) & 0xFFFFFF;
    else
		return crBg ^ 0xFFFFFF;
}

tstring RsxUtil::formatAdditionalInfo(const string& aIp, bool sIp, bool sCC) {
	string ret = Util::emptyString;

	if(!aIp.empty()) {
		string cc = Util::getIpCountry(aIp);
		bool showIp = RSXPP_BOOLSETTING(IP_IN_CHAT) || sIp;
		bool showCc = (RSXPP_BOOLSETTING(COUNTRY_IN_CHAT) || sCC) && !cc.empty();

		if(showIp) {
			ret = "[ " + aIp + " ] ";
		}
		if(showCc) {
			ret += "[ " + cc + " ] ";
		}
	}
	return Text::toT(ret);
}

#ifdef _WIN64
#define CONFIGURATION_TYPE "x86-64"
#else
#define CONFIGURATION_TYPE "x86-32"
#endif

tstring RsxUtil::getWndTitle() {
#ifdef SVNBUILD
	return _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(CONFIGURATION_TYPE) _T(" [SVN:") _T(BOOST_STRINGIZE(SVN_REVISION)) _T(" / ") _T(DCVERSIONSTRING) _T(" / ") _T(SVNVERSION) _T("]");
#elif _DEBUG
	return _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(CONFIGURATION_TYPE) _T(" DEBUG [SVN:") _T(BOOST_STRINGIZE(SVN_REVISION)) _T(" / ") _T(DCVERSIONSTRING) _T(" / ") _T(SVNVERSION) _T("]");
#else
	return _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(CONFIGURATION_TYPE);
#endif
}*/
}; // namespace dcpp
