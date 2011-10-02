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

#ifndef WULFOR_MAIN_WINDOW_HH
#define WULFOR_MAIN_WINDOW_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/DownloadManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/TimerManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/UserCommand.h>
#include <dcpp/HttpDownload.h>

#include "wulformanager.hh"
#include "entry.hh"
#include "treeview.hh"
#include "transfers.hh"
#include "message.hh"
#include "notify.hh"
#include "privatemessage.hh"
#include "hub.hh"

#include <queue>

class BookEntry;
class Search;

class MainWindow:
	public Entry,
	public dcpp::LogManagerListener,
	public dcpp::QueueManagerListener,
	public dcpp::TimerManagerListener
{
	public:
		MainWindow();
		~MainWindow();

		// Inherited from Entry
		GtkWidget *getContainer();

		// GUI functions
		void show();
		void setTitle(const std::string& text);
		void setUrgent_gui();
		bool isActive_gui();
		void removeBookEntry_gui(BookEntry *entry/*EntryItems *bEntry*/);
		GtkWidget *currentPage_gui();
		void raisePage_gui(GtkWidget *page);
		static bool getUserCommandLines_gui(const std::string &command, dcpp::StringMap &ucParams);
		void propertiesMagnetDialog_gui(std::string magnet);
		void showMessageDialog_gui(const std::string primaryText, const std::string secondaryText);
		void showDownloadQueue_gui();
		void showFavoriteHubs_gui();
		void showFavoriteUsers_gui();
		void showFinishedDownloads_gui();
		void showFinishedUploads_gui();
		void showHub_gui(std::string address, std::string encoding = "");
		void showSearchSpy_gui();
		//Add
		void showSystem_gui(); //System Tab
		void showIgnore_gui(); //Ignore Tab
		void showADLSearch_gui(); //ADL Tab
		void showNotepad_gui(); //Notepad Tab
		void showRecentHub_gui(); //RecentHub Tab
		//void showHigliting_gui(); //Higliting Tab
		void showUploadQueue();
		void showDetection_gui(); //Detection Tab
		void showcmddebug_gui(); //CMD DEBUG Tab
		//END
		void addPrivateMessage_gui(Msg::TypeMsg typemsg, std::string cid, std::string hubUrl = "", std::string message = "", bool useSetting = FALSE);
		void addPrivateStatusMessage_gui(Msg::TypeMsg typemsg, std::string cid, std::string message = "");
		void showPublicHubs_gui();
		void showShareBrowser_gui(dcpp::UserPtr user, std::string file, std::string dir, bool useSetting,int64_t speed);//ch
		Search *addSearch_gui();
		void addSearch_gui(std::string magnet);
		void actionMagnet_gui(std::string magnet);
		void setMainStatus_gui(std::string text, time_t t = time(NULL));
		void showNotification_gui(std::string head, std::string body, Notify::TypeNotify notify);
		GtkWidget* getChooserDialog_gui();
		void fileToDownload_gui(std::string magnet, std::string path);
		void setAwayIcon(bool isAway)
		{  
			if(isAway)
				gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("AwayIcon")), "bmdc-away-on");
			else	
				gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("AwayIcon")), "bmdc-away");
		}
		// Client functions
		void openOwnList_client(bool useSetting);

	private:
        //Checker
        class DirectoryListInfo {
            public:
                DirectoryListInfo(const dcpp::UserPtr& aUser, const std::string& aFile, const std::string& aDir, int64_t aSpeed) : user(aUser), file(aFile), dir(aDir), speed(aSpeed) { }
                dcpp::UserPtr user;
                std::string file;
                std::string dir;
                int64_t speed;
			};
       /* class DirectoryBrowseInfo {
            public:
                DirectoryBrowseInfo(const dcpp::UserPtr& ptr, std::string aText) : user(ptr), text(aText) { }
                dcpp::UserPtr user;
                std::string text;
        };*/
        class FileListQueue: public dcpp::Thread {
            public:
                bool stop;
                dcpp::Semaphore s;
                dcpp::CriticalSection cs;
				std::list<DirectoryListInfo*> fileLists;

				FileListQueue() : stop(true) {}
				~FileListQueue() throw() {
					shutdown();
				}
            int run();
            void shutdown() {
                stop = true;
                s.signal();
            }
        };

		// GUI functions
		void loadIcons_gui();
		void showTransfersPane_gui();
		void autoOpen_gui();
		void addTabMenuItem_gui(GtkWidget* menuItem, GtkWidget* page);
		void removeTabMenuItem_gui(GtkWidget *menuItem);
		void addBookEntry_gui(BookEntry *entry);
		void previousTab_gui();
		void nextTab_gui();
		BookEntry *findBookEntry(const EntryType type, const std::string &id = "");
		void createStatusIcon_gui();
		void updateStatusIconTooltip_gui(std::string download, std::string upload);
		void setStats_gui(std::string hubs, std::string downloadSpeed,
			std::string downloaded, std::string uploadSpeed, std::string uploaded);
		void setToolbarButton_gui();
		void setTabPosition_gui(int position);
		void setToolbarStyle_gui(int style);
		void removeTimerSource_gui();
		void setChooseMagnetDialog_gui();
		void showMagnetDialog_gui(const std::string &magnet, const std::string &name, const int64_t size,
			const std::string &tth);

		// GUI Callbacks
		static gboolean onWindowState_gui(GtkWidget *widget, GdkEventWindowState *event, gpointer data);
		static gboolean onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data);
		static gboolean onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data);
		static gboolean onKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onButtonReleasePage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean animationStatusIcon_gui(gpointer data);
		static void onRaisePage_gui(GtkMenuItem *item, gpointer data);
		static void onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data);
		static void onPaneRealized_gui(GtkWidget *pane, gpointer data);
		static void onConnectClicked_gui(GtkWidget *widget, gpointer data);
		static void onFavoriteHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void onFavoriteUsersClicked_gui(GtkWidget *widget, gpointer data);
		static void onPublicHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void onPreferencesClicked_gui(GtkWidget *widget, gpointer data);
		static void onHashClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchSpyClicked_gui(GtkWidget *widget, gpointer data);
		static void onDownloadQueueClicked_gui(GtkWidget *widget, gpointer data);
		static void onFinishedDownloadsClicked_gui(GtkWidget *widget, gpointer data);
		static void onFinishedUploadsClicked_gui(GtkWidget *widget, gpointer data);
		//Add
		static void onNotepad_gui(GtkWidget *widget, gpointer data);
		static void onADLSearch_gui(GtkWidget *widget, gpointer data);
		static void onSystem_gui(GtkWidget *widget, gpointer data);
		static void onIgnore_gui(GtkWidget *widget, gpointer data);
		static void onAway_gui(GtkWidget *widget, gpointer data);
		static void onLimiting(GtkWidget *widget, gpointer data);
		static void onRecent_gui(GtkWidget *widget, gpointer data);
		static void onTTHFileDialog_gui(GtkWidget *widget, gpointer data);
		static void onTTHFileButton_gui(GtkWidget *widget, gpointer data);
		//static void onHighliting(GtkWidget *widget , gpointer data);
		static void onUploadQueue_gui(GtkWidget *widget , gpointer data);
		static void onDetection(GtkWidget *widget , gpointer data);
		static void onDebugCMD(GtkWidget *widget, gpointer data);
        /*Close**/
		static void onCloseAllHub_gui(GtkWidget *widget, gpointer data);
		static void onCloseAllPM_gui(GtkWidget *widget, gpointer data);
		static void onCloseAllSearch_gui(GtkWidget *widget, gpointer data);
		//static void onCloseAllOffHub_gui(GtkWidget *widget, gpointer data);
		static void onCloseAlloffPM_gui(GtkWidget *widget, gpointer data);
		///Limiting Icons
		static void onLimitingMenuItem_gui(GtkWidget *widget, gpointer data);
		static void onLimitingDisable(GtkWidget *widget, gpointer data);
		//End
		static void onQuitClicked_gui(GtkWidget *widget, gpointer data);
		static void onOpenFileListClicked_gui(GtkWidget *widget, gpointer data);
		static void onOpenOwnListClicked_gui(GtkWidget *widget, gpointer data);
		static void onRefreshFileListClicked_gui(GtkWidget *widget, gpointer data);
		static void onReconnectClicked_gui(GtkWidget *widget, gpointer data);
		static void onCloseClicked_gui(GtkWidget *widget, gpointer data);
		static void onPreviousTabClicked_gui(GtkWidget* widget, gpointer data);
		static void onNextTabClicked_gui(GtkWidget* widget, gpointer data);
		static void onAboutClicked_gui(GtkWidget *widget, gpointer data);
		static void onAboutDialogActivateLink_gui(GtkAboutDialog *dialog, const gchar *link, gpointer data);
		static void onCloseBookEntry_gui(GtkWidget *widget, gpointer data);
		static void onStatusIconActivated_gui(GtkStatusIcon *statusIcon, gpointer data);
		static void onStatusIconPopupMenu_gui(GtkStatusIcon *statusIcon, guint button, guint time, gpointer data);
		static void onStatusIconBlinkUseToggled_gui(GtkWidget *widget, gpointer data);
		static void onShowInterfaceToggled_gui(GtkCheckMenuItem *item, gpointer data);
		static void onLinkClicked_gui(GtkWidget *widget, gpointer data);
		static void onTransferToggled_gui(GtkWidget *widget, gpointer data);
		static void onBrowseMagnetButton_gui(GtkWidget *widget, gpointer data);
		static void onDowloadQueueToggled_gui(GtkWidget *widget, gpointer data);
		static void onSearchMagnetToggled_gui(GtkWidget *widget, gpointer data);
		static void onSetMagnetChoiceDialog_gui(GtkWidget *widget, gpointer data);
		static void onResponseMagnetDialog_gui(GtkWidget *dialog, gint response, gpointer data);
		static gboolean onDeleteEventMagnetDialog_gui(GtkWidget *dialog, GdkEvent *event, gpointer data);

		// Client functions
		void autoConnect_client();
		void startSocket_client();
		void refreshFileList_client();
		void addFileDownloadQueue_client(std::string name, int64_t size, std::string tth);
		void getAway();
		void EnbDsbLimit();
		void checkUpdateofGeoIp(bool v6);
		void updateGeoIp(bool v6);
		void completeGeoIpUpdate(bool v6);

		// Client callbacks
		virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string &m) throw();
		virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem *item, const std::string& dir, int64_t avSpeed) throw();
		virtual void on(dcpp::TimerManagerListener::Second, uint64_t ticks) throw();

		GtkWindow *window;
		Transfers* transfers;
		GtkStatusIcon *statusIcon;
		int64_t lastUpdate, lastUp, lastDown;
		bool minimized;
		dcpp::StringList EntryList;
		guint timer;
		int statusFrame;
		bool useStatusIconBlink;
		bool onQuit;
		bool isLimiting;
		FileListQueue listQueue;

		std::vector<PrivateMessage*> allprivatemess;
		std::vector<Hub*> allhub;
		std::vector<Search*> allsearch;
		
		static const int maxTooltipCount = 5;//TODO setting;
		std::queue<std::string> statusTexts;
		
		enum
		{
			CONN_GEOIP_V4,
			CONN_GEOIP_V6,
			CONN_LAST
		};
		std::unique_ptr<dcpp::HttpDownload> conns[CONN_LAST];
};

#else
class MainWindow;
#endif
