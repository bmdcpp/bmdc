﻿/*
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

#include "stdinc.h"
#include "SettingsManager.h"
#include <glib.h>
#include "SimpleXML.h"
#include "Util.h"
#include "File.h"
#include "version.h"
#include "AdcHub.h"
#include "CID.h"
#include "SearchManager.h"
#include "StringTokenizer.h"

namespace dcpp {

StringList SettingsManager::connectionSpeeds;

const string SettingsManager::settingTags[] =
{
	// Strings
	"Nick", "UploadSpeed", "Description", "DownloadDirectory", "EMail", "ExternalIp", "ExternalIp6",
	"ConnectionsOrder", "ConnectionsWidths", "HubFrameOrder", "HubFrameWidths",
	"SearchFrameOrder", "SearchFrameWidths", "FavHubsFrameOrder", "FavHubsFrameWidths",
	"HublistServers", "QueueFrameOrder", "QueueFrameWidths", "PublicHubsFrameOrder", "PublicHubsFrameWidths",
	"FinishedDLFilesOrder", "FinishedDLFilesWidths", "FinishedDLUsersOrder", "FinishedDLUsersWidths",
	"FinishedULFilesOrder", "FinishedULFilesWidths", "FinishedULUsersOrder", "FinishedULUsersWidths",
	"UsersFrameOrder", "UsersFrameWidths", "HttpProxy", "LogDirectory", "LogFormatPostDownload",
	"LogFormatPostFinishedDownload", "LogFormatPostUpload", "LogFormatMainChat", "LogFormatPrivateChat",
	"TempDownloadDirectory", "BindAddress", "BindAddress6", "SocksServer", "SocksUser", "SocksPassword", "ConfigVersion",
	"DefaultAwayMessage", "TimeStampsFormat", "CountryFormat", "ADLSearchFrameOrder", "ADLSearchFrameWidths",
	"CID", "SpyFrameWidths", "SpyFrameOrder", "LogFileMainChat", "LogFilePrivateChat",
	"LogFileStatus", "LogFileUpload", "LogFileDownload", "LogFileFinishedDownload", "LogFileSystem",
	"LogFormatSystem", "LogFormatStatus", "DirectoryListingFrameOrder", "DirectoryListingFrameWidths",
	"TLSPrivateKeyFile", "TLSCertificateFile", "TLSTrustedCertificatesPath",
	"Language", "DownloadsOrder", "DownloadsWidth", "Toolbar", "LastSearchType", "Mapper", "Mapper6",
	"SoundMainChat", "SoundPM", "SoundPMWindow", "SoundFinishedDL", "SoundFinishedFL", "LastSharedFolder",
	"ACFrameOrder", "ACFrameWidths",
	"SharingSkiplistExtensions", "SharingSkiplistRegEx", "SharingSkiplistPaths",
	//[BMDC
	"BackupFilePattern", "LogFileRaw", "LogFormatRaw", "ProtectedUsers", "BackupTimeStamp", "EmotPack", "RipeDB",
	"HubIconStr", "HubTextStr", "HubULOrder", "HubULVisible", "HubULSize", "ChatExtraInfo",
	"BackgroundChatColor", "BackgroundChatImage", "BackgroundPmColor", "BackgroundPmImage",
	"RatioTemplate","LogFileProto", "LogFormatProto", "WildcardForExportSet",
	//]
	"SENTRY",
	// Ints
	"IncomingConnections", "OutgoingConnections", "InPort", "UDPPort", "TLSPort",
	"SocksPort", "SocketInBuffer", "SocketOutBuffer",
	"TextColor", "BackgroundColor", "UploadTextColor", "UploadBgColor", "DownloadTextColor",
	"DownloadBgColor", "LinkColor", "LogColor",
	"BandwidthLimitStart", "BandwidthLimitEnd", "MaxDownloadSpeedRealTime",
	"MaxUploadSpeedTime", "MaxDownloadSpeedPrimary", "MaxUploadSpeedPrimary",
	"SlotsAlternateLimiting", "SlotsPrimaryLimiting",
	"MainWindowState", "MainWindowSizeX", "MainWindowSizeY", "MainWindowPosX",
	"MainWindowPosY", "SettingsWidth", "SettingsHeight", "SettingsPage",
	"HighestPrioSize", "HighPrioSize", "NormalPrioSize", "LowPrioSize", "AutoDropSpeed",
	"AutoDropInterval", "AutoDropElapsed", "AutoDropInactivity", "AutoDropMinSources",
	"AutoDropFilesize",
	"BalloonMainChat", "BalloonPM", "BalloonPMWindow", "BalloonFinishedDL", "BalloonFinishedFL",
	"AwayIdle", "AutoRefreshTime", "AutoSearchLimit", "BufferSize", "DownloadSlots",
	"HubLastLogLines", "MagnetAction", "MaxCommandLength", "MaxCompression", "MaxDownloadSpeed",
	"MaxFilelistSize", "MaxHashSpeed", "MaxMessageLines", "MaxPMWindows", "MinMessageLines",
	"MinUploadSpeed", "PMLastLogLines", "SearchHistory", "SetMinislotSize",
	"SettingsSaveInterval", "Slots", "TabStyle", "TabWidth", "ToolbarSize",
	//BMDC
	"CheckDelay",
	"MaxTestSURs",
	"MaxFileLists",
	"SleepTime",
	"AutoBackupTime",
	"TimeReconnect",
	"PercentToleratedFake",
	"FakeShareRaw",
	"MaxDisconnect",
	"DisconnectRaw",
	"TooBigRaw",
	"MinFileListSize",
	"ListLenMisMatchRaw",
	"FileListNARaw",
	"AdlSearchDefAction",
	"MinPointsToDisplayCheat",
	"DetectionFail",
	"DetectionSuces",
	"DCPPemulationRaw",
	"VersionMisMatchRaw",
	"SlowRaw",
	"SlowSpeed",
	"SlowTime",
	"RmdcRaw",
	"FileListVersionMisMatch",
	"AdlRaw",
	"NickPanePos",
	"SENTRY",
	// Bools
	"AddFinishedInstantly", "AdlsBreakOnFirst", "AltSortOrder",
	"AllowUntrustedClients", "AllowUntrustedHubs", "AlwaysTray", "AutoAway",
	"AutoDetectIncomingConnection", "AutoFollow", "AutoKick", "AutoKickNoFavs", "AutoSearch",
	"AutoSearchAutoMatch", "AutoDropAll", "AutoDropDisconnect", "AutoDropFilelists",
	"AwayCompLock", "BoldFinishedDownloads", "BoldFinishedUploads", "BoldFL", "BoldHub", "BoldPm",
	"BoldQueue", "BoldSearch", "BoldSearchSpy", "BoldSystemLog", "ClearSearch",
	"CompressTransfers", "ConfirmADLSRemoval", "ConfirmExit", "ConfirmHubClosing",
	"ConfirmHubRemoval", "ConfirmItemRemoval", "ConfirmUserRemoval", "Coral",
	"DontDlAlreadyQueued", "DontDLAlreadyShared", "FavShowJoins", "FilterMessages",
	"FinishedDLOnlyFull", "FollowLinks", "GetUserCountry", "GetUserInfo", "HubUserCommands",
	"IgnoreBotPms", "IgnoreHubPms", "OpenNewWindow", "KeepFinishedFiles", "KeepLists",
	"ListDuplicates", "LogDownloads", "LogFilelistTransfers", "LogFinishedDownloads", "LogMainChat",
	"LogPrivateChat", "LogStatusMessages", "LogSystem", "LogUploads", "MagnetAsk",
	"MagnetRegister", "MinimizeToTray", "NoAwayMsgToBots", "NoIpOverride", "OpenUserCmdHelp",
	"OwnerDrawnMenus", "PopupBotPms", "PopupHubPms", "PopupPMs", "PopunderFilelist", "PopunderPm",
	"LowestPrio", "PromptPassword", "QueueFrameShowTree", "RequireTLS", "SearchFilterShared",
	"SearchMerge", "SearchOnlyFreeSlots", "SegmentedDL", "SendBloom", "SendUnknownCommands",
	"SFVCheck", "ShareHidden", "ShowJoins", "ShowMenuBar", "ShowStatusbar", "ShowToolbar",
	"ShowTransferview", "SkipZeroByte", "SocksResolve", "SortFavUsersFirst",
	"SpyFrameIgnoreTthSearches", "StatusInChat", "TimeDependentThrottle", "TimeStamps",
	"ToggleActiveTab", "UrlHandler", "UseCTRLForLineHistory", "UseSystemIcons",
	"UsersFilterFavorite", "UsersFilterOnline", "UsersFilterQueue", "UsersFilterWaiting",
	"ACDisclaimer",
	//BMDC++
	"CheckAllClientsBeforeList",
	"ShowFreeSlotsInDesc",
	"EnableBackup",
	"ShowFakeRaw",
	"LogRawCmd",
	"ShowDisconnect",
	"FileListToBig",
	"ListLenMatchShow",
	"UseSendDelayedRaw",
	"ShowNAFileList",
	"ShowAdlSearchDefAction",
	"ThrotleEnable",
	"UseIp",
	"FavIsProtectedUser",
	"ShowDCPPEmulation",
	"VersionMisMatch",
	"ShowRMDC",
	"UseSlowKick",
	"DontCheckProtectedUsersClient",
	"DontCheckProtectedUsersFileList",
	"ShowFileListVersionMismatch",
	"UseWildCardProtectUsers",
	"DisplayCheatsInMainChat",
	"UseOemFont", "ServerCommands",
	"UseAvFilter", "LogChatB",
	"UseCountryFlag", "UseEmots",
	"UseHighliting",
	"UseSock5",
	"NoIpOveride6",
	"SENTRY",
	// Int64
	"TotalUpload", "TotalDownload","SharingSkiplistMinSize", "SharingSkiplistMaxSize",
	"SENTRY",
	// Floats
	"TransfersPanedPos", "QueuePanedPos", "SearchPanedPos",
	"SENTRY"
};


SettingsManager::SettingsManager()
{

	connectionSpeeds.push_back("0.005");
	connectionSpeeds.push_back("0.01");
	connectionSpeeds.push_back("0.02");
	connectionSpeeds.push_back("0.05");
	connectionSpeeds.push_back("0.1");
	connectionSpeeds.push_back("0.2");
	connectionSpeeds.push_back("0.5");
	connectionSpeeds.push_back("1");
	connectionSpeeds.push_back("2");
	connectionSpeeds.push_back("5");
	connectionSpeeds.push_back("10");
	connectionSpeeds.push_back("20");
	connectionSpeeds.push_back("50");
	connectionSpeeds.push_back("100");
	connectionSpeeds.push_back("1000");

	for(int i=0; i < SETTINGS_LAST; i++)
		isSet[i] = false;

	for(int i=0; i<INT_LAST-INT_FIRST; i++) {
		intDefaults[i] = 0;
		intSettings[i] = 0;
	}
	for(int i=0; i<BOOL_LAST-BOOL_FIRST; i++) {
		boolDefaults[i] = false;
		boolSettings[i] = false;
	}
	for(int i=0; i<INT64_LAST-INT64_FIRST; i++) {
		int64Defaults[i] = 0;
		int64Settings[i] = 0;
	}
	for(int i=0; i<FLOAT_LAST-FLOAT_FIRST; i++) {
		floatDefaults[i] = 0;
		floatSettings[i] = 0;
	}


//should use xdg on *nix
#ifndef _WIN32
	const char* xdgDir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
	const string downloadDir = xdgDir ? string(xdgDir) + PATH_SEPARATOR_STR : Util::getPath(Util::PATH_DOWNLOADS);
	setDefault(DOWNLOAD_DIRECTORY, downloadDir);
#else
	setDefault(DOWNLOAD_DIRECTORY,Util::getPath(Util::PATH_DOWNLOADS));
#endif
	setDefault(TEMP_DOWNLOAD_DIRECTORY, Util::getPath(Util::PATH_USER_LOCAL) + "Incomplete" PATH_SEPARATOR_STR);

	setDefault(EXTERNAL_IP, "0.0.0.0");
	//defaultString.insert(StringMap::value_type("ExternalIP","0.0.0.0"));
	setDefault(BIND_ADDRESS, "0.0.0.0");
	setDefault(BIND_ADDRESS6, "::");
	setDefault(SLOTS, 3);
	setDefault(TCP_PORT, 0);
	setDefault(UDP_PORT, 0);
	setDefault(TLS_PORT, 0);
	setDefault(INCOMING_CONNECTIONS, INCOMING_DIRECT);
	setDefault(OUTGOING_CONNECTIONS, OUTGOING_DIRECT);
	setDefault(AUTO_DETECT_CONNECTION, true);
	setDefault(AUTO_FOLLOW, true);
	setDefault(CLEAR_SEARCH, true);
	setDefault(SHARE_HIDDEN, false);
	setDefault(FILTER_MESSAGES, true);
	setDefault(MINIMIZE_TRAY, true);
	setDefault(ALWAYS_TRAY, true);
	setDefault(AUTO_SEARCH, false);
	setDefault(TIME_STAMPS, true);
	setDefault(POPUP_HUB_PMS, true);
	setDefault(POPUP_BOT_PMS, true);
	setDefault(IGNORE_HUB_PMS, false);
	setDefault(IGNORE_BOT_PMS, false);
	setDefault(LIST_DUPES, true);
	setDefault(BUFFER_SIZE, 64);
	setDefault(HUBLIST_SERVERS, "https://www.te-home.net/?do=hublist&get=hublist.xml.bz2;https://dchublist.org/hublist.xml.bz2;https://dchublist.ru/hublist.xml.bz2;https://hublist.pwiam.com/hublist.xml.bz2;https://dcnf.github.io/Hublist/hublist.xml.bz2;");
	setDefault(DOWNLOAD_SLOTS, 6);
	setDefault(MAX_DOWNLOAD_SPEED, 0);
	setDefault(LOG_DIRECTORY, Util::getPath(Util::PATH_USER_LOCAL) + "Logs" PATH_SEPARATOR_STR);
	setDefault(LOG_UPLOADS, false);
	setDefault(LOG_DOWNLOADS, false);
	setDefault(LOG_FINISHED_DOWNLOADS, false);
	setDefault(LOG_PRIVATE_CHAT, false);
	setDefault(LOG_MAIN_CHAT, false);
	setDefault(STATUS_IN_CHAT, false);
	setDefault(SHOW_JOINS, false);
	setDefault(UPLOAD_SPEED, connectionSpeeds[0]);
	setDefault(USE_SYSTEM_ICONS, true);//TODO: yes?
	setDefault(POPUP_PMS, true);
	setDefault(MIN_UPLOAD_SPEED, 0);
	setDefault(LOG_FORMAT_POST_DOWNLOAD, "%Y-%m-%d %H:%M: %[target] " + string(_("downloaded from")) + " %[userNI] (%[userCID]), %[fileSI] (%[fileSIactual]), %[speed], %[time], %[fileTR]");
	setDefault(LOG_FORMAT_POST_FINISHED_DOWNLOAD, "%Y-%m-%d %H:%M: %[target] " + string(_("downloaded from")) + " %[userNI] (%[userCID]), %[fileSI] (%[fileSIsession]), %[speed], %[time], %[fileTR]");
	setDefault(LOG_FORMAT_POST_UPLOAD, "%Y-%m-%d %H:%M: %[source] " + string(_("uploaded to")) + " %[userNI] (%[userCID]), %[fileSI] (%[fileSIactual]), %[speed], %[time], %[fileTR]");
	setDefault(LOG_FORMAT_MAIN_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_PRIVATE_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_STATUS, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_SYSTEM, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_RAW, "%[message]");//BMDC
	setDefault(LOG_FILE_MAIN_CHAT, "%[hubURL].log");
	setDefault(LOG_FILE_STATUS, "%[hubURL]_status.log");
	setDefault(LOG_FILE_PRIVATE_CHAT, "%[userNI].log");
	setDefault(LOG_FILE_UPLOAD, "Uploads.log");
	setDefault(LOG_FILE_DOWNLOAD, "Downloads.log");
	setDefault(LOG_FILE_FINISHED_DOWNLOAD, "Finished_downloads.log");
	setDefault(LOG_FILE_SYSTEM, "System.log");
	setDefault(LOG_FILE_RAW, "Raws.log");//@TODO: BMDC : UI?
	setDefault(GET_USER_INFO, true);
	setDefault(URL_HANDLER, false);
	//Possible unused
	setDefault(SETTINGS_WIDTH, 700); //not used?
	setDefault(SETTINGS_HEIGHT, 600); //not used?
	//--//
	setDefault(SOCKS_PORT, 1080);
	setDefault(SOCKS_RESOLVE, true);
	setDefault(CONFIG_VERSION, "0.181");		// 0.181 is the last version missing configversion
	setDefault(KEEP_LISTS, false);
	setDefault(AUTO_KICK, false);
	setDefault(QUEUEFRAME_SHOW_TREE, true);//not used?
	setDefault(COMPRESS_TRANSFERS, true);
	setDefault(SFV_CHECK, true);
	setDefault(AUTO_AWAY, false);
	setDefault(AWAY_COMP_LOCK, true);
	setDefault(AWAY_IDLE, 10);//not used?
	setDefault(TIME_STAMPS_FORMAT, "%H:%M:%S");
	setDefault(COUNTRY_FORMAT, "%[2code] - %[name]");
	setDefault(MAX_COMPRESSION, 6);
	setDefault(NO_AWAYMSG_TO_BOTS, false);
	setDefault(SKIP_ZERO_BYTE, true);
	setDefault(ADLS_BREAK_ON_FIRST, false);
	setDefault(HUB_USER_COMMANDS, true);
	setDefault(AUTO_SEARCH_AUTO_MATCH, true);
	setDefault(LOG_FILELIST_TRANSFERS, false);
	setDefault(LOG_SYSTEM, false);
	setDefault(SEND_UNKNOWN_COMMANDS, true);
	setDefault(MAX_HASH_SPEED, 0);
	setDefault(OPEN_USER_CMD_HELP, true);
	setDefault(GET_USER_COUNTRY, true);
	setDefault(FAV_SHOW_JOINS, false);
	setDefault(LOG_STATUS_MESSAGES, false);
	setDefault(SHOW_MENU_BAR, true);//TODO?
	setDefault(SHOW_TRANSFERVIEW, true); //hide-transfer in WUl?
	setDefault(SHOW_STATUSBAR, true);
	setDefault(SHOW_TOOLBAR, true);
	setDefault(POPUNDER_PM, false);
	setDefault(POPUNDER_FILELIST, false);
	setDefault(MAGNET_REGISTER, true);
	setDefault(MAGNET_ASK, true);
	setDefault(MAGNET_ACTION, MAGNET_AUTO_SEARCH);
	setDefault(ADD_FINISHED_INSTANTLY, false);
	setDefault(DONT_DL_ALREADY_SHARED, false);
	setDefault(USE_CTRL_FOR_LINE_HISTORY, true);
	setDefault(JOIN_OPEN_NEW_WINDOW, false);
	setDefault(HUB_LAST_LOG_LINES, 10);//possible todo
	setDefault(PM_LAST_LOG_LINES, 10);
	setDefault(TOGGLE_ACTIVE_WINDOW, false);
	setDefault(SEARCH_HISTORY, 10);
	setDefault(SET_MINISLOT_SIZE, 512);
	setDefault(MAX_FILELIST_SIZE, 512);
	setDefault(PRIO_HIGHEST_SIZE, 64);
	setDefault(PRIO_HIGH_SIZE, 0);
	setDefault(PRIO_NORMAL_SIZE, 0);
	setDefault(PRIO_LOW_SIZE, 0);
	setDefault(PRIO_LOWEST, false);
	setDefault(AUTODROP_SPEED, 1024);
	setDefault(AUTODROP_INTERVAL, 10);
	setDefault(AUTODROP_ELAPSED, 15);
	setDefault(AUTODROP_INACTIVITY, 10);
	setDefault(AUTODROP_MINSOURCES, 2);
	setDefault(AUTODROP_FILESIZE, 0);
	setDefault(AUTODROP_ALL, false);
	setDefault(AUTODROP_FILELISTS, false);
	setDefault(AUTODROP_DISCONNECT, false);
	setDefault(NO_IP_OVERRIDE, false);
	setDefault(SEARCH_ONLY_FREE_SLOTS, false);
	setDefault(SEARCH_FILTER_SHARED, true);
	setDefault(SOCKET_IN_BUFFER, 64*1024);
	setDefault(SOCKET_OUT_BUFFER, 64*1024);
	setDefault(TLS_TRUSTED_CERTIFICATES_PATH, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR);
	setDefault(TLS_PRIVATE_KEY_FILE, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR "client.key");
	setDefault(TLS_CERTIFICATE_FILE, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR "client.crt");
	setDefault(BOLD_FINISHED_DOWNLOADS, true);
	setDefault(BOLD_FINISHED_UPLOADS, true);
	setDefault(BOLD_QUEUE, true);
	setDefault(BOLD_HUB, true);
	setDefault(BOLD_PM, true);
	setDefault(BOLD_FL, true);
	setDefault(BOLD_SEARCH, true);
	setDefault(BOLD_SEARCH_SPY, true);
	setDefault(BOLD_SYSTEM_LOG, true);
	setDefault(AUTO_REFRESH_TIME, 60);
	setDefault(AUTO_SEARCH_LIMIT, 5);
	setDefault(ALT_SORT_ORDER, false);
	setDefault(AUTO_KICK_NO_FAVS, false);
	setDefault(PROMPT_PASSWORD, false);
	setDefault(SPY_FRAME_IGNORE_TTH_SEARCHES, false);
	setDefault(DONT_DL_ALREADY_QUEUED, false);
	setDefault(MAX_COMMAND_LENGTH, 16*1024*1024);
	setDefault(ALLOW_UNTRUSTED_HUBS, true);
	setDefault(ALLOW_UNTRUSTED_CLIENTS, true);
	setDefault(SORT_FAVUSERS_FIRST, false);
	setDefault(SEGMENTED_DL, true);
	setDefault(FOLLOW_LINKS, false);
	setDefault(SEND_BLOOM, true);
	setDefault(OWNER_DRAWN_MENUS, true);//what?
	setDefault(CORAL, true);
	setDefault(FINISHED_DL_ONLY_FULL, true);
	setDefault(CONFIRM_EXIT, true);
	setDefault(CONFIRM_HUB_CLOSING, true);
	setDefault(CONFIRM_HUB_REMOVAL, true);
	setDefault(CONFIRM_USER_REMOVAL, true);
	setDefault(CONFIRM_ITEM_REMOVAL, true);
	setDefault(CONFIRM_ADLS_REMOVAL, true);
	setDefault(SEARCH_MERGE, true);//what?
	setDefault(TOOLBAR_SIZE, 20);
	setDefault(TAB_WIDTH, 150);
	setDefault(TRANSFERS_PANED_POS, .7);
	setDefault(QUEUE_PANED_POS, .3);
	setDefault(SEARCH_PANED_POS, .2);
	setDefault(KEEP_FINISHED_FILES, false);
	setDefault(MIN_MESSAGE_LINES, 1);
	setDefault(MAX_MESSAGE_LINES, 10);
	setDefault(MAX_UPLOAD_SPEED_MAIN, 0);
	setDefault(MAX_DOWNLOAD_SPEED_MAIN, 0);
	setDefault(TIME_DEPENDENT_THROTTLE, false);
	setDefault(MAX_DOWNLOAD_SPEED_ALTERNATE, 0);
	setDefault(MAX_UPLOAD_SPEED_ALTERNATE, 0);
	setDefault(BANDWIDTH_LIMIT_START, 1);
	setDefault(BANDWIDTH_LIMIT_END, 1);
	setDefault(SLOTS_ALTERNATE_LIMITING, 1);
	setDefault(SLOTS_PRIMARY, 3);
	setDefault(SETTINGS_SAVE_INTERVAL, 10);
	setDefault(USERS_FILTER_ONLINE, false);
	setDefault(USERS_FILTER_FAVORITE, true);
	setDefault(USERS_FILTER_QUEUE, false);
	setDefault(USERS_FILTER_WAITING, false);
	setDefault(MAX_PM_WINDOWS, 50);
	setDefault(REQUIRE_TLS, false);
	setDefault(LAST_SHARED_FOLDER, Util::emptyString);
	setDefault(AC_DISCLAIM, false);

	setDefault(SHARING_SKIPLIST_EXTENSIONS, Util::emptyString);
	setDefault(SHARING_SKIPLIST_REGEX, Util::emptyString);
	setDefault(SHARING_SKIPLIST_PATHS, Util::emptyString);
	setDefault(SHARING_SKIPLIST_MINSIZE, 0);
	setDefault(SHARING_SKIPLIST_MAXSIZE, 0);
//[BMDC++
	setDefault(EMOT_PACK, "bmicon");
	setDefault(TIME_RECCON, 10);
	setDefault(AUTOBACKUP_TIME, 60*60*60);
	setDefault(CHECK_DELAY, 1);
	setDefault(MAX_TESTSURS, 50);
	setDefault(MAX_FILELISTS, 10);
	setDefault(SLEEP_TIME, 0);
	setDefault(MIN_POINTS_TO_DISPLAY_CHEAT, 0);
	setDefault(DETECTIONF, 0);
	setDefault(DETECTIONS, 0);
	setDefault(DCPP_EMULATION_RAW, 0);
	setDefault(VERSION_MISMATCH_RAW, 0);
	setDefault(SDL_RAW, 0);
	setDefault(RIPE_DB, "https://apps.db.ripe.net/search/query.html?searchtext=%[IP]&search%3AdoSearch=Search#resultsAnchor");
	setDefault(SERVER_COMMANDS, true);
	setDefault(USE_AV_FILTER, true);
	//not UI for it
	setDefault(HUB_ICON_STR, Util::emptyString);
	setDefault(HUB_TEXT_STR, Util::emptyString);
	setDefault(HUB_UL_ORDER, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14");
	setDefault(HUB_UL_VISIBLE, "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1");
	setDefault(HUB_UL_SIZE, "157,75,85,100,85,85,90,70,100,50,50,80,80,80,80");
	setDefault(CHAT_EXTRA_INFO, Util::emptyString);
	setDefault(LOG_CHAT_B, true);
	setDefault(USE_COUNTRY_FLAG, true);
	setDefault(USE_EMOTS, true);
	setDefault(BACKGROUND_CHAT_COLOR, "white");
	setDefault(BACKGROUND_CHAT_IMAGE, Util::emptyString);
	setDefault(BACKGROUND_PM_COLOR, "white");
	setDefault(BACKGROUND_PM_IMAGE, Util::emptyString);
	setDefault(RATIO_TEMPLATE, "%[client] Ratio: %[ratio]  Uploads: %[up] / Downloads %[down] ");
	setDefault(USE_HIGHLITING, true);
	
	setDefault(NICK_PANE_POS, 400);//hub tab
	setDefault(WILDCARD_FOR_EXPORT_SET, "profile.lck;Emptyfiles.xml.bz2;..;.;GeoIP*.dat;GeoIP*.gz;TestSUR*;");
	setDefault(USE_SOCK5, false);

	setSearchTypeDefaults();
}

void SettingsManager::load(string const& aFileName)
{
	try {
		SimpleXML xml;

		xml.fromXML(File(aFileName, File::READ, File::OPEN).read());

		xml.resetCurrentChild();

		xml.stepIn();

		if(xml.findChild("Settings"))
		{
			xml.stepIn();

			int i;

			for(i=STR_FIRST; i<STR_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);

				if(xml.findChild(attr))
					set(StrSetting(i), xml.getChildData());
				xml.resetCurrentChild();
			}
			for(i=INT_FIRST; i<INT_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);

				if(xml.findChild(attr))
					set(IntSetting(i), Util::toInt(xml.getChildData()));
				xml.resetCurrentChild();
			}
			for(i=BOOL_FIRST; i<BOOL_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);

				if(xml.findChild(attr))
					set(BoolSetting(i), Util::toInt(xml.getChildData()));
				xml.resetCurrentChild();
			}
			for(i=FLOAT_FIRST; i<FLOAT_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);

				if(xml.findChild(attr))
					set(FloatSetting(i), Util::toInt(xml.getChildData()) / 1000.);
				xml.resetCurrentChild();
			}
			for(i=INT64_FIRST; i<INT64_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);

				if(xml.findChild(attr))
					set(Int64Setting(i), Util::toInt64(xml.getChildData()));
				xml.resetCurrentChild();
			}

			xml.stepOut();
		}

		xml.resetCurrentChild();
		if(xml.findChild("SearchTypes")) {
			searchTypes.clear();
			xml.stepIn();
			while(xml.findChild("SearchType")) {
				const string& extensions = xml.getChildData();
				if(extensions.empty()) {
					continue;
				}
				const string& name = xml.getChildAttrib("Id");
				if(name.empty()) {
					continue;
				}
				searchTypes[name] = StringTokenizer<string>(extensions, ';').getTokens();
			}
			xml.stepOut();
		}

		if(SETTING(PRIVATE_ID).length() != 39 || !CID(SETTING(PRIVATE_ID))) {
			set(PRIVATE_ID, CID::generate().toBase32());
		}

		double v = Util::toDouble(SETTING(CONFIG_VERSION));
		// if(v < 0.x) { // Fix old settings here }

		if(v <= 0.674) {
			// Formats changed, might as well remove these...
			unset(LOG_FORMAT_POST_DOWNLOAD);
			unset(LOG_FORMAT_POST_UPLOAD);
			unset(LOG_FORMAT_MAIN_CHAT);
			unset(LOG_FORMAT_PRIVATE_CHAT);
			unset(LOG_FORMAT_STATUS);
			unset(LOG_FORMAT_SYSTEM);
			unset(LOG_FILE_MAIN_CHAT);
			unset(LOG_FILE_STATUS);
			unset(LOG_FILE_PRIVATE_CHAT);
			unset(LOG_FILE_UPLOAD);
			unset(LOG_FILE_DOWNLOAD);
			unset(LOG_FILE_SYSTEM);
		}

		if(v <= 0.770 && SETTING(INCOMING_CONNECTIONS) != INCOMING_FIREWALL_PASSIVE) {
			set(AUTO_DETECT_CONNECTION, false); //Don't touch if it works
		}

		if(v <= 0.782) {
			// These were remade completely...
			unset(USERSFRAME_ORDER);
			unset(USERSFRAME_WIDTHS);

			// the id has changed
			if(isSet[TOOLBAR])
				Util::replace("FavUsers", "Users", strSettings[TOOLBAR - STR_FIRST]);
		}

		if(v <= 0.791) {
			// the meaning of a default away message has changed: it now means "no away message".
			if(SETTING(DEFAULT_AWAY_MESSAGE).empty()) {
				set(DEFAULT_AWAY_MESSAGE, "I'm away. State your business and I might answer later if you're lucky.");
			}
		}

		if(SETTING(SET_MINISLOT_SIZE) < 512)
			set(SET_MINISLOT_SIZE, 512);
		if(SETTING(AUTODROP_INTERVAL) < 1)
			set(AUTODROP_INTERVAL, 1);
		if(SETTING(AUTODROP_ELAPSED) < 1)
			set(AUTODROP_ELAPSED, 1);
		if(SETTING(AUTO_SEARCH_LIMIT) > 5)
			set(AUTO_SEARCH_LIMIT, 5);
		else if(SETTING(AUTO_SEARCH_LIMIT) < 1)
			set(AUTO_SEARCH_LIMIT, 1);

		File::ensureDirectory(SETTING(TLS_TRUSTED_CERTIFICATES_PATH));

		fire(SettingsManagerListener::Load(), xml);

		xml.stepOut();

	} catch(const Exception&) {
		if(!CID(SETTING(PRIVATE_ID)))
			set(PRIVATE_ID, CID::generate().toBase32());
	}
}

void SettingsManager::save(string const& aFileName) {

	SimpleXML xml;
	xml.addTag("DCPlusPlus");
	xml.stepIn();
	xml.addTag("Settings");
	xml.stepIn();

	int i;
	string type("type"), curType("string");

	for(i=STR_FIRST; i<STR_LAST; i++)
	{
		if(i == CONFIG_VERSION) {
			xml.addTag(settingTags[i], VERSIONSTRING); // need original DC++ version
			xml.addChildAttrib(type, curType);
		} else if(isSet[i]) {
			xml.addTag(settingTags[i], get(StrSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}

	curType = "int";
	for(i=INT_FIRST; i<INT_LAST; i++)
	{
		if(isSet[i]) {
			xml.addTag(settingTags[i], get(IntSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	for(i=BOOL_FIRST; i<BOOL_LAST; i++)
	{
		if(isSet[i]) {
			xml.addTag(settingTags[i], get(BoolSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	for(i=FLOAT_FIRST; i<FLOAT_LAST; i++)
	{
		if(isSet[i]) {
			xml.addTag(settingTags[i], static_cast<int>(get(FloatSetting(i), false) * 1000.));
			xml.addChildAttrib(type, curType);
		}
	}
	curType = "int64";
	for(i=INT64_FIRST; i<INT64_LAST; i++)
	{
		if(isSet[i])
		{
			xml.addTag(settingTags[i], get(Int64Setting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	xml.stepOut();

	xml.addTag("SearchTypes");
	xml.stepIn();
	for(auto i = searchTypes.begin(); i!= searchTypes.end();++i) {
		xml.addTag("SearchType", Util::toString(";", i->second));
		xml.addChildAttrib("Id", i->first);
	}
	xml.stepOut();

	fire(SettingsManagerListener::Save(), xml);

	try {
		File out(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(aFileName);
		File::renameFile(aFileName + ".tmp", aFileName);
	} catch(const FileException&) {
		// ...
	}
}

void SettingsManager::validateSearchTypeName(const string& name) const {
	if(name.empty() || (name.size() == 1 && name[0] >= '1' && name[0] <= '6')) {
		throw SearchTypeException(_("Invalid search type name"));
	}
	for(int type = SearchManager::TYPE_ANY; type != SearchManager::TYPE_LAST; ++type) {
		if(SearchManager::getTypeStr(type) == name) {
			throw SearchTypeException(_("This search type already exists"));
		}
	}
}

void SettingsManager::setSearchTypeDefaults() {
	searchTypes.clear();

	// for conveniency, the default search exts will be the same as the ones defined by SEGA.
	const auto& searchExts = AdcHub::getSearchExts();
	for(size_t i = 0, n = searchExts.size(); i < n; ++i)
		searchTypes[string(1, '1' + i)] = searchExts[i];

	fire(SettingsManagerListener::SearchTypesChanged());
}

void SettingsManager::addSearchType(const string& name, const StringList& extensions, bool validated) {
	if(!validated) {
		validateSearchTypeName(name);
	}

	if(searchTypes.find(name) != searchTypes.end()) {
		throw SearchTypeException(_("This search type already exists"));
	}

	searchTypes[name] = extensions;
	fire(SettingsManagerListener::SearchTypesChanged());
}

void SettingsManager::delSearchType(const string& name) {
	validateSearchTypeName(name);
	searchTypes.erase(name);
	fire(SettingsManagerListener::SearchTypesChanged());
}

void SettingsManager::renameSearchType(const string& oldName, const string& newName) {
	validateSearchTypeName(newName);
	StringList exts = getSearchType(oldName)->second;
	addSearchType(newName, exts, true);
	searchTypes.erase(oldName);
}

void SettingsManager::modSearchType(const string& name, const StringList& extensions) {
	getSearchType(name)->second = extensions;
	fire(SettingsManagerListener::SearchTypesChanged());
}

const StringList& SettingsManager::getExtensions(const string& name) {
	return getSearchType(name)->second;
}

SettingsManager::SearchTypes::iterator SettingsManager::getSearchType(const string& name) {
	auto ret = searchTypes.find(name);
	if(ret == searchTypes.end()) {
		throw SearchTypeException(_("No such search type"));
	}
	return ret;
}

bool SettingsManager::getType(const char* name, int& n, Types& type) const {
	for(n = 0; n < FLOAT_LAST; ++n) {
		if(strcmp(settingTags[n].c_str(), name) == 0) {
			return getType(n, type);
		}
	}
	return false;
}

bool SettingsManager::getType(const int& n, Types& type) const {
	if(n < STR_FIRST || n >= SETTINGS_LAST)
		return false;

	if(n < STR_LAST) {
		type = TYPE_STRING;
	} else if(n < INT_LAST) {
		type = TYPE_INT;
	} else if(n < BOOL_LAST) {
		type = TYPE_BOOL;
	} else if(n < INT64_LAST) {
		type = TYPE_INT64;
	} else {
		type = TYPE_FLOAT;
	}
	return true;
}

const string SettingsManager::parseCoreCmd(const string cmd) {
	StringTokenizer<string> sl(cmd, ' ');
	if (sl.getTokens().size() == 2) {
			int n = -1;
			SettingsManager::Types type = SettingsManager::TYPE_NONE;
			getType(sl.getTokens().at(0).c_str(),n,type);
           if (type == SettingsManager::TYPE_INT) {
                int i = atoi(sl.getTokens().at(1).c_str());
                set((SettingsManager::IntSetting)n,i);
                save();
            }
            else if( type == SettingsManager::TYPE_BOOL) {
				int i = atoi(sl.getTokens().at(1).c_str());
                set((SettingsManager::BoolSetting)n,i);
                save();
            }else if (type == SettingsManager::TYPE_STRING) {
                set((SettingsManager::StrSetting)n, sl.getTokens().at(1));
                save();
            } else
                return _("Error: setting not found!");
            return _("Change core setting ") + string(sl.getTokens().at(0)) + _(" to ") + string(sl.getTokens().at(1));
        }
    return _("Error: params have been not 2!");
}

} // namespace dcpp
