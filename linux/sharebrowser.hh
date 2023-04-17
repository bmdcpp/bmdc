/*
 * Copyright © 2004-2017 Jens Oknelid, paskharen@gmail.com
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

#ifndef _BMDC_SHARE_BROWSER_HH
#define _BMDC_SHARE_BROWSER_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/DirectoryListing.h"
#include "../dcpp/Thread.h"
#include "bookentry.hh"
#include "treeview.hh"

class UserCommandMenu;

class ShareBrowser:
	public BookEntry
{
	public:
		ShareBrowser(dcpp::HintedUser user, const std::string &file, const std::string &initialDirectory, int64_t speed, bool full = true);
		virtual ~ShareBrowser();
		virtual void show();
		virtual GMenu* createmenu();
		void loadXML(std::string txt);
	private:
		static void onCloseItem(gpointer data);
		static void onCopyCID(gpointer data);
		// GUI functions
		void buildList_gui();
		void openDir_gui(const std::string &dir);
		bool findDir_gui(const std::string &dir, GtkTreeIter *parent);
		void buildDirs_gui(dcpp::DirectoryListing::Directory *dir, GtkTreeIter *iter);
		void updateFiles_gui(dcpp::DirectoryListing::Directory *dir);
		void updateStatus_gui();
		void setStatus_gui(std::string statusBar, std::string msg);
		void fileViewSelected_gui();
		void downloadSelectedFiles_gui(const std::string &target);
		void downloadSelectedDirs_gui(const std::string &target);
		
		void popupFileMenu_gui();
		void popupDirMenu_gui();
		
		void find_gui();
		void load(std::string xml);

		// GUI callbacks
		/*static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onMatchButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onFindButtonClicked_gui(GtkWidget *widget, gpointer);
		static void onNextButtonClicked_gui(GtkWidget *widget, gpointer);
		
		static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCopyMagnetClicked_gui(GtkMenuItem* item, gpointer data);
		static void onCopyPictureClicked_gui(GtkMenuItem* item, gpointer data);
		static void onClickedPartial(GtkWidget *widget, gpointer data);
		
		static gpointer threadLoad_list(gpointer data);
*/
		// GUI callbacks
		static void on_widget_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data){
			g_debug("CLICK");
			ShareBrowser* dq = (ShareBrowser*)data;
			GtkTreeIter iter;
			dq->fileViewSelected_gui();
			if ( gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter) )
			{	
				auto pzt= dq->dirView.getString(&iter, "Dir");
				dq->openDir_gui(pzt);

			}	
		};

		static void on_widget_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget){
			g_debug("de-click");

		};
		// Client functions
		void downloadFile_client(dcpp::DirectoryListing::File *file, std::string target);
		void downloadDir_client(dcpp::DirectoryListing::Directory *dir, std::string target);
		void matchQueue_client();
		void downloadChangedDir(dcpp::DirectoryListing::Directory* d);

		GdkEventType oldType;
		dcpp::HintedUser user;
		std::string file;
		std::string initialDirectory;
		std::string nick;
		dcpp::DirectoryListing listing;
		std::string search;
		TreeView dirView, fileView;
		GtkListStore *fileStore;
		GtkTreeStore *dirStore;
		GtkTreeSelection *fileSelection, *dirSelection;
		UserCommandMenu *fileUserCommandMenu;
		UserCommandMenu *dirUserCommandMenu;
		UserCommandMenu *TabUserCommandMenu;
		int64_t shareSize;
		int64_t currentSize;
		int64_t speed;
		int64_t currentItems;
		int64_t shareItems;
		int skipHits;
		bool updateFileView;
		bool fullfl;
		
		class ThreadedDirectoryListing : public dcpp::Thread
 	{
		public:
			ThreadedDirectoryListing(ShareBrowser* pWindow,
			const std::string& pFile, const std::string& pTxt, const std::string& aDir = dcpp::Util::emptyString) : mWindow(pWindow),
			mFile(pFile), mTxt(pTxt), mDir(aDir) { }
 	
		protected:
			ShareBrowser* mWindow;
			std::string mFile;
			std::string mTxt;
			std::string mDir;
 	
		private:
			int run();
 	};
	
};

#else
class ShareBrowser;
#endif
