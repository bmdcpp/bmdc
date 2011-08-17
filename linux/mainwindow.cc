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

#include "mainwindow.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <dcpp/FavoriteManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/Text.h>
#include <dcpp/Upload.h>
#include <dcpp/Download.h>
#include <dcpp/ClientManager.h>
#include <dcpp/version.h>
#include "downloadqueue.hh"
#include "favoritehubs.hh"
#include "favoriteusers.hh"
#include "finishedtransfers.hh"
#include "func.hh"
#include "hub.hh"
#include "privatemessage.hh"
#include "publichubs.hh"
#include "search.hh"
#include "searchspy.hh"
#include "settingsmanager.hh"
#include "sharebrowser.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "version.hh"
//Mank
#include "System.hh"
#include "notepad.hh"
#include "ADLSearchGUI.hh"
#include "ignoreusers.hh"

#include "recenthub.hh"
#include <dcpp/HashManager.h>
#include <dcpp/ThrottleManager.h>

#include <dcpp/Util.h>
#ifdef _USELUA
	#include <dcpp/ScriptManager.h>
#endif
#include "highliting.hh"
#include "detectiontab.hh"
#include "cmddebug.hh"
//END

using namespace std;
using namespace dcpp;

MainWindow::MainWindow():
	Entry(Entry::MAIN_WINDOW, "mainwindow.glade"),
	transfers(NULL),
	lastUpdate(0),
	lastUp(0),
	lastDown(0),
	minimized(FALSE),
	timer(0),
	statusFrame(1),
	isLimiting(FALSE)
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

	// magnet dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("MagnetDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("MagnetDialog")), window);
	setChooseMagnetDialog_gui();
	g_signal_connect(getWidget("MagnetDialog"), "response", G_CALLBACK(onResponseMagnetDialog_gui), (gpointer) this);
	g_signal_connect(getWidget("MagnetDialog"), "delete-event", G_CALLBACK(onDeleteEventMagnetDialog_gui), (gpointer) this);
	// TTH file get dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("TTHFileDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL,-1);
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("TTHFileDialog")),window);
	///todo response
	g_signal_connect(getWidget("TTHFileDialog"), "delete-event", G_CALLBACK(onDeleteEventMagnetDialog_gui), (gpointer)this);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("transferCheckButton")), TRUE);

	// About dialog
	gchar *comments = g_strdup_printf(_("DC++ Client based on the source code FreeDC++\nBMDC++ version: %s.%s\nCore version: %s"),
		GUI_VERSION_STRING, GUI_VERSION_BUILD_STRING, VERSIONSTRING);
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
	///Limits menu
	GtkWidget *menu = gtk_menu_new();
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(getWidget("EnableLimit")), menu);
	gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback)gtk_widget_destroy, NULL);
	GtkWidget *upitem = gtk_menu_item_new_with_label(_("Upload Limit (disable)"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),upitem);
	g_object_set_data_full(G_OBJECT(upitem), "type", g_strdup("up"), g_free);
	g_signal_connect(G_OBJECT(upitem), "activate", G_CALLBACK(onLimitingDisable), (gpointer)this);
	GtkWidget *sep =  gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),sep);
	///@change to dynamic
	for(int i = 10240; i<2097152; i = i*2+40960/2) {
		string tmenu = Text::toT(Util::formatBytes(i)) + (_("/s"));
		string tspeed = Util::toString(i);
		GtkWidget *item = gtk_menu_item_new_with_label(tmenu.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		g_object_set_data_full(G_OBJECT(item), "speed", g_strdup(tspeed.c_str()), g_free);
		g_object_set_data_full(G_OBJECT(item), "type", g_strdup("up"), g_free);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onLimitingMenuItem_gui), (gpointer)this);
	}
	GtkWidget *sep3 = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),sep3);
	GtkWidget *dwitem = gtk_menu_item_new_with_label(_("Download Limit (disable)"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),dwitem);
	g_object_set_data_full(G_OBJECT(dwitem), "type", g_strdup("dw"), g_free);
	g_signal_connect(G_OBJECT(dwitem), "activate", G_CALLBACK(onLimitingDisable), (gpointer)this);
	GtkWidget *sep2 = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),sep2);

	for(int j = 10240; j<2097152; j = j*2+40960/2) {
		string tmenu = Text::toT(Util::formatBytes(j)) + (_("/s"));
		string tspeed = Util::toString(j);
		GtkWidget *item = gtk_menu_item_new_with_label(tmenu.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		g_object_set_data_full(G_OBJECT(item), "speed", g_strdup(tspeed.c_str()), g_free);
		g_object_set_data_full(G_OBJECT(item), "type", g_strdup("dw"), g_free);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onLimitingMenuItem_gui), (gpointer)this);
	}
	gtk_widget_show_all(menu);

	// colourstuff added by curse //add to bmdc by Mank
	string res = WulforManager::get()->getPath() + "/glade/resources.rc";
	gtk_rc_parse(res.c_str());
	// colourstuff end

	gtk_about_dialog_set_email_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
	gtk_about_dialog_set_url_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
	// This has to be set in code in order to activate the link
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), "http://bmdc.no-ip.sk");
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
	g_signal_connect(getWidget("publicHubs"), "clicked", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settings"), "clicked", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("hash"), "clicked", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("search"), "clicked", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchSpy"), "clicked", G_CALLBACK(onSearchSpyClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("queue"), "clicked", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
	//Me Toolbars
	g_signal_connect(getWidget("ADLSearch"), "clicked", G_CALLBACK(onADLSearch_gui), (gpointer)this);
	g_signal_connect(getWidget("System"), "clicked", G_CALLBACK(onSystem_gui), (gpointer)this);
	g_signal_connect(getWidget("Notepad"), "clicked", G_CALLBACK(onNotepad_gui), (gpointer)this);
	g_signal_connect(getWidget("IgnoreUsers"), "clicked", G_CALLBACK(onIgnore_gui), (gpointer)this);
	g_signal_connect(getWidget("AwayIcon"), "clicked", G_CALLBACK(onAway_gui), (gpointer)this);
	g_signal_connect(getWidget("EnableLimit"), "clicked", G_CALLBACK(onLimiting), (gpointer)this);
	//end
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
	///RecentHubs
	g_signal_connect(getWidget("recentHubsMenuItem"), "activate", G_CALLBACK(onRecent_gui), (gpointer)this);
	///TTHFileDialog
	g_signal_connect(getWidget("TTHFileMenu"), "activate", G_CALLBACK(onTTHFileDialog_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonfile"), "clicked", G_CALLBACK(onTTHFileButton_gui), (gpointer)this);
	///Higliting Tab
	g_signal_connect(getWidget("highlitingMenuItem"), "activate", G_CALLBACK(onHighliting), (gpointer)this);
	///Detection
	g_signal_connect(getWidget("detectionMenuItem"), "activate", G_CALLBACK(onDetection), (gpointer)this);
	///CMD TAB
	g_signal_connect(getWidget("cmdMenuItem1"), "activate", G_CALLBACK(onDebugCMD), (gpointer)this);

	g_signal_connect(getWidget("indexingProgressMenuItem"), "activate", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMenuItem"), "activate", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
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

	g_signal_connect(getWidget("CloseTabHubAllMenuItem"), "activate", G_CALLBACK(onCloseAllHub_gui), (gpointer)this);
	g_signal_connect(getWidget("CloseTabPMAllMenuItem"), "activate", G_CALLBACK(onCloseAllPM_gui), (gpointer)this);
	g_signal_connect(getWidget("closeTabSearchAllMenuItem"), "activate", G_CALLBACK(onCloseAllSearch_gui), (gpointer)this);
	g_signal_connect(getWidget("CloseTabPMOfflineItem"), "activate", G_CALLBACK(onCloseAlloffPM_gui), (gpointer)this);

	// Help menu
	g_object_set_data_full(G_OBJECT(getWidget("homeMenuItem")), "link",
		g_strdup("http://bmdc.no-ip.sk"), g_free);
	g_signal_connect(getWidget("homeMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

	onQuit = FALSE;

	// Load window state and position from settings manager
	gint posX = WGETI("main-window-pos-x");
	gint posY = WGETI("main-window-pos-y");
	gint sizeX = WGETI("main-window-size-x");
	gint sizeY = WGETI("main-window-size-y");

	gtk_window_move(window, posX, posY);
	gtk_window_resize(window, sizeX, sizeY);

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

	Sound::start();
	Emoticons::start();
	Notify::start();
	//Fix
	if (WGETI("main-window-maximized"))
		gtk_window_maximize(window);

	#ifdef _USELUA
	 ScriptManager::getInstance()->load();
	 // Start as late as possible, as we might (formatting.lua) need to examine settings
	 string defaultluascript = "startup.lua";
	 ScriptManager::getInstance()->EvaluateFile(defaultluascript);
	#endif

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


	WSET("main-window-maximized", maximized);
	if (transferPanePosition > 10)
		WSET("transfer-pane-position", transferPanePosition);

	if (timer > 0)
		g_source_remove(timer);

	WSET("status-icon-blink-use", useStatusIconBlink);

	gtk_widget_destroy(GTK_WIDGET(window));
	g_object_unref(statusIcon);

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

	if (WGETI("show-preferences-on-startup"))
    {
       onPreferencesClicked_gui(NULL, (gpointer)this);
        WSET("show-preferences-on-startup", 0);
    }
    //think
    isLimiting = BOOLSETTING(THROTTLE_ENABLE);
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
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("searchSpy")), "bmdc-search-spy");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("queue")), "bmdc-queue");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("finishedDownloads")), "bmdc-finished-downloads");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("finishedUploads")), "bmdc-finished-uploads");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("quit")), "bmdc-quit");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("connect")), "bmdc-connect");

	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("Notepad")), "bmdc-notepad");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("IgnoreUsers")), "bmdc-ignore-users");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("System")), "bmdc-system");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("ADLSearch")), "bmdc-adlsearch");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("AwayIcon")), "bmdc-away");
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("EnableLimit")), "bmdc-limiting");

	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageHubs")), "bmdc-public-hubs", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageDownloadSpeed")), "bmdc-download", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageUploadSpeed")), "bmdc-upload", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void MainWindow::autoOpen_gui()
{
//NOTE: core 0.762
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
	/**/
	if (WGETB("open-notepad"))
		showNotepad_gui();
	if (WGETB("open-ignore"))
		showIgnore_gui();
	if (WGETB("open-system"))
		showSystem_gui();
}

void MainWindow::getAway()
{
	if (Util::getAway())
	{
			Util::setAway(FALSE);
			Util::setManualAway(FALSE);
			MainWindow::setMainStatus_gui(_("Away mode off"),time(NULL));
			gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("AwayIcon")), "bmdc-away");
	}
	else
	{
			Util::setAway(TRUE);
			Util::setManualAway(TRUE);
			MainWindow::setMainStatus_gui(_("Away mode on"),time(NULL));
			gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("AwayIcon")), "bmdc-away-on");
	}
}

void MainWindow::onLimitingMenuItem_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string speed = (gchar*)g_object_get_data(G_OBJECT(widget), "speed");
	string type = (gchar*)g_object_get_data(G_OBJECT(widget), "type");
	if(speed.empty())
		return;

	bool isEnb = BOOLSETTING(THROTTLE_ENABLE);
    SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE,!isEnb);

	if(type == "up")
	{
		SettingsManager::getInstance()->set(ThrottleManager::getInstance()->getCurSetting(SettingsManager::MAX_UPLOAD_SPEED_MAIN), (Util::toInt(speed))/1024);
	}
	else if(type == "dw")
	{
		SettingsManager::getInstance()->set(ThrottleManager::getInstance()->getCurSetting(SettingsManager::MAX_DOWNLOAD_SPEED_MAIN), (Util::toInt(speed))/1024);
	}
}

void MainWindow::onLimitingDisable(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string type = (gchar*)g_object_get_data(G_OBJECT(widget), "type");
	if(type == "dw")
	{
		SettingsManager::getInstance()->set(ThrottleManager::getInstance()->getCurSetting(SettingsManager::MAX_DOWNLOAD_SPEED_MAIN), 0);

	}
	else if(type == "up")
	{
		SettingsManager::getInstance()->set(ThrottleManager::getInstance()->getCurSetting(SettingsManager::MAX_UPLOAD_SPEED_MAIN), 0);
	}
}

void MainWindow::EnbDsbLimit()
{
    bool isEnb = BOOLSETTING(THROTTLE_ENABLE);
    SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE,!isEnb);
    GtkWidget *widget = getWidget("EnableLimit");
    if(!isEnb)
    {
        MainWindow::setMainStatus_gui(_("Throtle off"), time(NULL));
         gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(widget),"bmdc-limiting");

     }
     else
     {
		MainWindow::setMainStatus_gui(_("Throtle on"), time(NULL));
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(widget),"bmdc-limiting-on");

	}
}

///close all
///Hubs
void MainWindow::onCloseAllHub_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(vector<Hub*>::const_iterator i= mw->allhub.begin(); i != mw->allhub.end();++i)
	{
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(*i);
	}
	mw->allhub.clear();
}
///PM
void MainWindow::onCloseAllPM_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(vector<PrivateMessage*>::const_iterator i= mw->allprivatemess.begin(); i != mw->allprivatemess.end();++i)
	{
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(*i);
	}
	mw->allprivatemess.clear();
}
///Search
void MainWindow::onCloseAllSearch_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(vector<Search*>::const_iterator i= mw->allsearch.begin(); i != mw->allsearch.end();++i)
	{
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(*i);//item
	}
	mw->allsearch.clear();
}
///ofline
/*void MainWindow::onCloseAlloffHub_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(vector<Hub*>::const_iterator i= mw->allhub.begin(); i != mw->allhub.end();++i)
	{
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(*i);
	}
	mw->allhub.clear();
}*/
///PM
void MainWindow::onCloseAlloffPM_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	vector<PrivateMessage*> noff;

	for(vector<PrivateMessage*>::const_iterator i= mw->allprivatemess.begin(); i != mw->allprivatemess.end();++i)
	{
		PrivateMessage *pm = dynamic_cast<PrivateMessage*>(*i);

		if(pm->getOffline())
		{
			WulforManager::get()->getMainWindow()->removeBookEntry_gui(*i);

		}else noff.push_back(*i);
	}
	mw->allprivatemess.clear();
	mw->allprivatemess = noff;
}

///end
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
	if(WGETB("show-close-butt"))
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

	if (BOOLSETTING(ALWAYS_TRAY))
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
	}
}

void MainWindow::showNotification_gui(string head, string body, Notify::TypeNotify notify)
{
	Notify::get()->showNotify(head, body, notify);
}

void MainWindow::setStats_gui(string hubs, string downloadSpeed,
	string downloaded, string uploadSpeed, string uploaded)
{

	int uploadSpeedl = SETTING(MAX_UPLOAD_SPEED_MAIN);
	int downloadSpeedl = SETTING(MAX_DOWNLOAD_SPEED_MAIN);
	string uploadRate,downloadRate;

	if(BOOLSETTING(THROTTLE_ENABLE))
    {
        uploadRate = uploadSpeedl ? Util::formatBytes(uploadSpeedl*1024) + "/" + _("s") : "";
        downloadRate = downloadSpeedl ? Util::formatBytes(downloadSpeedl*1024) + "/" + _("s") : "";
    }
    else
    {
        uploadRate = "";
        downloadRate = "";
    }
	gtk_label_set_text(GTK_LABEL(getWidget("labelHubs")), hubs.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelDownloadSpeed")), (string(" ") + downloadRate + string(" ") + downloadSpeed).c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelDownloaded")), downloaded.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelUploadSpeed")), (string(" ") + uploadRate+string(" ") + uploadSpeed).c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelUploaded")), uploaded.c_str());
	string fslots = Util::toString(UploadManager::getInstance()->getFreeSlots());
	string pslots = _("Slots:") + fslots + "/" + Util::toString(SETTING(SLOTS));
	string text = _("Shared: ") + Util::formatBytes(ShareManager::getInstance()->getShareSize()) + " "+ pslots;
	gtk_label_set_text(GTK_LABEL(getWidget("labelShare")), text.c_str());

}

BookEntry* MainWindow::findBookEntry(const EntryType type, const string &id)
{
	Entry *entry = getChild(type, id);
	return dynamic_cast<BookEntry*>(entry);
}

void MainWindow::showDownloadQueue_gui()
{
	BookEntry *entry = findBookEntry(Entry::DOWNLOAD_QUEUE);

	if (entry == NULL)
	{
		entry = new DownloadQueue();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFavoriteHubs_gui()
{
	BookEntry *entry = findBookEntry(Entry::FAVORITE_HUBS);

	if (entry == NULL)
	{
		entry = new FavoriteHubs();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFavoriteUsers_gui()
{
	BookEntry *entry = findBookEntry(Entry::FAVORITE_USERS);

	if (entry == NULL)
	{
		entry = new FavoriteUsers();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFinishedDownloads_gui()
{
	BookEntry *entry = findBookEntry(Entry::FINISHED_DOWNLOADS);

	if (entry == NULL)
	{
		entry = FinishedTransfers::createFinishedDownloads();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFinishedUploads_gui()
{
	BookEntry *entry = findBookEntry(Entry::FINISHED_UPLOADS);

	if (entry == NULL)
	{
		entry = FinishedTransfers::createFinishedUploads();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showHub_gui(string address, string encoding)
{
	BookEntry *entry = findBookEntry(Entry::HUB, address);

	if (entry == NULL)
	{
		entry = new Hub(address, encoding);
		addBookEntry_gui(entry);

		EntryList.push_back(address);

		allhub.push_back(dynamic_cast<Hub*>(entry));
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showSearchSpy_gui()
{
	BookEntry *entry = findBookEntry(Entry::SEARCH_SPY);

	if (entry == NULL)
	{
		entry = new SearchSpy();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}
//ADLSearch
void MainWindow::showADLSearch_gui()
{
	BookEntry *entry = findBookEntry(Entry::ADL);

	if (entry == NULL)
	{
		entry = new ADLSearchGUI();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}
//END
//Notepad
void MainWindow::showNotepad_gui()
{
	BookEntry *entry = findBookEntry(Entry::NOTEPAD);

	if (entry == NULL)
	{
		entry = new notepad();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}
//END
//Ignore
void MainWindow::showIgnore_gui()
{
	BookEntry *entry = findBookEntry(Entry::IGNORE_USERS);

	if (entry == NULL)
	{
		entry = new ignoreusers();
		addBookEntry_gui(entry);

	}

	raisePage_gui(entry->getContainer());
}
//END
//System
void MainWindow::showSystem_gui()
{
	BookEntry *entry = findBookEntry(Entry::SYSTEML);

	if (entry == NULL)
	{
		entry = new systemlog();
		addBookEntry_gui(entry);

	}

	raisePage_gui(entry->getContainer());
}
//END
//Recent
void MainWindow::showRecentHub_gui()
{
	BookEntry *entry = findBookEntry(Entry::RECENT);

	if (entry == NULL)
	{
		entry = new RecentTab();
		addBookEntry_gui(entry);

	}
	raisePage_gui(entry->getContainer());
}
//END
//Highliting
void MainWindow::showHigliting_gui()
{
	BookEntry *entry = findBookEntry(Entry::HIGHL);

	if(entry == NULL)
	{
		entry = new Highlighting();
		addBookEntry_gui(entry);

	}
	raisePage_gui(entry->getContainer());

}
//end
//Detection
void MainWindow::showDetection_gui()
{
	BookEntry *entry = findBookEntry(Entry::DET);

	if(entry == NULL)
	{
		entry = new DetectionTab();
		addBookEntry_gui(entry);

	}
	raisePage_gui(entry->getContainer());

}
///end
///CMD
void MainWindow::showcmddebug_gui()
{
	BookEntry *entry = findBookEntry(Entry::CMD);

	if(entry == NULL)
	{
		entry = new cmddebug();
		addBookEntry_gui(entry);

	}
	raisePage_gui(entry->getContainer());

}
///CMD

void MainWindow::addPrivateMessage_gui(Msg::TypeMsg typemsg, string cid, string hubUrl, string message, bool useSetting)
{
	BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);
	bool raise = TRUE;

	// If PM is initiated by another user, use setting except if tab is already open.
	if (useSetting)
		raise = (entry == NULL) ? !BOOLSETTING(POPUNDER_PM) : FALSE;

	if (entry == NULL)
	{
		entry = new PrivateMessage(cid, hubUrl);
		addBookEntry_gui(entry);

		EntryList.push_back(cid);

		allprivatemess.push_back(dynamic_cast<PrivateMessage*>(entry));

	}
	//Start Patch

	if(!message.empty() && hubUrl.empty())
	{
		dynamic_cast<PrivateMessage*>(entry)->SendCL(message);
		if(raise)
			raisePage_gui(entry->getContainer());
		return;
	}
	 //END

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
	BookEntry *entry = findBookEntry(Entry::PUBLIC_HUBS);

	if (entry == NULL)
	{
		entry = new PublicHubs();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showShareBrowser_gui(UserPtr user, string filename, string dir, bool useSetting,int64_t speed)
{
	bool raise = useSetting ? !BOOLSETTING(POPUNDER_FILELIST) : TRUE;
	BookEntry *entry = findBookEntry(Entry::SHARE_BROWSER, user->getCID().toBase32());

	if (entry == NULL)
	{
		entry = new ShareBrowser(user, filename, dir,speed);
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
	allsearch.push_back(entry);
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
	if (!WGETB("toolbar-button-queue"))
		gtk_widget_hide(getWidget("queue"));
	if (!WGETB("toolbar-button-quit"))
		gtk_widget_hide(getWidget("quit"));
	if (!WGETB("toolbar-button-finished-downloads"))
		gtk_widget_hide(getWidget("finishedDownloads"));
	if (!WGETB("toolbar-button-finished-uploads"))
		gtk_widget_hide(getWidget("finishedUploads"));
/**/
	if(!WGETB("toolbar-button-notepad"))
		gtk_widget_hide(getWidget("Notepad"));
	if(!WGETB("toolbar-button-adlsearch"))
		gtk_widget_hide(getWidget("ADLSearch"));
	if(!WGETB("toolbar-button-system"))
		gtk_widget_hide(getWidget("System"));
	if(!WGETB("toolbar-button-ignore"))
		gtk_widget_hide(getWidget("IgnoreUsers"));
	if(!WGETB("toolbar-button-away"))
		gtk_widget_hide(getWidget("Awayicon"));
	if(!WGETB("toolbar-limit-bandwith"))
        gtk_widget_hide(getWidget("EnableLimit"));

 /**/
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
		case 0:
			toolbarStyle = GTK_TOOLBAR_ICONS;
			break;
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
			break;
		case 5:
			return;
		default:
			toolbarStyle = GTK_TOOLBAR_BOTH;
	}

	if (style != 4)
	{
		gtk_widget_show(getWidget("toolbar1"));
		gtk_toolbar_set_style(GTK_TOOLBAR(getWidget("toolbar1")), toolbarStyle);
	}
}

bool MainWindow::getUserCommandLines_gui(const string &command, StringMap &ucParams)
{
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

	return true;
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

void MainWindow::onTTHFileDialog_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw =(MainWindow *)data;
	GtkWidget *dialog = mw->getWidget("TTHFileDialog");
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_NONE)
			return;
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
		string TTH;
		char *buf = new char[512*1024];
		try {
			File f(Text::fromT(string(temp)),File::READ, File::OPEN);
			TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
			if(f.getSize() > 0) {
					size_t n = 512*1024;
					while( (n = f.read(&buf[0], n)) > 0) {
						tth.update(&buf[0], n);
						n = 512*1024;
					}
			} else {
				tth.update("", 0);
		}
		tth.finalize();

		strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
		string magnetlink = "magnet:?xt=urn:tree:tiger:"+ TTH +"&xl="+Util::toString(f.getSize())+"&dn="+Util::encodeURI(Text::fromT(Util::getFileName(string(temp))));
		f.close();
		
		gtk_entry_set_text(GTK_ENTRY(mw->getWidget("entrymagnet")), magnetlink.c_str());
		gtk_entry_set_text(GTK_ENTRY(mw->getWidget("entrytthfileresult")), TTH.c_str());
		
		}
		catch(...)
		{ }
	}

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
			QueueManager::getInstance()->add(name, size, TTHValue(tth));

			// automatically search for alternative download locations
			if (BOOLSETTING(AUTO_SEARCH))
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
		if (BOOLSETTING(SettingsManager::AUTO_AWAY) && !Util::getAway())
			Util::setAway(TRUE);
	}
	else if (mw->minimized && (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED ||
		event->new_window_state == 0))
	{
		mw->minimized = FALSE;
		if (BOOLSETTING(SettingsManager::AUTO_AWAY) && !Util::getManualAway())
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
	else if (WGETB("main-window-no-close") && BOOLSETTING(ALWAYS_TRAY))
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconShowInterfaceItem")), FALSE);

		return TRUE;
	}

	if (!BOOLSETTING(CONFIRM_EXIT))
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
		else if (event->keyval == GDK_F4)
		{
			mw->onCloseClicked_gui(widget, data);
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
	if (event->button == 2 && event->x >= 0 && event->y >= 0
		&& event->x < width && event->y < height)
	{
		BookEntry *bEntry = (BookEntry *)data;
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(bEntry);
		return TRUE;
	}
	else if( event->button == 3 && event->x >= 0 && event->y >= 0
			&& event->x < width && event->y < height)
	{

		BookEntry *entry = (BookEntry *)data;
		entry->popmenu();
        GtkWidget *tab = entry->getTabMenu();

		gtk_widget_show_all(tab);

		gtk_menu_popup (GTK_MENU (tab), NULL, NULL, NULL, NULL,
                        event->button, event->time);

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
		mw->showHub_gui(address,"UTF-8");
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

void MainWindow::onPublicHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showPublicHubs_gui();
}

void MainWindow::onPreferencesClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func0<MainWindow> F0;

	unsigned short tcpPort = (unsigned short)SETTING(TCP_PORT);
	unsigned short udpPort = (unsigned short)SETTING(UDP_PORT);
	int lastConn = SETTING(INCOMING_CONNECTIONS);

	if (mw->useStatusIconBlink != WGETB("status-icon-blink-use"))
		WSET("status-icon-blink-use", mw->useStatusIconBlink);
	bool emoticons = WGETB("emoticons-use");

	gint response = WulforManager::get()->openSettingsDialog_gui();

	if (response == GTK_RESPONSE_OK)
	{
		if (SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != tcpPort || SETTING(UDP_PORT) != udpPort)
		{
			F0 *func = new F0(mw, &MainWindow::startSocket_client);
			WulforManager::get()->dispatchClientFunc(func);
		}

		if (BOOLSETTING(ALWAYS_TRAY))
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
		gtk_widget_show_all(mw->getWidget("toolbar1"));
		mw->setToolbarButton_gui();
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

//Notepad
void MainWindow::onNotepad_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showNotepad_gui();
}
//END
//ADL
void MainWindow::onADLSearch_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showADLSearch_gui();
}
//END
//SYSTEMTAB
void MainWindow::onSystem_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showSystem_gui();
}
//END
//Ignore
void MainWindow::onIgnore_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showIgnore_gui();
}
//END
//RecentTab
void MainWindow::onRecent_gui(GtkWidget *widget , gpointer data)
{
		MainWindow *mw = (MainWindow *)data;
		mw->showRecentHub_gui();
}
//END
//Highliting
void MainWindow::onHighliting(GtkWidget *widget, gpointer data)
{
		MainWindow *mw = (MainWindow *)data;
		mw->showHigliting_gui();
}
//end
//Detection
void MainWindow::onDetection(GtkWidget *widget, gpointer data)
{
		MainWindow *mw = (MainWindow *)data;
		mw->showDetection_gui();
}
///end
///CMD
void MainWindow::onDebugCMD(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showcmddebug_gui();
}
//Away Ico
void MainWindow::onAway_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->getAway();
}
//END
//Limiting Icon
void MainWindow::onLimiting(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->EnbDsbLimit();
}
//end

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
				mw->showShareBrowser_gui(user, path, "", FALSE,(int64_t)0);
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

	mw->setMainStatus_gui(_("Loading file list"));
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
	WulforUtil::openURI(link);
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
	string link = (gchar*)g_object_get_data(G_OBJECT(widget), "link");
	WulforUtil::openURI(link);
}

void MainWindow::autoConnect_client()
{
 	typedef Func2<MainWindow, string, string> F2;
 	F2 *func;
	const FavHubGroups& FavGroups = FavoriteManager::getInstance()->getFavHubGroups();

	for (dcpp::FavHubGroups::const_iterator i=FavGroups.begin();i!=FavGroups.end();++i)
	{
		if(i->second.connect)
		{
			FavoriteHubEntryList list= FavoriteManager::getInstance()->getFavoriteHubs(i->first);
			for ( FavoriteHubEntryList::const_iterator p = list.begin(); p!=list.end(); ++p)
			{
                    func = new F2(this, &MainWindow::showHub_gui, (*p)->getServer(), (*p)->getEncoding());
                    WulforManager::get()->dispatchGuiFunc(func);
			}
		}
	}

	string link = WulforManager::get()->getURL();

	if (link.empty()) return;

	typedef Func1<MainWindow, string> F1;
	F1 *func1;

	if (WulforUtil::isHubURL(link) && BOOLSETTING(URL_HANDLER))
	{
		typedef Func2<MainWindow, string, string> F2;//TODO: core 0.762
		F2 *func = new F2(this, &MainWindow::showHub_gui, link, "");//TODO: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if (WulforUtil::isMagnet(link) && BOOLSETTING(MAGNET_REGISTER))
	{
		func1 = new F1(this, &MainWindow::actionMagnet_gui, link);
		WulforManager::get()->dispatchGuiFunc(func1);
	}
}

void MainWindow::startSocket_client()
{
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if (ClientManager::getInstance()->isActive())
	{
		try
		{
			ConnectionManager::getInstance()->listen();
		}
		catch (const Exception &e)
		{
			string primaryText = _("Unable to open TCP/TLS port");
			string secondaryText = _("File transfers will not work correctly until you change settings or turn off any application that might be using the TCP/TLS port.");
			typedef Func2<MainWindow, string, string> F2;
			F2* func = new F2(this, &MainWindow::showMessageDialog_gui, primaryText, secondaryText);
			WulforManager::get()->dispatchGuiFunc(func);

		}

		try
		{
			SearchManager::getInstance()->listen();
		}
		catch (const Exception &e)
		{
			string primaryText = _("Unable to open UDP port");
			string secondaryText = _("Searching will not work correctly until you change settings or turn off any application that might be using the UDP port.");
			typedef Func2<MainWindow, string, string> F2;
			F2* func = new F2(this, &MainWindow::showMessageDialog_gui, primaryText, secondaryText);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}

	ClientManager::getInstance()->infoUpdated();
}

void MainWindow::refreshFileList_client()
{
	try
	{
		ShareManager::getInstance()->setDirty();
		ShareManager::getInstance()->refresh(TRUE, TRUE, FALSE);
		string text = _("Shared: ") + Util::formatBytes(ShareManager::getInstance()->getShareSize());
		gtk_label_set_text(GTK_LABEL(getWidget("labelShare")), text.c_str());
	}
	catch (const ShareException&)
	{	}
}

void MainWindow::openOwnList_client(bool useSetting)
{
	UserPtr user = ClientManager::getInstance()->getMe();
	string path = ShareManager::getInstance()->getOwnListFile();

	typedef Func5<MainWindow, UserPtr, string, string, bool,int64_t> F5;
	F5 *func = new F5(this, &MainWindow::showShareBrowser_gui, user, path, "", useSetting,(int64_t)0);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(LogManagerListener::Message, time_t t, const string &message) throw()
{
	typedef Func2<MainWindow, string, time_t> F2;
	F2 *func = new F2(this, &MainWindow::setMainStatus_gui, message, t);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw()
{
	typedef Func3<MainWindow, string, string, Notify::TypeNotify> F3;

	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
	{
		const HintedUser user = item->getDownloads()[0]->getHintedUser();//NOTE: core 0.762
		string listName = item->getListName();

		F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("file list from "), WulforUtil::getNicks(user),
			Notify::DOWNLOAD_FINISHED_USER_LIST);

		WulforManager::get()->dispatchGuiFunc(f3);

		typedef Func5<MainWindow, UserPtr, string, string, bool,int64_t> F5;
		F5 *func = new F5(this, &MainWindow::showShareBrowser_gui, user.user, listName, dir, TRUE, avSpeed);//NOTE: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if(item->isSet(QueueItem::FLAG_USER_LIST) && item->isSet(QueueItem::FLAG_CHECK_FILE_LIST))
	{
        DirectoryListInfo* i = new DirectoryListInfo(item->getDownloads()[0]->getUser(), item->getListName(), dir, avSpeed);
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
			} catch(...) {
			}
			delete dl;
			//RSX++
			/*if(RSXPP_BOOLSETTING(DELETE_CHECKED_FILELISTS)) {
				File::deleteFile(Text::fromT(i->file));
			}*/
			//END
		}
		delete i;
	}
	stop = true;
	return 0;
}

void MainWindow::on(TimerManagerListener::Second, uint64_t ticks) throw()
{
	Util::setUptime();//uptime
	// Avoid calculating status update if it's not needed
	if (!BOOLSETTING(ALWAYS_TRAY) && minimized)
		return;

	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : ticks - lastUpdate);
	int64_t downBytes = 0;
	int64_t upBytes = 0;

	if (diff > 0)
	{
		int64_t downDiff = Socket::getTotalDown() - lastDown;
		int64_t upDiff = Socket::getTotalUp() - lastUp;
		downBytes = (downDiff * 1000) / diff;
		upBytes = (upDiff * 1000) / diff;
	}

	string hubs = Client::getCounts();
	string downloadSpeed = Util::formatBytes(downBytes) + "/" + _("s");
	string downloaded = Util::formatBytes(Socket::getTotalDown());
	string uploadSpeed = Util::formatBytes(upBytes) + "/" + _("s");
	string uploaded = Util::formatBytes(Socket::getTotalUp());

	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	WSET("up-st", Util::toString(Util::toInt64(WGETS("up-st"))+upBytes));
	WSET("dw-st", Util::toString(Util::toInt64(WGETS("dw-st"))+downBytes));

	typedef Func5<MainWindow, string, string, string, string, string> F5;
	F5 *func = new F5(this, &MainWindow::setStats_gui, hubs, downloadSpeed, downloaded, uploadSpeed, uploaded);
	WulforManager::get()->dispatchGuiFunc(func);

	if (BOOLSETTING(ALWAYS_TRAY) && !downloadSpeed.empty() && !uploadSpeed.empty())
	{
		typedef Func2<MainWindow, string, string> F2;
		F2 *f2 = new F2(this, &MainWindow::updateStatusIconTooltip_gui, downloadSpeed, uploadSpeed);
		WulforManager::get()->dispatchGuiFunc(f2);
	}
}
