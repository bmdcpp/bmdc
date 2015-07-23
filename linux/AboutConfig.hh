//
//      Copyright 2011 - 2015 Mank <freedcpp at seznam dot cz>
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

#ifndef _BMDC_ABOUT_CONFIG_
#define _BMDC_ABOUT_CONFIG_

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/SettingsManager.h>
#include "WulforUtil.hh"
#include "bookentry.hh"
#include "treeview.hh"

class AboutConfig: 
	public BookEntry,
	private dcpp::SettingsManagerListener	
{
	private:
		using dcpp::SettingsManagerListener::on;
	public:
		AboutConfig();
		virtual ~AboutConfig();
		virtual void show();
	private:
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onPropertiesClicked_gui(GtkWidget *widget, gpointer data);
		static void onSetDefault(GtkWidget *widget, gpointer data);
		static void onInfoResponse(GtkWidget *info_bar, gint response_id,  gpointer data );
	
		bool getDialog(std::string name , std::string& value , gpointer data);
		void addItem_gui(std::string rowname, std::string isdefault, std::string types, std::string value, bool isWulf = false);
		void updateItem_gui(std::string rowname, std::string value);
		bool findAboutItem_gui(const std::string& about, GtkTreeIter *iter);
		void setStatus(std::string msg);
		
		TreeView aboutView;
		GtkListStore *aboutStore;
		GtkTreeSelection *aboutSelection;
		GdkEventType previous;
		UnMapIter aboutIters;
};
#else
class AboutConfig;
#endif
