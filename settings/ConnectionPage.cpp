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

#include "ConnectionPage.hh"
#include "definitons.hh"
#include "seUtil.hh"

#include <dcpp/SettingsManager.h>
using namespace std;
using namespace dcpp;	
/*----------------------------------------------------------------------*/
const char* ConnectionPage::name_page = "Connection";
/*----------------------------------------------------------------------*/
void ConnectionPage::show(GtkWidget *parent, GtkWidget* old)
{
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	GtkWidget* frame = gtk_frame_new (_("Settings Incoming Connection"));

	GtkWidget* table = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER(frame),table);
	//@:todo: small sized
	entry_tcp = gen;
	entry_tls = gen;
	entry_udp = gen;
	
	entry_ip = gen;
	entry_ip6 = gen;
	
	radio_direct = gtk_radio_button_new_with_label(NULL,
                                 _("Direct Connection"));
    radio_upnp = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_direct), _("Use uPnP"));
	radio_manual = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_direct), _("Manual Forward"));
    radio_pasive = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_direct), _("Pasive"));

	gtk_grid_attach(GTK_GRID(table),radio_direct,0,0,1,1);
	gtk_grid_attach(GTK_GRID(table),radio_upnp,0,1,1,1);
	gtk_grid_attach(GTK_GRID(table),radio_manual,0,2,1,1);
	gtk_grid_attach(GTK_GRID(table),radio_pasive,0,3,1,1);
		
	label_udp = gtk_label_new("UDP");
	label_tcp = gtk_label_new("TCP");
	label_tls = gtk_label_new("TLS");
	label_ip = gtk_label_new(_("IPv4 Address"));
	label_ip6 = gtk_label_new(_("IPv6 Address"));

	gtk_grid_attach(GTK_GRID(table),gtk_label_new(_("Ports")),4,0,1,1);
				
	gtk_grid_attach(GTK_GRID(table),label_tcp,4,1,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_tcp,5,1,1,1);

	gtk_grid_attach(GTK_GRID(table),label_tls,4,2,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_tls,5,2,1,1);

	gtk_grid_attach(GTK_GRID(table),label_udp,4,3,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_udp,5,3,1,1);
		
	gtk_grid_attach(GTK_GRID(table),label_ip,0,4,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_ip,1,4,1,1);

	gtk_grid_attach(GTK_GRID(table),label_ip6,0,5,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_ip6,1,5,1,1);

	overide_button = gtk_switch_new();
	gtk_grid_attach(GTK_GRID(table),overide_button,0,6,1,1);
	gtk_grid_attach(GTK_GRID(table),gtk_label_new(_("Don't Override by Hub/UPnP")),1,6,1,1);
				
	gtk_entry_set_text(GTK_ENTRY(entry_ip), SETTING(EXTERNAL_IP).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_ip6), SETTING(EXTERNAL_IP6).c_str());

	gtk_entry_set_text(GTK_ENTRY(entry_tcp), Util::toString(SETTING(TCP_PORT)).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_udp), Util::toString(SETTING(UDP_PORT)).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_tls), Util::toString(SETTING(TLS_PORT)).c_str());
	gtk_switch_set_active(GTK_SWITCH(overide_button), SETTING(NO_IP_OVERRIDE));

	switch (SETTING(INCOMING_CONNECTIONS))
	{
		case SettingsManager::INCOMING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_direct), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_NAT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_manual), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_UPNP:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_upnp), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_pasive), TRUE);
			onInPassive_gui(NULL,(gpointer)this);
			break;
	}
	gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,0);
	
	frame = gtk_frame_new(_("Settings Outgoing Connection"));

	table = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER(frame),table);
	radio_direct_out = gtk_radio_button_new_with_label(NULL,_("Direct"));
	radio_sock = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_direct_out),_("Sock5"));	
	gtk_grid_attach(GTK_GRID(table),radio_direct_out,0,0,1,1);
	gtk_grid_attach(GTK_GRID(table),radio_sock,0,1,1,1);
	label_ip_out = gtk_label_new(_("Address: "));
	label_port_out = gtk_label_new(_("Port: "));
	label_username = gtk_label_new(_("Username: "));
	label_password = gtk_label_new(_("Password: "));
	entry_ip_sock = gen;
	entry_sport = gen;
	entry_username = gen;
	entry_password = gen;
	check_hostname = gtk_switch_new();
	
	gtk_grid_attach(GTK_GRID(table),label_ip_out,0,3,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_ip_sock,1,3,1,1);
	gtk_grid_attach(GTK_GRID(table),label_username,0,4,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_username,1,4,1,1);

	gtk_grid_attach(GTK_GRID(table),label_port_out,0,5,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_sport,1,5,1,1);

	gtk_grid_attach(GTK_GRID(table),label_password,0,6,1,1);
	gtk_grid_attach(GTK_GRID(table),entry_password,1,6,1,1);

	gtk_grid_attach(GTK_GRID(table),check_hostname,0,7,1,1);
	gtk_grid_attach(GTK_GRID(table),gtk_label_new(_("Use also for resolving hostname")),1,7,1,1);

	gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,0);

	/*@Add to parent*/
	SEUtil::reAddItemCo(parent,old,box);
	/*end*/
	g_signal_connect(GTK_TOGGLE_BUTTON(radio_direct), "toggled", G_CALLBACK(onInDirect_gui),  (gpointer)this);
	g_signal_connect(GTK_TOGGLE_BUTTON(radio_upnp),   "toggled", G_CALLBACK(onInDirect_gui), (gpointer)this);
	g_signal_connect(GTK_TOGGLE_BUTTON(radio_manual), "toggled", G_CALLBACK(onInDirect_gui),  (gpointer)this);
	g_signal_connect(GTK_TOGGLE_BUTTON(radio_pasive), "toggled", G_CALLBACK(onInPassive_gui), (gpointer)this);
	
	// Outgoing
	g_signal_connect(radio_direct_out, "toggled", G_CALLBACK(onOutDirect_gui), (gpointer)this);
	g_signal_connect(radio_sock, "toggled", G_CALLBACK(onSocks5_gui), (gpointer)this);
	gtk_entry_set_text(GTK_ENTRY(entry_ip_sock), SETTING(SOCKS_SERVER).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_username), SETTING(SOCKS_USER).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_sport), Util::toString(SETTING(SOCKS_PORT)).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_password), SETTING(SOCKS_PASSWORD).c_str());
	
	gtk_switch_set_active(GTK_SWITCH(check_hostname), SETTING(SOCKS_RESOLVE));

	switch (SETTING(OUTGOING_CONNECTIONS))
	{
		case SettingsManager::OUTGOING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_direct_out), TRUE);
			onOutDirect_gui(NULL, (gpointer)this);
			break;
		case SettingsManager::OUTGOING_SOCKS5:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_sock), TRUE);
			break;
	}
	
}
void ConnectionPage::setWidgetStatusIncoming(gboolean is)
{
	gtk_widget_set_sensitive(entry_ip6, is);
	gtk_widget_set_sensitive(entry_ip, is);
	gtk_widget_set_sensitive(label_ip, is);
	gtk_widget_set_sensitive(label_ip6, is);
	gtk_widget_set_sensitive(entry_tcp, is);
	gtk_widget_set_sensitive(label_tcp, is);
	gtk_widget_set_sensitive(entry_udp, is);
	gtk_widget_set_sensitive(label_udp, is);
	gtk_widget_set_sensitive(entry_tls, is);
	gtk_widget_set_sensitive(label_tls, is);
	gtk_widget_set_sensitive(overide_button, is);		
}
//@actions functions
void ConnectionPage::onInDirect_gui(GtkToggleButton *button, gpointer data)
{
	ConnectionPage *cp = (ConnectionPage*)data;
	cp->setWidgetStatusIncoming(TRUE);
}
void ConnectionPage::onInPassive_gui(GtkToggleButton *button, gpointer data)
{
	ConnectionPage *cp = (ConnectionPage*)data;
	cp->setWidgetStatusIncoming(FALSE);
}

void ConnectionPage::setOutWidgetStatus(gboolean is)
{
	gtk_widget_set_sensitive(entry_ip_sock, is);
	gtk_widget_set_sensitive(label_ip, is);
	gtk_widget_set_sensitive(entry_password, is);
	gtk_widget_set_sensitive(entry_username, is);
	gtk_widget_set_sensitive(entry_sport, is);
	gtk_widget_set_sensitive(label_port_out, is);
	gtk_widget_set_sensitive(label_password, is);
	gtk_widget_set_sensitive(label_username, is);
	gtk_widget_set_sensitive(check_hostname, is);	
}

void ConnectionPage::onOutDirect_gui(GtkToggleButton *button, gpointer data)
{
	ConnectionPage *cp = (ConnectionPage *)data;
	cp->setOutWidgetStatus(FALSE);
}

void ConnectionPage::onSocks5_gui(GtkToggleButton *button, gpointer data)
{
	ConnectionPage *cp = (ConnectionPage *)data;
	cp->setOutWidgetStatus(TRUE);
}

void ConnectionPage::write()
{
	dcpp::SettingsManager *sm = dcpp::SettingsManager::getInstance();
	
// Incoming connection
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_direct)))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_upnp)))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_UPNP);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_manual)))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_NAT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_pasive)))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
		
		const gchar* ipv4 = gtk_entry_get_text(GTK_ENTRY(entry_ip));
		
		if(ipv4 != NULL)
			sm->set(SettingsManager::EXTERNAL_IP, ipv4);
		const gchar* ipv6 = gtk_entry_get_text(GTK_ENTRY(entry_ip6));
		if(ipv6 != NULL)
			sm->set(SettingsManager::EXTERNAL_IP6, ipv6 );

		sm->set(SettingsManager::NO_IP_OVERRIDE,  gtk_switch_get_active(GTK_SWITCH((overide_button))));

		const gchar* s_port = gtk_entry_get_text(GTK_ENTRY(entry_tcp));
		int port = 0;
		if(s_port != NULL) {
			port = Util::toInt(s_port);
		}
		if (port >= 0 && port <= 65535)
			sm->set(SettingsManager::TCP_PORT, port);

		const gchar* s_uport = gtk_entry_get_text(GTK_ENTRY(entry_udp));
		int uport = 0;
		if(s_uport != NULL) {
			uport = Util::toInt(s_uport);
		}
		if (uport >= 0 && uport <= 65535)
			sm->set(SettingsManager::UDP_PORT, uport);
		int sport = 0;
		const gchar* s_sport = gtk_entry_get_text(GTK_ENTRY(entry_tls));
		if(s_sport != NULL) {
			sport = Util::toInt(s_sport);
		}	
		if ((port != sport) || ( sport >= 0 && sport <= 65535))
			sm->set(SettingsManager::TLS_PORT, sport);
		else ;/*thow dialog?*/
		
		// Outgoing connection
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_direct_out)))
			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_DIRECT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sock)))
			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_SOCKS5);

		const gchar* s_server = gtk_entry_get_text(GTK_ENTRY(entry_ip_sock)); 
		if(s_server !=  NULL)
			sm->set(SettingsManager::SOCKS_SERVER, s_server );

		const gchar* s_user = gtk_entry_get_text(GTK_ENTRY(entry_username));
		if(s_user != NULL)
		sm->set(SettingsManager::SOCKS_USER, s_user);

		const gchar* s_pass = gtk_entry_get_text(GTK_ENTRY(entry_password));
		if(s_pass != NULL)
			sm->set(SettingsManager::SOCKS_PASSWORD, s_pass);

		sm->set(SettingsManager::SOCKS_RESOLVE, gtk_switch_get_active(GTK_SWITCH(check_hostname)));

		s_sport = gtk_entry_get_text(GTK_ENTRY(entry_sport));
		if(s_sport != NULL) {
			int port = Util::toInt(s_sport);
			if (port > 0 && port <= 65535)
				sm->set(SettingsManager::SOCKS_PORT, port);
		}	
}	


