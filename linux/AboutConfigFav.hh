//
//      Copyright 2011 - 2025 BMDC
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#ifndef _BMDC_ABOUT_CONFIG_FAV_
#define _BMDC_ABOUT_CONFIG_FAV_

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/SettingsManager.h"
#include "GuiUtil.hh"
#include "bookentry.hh"
#include "treeview.hh"

class AboutConfigFav:
	public BookEntry
{
	static bool isOk[dcpp::SettingsManager::SETTINGS_LAST-1];
	public:
		AboutConfigFav(dcpp::FavoriteHubEntry* entry );
		virtual ~AboutConfigFav();
		virtual void show();
	private:
		void setColorsRows();
		void setColorRow(std::string);
		static void makeColor(GtkTreeViewColumn *column,GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
		//static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onPropertiesClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onSetDefault(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onInfoResponse(GtkWidget *info_bar, gint response_id,  gpointer data );

		static void on_right_btn_pressed (GtkGestureClick *gesture, int                n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_right_btn_released (GtkGestureClick *gesture,int              n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget);

		bool getDialog(const std::string name , std::string& value , gpointer data);
		void addItem_gui(const gchar* rowname, const gchar* isdefault, const gchar* types, const gchar* value,bool bIsSame = true );
		void updateItem_gui(const std::string rowname, const std::string value, GtkTreeIter *iter, const gchar* status = _("Default"));
		void setStatus(const std::string msg);

		TreeView aboutView;
		GtkListStore *aboutStore;
		GtkTreeSelection *aboutSelection;
		GdkEventType previous;
		dcpp::FavoriteHubEntry* p_entry;
		static const GActionEntry win_entries[];
};
#else
class AboutConfigFav;
#endif
