/*
* Copyright © 2004-2018 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BMDC_FINISHED_TRANSFERS
#define BMDC_FINISHED_TRANSFERS

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/FinishedManager.h"
#include "../dcpp/FinishedItem.h"
#include "bookentry.hh"
#include "treeview.hh"

class PreviewMenu;

class FinishedTransfers:
	public BookEntry,
	public dcpp::FinishedManagerListener
{
	private:	
		using dcpp::FinishedManagerListener::on;
	public:
		static FinishedTransfers* createFinishedUploads();
		static FinishedTransfers* createFinishedDownloads();
		virtual ~FinishedTransfers();
		virtual void show();

	private:
		FinishedTransfers(const EntryType type, const std::string title, bool isUpload);

		// GUI functions
		void addFile_gui(dcpp::StringMap params, bool update);
		void addUser_gui(dcpp::StringMap params, bool update);
		void removeFile_gui(std::string target);
		void removeUser_gui(std::string cid);
		void updateStatus_gui();
		bool findFile_gui(GtkTreeIter* iter, const std::string& item);
		bool findUser_gui(GtkTreeIter* iter, const std::string& cid);

		// GUI callbacks
	//	static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onOpen_gui(GtkWidget *item,GVariant* var , gpointer data);
	//	static void onOpenFolder_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItems_gui(GtkWidget *item,GVariant* var, gpointer data);
	//	static void onRemoveAll_gui(GtkMenuItem *item, gpointer data);
	//	static void onPageSwitched_gui(GtkNotebook *notebook, GtkWidget *page, guint num, gpointer data);*/
	//	static void onShowOnlyFullFilesToggled_gui(GtkWidget *widget, gpointer data);
	static void on_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
							   double             x,
							   double             y,
							   gpointer         *data);

	static void on_right_btn_released (GtkGestureClick *gesture,int       n_press,
								double           x,
								double           y,
								gpointer       *data);
		// Client functions
		void initializeList_client();
		void getFinishedParams_client(const dcpp::FinishedFileItemPtr &item, const std::string &file,  dcpp::StringMap &params);
		void getFinishedParams_client(const dcpp::FinishedUserItemPtr &item, const dcpp::HintedUser &user,
			dcpp::StringMap &params);
		void removeFile_client(std::string target);
		void removeUser_client(std::string cid);
		void removeAll_client();

		// Client callbacks
		virtual void on(dcpp::FinishedManagerListener::AddedFile, bool upload, const std::string &file, const dcpp::FinishedFileItemPtr &item) noexcept;
		virtual void on(dcpp::FinishedManagerListener::AddedUser, bool upload, const dcpp::HintedUser &user, const dcpp::FinishedUserItemPtr &item) noexcept;
		virtual void on(dcpp::FinishedManagerListener::UpdatedFile, bool upload, const std::string &file, const dcpp::FinishedFileItemPtr &item) noexcept;
		virtual void on(dcpp::FinishedManagerListener::RemovedFile, bool upload, const std::string &file) noexcept;
		virtual void on(dcpp::FinishedManagerListener::UpdatedUser, bool upload, const dcpp::HintedUser &user) noexcept;
		virtual void on(dcpp::FinishedManagerListener::RemovedUser, bool upload, const dcpp::HintedUser &user) noexcept;

		GtkListStore *fileStore, *userStore;
		TreeView userView;
		TreeView fileView;
		GtkTreeSelection *fileSelection,*userSelection;
		bool isUpload;
		int totalFiles;
		int totalUsers;
		int64_t totalBytes, totalTime;
		PreviewMenu *appsPreviewMenu;
		static const GActionEntry finished_entries[];
};

#else
class FinishedTransfers;
#endif
