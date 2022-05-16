/*
 * Copyright (C) 2001-2018 Jacek Sieka, arnetheduck on gmail point com
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
// Some CODE Under GPL by BMDC-Team (Mank)

#include "stdinc.h"
#include "GeoManager.h"

#include "GeoIPc.h"
#include "Util.h"

namespace dcpp {

void GeoManager::init() {
	geo.reset(new GeoIP(getDbPath()));
}

void GeoManager::update() {
	if(geo) {
		geo->update();
	}
}

void GeoManager::close() {
	geo.reset();
}

const string GeoManager::getCountry(const string& ip) {
	
	if(!ip.empty()) {
		dcdebug("%s",geo->getCountry(ip).c_str());
		return geo->getCountry(ip);
	}

	return Util::emptyString;
}

const string GeoManager::getCountryAbbrevation(const string& ip)
{
	if(!ip.empty())
	{
		dcdebug("%s",geo->getCountryAB(ip).c_str());
		return geo->getCountryAB(ip);
	}
	return Util::emptyString;
}

const string GeoManager::getAnyInfo(const string ip, ...)
{
	va_list keys;
	va_start (keys, ip);
	string ret = geo->GetAnyInfoItem(ip,keys);
	va_end(keys);
	return ret;
	
}
string GeoManager::getDbPath() {
	return Util::getPath(Util::PATH_USER_LOCAL) + "GeoLite2-Country.mmdb";
}

} // namespace dcpp
