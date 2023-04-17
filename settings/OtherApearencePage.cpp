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

#include "OtherApearencePage.hh"
#include "definitons.hh"
#include "../dcpp/SettingsManager.h"
#include "../linux/settingsmanager.hh"
#include "../linux/message.hh"
#include "seUtil.hh"

using namespace std;
using namespace dcpp;

const char* OApearencePage::page_name = "→ Other";

OApearencePage::OApearencePage()
{
	//New opt-add at end
	usersAction.push_back("Browse");
	usersAction.push_back("Nick To Chat");
	usersAction.push_back("PM To Nick");
	usersAction.push_back("Match Quene");
	usersAction.push_back("Grant Slot");
	usersAction.push_back("Add as Favorite");
	usersAction.push_back("Get Partial Filelist");

}

void OApearencePage::show(GtkWidget *parent, GtkWidget* old)
{
	entry_country = gen;
	entry_away = gen;
	entry_timestamp = gen;
	entry_ripe = gen;
	entry_ratio = gen;
	entry_chat_info = gen;
	grid = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Global Away Message")),0,0,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_away,1,0,1,1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Country Format")),0,1,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_country,1,1,1,1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("TimeStamp Format")),0,2,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_timestamp,1,2,1,1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("RIPE Url")),0,3,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_ripe,1,3,1,1);
	
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(_("Ratio Template")),0,4,1,1);
	gtk_grid_attach(GTK_GRID(grid), entry_ratio,1,4,1,1);
	
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(_("Extra Chat Info")),0,5,1,1);
	gtk_grid_attach(GTK_GRID(grid), entry_chat_info,1,5,1,1);
	
	gtk_editable_set_text(GTK_EDITABLE(entry_chat_info), SETTING(CHAT_EXTRA_INFO).c_str());
	
	gtk_editable_set_text(GTK_EDITABLE(entry_ratio), SETTING(RATIO_TEMPLATE).c_str());

	gtk_editable_set_text(GTK_EDITABLE(entry_ripe),SETTING(RIPE_DB).c_str());

	gtk_editable_set_text(GTK_EDITABLE(entry_timestamp),SETTING(TIME_STAMPS_FORMAT).c_str());

	gtk_editable_set_text(GTK_EDITABLE(entry_country),SETTING(COUNTRY_FORMAT).c_str());

	gtk_editable_set_text(GTK_EDITABLE(entry_away),SETTING(DEFAULT_AWAY_MESSAGE).c_str());

	actionUL = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Action on Userlist")),0,6,1,1);
	gtk_grid_attach(GTK_GRID(grid),GTK_WIDGET(actionUL),1,6,1,1);

	for (vector<string>::const_iterator i = usersAction.begin(); i != usersAction.end(); ++i)
	{
		gtk_combo_box_text_append_text(actionUL, (*i).c_str());
			if (getNameAction(WGETI("double-click-action")) == *i)
				gtk_combo_box_set_active(GTK_COMBO_BOX(actionUL), i - usersAction.begin());
	}

	SEUtil::reAddItemCo(parent,old,grid);
}
//New opt-add at end
string OApearencePage::getNameAction(int num)
{
	switch(num) {
		case CActions::BROWSE:
			return	("Browse");
		case CActions::NICK_TO_CHAT:	
			return ("Nick To Chat");
		case CActions::PM_TO_NICK:
			return ("PM To Nick");
		case CActions::MATCH_Q:
			return ("Match Quene");
		case CActions::GRANT_SLOT:
			return ("Grant Slot");
		case CActions::ADD_AS_FAV:
			return	("Add as Favorite");
		case CActions::GET_PARTIAL_FILELIST:
			return ("Get Partial Filelist");
		default:
			return ("Browse");	
	}	
	return ("Browse");	
}

void OApearencePage::write()
{
	SettingsManager *sm  = SettingsManager::getInstance();
	
	const gchar* s_info = gtk_editable_get_text(GTK_EDITABLE(entry_chat_info));
	if(s_info)
		sm->set(SettingsManager::CHAT_EXTRA_INFO, s_info);
	
	const gchar* s_ratio = gtk_editable_get_text(GTK_EDITABLE(entry_ratio));
	if(s_ratio)
		sm->set(SettingsManager::RATIO_TEMPLATE, s_ratio);
	
	const gchar* s_ripedb = gtk_editable_get_text(GTK_EDITABLE(entry_ripe));
	if(s_ripedb)
		sm->set(SettingsManager::RIPE_DB,s_ripedb);
	const gchar* s_away = gtk_editable_get_text(GTK_EDITABLE(entry_away));
	if(s_away)
		sm->set(SettingsManager::DEFAULT_AWAY_MESSAGE ,s_away);
	const gchar* s_timestamp = gtk_editable_get_text(GTK_EDITABLE(entry_timestamp));
	if(s_timestamp)
		sm->set(SettingsManager::TIME_STAMPS_FORMAT, s_timestamp);

	sm->set(SettingsManager::COUNTRY_FORMAT, gtk_editable_get_text(GTK_EDITABLE(entry_country)));

	WSET("double-click-action", gtk_combo_box_get_active(GTK_COMBO_BOX(actionUL)));

}
