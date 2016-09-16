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

#include "stdinc.h"
#include "SettingsManager.h"
#include "HubSettings.h"
namespace dcpp {

HubSettings::HubSettings():
autoConnect(false),share(NULL)
{

}


HubSettings::~HubSettings()
{
	if(share != NULL)
	{
		if(!share->getName().empty())
			delete share;
	}
}

void HubSettings::merge(const HubSettings& sub) {
	
	for(auto i = sub.strings.begin(); i != sub.strings.end(); ++i) {
		strings[i->first] = i->second;
	}
	for(auto i = sub.ints.begin(); i != sub.ints.end(); ++i) {
		ints[i->first] = i->second;
	}
	for(auto i = sub.bools.begin(); i != sub.bools.end(); ++i) {
		bools[i->first] = i->second;
	}
	setAutoConnect(sub.getAutoConnect());
	if(sub.share != NULL)
		setShareManager(sub.share);
}

void HubSettings::load(SimpleXML& xml) {
	
	// convert old settings
	set(SettingsManager::NICK, xml.getChildAttrib("Nick"));
	set(SettingsManager::DESCRIPTION, xml.getChildAttrib("UserDescription"));
	set(SettingsManager::EMAIL, xml.getChildAttrib("Email"));
	//
	set(SettingsManager::EXTERNAL_IP, xml.getChildAttrib("UserIp"));
	set(SettingsManager::HUB_TEXT_STR, xml.getChildAttrib("TabText"));
	set(SettingsManager::HUB_ICON_STR, xml.getChildAttrib("TabIcon"));
	//TODO:convert others
	xml.stepIn();
		
	for(int i = SettingsManager::STR_FIRST; i < SettingsManager::STR_LAST; i++)
	{
		const string& attr = SettingsManager::settingTags[i];
		if(xml.findChild(attr))
			set(SettingsManager::StrSetting(i), xml.getChildData());
		xml.resetCurrentChild();
	}
	for(int i = SettingsManager::INT_FIRST; i < SettingsManager::INT_LAST; i++)
	{
		const string& attr = SettingsManager::settingTags[i];
		if(xml.findChild(attr))
			set(SettingsManager::IntSetting(i), Util::toInt(xml.getChildData()));
		xml.resetCurrentChild();
	}
	for(int i = SettingsManager::BOOL_FIRST; i < SettingsManager::BOOL_LAST; i++)
	{
		const string& attr = SettingsManager::settingTags[i];
		if(xml.findChild(attr))
			set(SettingsManager::BoolSetting(i), Util::toInt(xml.getChildData()) > 0);
		xml.resetCurrentChild();
	}
	
	if(xml.findChild("AutoConnect"))
		setAutoConnect(Util::toInt(xml.getChildData()));
	
	xml.stepOut();
	xml.stepIn();
	if(xml.findChild("Share")) {
		setShareManager(new ShareManager(url));
		share->load(xml);
		share->refresh(true,false,true);
	}	
	xml.stepOut();
}

void HubSettings::save(SimpleXML& xml) const {
	
	xml.stepIn();

	string type("type"), curType("string");
	for(auto i = strings.begin(); i != strings.end(); ++i) {
		xml.addTag(SettingsManager::settingTags[i->first], i->second);
		xml.addChildAttrib(type, curType);
	}

	curType = "int";
	for(auto i = ints.begin(); i != ints.end(); ++i) {
		xml.addTag(SettingsManager::settingTags[i->first], i->second);
		xml.addChildAttrib(type, curType);
	}

	curType = "bool";
	for(auto i = bools.begin(); i != bools.end(); ++i) {
		xml.addTag(SettingsManager::settingTags[i->first], i->second);
		xml.addChildAttrib(type, curType);
	}
	xml.addTag("AutoConnect",getAutoConnect());
	xml.addChildAttrib(type,curType);
	
	xml.stepOut();
	xml.stepIn();
	if(share != NULL)
	{
		share->save(xml);
	}
	xml.stepOut();
	
}

HubSettings& HubSettings::operator=(const HubSettings& rhs)
{
	strings = rhs.strings;
	ints = rhs.ints;
	bools = rhs.bools;
	share = rhs.share;
	autoConnect = rhs.autoConnect;
	url = rhs.url;
	return *this;
}

const string HubSettings::get(SettingsManager::StrSetting key, const string& defValue) const
{
	auto i = strings.find(key);
	return (i == strings.end()) ? defValue : i->second;
}

int HubSettings::get(SettingsManager::IntSetting key, int defValue) const
{
	auto i = ints.find(key);
	return (i == ints.end()) ? defValue : i->second;
}

bool HubSettings::get(SettingsManager::BoolSetting key, bool defValue)
{
	auto i = bools.find(key);
	return (i == bools.end()) ? defValue : (i->second != 0);
}

void HubSettings::set(SettingsManager::StrSetting key, string const& value)
{
	// clearing the string will set up the default value (but to set empty strings???)
	if(value.empty() || value == SettingsManager::getInstance()->get(key))	
		strings.erase(key);
	else
		strings[key] = value;
}

void HubSettings::set(SettingsManager::IntSetting key, const string& value)
{
	auto intValue = value.empty() ? 0 : Util::toInt(value);
	set(key, intValue);
}

void HubSettings::set(SettingsManager::IntSetting key, int value)
{
	if(value == SettingsManager::getInstance()->get(key))
		ints.erase(key);
	else
		ints[key] = value;
}

void HubSettings::set(SettingsManager::BoolSetting key, bool value)
{
	if(value == SettingsManager::getInstance()->get(key))
		bools.erase(key);
	else
		bools[key] = value;
}


ShareManager* HubSettings::getShareManager() const {
	return (share == NULL || share->getName().empty()) ? ShareManager::getInstance() : share;
}

} // namespace dcpp
