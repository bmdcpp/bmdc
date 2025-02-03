/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _PREVIEW_PAGE_
#define _PREVIEW_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"

/*-----------------------------------------------------------------------------*/
class PreviewPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page()
		{ return page_name;};
		virtual void write();
		GtkWidget* getTopWidget(){return box;}
	private:
		static const char* page_name;
		GtkListStore *previewAppToStore;
		TreeView previewAppView;
		GtkWidget *entry_name,*entry_app,*entry_type,
		*infoLabel,*grid,*box;
		static void onPreviewAdd_gui(GtkWidget *widget, gpointer data);
		static void onPreviewRemove_gui(GtkWidget *widget, gpointer data);
		static void onPreviewApply_gui(GtkWidget *widget, gpointer data);
//		static void onPreviewKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
//		static void onPreviewButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		void showErrorDialog(const char* info)
		{
			gtk_label_set_text(GTK_LABEL(infoLabel),info);
		};
};
#else
class PreviewPage;
#endif
/*-------------------------------------------------------------------------------*/
