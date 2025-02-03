/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _OUT_CONN_SETTINGS_
#define _OUT_CONN_SETTINGS_

#include <gtk/gtk.h>
#include "SettingsPage.hh"

class OutConnectionPage: public SettingsPage
{
		public:
			OutConnectionPage(){ };
			~OutConnectionPage(){ };
			void show(GtkWidget *parent, GtkWidget* old);
			const char* get_name_page()
			{ return name_page; }
			void write();
			virtual GtkWidget* getTopWidget(){ return frame;}
		private:
			static const char* name_page;
			GtkWidget* radio_direct,*radio_sock,
			*label_ip,*label_port,
			*label_username,*label_password,
			*entry_ip,*entry_sport,
			*entry_username,*entry_password,
			*check_hostname,
			*frame;
			//static void onOutDirect_gui(GtkToggleButton *button, gpointer data);
			//static void onSocks5_gui(GtkToggleButton *button, gpointer data);

};
#else
class OutConnectionPage;
#endif
