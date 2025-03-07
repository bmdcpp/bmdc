/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_RECENT_HUBS_HH
#define _BMDC_RECENT_HUBS_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/FavoriteManagerListener.h"
#include "GuiUtil.hh"
#include "bookentry.hh"
#include "treeview.hh"

class RecentHubs:
	public BookEntry,
	public dcpp::FavoriteManagerListener
{
	public:
		RecentHubs();
		virtual ~RecentHubs();
		virtual void show();

	private:
		using dcpp::FavoriteManagerListener::on;
		typedef std::map<std::string, std::string> ParamMap;

		bool findRecent_gui(const std::string &cid, GtkTreeIter *iter);
		void updateRecent_gui(ParamMap params);
		void removeRecent_gui(const std::string cid);
		// GUI callbacks
//		static void onConnectItemClicked_gui(GtkMenuItem *item, gpointer data);
//		static void onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDeleteAll_gui(GtkWidget *widget, gpointer data);

//		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
//		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

		void removeRecent_client(const std::string adr);

		// Favorite callbacks
		virtual void on(dcpp::FavoriteManagerListener::RecentUpdated, const dcpp::RecentHubEntry *entry) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::RecentRemoved, const dcpp::RecentHubEntry *entry) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::RecentAdded, const dcpp::RecentHubEntry *entry) noexcept;

		UnMapIter recIters;
		GdkEventType previous;
		TreeView recentView;
		GtkListStore *recentStore;
		GtkTreeSelection *recentSelection;
};

#else
class RecentHubs;
#endif
