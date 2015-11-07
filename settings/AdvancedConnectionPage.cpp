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

#include <dcpp/SettingsManager.h>
#include "definitons.hh"
#include "seUtil.hh"
#include "AdvancedConnectionPage.hh"

using namespace std;
using namespace dcpp;

const char* AdvancedConnectionPage::name_page = "→ Advanced";

void AdvancedConnectionPage::show(GtkWidget *parent, GtkWidget* old)
{
	box_grid = gtk_grid_new();
	spin_recon = gtk_spin_button_new_with_range(10,1000,1);
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Reconnect Time")),0,0,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),spin_recon,1,0,1,1);
	entry_bind = gen;
	entry_bind6 = gen;
//packing
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Bind IPv4")),0,1,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),entry_bind,1,1,1,1);
				
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Bind IPv6")),0,2,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),entry_bind6,1,2,1,1);
	
	http_proxy = gen;
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("HTTP Proxy: ")),0,3,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),http_proxy,1,3,1,1);

//set values
	gtk_entry_set_text(GTK_ENTRY(http_proxy), SETTING(HTTP_PROXY).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_bind), SETTING(BIND_ADDRESS).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_bind6), SETTING(BIND_ADDRESS6).c_str());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_recon), SETTING(TIME_RECCON));

	/*@Add to parent*/
	SEUtil::reAddItemCo(parent,old,box_grid);
}

void AdvancedConnectionPage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();
	
	sm->set(SettingsManager::TIME_RECCON, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_recon)));
	const gchar* bind =gtk_entry_get_text(GTK_ENTRY(entry_bind));
	if(bind)
		sm->set(SettingsManager::BIND_ADDRESS, bind);

	const gchar* bind6 = gtk_entry_get_text(GTK_ENTRY(entry_bind6));
	if(bind6)
		sm->set(SettingsManager::BIND_ADDRESS6,bind6);
	
	const gchar* s_proxy = gtk_entry_get_text(GTK_ENTRY(http_proxy));
	if(s_proxy)
		sm->set(SettingsManager::HTTP_PROXY, s_proxy);	
}



