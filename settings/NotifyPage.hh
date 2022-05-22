// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
// 

#ifndef _NOTI_SETTINGS_PAGE_
#define _NOTI_SETTINGS_PAGE_
#ifdef HAVE_NOTIFY
#include <gtk/gtk.h>
#include "../linux/treeview.hh"
#include "../linux/settingsmanager.hh"
#include <string>
#include "SettingsPage.hh"
/*----------------------------Meta---------------------------------------------*/
class NotifyPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page(){ return name_page;}
		void write();
		virtual GtkWidget* getTopWidget(){return box;} ;
	private:
		static const char* name_page;
		TreeView notifyView;
		GtkListStore *notifyStore;
		GtkWidget *bTest,*bIcon,*bOK,*bNone,*bDef,
		*eTitle, *sPMLenght,*tAppActive,
		*comboSize, *box;
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm,
		const std::string &name, const std::string &key1, const std::string &key2, const std::string &key3, const int key4);
		static void onNotifyTestButton_gui(GtkWidget *widget, gpointer data);
		static void onNotifyIconFileBrowseClicked_gui(GtkWidget *widget, gpointer data);
//		static void onNotifyKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
//		static void onNotifyButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onNotifyOKClicked_gui(GtkWidget *widget, gpointer data);
		static void onNotifyIconNoneButton_gui(GtkWidget *widget, gpointer data);
		static void onNotifyDefaultButton_gui(GtkWidget *widget, gpointer data);
};
#endif
#else
#ifdef HAVE_NOTIFY
class NotifyPage;
#endif
#endif
/*-------------------------------------------------------------------------------*/
