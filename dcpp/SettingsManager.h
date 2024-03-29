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

#ifndef DCPLUSPLUS_DCPP_SETTINGS_MANAGER_H
#define DCPLUSPLUS_DCPP_SETTINGS_MANAGER_H

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"
#include "Exception.h"

namespace dcpp {

STANDARD_EXCEPTION(SearchTypeException);

class SimpleXML;

class SettingsManagerListener {
public:
	virtual ~SettingsManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Load;
	typedef X<1> Save;
	typedef X<2> SearchTypesChanged;

	virtual void on(Load, SimpleXML&)  { }
	virtual void on(Save, SimpleXML&)  { }
	virtual void on(SearchTypesChanged) noexcept { }
};

class SettingsManager : public Singleton<SettingsManager>, public Speaker<SettingsManagerListener>
{
public:
	//const string name = "MainConfig";
	typedef std::unordered_map<string, StringList> SearchTypes;
	typedef std::map<std::string, std::string> StringMap;

	enum Types {
		TYPE_STRING,
		TYPE_INT,
		TYPE_BOOL,
		TYPE_INT64,
		TYPE_FLOAT,
		TYPE_NONE
	};

	static StringList connectionSpeeds;

	enum StrSetting { STR_FIRST,//0
		NICK = STR_FIRST,/*0*/ UPLOAD_SPEED, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, EXTERNAL_IP, EXTERNAL_IP6,
		CONNECTIONS_ORDER, CONNECTIONS_WIDTHS, HUBFRAME_ORDER, HUBFRAME_WIDTHS,
		SEARCHFRAME_ORDER, SEARCHFRAME_WIDTHS, FAVHUBSFRAME_ORDER, FAVHUBSFRAME_WIDTHS,
		HUBLIST_SERVERS, QUEUEFRAME_ORDER, QUEUEFRAME_WIDTHS, PUBLICHUBSFRAME_ORDER, PUBLICHUBSFRAME_WIDTHS,
		FINISHED_DL_FILES_ORDER, FINISHED_DL_FILES_WIDTHS, FINISHED_DL_USERS_ORDER, FINISHED_DL_USERS_WIDTHS,
		FINISHED_UL_FILES_ORDER, FINISHED_UL_FILES_WIDTHS, FINISHED_UL_USERS_ORDER, FINISHED_UL_USERS_WIDTHS,
		USERSFRAME_ORDER, USERSFRAME_WIDTHS, HTTP_PROXY,/*30*/ LOG_DIRECTORY, LOG_FORMAT_POST_DOWNLOAD,
		LOG_FORMAT_POST_FINISHED_DOWNLOAD, LOG_FORMAT_POST_UPLOAD, LOG_FORMAT_MAIN_CHAT, LOG_FORMAT_PRIVATE_CHAT,
		TEMP_DOWNLOAD_DIRECTORY, BIND_ADDRESS, BIND_ADDRESS6, SOCKS_SERVER, SOCKS_USER, SOCKS_PASSWORD, CONFIG_VERSION,
		DEFAULT_AWAY_MESSAGE, TIME_STAMPS_FORMAT, COUNTRY_FORMAT, ADLSEARCHFRAME_ORDER, ADLSEARCHFRAME_WIDTHS,
		PRIVATE_ID, SPYFRAME_WIDTHS, SPYFRAME_ORDER, LOG_FILE_MAIN_CHAT,
		LOG_FILE_PRIVATE_CHAT, LOG_FILE_STATUS, LOG_FILE_UPLOAD, LOG_FILE_DOWNLOAD, LOG_FILE_FINISHED_DOWNLOAD,
		LOG_FILE_SYSTEM, LOG_FORMAT_SYSTEM, LOG_FORMAT_STATUS,/*60*/ DIRECTORYLISTINGFRAME_ORDER, DIRECTORYLISTINGFRAME_WIDTHS,
		TLS_PRIVATE_KEY_FILE, TLS_CERTIFICATE_FILE, TLS_TRUSTED_CERTIFICATES_PATH,
		LANGUAGE, DOWNLOADS_ORDER, DOWNLOADS_WIDTHS, TOOLBAR, LAST_SEARCH_TYPE, MAPPER, MAPPER6,
		SOUND_MAIN_CHAT, SOUND_PM, SOUND_PM_WINDOW, SOUND_FINISHED_DL, SOUND_FINISHED_FL, LAST_SHARED_FOLDER,
		ACFRAME_ORDER, ACFRAME_WIDTHS, SHARING_SKIPLIST_REGEX,
		SHARING_SKIPLIST_EXTENSIONS, SHARING_SKIPLIST_PATHS,
		//[BMDC++
		BACKUP_FILE_PATTERN,
		LOG_FILE_RAW, LOG_FORMAT_RAW, PROTECTED_USERS, BACKUP_TIMESTAMP, EMOT_PACK, RIPE_DB,
		HUB_ICON_STR, HUB_TEXT_STR, HUB_UL_ORDER, HUB_UL_VISIBLE, HUB_UL_SIZE, CHAT_EXTRA_INFO,
		BACKGROUND_CHAT_COLOR, BACKGROUND_CHAT_IMAGE, BACKGROUND_PM_COLOR, BACKGROUND_PM_IMAGE,
		RATIO_TEMPLATE, LOG_FILE_PROTO, LOG_FORMAT_PROTO,
		WILDCARD_FOR_EXPORT_SET,
		STR_LAST };//105

		//105
	enum IntSetting { INT_FIRST = STR_LAST + 1,//106
		INCOMING_CONNECTIONS = INT_FIRST,/*107*/ OUTGOING_CONNECTIONS, TCP_PORT, UDP_PORT, TLS_PORT,
		SOCKS_PORT, SOCKET_IN_BUFFER, SOCKET_OUT_BUFFER,
//112
		TEXT_COLOR, BACKGROUND_COLOR, UPLOAD_TEXT_COLOR, UPLOAD_BG_COLOR, DOWNLOAD_TEXT_COLOR,
		DOWNLOAD_BG_COLOR, LINK_COLOR, LOG_COLOR,//122

		BANDWIDTH_LIMIT_START, BANDWIDTH_LIMIT_END, MAX_DOWNLOAD_SPEED_ALTERNATE,
		MAX_UPLOAD_SPEED_ALTERNATE, MAX_DOWNLOAD_SPEED_MAIN, MAX_UPLOAD_SPEED_MAIN,
		SLOTS_ALTERNATE_LIMITING, SLOTS_PRIMARY,//130

		MAIN_WINDOW_STATE, MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y, MAIN_WINDOW_POS_X,
		MAIN_WINDOW_POS_Y, SETTINGS_WIDTH, SETTINGS_HEIGHT, SETTINGS_PAGE,//138

		PRIO_HIGHEST_SIZE, PRIO_HIGH_SIZE, PRIO_NORMAL_SIZE, PRIO_LOW_SIZE, AUTODROP_SPEED,
		AUTODROP_INTERVAL, AUTODROP_ELAPSED, AUTODROP_INACTIVITY, AUTODROP_MINSOURCES,
		AUTODROP_FILESIZE,//148

		BALLOON_MAIN_CHAT, BALLOON_PM, BALLOON_PM_WINDOW, BALLOON_FINISHED_DL, BALLOON_FINISHED_FL,//153
//46
		// uncategorized
		AWAY_IDLE, AUTO_REFRESH_TIME, AUTO_SEARCH_LIMIT, BUFFER_SIZE, DOWNLOAD_SLOTS,
		HUB_LAST_LOG_LINES, MAGNET_ACTION, MAX_COMMAND_LENGTH, MAX_COMPRESSION, MAX_DOWNLOAD_SPEED,
		MAX_FILELIST_SIZE, MAX_HASH_SPEED, MAX_MESSAGE_LINES, MAX_PM_WINDOWS, MIN_MESSAGE_LINES,
		MIN_UPLOAD_SPEED, PM_LAST_LOG_LINES, SEARCH_HISTORY, SET_MINISLOT_SIZE,
		SETTINGS_SAVE_INTERVAL, SLOTS, TAB_STYLE, TAB_WIDTH, TOOLBAR_SIZE,//178
//70
		CHECK_DELAY,
		MAX_TESTSURS ,//180
		MAX_FILELISTS,
		SLEEP_TIME,
		AUTOBACKUP_TIME,
		TIME_RECCON,
		PERCENT_FAKE_SHARE_TOLERATED,
		FAKESHARE_RAW,
		MAX_DISCONNECTS,
		DISCONNECT_RAW,
		FILELIST_TOO_SMALL_BIG_RAW,
		MIN_FL_SIZE,
		LISTLEN_MISMATCH,
		FILELIST_NA_RAW,
		ADLSEARCH_DEFAULT_ACTION,
		MIN_POINTS_TO_DISPLAY_CHEAT,
		DETECTIONF,
		DETECTIONS,
		DCPP_EMULATION_RAW,
		VERSION_MISMATCH_RAW,//90
		SDL_RAW,
		SDL_SPEED,
		SDL_TIME,
		RMDC_RAW,
		FILELIST_VERSION_MISMATCH,
		ADL_RAW,
		NICK_PANE_POS,
		INT_LAST };
//206
	enum BoolSetting { BOOL_FIRST = INT_LAST + 1,
		ADD_FINISHED_INSTANTLY = BOOL_FIRST, ADLS_BREAK_ON_FIRST, ALT_SORT_ORDER,
		ALLOW_UNTRUSTED_CLIENTS, ALLOW_UNTRUSTED_HUBS, ALWAYS_TRAY, AUTO_AWAY,
		AUTO_DETECT_CONNECTION, AUTO_FOLLOW, AUTO_KICK, AUTO_KICK_NO_FAVS, AUTO_SEARCH,
		AUTO_SEARCH_AUTO_MATCH, AUTODROP_ALL, AUTODROP_DISCONNECT, AUTODROP_FILELISTS,
		AWAY_COMP_LOCK, BOLD_FINISHED_DOWNLOADS, BOLD_FINISHED_UPLOADS, BOLD_FL, BOLD_HUB, BOLD_PM,
		BOLD_QUEUE, BOLD_SEARCH, BOLD_SEARCH_SPY, BOLD_SYSTEM_LOG, CLEAR_SEARCH,
		COMPRESS_TRANSFERS, CONFIRM_ADLS_REMOVAL, CONFIRM_EXIT, CONFIRM_HUB_CLOSING,
		CONFIRM_HUB_REMOVAL, CONFIRM_ITEM_REMOVAL, CONFIRM_USER_REMOVAL, CORAL,
		DONT_DL_ALREADY_QUEUED, DONT_DL_ALREADY_SHARED, FAV_SHOW_JOINS, FILTER_MESSAGES,
		FINISHED_DL_ONLY_FULL, FOLLOW_LINKS, GET_USER_COUNTRY, GET_USER_INFO, HUB_USER_COMMANDS,
		IGNORE_BOT_PMS, IGNORE_HUB_PMS, JOIN_OPEN_NEW_WINDOW, KEEP_FINISHED_FILES, KEEP_LISTS,
		LIST_DUPES, LOG_DOWNLOADS, LOG_FILELIST_TRANSFERS, LOG_FINISHED_DOWNLOADS, LOG_MAIN_CHAT,
		LOG_PRIVATE_CHAT, LOG_STATUS_MESSAGES, LOG_SYSTEM, LOG_UPLOADS, MAGNET_ASK,
		MAGNET_REGISTER, MINIMIZE_TRAY, NO_AWAYMSG_TO_BOTS, NO_IP_OVERRIDE, OPEN_USER_CMD_HELP,
		OWNER_DRAWN_MENUS, POPUP_BOT_PMS, POPUP_HUB_PMS, POPUP_PMS, POPUNDER_FILELIST, POPUNDER_PM,
		PRIO_LOWEST, PROMPT_PASSWORD, QUEUEFRAME_SHOW_TREE, REQUIRE_TLS, SEARCH_FILTER_SHARED,
		SEARCH_MERGE, SEARCH_ONLY_FREE_SLOTS, SEGMENTED_DL, SEND_BLOOM, SEND_UNKNOWN_COMMANDS,
		SFV_CHECK, SHARE_HIDDEN, SHOW_JOINS, SHOW_MENU_BAR, SHOW_STATUSBAR, SHOW_TOOLBAR,
		SHOW_TRANSFERVIEW, SKIP_ZERO_BYTE, SOCKS_RESOLVE, SORT_FAVUSERS_FIRST,
		SPY_FRAME_IGNORE_TTH_SEARCHES, STATUS_IN_CHAT, TIME_DEPENDENT_THROTTLE, TIME_STAMPS,//100
		TOGGLE_ACTIVE_WINDOW, URL_HANDLER, USE_CTRL_FOR_LINE_HISTORY, USE_SYSTEM_ICONS,
		USERS_FILTER_FAVORITE, USERS_FILTER_ONLINE, USERS_FILTER_QUEUE, USERS_FILTER_WAITING,
		AC_DISCLAIM,

		//BMDC
		CHECK_ALL_CLIENTS_BEFORE_FILELISTS,
		SHOW_FREE_SLOTS_DESC,
		ENABLE_AUTOBACKUP,
		SHOW_FAKESHARE_RAW,
		LOG_RAW_CMD,
		SHOW_DISCONNECT,
		FILELIST_TOO_SMALL_BIG,
		LISTLEN_MISMATCH_SHOW,
		USE_SEND_DELAYED_RAW,
		SHOW_FILELIST_NA,
		SHOW_ADLSEARCH_DEFAULT_ACTION,
		THROTTLE_ENABLE,
		USE_IP,
		FAV_USER_IS_PROTECTED_USER,
		SHOW_DCPP_EMULATION,
		VERSION_MISMATCH,
		SHOW_RMDC,
		USE_SDL_KICK,
		UNCHECK_CLIENT_PROTECTED_USER,
		UNCHECK_LIST_PROTECTED_USER,
		SHOW_FILELIST_VERSION_MISMATCH,
		USE_WILDCARDS_TO_PROTECT,
		DISPLAY_CHEATS_IN_MAIN_CHAT,
		USE_OEM_MONOFONT, SERVER_COMMANDS,
		USE_AV_FILTER, LOG_CHAT_B,
		USE_COUNTRY_FLAG, USE_EMOTS,
		USE_HIGHLITING,
		USE_SOCK5,
		NO_IP_OVERRIDE6,
		BOOL_LAST };

	enum Int64Setting { INT64_FIRST = BOOL_LAST + 1,
		TOTAL_UPLOAD = INT64_FIRST, TOTAL_DOWNLOAD,
		SHARING_SKIPLIST_MINSIZE, SHARING_SKIPLIST_MAXSIZE,
		INT64_LAST };

	enum FloatSetting { FLOAT_FIRST = INT64_LAST +1,
		TRANSFERS_PANED_POS = FLOAT_FIRST, QUEUE_PANED_POS, SEARCH_PANED_POS,
		FLOAT_LAST, SETTINGS_LAST = FLOAT_LAST };

	enum {	INCOMING_DIRECT, INCOMING_FIREWALL_UPNP, INCOMING_FIREWALL_NAT,
		INCOMING_FIREWALL_PASSIVE };

	enum {	OUTGOING_DIRECT, OUTGOING_SOCKS5 };

	enum {	MAGNET_AUTO_SEARCH, MAGNET_AUTO_DOWNLOAD };
	
	
	StringMap defaultString;
	
	//const string get(string key,bool useDefault = true){
	//		if (!useDefault)
	//				return defaultString[key];

			//if (stringMap.find(key) == stringMap.end())
			//	return defaultString[key];
			//else
			//	return stringMap[key];
	//}
	
	//probably ref-to-var-ret is not good idea?
	const string& get(StrSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? strSettings[key - STR_FIRST] : strDefaults[key - STR_FIRST];
	}

	int get(IntSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? intSettings[key - INT_FIRST] : intDefaults[key - INT_FIRST];
	}
	bool get(BoolSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? boolSettings[key - BOOL_FIRST] : boolDefaults[key - BOOL_FIRST];
	}
	int64_t get(Int64Setting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? int64Settings[key - INT64_FIRST] : int64Defaults[key - INT64_FIRST];
	}
	float get(FloatSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? floatSettings[key - FLOAT_FIRST] : floatDefaults[key - FLOAT_FIRST];
	}

	void set(StrSetting key, string const& value) {
		strSettings[key - STR_FIRST] = value;
		isSet[key] = !value.empty();
	}

	void set(IntSetting key, int value) {
		if((key == SLOTS) && (value <= 0)) {
			value = 1;
		}
		intSettings[key - INT_FIRST] = value;
		isSet[key] = true;
	}

	void set(BoolSetting key, bool value) {
		boolSettings[key - BOOL_FIRST] = value;
		isSet[key] = true;
	}

	void set(IntSetting key, const string& value) {
		if(value.empty()) {
			intSettings[key - INT_FIRST] = 0;
			isSet[key] = false;
		} else {
			intSettings[key - INT_FIRST] = Util::toInt(value);
			isSet[key] = true;
		}
	}

	void set(Int64Setting key, int64_t value) {
		int64Settings[key - INT64_FIRST] = value;
		isSet[key] = true;
	}

	void set(Int64Setting key, const string& value) {
		if(value.empty()) {
			int64Settings[key - INT64_FIRST] = 0;
			isSet[key] = false;
		} else {
			int64Settings[key - INT64_FIRST] = Util::toInt64(value);
			isSet[key] = true;
		}
	}

	void set(FloatSetting key, float value) {
		floatSettings[key - FLOAT_FIRST] = value;
		isSet[key] = true;
	}
	void set(FloatSetting key, double value) {
		// yes, we loose precision here, but we're gonna loose even more when saving to the XML file...
		floatSettings[key - FLOAT_FIRST] = static_cast<float>(value);
		isSet[key] = true;
	}

	const string& getDefault(StrSetting key) const {
		return strDefaults[key - STR_FIRST];
	}

	int getDefault(IntSetting key) const {
		return intDefaults[key - INT_FIRST];
	}

	bool getDefault(BoolSetting key) const {
		return boolDefaults[key - BOOL_FIRST];
	}

	int64_t getDefault(Int64Setting key) const {
		return int64Defaults[key - INT64_FIRST];
	}

	float getDefault(FloatSetting key) const {
		return floatDefaults[key - FLOAT_FIRST];
	}

	void setDefault(StrSetting key, string const& value) {
		strDefaults[key - STR_FIRST] = value;
	}

	void setDefault(IntSetting key, int value) {
		intDefaults[key - INT_FIRST] = value;
	}

	void setDefault(BoolSetting key, int value) {
		boolDefaults[key - BOOL_FIRST] = value;
	}

	void setDefault(Int64Setting key, int64_t value) {
		int64Defaults[key - INT64_FIRST] = value;
	}

	void setDefault(FloatSetting key, float value) {
		floatDefaults[key - FLOAT_FIRST] = value;
	}

	template<typename KeyT> bool isDefault(KeyT key) {
		return !isSet[key] || get(key, false) == getDefault(key);
	}

	void unset(size_t key) { isSet[key] = false; }

	void load() {
		Util::migrate(getConfigFile());
		load(getConfigFile());
	}
	void save() {
		save(getConfigFile());
	}

	void load(const string& aFileName);
	void save(const string& aFileName);

	bool getType(const char* name, int& n, Types& type) const;
	bool getType(const int& n, Types& type) const;
	const string parseCoreCmd(const string cmd);

	const string (&getSettingTags() const)[SETTINGS_LAST+1] {
		return settingTags;
	}

	// Search types
	void validateSearchTypeName(const string& name) const;
	void setSearchTypeDefaults();
	void addSearchType(const string& name, const StringList& extensions, bool validated = false);
	void delSearchType(const string& name);
	void renameSearchType(const string& oldName, const string& newName);
	void modSearchType(const string& name, const StringList& extensions);

	const SearchTypes& getSearchTypes() const {
		return searchTypes;
	}
	const StringList& getExtensions(const string& name);

private:
	friend struct HubSettings;
	friend class Singleton<SettingsManager>;
	SettingsManager();
	virtual ~SettingsManager() { }

	static const string settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	bool boolSettings[BOOL_LAST - BOOL_FIRST];
	int64_t int64Settings[INT64_LAST - INT64_FIRST];
	float floatSettings[FLOAT_LAST - FLOAT_FIRST];

	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	bool boolDefaults[BOOL_LAST - BOOL_FIRST];
	int64_t int64Defaults[INT64_LAST - INT64_FIRST];
	float floatDefaults[FLOAT_LAST - FLOAT_FIRST];

	bool isSet[SETTINGS_LAST];

	static string getConfigFile() { return Util::getPath(Util::PATH_USER_CONFIG) + "DCPlusPlus.xml"; }

	// Search types
	SearchTypes searchTypes; // name, extlist

	SearchTypes::iterator getSearchType(const string& name);
};

// Shorthand accessor macros
#define SETTING(k) dcpp::SettingsManager::getInstance()->get(dcpp::SettingsManager::k)
#define SGET(k) dcpp::SettingsManager::getInstance()->get(k);
} // namespace dcpp

#endif // !defined(SETTINGS_MANAGER_H)
