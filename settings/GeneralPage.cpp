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

/* General Page */

#include <vector>
#include <string>
#include "GeneralPage.hh"
#include "definitons.hh"
#include "seUtil.hh"
#include "../dcpp/SettingsManager.h"
#include "../linux/GuiUtil.hh"
#include "../linux/settingsmanager.hh"

using namespace std;
using namespace dcpp;
/*-----------------------------------------------*/
const char* GeneralPage::page_name =  "General";
/*-----------------------------------------------*/
GeneralPage::GeneralPage()
{
	connectionSpeeds.push_back("0.005");
	connectionSpeeds.push_back("0.01");
	connectionSpeeds.push_back("0.02");
	connectionSpeeds.push_back("0.05");
	connectionSpeeds.push_back("0.1");
	connectionSpeeds.push_back("0.2");
	connectionSpeeds.push_back("0.5");
	connectionSpeeds.push_back("1");
	connectionSpeeds.push_back("2");
	connectionSpeeds.push_back("5");
	connectionSpeeds.push_back("10");
	connectionSpeeds.push_back("20");
	connectionSpeeds.push_back("50");
	connectionSpeeds.push_back("100");
	connectionSpeeds.push_back("1000");	
}

#define g_g_a(widget,x,y) gtk_grid_attach(GTK_GRID(box_grid),widget,x,y,1,1)

void GeneralPage::show(GtkWidget *parent, GtkWidget* old)
{
	
	//@Main container
	box_grid = gtk_grid_new();	
	entry_nick = gen;
	g_g_a(gtk_label_new(_("Nick: ")),0,0);
	g_g_a(entry_nick,1,0);
	
	entry_desc = gen;
	g_g_a(gtk_label_new("Description (Optional): "),0,1);
	g_g_a(entry_desc,1,1);
	
	entry_email = gen;
	g_g_a(gtk_label_new("e-mail (Optional): "),0,2);
	g_g_a(entry_email,1,2);
	
	conn = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	g_g_a(gtk_label_new("Connection: "),0,3);
	g_g_a(GTK_WIDGET(conn),1,3);
	
	codepage = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	g_g_a(gtk_label_new("Codepage: "),0,4);
	g_g_a(GTK_WIDGET(codepage),1,4);

	gtk_editable_set_text(GTK_EDITABLE(entry_nick), SETTING(NICK).c_str());
	gtk_editable_set_text(GTK_EDITABLE(entry_email), SETTING(EMAIL).c_str());
	gtk_editable_set_text(GTK_EDITABLE(entry_desc), SETTING(DESCRIPTION).c_str());

	for (vector<string>::const_iterator i = connectionSpeeds.begin(); i != connectionSpeeds.end(); ++i)
	{
		gtk_combo_box_text_append_text(conn, (*i).c_str());
			if (SETTING(UPLOAD_SPEED) == *i)
				gtk_combo_box_set_active(GTK_COMBO_BOX(conn), i - connectionSpeeds.begin());
	}
		
	// Fill charset drop-down list
	auto& charsets = WulforUtil::getCharsets();
	for (auto it = charsets.begin(); it != charsets.end(); ++it) {
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(codepage), (*it).c_str());
			if(WGETS("default-charset") == *it)
				gtk_combo_box_set_active(GTK_COMBO_BOX(codepage), (it - charsets.begin()) );
	}
	/*@Add to parent*/
	SEUtil::reAddItemCo(parent,old,box_grid);
}

void GeneralPage::write()
{
	dcpp::SettingsManager *sm = dcpp::SettingsManager::getInstance();
	
	const gchar* nick = NULL,*desc= NULL,*email = NULL;
	//if(GTK_IS_ENTRY(entry_nick))
		nick = gtk_editable_get_text(GTK_EDITABLE(entry_nick));
	//if(GTK_IS_ENTRY(entry_desc))
		desc = gtk_editable_get_text(GTK_EDITABLE(entry_desc));
	//if(GTK_IS_ENTRY(entry_email))
		email = gtk_editable_get_text(GTK_EDITABLE(entry_email));
	
	if(nick != NULL) {
		sm->set(dcpp::SettingsManager::NICK, nick);
	}
	if(email != NULL) {
		sm->set(dcpp::SettingsManager::EMAIL, email);
	}
	if(desc != NULL) {
		sm->set(dcpp::SettingsManager::DESCRIPTION, desc);
	}
	if(GTK_IS_COMBO_BOX(conn))
		sm->set(dcpp::SettingsManager::UPLOAD_SPEED, connectionSpeeds[gtk_combo_box_get_active(GTK_COMBO_BOX(conn))]);

	if(GTK_IS_COMBO_BOX_TEXT(codepage)) {
		gchar *encoding = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(codepage));
		if(encoding) {
			WSET("default-charset", string(encoding));
			g_free(encoding);
		}
	}	
}

