/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _OTHER_SHARING_PAGE_
#define _OTHER_SHARING_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"

/*-------------------------------------------------------------------------*/
class OSharingPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page()
		{ return page_name;};
		void write();
		GtkWidget* getTopWidget(){return grid;}
	private:
		static const char* page_name;
		GtkWidget *check_hiden,*check_follow,
		*spin_slots,
		*spin_slots_extra,
		*entry_skiplist_ext,
		*entry_skiplist_path,
		*entry_skiplist_reg,
		*spin_size_low,*spin_size_high,
		*grid;
		static gboolean onShareHiddenPressed_gui(GtkToggleButton *togglebutton, gpointer data);
		void shareHidden_client(bool show);


};
#else
class SharingPage;
#endif
/*-------------------------------------------------------------------------------*/
