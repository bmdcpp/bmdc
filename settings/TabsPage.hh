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

#ifndef _TABS_PAGE_
#define _TABS_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"
/*----------------------------Meta---------------------------------------------*/
class TabsPage : public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget *old);
		const char* get_name_page(){
			return name_page;
		}
		virtual void write(){}
		virtual GtkWidget* getTopWidget(){return box;}
	private:
		static const char* name_page;
		TreeView tabsView;
		void addItem_gui(std::string name, std::string key );
		GtkListStore *tabStore;
		GtkTreeSelection *tabsSelection;
		GtkWidget *button_normal_fore,*button_normal_back,
		*buttonb2, *buttonf2, *toggle, *frame, *box;
		bool getActive(std::string key);
		static void onChangeTabSelections(GtkTreeSelection *selection, gpointer data);
		static void onForeColorChooserTab(GtkWidget *button, gpointer data) ;
		static void onBackColorChooserTab(GtkWidget *button, gpointer data) ;
		static void onForeColorChooserTab_unread(GtkWidget *button, gpointer data) ;
		static void onBackColorChooserTab_unread(GtkWidget *button, gpointer data) ;
		static void onBoldToggle_gui(GtkWidget *toggle, gpointer data);

};
#else
class TabsPage;
#endif
/*-------------------------------------------------------------------------------*/
