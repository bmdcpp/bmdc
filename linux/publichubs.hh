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

#ifndef WULFOR_PUBLIC_HUBS_HH
#define WULFOR_PUBLIC_HUBS_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/StringSearch.h"

#include "bookentry.hh"
#include "treeview.hh"

class PublicHubs:
	public BookEntry,
	public dcpp::FavoriteManagerListener
{
	public:
		PublicHubs();
		virtual ~PublicHubs();
		virtual void show();
	private:
		using dcpp::FavoriteManagerListener::on;
		// GUI functions
		void buildHubList_gui();
		void updateList_gui();
		void setStatus_gui(std::string statusBar, std::string text);

		// GUI callbacks
/*		static gboolean onFilterHubs_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);*/
		static void onConnect_gui_click(GtkWidget* wid, GVariant* , gpointer data)
		{ onConnect_gui(wid , data);}
		static void onConnect_gui(GtkWidget *widget, gpointer data);
		static void onRefresh_gui(GtkWidget *widget, gpointer data);
//		static void onAddFav_gui(GtkMenuItem *item, gpointer data);
		static void onConfigure_gui(GtkWidget *widget, gpointer data);
		static void onAdd_gui(GtkWidget *widget, gpointer data);
		static void onMoveUp_gui(GtkWidget *widget, gpointer data);
		static void onMoveDown_gui(GtkWidget *widget, gpointer data);
		static void onRemove_gui(GtkWidget *widget, gpointer data);
		static void onCellEdited_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data);

		static void on_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget);

		// Client functions
		void downloadList_client();
		void refresh_client(int pos);
		void addFav_client(dcpp::FavoriteHubEntry& entry);

		// Client callbacks
		virtual void on(dcpp::FavoriteManagerListener::DownloadStarting, const std::string &file) throw();
		virtual void on(dcpp::FavoriteManagerListener::DownloadFailed, const std::string &file) throw();
		virtual void on(dcpp::FavoriteManagerListener::DownloadFinished, const std::string &file, bool fromCoral) throw();

		dcpp::HubEntryList hubs;
		dcpp::StringSearch filter;
		TreeView listsView, hubView;
		GtkTreeSelection *hubSelection, *listsSelection;
		GtkListStore *hubStore, *listsStore;
		guint oldButton, oldType;
		GtkWidget* mmenu;

		static const GActionEntry pub_entries[];
};

#else
class PublicHubs;
#endif
