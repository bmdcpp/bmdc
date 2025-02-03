/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _SHARING_PAGE_
#define _SHARING_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"

class SharingPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget *old);
		const char* get_name_page()
		{ return page_name;};
		void write() { }
		GtkWidget* getTopWidget(){return box;}
	private:
		static const char* page_name;
		GtkListStore *shareStore;
		TreeView shareView;
		void updateShares_gui();
		void addShare_gui(std::string path, std::string name);
		GtkWidget*	button_add,*button_rem,*button_edit,
		*labelShareSize,*grid,*box;
		static	void onAddShare_gui(GtkWidget *widget, gpointer data);
//		static gboolean onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onRemoveShare_gui(GtkWidget *widget, gpointer data);

};
#else
class SharingPage;
#endif

