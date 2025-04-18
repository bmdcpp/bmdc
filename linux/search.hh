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

#ifndef _SEARCH_HH
#define _SEARCH_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/SearchManager.h"
#include "../dcpp/SearchResult.h"
#include "../dcpp/format.h"
#include "bookentry.hh"
#include "treeview.hh"

class UserCommandMenu;

class Search:
	public BookEntry,
	public dcpp::SearchManagerListener,
	public dcpp::ClientManagerListener
{
	public:
		Search(const std::string& str);
		virtual ~Search();
		virtual void show();

		void putValue_gui(const std::string &str, int64_t size, dcpp::SearchManager::SizeModes mode, dcpp::SearchManager::TypeModes type);

	private:
		using dcpp::SearchManagerListener::on;
		using dcpp::ClientManagerListener::on;
		// Keep these and the items in .ui file in same order, otherwise it will break
		typedef enum
		{
			NOGROUPING = 0,
			FILENAME,
			FILEPATH,
			SIZE,
			CONNECTION,
			TTH,
			NICK,
			HUB,
			TYPE
		} GroupType;

		// GUI functions
		void initHubs_gui();
		void addHub_gui(std::string name, std::string url);
		void modifyHub_gui(std::string name, std::string url, bool op = TRUE);
		void removeHub_gui(std::string url);
		void popupMenu_gui();
		void setStatus_gui(std::string statusBar, std::string text);
		void search_gui();
		void addResult_gui(const dcpp::SearchResultPtr result);
		void updateParentRow_gui(GtkTreeIter *parent, GtkTreeIter *child = NULL);
		void ungroup_gui();
		void regroup_gui();
		std::string getGroupingColumn(GroupType groupBy);

		void set_Header_tooltip_gui();
		void columnHeader(int num, std::string name);

		// GUI callbacks
//		static gboolean onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data);
//		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
//		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
//		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
//		static gboolean onSearchEntryKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean searchFilterFunc_gui(GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
		static void onComboBoxChanged_gui(GtkWidget *widget, gpointer data);
		static void onGroupByComboBoxChanged_gui(GtkWidget* widget, gpointer data);
		static void onSearchButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onFilterButtonToggled_gui(GtkToggleButton *button, gpointer data);
		static void onSlotsButtonToggled_gui(GtkToggleButton *button, gpointer data);
		static void onSharedButtonToggled_gui(GtkToggleButton *button, gpointer data);
		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
//		static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onDownloadToMatchClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onSearchByTTHClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onCopyMagnetClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onGetFileListClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onRemoveClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCheckOp_gui(GtkToggleButton *button, gpointer data);
		static gboolean on_match_select_entry(GtkEntryCompletion *widget,GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
		static gboolean onResultView_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data);
		static void selection_changed_result_gui(GtkTreeSelection *selection, GtkWidget *tree_view);

		// GUI functions
		void parseSearchResult_gui(dcpp::SearchResultPtr result, dcpp::StringMap &resultMap);
		void setColorRow(std::string cell);
		void setColorsRows();
		static void makeColor(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);

		// Client functions
		void download_client(std::string target, std::string cid, std::string filename, int64_t size, std::string tth, std::string hubUrl);
		void downloadDir_client(std::string target, std::string cid, std::string filename, std::string hubUrl);
		void addSource_client(std::string source, std::string cid, int64_t size, std::string tth, std::string hubUrl);
		void getFileList_client(std::string cid, std::string dir, bool match, std::string hubUrl);
		void addFavUser_client(std::string cid);
		void grantSlot_client(std::string cid, std::string hubUrl);
		void removeSource_client(std::string cid);

		// Client callbacks
		virtual void on(dcpp::ClientManagerListener::ClientConnected, dcpp::Client *client) noexcept;
	 	virtual void on(dcpp::ClientManagerListener::ClientUpdated, dcpp::Client *client) noexcept;
		virtual void on(dcpp::ClientManagerListener::ClientDisconnected, dcpp::Client *client) noexcept;
		virtual void on(dcpp::SearchManagerListener::SR, const dcpp::SearchResultPtr &result) noexcept;

		TreeView hubView, resultView;
		GtkListStore *hubStore;
		GtkTreeStore *resultStore;
		GtkTreeModel *searchFilterModel;
		GtkTreeModel *sortedFilterModel;
		GtkTreeSelection *selection;
		GdkEventType oldEventType;
		dcpp::TStringList searchlist;
		int droppedResult;
		int searchHits;
		bool isHash;
		bool onlyFree;
		UserCommandMenu *userCommandMenu;
		GroupType previousGrouping;
		std::unordered_map<std::string, std::vector<dcpp::SearchResultPtr> > results;

		GtkEntryCompletion *completion;
		GtkListStore *emodel;
		bool isMenuCreated;
		GtkWidget* menu;	
		public:
			GMenu *createmenu() override;
		private:
			static void onCloseItem(gpointer data);
			static void onAddItem(gpointer data);
};

#else
class Search;
#endif
