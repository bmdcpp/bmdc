/*
* Copyright © 2009-2018 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FAVORITE_USERS_HH
#define FAVORITE_USERS_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/FavoriteManager.h"
#include "GuiUtil.hh"
#include "bookentry.hh"
#include "treeview.hh"

class FavoriteUsers:
	public BookEntry,
	public dcpp::FavoriteManagerListener
{
	public:
		FavoriteUsers();
		virtual ~FavoriteUsers();
		virtual void show();

	private:
		using dcpp::FavoriteManagerListener::on;
		typedef std::map<std::string, std::string> ParamMap;

		// GUI functions
		bool findUser_gui(const std::string &cid, GtkTreeIter *iter);
		bool findNicks_gui(const std::string &nick, GtkTreeIter *iter);
		void updateFavoriteUser_gui(ParamMap params);
		void updateFavoriteNicks_gui(ParamMap params);
		void removeFavoriteUser_gui(const std::string cid);
		void removeFavoriteNicks_gui(const std::string nick);
		void setStatus_gui(const std::string text);

		// GUI callbacks
		static void on_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    gpointer       *data);
		//static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
		//static void onMatchQueueItemClicked_gui(GtkMenuItem *item, gpointer data);
		//static void onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantSlotItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		//static void onConnectItemClicked_gui(GtkMenuItem *item, gpointer data);
		//static void onRemoveFromQueueItemClicked_gui(GtkMenuItem *item, gpointer data);
		//static void onDescriptionItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		//static void onAutoGrantSlotToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		//static void onIgnoreSetUserClicked_gui(GtkWidget *widget, gpointer data);

		// Client functions
		void getFileList_client(const std::string cid, const std::string hubUrl, bool match);
		void getFileListNick_client(const std::string nick, bool match);
		void grantSlot_client(const std::string cid, const std::string hubUrl);
		void removeUserFromQueue_client(const std::string cid);
		void removeFavoriteUser_client(const std::string cid);
		void setAutoGrantSlot_client(const std::string cid, bool grant);
		void setUserDescription_client(const std::string cid, const std::string description);
		void setDesc_client(const std::string nick, const std::string desc);
		void setIgnore(const std::string cid , const bool set);
		void clickAction(gpointer data);

		// Favorite callbacks
		virtual void on(dcpp::FavoriteManagerListener::UserAdded, const dcpp::FavoriteUser &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::UserRemoved, const dcpp::FavoriteUser &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::StatusChanged, const dcpp::FavoriteUser &user) noexcept;
		//Indepent
		virtual void on(dcpp::FavoriteManagerListener::FavoriteIAdded, const std::string &nick, dcpp::FavoriteUser* &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::FavoriteIRemoved, const std::string &nick, dcpp::FavoriteUser* &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::FavoriteIUpdate, const std::string &nick , dcpp::FavoriteUser* &user) noexcept;

		UnMapIter userIters;
		UnMapIter nicksIters;
		GdkEventType previous;
		TreeView favoriteUserView;
		GtkListStore *favoriteUserStore;
		GtkTreeSelection *favoriteUserSelection;
		static const GActionEntry fuser_entries[];
};

#else
class FavoriteUsers;
#endif
