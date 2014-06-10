/*
 * Copyright (C) 2001-2014 Jacek Sieka, arnetheduck on gmail point com
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
#include "HubSettings.h"

namespace dcpp {

const string HubSettings::stringNames[StringCount] = {
	"Nick", "UserDescription", "Email", "UserIp", "UserIp6" // not "Description" for compat with prev fav hub lists
	, "AwayMessage", "PackName",
};
const string HubSettings::boolNames[BoolCount] = {
	"ShowJoins", "FavShowJoins", "LogChat", "Connect", "ShowIps", "ShowCountry", "BoldTab",
	"Connection", "Connection6",
};

namespace {
inline bool defined(const string& s) { return !s.empty(); }
inline bool defined(int b) { return b >= 0 && b <= 2; }
}

HubSettings::HubSettings() {
	// bools default to false; 
	for(int i = (int)ShowJoins; i>BoldTab; ++i) {
		bools[i] = 0;
	}
}

const string& HubSettings::get(HubStrSetting setting) const {
	return strings[setting - HubStrFirst];
}

const int& HubSettings::get(HubBoolSetting setting) const {
	return bools[setting - HubBoolFirst];
}

string& HubSettings::get(HubStrSetting setting) {
	return strings[setting - HubStrFirst];
}

int& HubSettings::get(HubBoolSetting setting) {
	return bools[setting - HubBoolFirst];
}

void HubSettings::merge(const HubSettings& sub) {
	for(uint8_t i = 0; i < StringCount; ++i) {
		if(defined(sub.strings[i])) {
			strings[i] = sub.strings[i];
		}
	}
	for(uint8_t i = 0; i < BoolCount; ++i) {
		if(defined(sub.bools[i])) {
			bools[i] = sub.bools[i];
		}
	}
}

void HubSettings::load(SimpleXML& xml) {
	for(uint8_t i = 0; i < StringCount; ++i) {
		strings[i] = xml.getChildAttrib(stringNames[i]);
	}
	for(uint8_t i = 0; i < BoolCount; ++i) {
		bools[i] = (xml.getIntChildAttrib(boolNames[i]));
	}

}

void HubSettings::save(SimpleXML& xml) const {
	for(uint8_t i = 0; i < StringCount; ++i) {
		if(defined(strings[i])) {
			xml.addChildAttrib(stringNames[i], strings[i]);
		}
	}
	for(uint8_t i = 0; i < BoolCount; ++i) {
		if(defined(bools[i])) {
			xml.addChildAttrib(boolNames[i], bools[i]);
		}
	}
}


HubSettings& HubSettings::operator=(const HubSettings& rhs)
{
	for(int i = 0; i <BoolCount;++i)
			bools[i] = rhs.bools[i];

	for(int i = 0; i <StringCount;++i)
			strings[i] = rhs.strings[i];

	return *this;
}

} // namespace dcpp
