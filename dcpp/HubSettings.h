/*
 * Copyright (C) 2001-2016 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_HUBSETTINGS_H
#define DCPLUSPLUS_DCPP_HUBSETTINGS_H

#include <string>
#include <map>
#include "SimpleXML.h"
#include "GetSet.h"
#include "SettingsManager.h"

namespace dcpp {

using std::string;
using std::map;

/** Stores settings to be applied to a hub. There are 3 HubSettings levels in DC++: global; per
favorite hub group; per favorite hub entry. */
struct HubSettings
{
	HubSettings();
	HubSettings(const HubSettings& rhs) { *this = rhs; }
	HubSettings& operator=(const HubSettings& rhs);
	~HubSettings() { }

	const string& get(SettingsManager::StrSetting key, const std::string& defValue) const;
	int get(SettingsManager::IntSetting key, int defValue) const;
	bool get(SettingsManager::BoolSetting key, bool defValue);

	void set(SettingsManager::StrSetting key, string const& value);
	void set(SettingsManager::IntSetting key, const string& value);
	void set(SettingsManager::IntSetting key, int value);
	void set(SettingsManager::BoolSetting key, bool value);

	/** Apply a set of sub-settings that may override current ones. */
	void merge(const HubSettings& sub);

	void load(SimpleXML& xml);
	void save(SimpleXML& xml) const;
	GETSET(bool, autoConnect, AutoConnect);//used by group

private:
	map<SettingsManager::StrSetting, string> strings;
	map<SettingsManager::IntSetting, int> ints;
	map<SettingsManager::BoolSetting, bool> bools;
};

#define HUBSETTING(a) get(SettingsManager::a, SettingsManager::getInstance()->get(SettingsManager::a))
} // namespace dcpp

#endif
