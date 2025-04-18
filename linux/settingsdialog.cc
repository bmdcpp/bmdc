/*
* Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "settingsdialog.hh"

#include "../dcpp/File.h"
#include "../dcpp/SimpleXML.h"
#include "../dcpp/CryptoManager.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/NmdcHub.h"
#include "../dcpp/ShareManager.h"
#include "../dcpp/StringTokenizer.h"
#ifdef HAVE_LIBTAR
	#include "../dcpp/BackupManager.h"
#endif
#include "../dcpp/ConnectivityManager.h"
#include "settingsmanager.hh"
#include "sound.hh"
#include "notify.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"


#define GTK_ICON_LOOKUP_FORCE_SVG GTK_ICON_LOOKUP_FORCE_REGULAR
#define ICON_SIZE 32
#define ICON_SIZE_NORMAL 22

#define g_c_b_g gtk_color_chooser_get_rgba
#define g_c_b_s gtk_color_chooser_set_rgba
#define G_C_B GTK_COLOR_CHOOSER
#define g_c_p(x,y) gdk_rgba_parse(y,x)

using namespace std;
using namespace dcpp;

Settings::Settings(GtkWindow* parent):
	DialogEntry(Entry::SETTINGS_DIALOG, "settingsdialog", parent)
{
	// the reference count on the buffer is not incremented and caller of this function won't own a new reference.
//textStyleBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(getWidget("textViewPreviewStyles")));
//	gtk_text_view_set_buffer(GTK_TEXT_VIEW(getWidget("textViewPreviewStylesTheme")), textStyleBuffer);

	defaultStringTheme.insert(StringMap::value_type("icon-dc++", "bmdc-dc++"));
	defaultStringTheme.insert(StringMap::value_type("icon-dc++-fw", "bmdc-dc++-fw"));
	defaultStringTheme.insert(StringMap::value_type("icon-dc++-fw-op", "bmdc-dc++-fw-op"));
	defaultStringTheme.insert(StringMap::value_type("icon-dc++-op", "bmdc-dc++-op"));
	defaultStringTheme.insert(StringMap::value_type("icon-normal", "bmdc-normal"));
	defaultStringTheme.insert(StringMap::value_type("icon-normal-fw", "bmdc-normal-fw"));
	defaultStringTheme.insert(StringMap::value_type("icon-normal-fw-op", "bmdc-normal-fw-op"));
	defaultStringTheme.insert(StringMap::value_type("icon-normal-op", "bmdc-normal-op"));
	defaultStringTheme.insert(StringMap::value_type("icon-smile", "bmdc-smile"));
	defaultStringTheme.insert(StringMap::value_type("icon-download", "bmdc-download"));
	defaultStringTheme.insert(StringMap::value_type("icon-favorite-hubs", "bmdc-favorite-hubs"));
	defaultStringTheme.insert(StringMap::value_type("icon-favorite-users", "bmdc-favorite-users"));
	defaultStringTheme.insert(StringMap::value_type("icon-finished-downloads", "bmdc-finished-downloads"));
	defaultStringTheme.insert(StringMap::value_type("icon-finished-uploads", "bmdc-finished-uploads"));
	defaultStringTheme.insert(StringMap::value_type("icon-hash", "bmdc-hash"));
	defaultStringTheme.insert(StringMap::value_type("icon-preferences", "bmdc-preferences"));
	defaultStringTheme.insert(StringMap::value_type("icon-public-hubs", "bmdc-public-hubs"));
	defaultStringTheme.insert(StringMap::value_type("icon-queue", "bmdc-queue"));
	defaultStringTheme.insert(StringMap::value_type("icon-search", "bmdc-search"));
	defaultStringTheme.insert(StringMap::value_type("icon-search-spy", "bmdc-search-spy"));
	defaultStringTheme.insert(StringMap::value_type("icon-upload", "bmdc-upload"));
	defaultStringTheme.insert(StringMap::value_type("icon-quit", "bmdc-quit"));
	defaultStringTheme.insert(StringMap::value_type("icon-connect", "bmdc-connect"));
	/**/
	defaultStringTheme.insert(StringMap::value_type("icon-notepad", "bmdc-notepad"));
	defaultStringTheme.insert(StringMap::value_type("icon-system", "bmdc-system"));
	defaultStringTheme.insert(StringMap::value_type("icon-search-adl", "bmdc-search-adl"));
	defaultStringTheme.insert(StringMap::value_type("icon-search-adl-on", "bmdc-search-adl-on"));
	defaultStringTheme.insert(StringMap::value_type("icon-away", "bmdc-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-none", "bmdc-none"));
	defaultStringTheme.insert(StringMap::value_type("icon-limiting", "bmdc-limiting"));
	defaultStringTheme.insert(StringMap::value_type("icon-highlight", "bmdc-highlight"));
	/* icons of conn */
	defaultStringTheme.insert(StringMap::value_type("icon-op", "bmdc-op"));
	defaultStringTheme.insert(StringMap::value_type("icon-modem", "bmdc-modem"));
	defaultStringTheme.insert(StringMap::value_type("icon-wireless", "bmdc-wireless"));
	defaultStringTheme.insert(StringMap::value_type("icon-dsl", "bmdc-dsl"));
	defaultStringTheme.insert(StringMap::value_type("icon-lan", "bmdc-lan"));
	defaultStringTheme.insert(StringMap::value_type("icon-netlimiter", "bmdc-netlimiter"));
	/***/
	defaultStringTheme.insert(StringMap::value_type("icon-ten", "bmdc-ten"));
	defaultStringTheme.insert(StringMap::value_type("icon-zeroone", "bmdc-zeroone"));
	defaultStringTheme.insert(StringMap::value_type("icon-zerozeroone", "bmdc-zerozeroone"));
	defaultStringTheme.insert(StringMap::value_type("icon-other", "bmdc-other"));
	/* aways */
	defaultStringTheme.insert(StringMap::value_type("icon-op-away", "bmdc-op-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-modem-away", "bmdc-modem-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-wireless-away", "bmdc-wireless-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-dsl-away", "bmdc-dsl-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-lan-away", "bmdc-lan-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-netlimiter-away", "bmdc-netlimiter-away"));
	/***/
	defaultStringTheme.insert(StringMap::value_type("icon-ten-away", "bmdc-ten-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-zeroone-away", "bmdc-zeroone"));
	defaultStringTheme.insert(StringMap::value_type("icon-zerozeroone-away", "bmdc-zerozeroone-away"));
	defaultStringTheme.insert(StringMap::value_type("icon-other-away", "bmdc-other-away"));
	/* pasive */
	defaultStringTheme.insert(StringMap::value_type("icon-op-pasive", "bmdc-op-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-modem-pasive", "bmdc-modem-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-wireless-pasive", "bmdc-wireless-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-dsl-pasive", "bmdc-dsl-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-lan-pasive", "bmdc-lan-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-netlimiter-pasive", "bmdc-netlimiter-pasive"));
	/***/
	defaultStringTheme.insert(StringMap::value_type("icon-ten-pasive", "bmdc-ten-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-zeroone-pasive", "bmdc-zeroone-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-zerozeroone-pasive", "bmdc-zerozeroone-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-other-pasive", "bmdc-other-pasive"));
	/*aways*/
	defaultStringTheme.insert(StringMap::value_type("icon-op-away-pasive", "bmdc-op-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-modem-away-pasive", "bmdc-modem-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-wireless-away-pasive", "bmdc-wireless-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-dsl-away-pasive", "bmdc-dsl-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-lan-away-pasive", "bmdc-lan-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-netlimiter-away-pasive", "bmdc-netlimiter-away-pasive"));
	/***/
	defaultStringTheme.insert(StringMap::value_type("icon-ten-away-pasive", "bmdc-ten-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-zeroone-away-pasive", "bmdc-zeroone-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-zerozeroone-away-pasive", "bmdc-zerozeroone-away-pasive"));
	defaultStringTheme.insert(StringMap::value_type("icon-other-away-pasive", "bmdc-other-away-pasive"));
	/**/
	defaultStringTheme.insert(StringMap::value_type("icon-file", BMDC_STOCK_FILE));
	defaultStringTheme.insert(StringMap::value_type("icon-directory", BMDC_STOCK_DIRECTORY));
	defaultStringTheme.insert(StringMap::value_type("text-general-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-general-fore-color", "#4D4D4D"));
	defaultStringTheme.insert(StringMap::value_type("text-myown-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-myown-fore-color", "#207505"));
	defaultStringTheme.insert(StringMap::value_type("text-private-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-private-fore-color", "#2763CE"));
	defaultStringTheme.insert(StringMap::value_type("text-system-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-system-fore-color", "#1A1A1A"));
	defaultStringTheme.insert(StringMap::value_type("text-status-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-status-fore-color", "#7F7F7F"));
	defaultStringTheme.insert(StringMap::value_type("text-timestamp-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-timestamp-fore-color", "#43629A"));
	defaultStringTheme.insert(StringMap::value_type("text-mynick-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-mynick-fore-color", "#A52A2A"));
	defaultStringTheme.insert(StringMap::value_type("text-fav-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-fav-fore-color", "#FFA500"));
	defaultStringTheme.insert(StringMap::value_type("text-op-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-op-fore-color", "#0000FF"));
	defaultStringTheme.insert(StringMap::value_type("text-url-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-url-fore-color", "#0000FF"));

	defaultStringTheme.insert(StringMap::value_type("text-ip-fore-color", "#000000"));
	defaultStringTheme.insert(StringMap::value_type("text-ip-back-color", "#FFFFFF"));
	defaultStringTheme.insert(StringMap::value_type("text-cheat-fore-color", "#DE1515"));
	defaultStringTheme.insert(StringMap::value_type("text-cheat-back-color", "#EEE7E7"));
	defaultStringTheme.insert(StringMap::value_type("text-high-fore-color", "black"));
	defaultStringTheme.insert(StringMap::value_type("text-high-back-color", "white"));

	/*for UL color text&bg*///NOTE BMDC++
	defaultStringTheme.insert(StringMap::value_type("userlist-text-operator", "#1E90FF"));
	defaultStringTheme.insert(StringMap::value_type("userlist-text-pasive", "#747677"));
	defaultStringTheme.insert(StringMap::value_type("userlist-text-protected", "#8B6914"));
	defaultStringTheme.insert(StringMap::value_type("userlist-text-favorite", "#ff0000"));
	defaultStringTheme.insert(StringMap::value_type("userlist-text-ignored", "#9affaf"));
	defaultStringTheme.insert(StringMap::value_type("userlist-text-normal", "#000000"));
	//.bg
	defaultStringTheme.insert(StringMap::value_type("userlist-bg-operator", "#1E90FF"));
	defaultStringTheme.insert(StringMap::value_type("userlist-bg-bot-hub", "#000000"));
	defaultStringTheme.insert(StringMap::value_type("userlist-bg-favorite", "#00FF00"));
	defaultStringTheme.insert(StringMap::value_type("userlist-bg-normal", "#BFBFBF"));
	//NOTE: END
	defaultIntTheme.insert(IntMap::value_type("text-general-bold", 0));
	defaultIntTheme.insert(IntMap::value_type("text-general-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-myown-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-myown-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-private-bold", 0));
	defaultIntTheme.insert(IntMap::value_type("text-private-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-system-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-system-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-status-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-status-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-timestamp-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-timestamp-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-mynick-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-mynick-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-fav-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-fav-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-op-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-op-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-url-bold", 0));
	defaultIntTheme.insert(IntMap::value_type("text-url-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-ip-bold", 0));
	defaultIntTheme.insert(IntMap::value_type("text-ip-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-cheat-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-cheat-italic", 0));
	defaultIntTheme.insert(IntMap::value_type("text-high-bold", 1));
	defaultIntTheme.insert(IntMap::value_type("text-high-italic", 0));
	//Tabs
	defaultStringTheme.insert(StringMap::value_type("color-tab-text-bold", "blue"));
	defaultStringTheme.insert(StringMap::value_type("color-tab-text-urgent", "blue"));
	defaultIntTheme.insert(IntMap::value_type("colorize-tab-text",1));

	//For Highlighting...
	isSensitiveHG[0] = isSensitiveHG[1] = isSensitiveHG[2] = isSensitiveHG[3] = FALSE;
	// Initialize the tabs in the GtkNotebook.
	initPersonal_gui();
	/*initConnection_gui();
	initDownloads_gui();
	initSharing_gui();
	initAppearance_gui();
	initLog_gui();
	initAdvanced_gui();
	initBandwidthLimiting_gui();
	initSearchTypes_gui();
	initHighlighting_gui();*/
}

Settings::~Settings()
{
	if (getResponseID() == GTK_RESPONSE_OK)
		saveSettings_client();
}

void Settings::response_gui()
{
	gtk_dialog_response(GTK_DIALOG(getContainer()), GTK_RESPONSE_CANCEL);

	gtk_dialog_response(GTK_DIALOG(getWidget("publicHubsDialog")), GTK_RESPONSE_CANCEL);
	gtk_dialog_response(GTK_DIALOG(getWidget("nameDialog")), GTK_RESPONSE_CANCEL); 
	gtk_dialog_response(GTK_DIALOG(getWidget("dirChooserDialog")), GTK_RESPONSE_CANCEL);
	gtk_dialog_response(GTK_DIALOG(getWidget("fileChooserDialog")), GTK_RESPONSE_CANCEL);
	gtk_dialog_response(GTK_DIALOG(getWidget("commandDialog")), GTK_RESPONSE_CANCEL);
	gtk_dialog_response(GTK_DIALOG(getWidget("fontSelectionDialog")), GTK_RESPONSE_CANCEL);
	gtk_dialog_response(GTK_DIALOG(getWidget("colorSelectionDialog")), GTK_RESPONSE_CANCEL);
	gtk_dialog_response(GTK_DIALOG(getWidget("ExtensionsDialog")), GTK_RESPONSE_CANCEL); 

}

void Settings::saveSettings_client()
{
	SettingsManager *sm = SettingsManager::getInstance();
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	string path;

	{ // Personal
		sm->set(SettingsManager::NICK, gtk_editable_get_text(GTK_EDITABLE(getWidget("nickEntry"))));
		sm->set(SettingsManager::EMAIL, gtk_editable_get_text(GTK_EDITABLE(getWidget("emailEntry"))));
		sm->set(SettingsManager::DESCRIPTION, gtk_editable_get_text(GTK_EDITABLE(getWidget("descriptionEntry"))));
		//sm->set(SettingsManager::UPLOAD_SPEED, SettingsManager::connectionSpeeds[gtk_combo_box_get_active(GTK_COMBO_BOX(connectionSpeedComboBox))]);

		//g_autofree gchar *encoding = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxCharset")));
		//if(encoding) {
		//	WSET("default-charset", string(encoding));
		//}
	}

	{ // Connection
		// Incoming connection
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("activeRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("upnpRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_UPNP);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("portForwardRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_NAT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("passiveRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);

		string ipv4 = gtk_editable_get_text(GTK_EDITABLE(getWidget("entryIpExt")));
		if(ipv4.length() >= 7)
			sm->set(SettingsManager::EXTERNAL_IP, ipv4);
		
		sm->set(SettingsManager::EXTERNAL_IP6, gtk_editable_get_text(GTK_EDITABLE(getWidget("entryipv6"))));

		sm->set(SettingsManager::NO_IP_OVERRIDE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("forceIPCheckButton"))));

		uint16_t port = Util::toInt(gtk_editable_get_text(GTK_EDITABLE(getWidget("tcpEntry"))));
		if (port > 0)
			sm->set(SettingsManager::TCP_PORT, port);
		port = Util::toInt(gtk_editable_get_text(GTK_EDITABLE(getWidget("udpEntry"))));
		if (port > 0)
			sm->set(SettingsManager::UDP_PORT, port);
		port = Util::toInt(gtk_editable_get_text(GTK_EDITABLE(getWidget("tlsEntry"))));
		if (port > 0)
			sm->set(SettingsManager::TLS_PORT, port);

		// Outgoing connection
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("outDirectRadioButton"))))
			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_DIRECT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("socksRadioButton"))))
			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_SOCKS5);

		sm->set(SettingsManager::SOCKS_SERVER, gtk_editable_get_text(GTK_EDITABLE(getWidget("socksIPEntry"))));
		sm->set(SettingsManager::SOCKS_USER, gtk_editable_get_text(GTK_EDITABLE(getWidget("socksUserEntry"))));
		sm->set(SettingsManager::SOCKS_PASSWORD, gtk_editable_get_text(GTK_EDITABLE(getWidget("socksPassEntry"))));
		//sm->set(SettingsManager::SOCKS_RESOLVE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("socksCheckButton"))));

		port = Util::toInt(gtk_editable_get_text(GTK_EDITABLE(getWidget("socksPortEntry"))));
		if (port > 0)
			sm->set(SettingsManager::SOCKS_PORT, port);

		sm->set(SettingsManager::TIME_RECCON, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(getWidget("spinReconect"))));
	}

	{ // Downloads
		path = gtk_editable_get_text(GTK_EDITABLE (getWidget("finishedDownloadsEntry")));
		if (path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::DOWNLOAD_DIRECTORY, path);

		path = gtk_editable_get_text(GTK_EDITABLE(getWidget("unfinishedDownloadsEntry")));
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, path);
		sm->set(SettingsManager::DOWNLOAD_SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("maxDownloadsSpinButton"))));
		sm->set(SettingsManager::MAX_DOWNLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("newDownloadsSpinButton"))));
		sm->set(SettingsManager::HTTP_PROXY, gtk_editable_get_text(GTK_EDITABLE(getWidget("proxyEntry"))));

		{ // Queue
			// Auto-priority
			sm->set(SettingsManager::PRIO_HIGHEST_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("priorityHighestSpinButton")))));
			sm->set(SettingsManager::PRIO_HIGH_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("priorityHighSpinButton")))));
			sm->set(SettingsManager::PRIO_NORMAL_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("priorityNormalSpinButton")))));
			sm->set(SettingsManager::PRIO_LOW_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("priorityLowSpinButton")))));

			// Auto-drop
			sm->set(SettingsManager::AUTODROP_SPEED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("dropSpeedSpinButton")))));
			sm->set(SettingsManager::AUTODROP_ELAPSED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("dropElapsedSpinButton")))));
			sm->set(SettingsManager::AUTODROP_MINSOURCES, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("dropMinSourcesSpinButton")))));
			sm->set(SettingsManager::AUTODROP_INTERVAL, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("dropCheckSpinButton")))));
			sm->set(SettingsManager::AUTODROP_INACTIVITY, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("dropInactiveSpinButton")))));
			sm->set(SettingsManager::AUTODROP_FILESIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("dropSizeSpinButton")))));

			// Other queue options
			saveOptionsView_gui(queueView, sm);
		}
	}

	{ // Sharing
		sm->set(SettingsManager::FOLLOW_LINKS, (int)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("followLinksCheckButton"))));
		sm->set(SettingsManager::MIN_UPLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("sharedExtraSlotSpinButton"))));
		sm->set(SettingsManager::SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("sharedUploadSlotsSpinButton"))));
		sm->set(SettingsManager::SLOTS_PRIMARY, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("sharedUploadSlotsSpinButton"))));
	}

	{ // Appearance
		saveOptionsView_gui(appearanceView, sm);

		WSET("tab-position", gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("tabPositionComboBox"))));
		WSET("toolbar-style", gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("toolbarStyleComboBox"))));

		sm->set(SettingsManager::DEFAULT_AWAY_MESSAGE, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("awayMessageEntry")))));
		sm->set(SettingsManager::TIME_STAMPS_FORMAT, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("timestampEntry")))));
		sm->set(SettingsManager::COUNTRY_FORMAT, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("entryCountry")))));

		{ // Tabs
			saveOptionsView_gui(tabView, sm);
		}

		{ // Sounds
			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(soundStore);
			gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				wsm->set(soundView.getString(&iter, "keyUse"), soundView.getValue<int>(&iter, _("Use")));
				wsm->set(soundView.getString(&iter, "keyFile"), soundView.getString(&iter, _("File")));

				valid = gtk_tree_model_iter_next(m, &iter);
			}

			WSET("sound-pm",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("soundPMReceivedCheckButton"))));
			WSET("sound-pm-open", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("soundPMWindowCheckButton"))));
		}

		{ // Colors & Fonts

			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
			gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				wsm->set(textStyleView.getString(&iter, "keyForeColor"), textStyleView.getString(&iter, "ForeColor"));
				wsm->set(textStyleView.getString(&iter, "keyBackColor"), textStyleView.getString(&iter, "BackColor"));
				wsm->set(textStyleView.getString(&iter, "keyBolt"), textStyleView.getValue<int>(&iter, "Bolt"));
				wsm->set(textStyleView.getString(&iter, "keyItalic"), textStyleView.getValue<int>(&iter, "Italic"));

				valid = gtk_tree_model_iter_next(m, &iter);
			}

			WSET("text-bold-autors", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkBoldAuthors"))));
		}

		{ // Notify
			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(notifyStore);
			gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				wsm->set(notifyView.getString(&iter, "keyUse"), notifyView.getValue<int>(&iter, _("Use")));
				wsm->set(notifyView.getString(&iter, "keyTitle"), notifyView.getString(&iter, _("Title")));
				wsm->set(notifyView.getString(&iter, "keyIcon"), notifyView.getString(&iter, _("Icon")));

				valid = gtk_tree_model_iter_next(m, &iter);
			}

			WSET("notify-pm-length", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("notifyPMLengthSpinButton"))));
			WSET("notify-icon-size", gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("notifyIconSizeComboBox"))));
			WSET("notify-only-not-active", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("notifyAppActiveCheckButton"))));
		}

		{ // Theme
			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(themeIconsStore);
			gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				wsm->set(themeIconsView.getString(&iter, "keyIcon"), themeIconsView.getString(&iter, "iconName"));
				valid = gtk_tree_model_iter_next(m, &iter);
			}

			string theme = gtk_label_get_text(GTK_LABEL(getWidget("currentThemeLabel")));
			wsm->set("theme-name", theme);
		}

		{ // Toolbar
			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(toolbarStore);
			gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				wsm->set(toolbarView.getString(&iter, "keyUse"), toolbarView.getValue<int>(&iter, _("Use")));
				valid = gtk_tree_model_iter_next(m, &iter);
			}
		}

		{ // Search Spy
			GdkRGBA color;

			g_c_b_g(G_C_B(getWidget("aSPColorButton")), &color);
			WSET("search-spy-a-color", WulforUtil::colorToString(&color));

			g_c_b_g(G_C_B(getWidget("tSPColorButton")), &color);
			WSET("search-spy-t-color", WulforUtil::colorToString(&color));

			g_c_b_g(G_C_B(getWidget("qSPColorButton")), &color);
			WSET("search-spy-q-color", WulforUtil::colorToString(&color));

			g_c_b_g(G_C_B(getWidget("cSPColorButton")), &color);
			WSET("search-spy-c-color", WulforUtil::colorToString(&color));

			g_c_b_g(G_C_B(getWidget("rSPColorButton")), &color);
			WSET("search-spy-r-color", WulforUtil::colorToString(&color));

			WSET("search-spy-frame", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("frameSPSpinButton"))));
			WSET("search-spy-waiting", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("waitingSPSpinButton"))));
			WSET("search-spy-top", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("topSPSpinButton"))));
		}

		{ // Window
			// Auto-open on startup
			saveOptionsView_gui(windowView1, sm);

			// Window options
			saveOptionsView_gui(windowView2, sm);

			// Confirm dialog options
			saveOptionsView_gui(windowView3, sm);
		}

		{ // Colors UL

			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(userListStore1);
			gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				wsm->set(userListNames.getString(&iter, "Set"), userListNames.getString(&iter, "Color"));
				wsm->set(userListNames.getString(&iter, "Back"), userListNames.getString(&iter, "BackSet"));
				valid = gtk_tree_model_iter_next(m, &iter);
			}

		}
	}
	//Highlighting
	{
		HighlightManager::getInstance()->replaceList(pList);
	}

	{ // Logs
		path = gtk_editable_get_text(GTK_EDITABLE(getWidget("logDirectoryEntry")));
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::LOG_DIRECTORY, path);
		sm->set(SettingsManager::LOG_MAIN_CHAT, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logMainCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_MAIN_CHAT, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("logMainEntry")))));
		//sm->set(SettingsManager::LOG_PRIVATE_CHAT, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logPrivateCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_PRIVATE_CHAT, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("logPrivateEntry")))));
		//sm->set(SettingsManager::LOG_DOWNLOADS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logDownloadsCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_POST_DOWNLOAD, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("logDownloadsEntry")))));
		//sm->set(SettingsManager::LOG_UPLOADS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logUploadsCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_POST_UPLOAD, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("logUploadsEntry")))));
		//sm->set(SettingsManager::LOG_SYSTEM, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logSystemCheckButton"))));
		//sm->set(SettingsManager::LOG_STATUS_MESSAGES, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logStatusCheckButton"))));
		//sm->set(SettingsManager::LOG_FILELIST_TRANSFERS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("logFilelistTransfersCheckButton"))));
		//sm->set(SettingsManager::LOG_RAW_CMD, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkraws"))));
		sm->set(SettingsManager::LOG_FORMAT_RAW, gtk_editable_get_text(GTK_EDITABLE(getWidget("entryraws"))));
	}

	{ // Advanced
		saveOptionsView_gui(advancedView, sm);
/*
		// Expert
		sm->set(SettingsManager::MAX_HASH_SPEED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("hashSpeedSpinButton")))));
		sm->set(SettingsManager::BUFFER_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("writeBufferSpinButton")))));
		sm->set(SettingsManager::PM_LAST_LOG_LINES,
			Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("pmHistorySpinButton")))));//NOTE: core 0.762
		sm->set(SettingsManager::SET_MINISLOT_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("slotSizeSpinButton")))));
		sm->set(SettingsManager::MAX_FILELIST_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("maxListSizeSpinButton")))));
		sm->set(SettingsManager::PRIVATE_ID, string(gtk_entry_get_text(GTK_ENTRY(getWidget("CIDEntry")))));
		sm->set(SettingsManager::AUTO_REFRESH_TIME, Util::toString (gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("autoRefreshSpinButton")))));
		sm->set(SettingsManager::SEARCH_HISTORY, Util::toString (gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("searchHistorySpinButton")))));
		sm->set(SettingsManager::BIND_ADDRESS, string(gtk_entry_get_text(GTK_ENTRY(getWidget("bindAddressEntry")))));
		sm->set(SettingsManager::BIND_ADDRESS6, string(gtk_entry_get_text(GTK_ENTRY(getWidget("bind6AddressEntry")))));
		sm->set(SettingsManager::SOCKET_IN_BUFFER, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("socketReadSpinButton")))));
		sm->set(SettingsManager::SOCKET_OUT_BUFFER, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("socketWriteSpinButton")))));*/
		// Security Certificates
		path = gtk_editable_get_text(GTK_EDITABLE(getWidget("trustedCertificatesPathEntry")));
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::TLS_PRIVATE_KEY_FILE, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("privateKeyEntry")))));
		sm->set(SettingsManager::TLS_CERTIFICATE_FILE, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("certificateFileEntry")))));
		sm->set(SettingsManager::TLS_TRUSTED_CERTIFICATES_PATH, path);

		saveOptionsView_gui(certificatesView, sm);
#ifdef HAVE_LIBTAR
//		sm->set(SettingsManager::ENABLE_AUTOBACKUP,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("enableBackup"))));
		sm->set(SettingsManager::BACKUP_TIMESTAMP,string(gtk_editable_get_text(GTK_EDITABLE(getWidget("backupTimestampEntry")))));
		sm->set(SettingsManager::BACKUP_FILE_PATTERN, string(gtk_editable_get_text(GTK_EDITABLE(getWidget("backupPatternEntry")))));
//		sm->set(SettingsManager::AUTOBACKUP_TIME, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("backupSpin"))));
#endif
	}

	{
		// Transfer Rate Limiting
		sm->set(SettingsManager::MAX_UPLOAD_SPEED_MAIN,
			(int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("transferMaxUpload"))));
		sm->set(SettingsManager::MAX_DOWNLOAD_SPEED_MAIN,
			(int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("transferMaxDownload"))));

		// Secondary Transfer Rate Limiting
		bool secondary = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("useLimitSecondCheckButton")));
		sm->set(SettingsManager::TIME_DEPENDENT_THROTTLE, secondary);

		if (secondary)
		{
			// Secondary Transfer Rate Settings
			sm->set(SettingsManager::MAX_UPLOAD_SPEED_ALTERNATE,
				(int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("secondaryMaxUpload"))));
			sm->set(SettingsManager::MAX_DOWNLOAD_SPEED_ALTERNATE,
				(int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("secondaryMaxDownload"))));
			sm->set(SettingsManager::SLOTS_ALTERNATE_LIMITING,
				(int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("secondaryUploadSlots"))));
			sm->set(SettingsManager::BANDWIDTH_LIMIT_START,
				gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("limitsFromCombobox"))));
			sm->set(SettingsManager::BANDWIDTH_LIMIT_END,
				gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("limitsToCombobox"))));
		}
	}

	sm->save();
	wsm->save();
}

/* Adds a core option */

void Settings::addOption_gui(GtkListStore *store, const string &name, SettingsManager::IntSetting setting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, SettingsManager::getInstance()->get(setting),
		1, name.c_str(),
		2, setting,
		3, "",
		-1);
}

void Settings::addOption_gui(GtkListStore *store, char *name, dcpp::SettingsManager::BoolSetting setting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, SettingsManager::getInstance()->get(setting),
		1, name,
		2, (int)setting,
		3, "",
		-1);

}

/* Adds a custom UI specific option */

void Settings::addOption_gui(GtkListStore *store, const string &name, const string &setting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, WGETI(setting),
		1, name.c_str(),
		2, -2,
		3, setting.c_str(),
		-1);
}
//NOTE BMDC//Adds Option UL
void Settings::addOption_gui(GtkListStore *store, const string &name, const string &setting, const string &backSetting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, name.c_str(),
		1, WGETS(setting).c_str(),
		2, backSetting.c_str(),
		3, setting.c_str(),
		4, WGETS(backSetting).c_str(),
		-1);
}
//END
/* Adds a sounds options */

void Settings::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const string &name, const string &key1, const string &key2)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, wsm->getInt(key1),               //use
		1, name.c_str(),                    //sounds
		2, wsm->getString(key2).c_str(),    //file
		3, key1.c_str(),                    //key use
		4, key2.c_str(),                    //key file
		-1);
}

/* Adds a notify options */

void Settings::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm,
	const string &name, const string &key1, const string &key2, const string &key3, const int key4)
{
	GdkPixbuf *icon = NULL;
	string pathIcon = wsm->getString(key3);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(Text::fromUtf8(pathIcon).c_str(), NULL);

	if (pixbuf != NULL)
	{
		icon = WulforUtil::scalePixbuf(pixbuf, ICON_SIZE, ICON_SIZE);
		g_object_unref(pixbuf);
	}
	else
		pathIcon = string();

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, wsm->getInt(key1),               //use
		1, name.c_str(),                    //notify
		2, wsm->getString(key2).c_str(),    //title
		3, pathIcon.c_str(),                //icon path
		4, key1.c_str(),                    //key use
		5, key2.c_str(),                    //key title
		6, key3.c_str(),                    //key icon
		7, icon,                            //icon
		8, key4,                            //urgency
		-1);

	if (icon != NULL)
		g_object_unref(icon);
}

/* Adds a theme options */

void Settings::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, GtkIconTheme *iconTheme,
	const string &name, const string &key1)
{
	string iconName = wsm->getString(key1);
//	GdkPixbuf *icon = gtk_icon_theme_load_icon(iconTheme, iconName.c_str(),
//		ICON_SIZE, GTK_ICON_LOOKUP_FORCE_SVG, NULL);

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, name.c_str(),
//		1, icon,
		2, iconName.c_str(),
		3, key1.c_str(),
		-1);

//	if (icon != NULL)
//		g_object_unref(icon);

	set(key1, iconName);
}

/* Adds a toolbar buttons options*/
void Settings::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, GtkIconTheme *iconTheme,
	const string &name, const string &key1, const string &key2)
{
	string iconName = wsm->getString(key2);
//	GdkPixbuf *icon = gtk_icon_theme_load_icon(iconTheme, iconName.c_str(),
//		ICON_SIZE_NORMAL, GTK_ICON_LOOKUP_FORCE_SVG, NULL);

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, wsm->getInt(key1),
		1, name.c_str(),
//		2, icon,
		3, key1.c_str(),
		-1);

	//if (icon != NULL)
	//	g_object_unref(icon);
}

/* Adds a colors and fonts options */

void Settings::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const string &name,
	const string &key1, const string &key2, const string &key3, const string &key4)
{
 	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, name.c_str(),
		1, wsm->getString(key1).c_str(),
		2, wsm->getString(key2).c_str(),
		3, wsm->getInt(key3),
		4, wsm->getInt(key4),
		5, key1.c_str(),
		6, key2.c_str(),
		7, key3.c_str(),
		8, key4.c_str(),
		-1);
}
//@tabs
void Settings::addOption_gui_tabs(GtkListStore *store , const string& type, const string& key, const string& icon)
{
	GtkTreeIter iter;
	gtk_list_store_append(store,&iter);
	gtk_list_store_set(store, &iter,
					0, type.c_str(),
					1, key.c_str(),
					2, icon.c_str(),
					-1);
}

/* Creates a generic checkbox-based options GtkTreeView */

void Settings::createOptionsView_gui(TreeView &treeView, GtkListStore *&store, const string &widgetName)
{
	// Create the view
	treeView.setView(GTK_TREE_VIEW(getWidget(widgetName.c_str())));
	treeView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	treeView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, -1);
	treeView.insertHiddenColumn("Core Setting", G_TYPE_INT);
	treeView.insertHiddenColumn("UI Setting", G_TYPE_STRING);
	treeView.finalize();

	// Create the store
	store = gtk_list_store_newv(treeView.getColCount(), treeView.getGTypes());
	gtk_tree_view_set_model(treeView.get(), GTK_TREE_MODEL(store));
	g_object_unref(store);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), treeView.col(_("Name")), GTK_SORT_ASCENDING);

	// Connect the signal handlers
	g_signal_connect(treeView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(onOptionsViewToggled_gui), (gpointer)store);
}

/* Saves the core or UI values stored in the options GtkTreeView */

void Settings::saveOptionsView_gui(TreeView &treeView, SettingsManager *sm)
{
	GtkTreeIter iter;
	GtkTreeModel *m = gtk_tree_view_get_model(treeView.get());
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		gboolean toggled = treeView.getValue<gboolean>(&iter, _("Use"));
		gint coreSetting = treeView.getValue<gint>(&iter, "Core Setting");

		// If core setting has been set to a valid value
		if (coreSetting >= 0)
		{
			sm->set((SettingsManager::BoolSetting)coreSetting, toggled);
		}
		else
		{
			string uiSetting = treeView.getString(&iter, "UI Setting");
			WSET(uiSetting, toggled);
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Settings::initPersonal_gui()
{
	gtk_editable_set_text(GTK_EDITABLE(getWidget("nickEntry")), SETTING(NICK).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("emailEntry")), SETTING(EMAIL).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("descriptionEntry")), SETTING(DESCRIPTION).c_str());
//	connectionSpeedComboBox = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
//	gtk_box_pack_start(GTK_BOX(getWidget("connectionBox")), GTK_WIDGET(connectionSpeedComboBox), FALSE, TRUE, 0);
//	gtk_widget_show_all(GTK_WIDGET(connectionSpeedComboBox));

/*	for (StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i)
	{
		gtk_combo_box_text_append_text(connectionSpeedComboBox, (*i).c_str());
		if (SETTING(UPLOAD_SPEED) == *i)
			gtk_combo_box_set_active(GTK_COMBO_BOX(connectionSpeedComboBox), i - SettingsManager::connectionSpeeds.begin());
	}

	// Fill charset drop-down list
	auto& charsets = WulforUtil::getCharsets();
	for (auto it = charsets.begin(); it != charsets.end(); ++it) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxCharset")), (*it).c_str());
		if(WGETS("default-charset") == *it)
			gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxCharset")), (it - charsets.begin()) );
	}*/
}

void Settings::initConnection_gui()
{
	// Incoming
	g_signal_connect(getWidget("activeRadioButton"), "toggled", G_CALLBACK(onInDirect_gui), (gpointer)this);
	g_signal_connect(getWidget("upnpRadioButton"), "toggled", G_CALLBACK(onInFW_UPnP_gui), (gpointer)this);
	g_signal_connect(getWidget("portForwardRadioButton"), "toggled", G_CALLBACK(onInFW_NAT_gui), (gpointer)this);
	g_signal_connect(getWidget("passiveRadioButton"), "toggled", G_CALLBACK(onInPassive_gui), (gpointer)this);
	gtk_editable_set_text(GTK_EDITABLE(getWidget("entryIpExt")), SETTING(EXTERNAL_IP).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("entryipv6")), SETTING(EXTERNAL_IP6).c_str());

	gtk_editable_set_text(GTK_EDITABLE(getWidget("tcpEntry")), Util::toString(SETTING(TCP_PORT)).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("udpEntry")), Util::toString(SETTING(UDP_PORT)).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("tlsEntry")), Util::toString(SETTING(TLS_PORT)).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("forceIPCheckButton")), SETTING(NO_IP_OVERRIDE));

	switch (SETTING(INCOMING_CONNECTIONS))
	{
		case SettingsManager::INCOMING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("activeRadioButton")), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_NAT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("portForwardRadioButton")), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_UPNP:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("upnpRadioButton")), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("passiveRadioButton")), TRUE);
			break;
	}

	// Outgoing
	g_signal_connect(getWidget("outDirectRadioButton"), "toggled", G_CALLBACK(onOutDirect_gui), (gpointer)this);
	g_signal_connect(getWidget("socksRadioButton"), "toggled", G_CALLBACK(onSocks5_gui), (gpointer)this);
	gtk_editable_set_text(GTK_EDITABLE(getWidget("socksIPEntry")), SETTING(SOCKS_SERVER).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("socksUserEntry")), SETTING(SOCKS_USER).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("socksPortEntry")), Util::toString(SETTING(SOCKS_PORT)).c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("socksPassEntry")), SETTING(SOCKS_PASSWORD).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("socksCheckButton")), SETTING(SOCKS_RESOLVE));

	switch (SETTING(OUTGOING_CONNECTIONS))
	{
		case SettingsManager::OUTGOING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("outDirectRadioButton")), TRUE);
			onOutDirect_gui(NULL, (gpointer)this);
			break;
		case SettingsManager::OUTGOING_SOCKS5:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("socksRadioButton")), TRUE);
			break;
	}

	g_signal_connect(getWidget("checkautodet"), "toggled", G_CALLBACK(onToggleAutoDetection), (gpointer)this);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkautodet")), SETTING(AUTO_DETECT_CONNECTION));

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinReconect")), SETTING(TIME_RECCON));

}

void Settings::onToggleAutoDetection(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	// apply immediately so that ConnectivityManager updates.
	SettingsManager::getInstance()->set(SettingsManager::AUTO_DETECT_CONNECTION, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) );
	ConnectivityManager::getInstance()->fire(ConnectivityManagerListener::SettingChanged());

	gtk_widget_set_sensitive(s->getWidget("tabledad19"),!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}

void Settings::initDownloads_gui()
{
	GtkTreeIter iter;

	{ // Downloads
		g_signal_connect(getWidget("finishedDownloadsButton"), "clicked", G_CALLBACK(onBrowseFinished_gui), (gpointer)this);
		g_signal_connect(getWidget("unfinishedDownloadsButton"), "clicked", G_CALLBACK(onBrowseUnfinished_gui), (gpointer)this);
		g_signal_connect(getWidget("publicHubsButton"), "clicked", G_CALLBACK(onPublicHubs_gui), (gpointer)this);
		g_signal_connect(getWidget("publicHubsDialogAddButton"), "clicked", G_CALLBACK(onPublicAdd_gui), (gpointer)this);
		g_signal_connect(getWidget("publicHubsDialogUpButton"), "clicked", G_CALLBACK(onPublicMoveUp_gui), (gpointer)this);
		g_signal_connect(getWidget("publicHubsDialogDownButton"), "clicked", G_CALLBACK(onPublicMoveDown_gui), (gpointer)this);
		g_signal_connect(getWidget("publicHubsDialogRemoveButton"), "clicked", G_CALLBACK(onPublicRemove_gui), (gpointer)this);

		gtk_editable_set_text(GTK_EDITABLE(getWidget("finishedDownloadsEntry")), SETTING(DOWNLOAD_DIRECTORY).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("unfinishedDownloadsEntry")), SETTING(TEMP_DOWNLOAD_DIRECTORY).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("maxDownloadsSpinButton")), (double)SETTING(DOWNLOAD_SLOTS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("newDownloadsSpinButton")), (double)SETTING(MAX_DOWNLOAD_SPEED));
		gtk_editable_set_text(GTK_EDITABLE(getWidget("proxyEntry")), SETTING(HTTP_PROXY).c_str());

		publicListView.setView(GTK_TREE_VIEW(getWidget("publicHubsDialogTreeView")));
		publicListView.insertColumn("List", G_TYPE_STRING, TreeView::EDIT_STRING, -1);
		publicListView.finalize();
		publicListStore = gtk_list_store_newv(publicListView.getColCount(), publicListView.getGTypes());
		gtk_tree_view_set_model(publicListView.get(), GTK_TREE_MODEL(publicListStore));
		g_object_unref(publicListStore);
		gtk_tree_view_set_headers_visible(publicListView.get(), FALSE);
		g_signal_connect(publicListView.getCellRenderOf("List"), "edited", G_CALLBACK(onPublicEdit_gui), (gpointer)this);
	}

	{ // Download to
		g_signal_connect(getWidget("favoriteAddButton"), "clicked", G_CALLBACK(onAddFavorite_gui), (gpointer)this);
		g_signal_connect(getWidget("favoriteRemoveButton"), "clicked", G_CALLBACK(onRemoveFavorite_gui), (gpointer)this);
		downloadToView.setView(GTK_TREE_VIEW(getWidget("favoriteTreeView")));
		downloadToView.insertColumn(_("Favorite Name"), G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.insertColumn(_("Directory"), G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.finalize();
		downloadToStore = gtk_list_store_newv(downloadToView.getColCount(), downloadToView.getGTypes());
		gtk_tree_view_set_model(downloadToView.get(), GTK_TREE_MODEL(downloadToStore));
		g_object_unref(downloadToStore);
//		g_signal_connect(downloadToView.get(), "button-release-event", G_CALLBACK(onFavoriteButtonReleased_gui), (gpointer)this);
		gtk_widget_set_sensitive(getWidget("favoriteRemoveButton"), FALSE);

		StringPairList directories = FavoriteManager::getInstance()->getFavoriteDirs();
		for (auto j = directories.begin(); j != directories.end(); ++j)
		{
			gtk_list_store_append(downloadToStore, &iter);
			gtk_list_store_set(downloadToStore, &iter,
				downloadToView.col(_("Favorite Name")), j->second.c_str(),
				downloadToView.col(_("Directory")), j->first.c_str(),
				-1);
		}
	}

	{ // Preview
		previewAppView.setView(GTK_TREE_VIEW(getWidget("previewTreeView")));
		previewAppView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, -1);
		previewAppView.insertColumn(_("Application"), G_TYPE_STRING, TreeView::STRING, -1);
		previewAppView.insertColumn(_("Extensions"), G_TYPE_STRING, TreeView::STRING, -1);
		previewAppView.finalize();

		g_signal_connect(getWidget("previewAddButton"), "clicked", G_CALLBACK(onPreviewAdd_gui), (gpointer)this);
		g_signal_connect(getWidget("previewRemoveButton"), "clicked", G_CALLBACK(onPreviewRemove_gui), (gpointer)this);
		g_signal_connect(getWidget("previewApplyButton"), "clicked", G_CALLBACK(onPreviewApply_gui), (gpointer)this);
//		g_signal_connect(getWidget("previewTreeView"), "key-release-event", G_CALLBACK(onPreviewKeyReleased_gui), (gpointer)this);
//		g_signal_connect(previewAppView.get(), "button-release-event", G_CALLBACK(onPreviewButtonReleased_gui), (gpointer)this);

		previewAppToStore = gtk_list_store_newv(previewAppView.getColCount(), previewAppView.getGTypes());
		gtk_tree_view_set_model(previewAppView.get(), GTK_TREE_MODEL(previewAppToStore));
		g_object_unref(previewAppToStore);

		gtk_widget_set_sensitive(getWidget("previewAddButton"), TRUE);
		gtk_widget_set_sensitive(getWidget("previewRemoveButton"), TRUE);
		gtk_widget_set_sensitive(getWidget("previewApplyButton"), TRUE);

		WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
		const PreviewApp::List &Apps = wsm->getPreviewApps();

		// add default applications players
		if (Apps.empty())
		{
			wsm->addPreviewApp(_("Xine player"), "xine --no-logo --session volume=50", "avi; mov; vob; mpg; mp3");
			wsm->addPreviewApp(_("Kaffeine player"), "kaffeine -p", "avi; mov; mpg; vob; mp3");
			wsm->addPreviewApp(_("Mplayer player"), "mplayer", "avi; mov; vob; mp3");
		}

		for (auto item = Apps.begin(); item != Apps.end(); ++item)
		{
			gtk_list_store_append(previewAppToStore, &iter);
			gtk_list_store_set(previewAppToStore, &iter,
				previewAppView.col(_("Name")), ((*item)->name).c_str(),
				previewAppView.col(_("Application")), ((*item)->app).c_str(),
				previewAppView.col(_("Extensions")), ((*item)->ext).c_str(),
				-1);
		}
	}

	{ // Queue
		// Auto-priority
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("priorityHighestSpinButton")), (double)SETTING(PRIO_HIGHEST_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("priorityHighSpinButton")), (double)SETTING(PRIO_HIGH_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("priorityNormalSpinButton")), (double)SETTING(PRIO_NORMAL_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("priorityLowSpinButton")), (double)SETTING(PRIO_LOW_SIZE));

		// Auto-drop
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("dropSpeedSpinButton")), (double)SETTING(AUTODROP_SPEED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("dropElapsedSpinButton")), (double)SETTING(AUTODROP_ELAPSED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("dropMinSourcesSpinButton")), (double)SETTING(AUTODROP_MINSOURCES));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("dropCheckSpinButton")), (double)SETTING(AUTODROP_INTERVAL));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("dropInactiveSpinButton")), (double)SETTING(AUTODROP_INACTIVITY));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("dropSizeSpinButton")), (double)SETTING(AUTODROP_FILESIZE));

		// Other queue options
		createOptionsView_gui(queueView, queueStore, "queueOtherTreeView");

		addOption_gui(queueStore, _("Set lowest priority for newly added files larger than low priority size"), SettingsManager::PRIO_LOWEST);
		addOption_gui(queueStore, _("Auto-drop slow sources for all queue items (except filelists)"), SettingsManager::AUTODROP_ALL);
		addOption_gui(queueStore, _("Remove slow filelists"), SettingsManager::AUTODROP_FILELISTS);
		addOption_gui(queueStore, _("Don't remove the source when auto-dropping, only disconnect"), SettingsManager::AUTODROP_DISCONNECT);
		addOption_gui(queueStore, _("Automatically search for alternative download locations"), SettingsManager::AUTO_SEARCH);
		addOption_gui(queueStore, _("Automatically match queue for auto search hits"), SettingsManager::AUTO_SEARCH_AUTO_MATCH);
		addOption_gui(queueStore, _("Skip zero-byte files"), SettingsManager::SKIP_ZERO_BYTE);
		addOption_gui(queueStore, _("Don't download files already in share"), SettingsManager::DONT_DL_ALREADY_SHARED);
		addOption_gui(queueStore, _("Don't download files already in the queue"), SettingsManager::DONT_DL_ALREADY_QUEUED);
	}
}

void Settings::initSharing_gui()
{
	g_signal_connect(getWidget("shareHiddenCheckButton"), "toggled", G_CALLBACK(onShareHiddenPressed_gui), (gpointer)this);
	g_signal_connect(getWidget("sharedAddButton"), "clicked", G_CALLBACK(onAddShare_gui), (gpointer)this);
	g_signal_connect(getWidget("sharedRemoveButton"), "clicked", G_CALLBACK(onRemoveShare_gui), (gpointer)this);
	g_signal_connect(getWidget("pictureButton"), "clicked", G_CALLBACK(onPictureShare_gui), (gpointer)this);

	shareView.setView(GTK_TREE_VIEW(getWidget("sharedTreeView")));
	shareView.insertColumn(_("Virtual Name"), G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn(_("Directory"), G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn(_("Size"), G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	shareView.finalize();
	shareStore = gtk_list_store_newv(shareView.getColCount(), shareView.getGTypes());
	gtk_tree_view_set_model(shareView.get(), GTK_TREE_MODEL(shareStore));
	g_object_unref(shareStore);
	shareView.setSortColumn_gui(_("Size"), "Real Size");
//	g_signal_connect(shareView.get(), "button-release-event", G_CALLBACK(onShareButtonReleased_gui), (gpointer)this);
	gtk_widget_set_sensitive(getWidget("sharedRemoveButton"), FALSE);

	updateShares_gui();

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("shareHiddenCheckButton")), SETTING(SHARE_HIDDEN));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("followLinksCheckButton")), SETTING(FOLLOW_LINKS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("sharedExtraSlotSpinButton")), (double)SETTING(MIN_UPLOAD_SPEED));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("sharedUploadSlotsSpinButton")), (int)SETTING(SLOTS_PRIMARY));
}

void Settings::initAppearance_gui()
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	{ // Appearance
		createOptionsView_gui(appearanceView, appearanceStore, "appearanceOptionsTreeView");

		addOption_gui(appearanceStore, _("Filter kick and NMDC debug messages"), SettingsManager::FILTER_MESSAGES);
		addOption_gui(appearanceStore, _("Show status icon"), SettingsManager::ALWAYS_TRAY);
		addOption_gui(appearanceStore, _("Show timestamps in chat by default"), SettingsManager::TIME_STAMPS);
		addOption_gui(appearanceStore, _("View status messages in main chat"), SettingsManager::STATUS_IN_CHAT);
		addOption_gui(appearanceStore, _("Show joins / parts in chat by default"), SettingsManager::SHOW_JOINS);
		addOption_gui(appearanceStore, _("Only show joins / parts for favorite users"), SettingsManager::FAV_SHOW_JOINS);
		addOption_gui(appearanceStore, _("Sort favorite users first"), SettingsManager::SORT_FAVUSERS_FIRST);
		addOption_gui(appearanceStore, _("Use OEM monospaced font for chat windows"), SettingsManager::USE_OEM_MONOFONT);
		addOption_gui(appearanceStore, _("Use magnet split"), "use-magnet-split");
		addOption_gui(appearanceStore, _("Use blinking status icon"), "status-icon-blink-use");
		addOption_gui(appearanceStore, _("Use emoticons"), SettingsManager::USE_EMOTS);//"emoticons-use");
		addOption_gui(appearanceStore, _("Do not close the program, hide in the system tray"), "main-window-no-close");
		addOption_gui(appearanceStore, _("Show Country in chat"), SettingsManager::GET_USER_COUNTRY);
		addOption_gui(appearanceStore, _("Show IP in chat"), SettingsManager::USE_IP);
		addOption_gui(appearanceStore, _("Show Free Slots in Desc"), SettingsManager::SHOW_FREE_SLOTS_DESC);
		addOption_gui(appearanceStore, _("Use Highlighting"), "use-highlighting");
		addOption_gui(appearanceStore, _("Show Close Icon in Tab"), "use-close-button");
		addOption_gui(appearanceStore, _("Show send /commands in status message"), "show-commands");
		addOption_gui(appearanceStore, _("Show Country Flags in main chat"), SettingsManager::USE_COUNTRY_FLAG);
		addOption_gui(appearanceStore, _("Use DNS in Transfers"), "use-dns");
		addOption_gui(appearanceStore, _("Log Ignored Messages as STATUS messages"), SettingsManager::LOG_CHAT_B);
		addOption_gui(appearanceStore, _("Log Mainchat by default"), SettingsManager::LOG_MAIN_CHAT);
		addOption_gui(appearanceStore, _("Do not close Tab on middle button (wheel)"), "book-three-button-disable");
		addOption_gui(appearanceStore, _("Use ctrl for history in chat Books"), "key-hub-with-ctrl");
		addOption_gui(appearanceStore, _("Show Server Commands as Status Messages in Chat"),SettingsManager::SERVER_COMMANDS);
		addOption_gui(appearanceStore, _("Filter download by Antivirus db"),SettingsManager::USE_AV_FILTER);

		/// @todo: Uncomment when implemented
		//addOption_gui(appearanceStore, _("Use system icons"), SettingsManager::USE_SYSTEM_ICONS);

		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("tabPositionComboBox")), WGETI("tab-position"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("toolbarStyleComboBox")), WGETI("toolbar-style"));

		gtk_editable_set_text(GTK_EDITABLE(getWidget("awayMessageEntry")), SETTING(DEFAULT_AWAY_MESSAGE).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("timestampEntry")), SETTING(TIME_STAMPS_FORMAT).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("entryCountry")), SETTING(COUNTRY_FORMAT).c_str());
	}

	{ // Tabs
		createOptionsView_gui(tabView, tabStore, "tabBoldingTreeView");

		addOption_gui(tabStore, _("Finished Downloads"), SettingsManager::BOLD_FINISHED_DOWNLOADS);
		addOption_gui(tabStore, _("Finished Uploads"), SettingsManager::BOLD_FINISHED_UPLOADS);
		addOption_gui(tabStore, _("Download Queue"), SettingsManager::BOLD_QUEUE);
		addOption_gui(tabStore, _("Hub (also sets urgency hint)"), SettingsManager::BOLD_HUB);
		addOption_gui(tabStore, _("Private Message (also sets urgency hint)"), SettingsManager::BOLD_PM);
		addOption_gui(tabStore, _("Search"), SettingsManager::BOLD_SEARCH);
		addOption_gui(tabStore, _("Search Spy"), SettingsManager::BOLD_SEARCH_SPY);
		addOption_gui(tabStore, _("Set Hub to Bold everytime the content changes"), "bold-all-tab");
		//@tabs Colors
		tabsColors.setView(GTK_TREE_VIEW(getWidget("tabsColors")));
		tabsColors.insertColumn("Type", G_TYPE_STRING, TreeView::ICON_STRING,30,"Icon");
		tabsColors.insertHiddenColumn("key", G_TYPE_STRING);
		tabsColors.insertHiddenColumn("Icon", G_TYPE_STRING);
		tabsColors.finalize();

		tabColorStore = gtk_list_store_newv(tabsColors.getColCount(), tabsColors.getGTypes());
		gtk_tree_view_set_model(tabsColors.get(), GTK_TREE_MODEL(tabColorStore));
		g_object_unref(tabColorStore);
		//
		addOption_gui_tabs(tabColorStore,"Hub","hub",WGETS("icon-hub-online"));
		addOption_gui_tabs(tabColorStore,"Private Messages","pm",WGETS("icon-pm-online"));
		addOption_gui_tabs(tabColorStore,"Finished Uploads","uploads",WGETS("icon-finished-uploads"));
		addOption_gui_tabs(tabColorStore,"Finished Downloads","downloads",WGETS("icon-finished-downloads"));
		addOption_gui_tabs(tabColorStore,"Download Quene","downloads-quene",WGETS("icon-queue"));
		addOption_gui_tabs(tabColorStore,"Searchs","searchs",WGETS("icon-search"));
		addOption_gui_tabs(tabColorStore,"Search Spy","spy",WGETS("icon-search-spy"));
		addOption_gui_tabs(tabColorStore,"Search ADL","adl",WGETS("icon-search-adl"));
		addOption_gui_tabs(tabColorStore,"Share Browser","shareb",WGETS("icon-directory"));
		addOption_gui_tabs(tabColorStore,"Notepad","notepad",WGETS("icon-notepad"));
		addOption_gui_tabs(tabColorStore,"System Log","system",WGETS("icon-system"));
		addOption_gui_tabs(tabColorStore,"Public Hubs","public",WGETS("icon-public-hubs"));
		addOption_gui_tabs(tabColorStore,"Favorite Users","fav-users",WGETS("icon-favorite-users"));
		addOption_gui_tabs(tabColorStore,"Favorite Hubs","fav-hubs",WGETS("icon-favorite-hubs"));


		tabSelections = gtk_tree_view_get_selection (GTK_TREE_VIEW (tabsColors.get()));
		gtk_tree_selection_set_mode (tabSelections, GTK_SELECTION_SINGLE);
		g_signal_connect(G_OBJECT(tabSelections),"changed",G_CALLBACK(onChangeTabSelections), (gpointer)this);

	}

	{ // Sounds
		g_signal_connect(getWidget("soundPlayButton"), "clicked", G_CALLBACK(onSoundPlayButton_gui), (gpointer)this);
		g_signal_connect(getWidget("soundFileBrowseButton"), "clicked", G_CALLBACK(onSoundFileBrowseClicked_gui), (gpointer)this);

		soundView.setView(GTK_TREE_VIEW(getWidget("soundsTreeView")));
		soundView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		soundView.insertColumn(_("Sounds"), G_TYPE_STRING, TreeView::STRING, -1);
		soundView.insertColumn(_("File"), G_TYPE_STRING, TreeView::STRING, -1);
		soundView.insertHiddenColumn("keyUse", G_TYPE_STRING);
		soundView.insertHiddenColumn("keyFile", G_TYPE_STRING);
		soundView.finalize();

		soundStore = gtk_list_store_newv(soundView.getColCount(), soundView.getGTypes());
		gtk_tree_view_set_model(soundView.get(), GTK_TREE_MODEL(soundStore));
		g_object_unref(soundStore);

		g_signal_connect(soundView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(onOptionsViewToggled_gui), (gpointer)soundStore);

		addOption_gui(soundStore, wsm, _("Download begins"), "sound-download-begins-use", "sound-download-begins");
		addOption_gui(soundStore, wsm, _("Download finished"), "sound-download-finished-use", "sound-download-finished");
		addOption_gui(soundStore, wsm, _("Download finished file list"), "sound-download-finished-ul-use", "sound-download-finished-ul");
		addOption_gui(soundStore, wsm, _("Upload finished"), "sound-upload-finished-use", "sound-upload-finished");
		addOption_gui(soundStore, wsm, _("Private message"), "sound-private-message-use", "sound-private-message");
		addOption_gui(soundStore, wsm, _("Hub connected"), "sound-hub-connect-use", "sound-hub-connect");
		addOption_gui(soundStore, wsm, _("Hub disconnected"), "sound-hub-disconnect-use", "sound-hub-disconnect");
		addOption_gui(soundStore, wsm, _("Favorite user joined"), "sound-fuser-join-use", "sound-fuser-join");
		addOption_gui(soundStore, wsm, _("Favorite user quit"), "sound-fuser-quit-use", "sound-fuser-quit");

		gtk_widget_set_sensitive(getWidget("soundPlayButton"), TRUE);
		gtk_widget_set_sensitive(getWidget("soundFileBrowseButton"), TRUE);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("soundPMReceivedCheckButton")), WGETB("sound-pm"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("soundPMWindowCheckButton")), WGETB("sound-pm-open"));
	}

	{ // Colors & Fonts
		g_signal_connect(getWidget("textColorForeButton"), "clicked", G_CALLBACK(onTextColorForeClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("textColorBackButton"), "clicked", G_CALLBACK(onTextColorBackClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("textColorBWButton"), "clicked", G_CALLBACK(onTextColorBWClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("textStyleButton"), "clicked", G_CALLBACK(onTextStyleClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("textStylesDefaultButton"), "clicked", G_CALLBACK(onTextStyleDefaultClicked_gui), (gpointer)this);

		textStyleView.setView(GTK_TREE_VIEW(getWidget("treeViewAvailableStyles")));
		textStyleView.insertColumn(_("Style"), G_TYPE_STRING, TreeView::STRING, -1);
		textStyleView.insertHiddenColumn("ForeColor", G_TYPE_STRING);
		textStyleView.insertHiddenColumn("BackColor", G_TYPE_STRING);
		textStyleView.insertHiddenColumn("Bolt", G_TYPE_INT);
		textStyleView.insertHiddenColumn("Italic", G_TYPE_INT);
		textStyleView.insertHiddenColumn("keyForeColor", G_TYPE_STRING);
		textStyleView.insertHiddenColumn("keyBackColor", G_TYPE_STRING);
		textStyleView.insertHiddenColumn("keyBolt", G_TYPE_STRING);
		textStyleView.insertHiddenColumn("keyItalic", G_TYPE_STRING);
		textStyleView.finalize();

		textStyleStore = gtk_list_store_newv(textStyleView.getColCount(), textStyleView.getGTypes());
		gtk_tree_view_set_model(textStyleView.get(), GTK_TREE_MODEL(textStyleStore));
		g_object_unref(textStyleStore);

		// Available styles
		addOption_gui(textStyleStore, wsm, _("General text"),
			"text-general-fore-color", "text-general-back-color", "text-general-bold", "text-general-italic");

		addOption_gui(textStyleStore, wsm, _("My nick"),
			"text-mynick-fore-color", "text-mynick-back-color", "text-mynick-bold", "text-mynick-italic");

		addOption_gui(textStyleStore, wsm, _("My own message"),
			"text-myown-fore-color", "text-myown-back-color", "text-myown-bold", "text-myown-italic");

		addOption_gui(textStyleStore, wsm, _("Private message"),
			"text-private-fore-color", "text-private-back-color", "text-private-bold", "text-private-italic");

		addOption_gui(textStyleStore, wsm, _("System message"),
			"text-system-fore-color", "text-system-back-color", "text-system-bold", "text-system-italic");

		addOption_gui(textStyleStore, wsm, _("Status message"),
			"text-status-fore-color", "text-status-back-color", "text-status-bold", "text-status-italic");
		//Cheat
		addOption_gui(textStyleStore, wsm, _("Cheat message"),
			"text-cheat-fore-color", "text-cheat-back-color", "text-cheat-bold", "text-cheat-italic");

		addOption_gui(textStyleStore, wsm, _("Timestamp"),
			"text-timestamp-fore-color", "text-timestamp-back-color", "text-timestamp-bold", "text-timestamp-italic");

		addOption_gui(textStyleStore, wsm, _("URL"),
			"text-url-fore-color", "text-url-back-color", "text-url-bold", "text-url-italic");
		//IP adr.
		addOption_gui(textStyleStore, wsm, _("IP address"),
			"text-ip-fore-color", "text-ip-back-color", "text-ip-bold", "text-ip-italic");

		addOption_gui(textStyleStore, wsm, _("Favorite User"),
			"text-fav-fore-color", "text-fav-back-color", "text-fav-bold", "text-fav-italic");

		addOption_gui(textStyleStore, wsm, _("Operator"),
			"text-op-fore-color", "text-op-back-color", "text-op-bold", "text-op-italic");

		// Preview style
		GtkTreeIter treeIter;
		GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &treeIter);

		GtkTextIter textIter, textStartIter, textEndIter;
		string line, style;

		string timestamp = "[" + Util::getShortTimeString() + "] ";
		const gint ts_strlen = g_utf8_strlen(timestamp.c_str(), -1);
		string fore = wsm->getString("text-timestamp-fore-color");
		string back = wsm->getString("text-timestamp-back-color");
		int bold = wsm->getInt("text-timestamp-bold");
		int italic = wsm->getInt("text-timestamp-italic");

		GtkTextTag *tag = NULL;
		GtkTextTag *tagTimeStamp = gtk_text_buffer_create_tag(textStyleBuffer, _("Timestamp"),
			"background", back.c_str(),
			"foreground", fore.c_str(),
			"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
			"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
			NULL);

		gint row_count = 0;

		while (valid)
		{
			style = textStyleView.getString(&treeIter, _("Style"));
			fore = wsm->getString(textStyleView.getString(&treeIter, "keyForeColor"));
			back = wsm->getString(textStyleView.getString(&treeIter, "keyBackColor"));
			bold = wsm->getInt(textStyleView.getString(&treeIter, "keyBolt"));
			italic = wsm->getInt(textStyleView.getString(&treeIter, "keyItalic"));
			const gint st_strlen = g_utf8_strlen(style.c_str(), -1);

			line = timestamp + style + "\n";

			gtk_text_buffer_get_end_iter(textStyleBuffer, &textIter);
			gtk_text_buffer_insert(textStyleBuffer, &textIter, line.c_str(), line.size());

			textStartIter = textEndIter = textIter;

			// apply text style
			gtk_text_iter_backward_chars(&textStartIter, st_strlen + 1);

			if (row_count != 7)
				tag = gtk_text_buffer_create_tag(textStyleBuffer, style.c_str(),
					"background", back.c_str(),
					"foreground", fore.c_str(),
					"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					NULL);
			else
				tag = tagTimeStamp;

			gtk_text_buffer_apply_tag(textStyleBuffer, tag, &textStartIter, &textEndIter);

			// apply timestamp style
			gtk_text_iter_backward_chars(&textStartIter, ts_strlen);
			gtk_text_iter_backward_chars(&textEndIter, st_strlen + 2);
			gtk_text_buffer_apply_tag(textStyleBuffer, tagTimeStamp, &textStartIter, &textEndIter);

			row_count++;

			valid = gtk_tree_model_iter_next(m, &treeIter);
		}

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkBoldAuthors")), WGETB("text-bold-autors"));
		//[BMDC
/*		string strcolor = SETTING(BACKGROUND_CHAT_COLOR);

		gtk_widget_set_name(getWidget("textViewPreviewStyles"),"prewienTextView");
		GtkCssProvider *provider = gtk_css_provider_new ();
		GtkStyleContext *csc = gtk_widget_get_style_context(getWidget("textViewPreviewStyles")); 
		std::string t_css = std::string("#prewienTextView text { background: "+strcolor+" ;}\n\0");
//		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);

		gtk_style_context_add_provider(csc,
								GTK_STYLE_PROVIDER(provider),
								GTK_STYLE_PROVIDER_PRIORITY_USER);
		g_object_unref (provider);

		g_signal_connect(getWidget("setBackGroundChatWin"), "clicked", G_CALLBACK(onSetBackGroundChat), (gpointer)this);
		//]*/
	}

	{ // Notify
		notifyView.setView(GTK_TREE_VIEW(getWidget("notifyTreeView")));
		notifyView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		notifyView.insertColumn(_("Notify"), G_TYPE_STRING, TreeView::PIXBUF_STRING, -1, "icon");
		notifyView.insertColumn(_("Title"), G_TYPE_STRING, TreeView::STRING, -1);
		notifyView.insertColumn(_("Icon"), G_TYPE_STRING, TreeView::STRING, -1);
		notifyView.insertHiddenColumn("keyUse", G_TYPE_STRING);
		notifyView.insertHiddenColumn("keyTitle", G_TYPE_STRING);
		notifyView.insertHiddenColumn("keyIcon", G_TYPE_STRING);
		notifyView.insertHiddenColumn("icon", GDK_TYPE_PIXBUF);
		notifyView.insertHiddenColumn("Urgency", G_TYPE_INT);
		notifyView.finalize();

		notifyStore = gtk_list_store_newv(notifyView.getColCount(), notifyView.getGTypes());
		gtk_tree_view_set_model(notifyView.get(), GTK_TREE_MODEL(notifyStore));
		g_object_unref(notifyStore);

		g_signal_connect(notifyView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(onOptionsViewToggled_gui), (gpointer)notifyStore);

		addOption_gui(notifyStore, wsm, _("Download finished"),
			"notify-download-finished-use", "notify-download-finished-title",
			"notify-download-finished-icon", G_NOTIFICATION_PRIORITY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Download finished file list"),
			"notify-download-finished-ul-use", "notify-download-finished-ul-title",
			"notify-download-finished-ul-icon", G_NOTIFICATION_PRIORITY_LOW);

		addOption_gui(notifyStore, wsm, _("Private message"),
			"notify-private-message-use", "notify-private-message-title",
			"notify-private-message-icon", G_NOTIFICATION_PRIORITY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Hub connected"),
			"notify-hub-connect-use", "notify-hub-connect-title",
			"notify-hub-connect-icon", G_NOTIFICATION_PRIORITY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Hub disconnected"),
			"notify-hub-disconnect-use", "notify-hub-disconnect-title",
			"notify-hub-disconnect-icon", G_NOTIFICATION_PRIORITY_HIGH);

		addOption_gui(notifyStore, wsm, _("Favorite user joined"),
			"notify-fuser-join", "notify-fuser-join-title",
			"notify-fuser-join-icon", G_NOTIFICATION_PRIORITY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Favorite user quit"),
			"notify-fuser-quit", "notify-fuser-quit-title",
			"notify-fuser-quit-icon", G_NOTIFICATION_PRIORITY_NORMAL);
		addOption_gui(notifyStore, wsm, _("Highlighting string"),
			"notify-high-use", "notify-high-title",
			"notify-high-icon", G_NOTIFICATION_PRIORITY_LOW);

		addOption_gui(notifyStore, wsm, _("Chat Hub message"),
			"notify-hub-chat-use", "notify-hub-chat-title",
			"notify-hub-chat-icon", G_NOTIFICATION_PRIORITY_NORMAL);

		g_signal_connect(getWidget("notifyTestButton"), "clicked", G_CALLBACK(onNotifyTestButton_gui), (gpointer)this);
		g_signal_connect(getWidget("notifyIconFileBrowseButton"), "clicked", G_CALLBACK(onNotifyIconFileBrowseClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("notifyOKButton"), "clicked", G_CALLBACK(onNotifyOKClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("notifyIconNoneButton"), "clicked", G_CALLBACK(onNotifyIconNoneButton_gui), (gpointer)this);
		g_signal_connect(getWidget("notifyDefaultButton"), "clicked", G_CALLBACK(onNotifyDefaultButton_gui), (gpointer)this);
		//g_signal_connect(getWidget("notifyTreeView"), "key-release-event", G_CALLBACK(onNotifyKeyReleased_gui), (gpointer)this);
		//g_signal_connect(notifyView.get(), "button-release-event", G_CALLBACK(onNotifyButtonReleased_gui), (gpointer)this);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("notifyPMLengthSpinButton")), (gdouble)WGETI("notify-pm-length"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("notifyIconSizeComboBox")), WGETI("notify-icon-size"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("notifyAppActiveCheckButton")), WGETI("notify-only-not-active"));
	}

	{ // Themes
		themeIconsView.setView(GTK_TREE_VIEW(getWidget("treeViewIconsTheme")));
		themeIconsView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::PIXBUF_STRING, -1, "icon");
		themeIconsView.insertHiddenColumn("icon", GDK_TYPE_PIXBUF);
		themeIconsView.insertHiddenColumn("iconName", G_TYPE_STRING);
		themeIconsView.insertHiddenColumn("keyIcon", G_TYPE_STRING);
		themeIconsView.finalize();

		themeIconsStore = gtk_list_store_newv(themeIconsView.getColCount(), themeIconsView.getGTypes());
		gtk_tree_view_set_model(themeIconsView.get(), GTK_TREE_MODEL(themeIconsStore));
		g_object_unref(themeIconsStore);

/*		GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
        /**/
  /*      addOption_gui(themeIconsStore, wsm, iconTheme, _("User operator "), "icon-op");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User operator (away) "), "icon-op-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User operator (passive) "), "icon-op-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User operator (passive & away) "), "icon-op-away-pasive");
        /* modem */
 /*       addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type modem "), "icon-modem");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type modem (away)"), "icon-modem-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type modem (passive)"), "icon-modem-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type modem (passive & away)"), "icon-modem-away-pasive");
        /* wireles */
 /*       addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type wireless "), "icon-wireless");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type wireless (away)"), "icon-wireless-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type wireless (passive)"), "icon-wireless-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type wireless (passive & away)"), "icon-wireless-away-pasive");
        /* dsl */
/*        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type dsl "), "icon-dsl");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type dsl (away)"), "icon-dsl-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type dsl (passive)"), "icon-dsl-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type dsl (passive & away)"), "icon-dsl-away-pasive");
        /* lan */
  /*      addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type LAN "), "icon-lan");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type LAN (away)"), "icon-lan-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type LAN (passive)"), "icon-lan-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type LAN (passive & away)"), "icon-lan-away-pasive");
        /* lan */
    /*    addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type Netlimiter "), "icon-netlimiter");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type Netlimiter (away)"), "icon-netlimiter-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type Netlimiter (passive)"), "icon-netlimiter-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type Netlimiter (passive & away)"), "icon-netlimiter-away-pasive");
        /* ten** */
     /*   addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 10 or more "), "icon-ten");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 10 or more (away)"), "icon-ten-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 10 or more (passive)"), "icon-ten-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 10 or more (passive & away)"), "icon-ten-away-pasive");
        /* zeroone (0.x) */
     /*   addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.x"), "icon-zeroone");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.x (away)"), "icon-zeroone-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.x (passive)"), "icon-zeroone-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.x (passive & away)"), "icon-zeroone-away-pasive");
        /* zeroone (0.x) */
     /*   addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.0x or low"), "icon-zerozeroone");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.0x or low (away)"), "icon-zerozeroone-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.0x or low (passive)"), "icon-zerozeroone-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with connection type 0.0x or low (passive & away)"), "icon-zerozeroone-away-pasive");
        /* zeroone (0.x) */
    /*    addOption_gui(themeIconsStore, wsm, iconTheme, _("User with other connection type or unknown"), "icon-other");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with other connection type or unknown (away)"), "icon-other-away");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with other connection type or unknown (passive)"), "icon-other-pasive");
        addOption_gui(themeIconsStore, wsm, iconTheme, _("User with other connection type or unknown (passive & away)"), "icon-other-away-pasive");
  		/**/
/*		addOption_gui(themeIconsStore, wsm, iconTheme, _("Button smile"), "icon-smile");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Download"), "icon-download");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Upload"), "icon-upload");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Favorite Hubs"), "icon-favorite-hubs");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Favorite Users"), "icon-favorite-users");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Finished Downloads"), "icon-finished-downloads");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Finished Uploads"), "icon-finished-uploads");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Hash"), "icon-hash");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Preferences"), "icon-preferences");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Public Hubs"), "icon-public-hubs");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Queue"), "icon-queue");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Search"), "icon-search");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Search Spy"), "icon-search-spy");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("ADL Search"), "icon-search-adl");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Notepad"), "icon-notepad");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("System Tab"), "icon-system");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Away Icon"), "icon-away");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Quit"), "icon-quit");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Connect"), "icon-connect");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("File"), "icon-file");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Directory"), "icon-directory");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Limiting"),"icon-limiting");
		addOption_gui(themeIconsStore, wsm, iconTheme, _("Highlightings"),"icon-highlight");
*/
		g_signal_connect(getWidget("importThemeButton"), "clicked", G_CALLBACK(onImportThemeButton_gui), (gpointer)this);
		g_signal_connect(getWidget("exportThemeButton"), "clicked", G_CALLBACK(onExportThemeButton_gui), (gpointer)this);
		g_signal_connect(getWidget("defaultIconsThemeButton"), "clicked", G_CALLBACK(onDefaultIconsThemeButton_gui), (gpointer)this);
		g_signal_connect(getWidget("systemIconsThemeButton"), "clicked", G_CALLBACK(onSystemIconsThemeButton_gui), (gpointer)this);
		g_signal_connect(getWidget("defaultThemeButton"), "clicked", G_CALLBACK(onDefaultThemeButton_gui), (gpointer)this);

		gtk_label_set_text(GTK_LABEL(getWidget("currentThemeLabel")), wsm->getString("theme-name").c_str());
	}

	{ // Toolbar
		toolbarView.setView(GTK_TREE_VIEW(getWidget("toolbarTreeView")));
		toolbarView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		toolbarView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::PIXBUF_STRING, -1, "icon");
		toolbarView.insertHiddenColumn("icon", GDK_TYPE_PIXBUF);
		toolbarView.insertHiddenColumn("keyUse", G_TYPE_STRING);
		toolbarView.finalize();

		toolbarStore = gtk_list_store_newv(toolbarView.getColCount(), toolbarView.getGTypes());
		gtk_tree_view_set_model(toolbarView.get(), GTK_TREE_MODEL(toolbarStore));
		g_object_unref(toolbarStore);
		g_signal_connect(toolbarView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(onOptionsViewToggled_gui), (gpointer)toolbarStore);

/*		GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
		addOption_gui(toolbarStore, wsm, iconTheme, _("Connect"), "toolbar-button-connect",
			"icon-connect");
	//	addOption_gui(toolbarStore, wsm, iconTheme, _("Reconnect"), "toolbar-button-reconnect",
	//		"icon-connect");//TODO maybe
		addOption_gui(toolbarStore, wsm, iconTheme, _("Favorite Hubs"), "toolbar-button-fav-hubs",
			"icon-favorite-hubs");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Favorite Users"), "toolbar-button-fav-users",
			"icon-favorite-users");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Public Hubs"), "toolbar-button-public-hubs",
			"icon-public-hubs");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Preferences"), "toolbar-button-settings",
			"icon-preferences");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Hash"), "toolbar-button-hash",
			"icon-hash");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Search"), "toolbar-button-search",
			"icon-search");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Search Spy"), "toolbar-button-search-spy",
			"icon-search-spy");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Queue"), "toolbar-button-queue",
			"icon-queue");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Finished Downloads"), "toolbar-button-finished-downloads",
			"icon-finished-downloads");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Finished Uploads"), "toolbar-button-finished-uploads",
			"icon-finished-uploads");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Quit"), "toolbar-button-quit",
			"icon-quit");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Notepad"), "toolbar-button-notepad",
			"icon-notepad");
		addOption_gui(toolbarStore, wsm, iconTheme, _("ADL Search"), "toolbar-button-search-adl",
			"icon-search-adl");
		addOption_gui(toolbarStore, wsm, iconTheme, _("System Tab"), "toolbar-button-system",
			"icon-system");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Away Icon"), "toolbar-button-away",
			"icon-away");
		addOption_gui(toolbarStore, wsm, iconTheme, _("Limiting"), "toolbar-limit-bandwith",
                "icon-limiting");
		*/
	}

	{ // Search Spy
		GdkRGBA color;
		
		if (g_c_p(wsm->getString("search-spy-a-color").c_str(), &color))
			g_c_b_s(G_C_B(getWidget("aSPColorButton")), &color);

		if (g_c_p(wsm->getString("search-spy-t-color").c_str(), &color))
			g_c_b_s(G_C_B(getWidget("tSPColorButton")), &color);

		if (g_c_p(wsm->getString("search-spy-q-color").c_str(), &color))
			g_c_b_s(G_C_B(getWidget("qSPColorButton")), &color);

		if (g_c_p(wsm->getString("search-spy-c-color").c_str(), &color))
			g_c_b_s(G_C_B(getWidget("cSPColorButton")), &color);

		if (g_c_p(wsm->getString("search-spy-r-color").c_str(), &color))
			g_c_b_s(G_C_B(getWidget("rSPColorButton")), &color);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("frameSPSpinButton")), (double)WGETI("search-spy-frame"));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("waitingSPSpinButton")), (double)WGETI("search-spy-waiting"));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("topSPSpinButton")), (double)WGETI("search-spy-top"));

		g_signal_connect(getWidget("defaultColorsSPButton"), "clicked", G_CALLBACK(onDefaultColorsSPButton_gui), (gpointer)this);
		g_signal_connect(getWidget("defaultFrameSPButton"), "clicked", G_CALLBACK(onDefaultFrameSPButton_gui), (gpointer)this);
	}

	{ // Window
		// Auto-open
		createOptionsView_gui(windowView1, windowStore1, "windowsAutoOpenTreeView");

		addOption_gui(windowStore1, _("Public Hubs"), "open-public");
		addOption_gui(windowStore1, _("Favorite Hubs"), "open-favorite-hubs");
		addOption_gui(windowStore1, _("Download Queue"), "open-queue");
		addOption_gui(windowStore1, _("Finished Downloads"), "open-finished-downloads");
		addOption_gui(windowStore1, _("Finished Uploads"), "open-finished-uploads");
		addOption_gui(windowStore1, _("Favorite Users"), "open-favorite-users");
		addOption_gui(windowStore1, _("Search Spy"), "open-search-spy");
		addOption_gui(windowStore1, _("Notepad"), "open-notepad");
		addOption_gui(windowStore1, _("System Tab"), "open-system");
		addOption_gui(windowStore1, _("Upload Queue Tab"), "open-upload-queue");
		// Window options
		createOptionsView_gui(windowView2, windowStore2, "windowsOptionsTreeView");

		addOption_gui(windowStore2, _("Open file list window in the background"), SettingsManager::POPUNDER_FILELIST);
		addOption_gui(windowStore2, _("Open new private messages from other users in the background"), SettingsManager::POPUNDER_PM);
		addOption_gui(windowStore2, _("Open new window when using /join"), SettingsManager::JOIN_OPEN_NEW_WINDOW);
		addOption_gui(windowStore2, _("Ignore private messages from the hub"), SettingsManager::IGNORE_HUB_PMS);
		addOption_gui(windowStore2, _("Ignore private messages from bots"), SettingsManager::IGNORE_BOT_PMS);
		addOption_gui(windowStore2, _("Popup box to input password for hubs"), SettingsManager::PROMPT_PASSWORD);

		// Confirmation dialog
		createOptionsView_gui(windowView3, windowStore3, "windowsConfirmTreeView");

		addOption_gui(windowStore3, _("Confirm application exit"), SettingsManager::CONFIRM_EXIT);
		addOption_gui(windowStore3, _("Confirm favorite hub removal"), SettingsManager::CONFIRM_HUB_REMOVAL);
		/// @todo: Uncomment when implemented
		//addOption_gui(windowStore3, _("Confirm item removal in download queue"), SettingsManager::CONFIRM_ITEM_REMOVAL);
	}
//NOTE: BMDC+++
	{
		userListNames.setView(GTK_TREE_VIEW(getWidget("treeviewNames")));
		userListNames.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, -1);
		userListNames.insertHiddenColumn("Color", G_TYPE_STRING);
		userListNames.insertHiddenColumn("Back", G_TYPE_STRING);
		userListNames.insertHiddenColumn("Set", G_TYPE_STRING);
		userListNames.insertHiddenColumn("BackSet", G_TYPE_STRING);
		userListNames.finalize();

		userListStore1 = gtk_list_store_newv(userListNames.getColCount(), userListNames.getGTypes());
		gtk_tree_view_set_model(userListNames.get(), GTK_TREE_MODEL(userListStore1));
		g_object_unref(userListStore1);

		addOption_gui(userListStore1, _("Normal"), "userlist-text-normal", "userlist-bg-normal");
		addOption_gui(userListStore1, _("Operator"), "userlist-text-operator", "userlist-bg-operator");
		addOption_gui(userListStore1, _("Passive"), "userlist-text-pasive", "userlist-bg-pasive");
		addOption_gui(userListStore1, _("Favorite"), "userlist-text-favorite", "userlist-bg-favorite");
		addOption_gui(userListStore1, _("Protected"), "userlist-text-protected", "userlist-bg-protected");
		addOption_gui(userListStore1, _("Ignored"), "userlist-text-ignored", "userlist-bg-ignored");
		addOption_gui(userListStore1, _("Hub or Bot"), "userlist-text-bot-hub", "userlist-bg-bot-hub");
		/**/
		userListPreview.setView(GTK_TREE_VIEW(getWidget("treeviewUserListPreview")));
		userListPreview.insertColumn(_("Name"), G_TYPE_STRING, TreeView::ICON_STRING_TEXT_COLOR,-1, "Icon", "Color");
		userListPreview.insertHiddenColumn("Icon", G_TYPE_STRING);
		userListPreview.insertHiddenColumn("Color", G_TYPE_STRING);
		userListPreview.insertHiddenColumn("Back", G_TYPE_STRING);
		userListPreview.finalize();

		userListStore2 = gtk_list_store_newv(userListPreview.getColCount(), userListPreview.getGTypes());
		gtk_tree_view_set_model(userListPreview.get(), GTK_TREE_MODEL(userListStore2));
		g_object_unref(userListStore2);

		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Normal"), WGETS("userlist-text-normal"), WGETS("icon-normal"), "n");
		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Operator"), WGETS("userlist-text-operator"), WGETS("icon-normal"), "o");
		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Passive"), WGETS("userlist-text-pasive"), WGETS("icon-normal"), "p");
		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Favorite"), WGETS("userlist-text-favorite"), WGETS("icon-normal"), "f");
		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Protected"), WGETS("userlist-text-protected"), WGETS("icon-normal"), "r");
		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Ignored"), WGETS("userlist-text-ignored"), WGETS("icon-normal"), "i");
		addPreviewUL_gui(userListStore2, string(_("User ")) + _("Bot or Hub (adc(s) only)"), WGETS("userlist-text-bot-hub"), WGETS("icon-normal"), "a");

		g_signal_connect(getWidget("buttonForeColorTextUL"), "clicked", G_CALLBACK(onTextColorForeULClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("buttonDefUL"), "clicked", G_CALLBACK(onTextColorDefaultULClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("buttonULBGCL"), "clicked", G_CALLBACK(onBgColorULClicked_gui), (gpointer)this);
		setColorRow(_("Name"));
	}
}

void Settings::setColorRow(string cell)
{

	if(userListPreview.getCellRenderOf(cell) != NULL)
	gtk_tree_view_column_set_cell_data_func(userListPreview.getColumn(cell),
								userListPreview.getCellRenderOf(cell),
								Settings::makeColor,
								(gpointer)this,
								NULL);

}

void Settings::makeColor(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer )
{
		string color = "#A52A2A";
		gchar *cltype = NULL;

		if(model == NULL)
			return;
		if(iter == NULL)
			return;
		if( cell == NULL)
			return;
		if(column == NULL)
			return;

		gtk_tree_model_get(model,iter,
		                     3,&cltype,
						-1);
		gchar *a = cltype;
		//Hub&Bot
		if ( strcmp(a,"a") == 0)
		{
		  color = WGETS("userlist-bg-bot-hub");
		}//Ops
		else if ( strcmp(a,"o") == 0)
		{
			color = WGETS("userlist-bg-operator");
		}//Favorite
		else if ( strcmp(a,"f") == 0)
		{
			color = WGETS("userlist-bg-favorite");
		}
		//Normal
		else if ( strcmp(a,"n") == 0)
		{
			color = WGETS("userlist-bg-normal");
		}//Passive
		else if ( strcmp(a,"p") == 0)
		{
			color = WGETS("userlist-bg-pasive");
		}//Protected
		else if (strcmp(a,"r") == 0)
		{
			color = WGETS("userlist-bg-protected");
		}//Ignored
		else if (strcmp(a,"i") == 0)
		{
		    color = WGETS("userlist-bg-ignored");
		}
		else {
			color = WGETS("userlist-bg-normal");
		}

		g_object_set(cell,"cell-background-set",TRUE,"cell-background",color.c_str(),NULL);
		g_free(a);
}

void Settings::onSetBackGroundChat(GtkWidget* , gpointer data)
{
	/*
	Settings *s = (Settings *)data;
	GtkWidget *dialog = gtk_color_chooser_dialog_new(_("Set Color"),GTK_WINDOW(s->getContainer()));
	GdkRGBA color;
	if (gdk_rgba_parse(&color,SETTING(BACKGROUND_CHAT_COLOR).c_str()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &color);

	//gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	//if (response == GTK_RESPONSE_OK)
	{
/*		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&color);
		string strcolor = WulforUtil::colorToString(&color);
		SettingsManager::getInstance()->set(SettingsManager::BACKGROUND_CHAT_COLOR, strcolor);
		SettingsManager::getInstance()->save();
		GtkCssProvider *provider = gtk_css_provider_new ();
		GtkStyleContext *csc = gtk_widget_get_style_context(getWidget("textViewPreviewStyles")); 
		std::string t_css = std::string("#prewienTextView text { background: "+strcolor+" ;}\n\0");
		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);

		gtk_style_context_add_provider (csc,
											GTK_STYLE_PROVIDER(provider),
											GTK_STYLE_PROVIDER_PRIORITY_USER);*/
//	}
}

void Settings::initHighlighting_gui()//NOTE: BMDC++
{
	GtkTreeIter iter;
	hView.setView(GTK_TREE_VIEW(getWidget("treeviewHighlighting")));
	hView.insertColumn(_("String"), G_TYPE_STRING, TreeView::STRING, 100);
	hView.insertHiddenColumn("Bold", G_TYPE_STRING);
	hView.insertHiddenColumn("Underline", G_TYPE_STRING);
	hView.insertHiddenColumn("Italic", G_TYPE_STRING);
	hView.insertHiddenColumn("Bold Tab", G_TYPE_STRING);
	hView.insertHiddenColumn("Include Nick", G_TYPE_STRING);
	hView.insertHiddenColumn("Popup Enable", G_TYPE_STRING);
	hView.insertHiddenColumn("Popup String", G_TYPE_STRING);
	hView.insertHiddenColumn("Enable Sound", G_TYPE_STRING);
	hView.insertHiddenColumn("Sound File", G_TYPE_STRING);
	hView.insertHiddenColumn("Enable Fore Color", G_TYPE_STRING);
	hView.insertHiddenColumn("Fore Color", G_TYPE_STRING);
	hView.insertHiddenColumn("Enable Back Color", G_TYPE_STRING);
	hView.insertHiddenColumn("Back Color", G_TYPE_STRING);
	hView.insertHiddenColumn("Case sensitive", G_TYPE_STRING);
	hView.finalize();

	hStore = gtk_list_store_newv(hView.getColCount(), hView.getGTypes());
	gtk_tree_view_set_model(hView.get(), GTK_TREE_MODEL(hStore));
	g_object_unref(hStore);

	selection = gtk_tree_view_get_selection(hView.get());

	ColorList* cList = HighlightManager::getInstance()->getList();
		for(auto i = cList->begin();i != cList->end(); ++i) {
			ColorSettings *cs= &(*i);
			pList.push_back((*i));

			gtk_list_store_append(hStore,&iter);
			gtk_list_store_set(hStore,&iter,
							hView.col(_("String")), cs->getMatch().c_str(),
							hView.col("Bold"), cs->getBold() ? "1" : "0",
							hView.col("Underline"), cs->getUnderline() ? "1" : "0",
							hView.col("Italic"), cs->getItalic() ? "1" : "0",
							hView.col("Bold Tab"), cs->getTab() ? "1" : "0",
							hView.col("Include Nick"), cs->getIncludeNick() ? "1" : "0",
							hView.col("Popup Enable"), cs->getPopup() ? "1" : "0",
							hView.col("Popup String"), cs->getNoti().c_str(),
							hView.col("Enable Sound"), cs->getPlaySound() ? "1" : "0",
							hView.col("Sound File"), cs->getSoundFile().c_str(),
							hView.col("Enable Fore Color"), cs->getHasFgColor() ? "1" : "0",
							hView.col("Fore Color"), cs->getFgColor().c_str(),
							hView.col("Enable Back Color"), cs->getHasBgColor() ? "1" : "0",
							hView.col("Back Color"), cs->getBgColor().c_str(),
							hView.col("Case sensitive"), cs->usingRegexp() ? "1" : "0",
							-1);

	}
	//Main
	g_signal_connect(getWidget("buttonHGADD"), "clicked", G_CALLBACK(onAddHighlighting_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonHGED"), "clicked", G_CALLBACK(onEditHighlighting_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonHGDEL"), "clicked", G_CALLBACK(onRemoveHighlighting_gui), (gpointer)this);
	//Dialog
	g_signal_connect(getWidget("buttonColorText"), "clicked", G_CALLBACK(onColorText_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonBackground"), "clicked", G_CALLBACK(onColorBack_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonSound"), "clicked", G_CALLBACK(onSound_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("checkHGText"), "toggled", G_CALLBACK(onToggledHGText_gui), (gpointer)this);
	g_signal_connect(getWidget("checkHGColor"), "toggled", G_CALLBACK(onToggledHGColor_gui), (gpointer)this);
	g_signal_connect(getWidget("checkHGSound"), "toggled", G_CALLBACK(onToggledHGSound_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonHGNotify"), "toggled", G_CALLBACK(onToggledHGNotify_gui), (gpointer)this);
	//init dialog
	onToggledHGColor_gui(NULL, (gpointer)this);
	onToggledHGNotify_gui(NULL, (gpointer)this);
	onToggledHGSound_gui(NULL, (gpointer)this);
	onToggledHGText_gui(NULL, (gpointer)this);
}

void Settings::initLog_gui()
{
	// g_signal_connect(getWidget("logBrowseButton"), "clicked", G_CALLBACK(onLogBrowseClicked_gui), (gpointer)this);
	gtk_editable_set_text(GTK_EDITABLE(getWidget("logDirectoryEntry")), SETTING(LOG_DIRECTORY).c_str());

	//g_signal_connect(getWidget("logMainCheckButton"), "toggled", G_CALLBACK(onLogMainClicked_gui), (gpointer)this);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logMainCheckButton")), SETTING(LOG_MAIN_CHAT));
	gtk_editable_set_text(GTK_EDITABLE(getWidget("logMainEntry")), SETTING(LOG_FORMAT_MAIN_CHAT).c_str());

	//g_signal_connect(getWidget("logPrivateCheckButton"), "toggled", G_CALLBACK(onLogPrivateClicked_gui), (gpointer)this);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logPrivateCheckButton")), SETTING(LOG_PRIVATE_CHAT));
	gtk_editable_set_text(GTK_EDITABLE(getWidget("logPrivateEntry")), SETTING(LOG_FORMAT_PRIVATE_CHAT).c_str());

//	g_signal_connect(getWidget("logDownloadsCheckButton"), "toggled", G_CALLBACK(onLogDownloadClicked_gui), (gpointer)this);
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logDownloadsCheckButton")), SETTING(LOG_DOWNLOADS));
	gtk_editable_set_text(GTK_EDITABLE(getWidget("logDownloadsEntry")), SETTING(LOG_FORMAT_POST_DOWNLOAD).c_str());

//	g_signal_connect(getWidget("logUploadsCheckButton"), "toggled", G_CALLBACK(onLogUploadClicked_gui), (gpointer)this);
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logUploadsCheckButton")), SETTING(LOG_UPLOADS));
	gtk_editable_set_text(GTK_EDITABLE(getWidget("logUploadsEntry")), SETTING(LOG_FORMAT_POST_UPLOAD).c_str());

//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logSystemCheckButton")), SETTING(LOG_SYSTEM));
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logStatusCheckButton")), SETTING(LOG_STATUS_MESSAGES));
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("logFilelistTransfersCheckButton")), SETTING(LOG_FILELIST_TRANSFERS));
	//Raws
//	g_signal_connect(getWidget("checkraws"), "toggled", G_CALLBACK(onRawsClicked_gui), (gpointer)this);
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkraws")), SETTING(LOG_RAW_CMD));
	gtk_editable_set_text(GTK_EDITABLE(getWidget("entryraws")), SETTING(LOG_FORMAT_RAW).c_str());
}

void Settings::initAdvanced_gui()
{
	{ // Advanced
		createOptionsView_gui(advancedView, advancedStore, "advancedTreeView");

		addOption_gui(advancedStore, _("Auto-away on minimize (and back on restore)"), SettingsManager::AUTO_AWAY);
		addOption_gui(advancedStore, _("Automatically follow redirects"), SettingsManager::AUTO_FOLLOW);
		addOption_gui(advancedStore, _("Clear search box after each search"), SettingsManager::CLEAR_SEARCH);
		addOption_gui(advancedStore, _("Keep duplicate files in your file list (duplicates never count towards your share size)"), SettingsManager::LIST_DUPES);
		addOption_gui(advancedStore, _("Don't delete file lists when exiting"), SettingsManager::KEEP_LISTS);
		addOption_gui(advancedStore, _("Automatically disconnect users who leave the hub"), SettingsManager::AUTO_KICK);
		addOption_gui(advancedStore, _("Enable segmented downloads"), SettingsManager::SEGMENTED_DL);
		addOption_gui(advancedStore, _("Enable automatic SFV checking"), SettingsManager::SFV_CHECK);
		addOption_gui(advancedStore, _("Enable safe and compressed transfers"), SettingsManager::COMPRESS_TRANSFERS);
		addOption_gui(advancedStore, _("Accept custom user commands from hub"), SettingsManager::HUB_USER_COMMANDS);
		addOption_gui(advancedStore, _("Send unknown /commands to the hub"), SettingsManager::SEND_UNKNOWN_COMMANDS);
		addOption_gui(advancedStore, _("Add finished files to share instantly (if shared)"), SettingsManager::ADD_FINISHED_INSTANTLY);
		addOption_gui(advancedStore, _("Don't send the away message to bots"), SettingsManager::NO_AWAYMSG_TO_BOTS);
		addOption_gui(advancedStore, _("Register with the OS to handle dchub:// and adc:// URL links"), SettingsManager::URL_HANDLER);
		addOption_gui(advancedStore, _("Register with the OS to handle magnet: URL links"), SettingsManager::MAGNET_REGISTER);
		addOption_gui(advancedStore, _("Use Coral"), SettingsManager::CORAL);
	}

	{ // User Commands
		userCommandView.setView(GTK_TREE_VIEW(getWidget("userCommandTreeView")));
		userCommandView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, -1);
		userCommandView.insertColumn(_("Hub"), G_TYPE_STRING, TreeView::STRING, -1);
		userCommandView.insertColumn(_("Command"), G_TYPE_STRING, TreeView::STRING, -1);
		userCommandView.finalize();
		userCommandStore = gtk_list_store_newv(userCommandView.getColCount(), userCommandView.getGTypes());
		gtk_tree_view_set_model(userCommandView.get(), GTK_TREE_MODEL(userCommandStore));
		g_object_unref(userCommandStore);

		// Don't allow the columns to be sorted since we use move up/down functions
		gtk_tree_view_column_set_sort_column_id(gtk_tree_view_get_column(userCommandView.get(), userCommandView.col(_("Name"))), -1);
		gtk_tree_view_column_set_sort_column_id(gtk_tree_view_get_column(userCommandView.get(), userCommandView.col(_("Command"))), -1);
		gtk_tree_view_column_set_sort_column_id(gtk_tree_view_get_column(userCommandView.get(), userCommandView.col(_("Hub"))), -1);

		gtk_window_set_transient_for(GTK_WINDOW(getWidget("commandDialog")), GTK_WINDOW(getContainer()));

		g_signal_connect(getWidget("userCommandAddButton"), "clicked", G_CALLBACK(onUserCommandAdd_gui), (gpointer)this);
		g_signal_connect(getWidget("userCommandRemoveButton"), "clicked", G_CALLBACK(onUserCommandRemove_gui), (gpointer)this);
		g_signal_connect(getWidget("userCommandEditButton"), "clicked", G_CALLBACK(onUserCommandEdit_gui), (gpointer)this);
		g_signal_connect(getWidget("userCommandUpButton"), "clicked", G_CALLBACK(onUserCommandMoveUp_gui), (gpointer)this);
		g_signal_connect(getWidget("userCommandDownButton"), "clicked", G_CALLBACK(onUserCommandMoveDown_gui), (gpointer)this);
		g_signal_connect(getWidget("commandDialogSeparator"), "toggled", G_CALLBACK(onUserCommandTypeSeparator_gui), (gpointer)this);
		g_signal_connect(getWidget("commandDialogRaw"), "toggled", G_CALLBACK(onUserCommandTypeRaw_gui), (gpointer)this);
		g_signal_connect(getWidget("commandDialogChat"), "toggled", G_CALLBACK(onUserCommandTypeChat_gui), (gpointer)this);
		g_signal_connect(getWidget("commandDialogPM"), "toggled", G_CALLBACK(onUserCommandTypePM_gui), (gpointer)this);
//		g_signal_connect(getWidget("commandDialogCommand"), "key-release-event", G_CALLBACK(onUserCommandKeyPress_gui), (gpointer)this);
//		g_signal_connect(getWidget("commandDialogTo"), "key-release-event", G_CALLBACK(onUserCommandKeyPress_gui), (gpointer)this);
		loadUserCommands_gui();
	}

	{ // Experts
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("hashSpeedSpinButton")), (double)SETTING(MAX_HASH_SPEED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("writeBufferSpinButton")), (double)SETTING(BUFFER_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("pmHistorySpinButton")), (double)SETTING(PM_LAST_LOG_LINES));//NOTE: core 0.762
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("slotSizeSpinButton")), (double)SETTING(SET_MINISLOT_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("maxListSizeSpinButton")), (double)SETTING(MAX_FILELIST_SIZE));
		gtk_editable_set_text(GTK_EDITABLE(getWidget("CIDEntry")), SETTING(PRIVATE_ID).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("autoRefreshSpinButton")), (double)SETTING(AUTO_REFRESH_TIME));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("searchHistorySpinButton")), (double)SETTING(SEARCH_HISTORY));
		gtk_editable_set_text(GTK_EDITABLE(getWidget("bindAddressEntry")), SETTING(BIND_ADDRESS).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("bind6AddressEntry")), SETTING(BIND_ADDRESS6).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("socketReadSpinButton")), (double)SETTING(SOCKET_IN_BUFFER));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("socketWriteSpinButton")), (double)SETTING(SOCKET_OUT_BUFFER));
	}

	{ // Security Certificates
		gtk_editable_set_text(GTK_EDITABLE(getWidget("privateKeyEntry")), SETTING(TLS_PRIVATE_KEY_FILE).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("certificateFileEntry")), SETTING(TLS_CERTIFICATE_FILE).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("trustedCertificatesPathEntry")), SETTING(TLS_TRUSTED_CERTIFICATES_PATH).c_str());
		g_signal_connect(getWidget("privateKeyButton"), "clicked", G_CALLBACK(onCertificatesPrivateBrowseClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("certificateFileButton"), "clicked", G_CALLBACK(onCertificatesFileBrowseClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("trustedCertificatesPathButton"), "clicked", G_CALLBACK(onCertificatesPathBrowseClicked_gui), (gpointer)this);

		createOptionsView_gui(certificatesView, certificatesStore, "certificatesTreeView");

		addOption_gui(certificatesStore, _("Use TLS when remote client supports it"), SettingsManager::REQUIRE_TLS);
		addOption_gui(certificatesStore, _("Allow TLS connections to hubs without trusted certificate"), SettingsManager::ALLOW_UNTRUSTED_HUBS);
		addOption_gui(certificatesStore, _("Allow TLS connections to clients without trusted certificate"), SettingsManager::ALLOW_UNTRUSTED_CLIENTS);

		g_signal_connect(getWidget("generateCertificatesButton"), "clicked", G_CALLBACK(onGenerateCertificatesClicked_gui), (gpointer)this);
	}
	{
	#ifdef HAVE_LIBTAR
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("enableBackup")), SETTING(ENABLE_AUTOBACKUP) == 1 ? TRUE : FALSE);
		gtk_editable_set_text(GTK_EDITABLE(getWidget("backupTimestampEntry")), SETTING(BACKUP_TIMESTAMP).c_str());
		gtk_editable_set_text(GTK_EDITABLE(getWidget("backupPatternEntry")), SETTING(BACKUP_FILE_PATTERN).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("backupSpin")), (double)SETTING(AUTOBACKUP_TIME));

		g_signal_connect(getWidget("buttonRestore"), "clicked", G_CALLBACK([]() { RestoreManager::getInstance()->restoreBackup();}), (gpointer)this);
	#else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("enableBackup")), FALSE);
		gtk_widget_set_sensitive(getWidget("enableBackup"), FALSE);
	#endif
	}
}

void Settings::initBandwidthLimiting_gui()
{
	// Transfer Rate Limiting
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("transferMaxUpload")), (double)SETTING(MAX_UPLOAD_SPEED_MAIN));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("transferMaxDownload")), (double)SETTING(MAX_DOWNLOAD_SPEED_MAIN));

	// Secondary Transfer Rate Settings
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("secondaryMaxUpload")), (double)SETTING(MAX_UPLOAD_SPEED_ALTERNATE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("secondaryMaxDownload")), (double)SETTING(MAX_DOWNLOAD_SPEED_ALTERNATE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("secondaryUploadSlots")), (double)SETTING(SLOTS_ALTERNATE_LIMITING));

	// Secondary Transfer Rate Limiting
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("limitsFromCombobox")), SETTING(BANDWIDTH_LIMIT_START));
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("limitsToCombobox")), SETTING(BANDWIDTH_LIMIT_END));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("useLimitSecondCheckButton")), SETTING(TIME_DEPENDENT_THROTTLE));

	onLimitSecondToggled_gui(NULL, (gpointer)this);
	g_signal_connect(getWidget("useLimitSecondCheckButton"), "toggled", G_CALLBACK(onLimitSecondToggled_gui), (gpointer)this);
}

void Settings::initSearchTypes_gui()
{
	// search type list
	searchTypeView.setView(GTK_TREE_VIEW(getWidget("searchTypeTreeView")));
	searchTypeView.insertColumn(_("Search type"), G_TYPE_STRING, TreeView::STRING, -1);
	searchTypeView.insertColumn(_("Predefined"), G_TYPE_STRING, TreeView::STRING, -1);
	searchTypeView.insertColumn(_("Extensions"), G_TYPE_STRING, TreeView::STRING, -1);
	searchTypeView.insertHiddenColumn("Key", G_TYPE_INT);
	searchTypeView.finalize();

	searchTypeStore = gtk_list_store_newv(searchTypeView.getColCount(), searchTypeView.getGTypes());
	gtk_tree_view_set_model(searchTypeView.get(), GTK_TREE_MODEL(searchTypeStore));
	g_object_unref(searchTypeStore);

	// extension list
	extensionView.setView(GTK_TREE_VIEW(getWidget("extensionTreeView")));
	extensionView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
	extensionView.finalize();

	extensionStore = gtk_list_store_newv(extensionView.getColCount(), extensionView.getGTypes());
	gtk_tree_view_set_model(extensionView.get(), GTK_TREE_MODEL(extensionStore));
	g_object_unref(extensionStore);

	// search types
	const SettingsManager::SearchTypes &searchTypes = SettingsManager::getInstance()->getSearchTypes();
	for (auto i = searchTypes.begin(), iend = searchTypes.end(); i != iend; ++i)
	{
		string type = i->first;
		bool predefined = false;
		int key = SearchManager::TYPE_ANY;

		if (type.size() == 1 && type[0] >= '1' && type[0] <= '6')
		{
			key = type[0] - '0';
			type = SearchManager::getTypeStr(key);
			predefined = true;
		}
		addOption_gui(searchTypeStore, type, i->second, predefined, key);
	}

	g_signal_connect(getWidget("addSTButton"), "clicked", G_CALLBACK(onAddSTButton_gui), (gpointer)this);
	g_signal_connect(getWidget("modifySTButton"), "clicked", G_CALLBACK(onModifySTButton_gui), (gpointer)this);
	g_signal_connect(getWidget("renameSTButton"), "clicked", G_CALLBACK(onRenameSTButton_gui), (gpointer)this);
	g_signal_connect(getWidget("removeSTButton"), "clicked", G_CALLBACK(onRemoveSTButton_gui), (gpointer)this);
	g_signal_connect(getWidget("defaultSTButton"), "clicked", G_CALLBACK(onDefaultSTButton_gui), (gpointer)this);

	g_signal_connect(getWidget("addExtensionButton"), "clicked", G_CALLBACK(onAddExtensionButton_gui), (gpointer)this);
	g_signal_connect(getWidget("editExtensionButton"), "clicked", G_CALLBACK(onEditExtensionButton_gui), (gpointer)this);
	g_signal_connect(getWidget("removeExtensionButton"), "clicked", G_CALLBACK(onRemoveExtensionButton_gui), (gpointer)this);
	g_signal_connect(getWidget("upExtensionButton"), "clicked", G_CALLBACK(onUpExtensionButton_gui), (gpointer)this);
	g_signal_connect(getWidget("downExtensionButton"), "clicked", G_CALLBACK(onDownExtensionButton_gui), (gpointer)this);

//	g_signal_connect(searchTypeView.get(), "key-release-event", G_CALLBACK(onSTKeyReleased_gui), (gpointer)this);
	//g_signal_connect(searchTypeView.get(), "button-release-event", G_CALLBACK(onSTButtonReleased_gui), (gpointer)this);
}
/*
void Settings::onSTKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	Settings *s = (Settings *)data;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->searchTypeView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			int key = s->searchTypeView.getValue<int>(&iter, "Key");
			gboolean sensitive = FALSE;
			if (key == SearchManager::TYPE_ANY)
				sensitive = TRUE;
			gtk_widget_set_sensitive(s->getWidget("renameSTButton"), sensitive);
			gtk_widget_set_sensitive(s->getWidget("removeSTButton"), sensitive);
		}
	}
}*/
/*
void Settings::onSTButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings *)data;

	if (event->button == 3 || event->button == 1)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->searchTypeView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			int key = s->searchTypeView.getValue<int>(&iter, "Key");
			gboolean sensitive = FALSE;
			if (key == SearchManager::TYPE_ANY)
				sensitive = TRUE;
			gtk_widget_set_sensitive(s->getWidget("renameSTButton"), sensitive);
			gtk_widget_set_sensitive(s->getWidget("removeSTButton"), sensitive);
		}
	}
}
*/
void Settings::addOption_gui(GtkListStore *store, const string &type, const StringList &exts, bool predefined, const int key)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, type.c_str(),                      //searchtype
		1, predefined ? _("Predefined") : "", //predefined
		2, Util::toString(";", exts).c_str(), //extensions
		3, key,                               //key predefined
		-1);
}

void Settings::addPreviewUL_gui(GtkListStore *store, const std::string &name, const std::string &color, const std::string &icon, const string &back)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
			0, name.c_str(),
			1, icon.c_str(),
			2, color.c_str(),
			3, back.c_str(),
			-1);
	colorsIters.insert(UnMapIter::value_type(name, iter));
}

void Settings::onAddSTButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkWidget *dialog = s->getWidget("nameDialog");
	GtkWidget *entry = s->getWidget("nameDialogEntry");
	gtk_window_set_title(GTK_WINDOW(dialog), _("New search type"));
	gtk_label_set_markup(GTK_LABEL(s->getWidget("labelNameDialog")), _("<b>Name of the new search type</b>"));
//	gtk_entry_set_text(GTK_ENTRY(entry), "");
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
//		string name = gtk_entry_get_text(GTK_ENTRY(entry));
		gtk_widget_hide(dialog);
		try
		{
//			SettingsManager::getInstance()->validateSearchTypeName(name);
		}
		catch (const SearchTypeException& e)
		{
			s->showErrorDialog(e.getError());
			return;
		}
		gtk_list_store_clear(s->extensionStore);
		gtk_editable_set_text(GTK_EDITABLE(s->getWidget("extensionEntry")), "");
		s->showExtensionDialog_gui(TRUE);
	}
}

void Settings::showExtensionDialog_gui(bool add)
{
	GtkWidget *dialog = getWidget("ExtensionsDialog");
	gtk_window_set_title(GTK_WINDOW(dialog), _("Extensions"));
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
		if (add)
			addSearchType_gui();
		else
			modSearchType_gui();
	}
	gtk_widget_hide(dialog);
}

void Settings::addSearchType_gui()
{
	string name = gtk_editable_get_text(GTK_EDITABLE(getWidget("nameDialogEntry")));

	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(extensionStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	StringList extensions;

	while (valid)
	{
		string ext = extensionView.getString(&iter, "Name");
		extensions.push_back(ext);

		valid = gtk_tree_model_iter_next(m, &iter);
	}

	if (extensions.empty())
	{
		showErrorDialog(_("Error"));
		return;
	}

	try
	{
//		SettingsManager::getInstance()->addSearchType(name, extensions);
	}
	catch (const SearchTypeException& e)
	{
		showErrorDialog(e.getError());
		return;
	}

	gtk_list_store_append(searchTypeStore, &iter);
	gtk_list_store_set(searchTypeStore, &iter,
//		searchTypeView.col(_("Search type")), name.c_str(),
		searchTypeView.col(_("Predefined")), "",
		searchTypeView.col(_("Extensions")), Util::toString(";", extensions).c_str(),
		-1);
}

void Settings::modSearchType_gui()
{
	GtkTreeIter t_iter, e_iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(searchTypeView.get());
	if (!gtk_tree_selection_get_selected(selection, NULL, &t_iter))
		return;

	GtkTreeModel *m = GTK_TREE_MODEL(extensionStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &e_iter);

	StringList extensions;

	while (valid)
	{
		string ext = extensionView.getString(&e_iter, "Name");
		extensions.push_back(ext);

		valid = gtk_tree_model_iter_next(m, &e_iter);
	}

	if (extensions.empty())
	{
		showErrorDialog(_("Error"));
		return;
	}

	int key = searchTypeView.getValue<int>(&t_iter, "Key");
	string name = searchTypeView.getString(&t_iter, _("Search type"));

	try
	{
		if (key == SearchManager::TYPE_ANY)
		{
			// Custom searchtype
			SettingsManager::getInstance()->modSearchType(name, extensions);
		}
		else if (key > SearchManager::TYPE_ANY && key < SearchManager::TYPE_DIRECTORY)
		{
			// Predefined searchtype
			SettingsManager::getInstance()->modSearchType(string(1, '0' + key), extensions);
		}
		else
			return;
	}
	catch (const SearchTypeException& e)
	{
		showErrorDialog(e.getError());
		return;
	}

	gtk_list_store_set(searchTypeStore, &t_iter,
		searchTypeView.col(_("Extensions")), Util::toString(";", extensions).c_str(),
		-1);
}

void Settings::addExtension_gui(const string ext)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(extensionStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		string name = extensionView.getString(&iter, "Name");
		if (name == ext)
			return;

		valid = gtk_tree_model_iter_next(m, &iter);
	}

	gtk_list_store_append(extensionStore, &iter);
	gtk_list_store_set(extensionStore, &iter,
		0, ext.c_str(),
		-1);
}

void Settings::onAddExtensionButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	string error;
	string text = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("extensionEntry")));
	StringTokenizer<string> exts(text, ';');
	for (StringIterC i = exts.getTokens().begin(), j = exts.getTokens().end(); i != j; ++i)
	{
		if (!i->empty())
		{
			string ext = *i;
			if (Util::checkExtension(ext))
			{
				s->addExtension_gui(ext);
			}
			else
				error += ext + " ";
		}
	}

	if (!error.empty())
		s->showErrorDialog(string(_("Invalid extension: ")) + error);
}

void Settings::onModifySTButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->searchTypeView.get());
	int key = SearchManager::TYPE_ANY;
	string name;

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		key = s->searchTypeView.getValue<int>(&iter, "Key");
		name = s->searchTypeView.getString(&iter, _("Search type"));
	}
	else
		return;

	StringList list;
	try
	{
		if (key == SearchManager::TYPE_ANY)
		{
			// Custom searchtype
			list = SettingsManager::getInstance()->getExtensions(name);
		}
		else if (key > SearchManager::TYPE_ANY && key < SearchManager::TYPE_DIRECTORY)
		{
			// Predefined searchtype
			list = SettingsManager::getInstance()->getExtensions(string(1, '0' + key));
		}
		else
			return;
	}
	catch (const SearchTypeException& e)
	{
		s->showErrorDialog(e.getError());
		return;
	}

	gtk_list_store_clear(s->extensionStore);
	gtk_editable_set_text(GTK_EDITABLE(s->getWidget("extensionEntry")), "");

	for (StringIterC i = list.begin(), j = list.end(); i != j; ++i)
	{
		string ext = *i;
		gtk_list_store_append(s->extensionStore, &iter);
		gtk_list_store_set(s->extensionStore, &iter, 0, ext.c_str(), -1);
	}
	s->showExtensionDialog_gui(FALSE);
}

void Settings::onRenameSTButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->searchTypeView.get());
	string old_name, new_name;

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		old_name = s->searchTypeView.getString(&iter, _("Search type"));
	}
	else
		return;

	GtkWidget *dialog = s->getWidget("nameDialog");
	GtkWidget *entry = s->getWidget("nameDialogEntry");
	gtk_window_set_title(GTK_WINDOW(dialog), _("Rename a search type"));
	gtk_label_set_markup(GTK_LABEL(s->getWidget("labelNameDialog")), _("<b>New name</b>"));
	gtk_editable_set_text(GTK_EDITABLE(entry), old_name.c_str());

//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
/*		new_name = gtk_entry_get_text(GTK_ENTRY(entry));
		gtk_widget_hide(dialog);
		try
		{
			SettingsManager::getInstance()->renameSearchType(old_name, new_name);
		}
		catch (const SearchTypeException& e)
		{
			s->showErrorDialog(e.getError());
			return;
		}*/
	}
/*	else
	{
		gtk_widget_hide(dialog);
		return;
	}
*/
	gtk_list_store_set(s->searchTypeStore, &iter,
		0, new_name.c_str(),
		-1);
}

void Settings::onRemoveSTButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->searchTypeView.get());
	string name;

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		name = s->searchTypeView.getString(&iter, _("Search type"));
	}
	else
		return;

	try
	{
		SettingsManager::getInstance()->delSearchType(name);
	}
	catch (const SearchTypeException& e)
	{
		s->showErrorDialog(e.getError());
		return;
	}
	gtk_list_store_remove(s->searchTypeStore, &iter);
}

void Settings::onDefaultSTButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_list_store_clear(s->searchTypeStore);
	SettingsManager::getInstance()->setSearchTypeDefaults();

	// search types
	const SettingsManager::SearchTypes &searchTypes = SettingsManager::getInstance()->getSearchTypes();
	for (auto i = searchTypes.begin(), j = searchTypes.end(); i != j; ++i)
	{
		string type = i->first;
		bool predefined = false;
		int key = SearchManager::TYPE_ANY;

		if (type.size() == 1 && type[0] >= '1' && type[0] <= '6')
		{
			key = type[0] - '0';
			type = SearchManager::getTypeStr(key);
			predefined = true;
		}
		s->addOption_gui(s->searchTypeStore, type, i->second, predefined, key);
	}
}

void Settings::onEditExtensionButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->extensionView.get());
	string old_ext;

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		old_ext = s->extensionView.getString(&iter, "Name");
	}
	else
		return;

	GtkWidget *dialog = s->getWidget("nameDialog");
	GtkWidget *entry = s->getWidget("nameDialogEntry");
	gtk_window_set_title(GTK_WINDOW(dialog), _("Extension edition"));
	gtk_label_set_markup(GTK_LABEL(s->getWidget("labelNameDialog")), _("<b>Extension</b>"));

	gtk_editable_set_text(GTK_EDITABLE(entry), old_ext.c_str());
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
	/*	string new_ext;
		new_ext = gtk_entry_get_text(GTK_ENTRY(entry));
		if (new_ext.find(";") == string::npos && Util::checkExtension(new_ext))
		{
			gtk_list_store_set(s->extensionStore, &iter,
				0, new_ext.c_str(),
				-1);
		}
		else
			s->showErrorDialog(string(_("Invalid extension: ")) + new_ext);*/
	}
	gtk_widget_hide(dialog);
}

void Settings::onRemoveExtensionButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->extensionView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		gtk_list_store_remove(s->extensionStore, &iter);
	}
}

void Settings::onUpExtensionButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(s->extensionStore);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->extensionView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
			gtk_list_store_swap(s->extensionStore, &current, &prev);
		gtk_tree_path_free(path);
	}
}

void Settings::onDownExtensionButton_gui(GtkWidget *, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter current, next;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->extensionView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->extensionStore), &next))
			gtk_list_store_swap(s->extensionStore, &current, &next);
	}
}

void Settings::onNotifyTestButton_gui(GtkWidget *, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string title = s->notifyView.getString(&iter, _("Title"));
		string icon = s->notifyView.getString(&iter, _("Icon"));
		GNotificationPriority urgency = (GNotificationPriority)s->notifyView.getValue<int>(&iter, "Urgency");
		Notify::get()->showNotify(title, "<span weight=\"bold\" size=\"larger\">" + string(_("*** T E S T ***")) + "</span>",
			"", icon, gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("notifyIconSizeComboBox"))), urgency);
	}
}

void Settings::onNotifyIconFileBrowseClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_OPEN);
//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));
	gtk_widget_hide(s->getWidget("fileChooserDialog"));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));

//		if (path)
		{
			GtkTreeIter iter;
			GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

			if (gtk_tree_selection_get_selected(selection, NULL, &iter))
			{
//				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);

//				if (pixbuf != NULL)
//				{
//					string target = Text::toUtf8(path);
//					GdkPixbuf *icon = WulforUtil::scalePixbuf(pixbuf, ICON_SIZE, ICON_SIZE);
//					g_object_unref(pixbuf);

//					gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col(_("Icon")), target.c_str(), -1);
//					gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), icon, -1);
//					g_object_unref(icon);
//				}
			}
		}
	}
}
/*
void Settings::onNotifyKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	Settings *s = (Settings *)data;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("notifyTitleEntry")), s->notifyView.getString(&iter, _("Title")).c_str());
		}
	}
}*/
/*
void Settings::onNotifyButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings *)data;

	if (event->button == 3 || event->button == 1)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("notifyTitleEntry")), s->notifyView.getString(&iter, _("Title")).c_str());
		}
	}
}
*/
void Settings::onNotifyOKClicked_gui(GtkWidget*, gpointer data)
{
/*	Settings *s = (Settings *)data;

	string title = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("notifyTitleEntry")));

//	if (title.empty())
	{
		s->showErrorDialog(_("...must not be empty"));
		return;
	}

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string temp = s->notifyView.getString(&iter, _("Title"));

		if (temp != title)
		{
			gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col(_("Title")), title.c_str(), -1);
		}
	}*/
}

void Settings::onNotifyIconNoneButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))

		gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), NULL, s->notifyView.col(_("Icon")), "", -1);
	else
		s->showErrorDialog(_("...must not be empty"));
}

void Settings::onNotifyDefaultButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		GdkPixbuf *icon = NULL;
		WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
		string title = wsm->getString(s->notifyView.getString(&iter, "keyTitle"), TRUE);
		string path = wsm->getString(s->notifyView.getString(&iter, "keyIcon"), TRUE);

		if (!path.empty())
		{
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(Text::fromUtf8(path).c_str(), NULL);

			if (pixbuf != NULL)
			{
				icon = WulforUtil::scalePixbuf(pixbuf, ICON_SIZE, ICON_SIZE);
				g_object_unref(pixbuf);

				gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), icon, -1);
				g_object_unref(icon);
			}
		}
		else
			gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), icon, -1);

		gtk_list_store_set(s->notifyStore, &iter,
			s->notifyView.col(_("Icon")), path.c_str(), s->notifyView.col(_("Title")), _(title.c_str()), -1);
	}
	else
		s->showErrorDialog(_("...must not be empty"));
}

void Settings::onImportThemeButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_OPEN);
//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));

//		if (path)
		{
//			string file = path;

//			if (s->loadFileTheme(file))
			{
				s->applyIconsTheme();
				s->applyTextTheme();

//				string fileName = Util::getFileName(Text::toUtf8(file));
//				string::size_type i = fileName.rfind('.');
//				string theme = fileName.substr(0, i);
//				gtk_label_set_text(GTK_LABEL(s->getWidget("currentThemeLabel")), theme.c_str());
			}
//			else
//				s->showErrorDialog(_("...must be *.theme"));
		}
	}
}

void Settings::onExportThemeButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

//	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_SAVE);
//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));

//		if (path)
		{
//			string file = path;

//			if (Util::getFileExt(file) != ".theme" || Util::getFileName(file) == ".theme")
			{
				s->showErrorDialog(_("...must be *.theme"));
			}
//			else
			{
				s->setTheme();
//				s->saveFileTheme(file);
			}
		}
	}
}

void Settings::onDefaultIconsThemeButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_label_set_text(GTK_LABEL(s->getWidget("currentThemeLabel")), _("default icons"));
	s->applyIconsTheme(TRUE);
}

void Settings::onSystemIconsThemeButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	string theme = gtk_label_get_text(GTK_LABEL(s->getWidget("currentThemeLabel")));
	theme += _(" + system icons");
	gtk_label_set_text(GTK_LABEL(s->getWidget("currentThemeLabel")), theme.c_str());

	s->set("icon-download", BMDC_STOCK_GO_DOWN);
	s->set("icon-favorite-hubs", BMDC_STOCK_HOME);
	s->set("icon-finished-downloads", BMDC_STOCK_GO_DOWN);
	s->set("icon-finished-uploads", BMDC_STOCK_GO_UP);
	s->set("icon-hash", BMDC_STOCK_CONVERT);
	s->set("icon-preferences", BMDC_STOCK_PREFERENCES);
	s->set("icon-public-hubs", BMDC_STOCK_NETWORK);
	s->set("icon-queue", BMDC_STOCK_DIRECTORY);
	s->set("icon-search", BMDC_STOCK_FIND);
	s->set("icon-upload", BMDC_STOCK_GO_UP);
	s->set("icon-quit", BMDC_STOCK_QUIT);
	s->set("icon-connect", BMDC_STOCK_CONNECT);
	s->set("icon-file", BMDC_STOCK_FILE);
	s->set("icon-directory", BMDC_STOCK_DIRECTORY);
	s->set("icon-notepad",BMDC_STOCK_FILE);
	s->set("icon-adlsearch",BMDC_STOCK_GO_UP);
	s->set("icon-system",BMDC_STOCK_FIND);
	s->set("icon-away", BMDC_STOCK_NETWORK);

	s->applyIconsTheme();
}

void Settings::onDefaultThemeButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_label_set_text(GTK_LABEL(s->getWidget("currentThemeLabel")), _("default theme"));
	s->applyIconsTheme(TRUE);
	s->applyTextTheme(TRUE);
}

void Settings::onDefaultColorsSPButton_gui(GtkWidget *, gpointer data)
{
	Settings *s = (Settings *)data;

	GdkRGBA color;

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	if (g_c_p(wsm->getString("search-spy-a-color", TRUE).c_str(), &color))
		g_c_b_s(G_C_B(s->getWidget("aSPColorButton")), &color);

	if (g_c_p(wsm->getString("search-spy-t-color", TRUE).c_str(), &color))
		g_c_b_s(G_C_B(s->getWidget("tSPColorButton")), &color);

	if (g_c_p(wsm->getString("search-spy-q-color", TRUE).c_str(), &color))
		g_c_b_s(G_C_B(s->getWidget("qSPColorButton")), &color);

	if (g_c_p(wsm->getString("search-spy-c-color", TRUE).c_str(), &color))
		g_c_b_s(G_C_B(s->getWidget("cSPColorButton")), &color);

	if (g_c_p(wsm->getString("search-spy-r-color", TRUE).c_str(), &color))
		g_c_b_s(G_C_B(s->getWidget("rSPColorButton")), &color);
}

void Settings::onDefaultFrameSPButton_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("frameSPSpinButton")), double(wsm->getInt("search-spy-frame", TRUE)));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("waitingSPSpinButton")), double(wsm->getInt("search-spy-waiting", TRUE)));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("topSPSpinButton")), double(wsm->getInt("search-spy-top", TRUE)));
}

void Settings::onLimitSecondToggled_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("useLimitSecondCheckButton"))))
	{
		gtk_widget_set_sensitive(s->getWidget("secondaryTransferSettingsFrame"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("limitsFromCombobox"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("limitsToCombobox"), TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(s->getWidget("secondaryTransferSettingsFrame"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("limitsFromCombobox"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("limitsToCombobox"), FALSE);
	}
}

void Settings::applyIconsTheme(bool useDefault)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(themeIconsStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
//	GtkIconTheme *iconTheme = gtk_icon_theme_get_default();

	while (valid)
	{
		string keyIcon = themeIconsView.getString(&iter, "keyIcon");
		string iconName = getStringTheme(keyIcon, useDefault);
		//GdkPixbuf *icon = gtk_icon_theme_load_icon(iconTheme, iconName.c_str(),
		//	ICON_SIZE, GTK_ICON_LOOKUP_FORCE_SVG, NULL);

		gtk_list_store_set(themeIconsStore, &iter,
		//	themeIconsView.col("icon"), icon,
			themeIconsView.col("iconName"), iconName.c_str(),
			-1);

//		if (icon != NULL)
//			g_object_unref(icon);

		set(keyIcon, iconName);
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Settings::applyTextTheme(bool useDefault)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
	GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(textStyleBuffer);
	string style, fore, back;
	int bolt, italic;

	while (valid)
	{
		style = textStyleView.getString(&iter,_("Style"));
		fore = getStringTheme(textStyleView.getString(&iter, "keyForeColor"), useDefault);
		back = getStringTheme(textStyleView.getString(&iter, "keyBackColor"), useDefault);
		bolt = getIntTheme(textStyleView.getString(&iter, "keyBolt"), useDefault);
		italic = getIntTheme(textStyleView.getString(&iter, "keyItalic"), useDefault);

		gtk_list_store_set(textStyleStore, &iter,
			textStyleView.col("ForeColor"), fore.c_str(),
			textStyleView.col("BackColor"), back.c_str(),
			textStyleView.col("Bolt"), bolt,
			textStyleView.col("Italic"), italic,
			-1);

		GtkTextTag *tag = gtk_text_tag_table_lookup(tag_table, style.c_str());

		if (tag)
			g_object_set(tag,
				"foreground", fore.c_str(),
				"background", back.c_str(),
				"weight", bolt ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
				"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
				NULL);

		valid = gtk_tree_model_iter_next(m, &iter);
	}

	gtk_widget_queue_draw(getWidget("textViewPreviewStylesTheme"));
}

bool Settings::loadFileTheme(const string &file)
{
	if (Util::getFileExt(file) != ".theme" || Util::getFileName(file) == ".theme")
		return FALSE;

	intMapTheme.clear();
	stringMapTheme.clear();

	try
	{
		SimpleXML xml;
		xml.fromXML(File(file, File::READ, File::OPEN).read());
		xml.resetCurrentChild();
		xml.stepIn();

		if (xml.findChild("Settings"))
		{
			xml.stepIn();

			IntMap::iterator iit;
			for (iit = defaultIntTheme.begin(); iit != defaultIntTheme.end(); ++iit)
			{
				if (xml.findChild(iit->first))
					intMapTheme.insert(IntMap::value_type(iit->first, Util::toInt(xml.getChildData())));
				xml.resetCurrentChild();
			}

			StringMap::iterator sit;
			for (sit = defaultStringTheme.begin(); sit != defaultStringTheme.end(); ++sit)
			{
				if (xml.findChild(sit->first))
				{
					stringMapTheme.insert(StringMap::value_type(sit->first, xml.getChildData()));
				}
				xml.resetCurrentChild();
			}

			xml.stepOut();
		}
	}
	catch (const Exception& e)
	{
		dcdebug("bmdc: load theme %s...\n", e.getError().c_str());
		return FALSE;
	}

	return TRUE;
}

void Settings::setTheme()
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		set(textStyleView.getString(&iter, "keyForeColor"), textStyleView.getString(&iter, "ForeColor"));
		set(textStyleView.getString(&iter, "keyBackColor"), textStyleView.getString(&iter, "BackColor"));
		set(textStyleView.getString(&iter, "keyBolt"), textStyleView.getValue<int>(&iter, "Bolt"));
		set(textStyleView.getString(&iter, "keyItalic"), textStyleView.getValue<int>(&iter, "Italic"));

		valid = gtk_tree_model_iter_next(m, &iter);
	}

	m = GTK_TREE_MODEL(themeIconsStore);
	valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		set(themeIconsView.getString(&iter, "keyIcon"), themeIconsView.getString(&iter, "iconName"));
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Settings::saveFileTheme(const string &file)
{
	SimpleXML xml;
	xml.addTag("Theme");
	xml.stepIn();
	xml.addTag("Settings");
	xml.stepIn();

	IntMap::iterator iit;

	for (iit = intMapTheme.begin(); iit != intMapTheme.end(); ++iit)
	{
		xml.addTag(iit->first, iit->second);
		xml.addChildAttrib(string("type"), string("int"));
	}

	StringMap::iterator sit;

	for (sit = stringMapTheme.begin(); sit != stringMapTheme.end(); ++sit)
	{
		xml.addTag(sit->first, sit->second);
		xml.addChildAttrib(string("type"), string("string"));
	}

	xml.stepOut();

	try
	{
		File out(file + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(file);
		File::renameFile(file + ".tmp", file);
	}
	catch (const FileException &)
	{
	}
}

int Settings::getIntTheme(const string &key, bool useDefault)
{
	dcassert(intMapTheme.find(key) != intMapTheme.end() || defaultIntTheme.find(key) != defaultIntTheme.end());

	if (useDefault)
		return defaultIntTheme[key];

	if (intMapTheme.find(key) == intMapTheme.end())
		return defaultIntTheme[key];
	else
		return intMapTheme[key];
}

string Settings::getStringTheme(const string &key, bool useDefault)
{
	dcassert(stringMapTheme.find(key) != stringMapTheme.end() || defaultStringTheme.find(key) != defaultStringTheme.end());

	if (useDefault)
		return defaultStringTheme[key];

	if (stringMapTheme.find(key) == stringMapTheme.end())
		return defaultStringTheme[key];
	else
		return stringMapTheme[key];
}

void Settings::set(const string &key, int value)
{
	dcassert(defaultIntTheme.find(key) != defaultIntTheme.end());
	intMapTheme[key] = value;
}

void Settings::set(const string &key, const string &value)
{
	dcassert(defaultStringTheme.find(key) != defaultStringTheme.end());
	stringMapTheme[key] = value;
}

void Settings::onSoundFileBrowseClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_OPEN);
//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));

//		if (path)
		{
			GtkTreeIter iter;
			GtkTreeSelection *selection = gtk_tree_view_get_selection(s->soundView.get());

			if (gtk_tree_selection_get_selected(selection, NULL, &iter))
			{
//				string target = Text::toUtf8(path);
//				gtk_list_store_set(s->soundStore, &iter, s->soundView.col(_("File")), target.c_str(), -1);
			}
		}
	}
}

void Settings::onSoundPlayButton_gui(GtkWidget *, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->soundView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string target = s->soundView.getString(&iter, _("File"));
		Sound::get()->playSound(target);
	}
}

void Settings::onTextColorForeClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->selectTextColor_gui(0);
}

void Settings::onTextColorBackClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->selectTextColor_gui(1);
}

void Settings::onTextColorBWClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->selectTextColor_gui(2);
}

void Settings::onTextStyleClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->selectTextStyle_gui(0);
}

void Settings::onTextStyleDefaultClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->selectTextStyle_gui(1);
}

void Settings::onPreviewAdd_gui(GtkWidget*, gpointer data)
{
	/*Settings *s = (Settings*)data;

	string name = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("previewNameEntry")));
	string app = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("previewApplicationEntry")));
	string ext = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("previewExtensionsEntry")));

	if (name.empty() || app.empty() || ext.empty())
	{
		s->showErrorDialog(_("Must not be empty..."));
		return;
	}

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	if (wsm->getPreviewApp(name))
	{
		s->showErrorDialog(_("Error"));
		return;
	}

	if (wsm->addPreviewApp(name, app, ext) != NULL)
	{
		GtkTreeIter it;
		gtk_list_store_append(s->previewAppToStore, &it);
		gtk_list_store_set(s->previewAppToStore, &it,
			s->previewAppView.col(_("Name")), name.c_str(),
			s->previewAppView.col(_("Application")), app.c_str(),
			s->previewAppView.col(_("Extensions")), ext.c_str(),
			-1);
	}*/
}

void Settings::onPreviewRemove_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string name = s->previewAppView.getString(&iter, _("Name"));

		if (WulforSettingsManager::getInstance()->removePreviewApp(name))
			gtk_list_store_remove(s->previewAppToStore, &iter);
	}
}

/*void Settings::onPreviewKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	Settings *s = (Settings *)data;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("previewNameEntry")), s->previewAppView.getString(&iter, _("Name")).c_str() );
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("previewApplicationEntry")), s->previewAppView.getString(&iter, _("Application")).c_str());
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("previewExtensionsEntry")), s->previewAppView.getString(&iter, _("Extensions")).c_str());
		}
	}
}*/
/*
void Settings::onPreviewButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings *)data;

	if (event->button == 3 || event->button == 1)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("previewNameEntry")), s->previewAppView.getString(&iter, _("Name")).c_str());
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("previewApplicationEntry")), s->previewAppView.getString(&iter, _("Application")).c_str());
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("previewExtensionsEntry")), s->previewAppView.getString(&iter, _("Extensions")).c_str());
		}
	}
}
*/
void Settings::onPreviewApply_gui(GtkWidget*, gpointer data)
{
	/*Settings *s = (Settings *)data;

	string name = gtk_entry_get_text(GTK_ENTRY(s->getWidget("previewNameEntry")));
	string app = gtk_entry_get_text(GTK_ENTRY(s->getWidget("previewApplicationEntry")));
	string ext = gtk_entry_get_text(GTK_ENTRY(s->getWidget("previewExtensionsEntry")));

	if (name.empty() || app.empty() || ext.empty())
	{
		s->showErrorDialog(_("Must not be empty..."));
		return;
	}

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string oldName = s->previewAppView.getString(&iter, _("Name"));

		if (WulforSettingsManager::getInstance()->applyPreviewApp(oldName, name, app, ext))
		{
			gtk_list_store_set(s->previewAppToStore, &iter,
				s->previewAppView.col(_("Name")), name.c_str(),
				s->previewAppView.col(_("Application")), app.c_str(),
				s->previewAppView.col(_("Extensions")), ext.c_str(),
				-1);
		}
	}*/
}

void Settings::onAddShare_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
// 	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));
//		if (temp)
/*		{
			string path = temp;

			if (path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;

			GtkWidget *dialog = s->getWidget("nameDialog");
			gtk_window_set_title(GTK_WINDOW(dialog), _("Virtual name"));
//			gtk_entry_set_text(GTK_ENTRY(s->getWidget("nameDialogEntry")), Util::getLastDir(path).c_str());
//			gtk_editable_select_region(GTK_EDITABLE(s->getWidget("nameDialogEntry")), 0, -1);
			gtk_label_set_markup(GTK_LABEL(s->getWidget("labelNameDialog")), _("<b>Name under which the others see the directory</b>"));
//			response = gtk_dialog_run(GTK_DIALOG(dialog));
//			string name = gtk_entry_get_text(GTK_ENTRY(s->getWidget("nameDialogEntry")));

//			if (response == GTK_RESPONSE_OK)
			{
				try
				{
//					ShareManager::getInstance()->addDirectory(path, name);
				}
				catch (const ShareException &e)
				{
					s->showErrorDialog(e.getError());
					return;//should not update GUI if any Share* exception hapened
				}
				catch(...){g_print("Some other exception");}

//				s->addShare_gui(path, name);
			}*/
		}
	}
//}

void Settings::selectTextColor_gui(const int select)
{
	GtkTreeIter iter;

	if (select == 2)
	{
		/* black and white style */
		GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

		string style = "";
		GtkTextTag *tag = NULL;
		GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(textStyleBuffer);

		while (valid)
		{
			gtk_list_store_set(textStyleStore, &iter,
				textStyleView.col("ForeColor"), "#000000",
				textStyleView.col("BackColor"), "#FFFFFF", -1);

			style = textStyleView.getString(&iter, _("Style"));
			tag = gtk_text_tag_table_lookup(tag_table, style.c_str());

			if (tag)
				g_object_set(tag, "foreground", "#000000", "background", "#FFFFFF", NULL);

			valid = gtk_tree_model_iter_next(m, &iter);
		}

		gtk_widget_queue_draw(getWidget("textViewPreviewStyles"));

		return;
	}

	GtkTreeSelection *selection = gtk_tree_view_get_selection(textStyleView.get());

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		showErrorDialog(_("selected style failed"));
		return;
	}

	GdkRGBA color;
	string currentcolor = "";
	GtkWidget *dialog = gtk_color_chooser_dialog_new(_("Select Color"),GTK_WINDOW(getContainer()));

	if (select == 0)
		currentcolor = textStyleView.getString(&iter, "ForeColor");
	else
		currentcolor = textStyleView.getString(&iter, "BackColor");

	if (gdk_rgba_parse(&color,currentcolor.c_str()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &color);

//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);

		string ground = "";
		string strcolor = WulforUtil::colorToString(&color);
		string style = textStyleView.getString(&iter, _("Style"));

		if (select == 0)
		{
			ground = "foreground-rgba";
			gtk_list_store_set(textStyleStore, &iter, textStyleView.col("ForeColor"), strcolor.c_str(), -1);
		}
		else
		{
			ground = "background-rgba";
			gtk_list_store_set(textStyleStore, &iter, textStyleView.col("BackColor"), strcolor.c_str(), -1);
		}

		GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textStyleBuffer), style.c_str());

		if (tag)
			g_object_set(tag, ground.c_str(), &color, NULL);

		gtk_widget_queue_draw(getWidget("textViewPreviewStyles"));
	}
}

void Settings::selectTextStyle_gui(const int select)
{
	GtkTreeIter iter;
	int bolt, italic;
	GtkTextTag *tag = NULL;
	string style = "";

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	if (select == 1)
	{
		/* default style */
		GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(textStyleBuffer);
		string fore, back;

		while (valid)
		{
			style = textStyleView.getString(&iter, _("Style"));
			fore = wsm->getString(textStyleView.getString(&iter, "keyForeColor"), true);
			back = wsm->getString(textStyleView.getString(&iter, "keyBackColor"), true);
			bolt = wsm->getInt(textStyleView.getString(&iter, "keyBolt"), true);
			italic = wsm->getInt(textStyleView.getString(&iter, "keyItalic"), true);

			gtk_list_store_set(textStyleStore, &iter,
				textStyleView.col("ForeColor"), fore.c_str(),
				textStyleView.col("BackColor"), back.c_str(),
				textStyleView.col("Bolt"), bolt,
				textStyleView.col("Italic"), italic,
				-1);

			tag = gtk_text_tag_table_lookup(tag_table, style.c_str());

			if (tag)
				g_object_set(tag,
					"foreground", fore.c_str(),
					"background", back.c_str(),
					"weight", bolt ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					NULL);

			valid = gtk_tree_model_iter_next(m, &iter);
		}

		gtk_widget_queue_draw(getWidget("textViewPreviewStyles"));

		return;
	}

	GtkTreeSelection *selection = gtk_tree_view_get_selection(textStyleView.get());

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		showErrorDialog(_("selected style failed"));
		return;
	}
	GtkWidget *dialog = gtk_font_chooser_dialog_new (_("Select Font"),GTK_WINDOW(getContainer()));
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
		g_autofree gchar *temp =  gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));

		if (temp)
		{
			string font_name = temp;

			bolt = font_name.find("Bold") != string::npos ? 1 : 0;
			italic = font_name.find("Italic") != string::npos ? 1 : 0;

			style = textStyleView.getString(&iter, _("Style"));
			gtk_list_store_set(textStyleStore, &iter, textStyleView.col("Bolt"), bolt, textStyleView.col("Italic"), italic, -1);

			tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textStyleBuffer), style.c_str());

			if (tag)
				g_object_set(tag,
					"weight", bolt ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					NULL);
			gtk_widget_queue_draw(getWidget("textViewPreviewStyles"));
		}
	}
}
/*BMDC*/
void Settings::onTextColorForeULClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->setColorUL();
}

void Settings::onBgColorULClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	s->setBgColorUserList();
}

void Settings::setBgColorUserList()
{
	GtkTreeIter iter;
	GtkTreeSelection *selections = gtk_tree_view_get_selection(userListNames.get());

	if (!gtk_tree_selection_get_selected(selections, NULL, &iter))
	{
		showErrorDialog(_("selected color failed"));
		return;
	}


	string currentcolor = "", currname = "";
	GtkWidget *dialog = gtk_color_chooser_dialog_new (_("Color Select"),GTK_WINDOW(getContainer()));
	GdkRGBA color;

	currentcolor = userListNames.getString(&iter, "Back");
	currname = userListNames.getString(&iter, _("Name"));
	if(gdk_rgba_parse(&color,currentcolor.c_str()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&color);

//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

//	if (response == GTK_RESPONSE_OK)
	{
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);

		string strcolor = WulforUtil::colorToString(&color);

		gtk_list_store_set(userListStore1, &iter, userListNames.col("BackSet"), strcolor.c_str(), -1);
		if(currname.find(_("Normal")) != string::npos)
			WSET("userlist-bg-normal",strcolor);
		else if(currname.find(_("Operator")) != string::npos)
			WSET("userlist-bg-operator",strcolor);
		else if (currname.find(_("Bot")) != string::npos)
			WSET("userlist-bg-bot-hub",strcolor);
		else if (currname.find(_("Protect")) != string::npos)
			WSET("userlist-bg-protected",strcolor);
		else if (currname.find(_("Passive")) != string::npos)
			WSET("userlist-bg-pasive",strcolor);
		else if (currname.find(_("Favorite")) != string::npos)
			WSET("userlist-bg-favorite",strcolor);
		else if (currname.find(_("Ignored")) != string::npos)
			WSET("userlist-bg-ignored",strcolor);
		else
			dcdebug("dont go here");

		WulforSettingsManager::getInstance()->save();
		setColorRow(_("Name"));
	}
}

void Settings::onTextColorDefaultULClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(s->userListStore1), &iter);

	if (!valid)
	{
		s->showErrorDialog(_("selected name failed"));
		return;
	}

	s->setDefaultColor("#000000", _("Normal"), &iter);
	valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->userListStore1), &iter);
	if(valid) {
		s->setDefaultColor("#1E90FF", _("Operator"), &iter);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->userListStore1), &iter);
	}
	if(valid) {
		s->setDefaultColor("#747677", _("Passive"), &iter);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->userListStore1), &iter);
	}
	if(valid) {
		s->setDefaultColor("#FF0000", _("Favorite"), &iter);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->userListStore1), &iter);
	}
	if(valid)
	{
		s->setDefaultColor("#8B6914", _("Protected"), &iter);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->userListStore1), &iter);
	}
	if(valid)
	{
		s->setDefaultColor("#9AFFAF", _("Ignored"), &iter);
	}
}

void Settings::setColorUL()
{
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(userListNames.get());

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		showErrorDialog(_("selected color failed"));
		return;
	}
	GtkWidget *dialog = gtk_color_chooser_dialog_new (_("Color Select"),GTK_WINDOW(getContainer()));
	GdkRGBA color;

	string currentcolor = string(), currname = string();
	currentcolor = userListNames.getString(&iter, "Color");
	currname = userListNames.getString(&iter, _("Name"));
	
	if( gdk_rgba_parse(&color,currentcolor.c_str() ))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&color);
	
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

//	if (response == GTK_RESPONSE_OK)
	{
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&color);
		string strcolor = WulforUtil::colorToString(&color);

		UnMapIter::const_iterator qp = colorsIters.find(_("User ") + currname);
		GtkTreeIter qiter = qp->second;

		gtk_list_store_set(userListStore2, &qiter, userListPreview.col("Color"), strcolor.c_str(), -1);
		gtk_list_store_set(userListStore1, &iter, userListNames.col("Color"), strcolor.c_str(), -1);

	}
}

void Settings::setDefaultColor(string color, string name, GtkTreeIter *iter)
{
	UnMapIter::const_iterator qp = colorsIters.find(_("User ") + name);
	GtkTreeIter qiter = qp->second;

	gtk_list_store_set(userListStore2, &qiter, userListPreview.col("Color"), color.c_str(), -1);
	gtk_list_store_set(userListStore1, iter, userListNames.col("Color"), color.c_str(), -1);

}

void Settings::loadUserCommands_gui()
{
	GtkTreeIter iter;
	gtk_list_store_clear(userCommandStore);

	UserCommand::List userCommands = FavoriteManager::getInstance()->getUserCommands();

	for (auto i = userCommands.begin(); i != userCommands.end(); ++i)
	{
		UserCommand &uc = *i;
		if (!uc.isSet(UserCommand::FLAG_NOSAVE))
		{
			gtk_list_store_append(userCommandStore, &iter);
			gtk_list_store_set(userCommandStore, &iter,
				userCommandView.col(_("Name")), uc.getName().c_str(),
				userCommandView.col(_("Hub")), uc.getHub().c_str(),
				userCommandView.col(_("Command")), uc.getCommand().c_str(),
				-1);
		}
	}
}

void Settings::saveUserCommand(UserCommand *uc)
{
	string name, command, hub;
	int ctx = 0;
	int type = 0;
	GtkTreeIter iter;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogHubMenu"))))
		ctx |= UserCommand::CONTEXT_HUB;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogUserMenu"))))
		ctx |= UserCommand::CONTEXT_USER;//NOTE: core 0.762
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogSearchMenu"))))
		ctx |= UserCommand::CONTEXT_SEARCH;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogFilelistMenu"))))
		ctx |= UserCommand::CONTEXT_FILELIST;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogIpMenu"))))
		ctx |= UserCommand::CONTEXT_IP;

	string to = Util::emptyString;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogSeparator"))))
	{
		name = _("Separator");
		type = UserCommand::TYPE_SEPARATOR;
	}
	else
	{
//		name = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogName")));
//		command = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogCommand")));
//		hub = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogHub")));

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogChat"))))
		{
	//		command = "<%[myNI]> " + NmdcHub::validateMessage(command, FALSE) + "|";
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogPM"))))
		{
//			to = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogTo")));
//			if (to.length() == 0)
//				to = "%[userNI]";

//			command = "$To: " + to + " From: %[myNI] $<%[myNI]> " + NmdcHub::validateMessage(command, FALSE) + "|";
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogOnce"))))
			type = UserCommand::TYPE_RAW_ONCE;
		else
			type = UserCommand::TYPE_RAW;
	}

	if (uc == NULL)
	{
		FavoriteManager::getInstance()->addUserCommand(type, ctx, 0, name, command, to, hub);//NOTE: core 0.762
		gtk_list_store_append(userCommandStore, &iter);
	}
	else
	{
		uc->setType(type);
		uc->setCtx(ctx);
		uc->setName(name);
		uc->setCommand(command);
		uc->setHub(hub);
		FavoriteManager::getInstance()->updateUserCommand(*uc);

		GtkTreeSelection *selection = gtk_tree_view_get_selection(userCommandView.get());
		if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
			return;
	}

	gtk_list_store_set(userCommandStore, &iter,
		userCommandView.col(_("Name")), name.c_str(),
		userCommandView.col(_("Hub")), hub.c_str(),
		userCommandView.col(_("Command")), command.c_str(),
		-1);
}

void Settings::updateUserCommandTextSent_gui()
{
	string command = gtk_editable_get_text(GTK_EDITABLE(getWidget("commandDialogCommand")));

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogSeparator"))))
	{
		command.clear();
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogChat"))))
	{
		command = "<%[myNI]> " + NmdcHub::validateMessage(command, FALSE) + "|";
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogPM"))))
	{
	//	string to = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogTo")));
	//	if (to.length() == 0)
	//		to = "%[userNI]";
	//	command = "$To: " + to + " From: %[myNI] $<%[myNI]> " + NmdcHub::validateMessage(command, FALSE) + "|";
	}

	gtk_editable_set_text(GTK_EDITABLE(getWidget("commandDialogTextSent")), command.c_str());
}

bool Settings::validateUserCommandInput(const string &oldName)
{
	/*
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("commandDialogSeparator"))))
	{
		string name = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogName")));
		string command = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogCommand")));
		string hub = gtk_entry_get_text(GTK_ENTRY(getWidget("commandDialogHub")));

		if (name.length() == 0 || command.length() == 0)
		{
			showErrorDialog(_("Name and command must not be empty"));
			return false;
		}

		if (name != oldName && FavoriteManager::getInstance()->findUserCommand(name, hub) != -1)
		{
			showErrorDialog(_("Command name already exists"));
			return false;
		}
	}
*/
	return true;
}

void Settings::showErrorDialog(const string error)
{
	GtkWidget *errorDialog = gtk_message_dialog_new(GTK_WINDOW(getWidget("dialog")),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error.c_str());
	gtk_window_set_modal(GTK_WINDOW(errorDialog), TRUE);
	//g_signal_connect(errorDialog, "response", G_CALLBACK(gtk_widget_destroy), errorDialog);
}

void Settings::onOptionsViewToggled_gui(GtkCellRendererToggle*, gchar *path, gpointer data)
{
	GtkTreeIter iter;
	GtkListStore *store = (GtkListStore *)data;
	GtkTreeModel *model = GTK_TREE_MODEL(store);

	if (gtk_tree_model_get_iter_from_string(model, &iter, path))
	{
		gboolean fixed;
		gtk_tree_model_get(model, &iter, 0, &fixed, -1);
		gtk_list_store_set(store, &iter, 0, !fixed, -1);
	}
}

void Settings::onInDirect_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryipv6"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("entryIpExt"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("ipLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tcpEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tcpLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("udpEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("udpLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tlsEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tlsLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("forceIPCheckButton"), TRUE);
}

void Settings::onInFW_NAT_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryipv6"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("entryIpExt"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("ipLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tcpEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tcpLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("udpEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("udpLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tlsEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tlsLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("forceIPCheckButton"), TRUE);
}

void Settings::onInPassive_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryipv6"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("entryIpExt"), FALSE);//ipEntry
	gtk_widget_set_sensitive(s->getWidget("ipLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("tcpEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("tcpLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("udpEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("udpLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("tlsEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("tlsLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("forceIPCheckButton"), FALSE);
}

void Settings::onOutDirect_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("socksIPEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksIPLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksUserEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksUserLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksPortEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksPortLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksPassEntry"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksPassLabel"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("socksCheckButton"), FALSE);
}

void Settings::onSocks5_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("socksIPEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksIPLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksUserEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksUserLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksPortEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksPortLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksPassEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksPassLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("socksCheckButton"), TRUE);
}

void Settings::onBrowseFinished_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

// 	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));
//		if (path)
		{
//			gtk_entry_set_text(GTK_ENTRY(s->getWidget("finishedDownloadsEntry")), Text::toUtf8(path).c_str());
		}
	}
}

void Settings::onBrowseUnfinished_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

// 	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));
//		if (path)
		{
	//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("unfinishedDownloadsEntry")),  Text::toUtf8(path).c_str());
		}
	}
}

void Settings::onPublicHubs_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	gtk_list_store_clear(s->publicListStore);
	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for (auto idx = lists.begin(); idx != lists.end(); ++idx)
	{
		gtk_list_store_append(s->publicListStore, &iter);
		gtk_list_store_set(s->publicListStore, &iter, s->publicListView.col("List"), (*idx).c_str(), -1);
	}

//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("publicHubsDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
		string lists = string();
		GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		while (valid)
		{
			lists += s->publicListView.getString(&iter, "List") + ";";
			valid = gtk_tree_model_iter_next(m, &iter);
		}
		if (!lists.empty())
			lists.erase(lists.size() - 1);
		SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, lists);
	}
}

void Settings::onPublicAdd_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;

	gtk_list_store_append(s->publicListStore, &iter);
	gtk_list_store_set(s->publicListStore, &iter, s->publicListView.col("List"), _("New list"), -1);
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(s->publicListStore), &iter);
	col = gtk_tree_view_get_column(s->publicListView.get(), 0);
	gtk_tree_view_set_cursor(s->publicListView.get(), path, col, TRUE);
	gtk_tree_path_free(path);
}

void Settings::onPublicMoveUp_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->publicListView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
			gtk_list_store_swap(s->publicListStore, &current, &prev);
		gtk_tree_path_free(path);
	}
}

void Settings::onPublicMoveDown_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter current, next;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->publicListView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->publicListStore), &next))
			gtk_list_store_swap(s->publicListStore, &current, &next);
	}
}

void Settings::onPublicEdit_gui(GtkCellRendererText*, char *path, char *text, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->publicListStore), &iter, path))
		gtk_list_store_set(s->publicListStore, &iter, 0, text, -1);
}

void Settings::onPublicRemove_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicListView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		gtk_list_store_remove(s->publicListStore, &iter);
}

void Settings::onAddFavorite_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));
/*
	if (response == GTK_RESPONSE_OK)
	{
		g_autofree gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));
		if (temp)
		{
			string path = Text::toUtf8(temp);
			GtkWidget *dialog = s->getWidget("nameDialog");
			gtk_window_set_title(GTK_WINDOW(dialog), _("Favorite name"));
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("nameDialogEntry")), "");
			gtk_label_set_markup(GTK_LABEL(s->getWidget("labelNameDialog")), _("<b>Under what name you see the directory</b>"));
			response = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_hide(dialog);

			if (response == GTK_RESPONSE_OK)
			{
//				string name = gtk_entry_get_text(GTK_ENTRY(s->getWidget("nameDialogEntry")));
	//			if (path[path.length() - 1] != PATH_SEPARATOR)
					path += PATH_SEPARATOR;

				if (!name.empty() && FavoriteManager::getInstance()->addFavoriteDir(path, name))
				{
					GtkTreeIter iter;
					gtk_list_store_append(s->downloadToStore, &iter);
					gtk_list_store_set(s->downloadToStore, &iter,
						s->downloadToView.col(_("Favorite Name")), name.c_str(),
						s->downloadToView.col(_("Directory")), path.c_str(),
						-1);
				}
				else
				{
					s->showErrorDialog(_("Directory or favorite name already exists"));
				}
			}
		}
	}*/
}

void Settings::onRemoveFavorite_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string path = s->downloadToView.getString(&iter, _("Directory"));
		if (FavoriteManager::getInstance()->removeFavoriteDir(path))
		{
			gtk_list_store_remove(s->downloadToStore, &iter);
			gtk_widget_set_sensitive(s->getWidget("favoriteRemoveButton"), FALSE);
		}
	}
}
/*
gboolean Settings::onFavoriteButtonReleased_gui(GtkWidget*, GdkEventButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		gtk_widget_set_sensitive(s->getWidget("favoriteRemoveButton"), TRUE);
	else
		gtk_widget_set_sensitive(s->getWidget("favoriteRemoveButton"), FALSE);

	return FALSE;
}
*/
void Settings::addShare_gui(string path, string name)
{
	int64_t size = ShareManager::getInstance()->getShareSize(path);
	GtkTreeIter iter;
	gtk_list_store_append(shareStore, &iter);
	gtk_list_store_set(shareStore, &iter,
		shareView.col(_("Virtual Name")), name.c_str(),
		shareView.col(_("Directory")), path.c_str(),
		shareView.col(_("Size")), Util::formatBytes(size).c_str(),
		shareView.col("Real Size"), size,
		-1);
}

void Settings::onRemoveShare_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string path = s->shareView.getString(&iter, _("Directory"));
		gtk_list_store_remove(s->shareStore, &iter);
		gtk_widget_set_sensitive(s->getWidget("sharedRemoveButton"), FALSE);

		ShareManager::getInstance()->removeDirectory(path);
	}
}
/*
gboolean Settings::onShareButtonReleased_gui(GtkWidget*, GdkEventButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (gtk_tree_selection_count_selected_rows(selection) == 0)
		gtk_widget_set_sensitive(s->getWidget("sharedRemoveButton"), FALSE);
	else
		gtk_widget_set_sensitive(s->getWidget("sharedRemoveButton"), TRUE);

	return FALSE;
}
*/
gboolean Settings::onShareHiddenPressed_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;

	bool show = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("shareHiddenCheckButton")));

	Func1<Settings, bool> *func = new Func1<Settings, bool>(s, &Settings::shareHidden_client, show);
	WulforManager::get()->dispatchClientFunc(func);

	return FALSE;
}

void Settings::updateShares_gui()
{
	GtkTreeIter iter;
	int64_t size = 0;
	string vname;

	gtk_list_store_clear(shareStore);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it)
	{
		size = ShareManager::getInstance()->getShareSize(it->second);

		if (size == -1 && !SETTING(SHARE_HIDDEN))
		{
			vname = _("[HIDDEN SHARE] ") + it->first;
			size = 0;
		} else
			vname = it->first;

		gtk_list_store_append(shareStore, &iter);
		gtk_list_store_set(shareStore, &iter,
			shareView.col(_("Virtual Name")), vname.c_str(),
			shareView.col(_("Directory")), it->second.c_str(),
			shareView.col(_("Size")), Util::formatBytes(size).c_str(),
			shareView.col("Real Size"), size,
			-1);
	}

	string text = _("Total size: ") + Util::formatBytes(ShareManager::getInstance()->getShareSize());
	gtk_label_set_text(GTK_LABEL(getWidget("sharedSizeLabel")), text.c_str());
}

void Settings::onLogBrowseClicked_gui(GtkWidget*, gpointer data)
{
/*	Settings *s = (Settings *)data;

 	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

	if (response == GTK_RESPONSE_OK)
	{
		g_autofree gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));
		if (path)
		{
			gtk_entry_set_text(GTK_ENTRY(s->getWidget("logDirectoryEntry")), Text::toUtf8(path).c_str());
		}
	}*/
}

void Settings::onLogMainClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->getWidget("logMainLabel"), toggled);
	gtk_widget_set_sensitive(s->getWidget("logMainEntry"), toggled);
}

void Settings::onLogPrivateClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->getWidget("logPrivateLabel"), toggled);
	gtk_widget_set_sensitive(s->getWidget("logPrivateEntry"), toggled);
}

void Settings::onLogDownloadClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->getWidget("logDownloadsLabel"), toggled);
	gtk_widget_set_sensitive(s->getWidget("logDownloadsEntry"), toggled);
}

void Settings::onLogUploadClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->getWidget("logUploadsLabel"), toggled);
	gtk_widget_set_sensitive(s->getWidget("logUploadsEntry"), toggled);
}
//BMDC
void Settings::onRawsClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->getWidget("entryraws"),toggled);
}
//BMDC
void Settings::onUserCommandAdd_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	// Reset dialog to default
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogSeparator")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogHubMenu")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogUserMenu")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogSearchMenu")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogFilelistMenu")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogOnce")), FALSE);
//	gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogName")), "");
//	gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogCommand")), "");
//	gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogHub")), "");
//	gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogTo")), "");
	gtk_widget_set_sensitive(s->getWidget("commandDialogName"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("commandDialogCommand"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("commandDialogHub"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("commandDialogTo"), FALSE);
	gtk_widget_set_sensitive(s->getWidget("commandDialogOnce"), FALSE);
/*
	gint response;

	do
	{
		response = gtk_dialog_run(GTK_DIALOG(s->getWidget("commandDialog")));
	}
	while (response == GTK_RESPONSE_OK && !s->validateUserCommandInput());

*/
//	if (response == GTK_RESPONSE_OK)
//		s->saveUserCommand(NULL);
}

void Settings::onUserCommandEdit_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->userCommandView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string name = s->userCommandView.getString(&iter, _("Name"));
		string hubStr = s->userCommandView.getString(&iter, _("Hub"));
		int cid = FavoriteManager::getInstance()->findUserCommand(name, hubStr);
		if (cid < 0)
			return;

		UserCommand uc;
		string command, nick;
		FavoriteManager::getInstance()->getUserCommand(cid, uc);
		bool hub = uc.getCtx() & UserCommand::CONTEXT_HUB;
		bool user = uc.getCtx() & UserCommand::CONTEXT_USER;//NOTE: core 0.762
		bool search = uc.getCtx() & UserCommand::CONTEXT_SEARCH;
		bool filelist = uc.getCtx() & UserCommand::CONTEXT_FILELIST;
		bool ipItem = uc.getCtx() & UserCommand::CONTEXT_IP;

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogHubMenu")), hub);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogUserMenu")), user);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogSearchMenu")), search);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogFilelistMenu")), filelist);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogIpMenu")), ipItem);

		if (uc.getType() == UserCommand::TYPE_SEPARATOR)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogSeparator")), TRUE);
		}
		else
		{
			command = uc.getCommand();
			bool once = uc.getType() == UserCommand::TYPE_RAW_ONCE;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogOnce")), once);

			// Chat Command
			if ((strncmp(command.c_str(), "<%[mynick]> ", 12) == 0 ||
				strncmp(command.c_str(), "<%[myNI]> ", 10) == 0) &&
				command.find('|') == command.length() - 1)
			{
				string::size_type i = command.find('>') + 2;
				command = NmdcHub::validateMessage(command.substr(i, command.length() - i - 1), TRUE);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogChat")), TRUE);
			}
			// PM command
			else if (strncmp(command.c_str(), "$To: ", 5) == 0 && (
				command.find(" From: %[myNI] $<%[myNI]> ") != string::npos ||
				command.find(" From: %[mynick] $<%[mynick]> ") != string::npos) &&
				command.find('|') == command.length() - 1)
			{
				string::size_type i = command.find(' ', 5);
				nick = command.substr(5, i - 5);
				i = command.find('>', 5) + 2;
				command = NmdcHub::validateMessage(command.substr(i, command.length() - i - 1), FALSE);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogPM")), TRUE);
			}
			// Raw command
			else
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("commandDialogRaw")), TRUE);
			}
		}

//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogName")), name.c_str());
//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogCommand")), command.c_str());
//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogHub")), uc.getHub().c_str());
//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("commandDialogTo")), nick.c_str());

		s->updateUserCommandTextSent_gui();

		gint response;

//		do
		{
//			response = gtk_dialog_run(GTK_DIALOG(s->getWidget("commandDialog")));
		}
//		while (response == GTK_RESPONSE_OK && !s->validateUserCommandInput(name));

		gtk_widget_hide(s->getWidget("commandDialog"));

		if (response == GTK_RESPONSE_OK)
			s->saveUserCommand(&uc);
	}
}

void Settings::onUserCommandMoveUp_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(s->userCommandStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->userCommandView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &current))
	{
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
		{
			string name = s->userCommandView.getString(&current, _("Name"));
			string hub= s->userCommandView.getString(&current, _("Hub"));
			gtk_list_store_swap(s->userCommandStore, &current, &prev);

			typedef Func3<Settings, string, string, int> F3;
			F3 *func = new F3(s, &Settings::moveUserCommand_client, name, hub, -1);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
}

void Settings::onUserCommandMoveDown_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter current, next;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->userCommandView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &current))
	{
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->userCommandStore), &next))
		{
			string name = s->userCommandView.getString(&current, _("Name"));
			string hub = s->userCommandView.getString(&current, _("Hub"));
			gtk_list_store_swap(s->userCommandStore, &current, &next);

			typedef Func3<Settings, string, string, int> F3;
			F3 *func = new F3(s, &Settings::moveUserCommand_client, name, hub, 1);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void Settings::onUserCommandRemove_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->userCommandView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string name = s->userCommandView.getString(&iter, _("Name"));
		string hub = s->userCommandView.getString(&iter, _("Hub"));
		gtk_list_store_remove(s->userCommandStore, &iter);

		typedef Func2<Settings, string, string> F2;
		F2 *func = new F2(s, &Settings::removeUserCommand_client, name, hub);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Settings::onUserCommandTypeSeparator_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gtk_widget_set_sensitive(s->getWidget("commandDialogName"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogCommand"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogHub"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogTo"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogOnce"), FALSE);

		s->updateUserCommandTextSent_gui();
	}
}

void Settings::onUserCommandTypeRaw_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gtk_widget_set_sensitive(s->getWidget("commandDialogName"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogCommand"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogHub"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogTo"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogOnce"), TRUE);

		s->updateUserCommandTextSent_gui();
	}
}

void Settings::onUserCommandTypeChat_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gtk_widget_set_sensitive(s->getWidget("commandDialogName"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogCommand"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogHub"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogTo"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogOnce"), TRUE);

		s->updateUserCommandTextSent_gui();
	}
}

void Settings::onUserCommandTypePM_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gtk_widget_set_sensitive(s->getWidget("commandDialogName"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogCommand"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogHub"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogTo"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("commandDialogOnce"), TRUE);

		s->updateUserCommandTextSent_gui();
	}
}

/*gboolean Settings::onUserCommandKeyPress_gui(GtkWidget*, GdkEventKey*, gpointer data)
{
	Settings *s = (Settings *)data;

	s->updateUserCommandTextSent_gui();

	return FALSE;
}
*/
void Settings::onCertificatesPrivateBrowseClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

//	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_OPEN);
 //	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));
//		if (path)
		{
//			gtk_entry_set_text(GTK_ENTRY(s->getWidget("privateKeyEntry")), Text::toUtf8(path).c_str());
		}
	}
}

void Settings::onCertificatesFileBrowseClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

//	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_OPEN);
 //	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));
//		if (path)
		{
//			gtk_entry_set_text(GTK_ENTRY(s->getWidget("certificateFileEntry")), Text::toUtf8(path).c_str());
		}
	}
}

void Settings::onCertificatesPathBrowseClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

// 	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));
//		if (path)
		{
//			gtk_entry_set_text(GTK_ENTRY(s->getWidget("trustedCertificatesPathEntry")), Text::toUtf8(path).c_str());
		}
	}
}

void Settings::onGenerateCertificatesClicked_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	Func0<Settings> *func = new Func0<Settings>(s, &Settings::generateCertificates_client);
	WulforManager::get()->dispatchClientFunc(func);
}

void Settings::shareHidden_client(bool show)
{
	SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, show);
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(TRUE, FALSE, FALSE);//3TRUE

	//NOTE: updated share ui core 0.762
	Func0<Settings> *func = new Func0<Settings>(this, &Settings::updateShares_gui);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Settings::addShare_client(string path, string name)
{
	try
	{
		ShareManager::getInstance()->addDirectory(path, name);
	}
	catch (const ShareException &e)
	{
		typedef Func1<Settings, const string> F1;
		F1 *func = new F1(this, &Settings::showErrorDialog, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}

	typedef Func2<Settings, string, string> F3;
	F3 *func = new F3(this, &Settings::addShare_gui, path, name);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Settings::removeUserCommand_client(string name, string hub)
{
	if (!name.empty())
	{
		FavoriteManager *fm = FavoriteManager::getInstance();
		fm->removeUserCommand(fm->findUserCommand(name, hub));
	}
}

void Settings::moveUserCommand_client(string name, string hub, int pos)
{
	if (!name.empty())
	{
		FavoriteManager *fm = FavoriteManager::getInstance();
		fm->moveUserCommand(fm->findUserCommand(name, hub), pos);
	}
}

void Settings::generateCertificates_client()
{
	try
	{
		CryptoManager::getInstance()->generateCertificate();
	}
	catch (const CryptoException &e)
	{

	}
}

void Settings::onInFW_UPnP_gui(GtkToggleButton*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryIpExt"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("ipLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tcpEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tcpLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("udpEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("udpLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tlsEntry"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("tlsLabel"), TRUE);
	gtk_widget_set_sensitive(s->getWidget("forceIPCheckButton"), TRUE);
}

void Settings::onPictureShare_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	string name = "MAGNET-IMAGE";
	string path = Util::getPath(Util::PATH_USER_CONFIG) + "Images/";
	typedef Func2<Settings, string, string> F2;
	F2 *func = new F2(s, &Settings::addShare_client, path, name);
	WulforManager::get()->dispatchClientFunc(func);
}

//BMDC++
void Settings::onAddHighlighting_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	//GtkWidget *dialog = s->getWidget("HighlightingDialog");
	//gtk_window_set_title(GTK_WINDOW(dialog), _("New Highlighting Item"));
	gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGString")), "Example");
	gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGColorText")), "#000000");
	gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGColorBack")), "#FFFFFF");
	gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGSoundFile")), "Example.wav");
	gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGPopup")), "Popup Text");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGbold")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGundr")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGItal")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGuser")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGsens")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHgBoldTab")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGSound")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGNotify")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGColor")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGText")), TRUE);

	//gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	//if (response == GTK_RESPONSE_OK)
	{
		dcpp::StringMap params;
		params["String"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGString")));
		params["Color Text"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGColorText")));
		params["Color Back"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGColorBack")));
		params["Sound File"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGSoundFile")));
		params["Popup Text"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGPopup")));
		params["Bold"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGbold"))) ? "1" : "0";
		params["Underline"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGundr"))) ? "1" : "0";
		params["Italic"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGItal"))) ? "1" : "0";
		params["User"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGuser"))) ? "1" : "0";
		params["Sensitive"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGsens"))) ? "1" : "0";
		params["Tab"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHgBoldTab"))) ? "1" : "0";
		params["Enable Popup"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGNotify"))) ? "1" : "0";
		params["Enable Sound"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGSound"))) ? "1" : "0";
		params["Enable Background"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGColor"))) ? "1" : "0";
		params["Enable Text"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGText"))) ? "1" : "0";

		s->saveHighlighting(params,true);
	}
}

void Settings::onEditHighlighting_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(s->selection, NULL, &iter))
	{
		dcpp::StringMap params;
		string name = params["String"] = s->hView.getString(&iter, _("String"));
		params["Color Text"] = s->hView.getString(&iter, "Fore Color");
		params["Color Back"] = s->hView.getString(&iter, "Back Color");
		params["Sound File"] = s->hView.getString(&iter, "Sound File");
		params["Popup Text"] = s->hView.getString(&iter, "Popup String");
		params["Bold"] = s->hView.getString(&iter, "Bold");
		params["Italic"] = s->hView.getString(&iter, "Italic");
		params["Underline"] = s->hView.getString(&iter, "Underline");
		params["User"] = s->hView.getString(&iter, "Include Nick");
		params["Sensitive"] = s->hView.getString(&iter, "Case sensitive");
		params["Tab"] = s->hView.getString(&iter, "Bold Tab");
		params["Enable Popup"] = s->hView.getString(&iter, "Popup Enable");
		params["Enable Sound"] = s->hView.getString(&iter, "Enable Sound");
		params["Enable Background"] = s->hView.getString(&iter, "Enable Back Color");
		params["Enable Text"] = s->hView.getString(&iter, "Enable Fore Color");

		GtkWidget *dialog = s->getWidget("HighlightingDialog");
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Highlighting Item"));
		gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGString")), params["String"].c_str());
		gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGColorText")), params["Color Text"].c_str());
		gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGColorBack")), params["Color Back"].c_str());
		gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGSoundFile")), params["Sound File"].c_str());
		gtk_editable_set_text(GTK_EDITABLE(s->getWidget("entryHGPopup")), params["Popup Text"].c_str());
		gboolean isBold = params["Bold"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGbold")), isBold);
		gboolean isUnderline = params["Underline"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGundr")), isUnderline);
		gboolean isItalic = params["Italic"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGItal")), isItalic);
		gboolean isUser = params["User"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGuser")), isUser);
		gboolean isSensitive = params["Sensitive"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGsens")), isSensitive);
		gboolean isBoldTab = params["Tab"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHgBoldTab")), isBoldTab);
		gboolean isSound = params["Enable Sound"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGSound")), isSound);
		gboolean isNotify = params["Enable Popup"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGNotify")), isNotify);
		gboolean isColor = params["Enable Background"] ==  "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGColor")), isColor);
		gboolean isText = params["Enable Text"] == "1" ? TRUE : FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGText")), isText);

//		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//		if (response == GTK_RESPONSE_OK)
		{
            dcpp::StringMap params;
			params["String"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGString")));
			params["Color Text"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGColorText")));
			params["Color Back"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGColorBack")));
			params["Sound File"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGSoundFile")));
			params["Popup Text"] = gtk_editable_get_text(GTK_EDITABLE(s->getWidget("entryHGPopup")));
			params["Bold"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGbold"))) ? "1" : "0";
			params["Underline"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGundr"))) ? "1" : "0";
			params["Italic"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGItal"))) ? "1" : "0";
			params["User"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGuser"))) ? "1" : "0";
			params["Sensitive"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGsens"))) ? "1" : "0";
			params["Tab"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHgBoldTab"))) ? "1" : "0";
			params["Enable Popup"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonHGNotify"))) ? "1" : "0";
			params["Enable Sound"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGSound"))) ? "1" : "0";
			params["Enable Background"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGColor"))) ? "1" : "0";
			params["Enable Text"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkHGText"))) ? "1" : "0";

			s->saveHighlighting(params,false,name);
		}

	}
}

void Settings::onRemoveHighlighting_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(s->selection, NULL, &iter))
	{
		if (true)//todo settings
		{
			string name = s->hView.getString(&iter, _("String"));
			GtkWindow* parent = GTK_WINDOW(s->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete Highlighting \"%s\"?"), name.c_str());

			gtk_dialog_add_buttons(GTK_DIALOG(dialog), "_Cancel", GTK_RESPONSE_CANCEL, "_Yes", GTK_RESPONSE_YES, NULL);

		//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		//	if (response != GTK_RESPONSE_YES)
		//		return;
		}

		string name = s->hView.getString(&iter,_("String"));
		for(unsigned int i=0;i < s->pList.size() ;i++)
		{
			if(s->pList[i].getMatch() == name)
				s->pList.erase(s->pList.begin() + i);
		}
		gtk_list_store_remove(s->hStore,&iter);
	}
}

void Settings::onColorText_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
//	GtkWidget *dialog = gtk_color_chooser_dialog_new (_("Color Select"),GTK_WINDOW(s->getContainer()));
//	GdkRGBA color;
//	if(gdk_rgba_parse(&color,gtk_entry_get_text(GTK_ENTRY(s->getWidget("entryHGColorBack")))))
//		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&color);

//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
//		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&color);
//		string strcolor = WulforUtil::colorToString(&color);
//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryHGColorText")), strcolor.c_str());
	}
}

void Settings::onColorBack_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
//	GtkWidget *dialog = gtk_color_chooser_dialog_new (_("Color Select"),GTK_WINDOW(s->getContainer()));
//	GdkRGBA color;
//	if(gdk_rgba_parse(&color,gtk_entry_get_text(GTK_ENTRY(s->getWidget("entryHGColorBack")))))
//		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&color);

//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
	{
//		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&color);
//		string strcolor = WulforUtil::colorToString(&color);
//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryHGColorBack")), strcolor.c_str());
	}
}

void Settings::onSound_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;

//	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_OPEN);
//	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("fileChooserDialog")));

//	if (response == GTK_RESPONSE_OK)
	{
//		g_autofree gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(s->getWidget("fileChooserDialog")));
//		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryHGSoundFile")), path);
	}
}
/**/
void Settings::onToggledHGText_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryHGColorText"), s->isSensitiveHG[0]);
	gtk_widget_set_sensitive(s->getWidget("buttonColorText"), s->isSensitiveHG[0]);
	s->isSensitiveHG[0] = !s->isSensitiveHG[0];
}

void Settings::onToggledHGColor_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryHGColorBack"), s->isSensitiveHG[1]);
	gtk_widget_set_sensitive(s->getWidget("buttonBackground"), s->isSensitiveHG[1]);
	s->isSensitiveHG[1] = !s->isSensitiveHG[1];
}

void Settings::onToggledHGSound_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryHGSoundFile"), s->isSensitiveHG[2]);
	gtk_widget_set_sensitive(s->getWidget("buttonSound"), s->isSensitiveHG[2]);
	s->isSensitiveHG[2] = !s->isSensitiveHG[2];
}

void Settings::onToggledHGNotify_gui(GtkWidget*, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->getWidget("entryHGPopup"), s->isSensitiveHG[3]);
	s->isSensitiveHG[3] = !s->isSensitiveHG[3];
}
/**/
void Settings::saveHighlighting(dcpp::StringMap &params, bool add, const string&/*""*/)
{
	ColorSettings cs;

	cs.setMatch(params["String"]);
	cs.setPopup(Util::toInt(params["Enable Popup"]));
	cs.setBold(Util::toInt(params["Bold"]));
	cs.setUnderline(Util::toInt(params["Underline"]));
	cs.setItalic(Util::toInt(params["Italic"]));
	cs.setIncludeNick(Util::toInt(params["User"]));
	cs.setCaseSensitive(Util::toInt(params["Sensitive"]));
	cs.setTab(Util::toInt(params["Tab"]));
	cs.setNoti(params["Popup Text"]);
	cs.setPlaySound(Util::toInt(params["Enable Sound"]));
	cs.setSoundFile(params["Sound File"]);

	cs.setHasBgColor(Util::toInt(params["Enable Background"]));
	cs.setBgColor(params["Color Back"]);

	cs.setHasFgColor(Util::toInt(params["Enable Text"]));
	cs.setFgColor(params["Color Text"]);
	
    if(add)
	{
		pList.push_back(cs);
		addHighlighting_to_gui(cs,true);

	}
	else
	{
		for(unsigned int it = 0;it < pList.size();it++)
		{
			pList[it] = cs;
		}

		addHighlighting_to_gui(cs, false);
	}
}

void Settings::addHighlighting_to_gui(ColorSettings &cs, bool add)
{
	GtkTreeIter iter;

	if(add)
		gtk_list_store_append(hStore,&iter);

	gtk_list_store_set(hStore,&iter,
				hView.col(_("String")), cs.getMatch().c_str(),
				hView.col("Bold"), cs.getBold() ? "1" : "0",
				hView.col("Underline"), cs.getUnderline() ? "1" : "0",
				hView.col("Italic"), cs.getItalic() ? "1" : "0",
				hView.col("Bold Tab"), cs.getTab() ? "1" : "0",
				hView.col("Include Nick"), cs.getIncludeNick() ? "1" : "0",
				hView.col("Popup Enable"), cs.getPopup() ? "1" : "0",
				hView.col("Popup String"), cs.getNoti().c_str(),
				hView.col("Enable Sound"), cs.getPlaySound() ? "1" : "0",
				hView.col("Sound File"), cs.getSoundFile().c_str(),
				hView.col("Enable Fore Color"), cs.getHasFgColor() ? "1" : "0",
				hView.col("Fore Color"), cs.getFgColor().c_str(),
				hView.col("Enable Back Color"), cs.getHasBgColor() ? "1" : "0",
				hView.col("Back Color"), cs.getBgColor().c_str(),
				hView.col("Case sensitive"), cs.usingRegexp() ? "1" : "0",
				-1);

}

//tab

void Settings::onChangeTabSelections(GtkTreeSelection *selection, gpointer data)
{
	Settings *s = (Settings*)data;
	s->changeTab(selection);
}

void Settings::changeTab(GtkTreeSelection *selection) {
        GtkTreeIter iter;
        GtkTreeModel *model;
        g_autofree gchar *key = NULL;

        if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
                gtk_tree_model_get (model, &iter, 1, &key, -1);
                string keys = string();
                string fg_string = string();
                if(key)
				{
					keys = (string(key));
					fg_string = WGETS("colored-tabs-" +(string(key))+"-color-bg");
				}

				GdkRGBA fg;

				gdk_rgba_parse(&fg,fg_string.c_str());
				
				//TODO@ more...
				GtkWidget *box = getWidget("box");
				//gtk_container_foreach(GTK_CONTAINER(box),(GtkCallback)gtk_widget_destroy, NULL);
				
				GtkWidget *frame = gtk_frame_new("Tabs");
				GtkWidget *note = gtk_notebook_new();
//				gtk_container_add(GTK_CONTAINER(frame),note);
				//1st
				GtkWidget *pagelabel = gtk_label_new("NonBold/NonUnread");
				GtkWidget *grid = gtk_grid_new();
				GtkWidget *labelb = gtk_label_new("BackGround Color");
				GtkWidget *labelf = gtk_label_new("ForeGround Color");
				GtkWidget *buttonb = gtk_button_new_with_label("Choose");
				GtkWidget *buttonf = gtk_button_new_with_label("Choose");
				gtk_grid_attach(GTK_GRID(grid),labelb,0,0,1,1);
				gtk_grid_attach (GTK_GRID(grid),buttonb,0,1,1,1);
				gtk_grid_attach (GTK_GRID(grid),labelf,1,0,1,1);
				gtk_grid_attach (GTK_GRID(grid),buttonf,1,1,1,1);
//				gtk_container_add(GTK_CONTAINER(box),frame);

				g_object_set_data_full(G_OBJECT(buttonf), "name", g_strdup(keys.c_str()), g_free);
				g_object_set_data_full(G_OBJECT(buttonb), "name", g_strdup(keys.c_str()), g_free);
				g_signal_connect(buttonb, "clicked", G_CALLBACK(onBackColorChooserTab), (gpointer)this);
				g_signal_connect(buttonf, "clicked", G_CALLBACK(onForeColorChooserTab), (gpointer)this);
				gtk_notebook_append_page(GTK_NOTEBOOK(note), grid, pagelabel);

				//2nd
				GtkWidget *pagelabel2 = gtk_label_new("Unread");
				GtkWidget *grid2 = gtk_grid_new();
				GtkWidget *labelb2 = gtk_label_new("BackGround Color");
				GtkWidget *labelf2 = gtk_label_new("ForeGround Color");
				GtkWidget *buttonb2 = gtk_button_new_with_label("Choose");
				GtkWidget *buttonf2 = gtk_button_new_with_label("Choose");
				gtk_grid_attach(GTK_GRID(grid2),labelb2,0,0,1,1);
				gtk_grid_attach (GTK_GRID(grid2),buttonb2,0,1,1,1);
				gtk_grid_attach (GTK_GRID(grid2),labelf2,1,0,1,1);
				gtk_grid_attach (GTK_GRID(grid2),buttonf2,1,1,1,1);

				g_object_set_data_full(G_OBJECT(buttonf2), "name", g_strdup(keys.c_str()), g_free);
				g_object_set_data_full(G_OBJECT(buttonb2), "name", g_strdup(keys.c_str()), g_free);
				g_signal_connect(buttonb2, "clicked", G_CALLBACK(onBackColorChooserTab_unread), (gpointer)this);
				g_signal_connect(buttonf2, "clicked", G_CALLBACK(onForeColorChooserTab_unread), (gpointer)this);
				gtk_notebook_append_page(GTK_NOTEBOOK(note), grid2, pagelabel2);
        }
}

void Settings::onBackColorChooserTab(GtkWidget *button, gpointer data)
{
	Settings *s = (Settings *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string bg_string = WGETS("colored-tabs-" +key+"-color-bg");
	GdkRGBA bg;
	gdk_rgba_parse(&bg,bg_string.c_str());
	
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose BackGroun of "+key).c_str(),
                                                        GTK_WINDOW(s->getContainer()));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&bg);
//    int response = gtk_dialog_run(GTK_DIALOG(dialog));
  //  if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA bg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&bg_s);
		string color = WulforUtil::colorToString(&bg_s);
		WSET("colored-tabs-" +key+"-color-bg",color);

	}

}

void Settings::onForeColorChooserTab(GtkWidget *button, gpointer data)
{
	Settings *s = (Settings *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string fg_string = WGETS("colored-tabs-" +key+"-color-fg");
	GdkRGBA fg;
	gdk_rgba_parse(&fg,fg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose ForeGroun of "+key).c_str(),
                                                        GTK_WINDOW(s->getContainer()));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&fg);
//    int response = gtk_dialog_run(GTK_DIALOG(dialog));
 //   if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA fg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&fg_s);
		string color = WulforUtil::colorToString(&fg_s);
		WSET("colored-tabs-" +key+"-color-fg",color);

	}

}


void Settings::onBackColorChooserTab_unread(GtkWidget *button, gpointer data)
{
	Settings *s = (Settings *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string bg_string = WGETS("colored-tabs-" +key+"-color-bg-unread");
	GdkRGBA bg;
	gdk_rgba_parse(&bg,bg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose BackGroun of "+key).c_str(),
                                                        GTK_WINDOW(s->getContainer()));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&bg);
 //   int response = gtk_dialog_run(GTK_DIALOG(dialog));
  //  if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA bg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&bg_s);
		string color = WulforUtil::colorToString(&bg_s);
		WSET("colored-tabs-" +key+"-color-bg-unread",color);

	}

}

void Settings::onForeColorChooserTab_unread(GtkWidget *button, gpointer data)
{
	Settings *s = (Settings *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string fg_string = WGETS("colored-tabs-" +key+"-color-fg-unread");
	GdkRGBA fg;
	gdk_rgba_parse(&fg,fg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose ForeGroun of "+key).c_str(),
                                                        GTK_WINDOW(s->getContainer()));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&fg);
//    int response = gtk_dialog_run(GTK_DIALOG(dialog));
  //  if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA fg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&fg_s);
		string color = WulforUtil::colorToString(&fg_s);
		WSET("colored-tabs-" +key+"-color-fg-unread",color);

	}
}
