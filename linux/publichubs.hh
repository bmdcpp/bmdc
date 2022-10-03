/*
 * Copyright © 2004-2015 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2018 BMDC
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
/*		static gboolean onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data);*/
/*		static gboolean onKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onFilterHubs_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);*/
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

		static void on_widget_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_widget_right_btn_released (GtkGestureClick *gesture,int       n_press,
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

		static const GActionEntry win_entries[];
};

#else
class PublicHubs;
#endif
