/*
 * Copyright © 2004-2011 Jens Oknelid, paskharen@gmail.com
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

#include "settingsmanager.hh"

#include <dcpp/File.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/Util.h>
#include <dcpp/StringTokenizer.h>
#include "WulforUtil.hh"

#include <glib/gi18n.h>

using namespace std;
using namespace dcpp;

WulforSettingsManager::WulforSettingsManager():
	configFile(Util::getPath(Util::PATH_USER_CONFIG) + "BMDC++.xml")
{
	defaultInt.insert(IntMap::value_type("main-window-maximized", 0));
	defaultInt.insert(IntMap::value_type("main-window-size-x", 875));
	defaultInt.insert(IntMap::value_type("main-window-size-y", 685));
	defaultInt.insert(IntMap::value_type("main-window-pos-x", 100));
	defaultInt.insert(IntMap::value_type("main-window-pos-y", 100));
	defaultInt.insert(IntMap::value_type("main-window-no-close", 0));
	defaultInt.insert(IntMap::value_type("transfer-pane-position", 482));
	defaultInt.insert(IntMap::value_type("nick-pane-position", 500));
	defaultInt.insert(IntMap::value_type("downloadqueue-pane-position", 200));
	defaultInt.insert(IntMap::value_type("sharebrowser-pane-position", 200));
	defaultInt.insert(IntMap::value_type("tab-position", 0));
	defaultInt.insert(IntMap::value_type("toolbar-style", 5));
	defaultInt.insert(IntMap::value_type("sound-pm-open", 0));
	defaultInt.insert(IntMap::value_type("sound-pm", 1));
//	TODO: download begins, uncomment when implemented
//	defaultInt.insert(IntMap::value_type("sound-download-begins-use", 0));
	defaultInt.insert(IntMap::value_type("sound-download-finished-use", 0));
	defaultInt.insert(IntMap::value_type("sound-download-finished-ul-use", 0));
	defaultInt.insert(IntMap::value_type("sound-upload-finished-use", 0));
	defaultInt.insert(IntMap::value_type("sound-private-message-use", 0));
	defaultInt.insert(IntMap::value_type("sound-hub-connect-use", 0));
	defaultInt.insert(IntMap::value_type("sound-hub-disconnect-use", 0));
	defaultInt.insert(IntMap::value_type("sound-fuser-join-use", 0));
	defaultInt.insert(IntMap::value_type("sound-fuser-quit-use", 0));
	defaultInt.insert(IntMap::value_type("use-magnet-split", 1));
	defaultInt.insert(IntMap::value_type("text-bold-autors", 1));
	defaultInt.insert(IntMap::value_type("text-general-bold", 0));
	defaultInt.insert(IntMap::value_type("text-general-italic", 0));
	defaultInt.insert(IntMap::value_type("text-myown-bold", 1));
	defaultInt.insert(IntMap::value_type("text-myown-italic", 0));
	defaultInt.insert(IntMap::value_type("text-private-bold", 0));
	defaultInt.insert(IntMap::value_type("text-private-italic", 0));
	defaultInt.insert(IntMap::value_type("text-system-bold", 1));
	defaultInt.insert(IntMap::value_type("text-system-italic", 0));
	defaultInt.insert(IntMap::value_type("text-status-bold", 1));
	defaultInt.insert(IntMap::value_type("text-status-italic", 0));
	defaultInt.insert(IntMap::value_type("text-timestamp-bold", 1));
	defaultInt.insert(IntMap::value_type("text-timestamp-italic", 0));
	defaultInt.insert(IntMap::value_type("text-mynick-bold", 1));
	defaultInt.insert(IntMap::value_type("text-mynick-italic", 0));
	defaultInt.insert(IntMap::value_type("text-fav-bold", 1));
	defaultInt.insert(IntMap::value_type("text-fav-italic", 0));
	defaultInt.insert(IntMap::value_type("text-op-bold", 1));
	defaultInt.insert(IntMap::value_type("text-op-italic", 0));
	defaultInt.insert(IntMap::value_type("text-url-bold", 0));
	defaultInt.insert(IntMap::value_type("text-url-italic", 0));
	
	defaultInt.insert(IntMap::value_type("text-ip-bold", 0));
	defaultInt.insert(IntMap::value_type("text-ip-italic", 0));
	defaultInt.insert(IntMap::value_type("text-cheat-bold", 1));
	defaultInt.insert(IntMap::value_type("text-cheat-italic", 0));
	
	defaultInt.insert(IntMap::value_type("toolbar-button-connect", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-fav-hubs", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-fav-users", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-public-hubs", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-settings", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-hash", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-search", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-search-spy", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-queue", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-quit", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-finished-downloads", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-finished-uploads", 1));
    /*Me Toolbars*/
	defaultInt.insert(IntMap::value_type("toolbar-button-notepad", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-adlsearch", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-system", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-ignore", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-away", 1));
	defaultInt.insert(IntMap::value_type("toolbar-limit-bandwith", 0));

	defaultInt.insert(IntMap::value_type("notify-download-finished-use", 0));
	defaultInt.insert(IntMap::value_type("notify-download-finished-ul-use", 0));
	defaultInt.insert(IntMap::value_type("notify-private-message-use", 0));
	defaultInt.insert(IntMap::value_type("notify-hub-disconnect-use", 0));
	defaultInt.insert(IntMap::value_type("notify-hub-connect-use", 0));
	defaultInt.insert(IntMap::value_type("notify-fuser-join", 0));
	defaultInt.insert(IntMap::value_type("notify-fuser-quit", 0));
	/**/
	defaultInt.insert(IntMap::value_type("notify-higl-use", 1));
	/**/
	defaultInt.insert(IntMap::value_type("notify-pm-length", 50));
	defaultInt.insert(IntMap::value_type("notify-icon-size", 3));
	defaultInt.insert(IntMap::value_type("notify-only-not-active", 0));
	defaultInt.insert(IntMap::value_type("status-icon-blink-use", 1));
	defaultInt.insert(IntMap::value_type("emoticons-use", 1));
	defaultInt.insert(IntMap::value_type("pm", 0));//Send private message when double clicked in the user list.
	defaultInt.insert(IntMap::value_type("search-spy-frame", 50));
	defaultInt.insert(IntMap::value_type("search-spy-waiting", 40));
	defaultInt.insert(IntMap::value_type("search-spy-top", 4));
	defaultInt.insert(IntMap::value_type("magnet-action", -1));//default show magnet dialog
	///[core 0.762
	defaultInt.insert(IntMap::value_type("open-public", 0));
	defaultInt.insert(IntMap::value_type("open-favorite-hubs", 0));
	defaultInt.insert(IntMap::value_type("open-queue", 0));
	defaultInt.insert(IntMap::value_type("open-finished-downloads", 0));
	defaultInt.insert(IntMap::value_type("open-finished-uploads", 0));
	defaultInt.insert(IntMap::value_type("open-favorite-users", 0));
	defaultInt.insert(IntMap::value_type("open-search-spy", 0));
	defaultInt.insert(IntMap::value_type("open-notepad", 0));
	defaultInt.insert(IntMap::value_type("open-system", 0));
	defaultInt.insert(IntMap::value_type("open-ignore", 0));
	/**/
	defaultInt.insert(IntMap::value_type("use-highliting", 0));
	defaultInt.insert(IntMap::value_type("show-close-butt", 1));
	defaultInt.insert(IntMap::value_type("show-preferences-on-startup", 1));
	defaultInt.insert(IntMap::value_type("show-commnads", 1));
	defaultInt.insert(IntMap::value_type("use-flag", 1));
	defaultInt.insert(IntMap::value_type("use-dns", 0));
	defaultInt.insert(IntMap::value_type("only-fav", 0));
	defaultInt.insert(IntMap::value_type("log-messages", 0));
///core 0.762]
	/**/
	defaultString.insert(StringMap::value_type("magnet-choose-dir", SETTING(DOWNLOAD_DIRECTORY)));
	defaultString.insert(StringMap::value_type("downloadqueue-order", ""));
	defaultString.insert(StringMap::value_type("downloadqueue-width", ""));
	defaultString.insert(StringMap::value_type("downloadqueue-visibility", ""));
	defaultString.insert(StringMap::value_type("favoritehubs-order", ""));
	defaultString.insert(StringMap::value_type("favoritehubs-width", ""));
	defaultString.insert(StringMap::value_type("favoritehubs-visibility", ""));
	defaultString.insert(StringMap::value_type("favoriteusers-order", ""));
	defaultString.insert(StringMap::value_type("favoriteusers-width", ""));
	defaultString.insert(StringMap::value_type("favoriteusers-visibility", ""));
	defaultString.insert(StringMap::value_type("finished-order", ""));
	defaultString.insert(StringMap::value_type("finished-width", ""));
	defaultString.insert(StringMap::value_type("finished-visibility", ""));
	defaultString.insert(StringMap::value_type("hub-order", ""));
	defaultString.insert(StringMap::value_type("hub-width", ""));
	defaultString.insert(StringMap::value_type("hub-visibility", ""));
	defaultString.insert(StringMap::value_type("transfers-order", ""));
	defaultString.insert(StringMap::value_type("transfers-width", ""));
	defaultString.insert(StringMap::value_type("transfers-visibility", ""));
	defaultString.insert(StringMap::value_type("publichubs-order", ""));
	defaultString.insert(StringMap::value_type("publichubs-width", ""));
	defaultString.insert(StringMap::value_type("publichubs-visibility", ""));
	defaultString.insert(StringMap::value_type("search-order", ""));
	defaultString.insert(StringMap::value_type("search-width", ""));
	defaultString.insert(StringMap::value_type("search-visibility", ""));
	defaultString.insert(StringMap::value_type("searchspy-order", ""));
	defaultString.insert(StringMap::value_type("searchspy-width", ""));
	defaultString.insert(StringMap::value_type("searchspy-visibility", ""));
	defaultString.insert(StringMap::value_type("sharebrowser-order", ""));
	defaultString.insert(StringMap::value_type("sharebrowser-width", ""));
	defaultString.insert(StringMap::value_type("sharebrowser-visibility", ""));
	defaultString.insert(StringMap::value_type("default-charset", WulforUtil::ENCODING_LOCALE));
//	TODO: download begins, uncomment when implemented
// 	defaultString.insert(StringMap::value_type("sound-download-begins", ""));
	defaultString.insert(StringMap::value_type("sound-download-finished", ""));
	defaultString.insert(StringMap::value_type("sound-download-finished-ul", ""));
	defaultString.insert(StringMap::value_type("sound-upload-finished", ""));
	defaultString.insert(StringMap::value_type("sound-private-message", ""));
	defaultString.insert(StringMap::value_type("sound-hub-connect", ""));
	defaultString.insert(StringMap::value_type("sound-hub-disconnect", ""));
	defaultString.insert(StringMap::value_type("sound-fuser-join", ""));
	defaultString.insert(StringMap::value_type("sound-fuser-quit", ""));
	defaultString.insert(StringMap::value_type("text-general-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-general-fore-color", "#4D4D4D"));
	defaultString.insert(StringMap::value_type("text-myown-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-myown-fore-color", "#207505"));
	defaultString.insert(StringMap::value_type("text-private-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-private-fore-color", "#2763CE"));
	defaultString.insert(StringMap::value_type("text-system-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-system-fore-color", "#1A1A1A"));
	defaultString.insert(StringMap::value_type("text-status-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-status-fore-color", "#7F7F7F"));
	defaultString.insert(StringMap::value_type("text-timestamp-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-timestamp-fore-color", "#43629A"));
	defaultString.insert(StringMap::value_type("text-mynick-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-mynick-fore-color", "#A52A2A"));
	defaultString.insert(StringMap::value_type("text-fav-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-fav-fore-color", "#FFA500"));
	defaultString.insert(StringMap::value_type("text-op-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-op-fore-color", "#0000FF"));
	defaultString.insert(StringMap::value_type("text-url-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-url-fore-color", "#0000FF"));
	defaultString.insert(StringMap::value_type("search-spy-a-color", "#339900"));
	defaultString.insert(StringMap::value_type("search-spy-t-color", "#ff0000"));
	defaultString.insert(StringMap::value_type("search-spy-q-color", "#b0b0b0"));
	defaultString.insert(StringMap::value_type("search-spy-c-color", "#b28600"));
	defaultString.insert(StringMap::value_type("search-spy-r-color", "#6c85ca"));
	
	defaultString.insert(StringMap::value_type("text-ip-fore-color", "#000000"));
	defaultString.insert(StringMap::value_type("text-ip-back-color", "#FFFFFF"));
	defaultString.insert(StringMap::value_type("text-cheat-fore-color", "#DE1515"));
	defaultString.insert(StringMap::value_type("text-cheat-back-color", "#EEE7E7"));
	
	defaultString.insert(StringMap::value_type("emoticons-pack", ""));
	defaultString.insert(StringMap::value_type("emoticons-icon-size", "24x24"));
	defaultString.insert(StringMap::value_type("notify-download-finished-title", _("Download finished")));
	defaultString.insert(StringMap::value_type("notify-download-finished-icon", ""));
	defaultString.insert(StringMap::value_type("notify-download-finished-ul-title", _("Download finished file list")));
	defaultString.insert(StringMap::value_type("notify-download-finished-ul-icon", ""));
	defaultString.insert(StringMap::value_type("notify-private-message-title", _("Private message")));
	defaultString.insert(StringMap::value_type("notify-private-message-icon", ""));
	defaultString.insert(StringMap::value_type("notify-hub-disconnect-title", _("Hub disconnected")));
	defaultString.insert(StringMap::value_type("notify-hub-disconnect-icon", ""));
	defaultString.insert(StringMap::value_type("notify-hub-connect-title", _("Hub connected")));
	defaultString.insert(StringMap::value_type("notify-hub-connect-icon", ""));
	defaultString.insert(StringMap::value_type("notify-fuser-join-title", _("Favorite user joined")));
	defaultString.insert(StringMap::value_type("notify-fuser-join-icon", ""));
	defaultString.insert(StringMap::value_type("notify-fuser-quit-title", _("Favorite user quit")));
	defaultString.insert(StringMap::value_type("notify-fuser-quit-icon", ""));
	/**/
	defaultString.insert(StringMap::value_type("notify-higl-title", _("Higliting string")));
	defaultString.insert(StringMap::value_type("notify-higl-icon", ""));
	/**/
	defaultString.insert(StringMap::value_type("theme-name", "default"));
	/**/
	defaultString.insert(StringMap::value_type("icon-smile", "bmdc-smile"));
	defaultString.insert(StringMap::value_type("icon-download", "bmdc-download"));
	defaultString.insert(StringMap::value_type("icon-favorite-hubs", "bmdc-favorite-hubs"));
	defaultString.insert(StringMap::value_type("icon-favorite-users", "bmdc-favorite-users"));
	defaultString.insert(StringMap::value_type("icon-finished-downloads", "bmdc-finished-downloads"));
	defaultString.insert(StringMap::value_type("icon-finished-uploads", "bmdc-finished-uploads"));
	defaultString.insert(StringMap::value_type("icon-hash", "bmdc-hash"));
	defaultString.insert(StringMap::value_type("icon-preferences", "bmdc-preferences"));
	defaultString.insert(StringMap::value_type("icon-public-hubs", "bmdc-public-hubs"));
	defaultString.insert(StringMap::value_type("icon-queue", "bmdc-queue"));
	defaultString.insert(StringMap::value_type("icon-search", "bmdc-search"));
	defaultString.insert(StringMap::value_type("icon-search-spy", "bmdc-search-spy"));
	defaultString.insert(StringMap::value_type("icon-notepad", "bmdc-notepad"));
	defaultString.insert(StringMap::value_type("icon-system", "bmdc-system"));
	defaultString.insert(StringMap::value_type("icon-adlsearch", "bmdc-adlsearch"));
	defaultString.insert(StringMap::value_type("icon-ignore", "bmdc-ignore-users"));
	defaultString.insert(StringMap::value_type("icon-away", "bmdc-away"));
	defaultString.insert(StringMap::value_type("icon-away-on", "bmdc-away-on"));
	defaultString.insert(StringMap::value_type("icon-none", "bmdc-none"));
	defaultString.insert(StringMap::value_type("icon-upload", "bmdc-upload"));
	defaultString.insert(StringMap::value_type("icon-quit", "bmdc-quit"));
	defaultString.insert(StringMap::value_type("icon-connect", "bmdc-connect"));
	defaultString.insert(StringMap::value_type("icon-file", GTK_STOCK_FILE));
	defaultString.insert(StringMap::value_type("icon-directory", GTK_STOCK_DIRECTORY));
	/**/
	defaultString.insert(StringMap::value_type("icon-pm-online","bmdc-pm-online"));
	defaultString.insert(StringMap::value_type("icon-pm-offline","bmdc-pm-offline"));
	defaultString.insert(StringMap::value_type("icon-hub-online","bmdc-hub-online"));
	defaultString.insert(StringMap::value_type("icon-hub-offline","bmdc-hub-offline"));
	defaultString.insert(StringMap::value_type("icon-highlight","bmdc-highlight"));
	defaultString.insert(StringMap::value_type("icon-limiting","bmdc-limiting"));
	defaultString.insert(StringMap::value_type("icon-limiting-on","bmdc-limiting-on"));
	
	defaultString.insert(StringMap::value_type("icon-normal", "bmdc-normal"));
	/*Icon of conn*/
	defaultString.insert(StringMap::value_type("icon-op", "bmdc-op"));
	defaultString.insert(StringMap::value_type("icon-modem", "bmdc-modem"));
	defaultString.insert(StringMap::value_type("icon-wireless", "bmdc-wireless"));
	defaultString.insert(StringMap::value_type("icon-dsl", "bmdc-dsl"));
	defaultString.insert(StringMap::value_type("icon-lan", "bmdc-lan"));
	defaultString.insert(StringMap::value_type("icon-netlimiter", "bmdc-netlimiter"));
	/***/
	defaultString.insert(StringMap::value_type("icon-ten", "bmdc-ten"));
	defaultString.insert(StringMap::value_type("icon-zeroone", "bmdc-zeroone"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone", "bmdc-zerozeroone"));
	defaultString.insert(StringMap::value_type("icon-other", "bmdc-other"));
	/*aways*/
	defaultString.insert(StringMap::value_type("icon-op-away", "bmdc-op-away"));
	defaultString.insert(StringMap::value_type("icon-modem-away", "bmdc-modem-away"));
	defaultString.insert(StringMap::value_type("icon-wireless-away", "bmdc-wireless-away"));
	defaultString.insert(StringMap::value_type("icon-dsl-away", "bmdc-dsl-away"));
	defaultString.insert(StringMap::value_type("icon-lan-away", "bmdc-lan-away"));
	defaultString.insert(StringMap::value_type("icon-netlimiter-away", "bmdc-netlimiter-away"));
	/***/
	defaultString.insert(StringMap::value_type("icon-ten-away", "bmdc-ten-away"));
	defaultString.insert(StringMap::value_type("icon-zeroone-away", "bmdc-zeroone-away"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone-away", "bmdc-zerozeroone-away"));
	defaultString.insert(StringMap::value_type("icon-other-away", "bmdc-other-away"));
	/*pasive**/
	defaultString.insert(StringMap::value_type("icon-op-pasive", "bmdc-op-pasive"));
	defaultString.insert(StringMap::value_type("icon-modem-pasive", "bmdc-modem-pasive"));
	defaultString.insert(StringMap::value_type("icon-wireless-pasive", "bmdc-wireless-pasive"));
	defaultString.insert(StringMap::value_type("icon-dsl-pasive", "bmdc-dsl-pasive"));
	defaultString.insert(StringMap::value_type("icon-lan-pasive", "bmdc-lan-pasive"));
	defaultString.insert(StringMap::value_type("icon-netlimiter-pasive", "bmdc-netlimiter-pasive"));
	/***/
	defaultString.insert(StringMap::value_type("icon-ten-pasive", "bmdc-ten-pasive"));
	defaultString.insert(StringMap::value_type("icon-zeroone-pasive", "bmdc-zeroone-pasive"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone-pasive", "bmdc-zerozeroone-pasive"));
	defaultString.insert(StringMap::value_type("icon-other-pasive", "bmdc-other-pasive"));
	/*aways*/
	defaultString.insert(StringMap::value_type("icon-op-away-pasive", "bmdc-op-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-modem-away-pasive", "bmdc-modem-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-wireless-away-pasive", "bmdc-wireless-awayv"));
	defaultString.insert(StringMap::value_type("icon-dsl-away-pasive", "bmdc-dsl-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-lan-away-pasive", "bmdc-lan-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-netlimiter-away-pasive", "bmdc-netlimiter-away-pasive"));
	/***/
	defaultString.insert(StringMap::value_type("icon-ten-away-pasive", "bmdc-ten-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-zeroone-away-pasive", "bmdc-zeroone-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone-away-pasive", "bmdc-zerozeroone-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-other-away-pasive", "bmdc-other-away-pasive"));
	//Sounds
	defaultString.insert(StringMap::value_type("sound-command", "aplay -q"));
	/**/
	//customs aliasies
	defaultString.insert(StringMap::value_type("custom-aliases", ""));
	/*for ratio*/
	defaultString.insert(StringMap::value_type("up-st", "0"));
	defaultString.insert(StringMap::value_type("dw-st", "0"));
	/*for UL color text*/
	defaultString.insert(StringMap::value_type("userlist-text-operator", "#1E90FF"));
	defaultString.insert(StringMap::value_type("userlist-text-pasive", "#747677"));
	defaultString.insert(StringMap::value_type("userlist-text-protected", "#8B6914"));
	defaultString.insert(StringMap::value_type("userlist-text-favorite", "#ff0000"));
	defaultString.insert(StringMap::value_type("userlist-text-ignored", "#9affaf"));
	defaultString.insert(StringMap::value_type("userlist-text-normal", "#000000"));

	load();
}

WulforSettingsManager::~WulforSettingsManager()
{
	save();

	for_each(previewApps.begin(), previewApps.end(), DeleteFunction());
}

int WulforSettingsManager::getInt(const string &key, bool useDefault)
{
	dcassert(intMap.find(key) != intMap.end() || defaultInt.find(key) != defaultInt.end());

	if (useDefault)
		return defaultInt[key];

	if (intMap.find(key) == intMap.end())
		return defaultInt[key];
	else
		return intMap[key];
}

string WulforSettingsManager::getString(const string &key, bool useDefault)
{
	dcassert(stringMap.find(key) != stringMap.end() || defaultString.find(key) != defaultString.end());

	if (useDefault)
		return defaultString[key];

	if (stringMap.find(key) == stringMap.end())
		return defaultString[key];
	else
		return stringMap[key];
}

bool WulforSettingsManager::getBool(const string &key, bool useDefault)
{
	return (getInt(key, useDefault) != 0);
}

void WulforSettingsManager::set(const string &key, int value)
{
	dcassert(defaultInt.find(key) != defaultInt.end());
	intMap[key] = value;
}

void WulforSettingsManager::set(const string &key, bool value)
{
	set(key, (int)value);
}

void WulforSettingsManager::set(const string &key, const string &value)
{
	dcassert(defaultString.find(key) != defaultString.end());
	stringMap[key] = value;
}

void WulforSettingsManager::load()
{
	try
	{
		SimpleXML xml;
		xml.fromXML(File(configFile, File::READ, File::OPEN).read());
		xml.resetCurrentChild();
		xml.stepIn();

		if (xml.findChild("Settings"))
		{
			xml.stepIn();

			IntMap::iterator iit;
			for (iit = defaultInt.begin(); iit != defaultInt.end(); ++iit)
			{
				if (xml.findChild(iit->first))
					intMap.insert(IntMap::value_type(iit->first, Util::toInt(xml.getChildData())));
				xml.resetCurrentChild();
			}

			StringMap::iterator sit;
			for (sit = defaultString.begin(); sit != defaultString.end(); ++sit)
			{
				if (xml.findChild(sit->first))
					stringMap.insert(StringMap::value_type(sit->first, xml.getChildData()));
				xml.resetCurrentChild();
			}

			xml.stepOut();
		}

		if (xml.findChild("PreviewApps"))
		{
			xml.stepIn();

			for (;xml.findChild("Application");)
				addPreviewApp(xml.getChildAttrib("Name"), xml.getChildAttrib("Application"), xml.getChildAttrib("Extension"));

			xml.stepOut();
		}
	}
	catch (const Exception&)
	{
	}
}

void WulforSettingsManager::save()
{
	SimpleXML xml;
	xml.addTag(g_get_application_name());
	xml.stepIn();
	xml.addTag("Settings");
	xml.stepIn();

	IntMap::iterator iit;
	for (iit = intMap.begin(); iit != intMap.end(); ++iit)
	{
		xml.addTag(iit->first, iit->second);
		xml.addChildAttrib(string("type"), string("int"));
	}

	StringMap::iterator sit;
	for (sit = stringMap.begin(); sit != stringMap.end(); ++sit)
	{
		xml.addTag(sit->first, sit->second);
		xml.addChildAttrib(string("type"), string("string"));
	}

	xml.stepOut();

	xml.addTag("PreviewApps");
	xml.stepIn();

	for(PreviewApp::Iter i = previewApps.begin(); i != previewApps.end(); ++i)
	{
		xml.addTag("Application");
		xml.addChildAttrib("Name", (*i)->name);
		xml.addChildAttrib("Application", (*i)->app);
		xml.addChildAttrib("Extension", (*i)->ext);
	}

	xml.stepOut();

	try
	{
		File out(configFile + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(configFile);
		File::renameFile(configFile + ".tmp", configFile);
	}
	catch (const FileException &)
	{
	}
}

PreviewApp* WulforSettingsManager::addPreviewApp(string name, string app, string ext)
{
	PreviewApp* pa = new PreviewApp(name, app, ext);
	previewApps.push_back(pa);

	return pa;
}

bool WulforSettingsManager::removePreviewApp(string &name)
{
	PreviewApp::size index;

	if (getPreviewApp(name, index))
	{
		delete previewApps[index];
		previewApps.erase(previewApps.begin() + index);

		return true;
	}

	return false;
}

PreviewApp* WulforSettingsManager::applyPreviewApp(string &oldName, string &newName, string &app, string &ext)
{
	PreviewApp::size index;
	PreviewApp *pa = NULL;

	if(getPreviewApp(oldName, index))
	{
		delete previewApps[index];
		pa = new PreviewApp(newName, app, ext);
		previewApps[index] = pa;
	}

	return pa;
}

bool WulforSettingsManager::getPreviewApp(string &name)
{
	PreviewApp::size index;
	return getPreviewApp(name, index);
}

bool WulforSettingsManager::getPreviewApp(string &name, PreviewApp::size &index)
{
	index = 0;

	for (PreviewApp::Iter item = previewApps.begin(); item != previewApps.end(); ++item, ++index)
		if((*item)->name == name) return true;

	return false;
}

const std::string WulforSettingsManager::parseCmd(const std::string cmd)
{
    StringTokenizer<string> sl(cmd, ' ');
        if (sl.getTokens().size() == 2) {
            if (intMap.find(sl.getTokens().at(0)) != intMap.end() && defaultInt.find(sl.getTokens().at(0)) != defaultInt.end()) {
                int i = atoi(sl.getTokens().at(1).c_str());
                WSET(sl.getTokens().at(0), i);
                save();
            }
            else if (stringMap.find(sl.getTokens().at(0)) != stringMap.end() && defaultString.find(sl.getTokens().at(0)) != defaultString.end()) {
                WSET(sl.getTokens().at(0), sl.getTokens().at(1));
                save();
            } else
                return _("Error: setting not found!");
            string msg = _("Change setting ") + string(sl.getTokens().at(0)) + _(" to ") + string(sl.getTokens().at(1));
            return msg;
        }
    return _("Error: params have been not 2!");
}
