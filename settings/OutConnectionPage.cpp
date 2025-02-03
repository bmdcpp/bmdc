/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include <dcpp/SettingsManager.h>
#include "OutConnectionPage.hh"
#include "definitons.hh"

using namespace std;
using namespace dcpp;

const char* OutConnectionPage::name_page = "→ Outgoing";

void OutConnectionPage::show(GtkWidget *parent, GtkWidget* old) 
{
		frame = gtk_frame_new("Settings Outgoing Connection");

		GtkWidget* table = gtk_grid_new();
		gtk_frame_set_child  (GTK_FRAME(frame),table);
		//radio_direct = gtk_radio_button_new_with_label(NULL,"Direct");
		//radio_sock = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_direct),"Sock5");	
		gtk_grid_attach(GTK_GRID(table),radio_direct,0,0,1,1);
		gtk_grid_attach(GTK_GRID(table),radio_sock,0,1,1,1);
		label_ip = gtk_label_new("Address: ");
		label_port = gtk_label_new("Port: ");
		label_username = gtk_label_new("Username: ");
		label_password = gtk_label_new("Password: ");
		entry_ip = gtk_entry_new();
		entry_sport = gtk_entry_new();
		entry_username = gtk_entry_new();
		entry_password = gtk_entry_new();
		//check_hostname = gtk_check_button_new_with_label("Use also for resolving hostname");
				
		gtk_grid_attach(GTK_GRID(table),label_ip,0,3,1,1);
		gtk_grid_attach(GTK_GRID(table),entry_ip,1,3,1,1);
		gtk_grid_attach(GTK_GRID(table),label_port,2,3,1,1);
		gtk_grid_attach(GTK_GRID(table),entry_sport,3,3,1,1);
				
		gtk_grid_attach(GTK_GRID(table),label_username,0,4,1,1);
		gtk_grid_attach(GTK_GRID(table),entry_username,1,4,1,1);
		gtk_grid_attach(GTK_GRID(table),label_password,2,4,1,1);
		gtk_grid_attach(GTK_GRID(table),entry_password,3,4,1,1);
		
		gtk_grid_attach(GTK_GRID(table),check_hostname,0,5,1,1);
				
		// Outgoing
		//g_signal_connect(radio_direct, "toggled", G_CALLBACK(onOutDirect_gui), (gpointer)this);
		//g_signal_connect(radio_sock, "toggled", G_CALLBACK(onSocks5_gui), (gpointer)this);
		gtk_editable_set_text(GTK_EDITABLE(entry_ip), SETTING(SOCKS_SERVER).c_str());
		gtk_editable_set_text(GTK_EDITABLE(entry_username), SETTING(SOCKS_USER).c_str());
		gtk_editable_set_text(GTK_EDITABLE(entry_sport), Util::toString(SETTING(SOCKS_PORT)).c_str());
		gtk_editable_set_text(GTK_EDITABLE(entry_password), SETTING(SOCKS_PASSWORD).c_str());
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_hostname), SETTING(SOCKS_RESOLVE));

	switch (SETTING(OUTGOING_CONNECTIONS))
	{
		case SettingsManager::OUTGOING_DIRECT:
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_direct), TRUE);
//			onOutDirect_gui(NULL, (gpointer)this);
			break;
		case SettingsManager::OUTGOING_SOCKS5:
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_sock), TRUE);
			break;
	}

}

void OutConnectionPage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();
	// Outgoing connection
//		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_direct)))
//			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_DIRECT);
//		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sock)))
//			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_SOCKS5);

		const gchar* s_server = gtk_editable_get_text(GTK_EDITABLE(entry_ip)); 
		if(s_server !=  NULL)
			sm->set(SettingsManager::SOCKS_SERVER, s_server );

		const gchar* s_user = gtk_editable_get_text(GTK_EDITABLE(entry_username));
		if(s_user != NULL)
		sm->set(SettingsManager::SOCKS_USER, s_user);

		const gchar* s_passy = gtk_editable_get_text(GTK_EDITABLE(entry_password));
		if(s_passy != NULL)
			sm->set(SettingsManager::SOCKS_PASSWORD, s_passy);

		//sm->set(SettingsManager::SOCKS_RESOLVE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_hostname)));

		const gchar* s_sport = gtk_editable_get_text(GTK_EDITABLE(entry_sport));
		if(s_sport != NULL) {
			int port = Util::toInt(s_sport);
			if (port > 0 && port <= 65535)
				sm->set(SettingsManager::SOCKS_PORT, port);
		}	
}

/*
void OutConnectionPage::onOutDirect_gui(GtkToggleButton *button, gpointer data)
{
	OutConnectionPage *s = (OutConnectionPage *)data;

}

void OutConnectionPage::onSocks5_gui(GtkToggleButton *button, gpointer data)
{
	OutConnectionPage *s = (OutConnectionPage *)data;

}
*/