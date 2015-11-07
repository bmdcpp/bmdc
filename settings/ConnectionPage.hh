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

#ifndef _CONNECTION_PAGE_
#define _CONNECTION_PAGE_
#include "SettingsPage.hh"
#include <vector>
#include <string>
/*---------------------------------------------------------------------------*/
class ConnectionPage: public SettingsPage
{
	public:
		const char* get_name_page() { return name_page;}
		void show(GtkWidget *parent, GtkWidget* old);
		void write();
		GtkWidget* getTopWidget() { return box;	}
	private:
//@name of page		
		static const char* name_page;
//Actions & labels widgets
		GtkWidget *entry_tcp,*entry_tls,*entry_udp,*entry_ip,*entry_ip6,
		*label_tcp,*label_tls,*label_udp,*label_ip,*label_ip6,
		*radio_direct,*radio_upnp,*radio_manual, *radio_pasive,
		*overide_button,
		*box,//@top
		*radio_direct_out,*radio_sock,
		*label_ip_out,*label_port_out,
		*label_username,*label_password,
		*entry_ip_sock,*entry_sport,
		*entry_username,*entry_password,
		*check_hostname;
		static void onInDirect_gui(GtkToggleButton *button, gpointer data);
		static void onInPassive_gui(GtkToggleButton *button, gpointer data);
		static void onOutDirect_gui(GtkToggleButton *button, gpointer data);
		static void onSocks5_gui(GtkToggleButton *button, gpointer data);
		void setWidgetStatusIncoming(gboolean is);
		void setOutWidgetStatus(gboolean is);
	
};
#else
class ConnectionPage;
#endif
