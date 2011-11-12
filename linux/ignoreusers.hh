/*
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

#ifndef IGNORE_USERS_HH
#define IGNORE_USERS_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/FavoriteUser.h>
#include <dcpp/FavoriteManagerListener.h>

#include "bookentry.hh"
#include "treeview.hh"

class IgnoreUsers:
	public BookEntry,
	public dcpp::FavoriteManagerListener
{
	public:
		IgnoreUsers();
		virtual ~IgnoreUsers();
		virtual void show();

	private:
		typedef std::map<std::string, std::string> ParamMap;
		typedef std::unordered_map<std::string, GtkTreeIter> UserIters;

		// GUI functions

		bool findUser_gui(const std::string &cid, GtkTreeIter *iter);
		void updateFavoriteUser_gui(ParamMap params);
		void removeFavoriteUser_gui(const std::string cid);
		void setStatus_gui(const std::string text);

		// GUI callbacks
		static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchQueueItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onConnectItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveFromQueueItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

		// Client functions
		void getFileList_client(const std::string cid, const std::string hubUrl, bool match);
		void removeUserFromQueue_client(const std::string cid);
		void removeFavoriteUser_client(const std::string cid);

		// Favorite callbacks
		virtual void on(dcpp::FavoriteManagerListener::IgnoreUserAdded, const dcpp::FavoriteUser &user) throw();
		virtual void on(dcpp::FavoriteManagerListener::IgnoreUserRemoved, const dcpp::FavoriteUser &user) throw();
		virtual void on(dcpp::FavoriteManagerListener::IgnoreStatusChanges, const dcpp::FavoriteUser &user) throw();

		UserIters userIters;
		GdkEventType previous;
		TreeView favoriteUserView;
		GtkListStore *favoriteUserStore;
		GtkTreeSelection *favoriteUserSelection;
};

#else
class IgnoreUsers;
#endif
