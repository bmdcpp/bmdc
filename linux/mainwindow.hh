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

#ifndef _BMDC_MAIN_WINDOW_HH_
#define _BMDC_MAIN_WINDOW_HH_

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ConnectionManager.h"
#include "../dcpp/DownloadManager.h"
#include "../dcpp/LogManager.h"
#include "../dcpp/QueueManager.h"
#include "../dcpp/TimerManager.h"
#include "../dcpp/UploadManager.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/UserCommand.h"
#include "../dcpp/StringTokenizer.h"

#include "entry.hh"
#include "treeview.hh"
#include "transfers.hh"
#include "message.hh"
#include "notify.hh"
#include "SearchEntry.hh"
#include "settingsmanager.hh"

#include <queue>
#include <vector>

class BookEntry;
class SearchEntry;
class Search;
class PrivateMessage;
class Hub;

class MainWindow:
	public Entry,
	public dcpp::LogManagerListener,
	public dcpp::QueueManagerListener,
	public dcpp::TimerManagerListener
{
	private:
		using dcpp::LogManagerListener::on;
		using dcpp::QueueManagerListener::on;
		using dcpp::TimerManagerListener::on;
	public:
		MainWindow(GtkWidget* window = NULL);
		virtual ~MainWindow();

		// Inherited from Entry
		GtkWidget *getContainer();

		static void onResponse(GtkWidget* wid , int response , gpointer data);

		// GUI functions
		void show();
		void setTitle(const std::string& text);
		void setUrgent_gui();
		bool isActive_gui();
		void removeBookEntry_gui(BookEntry *entry);
		GtkWidget *currentPage_gui();
		void raisePage_gui(GtkWidget *page);
		static bool getUserCommandLines_gui(const std::string &commands, dcpp::ParamMap &ucParams);
		void propertiesMagnetDialog_gui(std::string magnet);
		void showMessageDialog_gui(const std::string primaryText, const std::string secondaryText);

		void showBook(const EntryType type, BookEntry* book);

		void showFavoriteHubs_gui();
		void showFavoriteUsers_gui();
		void showFinishedDownloads_gui();
		void showFinishedUploads_gui();
		void showHub_gui(std::string address, std::string encoding = WGETS("default-charset") );
		void showSearchSpy_gui();
		void showSearchADL_gui();
		void showDetection_gui();
		void showCmdDebug_gui();
		void showSystemLog_gui();
		void showNotepad_gui();
		void showUploadQueue_gui();
		void showRecentHubs_gui();

		void addPrivateMessage_gui(Msg::TypeMsg typemsg, std::string cid, std::string hubUrl = "", std::string message = "", bool useSetting = FALSE);
		void addPrivateStatusMessage_gui(Msg::TypeMsg typemsg, std::string cid, std::string message = "");
		void addPublicStatusMessage_gui(std::string hub,std::string message, Msg::TypeMsg typemsg, Sound::TypeSound sound, Notify::TypeNotify notify);
		void showPublicHubs_gui();
		void showShareBrowser_gui(dcpp::HintedUser user, std::string file, std::string dir, int64_t speed ,bool useSetting);
		SearchEntry *addSearch_gui();
		void addSearch_gui(std::string magnet);
		void actionMagnet_gui(std::string magnet);
		void setMainStatus_gui(std::string text, time_t t = time(NULL));
		void showNotification_gui(std::string head, std::string body, Notify::TypeNotify notify);
		GtkWidget* getChooserDialog_gui();
		void fileToDownload_gui(std::string magnet, std::string path);

		// Client functions
		void openOwnList_client(bool useSetting);

		SearchEntry *getSearchEntry () { return dynamic_cast<SearchEntry*>(findBookEntry(Entry::SEARCHS));}

	private:
		typedef std::pair<std::string, std::string> ParamPair;
		typedef std::vector<ParamPair> ListParamPair;
		// GUI functions
		void showTransfersPane_gui();
		void autoOpen_gui();
		void addTabMenuItem_gui(GMenu* menuItem, GtkWidget* page);
		void removeTabMenuItem_gui(GtkWidget *menuItem);
		void addBookEntry_gui(BookEntry *entry);
		void previousTab_gui();
		void nextTab_gui();
		BookEntry *findBookEntry(const EntryType type, const std::string &id = "");

		void setStats_gui(std::string hubs, std::string downloadSpeed,
			std::string downloaded, std::string uploadSpeed, std::string uploaded);
		void setTabPosition_gui(int position);

		void setChooseMagnetDialog_gui();
		void showMagnetDialog_gui(const std::string &magnet, const std::string &name, const int64_t size,
			const std::string &tth);
		void setStatRate_gui();
		// GUI Callbacks
		static void onSizeWindowState_gui(GtkWidget* widget,GtkAllocation *allocation,gpointer data);
		static void onPageSwitched_gui(GtkNotebook *notebook, GtkWidget *page, guint num, gpointer data);
		static void onPaneRealized_gui(GtkWidget *pane, gpointer data);
		static void onConnectClicked_gui(GtkWidget *widget, gpointer data);
		static void onFavoriteHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void onFavoriteUsersClicked_gui(GtkWidget *widget, gpointer data);
		static void onPublicHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void onPreferencesClicked_gui(GtkWidget *widget, gpointer data);
		static void onHashClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchADLClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchSpyClicked_gui(GtkWidget *widget, gpointer data);
		static void onDownloadQueueClicked_gui(GtkWidget *widget, gpointer data);
		static void onFinishedDownloadsClicked_gui(GtkWidget *widget, gpointer data);
		static void onFinishedUploadsClicked_gui(GtkWidget *widget, gpointer data);
		static void onQuitClicked_gui(GtkWidget *widget, gpointer data);
		static void onOpenFileListClicked_gui(GtkWidget *widget, gpointer data);
		static void onOpenOwnListClicked_gui(GtkWidget *widget, gpointer data);
		static void onRefreshFileListClicked_gui(GtkWidget *widget, gpointer data);
		static void onReconnectClicked_gui(GtkWidget *widget, gpointer data);
		static void onCloseClicked_gui(GtkWidget *widget,GVariant* v, gpointer data);
		static void onPreviousTabClicked_gui(GtkWidget* widget, gpointer data);
		static void onNextTabClicked_gui(GtkWidget* widget, gpointer data);
		static void onAboutClicked_gui(GtkWidget *widget, gpointer data);
		static void onAboutDialogActivateLink_gui(GtkAboutDialog *dialog, const gchar *link, gpointer data);
		static void onCloseBookEntry_gui(GtkWidget *widget, gpointer data);
		static void onLinkClicked_gui(GtkWidget *widget, gpointer data);
		static void onTransferToggled_gui(GtkWidget *widget, gpointer data);
		static void onBrowseMagnetButton_gui(GtkWidget *widget, gpointer data);
		static void onDowloadQueueToggled_gui(GtkWidget *widget, gpointer data);
		static void onSearchMagnetToggled_gui(GtkWidget *widget, gpointer data);
		static void onSetMagnetChoiceDialog_gui(GtkWidget *widget, gpointer data);
		static void onResponseMagnetDialog_gui(GtkWidget *dialog, gint response, gpointer data);
		static gboolean onDeleteEventMagnetDialog_gui(GtkWidget *dialog, GdkEvent *event, gpointer data);
		static gboolean onMenuButtonClicked_gui(GtkWidget *widget, gpointer data);
		static gboolean onAddButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onHubClicked_gui(GtkWidget *widget, gpointer data);
		static void onCmdDebugClicked_gui( GtkWidget *widget, gpointer data);
		static void onSystemLogClicked_gui(GtkWidget *widget, gpointer data);
		static void onNotepadClicked_gui(GtkWidget *widget, gpointer data);
		static void onUploadQueueClicked_gui(GtkWidget *widget, gpointer data);
		static void onDetectionClicked_gui(GtkWidget *widget, gpointer data);
		static void onRecentHubClicked_gui(GtkWidget *widget, gpointer data);
		static void onAwayClicked_gui(GtkWidget *widget, gpointer data);
		static void onLimitingMenuItem_gui(GtkRange *widget, gpointer data);
		static void onTTHFileDialog_gui(GtkWidget *widget, gpointer data);
		static void onTTHFileButton_gui(GtkWidget *widget, gpointer data);

		static void onCloseAllHub_gui(GtkWidget *widget, gpointer data);
		static void onCloseAllPM_gui(GtkWidget *widget, gpointer data);
		static void onCloseAllofPM_gui(GtkWidget *widget, gpointer data);
		static void onReconectAllHub_gui(GtkWidget *widget, gpointer data);

		static void onShortcutsWin(GtkWidget* widget, gpointer data);

		static void onAboutConfigClicked_gui(GtkWidget *widget, gpointer data);
		#ifdef HAVE_LIBTAR
		static void onExportItemClicked_gui(GtkWidget *widget, gpointer data);
		#endif
        static void onPopupPopover(GtkWidget* widget , gpointer data);
		// Client functions
		void autoConnect_client();
		void startSocket_client();
		void refreshFileList_client();
		void addFileDownloadQueue_client(std::string name, int64_t size, std::string tth);
		void removeItemFromList(Entry::EntryType type, std::string id);
		void setInitThrotles();

		// Client callbacks
		virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string &m,int sev) noexcept;
		virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem *item, const std::string& dir, int64_t avSpeed) noexcept;
		virtual void on(dcpp::TimerManagerListener::Second, uint64_t ticks) noexcept;
		virtual void on(dcpp::TimerManagerListener::Minute, uint64_t ticks) noexcept;
		static void onButtonPressed_gui(GtkGestureClick* self, gint n_press, gdouble x, gdouble y, gpointer user_data);
		static void responseDialogOnClicked_gui(GtkWidget* dialog ,int, gpointer data);

		GtkWidget *window;
		int current_width,current_height;
		gboolean is_maximized;
		Transfers* transfers;
		bool minimized;

		int64_t lastUpdate, lastUp, lastDown;
		dcpp::StringList EntryList;
		int statusFrame;
		bool onQuit;

		class DirectoryListInfo {
            public:
                DirectoryListInfo(const dcpp::HintedUser& hintedUser, const std::string& aFile, const std::string& aDir, int64_t aSpeed) : user(hintedUser), file(aFile), dir(aDir), speed(aSpeed) { }
                dcpp::HintedUser user;
                std::string file;
                std::string dir;
                int64_t speed;
		};

		class FileListQueue: public dcpp::Thread {
            public:
                bool stop;
                dcpp::Semaphore s;
                dcpp::CriticalSection cs;
				std::list<DirectoryListInfo*> fileLists;

				FileListQueue() : stop(true) {}
				~FileListQueue() noexcept {
					shutdown();
				}
            int run();
            void shutdown() {
                stop = true;
                s.signal();
            }
        };

        void back(std::string TTH, std::string filename, int64_t size);
        void progress(bool progress);

        struct TTHHash: public dcpp::Thread
        {
			public:
				bool stop;
				dcpp::Semaphore s;
				dcpp::CriticalSection cs;
				std::string filename;
				MainWindow *mw;
				TTHHash(): stop(true), mw(NULL) { }
				~TTHHash() noexcept
				{ shutdown(); }
				void shutdown()
				{
					stop = true;
					s.signal();
				}
				int run();
		};

		struct Widgets
		{
		  public:
			Widgets() : widget(NULL), label(NULL) { }
			GtkWidget *widget;
			GtkWidget *label;
		};

		std::vector<Entry*> privateMessage;
		std::vector<Entry*> Hubs;

		FileListQueue listQueue;

		std::queue<std::string> statustext;
		// Hash statusbar
		uint64_t startBytes;
		size_t startFiles;
		uint32_t startTime;
		void updateStats_gui(std::string file, uint64_t bytes, size_t files, uint32_t tick);

		GtkWidget *statusBar,*note;
		bool bText;
		static const GActionEntry win_entries[] ;

};

#else
class MainWindow;
#endif
