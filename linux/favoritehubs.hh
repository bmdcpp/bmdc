/*
 * Copyright © 2004-2014 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2015 Mank freedcpp@seznam.cz
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

#ifndef _BMDC_FAVORITE_HUBS_HH
#define _BMDC_FAVORITE_HUBS_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/ClientManager.h>
#include "WulforUtil.hh"
#include "bookentry.hh"
#include "treeview.hh"

class FavoriteHubs:
	public BookEntry,
	public dcpp::FavoriteManagerListener,
	public dcpp::ClientManagerListener
{
	public:
		FavoriteHubs();
		virtual ~FavoriteHubs();
		virtual void show();

	private:
		// GUI functions
		void addEntry_gui(dcpp::FavoriteHubEntry* entry);
		void editEntry_gui(dcpp::FavoriteHubEntry* entry, GtkTreeIter *iter);
		void removeEntry_gui(std::string address);
		void removeGroupComboBox_gui(const std::string &group);
		void addGroupComboBox_gui(const std::string &group);
		void setFavoriteHubs_gui(bool remove, const std::string &group);
		void popupMenu_gui();
		static bool showErrorDialog_gui(const std::string &description, FavoriteHubs *fh);
		void updateFavHubGroups_gui(bool updated);
		void saveFavHubGroups();
		void initFavHubGroupsDialog_gui();

		// GUI callbacks
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onAddEntry_gui(GtkWidget *widget, gpointer data);
		static void onEditEntry_gui(GtkWidget *widget, gpointer data);
		static void onRemoveEntry_gui(GtkWidget *widget, gpointer data);
		static void onConnect_gui(GtkButton *widget, gpointer data);
		static void onCopyAddress(GtkWidget *item, gpointer data);
		static void onAddGroupClicked_gui(GtkWidget *widget, gpointer data);
		static void onRemoveGroupClicked_gui(GtkWidget *widget, gpointer data);
		static void onUpdateGroupClicked_gui(GtkWidget *widget, gpointer data);
		static void onManageGroupsClicked_gui(GtkWidget *widget, gpointer data);
		static gboolean onGroupsButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onGroupsKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);

		// Client functions
		void initializeList_client();
		void addEntry_client(dcpp::FavoriteHubEntry entry);
		void editEntry_client(dcpp::FavoriteHubEntry* entry);
		void removeEntry_client(std::string address);

		// Client callbacks
		virtual void on(dcpp::FavoriteManagerListener::FavoriteAdded, const dcpp::FavoriteHubEntryPtr entry) throw();
		virtual void on(dcpp::FavoriteManagerListener::FavoriteRemoved, const dcpp::FavoriteHubEntryPtr entry) throw();
		
		// ClientManagerListener
		virtual void on(dcpp::ClientManagerListener::ClientConnected, dcpp::Client*) noexcept;
		virtual void on(dcpp::ClientManagerListener::ClientDisconnected, dcpp::Client*) noexcept;

		TreeView favoriteView, groupsView;
		GtkListStore *favoriteStore, *groupsStore;
		GtkTreeSelection *favoriteSelection, *groupsSelection;
		GdkEventType previous;
		UnMapIter GroupsIter;
		UnMapIter HubsIter;
		void edit_online_status(std::string url,bool online);
		bool checkAddys(std::string url);//@True if not find in FavList

};

#else
class FavoriteHubs;
#endif
