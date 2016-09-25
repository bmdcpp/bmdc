/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_GEOIP_H
#define DCPLUSPLUS_DCPP_GEOIP_H

#include "CriticalSection.h"
#include <string>
#include <vector>

typedef struct GeoIPTag GeoIP;

namespace dcpp {

using std::string;
using std::vector;

class GeoIP {
public:
	explicit GeoIP(string&& _path);
	~GeoIP();

	const string& getCountry(const string& ip) const;
	const string getCountryAB(const string& ip) const;
	void update();
	void rebuild();

private:
#ifdef _WIN32
	bool decompress() const;
#endif	
	void open();
	void close();
	bool v6() const;

	mutable CriticalSection cs;
	::GeoIP* geo;
	vector<string> cache;
	const string path;

	GeoIP(GeoIP&);
	GeoIP operator=(GeoIP&);
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_GEOIP_H)
