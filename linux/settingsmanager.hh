/*
 * Copyright © 2004-2017 Jens Oknelid, paskharen@gmail.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#ifndef _BMDC_SETTINGSMANAGER_HH
#define _BMDC_SETTINGSMANAGER_HH

#include <string>
#include <map>
#include <vector>
#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/Singleton.h"

#define WSET(key, value) WulforSettingsManager::getInstance()->set(key, value)
#define WGETI(key) WulforSettingsManager::getInstance()->getInt(key)
#define WGETS(key) WulforSettingsManager::getInstance()->getString(key)
#define WGETB(key) WulforSettingsManager::getInstance()->getBool(key)
#define WSCMD(key) WulforSettingsManager::getInstance()->parseCmd(key)

/* default font theme */
#define TEXT_WEIGHT_NORMAL PANGO_WEIGHT_NORMAL
#define TEXT_WEIGHT_BOLD   PANGO_WEIGHT_BOLD
#define TEXT_STYLE_NORMAL  PANGO_STYLE_NORMAL
#define TEXT_STYLE_ITALIC  PANGO_STYLE_ITALIC

class PreviewApp
{
	public:

	typedef std::vector<PreviewApp*> List;
	typedef List::size_type size;
	typedef List::const_iterator Iter;

	PreviewApp(std::string name, std::string app, std::string ext) : name(name), app(app), ext(ext) {}
	~PreviewApp() {}

	std::string name;
	std::string app;
	std::string ext;
};

class WulforSettingsManager : public dcpp::Singleton<WulforSettingsManager>
{
	public:
		typedef std::map<std::string, int> IntMap;
		typedef std::map<std::string, std::string> StringMap;

		WulforSettingsManager();
		virtual ~WulforSettingsManager();

		const std::string parseCmd(const std::string cmd);

		int getInt(const std::string &key, bool useDefault = false);
		bool getBool(const std::string &key, bool useDefault = false);
		std::string getString(const std::string &key, bool useDefault = false);
		void set(const std::string &key, int value);
		void set(const std::string &key, bool value);
		void set(const std::string &key, const std::string &value);
		void load();
		void save();

		PreviewApp* applyPreviewApp(std::string &oldName, std::string &newName, std::string &app, std::string &ext);
		PreviewApp* addPreviewApp(std::string name, std::string app, std::string ext);
		bool getPreviewApp(std::string &name, PreviewApp::size &index);
		bool getPreviewApp(std::string &name);
		bool removePreviewApp(std::string &name);

		const PreviewApp::List& getPreviewApps() const {return previewApps;}
		//[BMDC
		IntMap getIntDMap() { return defaultInt; }
		StringMap getStringDMap() { return defaultString; }
		IntMap getIntMap() { return intMap; }
		StringMap getStringMap() { return stringMap; }

		bool isDefaultString(std::string name){
			auto a = stringMap.find(name);
			auto d = defaultString.find(name);
			if(d->second == a->second)
				return true;
			return false;
		}

		bool isDefaultInt(std::string name){
			auto a = intMap.find(name);
			auto d = defaultInt.find(name);
			if(d->second == a->second)
				return true;
			return false;
		}

		void SetIntDef(std::string name)
		{
			auto d = defaultInt.find(name);
			auto i = intMap.find(name);
			intMap.erase(i);
			intMap.insert(IntMap::value_type(name,d->second));
		}
		void SetStringDef(std::string name)
		{
			auto d = defaultString.find(name);
			auto i = stringMap.find(name);
			stringMap.erase(i);
			stringMap.insert(StringMap::value_type(name,d->second));
		}
		bool isInt(std::string name) { return defaultInt.find(name) != defaultInt.end(); }
		bool isString(std::string name) { return defaultString.find(name) != defaultString.end(); }
	private:

		friend class dcpp::Singleton<WulforSettingsManager>;
		IntMap intMap;
		StringMap stringMap;
		IntMap defaultInt;
		StringMap defaultString;
		std::string configFile;

		PreviewApp::List previewApps;
};

#else
class WulforSettingsManager;
#endif
