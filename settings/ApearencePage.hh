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

#ifndef _APPEARENCE_PAGE_
#define _APPEARENCE_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"
/*----------------------------------------------------------------------------*/
class ApearencePage : public SettingsPage
{
	public:
		virtual void show(GtkWidget *parent , GtkWidget* old);
		virtual const char* get_name_page(){ return page_name;}
		virtual void write();
		virtual GtkWidget* getTopWidget(){return box3;} 
	private:
		static const char* page_name;
		TreeView appearenceView;
		GtkListStore* appStore;
		GtkWidget *box3;
};
#else
class ApearencePage;
#endif
/*-------------------------------------------------------------------------------*/
