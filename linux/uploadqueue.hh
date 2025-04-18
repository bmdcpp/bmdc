/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_UPLOADQUEUE_H
#define _BMDC_UPLOADQUEUE_H

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/UploadManager.h"
#include <unordered_map>
#include "bookentry.hh"
#include "GuiUtil.hh"
#include "treeview.hh"

class UploadQueue:
		public BookEntry,
		private dcpp::UploadManagerListener
{
	public:
		UploadQueue();
		virtual ~UploadQueue();
		virtual void show();

	private:
		using dcpp::UploadManagerListener::on;
		void getParams(const std::string& file, dcpp::UserPtr user, dcpp::StringMap &params);
		void addFile(dcpp::StringMap &params, GtkTreeIter *iter);
		void AddFile_gui(dcpp::StringMap params);
		void removeUser(const std::string &cid);

		static void onGrantSlotItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onRemoveItem_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onSendPMItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onBrowseItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onFavoriteUserAddItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);

		static void onWidgetPressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget);
		
/*		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
*/
		//client funcs
		void grantSlot_client(const std::string &cid);
		void getFileList_client(const std::string &cid);
		void removeUploadFromQueue(const std::string &cid);
		void addFavoriteUser_client(const std::string &cid);

		void intilaize_client();

		virtual void on(dcpp::UploadManagerListener::WaitingAddFile, const dcpp::HintedUser& hUser, const std::string& file) noexcept;
		virtual void on(dcpp::UploadManagerListener::WaitingRemoveUser, const dcpp::HintedUser& user) noexcept;

		TreeView users;
		GtkListStore *store;
		UnMapIter mapUsers;
		GtkTreeSelection *selection;
		GdkEventType previous;

		static const GActionEntry win_entries[];

};

#endif /* UPLOADQUEUE_H */
