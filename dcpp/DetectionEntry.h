/*
 * Copyright (C) 2007-2017 adrian_007, adrian-007 on o2 point pl
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

#ifndef DETECTION_ENTRY_H
#define DETECTION_ENTRY_H

#include <deque>
#include "Flags.h"

namespace dcpp {

class DetectionEntry {
public:
	typedef std::deque<pair<std::string, std::string> > INFMap;

	enum {
		GREEN = 1,
		YELLOW,
		RED
	};

	DetectionEntry() : Id(0), name(), cheat(), comment(), rawToSend(0), clientFlag(1), checkMismatch(false), isEnabled(true) { };
	~DetectionEntry() { defaultMap.clear(); adcMap.clear(); nmdcMap.clear(); };

	INFMap defaultMap;
	INFMap adcMap;
	INFMap nmdcMap;

	string name;
	string cheat;
	string comment;
	size_t Id;
	uint32_t rawToSend;
	uint32_t clientFlag;
	bool checkMismatch;
	bool isEnabled;

};

} // namespace dcpp

#endif // DETECTION_ENTRY_H

/**
 * @file
 * $Id: DetectionEntry.h 141 2009-08-10 00:06:13Z adrian_007 $
 */
