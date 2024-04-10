/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © BMDC 2022 - 2025
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
#include "../dcpp/ShareManager.h"
#include "../dcpp/Text.h"
#include "../dcpp/Upload.h"
#include "../dcpp/Download.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/MappingManager.h"
#include "../dcpp/GeoManager.h"
#include "../dcpp/HttpDownload.h"
#include "../dcpp/version.h"
#include "../dcpp/ThrottleManager.h"
#include "../dcpp/ConnectivityManager.h"
#include "../dcpp/HashManager.h"

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
#include "GuiUtil.hh"
#include "version.hh"
#include "System.hh"
#include "cmddebug.hh"
#include "notepad.hh"
#include "uploadqueue.hh"
#include "recenthub.hh"
#include "detectiontab.hh"
#include "AboutConfig.hh"
#ifdef HAVE_LIBTAR
	#include "exportimport.hh"
#endif
#include "SearchEntry.hh"
#include "settingsdialog.hh"
#include "hashdialog.hh"

#include "Shortcuts.hh"

using namespace std;
using namespace dcpp;

const GActionEntry MainWindow::win_entries[] = {
		{ "close-tab", onCloseClicked_gui, NULL, NULL, NULL }
};

static GtkWidget* createButtonToolbarWidget(string name, string label, bool bText )
{
	GtkWidget* widget;
	if(bText)
	 	widget = gtk_button_new_with_label(label.c_str());
	else		
		widget = gtk_button_new_from_icon_name(name.c_str());
	
	return widget;	
}

MainWindow::MainWindow(GtkWidget* window /* = NULL*/):
	Entry(Entry::MAIN_WINDOW, "mainwindow"),
	transfers(NULL), minimized(false),
	lastUpdate(0), lastUp(0), lastDown(0),
	statusFrame(1),	current_width(-1),
	current_height(-1),	is_maximized(FALSE),
	window(window), bText(false)
{
	if(bText) //set
		bText = true;
//	string stmp;
//	startTime = GET_TICK();
//	HashManager::getInstance()->getStats(stmp, startBytes, startFiles);
//	updateStats_gui("", 0, 0, 0);
//	setStatRate_gui();

	GtkWidget* mWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
	gtk_window_set_child(GTK_WINDOW(window),mWidget);

	GtkWidget* wTool = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_box_append(GTK_BOX(mWidget),wTool);

	GtkWidget* wConnect = createButtonToolbarWidget("bmdc-connect", "Connect", bText);
	gtk_box_append(GTK_BOX(wTool), wConnect);
	g_signal_connect(wConnect, "clicked", G_CALLBACK(onConnectClicked_gui), (gpointer)this);

  	GtkWidget* favHub = createButtonToolbarWidget("bmdc-favorite-hubs", "Favorite Hubs", bText);
  	gtk_box_append(GTK_BOX(wTool), favHub);
  
  	GtkWidget* recHub = createButtonToolbarWidget("bmdc-recent-hubs", "Recent Hubs", bText); 
  	gtk_box_append(GTK_BOX(wTool), recHub);

	GtkWidget* favuser = createButtonToolbarWidget("bmdc-favorite-users", "Favorite Users", bText);
	gtk_box_append(GTK_BOX(wTool), favuser);
  
	GtkWidget* dq = createButtonToolbarWidget("bmdc-queue", "Download Queue", bText);
	gtk_box_append(GTK_BOX(wTool), dq);

	GtkWidget* wHash = createButtonToolbarWidget("bmdc-hash", "Hashing", bText);
	gtk_box_append(GTK_BOX(wTool), wHash);

	GtkWidget* search = createButtonToolbarWidget("bmdc-search", "Search", bText);
	gtk_box_append(GTK_BOX(wTool), search);

	GtkWidget* publicHub = createButtonToolbarWidget("bmdc-public-hubs", "Public Hubs", bText);
	gtk_box_append(GTK_BOX(wTool), publicHub);
  
	GtkWidget* df = createButtonToolbarWidget("bmdc-finished-downloads", "Finished Downloads", bText);
	gtk_box_append(GTK_BOX(wTool), df);

	GtkWidget* du = createButtonToolbarWidget("bmdc-finished-uploads", "Finished Uploads", bText);
	gtk_box_append(GTK_BOX(wTool), du);
  
	GtkWidget* uq = createButtonToolbarWidget("bmdc-upload-quene", "Upload Queue", bText);
	gtk_box_append(GTK_BOX(wTool), uq);

	GtkWidget* sp =createButtonToolbarWidget("bmdc-preferences","Settings", bText);
	gtk_box_append(GTK_BOX(wTool), sp);

	GtkWidget* ac = gtk_button_new_with_label("About Config");
	gtk_box_append(GTK_BOX(wTool), ac);

	GtkWidget* nt = createButtonToolbarWidget("bmdc-notepad", "Notepad", bText);
 	gtk_box_append(GTK_BOX(wTool), nt);

	GtkWidget* sl = gtk_button_new_with_label("System Log");
	gtk_box_append(GTK_BOX(wTool), sl);
  
	GtkWidget* cmd = gtk_button_new_with_label("Cmd Log");
	gtk_box_append(GTK_BOX(wTool), cmd);

	GtkWidget* adl = createButtonToolbarWidget("bmdc-search-adl", "ADL Search", bText);
	gtk_box_append(GTK_BOX(wTool), adl);

	GtkWidget* sspy = createButtonToolbarWidget("bmdc-search-spy", "Search Spy", bText);
  	gtk_box_append(GTK_BOX(wTool), sspy);
  
	GtkWidget* hpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

	note = gtk_notebook_new();

	GSimpleActionGroup *group;
	group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (group), win_entries, G_N_ELEMENTS (win_entries), (gpointer)this);
	gtk_widget_insert_action_group(note, "app" ,G_ACTION_GROUP(group));
	
	gtk_paned_set_start_child (GTK_PANED (hpaned), note);

    transfers = new Transfers();
    gtk_paned_set_end_child(GTK_PANED(hpaned) , transfers->getContainer());
    transfers->show();

	int  pos = WGETI("transfer-pane-position");
	gtk_paned_set_position(GTK_PANED(hpaned), pos);

    gtk_box_append(GTK_BOX(mWidget) , GTK_WIDGET(hpaned));

	GtkWidget* bBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 300);

    gtk_box_append(GTK_BOX(mWidget) , bBox) ;

    statusBar = gtk_statusbar_new();
    gtk_box_append(GTK_BOX(bBox) , statusBar);
	// Set all windows to the default icon
	gtk_window_set_default_icon_name(g_get_prgname());
	//  Connect the signals to their callback functions.
	g_signal_connect(note, "switch-page", G_CALLBACK(onPageSwitched_gui), (gpointer)this);
	g_signal_connect(favHub, "clicked", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
	g_signal_connect(favuser, "clicked", G_CALLBACK(onFavoriteUsersClicked_gui), (gpointer)this);
	g_signal_connect(publicHub, "clicked", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
	g_signal_connect(wHash, "clicked", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	g_signal_connect(search, "clicked", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
	g_signal_connect(adl, "clicked", G_CALLBACK(onSearchADLClicked_gui), (gpointer)this);
	g_signal_connect(sspy, "clicked", G_CALLBACK(onSearchSpyClicked_gui), (gpointer)this);
	g_signal_connect(dq, "clicked", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
	g_signal_connect(nt, "clicked", G_CALLBACK(onNotepadClicked_gui), (gpointer)this);
	g_signal_connect(sl, "clicked", G_CALLBACK(onSystemLogClicked_gui), (gpointer)this);
	g_signal_connect(df, "clicked", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
	g_signal_connect(du, "clicked", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("openFileListMenuItem"), "activate", G_CALLBACK(onOpenFileListClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("openOwnListMenuItem"), "activate", G_CALLBACK(onOpenOwnListClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("refreshFileListMenuItem"), "activate", G_CALLBACK(onRefreshFileListClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("reconnectMenuItem"), "activate", G_CALLBACK(onReconnectClicked_gui), (gpointer)this);
	g_signal_connect(sp, "clicked", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
	g_signal_connect(cmd, "clicked", G_CALLBACK(onCmdDebugClicked_gui), (gpointer)this);
	g_signal_connect(uq, "clicked", G_CALLBACK(onUploadQueueClicked_gui), (gpointer)this);
	g_signal_connect(recHub, "clicked", G_CALLBACK(onRecentHubClicked_gui), (gpointer)this);
	g_signal_connect(ac, "clicked", G_CALLBACK(onAboutConfigClicked_gui), (gpointer)this);
	onQuit = false;
	// colourstuff
/*	string s_css = WulforManager::get()->getPath() + "/ui/resources.css";
	if(Util::fileExists(s_css) == true) {
		GtkCssProvider *provider =  gtk_css_provider_get_default ();
		GError *error = NULL;
		gtk_css_provider_load_from_path (provider,s_css.c_str(),&error);
		if(error != NULL) {
			g_print("Error while loading custom CSS for BMDC %s",error->message);
			g_error_free(error);
		}
	}*/
	// colourstuff end
	// Load window state and position from settings manager
//	gint iposX = WGETI("main-window-pos-x");
//	gint iposY = WGETI("main-window-pos-y");
//	gint isizeX = WGETI("main-window-size-x");
//	gint isizeY = WGETI("main-window-size-y");
	setMainStatus_gui(_("Welcome to ") + string(g_get_application_name()));

	gtk_widget_show(GTK_WIDGET(window));

	Sound::start();
	Notify::start();
}

void MainWindow::onButtonPressed_gui(GtkGestureClick* self, gint n_press, gdouble x, gdouble y, gpointer data)
{

}

MainWindow::~MainWindow()
{
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	LogManager::getInstance()->removeListener(this);

	listQueue.shutdown();

	// Save window state and position
	gint posX = 0 , posY = 0, transferPanePosition = 0;

//	gtk_window_get_position(window, &posX, &posY);
	transferPanePosition =  gtk_paned_get_position(GTK_PANED(getWidget("pane")));

	if(!is_maximized || (minimized == false)) {
//		WSET("main-window-pos-x", posX);
//		WSET("main-window-pos-y", posY);
//		WSET("main-window-size-x", current_width);
//		WSET("main-window-size-y", current_height);
	}

	if (transferPanePosition)
		WSET("transfer-pane-position", transferPanePosition);

	Sound::stop();
	Notify::stop();
}

GtkWidget *MainWindow::getContainer()
{
	return window;
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
	string sTitle;

	if (!text.empty())
		sTitle = text + " - " + g_get_application_name();
	else
		sTitle = g_get_application_name();

	gtk_window_set_title(GTK_WINDOW(window), sTitle.c_str());
}

bool MainWindow::isActive_gui()
{
	return true;
}

void MainWindow::setUrgent_gui()
{

}
/*
 * Create and show Transfers pane
 */
void MainWindow::showTransfersPane_gui()
{

}

void MainWindow::autoOpen_gui()
{
	if (WGETB("open-public"))
		showPublicHubs_gui();
	if (WGETB("open-queue"))
		showFinishedDownloads_gui();
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
	if (WGETB("open-upload-queue"))
		showUploadQueue_gui();
}

void MainWindow::onPopupPopover(GtkWidget*  , gpointer data)
{
    MainWindow* mw = (MainWindow*)data;
    #ifndef grid_add
    #define grid_add(box,widget,x,y,z,c) gtk_grid_attach(GTK_GRID(box), widget ,x,y,z,c)
    #endif
//    GtkWidget *popover = gtk_popover_new(mw->getWidget("limitingButton"));
    GtkWidget *labelup = gtk_label_new("Upload Limit");
    GtkWidget *labeldown = gtk_label_new("Download Limit");
    GtkWidget* scaleUp = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0,9999999,1);
    GtkWidget* scaleDown = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0,9999999,1);
    GtkWidget* grid = gtk_grid_new();

    grid_add(grid,labeldown,0,0,1,1);
    grid_add(grid,scaleDown,1,0,1,1);
    grid_add(grid,labelup,0,2,1,1);
    grid_add(grid,scaleUp,1,2,1,1);

    int iup = SETTING(MAX_UPLOAD_SPEED_MAIN);
	int idown = SETTING(MAX_DOWNLOAD_SPEED_MAIN);
    gtk_range_set_value (GTK_RANGE(scaleDown),idown);
    gtk_range_set_value (GTK_RANGE(scaleUp),iup);

    g_signal_connect(GTK_RANGE(scaleDown),"value-changed",G_CALLBACK(onLimitingMenuItem_gui),(gpointer)mw);
    g_object_set_data_full(G_OBJECT(scaleDown),"type",g_strdup("dw"),g_free);
    g_signal_connect(GTK_RANGE(scaleUp),"value-changed",G_CALLBACK(onLimitingMenuItem_gui),(gpointer)mw);
    g_object_set_data_full(G_OBJECT(scaleUp),"type",g_strdup("up"),g_free);

}

void MainWindow::onLimitingMenuItem_gui(GtkRange *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string s_type = (gchar *)g_object_get_data(G_OBJECT(widget), "type");

	if(s_type.empty())
			return;

	if(s_type == "up")
	{
		ThrottleManager::setSetting(SettingsManager::MAX_UPLOAD_SPEED_MAIN, gtk_range_get_value(widget)/1024 );
	}
	else if(s_type == "dw")
	{
		ThrottleManager::setSetting(SettingsManager::MAX_DOWNLOAD_SPEED_MAIN, gtk_range_get_value(widget)/1024 );
	}

    if(gtk_range_get_value(widget) == 0)
    {
        SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, false);
    }
    else
    {
        SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, true);
    }

	mw->setStatRate_gui();

}

void MainWindow::setInitThrotles()
{
	int iup = SETTING(MAX_UPLOAD_SPEED_MAIN);
	int idown = SETTING(MAX_DOWNLOAD_SPEED_MAIN);
	bool benabled = SETTING(THROTTLE_ENABLE);
//@:Enabled
	if(benabled && (iup > 0) ) {
		return;
	}
	if(benabled && (idown > 0)) {
		return;
	}
//@:disabled
	if(!benabled && (iup == 0)) {
		return;
	}
	if(!benabled && (idown == 0)) {
		return;
	}
}

void MainWindow::addBookEntry_gui(BookEntry *entry)
{
	addChild(entry);

	GtkWidget *page = entry->getContainer();
	GtkWidget *label = entry->getLabelBox();
	GMenu *tabMenuItem = entry->getTabMenuItem();
	int ipos = entry->getPositionTab();

	addTabMenuItem_gui(tabMenuItem, page);

	gtk_notebook_insert_page(GTK_NOTEBOOK(note), page, label, ipos);

	if(WGETB("use-close-button"))
	{
		GtkWidget *closeButton = entry->getCloseButton();
		g_signal_connect(closeButton, "clicked", G_CALLBACK(onCloseBookEntry_gui), (gpointer)entry);
	}
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(note), page, TRUE);
	entry->show();
}

GtkWidget *MainWindow::currentPage_gui()
{
	int iPageNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(note));

	if (iPageNum == -1)
		return NULL;
	else
		return gtk_notebook_get_nth_page(GTK_NOTEBOOK(note), iPageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page)
{
	int inum = gtk_notebook_page_num(GTK_NOTEBOOK(note), page);
	int icurrentNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(note));

	if (inum != -1 && inum != icurrentNum)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(note), inum);
}

void MainWindow::removeBookEntry_gui(BookEntry *entry)
{
	if(!entry) return;

	string entryID = entry->getID();
	Entry::EntryType type = entry->getType();
	removeItemFromList(type, entryID);

	string::size_type pos = entryID.find(':');
	if (pos != string::npos) entryID.erase(0, pos + 1);

	StringIter it = find(EntryList.begin(), EntryList.end(), entryID);

	if (it != EntryList.end())
		EntryList.erase(it);

	GtkNotebook *book = GTK_NOTEBOOK(note);
	GtkWidget *page = entry->getContainer();
	GMenu* menuItem = entry->getTabMenuItem();
	int inum = gtk_notebook_page_num(book, page);
	removeChild(entry);

	if (inum != -1)
	{
		GList *list = (GList *)g_object_get_data(G_OBJECT(book), "page-rotation-list");
		list = g_list_remove(list, (gpointer)page);
		g_object_set_data(G_OBJECT(book), "page-rotation-list", (gpointer)list);

		// if removing the current page, switch to the previous page in the rotation list
		if (inum == gtk_notebook_get_current_page(book))
		{
			GList *prev = g_list_first(list);
			if (prev != NULL)
			{
				gint childNum = gtk_notebook_page_num(book, GTK_WIDGET(prev->data));
				gtk_notebook_set_current_page(book, childNum);
			}
		}
		gtk_notebook_remove_page(book, inum);

		if (gtk_notebook_get_n_pages(book) == 0)
		{
			setTitle(""); // Reset window title to default
		}
	}
}

void MainWindow::removeItemFromList(Entry::EntryType type, string sid)
{
	vector<Entry*> tmp;
	switch(type)
	{
		/*case Entry::FAVORITE_HUBS:
			setStatusOfIcons(FAVORITE_HUBS,false);
			break;
		case Entry::FAVORITE_USERS:
			setStatusOfIcons(FAVORITE_USERS,false);
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
			break;*/
		case Entry::HUB:
			if(Hubs.empty()) break;
			 for(auto it = Hubs.begin();it != Hubs.end();++it)
			 {
				 string hubId = (*it)->getID();
				 if(hubId == sid)
					 continue;
				tmp.push_back(*it);
			 }
			 Hubs = tmp;
			 break;
		case Entry::PRIVATE_MESSAGE:
			if(privateMessage.empty()) break;
			for(auto it = privateMessage.begin();it != privateMessage.end();++it)
			 {
				 string pId = (*it)->getID();
				 if(pId == sid)
					 continue;
				tmp.push_back(*it);
			 }
			 privateMessage = tmp;
			 break;
		case Entry::SEARCHS:
		default:break;
	}
}

void MainWindow::previousTab_gui()
{
	GtkNotebook *book = GTK_NOTEBOOK(note);

	if (gtk_notebook_get_current_page(book) == 0)
		gtk_notebook_set_current_page(book, -1);
	else
		gtk_notebook_prev_page(book);
}

void MainWindow::nextTab_gui()
{
	GtkNotebook *book = GTK_NOTEBOOK(note);

	if (gtk_notebook_get_n_pages(book) - 1 == gtk_notebook_get_current_page(book))
		gtk_notebook_set_current_page(book, 0);
	else
		gtk_notebook_next_page(book);
}

void MainWindow::addTabMenuItem_gui(GMenu* menu, GtkWidget* page)
{
 	GMenuItem * menuItem = g_menu_item_new("Close Tab","app.tab-close");
 	g_menu_append_item(menu ,menuItem);
}

void MainWindow::removeTabMenuItem_gui(GtkWidget *menuItem)
{
	GtkNotebook *book = GTK_NOTEBOOK(note);

	if (gtk_notebook_get_n_pages(book) == 0)
	{ ;	}
}

void MainWindow::setMainStatus_gui(string text, time_t t)
{
	if (!text.empty())
	{
		text = "[" + Util::getShortTimeString(t) + "] " + text;
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
		statusTextOnToolTip += "\n" + text;
		gtk_statusbar_push(GTK_STATUSBAR(statusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar),"Info") , statusTextOnToolTip.c_str());
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

	string sfreeslots = std::to_string(UploadManager::getInstance()->getFreeSlots());
	string sslots = _("Slots: ") + sfreeslots + "/" + std::to_string(SETTING(SLOTS));
	string sshared = _("Shared: ") + Util::formatBytes(ShareManager::getInstance()->getShareSize());
	gtk_label_set_text(GTK_LABEL(getWidget("labelShare")), sshared.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelSlots")), sslots.c_str());
}

void MainWindow::setStatRate_gui()
{
	int iuploadSpeed = SETTING(MAX_UPLOAD_SPEED_MAIN);
	int idownloadSpeed = SETTING(MAX_DOWNLOAD_SPEED_MAIN);
	string suploadRate = iuploadSpeed ? Util::formatBytes(iuploadSpeed*1024) + "/" + _("s") : "max";
	string sdownloadRate = idownloadSpeed ? Util::formatBytes(idownloadSpeed*1024) + "/" + _("s") : "max";
	gtk_label_set_text(GTK_LABEL(getWidget("labelDownloadRate")), sdownloadRate.c_str());
	gtk_label_set_text(GTK_LABEL(getWidget("labelUploadRate")), suploadRate.c_str());
}

BookEntry* MainWindow::findBookEntry(const EntryType type, const string &id)
{
	Entry *entry = getChild(type, id);
	return dynamic_cast<BookEntry*>(entry);
}

void MainWindow::showBook(const EntryType type, BookEntry* book)
{
	BookEntry *entry = findBookEntry(type);

	if(!entry)
	{
		addBookEntry_gui(book);
		raisePage_gui(book->getContainer());
	}
	else raisePage_gui(entry->getContainer());

}

void MainWindow::onAboutConfigClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showBook(Entry::ABOUT_CONFIG, new AboutConfig());
}

void MainWindow::showFavoriteHubs_gui()
{
	showBook(Entry::FAVORITE_HUBS, new FavoriteHubs());
}

void MainWindow::showFavoriteUsers_gui()
{
	showBook(Entry::FAVORITE_USERS, new FavoriteUsers());
}

void MainWindow::showCmdDebug_gui()
{
	showBook(Entry::CMD, new cmddebug());
}

void MainWindow::showSystemLog_gui()
{
	showBook(Entry::SYSTEML, new SystemLog());
}

void MainWindow::showNotepad_gui()
{
	showBook(Entry::NOTEPAD, new Notepad());
}

void MainWindow::showUploadQueue_gui()
{
	showBook(Entry::UPLOADQUEUE, new UploadQueue());
}

void MainWindow::showRecentHubs_gui()
{
	showBook(Entry::RECENT, new RecentHubs());
}

void MainWindow::showDetection_gui()
{
	showBook(Entry::DETECTION, new DetectionTab());
}

void MainWindow::showFinishedDownloads_gui()
{
	showBook(Entry::FINISHED_DOWNLOADS, FinishedTransfers::createFinishedDownloads());
}

void MainWindow::showFinishedUploads_gui()
{
	showBook(Entry::FINISHED_UPLOADS, FinishedTransfers::createFinishedUploads());
}

void MainWindow::showHub_gui(string saddress, string encoding)
{
	saddress = Util::trimUrl(saddress);

	if(saddress.empty())
	{
	//	showMessageDialog_gui(_("Empty hub address specified"),_("Empty hub address specified"));
		return;
	}

	BookEntry *entry = findBookEntry(Entry::HUB, saddress);

	if (entry == NULL)
	{
		entry = new Hub(saddress, encoding);
		addBookEntry_gui(entry);

		EntryList.push_back(saddress);
		Hubs.push_back(dynamic_cast<Entry*>(entry));
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showSearchSpy_gui()
{
	showBook(Entry::SEARCH_SPY, new SearchSpy());
}

void MainWindow::showSearchADL_gui()
{
	showBook(Entry::SEARCH_ADL, new SearchADL());
}

void MainWindow::addPrivateMessage_gui(Msg::TypeMsg typemsg, string cid, string hubUrl, string message, bool useSetting)
{
	BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);
	bool braise = true;

	// If PM is initiated by another user, use setting except if tab is already open.
	if (useSetting)
		braise = (entry == NULL) ? !SETTING(POPUNDER_PM) : false;

	if (entry == NULL)
	{
		entry = new PrivateMessage(cid, hubUrl);
		addBookEntry_gui(entry);

		EntryList.push_back(cid);
		privateMessage.push_back(dynamic_cast<Entry*>(entry));
	}

	if(!message.empty() && hubUrl.empty())
	{
		dynamic_cast<PrivateMessage*>(entry)->sendMessage_p(message);
		if(braise)
			raisePage_gui(entry->getContainer());
		return;
	}

	if (!message.empty())
	{
		dynamic_cast<PrivateMessage*>(entry)->addMessage_gui(message, typemsg);

		bool show = false;

		if (!isActive_gui())
		{
			show = true;
		}
		else if (currentPage_gui() != entry->getContainer() && !WGETI("notify-only-not-active"))
		{
			show = true;
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

	if (braise)
		raisePage_gui(entry->getContainer());
}

void MainWindow::addPrivateStatusMessage_gui(Msg::TypeMsg typemsg, string cid, string message)
{
	BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);

	if (entry != NULL)
		dynamic_cast<PrivateMessage*>(entry)->addStatusMessage_gui(message, typemsg);
}

void MainWindow::addPublicStatusMessage_gui(std::string hub,std::string message, Msg::TypeMsg typemsg, Sound::TypeSound sound, Notify::TypeNotify notify)
{
	BookEntry *entry = findBookEntry(Entry::HUB, hub);
	if(entry != NULL)
		dynamic_cast<Hub*>(entry)->addStatusMessage_gui(message,typemsg,sound ,notify);
}

void MainWindow::showPublicHubs_gui()
{
	showBook(Entry::PUBLIC_HUBS, new PublicHubs());
}

void MainWindow::showShareBrowser_gui(HintedUser user, string filename, string dir, int64_t speed ,bool useSetting)
{
	bool braise = useSetting ? !SETTING(POPUNDER_FILELIST) : true;
	BookEntry *entry = findBookEntry(Entry::SHARE_BROWSER, user.user->getCID().toBase32());

	if (entry == NULL)
	{
		entry = new ShareBrowser(user, filename, dir, speed);
		addBookEntry_gui(entry);
	}

	if (braise)
		raisePage_gui(entry->getContainer());
}

SearchEntry *MainWindow::addSearch_gui()
{
	SearchEntry *entry =  dynamic_cast<SearchEntry*>(findBookEntry(Entry::SEARCHS));
	if(entry == NULL) {
		entry = new SearchEntry();
		addBookEntry_gui(entry);
	}
	raisePage_gui(entry->getContainer());
	return entry;
}

void MainWindow::addSearch_gui(string magnet)
{
	string sname;
	int64_t i64size;
	string stth;

	if (WulforUtil::splitMagnet(magnet, sname, i64size, stth))
	{
		SearchEntry *s = addSearch_gui();
		s->putValue_gui(stth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	}
}

void MainWindow::fileToDownload_gui(string magnet, string path)
{
	string sname;
	int64_t i64size;
	string stth;

	if (!WulforUtil::splitMagnet(magnet, sname, i64size, stth))
		return;
	sname = path + sname;

	typedef Func3<MainWindow, string, int64_t, string> F3;
	F3 *func = new F3(this, &MainWindow::addFileDownloadQueue_client, sname, i64size, stth);
	WulforManager::get()->dispatchClientFunc(func);
}

GtkWidget* MainWindow::getChooserDialog_gui()
{
	return getWidget("flistDialog");
}

void MainWindow::actionMagnet_gui(string magnet)
{
	if (gtk_widget_get_visible(getWidget("MagnetDialog")))
		return;

	string name, tth;
	int64_t size;
	int action = WGETI("magnet-action");
	bool split = WulforUtil::splitMagnet(magnet, name, size, tth);

	if (action == 0 && split)
	{
		SearchEntry *s = addSearch_gui();
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

void MainWindow::onHubClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string saddress = (gchar *)g_object_get_data(G_OBJECT(widget), "address");
	string sencoding = (gchar *)g_object_get_data(G_OBJECT(widget), "encoding");
	mw->showHub_gui(saddress, sencoding);
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

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(note), tabPosition);
}

bool MainWindow::getUserCommandLines_gui(const string &commands, ParamMap &ucParams)
{
	MainWindow *mw = WulforManager::get()->getMainWindow();
	GtkDialog *dialog =  GTK_DIALOG(gtk_dialog_new_with_buttons (_("User Commands Dialog"),
								GTK_WINDOW(mw->getContainer()),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								"_OK",
								GTK_RESPONSE_OK,
								"_Cancel",
								GTK_RESPONSE_CANCEL,
								NULL));

	GtkWidget *content_area = gtk_dialog_get_content_area (dialog);
	GtkWidget *table = gtk_grid_new();
//	gtk_container_add(GTK_CONTAINER(content_area), table);

	string::size_type i = 0;
	StringList names;
	guint iuRow = 0,iuacolums= 0;
	vector<Widgets*> WidgList;
	while((i = commands.find("%[line:", i)) != string::npos) {
		i += 7;
		string::size_type j = commands.find(']', i);
		if(j == string::npos && i == string::npos)
			break;

		const string name = commands.substr(i, j - i);
		if(find(names.begin(), names.end(), name) == names.end()) {
			string caption = name;
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
				combo_sel = Util::toInt((*sec));
				if(combo_values.empty())
				{ combo_sel = -1; }

				combo_values.erase(sec);
				if(static_cast<size_t>(combo_sel) >= combo_values.size())
					combo_sel = -1; // default selection value too high
			}

			if(combo_sel >= 0) {
				iuacolums = 0;
				for(auto i = combo_values.begin(), iend = combo_values.end(); i != iend; ++i)
					Util::replace("\t", "/", *i);

				// if the combo has already been displayed before, retrieve the prev value and bypass combo_sel
				auto prev = find(combo_values.begin(), combo_values.end(), (ucParams["line:" + name]));
				if(prev != combo_values.end())
					combo_sel = prev - combo_values.begin();

				GtkWidget *comboBox = gtk_combo_box_text_new();

				while(!combo_values.empty()) {
					gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), combo_values.back().c_str() );
					combo_values.pop_back();
				}
				gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox),combo_sel);
				GtkWidget *label =  gtk_label_new(combo_caption.c_str());
				Widgets *wid = new Widgets();
				wid->widget = comboBox;
				wid->label = label;
				WidgList.push_back(wid);
	//			gtk_grid_attach(GTK_GRID(table), label, iuacolums++, iuRow,1,1);
	//			gtk_grid_attach(GTK_GRID(table), comboBox, iuacolums, ++iuRow,1,1);
	
		  } else {
				iuacolums = 0;
				GtkWidget *label = gtk_label_new(caption.c_str());
				GtkWidget *entry = gtk_entry_new();
	//			gtk_entry_set_text(GTK_ENTRY(entry),(ucParams["line:"+ name]).c_str());
				Widgets *wid = new Widgets();
				wid->widget = entry;
				wid->label = label;
				WidgList.push_back(wid);
	//			gtk_grid_attach(GTK_GRID(table),label,iuacolums++,iuRow,1,1);
	//			gtk_grid_attach(GTK_GRID(table),entry,iuacolums,++iuRow,1,1);

			}

			names.push_back(name);
		}
		i = j + 1;
	}

	if(names.empty())
		return true;
//	gint response = gtk_dialog_run(dialog);
//	if (response == GTK_RESPONSE_OK)
	{
		for(size_t i = 0, iend = WidgList.size(); i < iend; ++i)
		{
			Widgets *wid = WidgList[i];

			if(wid->label == NULL)
				continue;

		   if(wid->widget == NULL)
				continue;

			if(GTK_IS_COMBO_BOX_TEXT(wid->widget))
			{
				const gchar *value = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(wid->widget));
				ucParams["line:"+names[i]] = string (value);
			}
			else if( GTK_IS_ENTRY(wid->widget))
			{
				const gchar *value = gtk_editable_get_text(GTK_EDITABLE(wid->widget));
				ucParams["line:"+names[i]] = string(value);
			}
			else { return true;}

			WidgList.pop_back();
		}
		return true;
	}
	return false;
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
	gtk_editable_set_text(GTK_EDITABLE(getWidget("magnetEntry")), magnet.c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("magnetNameEntry")), name.c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("magnetSizeEntry")), Util::formatBytes(size).c_str());
	gtk_editable_set_text (GTK_EDITABLE(getWidget("exactSizeEntry")), Util::formatExactSize(size).c_str());
	gtk_editable_set_text (GTK_EDITABLE(getWidget("tthEntry")), tth.c_str());
	// chooser dialog
	GtkWidget *chooser = getWidget("flistDialog");
	gtk_window_set_title(GTK_WINDOW(chooser), _("Choose a directory"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	// choose
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("choiceCheckButton")), FALSE);
	setChooseMagnetDialog_gui();

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

void MainWindow::onBrowseMagnetButton_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	GtkWidget *dialog = mw->getWidget("flistDialog");
	//@TODO: show dialog
}

void MainWindow::onDowloadQueueToggled_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gtk_widget_set_sensitive(mw->getWidget("browseButton"), TRUE);
}

void MainWindow::onSearchMagnetToggled_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gtk_widget_set_sensitive(mw->getWidget("browseButton"), FALSE);
}

void MainWindow::onSetMagnetChoiceDialog_gui(GtkWidget*, gpointer data)
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
		if (!gtk_widget_get_visible(mw->getWidget("magnetPropertiesFrame")))
		{
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("dowloadQueueRadioButton"))))
			{
				GFile* file = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
				gchar *temp = g_file_get_path(file);
				if (temp)
				{
					path = string(temp) + G_DIR_SEPARATOR_S;
				}
				if (!File::isAbsolute(path))
					path = SETTING(DOWNLOAD_DIRECTORY);

				WSET("magnet-choose-dir", path);
				WSET("magnet-action", 1);
				g_free(temp);
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
		string magnet = gtk_editable_get_text(GTK_EDITABLE(mw->getWidget("magnetEntry")));

		WulforUtil::splitMagnet(magnet, name, size, tth);
		gboolean set = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("choiceCheckButton")));

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("dowloadQueueRadioButton"))))
		{
			GFile* file = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
			gchar *temp = g_file_get_path(file);
			if (temp)
			{
				path = string(temp) + G_DIR_SEPARATOR_S;
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
			SearchEntry *s = mw->addSearch_gui();
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
			QueueManager::getInstance()->add(name, size, TTHValue(tth) , HintedUser(make_shared<User>(User(CID())), dcpp::Util::emptyString));

			// automatically search for alternative download locations
			if (SETTING(AUTO_SEARCH))
				SearchManager::getInstance()->search(tth, 0, SearchManager::TYPE_TTH, SearchManager::SIZE_DONTCARE,
					dcpp::Util::emptyString);
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
	if (secondaryText.empty())
		return;

	//GtkAlertDialog* dialog = gtk_alert_dialog_new ("%s - %s", primaryText.c_str() , secondaryText.c_str() );
	//gtk_alert_dialog_show(dialog ,GTK_WINDOW(window));
}

void MainWindow::onSizeWindowState_gui(GtkWidget* /*widget*/,GtkAllocation*,gpointer data)
{
	MainWindow* mw = ( MainWindow*)data;
	if(!mw->is_maximized)
	{
	//	gtk_window_get_size (GTK_WINDOW (widget),
    //                     &mw->current_width,
     //                    &mw->current_height);
	}

}
/*
gboolean MainWindow::onWindowState_gui(GtkWidget*, GdkEventWindowState *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (mw->minimized  || (event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_WITHDRAWN)))
	{
		mw->minimized = true;
		if (SETTING(SettingsManager::AUTO_AWAY) && !Util::getAway())
			Util::setAway(true);
	}
	else if (!mw->minimized || (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED )!= 0)
	{
		mw->minimized = false;

		if (SETTING(SettingsManager::AUTO_AWAY) && !Util::getManualAway())
			Util::setAway(false);
	}

	mw->is_maximized =
    (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
	return GDK_EVENT_PROPAGATE;
}

gboolean MainWindow::onFocusIn_gui(GtkWidget*, GdkEventFocus*, gpointer data)
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

gboolean MainWindow::onCloseWindow_gui(GtkWidget*, GdkEvent*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (mw->onQuit)
	{
		mw->onQuit = false;
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

	if (response == GTK_RESPONSE_OK)
	{
		return FALSE;
	}

	return TRUE;
}
*/
gboolean MainWindow::onDeleteEventMagnetDialog_gui(GtkWidget *dialog, GdkEvent*, gpointer )
{
	gtk_widget_set_visible (dialog , FALSE);
	return TRUE;
}

void MainWindow::onPageSwitched_gui(GtkNotebook *notebook, GtkWidget*, guint num, gpointer data)
{
	MainWindow* mw = (MainWindow *)data;
	GtkWidget *child = gtk_notebook_get_nth_page(notebook, num);
	BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");

	if (entry)
	{
		entry->setActive_gui();
		mw->setTitle(entry->getLabelText()); // Update window title with selected tab label
	}
	GList *list = (GList *)g_object_get_data(G_OBJECT(notebook), "page-rotation-list");
	list = g_list_remove(list, (gpointer)child);
	list = g_list_prepend(list, (gpointer)child);
	g_object_set_data(G_OBJECT(notebook), "page-rotation-list", (gpointer)list);
}

void MainWindow::onPaneRealized_gui(GtkWidget *, gpointer )
{
//	MainWindow *mw = (MainWindow *)data;
	gint iposition = WGETI("transfer-pane-position");

	if (iposition > 10)
	{
		// @todo: fix get window height when maximized
//		gint iheight;
//		gtk_window_get_size(mw->window, NULL, &iheight);
//		gtk_paned_set_position(GTK_PANED(pane), iheight - iposition);
	}
}

typedef struct
{
	MainWindow* mw;
	GtkWidget* entry;
} ResponseData;

void MainWindow::onConnectClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *dialog, *label, *content_area, *entry;
 	GtkDialogFlags flags;

 	// Create the widgets
 	flags = GTK_DIALOG_DESTROY_WITH_PARENT;
 	dialog = gtk_dialog_new_with_buttons ("Connect to ",
                                       GTK_WINDOW(mw->window),
                                       flags,
                                       _("_OK"),
                                       GTK_RESPONSE_NONE,
                                       NULL);
 	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
 	label = gtk_label_new ("Address ?");
 	entry = gtk_entry_new();
 	
	ResponseData* dat = g_new(ResponseData ,1);
	dat->mw = mw;
	dat->entry = entry;
	g_signal_connect_data(dialog ,"response" , G_CALLBACK(responseDialogOnClicked_gui), dat , NULL , (GConnectFlags)0);
	// Add the label, and show everything we’ve added
	gtk_box_append (GTK_BOX (content_area), label);
	gtk_box_append (GTK_BOX (content_area), entry);
	gtk_widget_show (dialog);

}
void MainWindow::responseDialogOnClicked_gui(GtkWidget* dialog ,int , gpointer data)
{
	ResponseData* res = (ResponseData*)data;
	string saddress = gtk_editable_get_text( GTK_EDITABLE(res->entry));
	res->mw->showHub_gui(saddress);
	gtk_window_destroy(GTK_WINDOW(dialog));
}

void MainWindow::onFavoriteHubsClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showBook(Entry::FAVORITE_HUBS, new FavoriteHubs());
}

void MainWindow::onFavoriteUsersClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFavoriteUsers_gui();
}

void MainWindow::onCmdDebugClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showCmdDebug_gui();
}

void MainWindow::onSystemLogClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showSystemLog_gui();
}

void MainWindow::onNotepadClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showNotepad_gui();
}

void MainWindow::onUploadQueueClicked_gui(GtkWidget* , gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showUploadQueue_gui();
}

void MainWindow::onRecentHubClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showRecentHubs_gui();
}

void MainWindow::onDetectionClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showDetection_gui();
}

void MainWindow::onPublicHubsClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showPublicHubs_gui();
}

void MainWindow::onPreferencesClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	GtkWidget* response = WulforManager::get()->openSettingsDialog_gui();

	g_signal_connect(response , "response" ,G_CALLBACK(onResponse) ,mw);
	gtk_widget_show(response);

}
void MainWindow::onResponse(GtkWidget* wid , int response ,gpointer data)
{	
	MainWindow* mw = (MainWindow*)data;
	uint16_t ui16prevTCP = SETTING(TCP_PORT);
	uint16_t ui16prevUDP = SETTING(UDP_PORT);
	uint16_t ui16prevTLS = SETTING(TLS_PORT);

	auto prevConn = SETTING(INCOMING_CONNECTIONS);
	string sprevMapper = SETTING(MAPPER);
	string sprevBind = SETTING(BIND_ADDRESS);
	string sprevBind6 = SETTING(BIND_ADDRESS6);
	auto prevProxy = CONNSETTING(OUTGOING_CONNECTIONS);

	if (response == GTK_RESPONSE_OK)
	{
		//NOTE: BMDC
		try {
			ConnectivityManager::getInstance()->setup(SETTING(INCOMING_CONNECTIONS) != prevConn ||
				SETTING(TCP_PORT) != ui16prevTCP || SETTING(UDP_PORT) != ui16prevUDP || SETTING(TLS_PORT) != ui16prevTLS ||
				SETTING(MAPPER) != sprevMapper || SETTING(BIND_ADDRESS) != sprevBind || SETTING(BIND_ADDRESS6) != sprevBind6);
		} catch (const Exception& e) {
			//mw->showMessageDialog_gui(e.getError(),e.getError());
		}

		auto outConns = CONNSETTING(OUTGOING_CONNECTIONS);
		if(outConns != prevProxy || outConns == SettingsManager::OUTGOING_SOCKS5) {
			Socket::socksUpdated();
		}
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
		{
		// Search Spy
		BookEntry *entry = mw->findBookEntry(Entry::SEARCH_SPY);

		if (entry != NULL)
			dynamic_cast<SearchSpy *>(entry)->preferences_gui();
		}
		{
		// System Log
		BookEntry *entry = mw->findBookEntry(Entry::SYSTEML);

		if (entry != NULL)
			dynamic_cast<SystemLog *>(entry)->preferences_gui();
		}

		{
		// CMD Log
		BookEntry *entry = mw->findBookEntry(Entry::CMD);

		if (entry != NULL)
			dynamic_cast<cmddebug *>(entry)->preferences_gui();
		}

	}
	gtk_widget_hide(wid);
}

void MainWindow::onAwayClicked_gui(GtkWidget*, gpointer data)
{
	if(Util::getAway())
	{
		Util::switchAway();
		Util::setManualAway(false);

	}else
	{
		Util::switchAway();
		Util::setManualAway(true);
	}
}

void MainWindow::onTransferToggled_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *transfer = mw->transfers->getContainer();

	if (gtk_widget_get_visible(transfer)) {
		gtk_widget_set_visible (transfer, TRUE);
		WSET("hide-transfers",TRUE);
	} else {
		WSET("hide-transfers",FALSE);
		gtk_widget_set_visible (transfer, FALSE);
	}
}

void MainWindow::onHashClicked_gui(GtkWidget*, gpointer )
{
	WulforManager::get()->openHashDialog_gui();
}

#ifdef HAVE_LIBTAR
void MainWindow::onExportItemClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	ExportDialog *h = new ExportDialog(GTK_WINDOW(mw->getContainer()));
	h->run();
	if(h)
		delete h;
}
#endif

void MainWindow::onSearchClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->addSearch_gui();
}

void MainWindow::onSearchSpyClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showBook(Entry::SEARCH_SPY,new SearchSpy());
}

void MainWindow::onSearchADLClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showBook(Entry::SEARCH_ADL,new SearchADL());
}

void MainWindow::onDownloadQueueClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showBook(Entry::DOWNLOAD_QUEUE,new DownloadQueue());
}

void MainWindow::onFinishedDownloadsClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFinishedDownloads_gui();
}

void MainWindow::onFinishedUploadsClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFinishedUploads_gui();
}

void MainWindow::onQuitClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	// fix emit signal (status-icon menu quit)
	if (gtk_widget_get_visible(mw->getWidget("exitDialog")))
		return;

	mw->onQuit = true;
	gboolean bretVal; // Not interested in the value, though.
	g_signal_emit_by_name(mw->window, "delete-event", NULL, &bretVal);
}

void MainWindow::onOpenFileListClicked_gui(GtkWidget*, gpointer )
{
/*	MainWindow *mw = (MainWindow *)data;

	GtkWidget *chooser = mw->getWidget("flistDialog");
	gtk_window_set_title(GTK_WINDOW(chooser), _("Select filelist to browse"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Util::getListPath().c_str());

	gint response = gtk_widget_show(GTK_DIALOG(chooser));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *cptemp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));

		if (cptemp)
		{
			string spath(cptemp);
			g_free(cptemp);

			UserPtr user = DirectoryListing::getUserFromFilename(spath);
			if (user)
				mw->showShareBrowser_gui(HintedUser(user,string()), spath, "", 0, FALSE);
			else
				mw->setMainStatus_gui(_("Unable to load file list: Invalid file list name"));
		}
	}*/
}

void MainWindow::onOpenOwnListClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func1<MainWindow, bool> F1;
	F1 *func = new F1(mw, &MainWindow::openOwnList_client, FALSE);
	WulforManager::get()->dispatchClientFunc(func);

	mw->setMainStatus_gui(_("Loading Own file list"));
}

void MainWindow::onRefreshFileListClicked_gui(GtkWidget*, gpointer data)
{
	typedef Func0<MainWindow> F0;
	F0 *func = new F0((MainWindow *)data, &MainWindow::refreshFileList_client);
	WulforManager::get()->dispatchClientFunc(func);
}

void MainWindow::onReconnectClicked_gui(GtkWidget*, gpointer data)
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

void MainWindow::onCloseClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	int iNumber = gtk_notebook_get_current_page(GTK_NOTEBOOK(mw->note));
	if(iNumber != -1 )
		gtk_notebook_remove_page(GTK_NOTEBOOK(mw->note),iNumber);
}

void MainWindow::onPreviousTabClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->previousTab_gui();
}

void MainWindow::onNextTabClicked_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->nextTab_gui();
}

void MainWindow::onAboutClicked_gui(GtkWidget*, gpointer)
{
	
}

void MainWindow::onAboutDialogActivateLink_gui(GtkAboutDialog*, const gchar *link, gpointer)
{
	WulforUtil::openURI(link);
}

void MainWindow::onCloseBookEntry_gui(GtkWidget*, gpointer data)
{
	BookEntry *entry = (BookEntry *)data;
	WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void MainWindow::onLinkClicked_gui(GtkWidget *widget, gpointer )
{
	string slink = (gchar *)g_object_get_data(G_OBJECT(widget), "link");
	WulforUtil::openURI(slink);
}

void MainWindow::autoConnect_client()
{
	FavoriteManager* fm = FavoriteManager::getInstance();
	const FavHubGroups &favHubGroups = fm->getFavHubGroups();
	const FavoriteHubEntryList &favoriteHubs = fm->getFavoriteHubs();

	for (auto i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i)
	{
		FavoriteHubEntry *hub = *i;
		string sgroup = hub->getGroup();
		FavHubGroups::const_iterator it = favHubGroups.find(sgroup);

		if (it != favHubGroups.end())
		{
			const HubSettings* p = &(it->second);
			if (p->getAutoConnect())
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

	string slink = WulforManager::get()->getURL();

	if (slink.empty()) return;

	typedef Func1<MainWindow, string> F1;
	F1 *func1;

	if (WulforUtil::isHubURL(slink) && SETTING(URL_HANDLER))
	{
		typedef Func2<MainWindow, string, string> F2;
		F2 *func = new F2(this, &MainWindow::showHub_gui, slink, "");
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if (WulforUtil::isMagnet(slink) && SETTING(MAGNET_REGISTER))
	{
		func1 = new F1(this, &MainWindow::actionMagnet_gui, slink);
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
		dcdebug("%s",e.getError().c_str());
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

	typedef Func5<MainWindow, HintedUser, string, string, int64_t,bool> F5;
	F5 *func = new F5(this, &MainWindow::showShareBrowser_gui,HintedUser(user,dcpp::Util::emptyString), path, "", 0, useSetting);
	WulforManager::get()->dispatchGuiFunc(func);
}

int MainWindow::FileListQueue::run() {
	setThreadPriority(Thread::LOW);

	while(true) {
		s.wait(15000);
		if(stop || fileLists.empty()) {
			break;
		}

		DirectoryListInfo* i = nullptr;
		{
			Lock l(cs);
			i = fileLists.front();
			fileLists.pop_front();
		}

		if(Util::fileExists(i->file)) {
			DirectoryListing* pdl = new DirectoryListing(i->user);
			try {
				pdl->loadFile(i->file);
				ADLSearchManager::getInstance()->matchListing(*pdl);
				ClientManager::getInstance()->checkCheating(i->user, pdl);
			} catch(...)
			{
				//...
			}
			delete pdl;
		}
		delete i;
	}
	stop = true;
	return 0;
}

void MainWindow::on(LogManagerListener::Message, time_t t, const string &message,int ) noexcept
{
	typedef Func2<MainWindow, string, time_t> F2;
	F2 *func = new F2(this, &MainWindow::setMainStatus_gui, message, t);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) noexcept
{
	if(item->isSet(QueueItem::FLAG_TESTSUR)) return;

	typedef Func3<MainWindow, string, string, Notify::TypeNotify> F3;

	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
	{
		const HintedUser user = item->getDownloads()[0]->getHintedUser();
		string listName = item->getListName();

		F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("file list from "), WulforUtil::getNicks(user),
			Notify::DOWNLOAD_FINISHED_USER_LIST);

		WulforManager::get()->dispatchGuiFunc(f3);

		typedef Func5<MainWindow, HintedUser, string, string, int64_t ,bool> F5;
		F5 *func = new F5(this, &MainWindow::showShareBrowser_gui, user, listName, dir,avSpeed, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if(item->isSet(QueueItem::FLAG_USER_LIST) && item->isSet(QueueItem::FLAG_CHECK_FILE_LIST))
	{
        DirectoryListInfo* i = new DirectoryListInfo(item->getDownloads()[0]->getHintedUser(), item->getListName(), dir, avSpeed);
        try {
        if(listQueue.stop) {
			listQueue.stop = false;
			listQueue.start();
		}
		{
			Lock l(listQueue.cs);
			listQueue.fileLists.push_back(i);
		}
		listQueue.s.signal();
		}catch(...){ }

	}else if (!item->isSet(QueueItem::FLAG_XML_BZLIST))
	{
		F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("<b>file:</b> "), item->getTarget(), Notify::DOWNLOAD_FINISHED);
		WulforManager::get()->dispatchGuiFunc(f3);
	}
}

void MainWindow::on(TimerManagerListener::Second, uint64_t ticks) noexcept
{
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

	//typedef Func5<MainWindow, string, string, string, string, string> F5;
	//F5 *func = new F5(this, &MainWindow::setStats_gui, hubs, downloadSpeed, downloaded, uploadSpeed, uploaded);
	//WulforManager::get()->dispatchGuiFunc(func);

	string file;
	uint64_t bytes = 0;
	size_t files = 0;

	HashManager::getInstance()->getStats(file, bytes, files);

	//typedef Func4<MainWindow, string, uint64_t, size_t, uint32_t> FX;
	//FX *funcx = new FX(this, &MainWindow::updateStats_gui, file, bytes, files, GET_TICK());
	//WulforManager::get()->dispatchGuiFunc(funcx);
}

void MainWindow::on(dcpp::TimerManagerListener::Minute, uint64_t ) noexcept
{

}

void MainWindow::onTTHFileDialog_gui(GtkWidget*, gpointer /*data*/)
{
//	MainWindow *mw =(MainWindow *)data;
//	GtkWidget *dialog = mw->getWidget("TTHFileDialog");
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
}

void MainWindow::onTTHFileButton_gui(GtkWidget* , gpointer /*data*/)
{
//	MainWindow *mw = (MainWindow *)data;
//	GtkWidget *chooser = mw->getChooserDialog_gui();
//	gtk_window_set_title(GTK_WINDOW(chooser), _("Select file to Get TTH"));
//	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
//	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), "/home/");

//	gint response = gtk_widget_show(GTK_DIALOG(chooser));

//	if (response == GTK_RESPONSE_OK)
	{
//		gchar *cptemp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
//		gtk_widget_set_sensitive(mw->getWidget("buttonok"),FALSE);
//		TTHHash hasht;
//		if(hasht.stop)
//		{
//			hasht.stop = false;
//			Lock l(hasht.cs);
//			hasht.mw = mw;
//			hasht.filename = cptemp;
//			hasht.start();
//			hasht.s.signal();
//		}
	}
}

void MainWindow::back(string tth, string filename, int64_t size)
{
	string magnetlink = "magnet:?xt=urn:tree:tiger:" + tth + "&xl=" + Util::toString(size) + "&dn=" + Util::encodeURI(Text::fromT(Util::getFileName(filename)));
	gtk_editable_set_text(GTK_EDITABLE(getWidget("entrymagnet")), magnetlink.c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("entrytthfileresult")), tth.c_str());
}

void MainWindow::progress(bool progress)
{
	if(progress)
	{
		gtk_progress_bar_pulse (GTK_PROGRESS_BAR (getWidget("progressbar")));
	}
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
				string sTTH;
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
				delete [] buf;
				tth.finalize();
				f.close();
				strcpy(&sTTH[0], tth.getRoot().toBase32().c_str());

				typedef Func3<MainWindow, std::string, std::string, int64_t> F3;
				F3 *func = new F3(mw,&MainWindow::back,sTTH,filename,sized);
				WulforManager::get()->dispatchGuiFunc(func);
				typedef Func1<MainWindow, bool> F1;
				F1 *func1 = new F1(mw,&MainWindow::progress,false);
				WulforManager::get()->dispatchGuiFunc(func1);

			LogManager::getInstance()->message("TTH: " + sTTH + "filename: " + filename + "sized: " + dcpp::Util::toString(sized));
			stop = true;
		} catch(...) { }
	}

	stop = true;
	return 0;
}
///Close all
void MainWindow::onCloseAllHub_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	while(!mw->Hubs.empty())
	{
		Hub *hub = dynamic_cast<Hub*>(mw->Hubs.back());
		if(!hub) continue;//should never hapen but ...
		typedef Func1<MainWindow,BookEntry*> F1;
		F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,hub);
		WulforManager::get()->dispatchGuiFunc(func);

		mw->Hubs.pop_back();
	}
	mw->Hubs.clear();
}
///PM
void MainWindow::onCloseAllPM_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(auto i= mw->privateMessage.begin(); i != mw->privateMessage.end();++i)
	{
		PrivateMessage *pm = dynamic_cast<PrivateMessage*>(*i);
		if(!pm) continue;
		typedef Func1<MainWindow,BookEntry*> F1;
		F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,pm);
		WulforManager::get()->dispatchGuiFunc(func);
	}

	mw->privateMessage.clear();
}
///rec-all
void MainWindow::onReconectAllHub_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	for(auto i= mw->Hubs.begin(); i != mw->Hubs.end();++i)
	{
		Hub *hub = dynamic_cast<Hub*>(*i);
		if(hub)
			hub->reconnect_client();
	}
}
///PM
void MainWindow::onCloseAllofPM_gui(GtkWidget*, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	vector<Entry*> noff;

	for(auto i= mw->privateMessage.begin(); i != mw->privateMessage.end();++i)
	{
		PrivateMessage *pm = dynamic_cast<PrivateMessage*>(*i);

		if( pm && pm->getIsOffline())
		{
			typedef Func1<MainWindow,BookEntry*> F1;
			F1 *func = new F1(mw,&MainWindow::removeBookEntry_gui,pm);
			WulforManager::get()->dispatchGuiFunc(func);

		}else {noff.push_back(*i);}
	}
	mw->privateMessage = noff;
}

void MainWindow::updateStats_gui(string file, uint64_t bytes, size_t files, uint32_t tick)
{
	if (bytes > startBytes)
		startBytes = bytes;

	if (files > startFiles)
		startFiles = files;

	double diff = tick - startTime;
	bool bpaused = HashManager::getInstance()->isHashingPaused();

	if (diff < 1000 || files == 0 || bytes == 0 || bpaused)
	{
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), "0%");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), 0.0);
	}
	else
	{
		double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")), string(Util::formatBytes((int64_t)speedStat) + "/" + _("s") + ", " + Util::formatBytes(bytes) + _(" left")).c_str());

		if (speedStat == 0)
		{
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), _("-:--:-- left"));
		}
		else
		{
			double ss = (double)bytes / speedStat;
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), string(Util::formatSeconds((int64_t)ss) + _(" left")).c_str());
		}
	}

	if (startFiles == 0 || startBytes == 0)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), "100%");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), 1.0);
	}
	else
	{
		double progress = ((0.5 * (double)(startFiles - files)/(double)startFiles) + (0.5 * (double)(startBytes - bytes)/(double)startBytes));
		char buf[24];
		snprintf(buf, sizeof(buf), "%.0lf%%", progress * 100);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), buf);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbarHashBar")), progress);
	}

	if (files == 0)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")), _("100% Done"));
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbar")), 1.0);
	}
	else
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")), file.c_str());
	}
}

void MainWindow::onShortcutsWin(GtkWidget*, gpointer)
{
	ShortCuts* sc = new ShortCuts();
	sc->show();
}
