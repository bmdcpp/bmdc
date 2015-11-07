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

#ifndef _HIGLIT_PAGE_
#define _HIGLIT_PAGE_
#include <gtk/gtk.h>
#include <linux/treeview.hh>
#include "SettingsPage.hh"
/*----------------------------HigPage---------------------------------------------*/
class HigPage: public SettingsPage
{
	public:
	HigPage() { }
	~HigPage() { }
	void show(GtkWidget *parent, GtkWidget* old);
	GtkWidget* getTopWidget() {return box;}
	const char* get_name_page()
	{ return page_name;};
	void write();
	private:
		static const char* page_name;
		GtkWidget* box;
		TreeView higTree;
		GtkListStore* higStore;
		GtkTreeSelection *higSelection;
		static void onAddHighlighting_gui(GtkWidget* widget, gpointer data);
		static void onEditHighlighting_gui(GtkWidget* widget, gpointer data);
		static void onRemoveHighlighting_gui(GtkWidget* widget, gpointer data);
};
#else
class HigPage;
#endif
/*-------------------------------------------------------------------------------*/
