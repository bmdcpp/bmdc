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

#ifndef _ADVACED_CONNECTION_PAGE_
#define _ADVACED_CONNECTION_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"

class AdvancedConnectionPage: public SettingsPage
{
		public:
			const char* get_name_page()
			{ return name_page; }
			void show(GtkWidget* parent , GtkWidget* old);
			void write();
			GtkWidget* getTopWidget() { return box_grid;}
		private:
			static const char* name_page;
			GtkWidget* spin_recon, *entry_bind,*entry_bind6,
			*http_proxy,*box_grid;
};
#else
class AdvancedConnectionPage;		
#endif
