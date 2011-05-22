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

#ifndef WULFOR_RECENT_TAB_HH
#define WULFOR_RECENT_TAB_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/FavoriteManagerListener.h>

#include "bookentry.hh"
#include "treeview.hh"


class RecentTab:
		public BookEntry,
	public dcpp::FavoriteManagerListener
{
	public:
		RecentTab();
		virtual ~RecentTab();
		virtual void show();
		virtual void popmenu();

	private:
		typedef std::map<std::string, std::string> ParamMap;
		typedef std::tr1::unordered_map<std::string, GtkTreeIter> UserIters;

		// GUI functions
		static void onCloseItem(gpointer data);

		bool findUser_gui(const std::string &cid, GtkTreeIter *iter);
		void updateFavoriteUser_gui(ParamMap params);
		void removeFavoriteUser_gui(const std::string cid);
		// GUI callbacks
		static void onConnectItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data);

		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

		void removeFavoriteUser_client(const std::string adr);

		// Favorite callbacks
		virtual void on(dcpp::FavoriteManagerListener::RecentUpdated, const dcpp::RecentHubEntry *entry) throw();
		virtual void on(dcpp::FavoriteManagerListener::RecentRemoved, const dcpp::RecentHubEntry *entry) throw();
		virtual void on(dcpp::FavoriteManagerListener::RecentAdded, const dcpp::RecentHubEntry *entry) throw();

		UserIters userIters;
		GdkEventType previous;
		TreeView recentView;
		GtkListStore *recentStore;
		GtkTreeSelection *recentSelection;
};

#else
class RecentTab;
#endif
