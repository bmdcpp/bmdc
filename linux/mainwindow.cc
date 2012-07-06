/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2012 Mank
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

#include "mainwindow.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <dcpp/ShareManager.h>
#include <dcpp/Text.h>
#include <dcpp/Upload.h>
#include <dcpp/Download.h>
#include <dcpp/ClientManager.h>
#include <dcpp/MappingManager.h>
#include <dcpp/GeoManager.h>
#include <dcpp/HttpDownload.h>
#include <dcpp/version.h>
#include <dcpp/ThrottleManager.h>
#include <dcpp/PluginManager.h>
#include <dcpp/ConnectivityManager.h>

#include "downloadqueue.hh"
#include "favoritehubs.hh"
#include "favoriteusers.hh"
#include "finishedtransfers.hh"
#include "func.hh"
#include "hub.hh"
#include "privatemessage.hh"
#include "publichubs.hh"
#include "search.hh"
#include "adlsearch.hh"
#include "searchspy.hh"
#include "settingsmanager.hh"
#include "sharebrowser.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "version.hh"
#include "ignoreusers.hh"
#include "System.hh"
#include "cmddebug.hh"
#include "notepad.hh"
#include "uploadqueue.hh"
#include "recenthub.hh"
#include "detectiontab.hh"
#include "AboutConfig.hh"

#include <boost/algorithm/string/trim.hpp>

using namespace std;
using namespace dcpp;

void* MainWindow::icons[(MainWindow::IconsToolbar)END][4] =
{
{ ((void*)(MainWindow::IconsToolbar)MainWindow::QUICKCON),(void*)"bmdc-connect",(void*)"bmdc-connect-on",(void*)"connect"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::FAVORITE_HUBS), (void*)"bmdc-favorite-hubs",(void*)"bmdc-favorite-hubs-on",(void*)"favHubs"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::FAVORITE_USERS), (void*)"bmdc-favorite-users",(void*)"bmdc-favorite-users-on",(void*)"favUsers"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::IGNORE_USERS), (void*)"bmdc-ignore-users",(void*)"bmdc-ignore-users-on",(void*)"ignUser"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::PUBLIC_HUBS), (void*)"bmdc-public-hubs",(void*)"bmdc-public-hubs-on",(void*)"publicHubs"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::SEARCH_ADL), (void*)"bmdc-search-adl",(void*)"bmdc-search-adl-on",(void*)"searchADL"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::SEARCH_SPY), (void*)"bmdc-search-spy",(void*)"bmdc-search-spy-on",(void*)"searchSpy"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::QUEUE), (void*)"bmdc-queue",(void*)"bmdc-queue-on",(void*)"queue"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::FDOWNLOADS), (void*)"bmdc-finished-downloads",(void*)"bmdc-finished-downloads-on",(void*)"finishedDownloads"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::FUPLOADS), (void*)"bmdc-finished-uploads",(void*)"bmdc-finished-uploads-on",(void*)"finishedUploads"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::NOTEPAD), (void*)"bmdc-notepad",(void*)"bmdc-notepad-on",(void*)"notepad"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::SYSTEM), (void*)"bmdc-system",(void*)"bmdc-system-on",(void*)"system"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::AWAY), (void*)"bmdc-away",(void*)"bmdc-away-on",(void*)"AwayIcon"},
{ ((void*)(MainWindow::IconsToolbar)MainWindow::LIMITING), (void*)"bmdc-limiting",(void*)"bmdc-limiting-on",(void*)"limitingButton"}
};

MainWindow::MainWindow():
	Entry(Entry::MAIN_WINDOW, "mainwindow.glade"),
	transfers(NULL),
	lastUpdate(0),
	lastUp(0),
	lastDown(0),
	minimized(FALSE),
	timer(0),
	statusFrame(1)
{
	window = GTK_WINDOW(getWidget("mainWindow"));
	gtk_window_set_role(window, getID().c_str());

	// Configure the dialogs
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("exitDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("connectDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("flistDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("ucLineDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("exitDialog")), window);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("connectDialog")), window);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("flistDialog")), window);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("ucLineDialog")), window);

	setStatRate_gui();//NOTE: core 0.762

	// toolbar
	setToolbarMenu_gui("connectMenuItemBar", "connect", "toolbar-button-connect");
	setToolbarMenu_gui("favHubsMenuItemBar", "favHubs", "toolbar-button-fav-hubs");
	setToolbarMenu_gui("favUsersMenuItemBar", "favUsers", "toolbar-button-fav-users");
	setToolbarMenu_gui("publicHubsMenuItemBar", "publicHubs", "toolbar-button-public-hubs");
	setToolbarMenu_gui("settingsMenuItemBar", "settings", "toolbar-button-settings");
	setToolbarMenu_gui("hashMenuItemBar", "hash", "toolbar-button-hash");
	setToolbarMenu_gui("searchMenuItemBar", "search", "toolbar-button-search");
	setToolbarMenu_gui("searchADLMenuItemBar", "searchADL", "toolbar-button-search-adl");
	setToolbarMenu_gui("searchSpyMenuItemBar", "searchSpy", "toolbar-button-search-spy");
	setToolbarMenu_gui("queueMenuItemBar", "queue", "toolbar-button-queue");
	setToolbarMenu_gui("finishedDownloadsMenuItemBar", "finishedDownloads", "toolbar-button-finished-downloads");
	setToolbarMenu_gui("finishedUploadsMenuItemBar", "finishedUploads", "toolbar-button-finished-uploads");
	setToolbarMenu_gui("quitMenuItemBar", "quit", "toolbar-button-quit");
	//BMDC++
	setToolbarMenu_gui("ignUserMenuItemBar", "ignUser", "toolbar-button-ignore");
	setToolbarMenu_gui("checknotepad", "notepad", "toolbar-button-notepad");
	setToolbarMenu_gui("checksystem", "system", "toolbar-button-system");
	setToolbarMenu_gui("awaycitem", "AwayIcon", "toolbar-button-away");
	setToolbarMenu_gui("limitingcmenu", "limitingButton", "toolbar-button-limiting");

	gint pos = 0;
	ToolbarStyle = 0;
	GtkBox *box = GTK_BOX(getWidget("hbox4"));
	GtkWidget *child = getWidget("toolbar1");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("leftToolbarItem")), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("hideToolbarItem")), (WGETI("toolbar-style") == 4) ? TRUE : FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("sizeToolbarItem")), WGETB("toolbar-small"));
	if (WGETB("toolbar-small"))
		g_object_set(G_OBJECT(child), "icon-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);

	if (WGETI("toolbar-position") == 1)
	{
		box = GTK_BOX(getWidget("vbox1"));
		gtk_toolbar_set_orientation(GTK_TOOLBAR(child), GTK_ORIENTATION_HORIZONTAL);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("topToolbarItem")), TRUE);
		pos = 1;
	}
	gtk_box_pack_start(box, child, FALSE, FALSE, 2);
	gtk_box_reorder_child(box, child, pos);
	g_object_unref(child);

	g_signal_connect(G_OBJECT(getWidget("sizeToolbarItem")), "toggled", G_CALLBACK(onSizeToolbarToggled_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(getWidget("hideToolbarItem")), "toggled", G_CALLBACK(onHideToolbarToggled_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(getWidget("topToolbarItem")), "toggled", G_CALLBACK(onTopToolbarToggled_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(getWidget("leftToolbarItem")), "toggled", G_CALLBACK(onLeftToolbarToggled_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(getWidget("add")), "clicked", G_CALLBACK(onAddButtonClicked_gui), (gpointer)this);

	GtkWidget *menu = gtk_menu_new();
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(getWidget("favHubs")), menu);
	const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
	gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback)gtk_widget_destroy, NULL);

	for (FavoriteHubEntryList::const_iterator it = fh.begin(); it != fh.end(); ++it)
	{
		FavoriteHubEntry *entry = *it;
		string address = entry->getServer();
		string encoding = entry->getEncoding();
		GtkWidget *item = gtk_menu_item_new_with_label(address.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		g_object_set_data_full(G_OBJECT(item), "address", g_strdup(address.c_str()), g_free);
		g_object_set_data_full(G_OBJECT(item), "encoding", g_strdup(encoding.c_str()), g_free);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onHubClicked_gui), (gpointer)this);
	}
	gtk_widget_show_all(menu);
	
	///Limits menu
	menu = gtk_menu_new();
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(getWidget("limitingButton")), menu);
	gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback)gtk_widget_destroy, NULL);
	GtkWidget *upitem = gtk_menu_item_new_with_label(_("Upload Limit (disable)"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), upitem);
	g_object_set_data_full(G_OBJECT(upitem), "type", g_strdup("up"), g_free);
	g_signal_connect(G_OBJECT(upitem), "activate", G_CALLBACK(onLimitingDisable), (gpointer)this);
	GtkWidget *sep =  gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
	///TODO @change to dynamic
	for(int i = 10240; i<2097152; i = i*2+40960/2) {
		string tmenu = Text::toT(Util::formatBytes(i)) + (_("/s"));
		string tspeed = Util::toString(i);
		GtkWidget *item = gtk_menu_item_new_with_label(tmenu.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		g_object_set_data_full(G_OBJECT(item), "speed", g_strdup(tspeed.c_str()), g_free);
		g_object_set_data_full(G_OBJECT(item), "type", g_strdup("up"), g_free);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onLimitingMenuItem_gui), (gpointer)this);
	}
	GtkWidget *sep3 = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep3);
	GtkWidget *dwitem = gtk_menu_item_new_with_label(_("Download Limit (disable)"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), dwitem);
	g_object_set_data_full(G_OBJECT(dwitem), "type", g_strdup("dw"), g_free);
	g_signal_connect(G_OBJECT(dwitem), "activate", G_CALLBACK(onLimitingDisable), (gpointer)this);
	GtkWidget *sep2 = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep2);

	for(int j = 10240; j<2097152; j = j*2+40960/2) {
		string tmenu = Text::toT(Util::formatBytes(j)) + (_("/s"));
		string tspeed = Util::toString(j);
		GtkWidget *item = gtk_menu_item_new_with_label(tmenu.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		g_object_set_data_full(G_OBJECT(item), "speed", g_strdup(tspeed.c_str()), g_free);
		g_object_set_data_full(G_OBJECT(item), "type", g_strdup("dw"), g_free);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onLimitingMenuItem_gui), (gpointer)this);
	}

	gtk_widget_show_all(menu);

	// menu
	g_object_ref_sink(getWidget("statusIconMenu"));
	g_object_ref_sink(getWidget("toolbarMenu"));

	// magnet dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("MagnetDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("MagnetDialog")), window);
	setChooseMagnetDialog_gui();
	g_signal_connect(getWidget("MagnetDialog"), "response", G_CALLBACK(onResponseMagnetDialog_gui), (gpointer) this);
	g_signal_connect(getWidget("MagnetDialog"), "delete-event", G_CALLBACK(onDeleteEventMagnetDialog_gui), (gpointer) this);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("transferCheckButton")), TRUE);

	// About dialog
	gchar *comments = g_strdup_printf(_("DC++ Client based on the source code FreeDC++\n\nBMDC++ version: %s.%s\nCore version: %s"),
		GUI_VERSION_STRING, BMDC_REVISION_STRING, VERSIONSTRING);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), comments);
	g_free(comments);

	// set logo 96x96
	GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
	GdkPixbuf *logo = gtk_icon_theme_load_icon(iconTheme, g_get_prgname(), 96, GTK_ICON_LOOKUP_FORCE_SVG, NULL);

	if (logo != NULL)
	{
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), logo);
		g_object_unref(logo);
	}

	gtk_about_dialog_set_email_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
	gtk_about_dialog_set_url_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
	// This has to be set in code in order to activate the link
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), "http://launchpad.net/bmdc++");
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("aboutDialog")), window);

	// Set all windows to the default icon
	gtk_window_set_default_icon_name(g_get_prgname());

	// All notebooks created in glade need one page.
	// In our case, this is just a placeholder, so we remove it.
	gtk_notebook_remove_page(GTK_NOTEBOOK(getWidget("book")), -1);
	g_object_set_data(G_OBJECT(getWidget("book")), "page-rotation-list", NULL);
	gtk_widget_set_sensitive(getWidget("closeMenuItem"), FALSE);

	// Connect the signals to their callback functions.
	g_signal_connect(window, "delete-event", G_CALLBACK(onCloseWindow_gui), (gpointer)this);
	g_signal_connect(window, "window-state-event", G_CALLBACK(onWindowState_gui), (gpointer)this);
	g_signal_connect(window, "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
	g_signal_connect(window, "key-press-event", G_CALLBACK(onKeyPressed_gui), (gpointer)this);
	g_signal_connect(getWidget("book"), "switch-page", G_CALLBACK(onPageSwitched_gui), (gpointer)this);
	g_signal_connect_after(getWidget("pane"), "realize", G_CALLBACK(onPaneRealized_gui), (gpointer)this);
	g_signal_connect(getWidget("connect"), "clicked", G_CALLBACK(onConnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favHubs"), "clicked", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favUsers"), "clicked", G_CALLBACK(onFavoriteUsersClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("ignUser"), "clicked", G_CALLBACK(onIgnoreUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("publicHubs"), "clicked", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settings"), "clicked", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("hash"), "clicked", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("search"), "clicked", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchADL"), "clicked", G_CALLBACK(onSearchADLClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchSpy"), "clicked", G_CALLBACK(onSearchSpyClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("queue"), "clicked", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("notepad"), "clicked", G_CALLBACK(onNotepadClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("system"), "clicked", G_CALLBACK(onSystemLogClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("AwayIcon"), "clicked", G_CALLBACK(onAwayClicked_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("quit"), "clicked", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedDownloads"), "clicked", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedUploads"), "clicked", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openFileListMenuItem"), "activate", G_CALLBACK(onOpenFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openOwnListMenuItem"), "activate", G_CALLBACK(onOpenOwnListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("refreshFileListMenuItem"), "activate", G_CALLBACK(onRefreshFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("quickConnectMenuItem"), "activate", G_CALLBACK(onConnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("reconnectMenuItem"), "activate", G_CALLBACK(onReconnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settingsMenuItem"), "activate", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("closeMenuItem"), "activate", G_CALLBACK(onCloseClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("exitMenuItem"), "activate", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favoriteHubsMenuItem"), "activate", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favoriteUsersMenuItem"), "activate", G_CALLBACK(onFavoriteUsersClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("publicHubsMenuItem"), "activate", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("indexingProgressMenuItem"), "activate", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("detitem"), "activate", G_CALLBACK(onDetectionClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("cmditem"), "activate", G_CALLBACK(onCmdDebugClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("uqueueitem"), "activate", G_CALLBACK(onUploadQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("RecentHubMenuItem"), "activate", G_CALLBACK(onRecentHubClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("tthitem"), "activate", G_CALLBACK(onTTHFileDialog_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonfile"), "clicked", G_CALLBACK(onTTHFileButton_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("searchMenuItem"), "activate", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchADLMenuItem"), "activate", G_CALLBACK(onSearchADLClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchSpyMenuItem"), "activate", G_CALLBACK(onSearchSpyClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadQueueMenuItem"), "activate", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedDownloadsMenuItem"), "activate", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedUploadsMenuItem"), "activate", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("previousTabMenuItem"), "activate", G_CALLBACK(onPreviousTabClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("nextTabMenuItem"), "activate", G_CALLBACK(onNextTabClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("aboutMenuItem"), "activate", G_CALLBACK(onAboutClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("transferCheckButton"), "toggled", G_CALLBACK(onTransferToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("browseButton"), "clicked", G_CALLBACK(onBrowseMagnetButton_gui), (gpointer)this);
	g_signal_connect(getWidget("dowloadQueueRadioButton"), "toggled", G_CALLBACK(onDowloadQueueToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("searchRadioButton"), "toggled", G_CALLBACK(onSearchMagnetToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("showRadioButton"), "toggled", G_CALLBACK(onSearchMagnetToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("setMagnetChoiceItem"), "activate", G_CALLBACK(onSetMagnetChoiceDialog_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("CloseTabHubAllMenuItem"), "activate", G_CALLBACK(onCloseAllHub_gui), (gpointer)this);
	g_signal_connect(getWidget("CloseTabPMAllMenuItem"), "activate", G_CALLBACK(onCloseAllPM_gui), (gpointer)this);
	g_signal_connect(getWidget("closeTabSearchAllMenuItem"), "activate", G_CALLBACK(onCloseAllSearch_gui), (gpointer)this);
	g_signal_connect(getWidget("CloseTabPMOfflineItem"), "activate", G_CALLBACK(onCloseAlloffPM_gui), (gpointer)this);
	g_signal_connect(getWidget("recontallitem"), "activate", G_CALLBACK(onReconectAllHub_gui), (gpointer)this);
	/**/
	g_signal_connect(getWidget("AboutConfigItem"), "activate", G_CALLBACK(onAboutConfigClicked_gui), (gpointer)this);
	// Help menu
	g_object_set_data_full(G_OBJECT(getWidget("homeMenuItem")), "link",
		g_strdup("http://launchpad.net/bmdc++"), g_free);
	g_signal_connect(getWidget("homeMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	g_object_set_data_full(G_OBJECT(getWidget("sourceMenuItem")), "link",
		g_strdup("http://code.launchpad.net/bmdc++"), g_free);
	g_signal_connect(getWidget("sourceMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	g_object_set_data_full(G_OBJECT(getWidget("issueMenuItem")), "link",
		g_strdup("http://bugs.launchpad.net/bmdc++"), g_free);
	g_signal_connect(getWidget("issueMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	g_object_set_data_full(G_OBJECT(getWidget("forumDiscussionItem")), "link",
		g_strdup("http://answers.launchpad.net/bmdc++"), g_free);
	g_signal_connect(getWidget("forumDiscussionItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	g_object_set_data_full(G_OBJECT(getWidget("changeLogItem")), "link",
		g_strdup((string(_DATADIR) + "/doc/bmdc/BMDC-Changelog.txt").c_str()), g_free);
	g_signal_connect(getWidget("changeLogItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	g_object_set_data_full(G_OBJECT(getWidget("dowloadMenuItem1")), "link",
		g_strdup("https://sourceforge.net/projects/freedcppmv/files/"), g_free);
	g_signal_connect(getWidget("dowloadMenuItem1"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	g_object_set_data_full(G_OBJECT(getWidget("dowloadMenuItem2")), "link",
		g_strdup("https://launchpad.net/~bmdc-team/+archive/ppa-stable"), g_free);
	g_signal_connect(getWidget("dowloadMenuItem2"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	onQuit = FALSE;
	
	// colourstuff added by curse //add to BMDC++ by Mank
	string res = WulforManager::get()->getPath() + "/glade/resources.rc";
	gtk_rc_parse(res.c_str());
	// colourstuff end

	// Load window state and position from settings manager
	gint posX = WGETI("main-window-pos-x");
	gint posY = WGETI("main-window-pos-y");
	gint sizeX = WGETI("main-window-size-x");
	gint sizeY = WGETI("main-window-size-y");

	gtk_window_move(window, posX, posY);
	gtk_window_resize(window, sizeX, sizeY);
	if (WGETI("main-window-maximized"))
		gtk_window_maximize(window);

	setMainStatus_gui(_("Welcome to ") + string(g_get_application_name()));

	loadIcons_gui();
	showTransfersPane_gui();

	// Putting this after all the resizing and moving makes the window appear
	// in the correct position instantly, looking slightly more cool
	// (seems we have rather poor standards for cool?)
	gtk_widget_show_all(GTK_WIDGET(window));

	setToolbarButton_gui();
	setTabPosition_gui(WGETI("tab-position"));
	setToolbarStyle_gui(WGETI("toolbar-style"));

	createStatusIcon_gui();
	setInitThrotles();
	Sound::start();
	Emoticons::start();
	Notify::start();
	
	if(SETTING(GET_USER_COUNTRY)) {
			checkUpdateofGeoIp(true);
			checkUpdateofGeoIp(false);
	}

	PluginManager::getInstance()->runHook(HOOK_UI_CREATED, getContainer(), NULL);
}

MainWindow::~MainWindow()
{
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	LogManager::getInstance()->removeListener(this);

	listQueue.shutdown();

	GList *list = (GList *)g_object_get_data(G_OBJECT(getWidget("book")), "page-rotation-list");
	g_list_free(list);

	// Save window state and position
	gint posX, posY, sizeX, sizeY, transferPanePosition;
	bool maximized = TRUE;
	GdkWindowState gdkState;

	gtk_window_get_position(window, &posX, &posY);
	gtk_window_get_size(window, &sizeX, &sizeY);
	gdkState = gdk_window_get_state(GTK_WIDGET(window)->window);
	transferPanePosition = sizeY - gtk_paned_get_position(GTK_PANED(getWidget("pane")));

	if (!(gdkState & GDK_WINDOW_STATE_MAXIMIZED))
		maximized = FALSE;
		
		WSET("main-window-pos-x", posX);
		WSET("main-window-pos-y", posY);
		WSET("main-window-size-x", sizeX);
		WSET("main-window-size-y", sizeY);

	WSET("main-window-maximized", (int)maximized);
	if (transferPanePosition > 10)
		WSET("transfer-pane-position", transferPanePosition);

	if (timer > 0)
		g_source_remove(timer);

	WSET("status-icon-blink-use", useStatusIconBlink);
	gtk_widget_destroy(GTK_WIDGET(window));
	g_object_unref(statusIcon);
	g_object_unref(getWidget("statusIconMenu"));
	g_object_unref(getWidget("toolbarMenu"));
	Sound::stop();
	Emoticons::stop();
	Notify::stop();
}

GtkWidget *MainWindow::getContainer()
{
	return getWidget("mainWindow");
}

void MainWindow::show()
{
	QueueManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);

	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0(this, &MainWindow::startSocket_client);
	WulforManager::get()->dispatchClientFunc(f0);

	f0 = new F0(this, &MainWindow::autoConnect_client);
	WulforManager::get()->dispatchClientFunc(f0);

	autoOpen_gui();
}

void MainWindow::setTitle(const string& text)
{
	string title;

	if (!text.empty())
		title = text + " - " + g_get_application_name();
	else
		title = g_get_application_name();

	gtk_window_set_title(window, title.c_str());
}

bool MainWindow::isActive_gui()
{
	return gtk_window_is_active(window);
}

void MainWindow::setUrgent_gui()
{
	gtk_window_set_urgency_hint(window, true);
}

/*
 * Create and show Transfers pane
 */
void MainWindow::showTransfersPane_gui()
{
	dcassert(transfers == NULL);

	transfers = new Transfers();
	gtk_paned_pack2(GTK_PANED(getWidget("pane")), transfers->getContainer(), TRUE, TRUE);
	addChild(transfers);
	transfers->show();
}

/*
 * Load the custom icons or the stock icons as per the setting
 */
void MainWindow::loadIcons_gui()
{
	WulforUtil::registerIcons();

	// Reset the stock IDs manually to force the icon to refresh
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("favHubs")), "bmdc-favorite-hubs");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("favUsers")), "bmdc-favorite-users");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("publicHubs")), "bmdc-public-hubs");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("settings")), "bmdc-preferences");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("hash")), "bmdc-hash");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("search")), "bmdc-search");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("searchADL")), "bmdc-search-adl");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("searchSpy")), "bmdc-search-spy");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("queue")), "bmdc-queue");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("finishedDownloads")), "bmdc-finished-downloads");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("finishedUploads")), "bmdc-finished-uploads");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("quit")), "bmdc-quit");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("connect")), "bmdc-connect");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("ignUser")), "bmdc-ignore-users");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("system")), "bmdc-system");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("notepad")), "bmdc-notepad");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("AwayIcon")), "bmdc-away");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("limitingButton")), "bmdc-limiting");
	
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageHubs")), "bmdc-public-hubs", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageDownloadSpeed")), "bmdc-download", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageUploadSpeed")), "bmdc-upload", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageDownloadRate")), "bmdc-download", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageUploadRate")), "bmdc-upload", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void MainWindow::autoOpen_gui()
{
	if (WGETB("open-public"))
		showPublicHubs_gui();
	if (WGETB("open-queue"))
		showDownloadQueue_gui();
	if (WGETB("open-favorite-hubs"))
		showFavoriteHubs_gui();
	if (WGETB("open-favorite-users"))
		showFavoriteUsers_gui();
	if (WGETB("open-finished-downloads"))
		showFinishedDownloads_gui();
	if (WGETB("open-finished-uploads"))
		showFinishedUploads_gui();
	if (WGETB("open-search-spy"))
		showSearchSpy_gui();
	if (WGETB("open-system"))
		showSystemLog_gui();
	if (WGETB("open-notepad"))
		showNotepad_gui();
	if (WGETB("open-ignore"))
		showIgnoreUsers_gui();
	if (WGETB("open-upload-queue"))
		showUploadQueue_gui();				
}

void MainWindow::onLimitingMenuItem_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string speed = (gchar *)g_object_get_data(G_OBJECT(widget), "speed");
	string type = (gchar *)g_object_get_data(G_OBJECT(widget), "type");
	if(speed.empty())
		return;

	if(type == "up")
	{
		ThrottleManager::setSetting(SettingsManager::MAX_UPLOAD_SPEED_MAIN, Util::toInt(speed)/1024 );
		mw->setLimitingIcon(true);
	}
	else if(type == "dw")
	{
		ThrottleManager::setSetting(SettingsManager::MAX_DOWNLOAD_SPEED_MAIN, Util::toInt(speed)/1024 );
		mw->setLimitingIcon(true);
	}
	SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, true);
	mw->setStatRate_gui();
	
}

void MainWindow::setLimitingIcon(bool Limited)
{
	setMainStatus_gui(_("Throtle ") + Limited ? "on" : "off", time(NULL));
	setStatusOfIcons(LIMITING,Limited);
}

void MainWindow::onLimitingDisable(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string type = (gchar *)g_object_get_data(G_OBJECT(widget), "type");

	if(type == "dw")
	{
		ThrottleManager::setSetting(SettingsManager::MAX_DOWNLOAD_SPEED_MAIN, 0);
	}
	else if(type == "up")
	{
		ThrottleManager::setSetting(SettingsManager::MAX_UPLOAD_SPEED_MAIN, 0);
	}
	SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, false);
	
    mw->setLimitingIcon(false);
    mw->setStatRate_gui();
}

void MainWindow::setInitThrotles()
{
	int up = SETTING(MAX_UPLOAD_SPEED_MAIN);
	int down = SETTING(MAX_DOWNLOAD_SPEED_MAIN);
	bool enabled = SETTING(THROTTLE_ENABLE);
	if(enabled && (up > 0) ) {
		setLimitingIcon(true);
		return;
	}
	if(enabled && (down > 0)) {
		setLimitingIcon(true);
		return;
	}
	
	if(!enabled && (up == 0)) {
		setLimitingIcon(false);
		return;
	}
	if(!enabled && (down == 0)) {
		setLimitingIcon(false);
		return;
	}
}

void MainWindow::setToolbarMenu_gui(const string &item_key, const string &button_key, const string &key)
{
	GtkWidget *item = getWidget(item_key);
	GtkWidget *button = getWidget(button_key);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), WGETB(key));
	g_object_set_data_full(G_OBJECT(item), "key", g_strdup(key.c_str()), g_free);
	g_signal_connect(G_OBJECT(item), "toggled", G_CALLBACK(onToolToggled_gui), (gpointer)button);
}

void MainWindow::addBookEntry_gui(BookEntry *entry)
{
	addChild(entry);

	GtkWidget *page = entry->getContainer();
	GtkWidget *label = entry->getLabelBox();
	GtkWidget *closeButton = entry->getCloseButton();
	GtkWidget *tabMenuItem = entry->getTabMenuItem();

	addTabMenuItem_gui(tabMenuItem, page);

	gtk_notebook_append_page(GTK_NOTEBOOK(getWidget("book")), page, label);

	g_signal_connect(label, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
    if(WGETB("use-close-button"))
    {
        g_signal_connect(closeButton, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
        g_signal_connect(closeButton, "clicked", G_CALLBACK(onCloseBookEntry_gui), (gpointer)entry);
    }
	gtk_widget_set_sensitive(getWidget("closeMenuItem"), TRUE);

	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(getWidget("book")), page, TRUE);

	entry->show();
}

GtkWidget *MainWindow::currentPage_gui()
{
	int pageNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("book")));

	if (pageNum == -1)
		return NULL;
	else
		return gtk_notebook_get_nth_page(GTK_NOTEBOOK(getWidget("book")), pageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page)
{
	int num = gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("book")), page);
	int currentNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("book")));

	if (num != -1 && num != currentNum)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(getWidget("book")), num);
}

void MainWindow::removeBookEntry_gui(BookEntry *entry)
{
	string entryID = entry->getID();
	Entry::EntryType type = entry->getType();
	removeItemFromList(type, entryID);

	string::size_type pos = entryID.find(':');
	if (pos != string::npos) entryID.erase(0, pos + 1);

	StringIter it = find(EntryList.begin(), EntryList.end(), entryID);

	if (it != EntryList.end())
		EntryList.erase(it);

	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));
	GtkWidget *page = entry->getContainer();
	GtkWidget* menuItem = entry->getTabMenuItem();
	int num = gtk_notebook_page_num(book, page);
	removeChild(entry);

	if (num != -1)
	{
		GList *list = (GList *)g_object_get_data(G_OBJECT(book), "page-rotation-list");
		list = g_list_remove(list, (gpointer)page);
		g_object_set_data(G_OBJECT(book), "page-rotation-list", (gpointer)list);

		// if removing the current page, switch to the previous page in the rotation list
		if (num == gtk_notebook_get_current_page(book))
		{
			GList *prev = g_list_first(list);
			if (prev != NULL)
			{
				gint childNum = gtk_notebook_page_num(book, GTK_WIDGET(prev->data));
				gtk_notebook_set_current_page(book, childNum);
			}
		}
		gtk_notebook_remove_page(book, num);

		removeTabMenuItem_gui(menuItem);

		if (gtk_notebook_get_n_pages(book) == 0)
		{
			gtk_widget_set_sensitive(getWidget("closeMenuItem"), FALSE);
			setTitle(""); // Reset window title to default
		}
		
	}
}

void MainWindow::removeItemFromList(Entry::EntryType type, string id)
{
	vector<Hub*> hubs;
	vector<PrivateMessage*> pms;
	vector<Search*> searchs;
	switch(type)
	{
		case Entry::FAVORITE_HUBS:
			setStatusOfIcons(FAVORITE_HUBS,false);
			break;	
		case Entry::FAVORITE_USERS:
			setStatusOfIcons(FAVORITE_USERS,false);
			break;
		case Entry::IGNORE_USERS:
			setStatusOfIcons(IGNORE_USERS,false);
			break;
		case Entry::PUBLIC_HUBS:
			setStatusOfIcons(PUBLIC_HUBS,false);
			break;
		case Entry::NOTEPAD:
			setStatusOfIcons(NOTEPAD,false);
			break;
		case Entry::SYSTEML:
			setStatusOfIcons(SYSTEM,false);
			break;
		case Entry::FINISHED_DOWNLOADS:
			setStatusOfIcons(FDOWNLOADS,false);
			break;
		case Entry::FINISHED_UPLOADS:
			setStatusOfIcons(FUPLOADS,false);
			break;
		case Entry::SEARCH_ADL:
			setStatusOfIcons(SEARCH_ADL,false);
			break;
		case Entry::SEARCH_SPY:
			setStatusOfIcons(SEARCH_SPY,false);
			break;
		case Entry::DOWNLOAD_QUEUE:
			setStatusOfIcons(QUEUE,false);
			break;	
		case Entry::HUB:
			if(Hubs.empty()) break;
			 for(vector<Hub*>::const_iterator it = Hubs.begin();it != Hubs.end();++it)
			 {
				 Hub *hub = *it;
				 string hubId = (dynamic_cast<Entry*>(hub))->getID();
				 if(hubId == id)
					 continue;
				hubs.push_back(*it);	 
			 }
			 Hubs = hubs;
			 break;
		case Entry::PRIVATE_MESSAGE:
			if(privateMessage.empty()) break;
			for(vector<PrivateMessage*>::const_iterator it = privateMessage.begin();it != privateMessage.end();++it)
			 {
				 string pId = (dynamic_cast<Entry*>(*it))->getID();
				 if(pId == id)
					 continue;
				pms.push_back(*it);	 
			 }
			 privateMessage = pms;
			 break;
		case Entry::SEARCH:
			if(search.empty()) break;
			for(vector<Search*>::const_iterator it = search.begin();it != search.end();++it)
			 {
				 Search *s = *it;
				 string sId = (dynamic_cast<Entry*>(s))->getID();
				 if(sId == id)
					 continue;
				searchs.push_back(*it);	 
			 }
			 search = searchs;
			 break;
		default:break;
	}	
}

void MainWindow::previousTab_gui()
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

	if (gtk_notebook_get_current_page(book) == 0)
		gtk_notebook_set_current_page(book, -1);
	else
		gtk_notebook_prev_page(book);
}

void MainWindow::nextTab_gui()
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

	if (gtk_notebook_get_n_pages(book) - 1 == gtk_notebook_get_current_page(book))
		gtk_notebook_set_current_page(book, 0);
	else
		gtk_notebook_next_page(book);
}

void MainWindow::addTabMenuItem_gui(GtkWidget* menuItem, GtkWidget* page)
{
	g_signal_connect(menuItem, "activate", G_CALLBACK(onRaisePage_gui), (gpointer)page);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("tabsMenu")), menuItem);
	gtk_widget_show_all(getWidget("tabsMenu"));

	gtk_widget_set_sensitive(getWidget("previousTabMenuItem"), TRUE);
	gtk_widget_set_sensitive(getWidget("nextTabMenuItem"), TRUE);
	gtk_widget_set_sensitive(getWidget("tabMenuSeparator"), TRUE);
}

void MainWindow::removeTabMenuItem_gui(GtkWidget *menuItem)
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

	gtk_container_remove(GTK_CONTAINER(getWidget("tabsMenu")), menuItem);

	if (gtk_notebook_get_n_pages(book) == 0)
	{
		gtk_widget_set_sensitive(getWidget("previousTabMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("nextTabMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("tabMenuSeparator"), FALSE);
	}
}

/*
 * Create status icon.
 */
void MainWindow::createStatusIcon_gui()
{
	useStatusIconBlink = WGETB("status-icon-blink-use");
	statusIcon = gtk_status_icon_new_from_icon_name(g_get_prgname());

	g_signal_connect(getWidget("statusIconQuitItem"), "activate", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("statusIconShowInterfaceItem"), "toggled", G_CALLBACK(onShowInterfaceToggled_gui), (gpointer)this);
	g_signal_connect(statusIcon, "activate", G_CALLBACK(onStatusIconActivated_gui), (gpointer)this);
	g_signal_connect(statusIcon, "popup-menu", G_CALLBACK(onStatusIconPopupMenu_gui), (gpointer)this);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("statusIconBlinkUseItem")), useStatusIconBlink);
	g_signal_connect(getWidget("statusIconBlinkUseItem"), "toggled", G_CALLBACK(onStatusIconBlinkUseToggled_gui), (gpointer)this);

	if (SETTING(ALWAYS_TRAY))
		gtk_status_icon_set_visible(statusIcon, TRUE);
	else
		gtk_status_icon_set_visible(statusIcon, FALSE);
}

void MainWindow::updateStatusIconTooltip_gui(string download, string upload)
{
	ostringstream toolTip;
	toolTip << g_get_application_name() << endl << _("Download: ") << download << endl << _("Upload: ") << upload;
	gtk_status_icon_set_tooltip(statusIcon, toolTip.str().c_str());
}

void MainWindow::setMainStatus_gui(string text, time_t t)
{
	if (!text.empty())
	{
		text = "[" + Util::getShortTimeString(t) + "] " + text;
		gtk_label_set_text(GTK_LABEL(getWidget("labelStatus")), text.c_str());
		
		if(statustext.size() > (uint32_t)WGETI("max-tooltips"))
		{
			    statustext.pop();
		}
        queue<string> tmp = statustext;
        string statusTextOnToolTip;
        while(!tmp.empty())
        {
           statusTextOnToolTip += "\n" + tmp.front();
           tmp.pop();
        }
        statustext.push(text);
        
       #if !GTK_CHECK_VERSION(2, 12, 0)
             gtk_tooltips_set_tip (statusTips, getWidget("labelStatus"), statusTextOnToolTip.c_str(), NULL);
       #else
             gtk_widget_set_tooltip_text(getWidget("labelStatus"), statusTextOnToolTip.c_str());
       #endif
	}
}

void MainWindow::showNotification_gui(string head, string body, Notify::TypeNotify notify)
{
	Notify::get()->showNotify(head, body, notify);
}

void MainWindow::setStats_gui(string hubs, string downloadSpeed,
	string downloaded, string uploadSpeed, string uploaded)
{
	gtk_label_set_text(GTK_LABEL(getWidget("labelHubs")), hubs.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelDownloadSpeed")), downloadSpeed.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelDownloaded")), downloaded.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelUploadSpeed")), uploadSpeed.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelUploaded")), uploaded.c_str());
	
	string fslots = Util::toString(UploadManager::getInstance()->getFreeSlots());
	string slots = _("Slots: ") + fslots + "/" + Util::toString(SETTING(SLOTS));
	string shared = _("Shared: ") + Util::formatBytes(ShareManager::getInstance()->getShareSize());
	gtk_label_set_text(GTK_LABEL(getWidget("labelShare")), shared.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelSlots")), slots.c_str());
}

void MainWindow::setStatRate_gui()//NOTE: core 0.762
{
	int uploadSpeed = SETTING(MAX_UPLOAD_SPEED_MAIN);
	int downloadSpeed = SETTING(MAX_DOWNLOAD_SPEED_MAIN);
	string uploadRate = uploadSpeed ? Util::formatBytes(uploadSpeed*1024) + "/" + _("s") : "max";
	string downloadRate = downloadSpeed ? Util::formatBytes(downloadSpeed*1024) + "/" + _("s") : "max";
	gtk_label_set_text(GTK_LABEL(getWidget("labelDownloadRate")), downloadRate.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelUploadRate")), uploadRate.c_str());
}

BookEntry* MainWindow::findBookEntry(const EntryType type, const string &id)
{
	Entry *entry = getChild(type, id);
	return dynamic_cast<BookEntry*>(entry);
}

template<typename T, typename B>
void MainWindow::showBook(const T& type, const B& book)
{ 
	BookEntry *entry = findBookEntry(type);
	
	if(entry == NULL)
	{
			entry = book;
			addBookEntry_gui(entry);	
	}
	raisePage_gui(entry->getContainer());
}

void MainWindow::onAboutConfigClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;	
	mw->showBook<Entry::EntryType,BookEntry *>(Entry::ABOUT_CONFIG,new AboutConfig());
}

void MainWindow::showDownloadQueue_gui()
{
	showBook<Entry::EntryType,BookEntry *>(Entry::DOWNLOAD_QUEUE,new DownloadQueue());
	setStatusOfIcons(QUEUE,true);
}

void MainWindow::showFavoriteHubs_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::FAVORITE_HUBS, new FavoriteHubs());
	setStatusOfIcons(FAVORITE_HUBS,true);
}

void MainWindow::showFavoriteUsers_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::FAVORITE_USERS, new FavoriteUsers());
	setStatusOfIcons(FAVORITE_USERS,true);
}
//[BMDC++
void MainWindow::showIgnoreUsers_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::IGNORE_USERS, new IgnoreUsers());
	setStatusOfIcons(IGNORE_USERS,true);
}

void MainWindow::showCmdDebug_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::CMD, new cmddebug());
}

void MainWindow::showSystemLog_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::SYSTEML, new systemlog());
	setStatusOfIcons(SYSTEM, true);
}

void MainWindow::showNotepad_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::NOTEPAD, new notepad());
	setStatusOfIcons(NOTEPAD, true);
}

void MainWindow::showUploadQueue_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::UPLOADQUEUE, new UploadQueue());
}

void MainWindow::showRecentHubs_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::RECENT, new RecentHubs());
}

void MainWindow::showDetection_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::DETECTION, new DetectionTab());
}

void MainWindow::showFinishedDownloads_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::FINISHED_DOWNLOADS, FinishedTransfers::createFinishedDownloads());
	setStatusOfIcons(FDOWNLOADS, true);
}

void MainWindow::showFinishedUploads_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::FINISHED_UPLOADS, FinishedTransfers::createFinishedUploads());
	setStatusOfIcons(FUPLOADS, true);
}

void MainWindow::showHub_gui(string address, string encoding)
{
	boost::algorithm::trim(address);
	
	if(address.empty())
	{
		showMessageDialog_gui(_("Empty hub address specified"),_("Empty hub address specified"));
		return;
	}
	
	BookEntry *entry = findBookEntry(Entry::HUB, address);

	if (entry == NULL)
	{
		entry = new Hub(address, encoding);
		addBookEntry_gui(entry);

		EntryList.push_back(address);
		Hubs.push_back(dynamic_cast<Hub *>(entry));
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showSearchSpy_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::SEARCH_SPY, new SearchSpy());
	setStatusOfIcons(SEARCH_SPY,true);
}

void MainWindow::showSearchADL_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::SEARCH_ADL, new SearchADL());
	setStatusOfIcons(SEARCH_ADL,true);
}

void MainWindow::addPrivateMessage_gui(Msg::TypeMsg typemsg, string cid, string hubUrl, string message, bool useSetting)
{
	BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);
	bool raise = TRUE;

	// If PM is initiated by another user, use setting except if tab is already open.
	if (useSetting)
		raise = (entry == NULL) ? !SETTING(POPUNDER_PM) : FALSE;

	if (entry == NULL)
	{
		entry = new PrivateMessage(cid, hubUrl);
		addBookEntry_gui(entry);

		EntryList.push_back(cid);
		privateMessage.push_back(dynamic_cast<PrivateMessage*>(entry));
	}
	
	if(!message.empty() && hubUrl.empty())
	{
		dynamic_cast<PrivateMessage*>(entry)->sendMessage_p(message);
		if(raise)
			raisePage_gui(entry->getContainer());
		return;
	}

	if (!message.empty())
	{
		dynamic_cast<PrivateMessage*>(entry)->addMessage_gui(message, typemsg);

		bool show = FALSE;

		if (!isActive_gui())
		{
			show = TRUE;
			if (useStatusIconBlink && timer == 0)
			{
				timer = g_timeout_add(1000, animationStatusIcon_gui, (gpointer)this);
			}
		}
		else if (currentPage_gui() != entry->getContainer() && !WGETI("notify-only-not-active"))
		{
			show = TRUE;
		}

		if (show)
		{
			const int length = WGETI("notify-pm-length");

			if (length > 0 && g_utf8_strlen(message.c_str(), -1) > length)
			{
				const gchar *p = message.c_str();
				int ii = 0;

				while (*p)
				{
					p = g_utf8_next_char(p);

					if (++ii >= length)
						break;
				}

				string::size_type i = string(p).size();
				string::size_type j = message.size();

				Notify::get()->showNotify("", message.substr(0, j - i) + "...", Notify::PRIVATE_MESSAGE);
			}
			else
				Notify::get()->showNotify("", message, Notify::PRIVATE_MESSAGE);
		}
	}

	if (raise)
		raisePage_gui(entry->getContainer());
}

void MainWindow::removeTimerSource_gui()
{
	if (timer > 0)
	{
		g_source_remove(timer);
		timer = 0;
		gtk_status_icon_set_from_icon_name(statusIcon, g_get_prgname());
	}
}

void MainWindow::addPrivateStatusMessage_gui(Msg::TypeMsg typemsg, string cid, string message)
{
	BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);

	if (entry != NULL)
		dynamic_cast<PrivateMessage*>(entry)->addStatusMessage_gui(message, typemsg);
}

void MainWindow::showPublicHubs_gui()
{
	showBook<Entry::EntryType, BookEntry *>(Entry::PUBLIC_HUBS, new PublicHubs());
	setStatusOfIcons(PUBLIC_HUBS,true);
}

void MainWindow::showShareBrowser_gui(UserPtr user, string filename, string dir, int64_t speed ,bool useSetting)
{
	bool raise = useSetting ? !SETTING(POPUNDER_FILELIST) : TRUE;
	BookEntry *entry = findBookEntry(Entry::SHARE_BROWSER, user->getCID().toBase32());

	if (entry == NULL)
	{
		entry = new ShareBrowser(user, filename, dir, speed);
		addBookEntry_gui(entry);
	}

	if (raise)
		raisePage_gui(entry->getContainer());
}

Search *MainWindow::addSearch_gui()
{
	Search *entry = new Search();
	addBookEntry_gui(entry);
	raisePage_gui(entry->getContainer());
	
	search.push_back(entry);
	return entry;
}

void MainWindow::addSearch_gui(string magnet)
{
	string name;
	int64_t size;
	string tth;

	if (WulforUtil::splitMagnet(magnet, name, size, tth))
	{
		Search *s = addSearch_gui();
		s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	}
}

void MainWindow::fileToDownload_gui(string magnet, string path)
{
	string name;
	int64_t size;
	string tth;

	if (!WulforUtil::splitMagnet(magnet, name, size, tth))
		return;
	name = path + name;

	typedef Func3<MainWindow, string, int64_t, string> F3;
	F3 *func = new F3(this, &MainWindow::addFileDownloadQueue_client, name, size, tth);
	WulforManager::get()->dispatchClientFunc(func);
}

GtkWidget* MainWindow::getChooserDialog_gui()
{
	return getWidget("flistDialog");
}

void MainWindow::actionMagnet_gui(string magnet)
{
	if (GTK_WIDGET_VISIBLE(getWidget("MagnetDialog")))
		return;

	string name, tth;
	int64_t size;
	int action = WGETI("magnet-action");
	bool split = WulforUtil::splitMagnet(magnet, name, size, tth);

	if (action == 0 && split)
	{
		Search *s = addSearch_gui();
		s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	}
	else if (action == 1 && split)
	{
		name = WGETS("magnet-choose-dir") + name;

		if (!File::isAbsolute(name))
			name = SETTING(DOWNLOAD_DIRECTORY) + name;

		typedef Func3<MainWindow, string, int64_t, string> F3;
		F3 *func = new F3(this, &MainWindow::addFileDownloadQueue_client, name, size, tth);
		WulforManager::get()->dispatchClientFunc(func);
	}
	else if (split)
	{
		showMagnetDialog_gui(magnet, name, size, tth);
	}
}

void MainWindow::updateFavoriteHubMenu_client(const FavoriteHubEntryList &fh)
{
	ListParamPair list;
	for (FavoriteHubEntryList::const_iterator it = fh.begin(); it != fh.end(); ++it)
	{
		ParamPair param;
		FavoriteHubEntry *entry = *it;
		param.first = entry->getServer();
		param.second = entry->getEncoding();
		list.push_back(param);
	}

	Func1<MainWindow, ListParamPair> *func = new Func1<MainWindow, ListParamPair>(this, &MainWindow::updateFavoriteHubMenu_gui, list);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::updateFavoriteHubMenu_gui(ListParamPair list)
{
	GtkWidget *menu = gtk_menu_tool_button_get_menu(GTK_MENU_TOOL_BUTTON(getWidget("favHubs")));
	gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback)gtk_widget_destroy, NULL);

	for (ListParamPair::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		const ParamPair &param = *it;
		string address = param.first;
		string encoding = param.second;
		GtkWidget *item = gtk_menu_item_new_with_label(address.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		g_object_set_data_full(G_OBJECT(item), "address", g_strdup(address.c_str()), g_free);
		g_object_set_data_full(G_OBJECT(item), "encoding", g_strdup(encoding.c_str()), g_free);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onHubClicked_gui), (gpointer)this);
	}
	gtk_widget_show_all(menu);
}

void MainWindow::onHubClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string address = (gchar *)g_object_get_data(G_OBJECT(widget), "address");
	string encoding = (gchar *)g_object_get_data(G_OBJECT(widget), "encoding");
	mw->showHub_gui(address, encoding);
}

void MainWindow::setToolbarButton_gui()
{
	if (!WGETB("toolbar-button-connect"))
		gtk_widget_hide(getWidget("connect"));
	if (!WGETB("toolbar-button-fav-hubs"))
		gtk_widget_hide(getWidget("favHubs"));
	if (!WGETB("toolbar-button-fav-users"))
		gtk_widget_hide(getWidget("favUsers"));
	if (!WGETB("toolbar-button-public-hubs"))
		gtk_widget_hide(getWidget("publicHubs"));
	if (!WGETB("toolbar-button-settings"))
		gtk_widget_hide(getWidget("settings"));
	if (!WGETB("toolbar-button-hash"))
		gtk_widget_hide(getWidget("hash"));
	if (!WGETB("toolbar-button-search"))
		gtk_widget_hide(getWidget("search"));
	if (!WGETB("toolbar-button-search-spy"))
		gtk_widget_hide(getWidget("searchSpy"));
	if (!WGETB("toolbar-button-search-adl"))
		gtk_widget_hide(getWidget("searchADL"));
	if (!WGETB("toolbar-button-queue"))
		gtk_widget_hide(getWidget("queue"));
	if (!WGETB("toolbar-button-quit"))
		gtk_widget_hide(getWidget("quit"));
	if (!WGETB("toolbar-button-finished-downloads"))
		gtk_widget_hide(getWidget("finishedDownloads"));
	if (!WGETB("toolbar-button-finished-uploads"))
		gtk_widget_hide(getWidget("finishedUploads"));
	//[BMDC
	if (!WGETB("toolbar-button-ignore"))
		gtk_widget_hide(getWidget("ignUser"));
	if (!WGETB("toolbar-button-notepad"))
		gtk_widget_hide(getWidget("notepad"));
	if (!WGETB("toolbar-button-system"))
		gtk_widget_hide(getWidget("system"));
	if (!WGETB("toolbar-button-away"))
		gtk_widget_hide(getWidget("AwayIcon"));
	if (!WGETB("toolbar-button-limiting"))
		gtk_widget_hide(getWidget("limitingButton"));		
	//]	
}

void MainWindow::setTabPosition_gui(int position)
{
	GtkPositionType tabPosition;

	switch (position)
	{
		case 0:
			tabPosition = GTK_POS_TOP;
			break;
		case 1:
			tabPosition = GTK_POS_LEFT;
			break;
		case 2:
			tabPosition = GTK_POS_RIGHT;
			break;
		case 3:
			tabPosition = GTK_POS_BOTTOM;
			break;
		default:
			tabPosition = GTK_POS_TOP;
	}

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(getWidget("book")), tabPosition);
}

void MainWindow::setToolbarStyle_gui(int style)
{
	GtkToolbarStyle toolbarStyle;

	switch (style)
	{
		case 1:
			toolbarStyle = GTK_TOOLBAR_TEXT;
			break;
		case 2:
			toolbarStyle = GTK_TOOLBAR_BOTH;
			break;
		case 3:
			toolbarStyle = GTK_TOOLBAR_BOTH_HORIZ;
			break;
		case 4:
			gtk_widget_hide(getWidget("toolbar1"));
			return;
		case 0:
		default:
			toolbarStyle = GTK_TOOLBAR_ICONS;
	}

	if (style != 4)
	{
		gtk_widget_show(getWidget("toolbar1"));
		gtk_toolbar_set_style(GTK_TOOLBAR(getWidget("toolbar1")), toolbarStyle);
	}
}

bool MainWindow::getUserCommandLines_gui(const string &commands, ParamMap &ucParams)
{
	MainWindow *mw = WulforManager::get()->getMainWindow();
	GtkDialog *dialog =  GTK_DIALOG(gtk_dialog_new_with_buttons ("User Commands Dialog",
                                         GTK_WINDOW(mw->getContainer()),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_CANCEL,
                                         NULL));
                                         
     GtkWidget *content_area = gtk_dialog_get_content_area (dialog);
     GtkWidget *table = gtk_table_new(1,2,FALSE);
     gtk_container_add(GTK_CONTAINER(content_area), table);                                                                                 

	string::size_type i = 0;
	StringList names;
	
	vector<Widgets*> WidgList;
	while((i = commands.find("%[line:", i)) != string::npos) {
		i += 7;
		string::size_type j = commands.find(']', i);
		if(j == string::npos)
			break;
		if(j == string::npos && i == string::npos)
			break;	

		const string name = commands.substr(i, j - i);
		if(find(names.begin(), names.end(), name) == names.end()) {
			string caption = name;
			/*if(uc->adc()) {
				Util::replace("\\\\", "\\", caption);
				Util::replace("\\s", " ", caption);
			}
		*/
			// let's break between slashes (while ignoring double-slashes) to see if it's a combo
			int combo_sel = -1;
			string combo_caption = caption;
			Util::replace("//", "\t", combo_caption);
			StringList combo_values = StringTokenizer<string>(combo_caption, '/').getTokens();
			if(combo_values.size() > 2) { // must contain at least: caption, default sel, 1 value

				auto first = combo_values.begin();
				combo_caption = *first;
				combo_values.erase(first);

				auto sec = combo_values.begin();
				combo_sel = Util::toInt(Text::fromT(*sec)); 
				if(combo_values.empty())
				{ combo_sel = -1; }
				
				combo_values.erase(first);
				if(static_cast<size_t>(combo_sel) >= combo_values.size())
					combo_sel = -1; // default selection value too high
			}

			if(combo_sel >= 0) {
				for(auto i = combo_values.begin(), iend = combo_values.end(); i != iend; ++i)
					Util::replace("\t", "/", *i);

				// if the combo has already been displayed before, retrieve the prev value and bypass combo_sel
				auto prev = find(combo_values.begin(), combo_values.end(), boost::get<string>(ucParams["line:" + name]));
				if(prev != combo_values.end())
					combo_sel = prev - combo_values.begin();
					
				GtkWidget *comboBox = gtk_combo_box_new_text();
				
				while(!combo_values.empty()) {
					gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), combo_values.back().c_str() );
					combo_values.pop_back();
				}
				gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox),combo_sel);
				GtkWidget *label =  gtk_label_new(combo_caption.c_str());
				Widgets *wid = new Widgets();
				wid->widget = comboBox;
				wid->label = label;
				WidgList.push_back(wid);
				guint row = 0; guint acolums = 0;
				gtk_table_get_size(GTK_TABLE(table),&row,&acolums);
				if(row > 1 && acolums > 1)
					gtk_table_resize(GTK_TABLE(table),row+1,acolums+1);
				gtk_table_attach_defaults(GTK_TABLE(table),label, acolums, acolums+1,row,row+1);
				gtk_table_attach_defaults(GTK_TABLE(table),comboBox, acolums+1, acolums+2,row,row+1);
				gtk_widget_show(label);
				gtk_widget_show(comboBox);
				
		  } else {
				GtkWidget *label = gtk_label_new(caption.c_str());
				GtkWidget *entry = gtk_entry_new();
				gtk_entry_set_text(GTK_ENTRY(entry),boost::get<string>(ucParams["line:"+ name]).c_str());
				Widgets *wid = new Widgets();
				wid->widget = entry;
				wid->label = label;
				WidgList.push_back(wid);
				guint row = 0;guint acolums= 0;
				gtk_table_get_size(GTK_TABLE(table),&row,&acolums);
				if(row > 1 && acolums > 1)
					gtk_table_resize(GTK_TABLE(table),row+1,acolums+1);
				gtk_table_attach_defaults(GTK_TABLE(table),label,acolums,acolums+1,row,row+1);
				gtk_table_attach_defaults(GTK_TABLE(table),entry,acolums+1,acolums+2,row,row+1);	
								
				gtk_widget_show(label);
				gtk_widget_show(entry);
			}
			gtk_widget_show_all(table);
			names.push_back(name);
		}
		i = j + 1;
	}

	if(names.empty())
		return true;
	gint response = gtk_dialog_run(dialog);

   // Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return false;
	if (response == GTK_RESPONSE_OK)
	{
		for(size_t i = 0, iend = WidgList.size(); i < iend; ++i) 
		{
			Widgets *wid = WidgList[i];
			
			if(wid->label != NULL)
				const gchar * name = gtk_label_get_text(GTK_LABEL(wid->label));
			else continue;
		   if(wid->widget == NULL)
				continue;					
			if(GTK_IS_COMBO_BOX(wid->widget))
			{
				const gchar *value = gtk_combo_box_get_active_text(GTK_COMBO_BOX(wid->widget));	
				ucParams["line:"+names[i]] = string (value);
			}
			else if( GTK_IS_ENTRY(wid->widget))
			{
				const gchar *value = gtk_entry_get_text(GTK_ENTRY(wid->widget));
				ucParams["line:"+names[i]] = string(value);
			}
			else { return true;}
			
			WidgList.pop_back();
		}
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return true;
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	return false;
	/*
	string name;
	string label;
	string line;
	StringMap done;
	string::size_type i = 0;
	string::size_type j = 0;
	string text = string("<b>") + _("Enter value for ") + "\'";
	MainWindow *mw = WulforManager::get()->getMainWindow();

	while ((i = command.find("%[line:", i)) != string::npos)
	{
		i += 7;
		j = command.find(']', i);
		if (j == string::npos)
			break;

		name = command.substr(i, j - i);
		
		
		if (done.find(name) == done.end())
		{
			line.clear();
			label = text + name + "\'</b>";

			gtk_label_set_label(GTK_LABEL(mw->getWidget("ucLabel")), label.c_str());
			gtk_entry_set_text(GTK_ENTRY(mw->getWidget("ucLineEntry")), "");
			gtk_widget_grab_focus(mw->getWidget("ucLineEntry"));

			gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("ucLineDialog")));

			// Fix crash, if the dialog gets programmatically destroyed.
			if (response == GTK_RESPONSE_NONE)
				return false;

			gtk_widget_hide(mw->getWidget("ucLineDialog"));

			if (response == GTK_RESPONSE_OK)
				line = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("ucLineEntry")));

			if (!line.empty())
			{
				ucParams["line:" + name] = line;
				done[name] = line;
			}
			else
				return false;
		}
		i = j + 1;
	}
*/
//	return true;
}

void MainWindow::propertiesMagnetDialog_gui(string magnet)
{
	string name;
	int64_t size;
	string tth;

	if (WulforUtil::splitMagnet(magnet, name, size, tth))
		showMagnetDialog_gui(magnet, name, size, tth);
}

void MainWindow::showMagnetDialog_gui(const string &magnet, const string &name, const int64_t size, const string &tth)
{
	gtk_window_set_title(GTK_WINDOW(getWidget("MagnetDialog")), _("Magnet Properties / Choice"));
	// entry
	gtk_entry_set_text(GTK_ENTRY(getWidget("magnetEntry")), magnet.c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("magnetNameEntry")), name.c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("magnetSizeEntry")), Util::formatBytes(size).c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("exactSizeEntry")), Util::formatExactSize(size).c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("tthEntry")), tth.c_str());
	// chooser dialog
	GtkWidget *chooser = getWidget("flistDialog");
	gtk_window_set_title(GTK_WINDOW(chooser), _("Choose a directory"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());
	// choose
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("choiceCheckButton")), FALSE);
	setChooseMagnetDialog_gui();

	gtk_widget_show_all(getWidget("MagnetDialog"));
}

void MainWindow::setChooseMagnetDialog_gui()
{
	switch (WGETI("magnet-action"))
	{
		case 0: // start a search for this file
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("searchRadioButton")), TRUE);
			gtk_widget_set_sensitive(getWidget("browseButton"), FALSE);
		break;

		case 1: // add this file to your download queue
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("dowloadQueueRadioButton")), TRUE);
			gtk_widget_set_sensitive(getWidget("browseButton"), TRUE);
		break;

		default: // show magnet dialog
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("showRadioButton")), TRUE);
			gtk_widget_set_sensitive(getWidget("browseButton"), FALSE);
	}
}

void MainWindow::onBrowseMagnetButton_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	GtkWidget *dialog = mw->getWidget("flistDialog");
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(dialog);
}

void MainWindow::onDowloadQueueToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gtk_widget_set_sensitive(mw->getWidget("browseButton"), TRUE);
}

void MainWindow::onSearchMagnetToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gtk_widget_set_sensitive(mw->getWidget("browseButton"), FALSE);
}

void MainWindow::onSetMagnetChoiceDialog_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	// magnet choice frame
	gtk_window_set_title(GTK_WINDOW(mw->getWidget("MagnetDialog")), _("Magnet Choice"));
	gtk_widget_hide(mw->getWidget("setHBox"));
	gtk_widget_hide(mw->getWidget("magnetPropertiesFrame"));
	gtk_widget_hide(mw->getWidget("hseparator1"));

	// chooser dialog
	GtkWidget *chooser = mw->getWidget("flistDialog");
	gtk_window_set_title(GTK_WINDOW(chooser), _("Choose a directory"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());

	mw->setChooseMagnetDialog_gui();

	gtk_widget_show(mw->getWidget("MagnetDialog"));
}

void MainWindow::onResponseMagnetDialog_gui(GtkWidget *dialog, gint response, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (response == GTK_RESPONSE_OK)
	{
		string path;

		// magnet choice frame
		if (!GTK_WIDGET_VISIBLE(mw->getWidget("magnetPropertiesFrame")))
		{
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("dowloadQueueRadioButton"))))
			{
				gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
				if (temp)
				{
					path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
					g_free(temp);
				}
				if (!File::isAbsolute(path))
					path = SETTING(DOWNLOAD_DIRECTORY);

				WSET("magnet-choose-dir", path);
				WSET("magnet-action", 1);
			}
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("searchRadioButton"))))
				WSET("magnet-action", 0);
			else
				WSET("magnet-action", -1);

			gtk_widget_hide(dialog);

			return;
		}

		// magnet properties plus choice frame
		string name, tth;
		int64_t size;
		string magnet = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("magnetEntry")));

		WulforUtil::splitMagnet(magnet, name, size, tth);
		gboolean set = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("choiceCheckButton")));

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("dowloadQueueRadioButton"))))
		{
			gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
			if (temp)
			{
				path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
				g_free(temp);
			}

			if (!File::isAbsolute(path))
				path = SETTING(DOWNLOAD_DIRECTORY);

			if (set)
			{
				WSET("magnet-action", 1);
				WSET("magnet-choose-dir", path);
			}

			// add this file to your download queue
			typedef Func3<MainWindow, string, int64_t, string> F3;
			F3 *func = new F3(mw, &MainWindow::addFileDownloadQueue_client, path + name, size, tth);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("searchRadioButton"))))
		{
			if (set)
				WSET("magnet-action", 0);

			// start a search for this file
			Search *s = mw->addSearch_gui();
			s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
		}
		else if (set)
			WSET("magnet-action", -1);
	}

	gtk_widget_hide(dialog);
}

void MainWindow::addFileDownloadQueue_client(string name, int64_t size, string tth)
{
	try
	{
		if (!tth.empty())
		{
			QueueManager::getInstance()->add(name, size, TTHValue(tth) , HintedUser(new User(CID()),Util::emptyString));

			// automatically search for alternative download locations
			if (SETTING(AUTO_SEARCH))
				SearchManager::getInstance()->search(tth, 0, SearchManager::TYPE_TTH, SearchManager::SIZE_DONTCARE,
					Util::emptyString);
		}
	}
	catch (const Exception& e)
	{
		// add error to main status
		typedef Func2<MainWindow, string, time_t> F2;
		F2 *func = new F2(this, &MainWindow::setMainStatus_gui, e.getError(), time(NULL));
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::showMessageDialog_gui(const string primaryText, const string secondaryText)
{
	if (primaryText.empty())
		return;

	GtkWidget* dialog = gtk_message_dialog_new(window, GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", primaryText.c_str());

	if (!secondaryText.empty())
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondaryText.c_str());

	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_widget_show(dialog);
}

gboolean MainWindow::onWindowState_gui(GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (!mw->minimized && event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_WITHDRAWN))
	{
		mw->minimized = TRUE;
		if (SETTING(SettingsManager::AUTO_AWAY) && !Util::getAway())
			Util::setAway(TRUE);
	}
	else if (mw->minimized && (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED ||
		event->new_window_state == 0))
	{
		mw->minimized = FALSE;
		if (SETTING(SettingsManager::AUTO_AWAY) && !Util::getManualAway())
			Util::setAway(FALSE);
	}

	return TRUE;
}

gboolean MainWindow::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *child = mw->currentPage_gui();

	if (child != NULL)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");
		entry->setActive_gui();
	}

	gtk_window_set_urgency_hint(mw->window, FALSE);
	return FALSE;
}

gboolean MainWindow::onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (mw->onQuit)
	{
		mw->onQuit = FALSE;
	}
	else if (WGETB("main-window-no-close") && SETTING(ALWAYS_TRAY))
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconShowInterfaceItem")), FALSE);

		return TRUE;
	}

	if (!SETTING(CONFIRM_EXIT))
	{
		WulforManager::get()->deleteMainWindow();
		return FALSE;
	}

	gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("exitDialog")));
	gtk_widget_hide(mw->getWidget("exitDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		WulforManager::get()->deleteMainWindow();
		return FALSE;
	}

	return TRUE;
}

gboolean MainWindow::onDeleteEventMagnetDialog_gui(GtkWidget *dialog, GdkEvent *event, gpointer data)
{
	gtk_widget_hide(dialog);
	return TRUE;
}

void MainWindow::onTopToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	GtkWidget *parent = mw->getWidget("hbox4");
	GtkWidget *child = mw->getWidget("toolbar1");
	if (child->parent != GTK_WIDGET(parent))
		return;
	g_object_ref(child);
	gtk_container_remove(GTK_CONTAINER(parent), child);
	parent = mw->getWidget("vbox1");
	gtk_toolbar_set_orientation(GTK_TOOLBAR(child), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(parent), child, FALSE, FALSE, 2);
	gtk_box_reorder_child(GTK_BOX(parent), child, 1);
	g_object_unref(child);
	WSET("toolbar-position", 1);//0
}

void MainWindow::onLeftToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	GtkWidget *parent = mw->getWidget("vbox1");
	GtkWidget *child = mw->getWidget("toolbar1");
	if (child->parent != GTK_WIDGET(parent))
		return;
	g_object_ref(child);
	gtk_container_remove(GTK_CONTAINER(parent), child);
	parent = mw->getWidget("hbox4");
	gtk_toolbar_set_orientation(GTK_TOOLBAR(child), GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start(GTK_BOX(parent), child, FALSE, FALSE, 2);
	gtk_box_reorder_child(GTK_BOX(parent), child, 0);
	g_object_unref(child);
	WSET("toolbar-position", 0);//1
}

void MainWindow::onHideToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mw->getWidget("hideToolbarItem")));
	if (active)
	{
		gtk_widget_hide(mw->getWidget("toolbar1"));
		mw->ToolbarStyle = WGETI("toolbar-style");
		WSET("toolbar-style", 4);
	}
	else
	{
		gtk_widget_show(mw->getWidget("toolbar1"));
		WSET("toolbar-style", mw->ToolbarStyle);
	}
}

void MainWindow::onSizeToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mw->getWidget("sizeToolbarItem")));
	GtkWidget *toolbar = mw->getWidget("toolbar1");
	GtkIconSize size;
	if (active)
	{
		WSET("toolbar-small", TRUE);
		size = GTK_ICON_SIZE_SMALL_TOOLBAR;
	}
	else
	{
		WSET("toolbar-small", FALSE);
		size = GTK_ICON_SIZE_LARGE_TOOLBAR;
	}
	g_object_set(G_OBJECT(toolbar), "icon-size", size, NULL);
}

gboolean MainWindow::onAddButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	gtk_menu_popup(GTK_MENU(mw->getWidget("toolbarMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	return FALSE;
}

void MainWindow::onToolToggled_gui(GtkWidget *widget, gpointer data)
{
	string key = (gchar*) g_object_get_data(G_OBJECT(widget), "key");
	GtkWidget *button = (GtkWidget*) data;
	bool active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	active ? gtk_widget_show(button) : gtk_widget_hide(button);
	WSET(key, active);
}

void MainWindow::checkToolbarMenu_gui()
{
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("connectMenuItemBar")), WGETB("toolbar-button-connect"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("favHubsMenuItemBar")), WGETB("toolbar-button-fav-hubs"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("favUsersMenuItemBar")), WGETB("toolbar-button-fav-users"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("publicHubsMenuItemBar")), WGETB("toolbar-button-public-hubs"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("settingsMenuItemBar")), WGETB("toolbar-button-settings"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("hashMenuItemBar")), WGETB("toolbar-button-hash"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("searchMenuItemBar")), WGETB("toolbar-button-search"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("searchSpyMenuItemBar")), WGETB("toolbar-button-search-spy"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("searchADLMenuItemBar")), WGETB("toolbar-button-search-adl"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("queueMenuItemBar")), WGETB("toolbar-button-queue"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("finishedDownloadsMenuItemBar")), WGETB("toolbar-button-finished-downloads"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("finishedUploadsMenuItemBar")), WGETB("toolbar-button-finished-uploads"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("quitMenuItemBar")), WGETB("toolbar-button-quit"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("ignUserMenuItemBar")),WGETB("toolbar-button-ignore"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("hideToolbarItem")), ((ToolbarStyle = WGETI("toolbar-style")) == 4) ? TRUE : FALSE);
}

gboolean MainWindow::onKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (event->state & GDK_CONTROL_MASK)
	{
		if (event->state & GDK_SHIFT_MASK && event->keyval == GDK_ISO_Left_Tab)
		{
			mw->previousTab_gui();
			return TRUE;
		}
		else if (event->keyval == GDK_Tab)
		{
			mw->nextTab_gui();
			return TRUE;
		}
	}

	return FALSE;
}

gboolean MainWindow::onButtonReleasePage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gint width, height;
	gdk_drawable_get_size(event->window, &width, &height);

	// If middle mouse button was released when hovering over tab label
	// with setting to it
	if ( (!WGETB("book-three-button-disable")) &&  (event->button == 2 && event->x >= 0 && event->y >= 0
		&& event->x < width && event->y < height))
	{
		BookEntry *entry = (BookEntry *)data;
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
		return TRUE;
	}

	return FALSE;
}

gboolean MainWindow::animationStatusIcon_gui(gpointer data)
{
	MainWindow *mw = (MainWindow *) data;

	if (mw->isActive_gui())
	{
		gtk_status_icon_set_from_icon_name(mw->statusIcon, g_get_prgname());
		mw->timer = 0;

		return FALSE;
	}

	gtk_status_icon_set_from_icon_name(mw->statusIcon, (mw->statusFrame *= -1) > 0 ? "bmdc" : "bmdc-normal");

	return TRUE;
}

void MainWindow::onRaisePage_gui(GtkMenuItem *item, gpointer data)
{
	WulforManager::get()->getMainWindow()->raisePage_gui((GtkWidget *)data);
}

void MainWindow::onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
	MainWindow* mw = (MainWindow*)data;
	GtkWidget *child = gtk_notebook_get_nth_page(notebook, num);
	BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");

	if (entry)
	{
		// Disable "activate" signal on the tab menu item since it can cause
		// onPageSwitched_gui to be called multiple times
		GtkWidget *item = entry->getTabMenuItem();
		g_signal_handlers_block_by_func(item, (gpointer)onRaisePage_gui, child);

		entry->setActive_gui();
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(entry->getTabMenuItem()), TRUE);
		mw->setTitle(entry->getLabelText()); // Update window title with selected tab label

		g_signal_handlers_unblock_by_func(item, (gpointer)onRaisePage_gui, (gpointer)child);
	}

	GList *list = (GList *)g_object_get_data(G_OBJECT(notebook), "page-rotation-list");
	list = g_list_remove(list, (gpointer)child);
	list = g_list_prepend(list, (gpointer)child);
	g_object_set_data(G_OBJECT(notebook), "page-rotation-list", (gpointer)list);

	// Focus the tab so it will focus its children (e.g. a text entry box)
	gtk_widget_grab_focus(child);
}

void MainWindow::onPaneRealized_gui(GtkWidget *pane, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gint position = WGETI("transfer-pane-position");

	if (position > 10)
	{
		// @todo: fix get window height when maximized
		gint height;
		gtk_window_get_size(mw->window, NULL, &height);
		gtk_paned_set_position(GTK_PANED(pane), height - position);
	}
}

void MainWindow::onConnectClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	gtk_editable_select_region(GTK_EDITABLE(mw->getWidget("connectEntry")), 0, -1);
	gtk_widget_grab_focus(mw->getWidget("connectEntry"));

	gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("connectDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(mw->getWidget("connectDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		string address = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("connectEntry")));
		mw->showHub_gui(address);
	}
}

void MainWindow::onFavoriteHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFavoriteHubs_gui();
}

void MainWindow::onFavoriteUsersClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFavoriteUsers_gui();
}
//BMDC++
void MainWindow::onIgnoreUserClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showIgnoreUsers_gui();
}

void MainWindow::onCmdDebugClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showCmdDebug_gui();
}

void MainWindow::onSystemLogClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showSystemLog_gui();
}

void MainWindow::onNotepadClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showNotepad_gui();	
}

void MainWindow::onUploadQueueClicked_gui(GtkWidget *widget , gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showUploadQueue_gui();	
}

void MainWindow::onRecentHubClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showRecentHubs_gui();	
}

void MainWindow::onDetectionClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;	
	mw->showDetection_gui();
}
//]
void MainWindow::onPublicHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showPublicHubs_gui();
}

void MainWindow::onPreferencesClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func0<MainWindow> F0;
	
	auto prevTCP = SETTING(TCP_PORT);
	auto prevUDP = SETTING(UDP_PORT);
	auto prevTLS = SETTING(TLS_PORT);

	auto prevConn = SETTING(INCOMING_CONNECTIONS);
	auto prevMapper = SETTING(MAPPER);
	auto prevBind = SETTING(BIND_ADDRESS);
	auto prevBind6 = SETTING(BIND_ADDRESS6);
	auto prevProxy = CONNSETTING(OUTGOING_CONNECTIONS);

	if (mw->useStatusIconBlink != WGETB("status-icon-blink-use"))
		WSET("status-icon-blink-use", mw->useStatusIconBlink);
	bool emoticons = WGETB("emoticons-use");

	gint response = WulforManager::get()->openSettingsDialog_gui();

	if (response == GTK_RESPONSE_OK)
	{
		//NOTE BMDC >=0.785+
		try {
			ConnectivityManager::getInstance()->setup(SETTING(INCOMING_CONNECTIONS) != prevConn ||
				SETTING(TCP_PORT) != prevTCP || SETTING(UDP_PORT) != prevUDP || SETTING(TLS_PORT) != prevTLS ||
				SETTING(MAPPER) != prevMapper || SETTING(BIND_ADDRESS) != prevBind || SETTING(BIND_ADDRESS6) != prevBind6);
		} catch (const Exception& e) {
			mw->showMessageDialog_gui(e.getError(),e.getError());
		}

		auto outConns = CONNSETTING(OUTGOING_CONNECTIONS);
		if(outConns != prevProxy || outConns == SettingsManager::OUTGOING_SOCKS5) {
			Socket::socksUpdated();
		}
		//END
		
		if (SETTING(ALWAYS_TRAY))
			gtk_status_icon_set_visible(mw->statusIcon, TRUE);
		else
			gtk_status_icon_set_visible(mw->statusIcon, FALSE);

		mw->setTabPosition_gui(WGETI("tab-position"));
		mw->setToolbarStyle_gui(WGETI("toolbar-style"));

		// Reload the icons only if the setting has changed
		mw->loadIcons_gui();

		// All hubs and PMs
		for (StringIterC it = mw->EntryList.begin(); it != mw->EntryList.end(); ++it)
		{
			BookEntry *entry = mw->findBookEntry(Entry::HUB, *it);

			if (entry != NULL)
				dynamic_cast<Hub*>(entry)->preferences_gui();
			else
			{
				entry = mw->findBookEntry(Entry::PRIVATE_MESSAGE, *it);

				if (entry != NULL)
					dynamic_cast<PrivateMessage*>(entry)->preferences_gui();
			}
		}

		// Search Spy
		BookEntry *entry = mw->findBookEntry(Entry::SEARCH_SPY);

		if (entry != NULL)
			dynamic_cast<SearchSpy *>(entry)->preferences_gui();

		// Status menu
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconBlinkUseItem")), WGETB("status-icon-blink-use"));

		// Emoticons
		if (emoticons != WGETB("emoticons-use"))
			Emoticons::get()->reloadPack_gui();

		// Toolbar
		mw->checkToolbarMenu_gui();

		// Rate
		mw->setStatRate_gui();//NOTE: core 0.762
	}
}

void MainWindow::onAwayClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func1<MainWindow, bool> F1;
	
	if(Util::getAway())
	{
		Util::switchAway();
		Util::setManualAway(false);
		mw->setMainStatus_gui(_("Away mode off"), time(NULL));
		
		F1 *func = new F1(mw,&MainWindow::setAwayIcon, false);
		WulforManager::get()->dispatchGuiFunc(func);
		
	}else
	{
		Util::switchAway();
		Util::setManualAway(true);
		mw->setMainStatus_gui(_("Away mode on"), time(NULL));
		F1 *func = new F1(mw,&MainWindow::setAwayIcon, true);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::onTransferToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *transfer = mw->transfers->getContainer();

	if (GTK_WIDGET_VISIBLE(transfer))
		gtk_widget_hide(transfer);
	else
		gtk_widget_show_all(transfer);
}

void MainWindow::onHashClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->openHashDialog_gui();
}

void MainWindow::onSearchClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->addSearch_gui();
}

void MainWindow::onSearchSpyClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showSearchSpy_gui();
}

void MainWindow::onSearchADLClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showSearchADL_gui();
}

void MainWindow::onDownloadQueueClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showDownloadQueue_gui();
}

void MainWindow::onFinishedDownloadsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFinishedDownloads_gui();
}

void MainWindow::onFinishedUploadsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFinishedUploads_gui();
}

void MainWindow::onQuitClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	// fix emit signal (status-icon menu quit)
	if (GTK_WIDGET_VISIBLE(mw->getWidget("exitDialog")))
		return;

	mw->onQuit = TRUE;
	gboolean retVal; // Not interested in the value, though.
	g_signal_emit_by_name(mw->window, "delete-event", NULL, &retVal);
}

void MainWindow::onOpenFileListClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	GtkWidget *chooser = mw->getWidget("flistDialog");
	gtk_window_set_title(GTK_WINDOW(chooser), _("Select filelist to browse"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::fromUtf8(Util::getListPath()).c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(chooser));

	// if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(chooser);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));

		if (temp)
		{
			string path = Text::toUtf8(temp);
			g_free(temp);

			UserPtr user = DirectoryListing::getUserFromFilename(path);
			if (user)
				mw->showShareBrowser_gui(user, path, "", 0, FALSE);
			else
				mw->setMainStatus_gui(_("Unable to load file list: Invalid file list name"));
		}
	}
}

void MainWindow::onOpenOwnListClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func1<MainWindow, bool> F1;
	F1 *func = new F1(mw, &MainWindow::openOwnList_client, FALSE);
	WulforManager::get()->dispatchClientFunc(func);

	mw->setMainStatus_gui(_("Loading Own file list"));
}

void MainWindow::onRefreshFileListClicked_gui(GtkWidget *widget, gpointer data)
{
	typedef Func0<MainWindow> F0;
	F0 *func = new F0((MainWindow *)data, &MainWindow::refreshFileList_client);
	WulforManager::get()->dispatchClientFunc(func);
}

void MainWindow::onReconnectClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *entryWidget = mw->currentPage_gui();

	if (entryWidget)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

		if (entry && entry->getType() == Entry::HUB)
		{
			Func0<Hub> *func = new Func0<Hub>(dynamic_cast<Hub *>(entry), &Hub::reconnect_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void MainWindow::onCloseClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *entryWidget = mw->currentPage_gui();

	if (entryWidget)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

		if (entry)
			mw->removeBookEntry_gui(entry);
	}
}

void MainWindow::onPreviousTabClicked_gui(GtkWidget* widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->previousTab_gui();
}

void MainWindow::onNextTabClicked_gui(GtkWidget* widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->nextTab_gui();
}

void MainWindow::onAboutClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("aboutDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(mw->getWidget("aboutDialog"));
}

void MainWindow::onAboutDialogActivateLink_gui(GtkAboutDialog *dialog, const gchar *link, gpointer data)
{
	MainWindow *mw =(MainWindow *)data;
	string error = Util::emptyString;
	WulforUtil::openURI(link,error);
	if(!error.empty())
	    mw->setMainStatus_gui(error, time(NULL));
}

void MainWindow::onCloseBookEntry_gui(GtkWidget *widget, gpointer data)
{
	BookEntry *entry = (BookEntry *)data;
	WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void MainWindow::onStatusIconActivated_gui(GtkStatusIcon *statusIcon, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconShowInterfaceItem"));

	// Toggle the "Show Interface" check menu item. This will in turn invoke its callback.
	gboolean active = gtk_check_menu_item_get_active(item);
	gtk_check_menu_item_set_active(item, !active);
}

void MainWindow::onStatusIconPopupMenu_gui(GtkStatusIcon *statusIcon, guint button, guint time, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkMenu *menu = GTK_MENU(mw->getWidget("statusIconMenu"));
	gtk_menu_popup(menu, NULL, NULL, gtk_status_icon_position_menu, statusIcon, button, time);
}

void MainWindow::onShowInterfaceToggled_gui(GtkCheckMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWindow *win = mw->window;
	static int x, y;
	static bool isMaximized, isIconified;

	if (GTK_WIDGET_VISIBLE(win))
	{
		GdkWindowState state;
		gtk_window_get_position(win, &x, &y);
		state = gdk_window_get_state(GTK_WIDGET(win)->window);
		isMaximized = (state & GDK_WINDOW_STATE_MAXIMIZED);
		isIconified = (state & GDK_WINDOW_STATE_ICONIFIED);
		gtk_widget_hide(GTK_WIDGET(win));
	}
	else
	{
		gtk_window_move(win, x, y);
		if (isMaximized) gtk_window_maximize(win);
		if (isIconified) gtk_window_iconify(win);
		gtk_widget_show(GTK_WIDGET(win));
	}
}

void MainWindow::onStatusIconBlinkUseToggled_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->removeTimerSource_gui();

	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconBlinkUseItem"))))
		mw->useStatusIconBlink = TRUE;
	else
		mw->useStatusIconBlink = FALSE;
}

void MainWindow::onLinkClicked_gui(GtkWidget *widget, gpointer data)
{
	string link = (gchar *)g_object_get_data(G_OBJECT(widget), "link");
	WulforUtil::openURI(link);
}

void MainWindow::autoConnect_client()
{
	const FavHubGroups &favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();
	const FavoriteHubEntryList &favoriteHubs = FavoriteManager::getInstance()->getFavoriteHubs();

	for (FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i)
	{
		FavoriteHubEntry *hub = *i;
		string group = hub->getGroup();
		FavHubGroups::const_iterator it = favHubGroups.find(group);

		if (it != favHubGroups.end())
		{
			HubSettings p = it->second;
			if (p.getConnect())
			{
				typedef Func2<MainWindow, string, string> F2;
				F2 *func = new F2(this, &MainWindow::showHub_gui, hub->getServer(), hub->getEncoding());
				WulforManager::get()->dispatchGuiFunc(func);
			}
		}
		
		if(hub->getAutoConnect())
		{
			typedef Func2<MainWindow, string, string> F2;
			F2 *func = new F2(this, &MainWindow::showHub_gui, hub->getServer(), hub->getEncoding());
			WulforManager::get()->dispatchGuiFunc(func);		
		}
	}

	string link = WulforManager::get()->getURL();

	if (link.empty()) return;

	typedef Func1<MainWindow, string> F1;
	F1 *func1;

	if (WulforUtil::isHubURL(link) && SETTING(URL_HANDLER))
	{
		typedef Func2<MainWindow, string, string> F2;
		F2 *func = new F2(this, &MainWindow::showHub_gui, link, "");//TODO: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if (WulforUtil::isMagnet(link) && SETTING(MAGNET_REGISTER))
	{
		func1 = new F1(this, &MainWindow::actionMagnet_gui, link);
		WulforManager::get()->dispatchGuiFunc(func1);
	}
}

void MainWindow::startSocket_client()
{
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();
	
	try {
		ConnectivityManager::getInstance()->setup(true);
	} catch (const Exception& e) {
		string error = e.getError();
		typedef Func2<MainWindow, string, string> F2;
		F2 *func = new F2(this, &MainWindow::showMessageDialog_gui, error, error);
		WulforManager::get()->dispatchGuiFunc(func);
	}

	ClientManager::getInstance()->infoUpdated();
}

void MainWindow::refreshFileList_client()
{
	try
	{
		ShareManager::getInstance()->setDirty();
		ShareManager::getInstance()->refresh(TRUE, TRUE, FALSE);
	}
	catch (const ShareException&)
	{
	}
}

void MainWindow::openOwnList_client(bool useSetting)
{
	UserPtr user = ClientManager::getInstance()->getMe();
	string path = ShareManager::getInstance()->getOwnListFile();

	typedef Func5<MainWindow, UserPtr, string, string, int64_t,bool> F5;
	F5 *func = new F5(this, &MainWindow::showShareBrowser_gui, user, path, "",0, useSetting);
	WulforManager::get()->dispatchGuiFunc(func);
}

int MainWindow::FileListQueue::run() {
	setThreadPriority(Thread::LOW);

	while(true) {
		s.wait(15000);
		if(stop || fileLists.empty()) {
			break;
		}

		DirectoryListInfo* i;
		{
			Lock l(cs);
			i = fileLists.front();
			fileLists.pop_front();
		}
		
		if(Util::fileExists(i->file)) {
			DirectoryListing* dl = new DirectoryListing(i->user);
			try {
				dl->loadFile(i->file);
				ADLSearchManager::getInstance()->matchListing(*dl);
				ClientManager::getInstance()->checkCheating(i->user, dl);
			} catch(...)
			{
				//...
			}
			delete dl;
		}
		delete i;
	}
	stop = true;
	return 0;
}

void MainWindow::on(LogManagerListener::Message, time_t t, const string &message) throw()
{
	typedef Func2<MainWindow, string, time_t> F2;
	F2 *func = new F2(this, &MainWindow::setMainStatus_gui, message, t);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw()
{
	if(item->isSet(QueueItem::FLAG_TESTSUR)) return;
	
	typedef Func3<MainWindow, string, string, Notify::TypeNotify> F3;

	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
	{
		const HintedUser user = item->getDownloads()[0]->getHintedUser();//NOTE: core 0.762
		string listName = item->getListName();

		F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("file list from "), WulforUtil::getNicks(user),
			Notify::DOWNLOAD_FINISHED_USER_LIST);

		WulforManager::get()->dispatchGuiFunc(f3);

		typedef Func5<MainWindow, UserPtr, string, string, int64_t ,bool> F5;
		F5 *func = new F5(this, &MainWindow::showShareBrowser_gui, user.user, listName, dir,avSpeed, TRUE);//NOTE: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if(item->isSet(QueueItem::FLAG_USER_LIST) && item->isSet(QueueItem::FLAG_CHECK_FILE_LIST))
	{
        DirectoryListInfo* i = new DirectoryListInfo(item->getDownloads()[0]->getHintedUser(), item->getListName(), dir, avSpeed);
        if(listQueue.stop) {
			listQueue.stop = false;
			listQueue.start();
		}
		{
			Lock l(listQueue.cs);
			listQueue.fileLists.push_back(i);
		}
		listQueue.s.signal();
	}
	else if (!item->isSet(QueueItem::FLAG_XML_BZLIST))
	{
		F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("<b>file:</b> "), item->getTarget(), Notify::DOWNLOAD_FINISHED);
		WulforManager::get()->dispatchGuiFunc(f3);
	}
}

void MainWindow::on(TimerManagerListener::Second, uint64_t ticks) noexcept
{
	Util::setUptime();
	// Avoid calculating status update if it's not needed
	if (!SETTING(ALWAYS_TRAY) && minimized)
		return;

	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : ticks - lastUpdate);
	int64_t downBytes = 0;
	int64_t upBytes = 0;
	int64_t downDiff = Socket::getTotalDown() - lastDown;
	int64_t upDiff = Socket::getTotalUp() - lastUp;

	if (diff > 0)
	{
		downBytes = (downDiff * 1000) / diff;
		upBytes = (upDiff * 1000) / diff;
	}

	string hubs = Client::getCounts();
	string downloadSpeed = Util::formatBytes(downBytes) + "/" + _("s");
	string downloaded = Util::formatBytes(Socket::getTotalDown());
	string uploadSpeed = Util::formatBytes(upBytes) + "/" + _("s");
	string uploaded = Util::formatBytes(Socket::getTotalUp());
	
	SettingsManager *sm = SettingsManager::getInstance();
	sm->set(SettingsManager::TOTAL_UPLOAD,   SETTING(TOTAL_UPLOAD)   + upDiff);
	sm->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downDiff);
	
	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	typedef Func5<MainWindow, string, string, string, string, string> F5;
	F5 *func = new F5(this, &MainWindow::setStats_gui, hubs, downloadSpeed, downloaded, uploadSpeed, uploaded);
	WulforManager::get()->dispatchGuiFunc(func);

	if (SETTING(ALWAYS_TRAY) && !downloadSpeed.empty() && !uploadSpeed.empty())
	{
		typedef Func2<MainWindow, string, string> F2;
		F2 *f2 = new F2(this, &MainWindow::updateStatusIconTooltip_gui, downloadSpeed, uploadSpeed);
		WulforManager::get()->dispatchGuiFunc(f2);
	}
}

void MainWindow::onTTHFileDialog_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw =(MainWindow *)data;
	GtkWidget *dialog = mw->getWidget("TTHFileDialog");
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_NONE)
	{ ; }
	gtk_widget_hide(dialog);
}

void MainWindow::onTTHFileButton_gui(GtkWidget *widget , gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *chooser = mw->getChooserDialog_gui();
	gtk_window_set_title(GTK_WINDOW(chooser), _("Select file to Get TTH"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), "/home/");

	gint response = gtk_dialog_run(GTK_DIALOG(chooser));

	// if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(chooser);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		gtk_widget_set_sensitive(mw->getWidget("buttonok"),FALSE);
		if(mw->hasht.stop)
		{
			mw->hasht.stop = false;
			Lock l(mw->hasht.cs);
			mw->hasht.mw = mw;
			mw->hasht.filename = temp;
			mw->hasht.start();
			mw->hasht.s.signal();
		}
	}
}

void MainWindow::back(string TTH, string filename, int64_t size)
{
	string magnetlink = "magnet:?xt=urn:tree:tiger:" + TTH + "&xl=" + Util::toString(size) + "&dn=" + Util::encodeURI(Text::fromT(Util::getFileName(filename)));
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrymagnet")), magnetlink.c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrytthfileresult")), TTH.c_str());	
}

void MainWindow::progress(bool progress)
{
	if(progress)
		gtk_progress_bar_pulse (GTK_PROGRESS_BAR (getWidget("progressbar")));
	else 
	{
		gtk_widget_set_sensitive(getWidget("buttonok"),TRUE);	
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")),"Done");	
	}
}

int MainWindow::TTHHash::run()
{
		setThreadPriority(Thread::LOW);
		while(true) {
				s.wait(15000);
				if(stop)
						break;
				string TTH;
				char *buf = new char[512*1024];
				int64_t sized = 0;
				try {
						File f(Text::fromT(filename),dcpp::File::READ, dcpp::File::OPEN);
						TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
						sized = f.getSize();
						if(f.getSize() > 0) {
						size_t n = 512*1024;
						while( (n = f.read(&buf[0], n)) > 0) {
							tth.update(&buf[0], n);
							n = 512*1024;
							typedef Func1<MainWindow, bool> F1;
							F1 *func = new F1(mw,&MainWindow::progress,true);
							WulforManager::get()->dispatchGuiFunc(func);
							
						}
					} else {
					tth.update("", 0);
				}
				tth.finalize();
				f.close();
				strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
				
				typedef Func3<MainWindow, std::string, std::string, int64_t> F3;
				F3 *func = new F3(mw,&MainWindow::back,TTH,filename,sized);
				WulforManager::get()->dispatchGuiFunc(func);
				typedef Func1<MainWindow, bool> F1;
				F1 *func1 = new F1(mw,&MainWindow::progress,false);
				WulforManager::get()->dispatchGuiFunc(func1);
			
			LogManager::getInstance()->message("TTH: " + TTH + "filename: " + filename + "sized: " + dcpp::Util::toString(sized));
			stop = true;
		} catch(...) { }	
	}
				
	stop = true;
	return 0;
}	

void MainWindow::checkUpdateofGeoIp(bool v6)
{
	try {
		File f(GeoManager::getInstance()->getDbPath(v6) + ".gz", File::READ, File::OPEN);
		if(f.getSize() > 0 && f.getLastModified() > GET_TIME() - 3600 * 24 * 16) {
			return;
		}
	} catch(const FileException&) { }
	updateGeoIp(v6);
}

void MainWindow::updateGeoIp(bool v6)
{
	string v6str = "http://geolite.maxmind.com/download/geoip/database/GeoIPv6.dat.gz";
	string v4str = "http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz";
	auto& conn = conns[v6 ? CONN_GEOIP_V6 : CONN_GEOIP_V4];
	if(conn.get())
		return;
	LogManager::getInstance()->message(string(_("Updating the GeoIP database...v")) + (v6 ? "IPv6" : "IPv4"));
	conn.reset(new HttpDownload(v6 ? v6str : v4str, [this, v6] { completeGeoIpUpdate(v6); }, false));
}

void MainWindow::completeGeoIpUpdate(bool v6)
{
	auto& conn = conns[v6 ? CONN_GEOIP_V6 : CONN_GEOIP_V4];

	if(!conn->buf.empty()) {
		try {
			File(GeoManager::getInstance()->getDbPath(v6) + ".gz", File::WRITE, File::CREATE | File::TRUNCATE).write(conn->buf);
			File f(GeoManager::getInstance()->getDbPath(v6), File::WRITE, File::CREATE | File::TRUNCATE); // clear the previous db
			LogManager::getInstance()->message(string(_("The GeoIP database has been successfully updated; restart DC++ to apply....v")) + (v6 ? "IPv6" : "IPv4"));
			GeoManager::getInstance()->update(v6);
			return;
		} catch(const FileException&) {}
	}
	LogManager::getInstance()->message(string(_("The GeoIP database could not be updated...v")) + (v6 ? "IPv6" : "IPv4"));

}
///close all
void MainWindow::onCloseAllHub_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	while(!mw->Hubs.empty())
	{
		Hub *hub = mw->Hubs.back();
		typedef Func1<MainWindow,BookEntry*> F1;
		F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,hub);
		WulforManager::get()->dispatchGuiFunc(func);

		mw->Hubs.pop_back();	
	}
	mw->Hubs.clear();
}
///PM
void MainWindow::onCloseAllPM_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	
	for(vector<PrivateMessage*>::const_iterator i= mw->privateMessage.begin(); i != mw->privateMessage.end();++i)
	{
		PrivateMessage *pm = *i;
		typedef Func1<MainWindow,BookEntry*> F1;
		F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,pm);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	
	mw->privateMessage.clear();
}
///Search
void MainWindow::onCloseAllSearch_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(vector<Search*>::const_iterator i= mw->search.begin(); i != mw->search.end();++i)
	{
		Search *s = *i;
		typedef Func1<MainWindow,BookEntry*> F1;
		F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,s);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	mw->search.clear();
}
///ofline
void MainWindow::onReconectAllHub_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(vector<Hub*>::const_iterator i= mw->Hubs.begin(); i != mw->Hubs.end();++i)
	{
		Hub *hub = *i;
		hub->reconnect_p();
	}
}
///PM
void MainWindow::onCloseAlloffPM_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	vector<PrivateMessage*> noff;

	for(vector<PrivateMessage*>::const_iterator i= mw->privateMessage.begin(); i != mw->privateMessage.end();++i)
	{
		PrivateMessage *pm = dynamic_cast<PrivateMessage*>(*i);

		if(pm->getIsOffline())
		{
			typedef Func1<MainWindow,BookEntry*> F1;
			F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,pm);
			WulforManager::get()->dispatchGuiFunc(func);

		}else noff.push_back(*i);
	}
	mw->privateMessage.clear();
	mw->privateMessage = noff;
}
/* partial */
void MainWindow::parsePartial(HintedUser aUser, string txt)
{
	const string cid = aUser.user->getCID().toBase32();
	bool raise = !SETTING(POPUNDER_FILELIST);
	BookEntry *entry = findBookEntry(Entry::SHARE_BROWSER, cid);
	string path = QueueManager::getInstance()->getListPath(aUser) + ".xml.bz2";
	
	if(entry != NULL)
	{
	  dynamic_cast<ShareBrowser*>(entry)->loadXML(txt);
	}
	else
	{
		if ( (entry == NULL) && !path.empty())
		{
			entry = new ShareBrowser(aUser.user, path, "/",0,false);
			addBookEntry_gui(entry);
			dynamic_cast<ShareBrowser*>(entry)->loadXML(txt);		
		}
	}
	if (raise)
		raisePage_gui(entry->getContainer());
}

void MainWindow::on(QueueManagerListener::PartialList, const HintedUser& aUser, const string& text) noexcept {
	typedef Func2<MainWindow, HintedUser, string> F2;
	F2 *func = new F2(this,&MainWindow::parsePartial,aUser,text);
	WulforManager::get()->dispatchGuiFunc(func);
}
/**/
