/*
 * Copyright © 2004-2016 Jens Oknelid, paskharen@gmail.com
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
#include "wulformanager.hh"
#include "../dcpp/File.h"
#include "../dcpp/SimpleXML.h"
#include "../dcpp/Util.h"
#include "../dcpp/StringTokenizer.h"
#include "WulforUtil.hh"
#include "message.hh"
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#ifdef _WIN32
#include <direct.h>
#endif

using namespace std;
using namespace dcpp;

WulforSettingsManager::WulforSettingsManager():
	configFile(Util::getPath(Util::PATH_USER_CONFIG) + "BMDC.xml")
{
	#if !GTK_CHECK_VERSION(3,14,1)
	//obtain "background" color from theme what used on 1st startup
	GdkRGBA color;
	//  "gtk-theme-name"           gchar*    : Read / Write (from gtk-doc)
	gchar *name_theme = NULL;
	g_object_get(gtk_settings_get_default(),"gtk-theme-name",&name_theme,NULL);
	GtkThemingEngine *engine = gtk_theming_engine_load(name_theme);
	gtk_theming_engine_get_background_color (engine,(GtkStateFlags)GTK_STATE_FLAG_NORMAL,&color);
	#else
	//NOTE: get from theme/sys/ how?
		GdkRGBA color;
		gdk_rgba_parse (&color,"white");
	#endif
	
	defaultInt.insert(IntMap::value_type("country-hub-icon",1));

	defaultInt.insert(IntMap::value_type("main-window-maximized", 0));
	defaultInt.insert(IntMap::value_type("main-window-size-x", 875));
	defaultInt.insert(IntMap::value_type("main-window-size-y", 685));
	defaultInt.insert(IntMap::value_type("main-window-pos-x", 100));
	defaultInt.insert(IntMap::value_type("main-window-pos-y", 100));
	defaultInt.insert(IntMap::value_type("main-window-no-close", 0));
	defaultInt.insert(IntMap::value_type("transfer-pane-position", 482));
	defaultInt.insert(IntMap::value_type("downloadqueue-pane-position", 200));
	defaultInt.insert(IntMap::value_type("sharebrowser-pane-position", 200));
	defaultInt.insert(IntMap::value_type("tab-position", 0));
	defaultInt.insert(IntMap::value_type("toolbar-style", 5));
	defaultInt.insert(IntMap::value_type("sound-pm-open", 0));
	defaultInt.insert(IntMap::value_type("sound-pm", 1));
	defaultInt.insert(IntMap::value_type("sound-download-begins-use", 0));
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
	defaultInt.insert(IntMap::value_type("toolbar-button-connect", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-fav-hubs", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-fav-users", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-public-hubs", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-settings", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-hash", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-search", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-search-spy", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-search-adl", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-queue", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-quit", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-finished-downloads", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-finished-uploads", 1));
	defaultInt.insert(IntMap::value_type("notify-download-finished-use", 0));
	defaultInt.insert(IntMap::value_type("notify-download-finished-ul-use", 0));
	defaultInt.insert(IntMap::value_type("notify-private-message-use", 0));
	defaultInt.insert(IntMap::value_type("notify-hub-disconnect-use", 0));
	defaultInt.insert(IntMap::value_type("notify-hub-connect-use", 0));
	defaultInt.insert(IntMap::value_type("notify-fuser-join", 0));
	defaultInt.insert(IntMap::value_type("notify-fuser-quit", 0));
	defaultInt.insert(IntMap::value_type("notify-pm-length", 50));
	defaultInt.insert(IntMap::value_type("notify-icon-size", 3));
	defaultInt.insert(IntMap::value_type("notify-only-not-active", 0));
	defaultInt.insert(IntMap::value_type("status-icon-blink-use", 1));
	defaultInt.insert(IntMap::value_type("search-spy-frame", 50));
	defaultInt.insert(IntMap::value_type("search-spy-waiting", 40));
	defaultInt.insert(IntMap::value_type("search-spy-top", 4));
	defaultInt.insert(IntMap::value_type("magnet-action", -1));//default show magnet dialog
//open tabs/win
	defaultInt.insert(IntMap::value_type("open-public", 0));
	defaultInt.insert(IntMap::value_type("open-favorite-hubs", 0));
	defaultInt.insert(IntMap::value_type("open-queue", 0));
	defaultInt.insert(IntMap::value_type("open-finished-downloads", 0));
	defaultInt.insert(IntMap::value_type("open-finished-uploads", 0));
	defaultInt.insert(IntMap::value_type("open-favorite-users", 0));
	defaultInt.insert(IntMap::value_type("open-search-spy", 0));

	defaultInt.insert(IntMap::value_type("toolbar-position", 1));
	defaultInt.insert(IntMap::value_type("toolbar-small", 1));
	///[BMDC++
	defaultInt.insert(IntMap::value_type("bold-all-tab", 1));
	defaultInt.insert(IntMap::value_type("use-close-button", 0));
	defaultInt.insert(IntMap::value_type("size-label-box-bookentry",15));
	defaultInt.insert(IntMap::value_type("max-tooltips", 10));
	defaultInt.insert(IntMap::value_type("show-commands", 0));//per Fav dcpp
	defaultInt.insert(IntMap::value_type("use-highlighting", 0));//per Fav?
	defaultInt.insert(IntMap::value_type("use-dns", 0));
	defaultInt.insert(IntMap::value_type("log-messages", 0));//per fav
	defaultInt.insert(IntMap::value_type("text-cheat-bold", 1));
	defaultInt.insert(IntMap::value_type("text-cheat-italic", 0));

	defaultInt.insert(IntMap::value_type("text-ip-bold", 1));
	defaultInt.insert(IntMap::value_type("text-ip-italic", 1));

	defaultInt.insert(IntMap::value_type("text-high-bold", 1));
	defaultInt.insert(IntMap::value_type("text-high-italic", 1));

	defaultInt.insert(IntMap::value_type("notify-high-use", 1));
	defaultInt.insert(IntMap::value_type("notify-hub-chat-use", 1));

	defaultInt.insert(IntMap::value_type("toolbar-button-notepad", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-system", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-away", 1));
	defaultInt.insert(IntMap::value_type("toolbar-limit-bandwith", 1));
	defaultInt.insert(IntMap::value_type("toolbar-button-limiting", 1));
	defaultInt.insert(IntMap::value_type("open-notepad", 0));
	defaultInt.insert(IntMap::value_type("open-system", 1));
	defaultInt.insert(IntMap::value_type("open-upload-queue", 0));
	defaultInt.insert(IntMap::value_type("cmd-debug-hub-in", 1));
	defaultInt.insert(IntMap::value_type("cmd-debug-hub-out", 1));
	defaultInt.insert(IntMap::value_type("cmd-debug-client-out", 1));
	defaultInt.insert(IntMap::value_type("cmd-debug-client-in", 1));
	defaultInt.insert(IntMap::value_type("cmd-debug-detection", 0));

	defaultInt.insert(IntMap::value_type("custom-font-size", 0));
	defaultInt.insert(IntMap::value_type("book-font-size", 1));// in %
	defaultInt.insert(IntMap::value_type("book-three-button-disable", 0));
	defaultInt.insert(IntMap::value_type("key-hub-with-ctrl", 0));
	defaultInt.insert(IntMap::value_type("colorize-tab-text", 1));
    ///BMDC]
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
	//redo this or rem?
	defaultString.insert(StringMap::value_type("hub-order", "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14"));
	defaultString.insert(StringMap::value_type("hub-width", "157,75,85,100,85,85,90,70,100,50,50,80,80,80,80"));
	defaultString.insert(StringMap::value_type("hub-visibility", "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1"));
	//end
	defaultString.insert(StringMap::value_type("transfers-order", ""));
	defaultString.insert(StringMap::value_type("transfers-width", ""));
	defaultString.insert(StringMap::value_type("transfers-visibility", ""));
	defaultString.insert(StringMap::value_type("publichubs-order", ""));
	defaultString.insert(StringMap::value_type("publichubs-width", ""));
	defaultString.insert(StringMap::value_type("publichubs-visibility", ""));
	defaultString.insert(StringMap::value_type("search-order", ""));
	defaultString.insert(StringMap::value_type("search-width", ""));
	defaultString.insert(StringMap::value_type("search-visibility", ""));
	defaultString.insert(StringMap::value_type("searchadl-order", ""));
	defaultString.insert(StringMap::value_type("searchadl-width", ""));
	defaultString.insert(StringMap::value_type("searchadl-visibility", ""));
	defaultString.insert(StringMap::value_type("searchspy-order", ""));
	defaultString.insert(StringMap::value_type("searchspy-width", ""));
	defaultString.insert(StringMap::value_type("searchspy-visibility", ""));
	defaultString.insert(StringMap::value_type("sharebrowser-order", ""));
	defaultString.insert(StringMap::value_type("sharebrowser-width", ""));
	defaultString.insert(StringMap::value_type("sharebrowser-visibility", ""));
	defaultString.insert(StringMap::value_type("default-charset", WulforUtil::ENCODING_LOCALE));
 	defaultString.insert(StringMap::value_type("sound-download-begins", ""));
	defaultString.insert(StringMap::value_type("sound-download-finished", ""));
	defaultString.insert(StringMap::value_type("sound-download-finished-ul", ""));
	defaultString.insert(StringMap::value_type("sound-upload-finished", ""));
	defaultString.insert(StringMap::value_type("sound-private-message", ""));
	defaultString.insert(StringMap::value_type("sound-hub-connect", ""));
	defaultString.insert(StringMap::value_type("sound-hub-disconnect", ""));
	defaultString.insert(StringMap::value_type("sound-fuser-join", ""));
	defaultString.insert(StringMap::value_type("sound-fuser-quit", ""));
	defaultString.insert(StringMap::value_type("text-general-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-general-fore-color", "#4D4D4D"));
	defaultString.insert(StringMap::value_type("text-myown-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-myown-fore-color", "#207505"));
	defaultString.insert(StringMap::value_type("text-private-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-private-fore-color", "#2763CE"));
	defaultString.insert(StringMap::value_type("text-system-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-system-fore-color", "#1A1A1A"));
	defaultString.insert(StringMap::value_type("text-status-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-status-fore-color", "#7F7F7F"));
	defaultString.insert(StringMap::value_type("text-timestamp-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-timestamp-fore-color", "#43629A"));
	defaultString.insert(StringMap::value_type("text-mynick-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-mynick-fore-color", "#A52A2A"));
	defaultString.insert(StringMap::value_type("text-fav-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-fav-fore-color", "#FFA500"));
	defaultString.insert(StringMap::value_type("text-op-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-op-fore-color", "#0000FF"));
	defaultString.insert(StringMap::value_type("text-url-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-url-fore-color", "#0000FF"));
	defaultString.insert(StringMap::value_type("search-spy-a-color", "#339900"));
	defaultString.insert(StringMap::value_type("search-spy-t-color", "#ff0000"));
	defaultString.insert(StringMap::value_type("search-spy-q-color", "#b0b0b0"));
	defaultString.insert(StringMap::value_type("search-spy-c-color", "#b28600"));
	defaultString.insert(StringMap::value_type("search-spy-r-color", "#6c85ca"));

	defaultString.insert(StringMap::value_type("emoticons-icon-size", "24x24"));//per Fav?
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

	defaultString.insert(StringMap::value_type("theme-name", "default"));
	defaultString.insert(StringMap::value_type("icon-dc++", "bmdc-dc++"));
	defaultString.insert(StringMap::value_type("icon-dc++-fw", "bmdc-dc++-fw"));
	defaultString.insert(StringMap::value_type("icon-dc++-fw-op", "bmdc-dc++-fw-op"));
	defaultString.insert(StringMap::value_type("icon-dc++-op", "bmdc-dc++-op"));
	defaultString.insert(StringMap::value_type("icon-normal", "bmdc-normal"));
	defaultString.insert(StringMap::value_type("icon-normal-fw", "bmdc-normal-fw"));
	defaultString.insert(StringMap::value_type("icon-normal-fw-op", "bmdc-normal-fw-op"));
	defaultString.insert(StringMap::value_type("icon-normal-op", "bmdc-normal-op"));
	defaultString.insert(StringMap::value_type("icon-smile", "bmdc-smile"));
	defaultString.insert(StringMap::value_type("icon-download", "bmdc-download"));
	defaultString.insert(StringMap::value_type("icon-favorite-hubs", "bmdc-favorite-hubs"));
	defaultString.insert(StringMap::value_type("icon-favorite-hubs-on", "bmdc-favorite-hubs-on"));
	defaultString.insert(StringMap::value_type("icon-favorite-users", "bmdc-favorite-users"));
	defaultString.insert(StringMap::value_type("icon-favorite-users-on", "bmdc-favorite-users-on"));
	defaultString.insert(StringMap::value_type("icon-finished-downloads", "bmdc-finished-downloads"));
	defaultString.insert(StringMap::value_type("icon-finished-downloads-on", "bmdc-finished-downloads-on"));
	defaultString.insert(StringMap::value_type("icon-finished-uploads", "bmdc-finished-uploads"));
	defaultString.insert(StringMap::value_type("icon-finished-uploads-on", "bmdc-finished-uploads-on"));
	defaultString.insert(StringMap::value_type("icon-hash", "bmdc-hash"));
	defaultString.insert(StringMap::value_type("icon-preferences", "bmdc-preferences"));
	defaultString.insert(StringMap::value_type("icon-public-hubs", "bmdc-public-hubs"));
	defaultString.insert(StringMap::value_type("icon-public-hubs-on", "bmdc-public-hubs-on"));
	defaultString.insert(StringMap::value_type("icon-queue", "bmdc-queue"));
	defaultString.insert(StringMap::value_type("icon-queue-on", "bmdc-queue-on"));
	defaultString.insert(StringMap::value_type("icon-search", "bmdc-search"));
	defaultString.insert(StringMap::value_type("icon-search-adl", "bmdc-search-adl"));
	defaultString.insert(StringMap::value_type("icon-search-adl-on", "bmdc-search-adl-on"));
	defaultString.insert(StringMap::value_type("icon-search-spy", "bmdc-search-spy"));
	defaultString.insert(StringMap::value_type("icon-search-spy-on", "bmdc-search-spy-on"));
	defaultString.insert(StringMap::value_type("icon-upload", "bmdc-upload"));
	defaultString.insert(StringMap::value_type("icon-quit", "bmdc-quit"));
	defaultString.insert(StringMap::value_type("icon-connect", "bmdc-connect"));
	defaultString.insert(StringMap::value_type("icon-file", BMDC_STOCK_FILE));
	defaultString.insert(StringMap::value_type("icon-directory", BMDC_STOCK_DIRECTORY));
	defaultString.insert(StringMap::value_type("icon-pm-online", "bmdc-pm-online"));
	defaultString.insert(StringMap::value_type("icon-pm-offline", "bmdc-pm-offline"));
	defaultString.insert(StringMap::value_type("icon-hub-online", "bmdc-hub-online"));
	defaultString.insert(StringMap::value_type("icon-hub-offline", "bmdc-hub-offline"));
	//..[BMDC++
	defaultString.insert(StringMap::value_type("icon-limiting","bmdc-limiting"));
	defaultString.insert(StringMap::value_type("icon-limiting-on","bmdc-limiting-on"));
	defaultString.insert(StringMap::value_type("icon-notepad","bmdc-notepad"));
	defaultString.insert(StringMap::value_type("icon-notepad-on","bmdc-notepad-on"));
	defaultString.insert(StringMap::value_type("icon-ignore", "bmdc-ignore-users"));
	defaultString.insert(StringMap::value_type("icon-ignore-on", "bmdc-ignore-users-on"));
	defaultString.insert(StringMap::value_type("icon-away", "bmdc-away"));
	defaultString.insert(StringMap::value_type("icon-away-on", "bmdc-away-on"));
	defaultString.insert(StringMap::value_type("icon-system", "bmdc-system"));
	defaultString.insert(StringMap::value_type("icon-system-on", "bmdc-system-on"));
	defaultString.insert(StringMap::value_type("icon-highlight", "bmdc-highlight"));
	/* * */
	defaultString.insert(StringMap::value_type("icon-normal", "bmdc-normal"));
	/* Icon of conn */
	defaultString.insert(StringMap::value_type("icon-op", "bmdc-op"));
	defaultString.insert(StringMap::value_type("icon-modem", "bmdc-modem"));
	defaultString.insert(StringMap::value_type("icon-wireless", "bmdc-wireless"));
	defaultString.insert(StringMap::value_type("icon-dsl", "bmdc-dsl"));
	defaultString.insert(StringMap::value_type("icon-lan", "bmdc-lan"));
	defaultString.insert(StringMap::value_type("icon-netlimiter", "bmdc-netlimiter"));
	/* * */
	defaultString.insert(StringMap::value_type("icon-ten", "bmdc-ten"));
	defaultString.insert(StringMap::value_type("icon-zeroone", "bmdc-zeroone"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone", "bmdc-zerozeroone"));
	defaultString.insert(StringMap::value_type("icon-other", "bmdc-other"));
	/* aways */
	defaultString.insert(StringMap::value_type("icon-op-away", "bmdc-op-away"));
	defaultString.insert(StringMap::value_type("icon-modem-away", "bmdc-modem-away"));
	defaultString.insert(StringMap::value_type("icon-wireless-away", "bmdc-wireless-away"));
	defaultString.insert(StringMap::value_type("icon-dsl-away", "bmdc-dsl-away"));
	defaultString.insert(StringMap::value_type("icon-lan-away", "bmdc-lan-away"));
	defaultString.insert(StringMap::value_type("icon-netlimiter-away", "bmdc-netlimiter-away"));
	/* * */
	defaultString.insert(StringMap::value_type("icon-ten-away", "bmdc-ten-away"));
	defaultString.insert(StringMap::value_type("icon-zeroone-away", "bmdc-zeroone-away"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone-away", "bmdc-zerozeroone-away"));
	defaultString.insert(StringMap::value_type("icon-other-away", "bmdc-other-away"));
	/* pasive */
	defaultString.insert(StringMap::value_type("icon-op-pasive", "bmdc-op-pasive"));
	defaultString.insert(StringMap::value_type("icon-modem-pasive", "bmdc-modem-pasive"));
	defaultString.insert(StringMap::value_type("icon-wireless-pasive", "bmdc-wireless-pasive"));
	defaultString.insert(StringMap::value_type("icon-dsl-pasive", "bmdc-dsl-pasive"));
	defaultString.insert(StringMap::value_type("icon-lan-pasive", "bmdc-lan-pasive"));
	defaultString.insert(StringMap::value_type("icon-netlimiter-pasive", "bmdc-netlimiter-pasive"));
	/* * */
	defaultString.insert(StringMap::value_type("icon-ten-pasive", "bmdc-ten-pasive"));
	defaultString.insert(StringMap::value_type("icon-zeroone-pasive", "bmdc-zeroone-pasive"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone-pasive", "bmdc-zerozeroone-pasive"));
	defaultString.insert(StringMap::value_type("icon-other-pasive", "bmdc-other-pasive"));
	/* aways */
	defaultString.insert(StringMap::value_type("icon-op-away-pasive", "bmdc-op-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-modem-away-pasive", "bmdc-modem-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-wireless-away-pasive", "bmdc-wireless-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-dsl-away-pasive", "bmdc-dsl-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-lan-away-pasive", "bmdc-lan-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-netlimiter-away-pasive", "bmdc-netlimiter-away-pasive"));
	/* * */
	defaultString.insert(StringMap::value_type("icon-ten-away-pasive", "bmdc-ten-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-zeroone-away-pasive", "bmdc-zeroone-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-zerozeroone-away-pasive", "bmdc-zerozeroone-away-pasive"));
	defaultString.insert(StringMap::value_type("icon-other-away-pasive", "bmdc-other-away-pasive"));
	//TODO ? Per Fav?
	/* for UL color text */
	defaultString.insert(StringMap::value_type("userlist-text-operator", "#000000"));
	defaultString.insert(StringMap::value_type("userlist-text-pasive", "#747677"));
	defaultString.insert(StringMap::value_type("userlist-text-protected", "#8B6914"));
	defaultString.insert(StringMap::value_type("userlist-text-favorite", "#ff0000"));
	defaultString.insert(StringMap::value_type("userlist-text-ignored", "#9affaf"));
	defaultString.insert(StringMap::value_type("userlist-text-normal", "#000000"));
	defaultString.insert(StringMap::value_type("userlist-text-bot-hub", "#1E90FF"));
	/* for UserList Background*/
	defaultString.insert(StringMap::value_type("userlist-bg-operator", "#1E90FF"));
	defaultString.insert(StringMap::value_type("userlist-bg-bot-hub", "#000000"));
	defaultString.insert(StringMap::value_type("userlist-bg-favorite", "#00FF00"));
	defaultString.insert(StringMap::value_type("userlist-bg-normal", "#BFBFBF"));
	defaultString.insert(StringMap::value_type("userlist-bg-pasive", "#BFBFBF"));
	defaultString.insert(StringMap::value_type("userlist-bg-protected", "#BFBFBF"));
	defaultString.insert(StringMap::value_type("userlist-bg-ignored", "#BFBFBF"));

	defaultString.insert(StringMap::value_type("userlist-text-virus","white"));
	defaultString.insert(StringMap::value_type("userlist-bg-virus","red"));
	defaultString.insert(StringMap::value_type("userlist-bg-virus-an","#A52A2A"));

	/*end*/
	defaultString.insert(StringMap::value_type("custom-aliases", "#"));
	/* Extended text color*/
	defaultString.insert(StringMap::value_type("text-cheat-fore-color", "red"));
	defaultString.insert(StringMap::value_type("text-cheat-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-ip-fore-color", "black"));
	defaultString.insert(StringMap::value_type("text-ip-back-color", WulforUtil::colorToString(&color)));
	defaultString.insert(StringMap::value_type("text-high-fore-color", "black"));
	defaultString.insert(StringMap::value_type("text-high-back-color", WulforUtil::colorToString(&color)));

	defaultString.insert(StringMap::value_type("notify-high-title", _("Highliting")));
	defaultString.insert(StringMap::value_type("notify-high-icon", "bmdc-icon-highliting"));

	defaultString.insert(StringMap::value_type("notify-hub-chat-title", _("Hub Chat")));
	defaultString.insert(StringMap::value_type("notify-hub-chat-icon", "bmdc-hub-online"));

	defaultString.insert(StringMap::value_type("share-shared", "#1E90FF"));
	defaultString.insert(StringMap::value_type("share-queue", "#E32020"));
	defaultString.insert(StringMap::value_type("share-default", "black"));
	defaultString.insert(StringMap::value_type("sound-command", "aplay -q"));
	defaultString.insert(StringMap::value_type("last-searchs", "."));

	defaultString.insert(StringMap::value_type("color-tab-text-bold", "blue"));
	defaultString.insert(StringMap::value_type("color-tab-text-urgent", "blue"));

	defaultInt.insert(IntMap::value_type("colored-tabs-fav-hubs", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-fav-users", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-public", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-download-quene", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-downloads", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-uploads", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-searchs", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-spy", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-adl", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-pm", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-hub", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-shareb", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-notepad", 1));
	defaultInt.insert(IntMap::value_type("colored-tabs-system", 1));

	defaultInt.insert(IntMap::value_type("colored-tabs-pm-unread",1));
	defaultInt.insert(IntMap::value_type("colored-tabs-hub-unread",1));
	defaultInt.insert(IntMap::value_type("colored-tabs-searchs-unread",1));
	defaultInt.insert(IntMap::value_type("colored-tabs-system-unread",1));

	defaultString.insert(StringMap::value_type("colored-tabs-fav-hubs-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-fav-users-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-public-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-download-quene-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-downloads-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-uploads-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-searchs-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-spy-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-adl-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-pm-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-hub-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-shareb-color-bg","#7F7F7F"));
	defaultString.insert(StringMap::value_type("colored-tabs-notepad-color-bg","green"));
	defaultString.insert(StringMap::value_type("colored-tabs-system-color-bg","white"));

	defaultString.insert(StringMap::value_type("colored-tabs-pm-color-bg-unread","#E5E5E5"));
	defaultString.insert(StringMap::value_type("colored-tabs-hub-color-bg-unread","#E5E5E5"));
	defaultString.insert(StringMap::value_type("colored-tabs-system-color-bg-unread","#90EE90"));
	defaultString.insert(StringMap::value_type("colored-tabs-searchs-color-bg-unread","#E5E5E5"));

	defaultString.insert(StringMap::value_type("colored-tabs-fav-hubs-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-fav-users-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-public-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-download-quene-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-downloads-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-uploads-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-searchs-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-spy-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-adl-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-pm-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-hub-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-shareb-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-notepad-color-fg","black"));
	defaultString.insert(StringMap::value_type("colored-tabs-system-color-fg","black"));
	//..
	defaultString.insert(StringMap::value_type("colored-tabs-pm-color-fg-unread","#FF0000"));
	defaultString.insert(StringMap::value_type("colored-tabs-hub-color-fg-unread","#90EE90"));
	defaultString.insert(StringMap::value_type("colored-tabs-system-color-fg-unread","#4D4D4D"));
	defaultString.insert(StringMap::value_type("colored-tabs-searchs-color-fg-unread","#E5E5E5"));
	/*
	 * hub db click
	 * /ws double-click-action #
	 * 0 - Browse FL
	 * 1 - Nick to Chat
	 * 2 - PM to nick
	 * 3 - match quene
	 * 4 - grant slot
	 * 5 - add to fav users
	 * 6 - Partial Filelist
	 * message.hh
	*/
	defaultInt.insert(IntMap::value_type("double-click-action",(int)CActions::BROWSE));
	defaultInt.insert(IntMap::value_type("hide-transfers",0));

	defaultString.insert(StringMap::value_type("menu-userlist-color","blue"));//TODO: dcpp set
	defaultString.insert(StringMap::value_type("text-cmd-fore-color","black"));
	defaultString.insert(StringMap::value_type("text-system-fore-color","black"));

	load();

	string path_image = Util::getPath(Util::PATH_USER_CONFIG) + "Images/";
	#ifndef _WIN32
	g_mkdir_with_parents(path_image.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
	#else
	_mkdir(path_image.c_str());
	#endif
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
	xml.addTag("BMDCPlusPlus");//@prg name
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

const string WulforSettingsManager::parseCmd(const string cmd)
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
