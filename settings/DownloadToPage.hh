/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _DOWNLOAD_TO_PAGE_
#define _DOWNLOAD_TO_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"
/*-----------------------------------------------------------------------------*/
class DownloadToPage: public SettingsPage
{
	public:
		virtual void show(GtkWidget *parent, GtkWidget* old);
		virtual const char* get_name_page()
		{return page_name;}
		virtual void write(){}
		GtkWidget* getTopWidget(){return box;} 
	private:
		TreeView downloadToView;
		GtkListStore* downloadToStore;
		GtkWidget *buttonAdd,*buttonRem,*grid,*box;
		static const char* page_name;
		static void onAddFavorite_gui(GtkWidget *widget, gpointer data);
		static void onRemoveFavorite_gui(GtkWidget *widget, gpointer data);
		//static gboolean onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

};
#else
class DownloadToPage;
#endif
/*-------------------------------------------------------------------------------*/
