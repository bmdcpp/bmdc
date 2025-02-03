/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "version.h"
#include <cstring>
#include "BMDCUtil.h"
#include "Util.h"
#include "Text.h"

namespace dcpp {

string bmUtil::tmpTestSur;

const string defaultTestSURName = "TestSUR";

void bmUtil::init() {
	generateTestSURString();
}

bool bmUtil::checkVersion(const string& tag) {
	const char* aTag = tag.c_str();
	if(strncmp(aTag, "<++ V:0.69", 10) == 0) {
		return true;
	} else if(strncmp(aTag, "<++ V:0.7", 9) == 0) {
		return true;
	}
	return false;
}

uint32_t bmUtil::toIpNumber(const string& aIp) {
	// you must supply a valid ip!!
	string::size_type a = aIp.find('.');
	string::size_type b = aIp.find('.', a+1);
	string::size_type c = aIp.find('.', b+2);

	return (Util::toUInt32(aIp.c_str()) << 24) |
		(Util::toUInt32(aIp.c_str() + a + 1) << 16) |
		(Util::toUInt32(aIp.c_str() + b + 1) << 8) |
		(Util::toUInt32(aIp.c_str() + c + 1) );
}
bool bmUtil::isIpInRange(const string& aIp, const string& aRange) {
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

void bmUtil::generateTestSURString() {
	string tmp;
	tmp.reserve(3);
	for(int i = 0; i < 3; i++) {
		tmp.append(1, (char)Util::rand('a', 'z'));
	}
	tmp = tmp + Util::toString(Util::rand(1, 99));
	tmpTestSur = tmp;
}

const string& bmUtil::getTestSURString() {
	return tmpTestSur.empty() ? defaultTestSURName : tmpTestSur;
}

}; // namespace dcpp
