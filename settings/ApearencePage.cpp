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

#include "ApearencePage.hh"
#include "seUtil.hh"
#include "definitons.hh"
#include <dcpp/SettingsManager.h>
#include <dcpp/format.h>

using namespace dcpp;

const char* ApearencePage::page_name = "Apearence";

void  ApearencePage::show(GtkWidget *parent, GtkWidget* old)
{
	box3 = gtk_scrolled_window_new(NULL,NULL);
	appearenceView = TreeView();//Fix Crash
	SEUtil::createOptionsView_gui(appearenceView,appStore);
	gtk_container_add(GTK_CONTAINER(box3),GTK_WIDGET(appearenceView.get()));
	/*@Add to parent*/
	SEUtil::reAddItemCo(parent,old,box3);

	SEUtil::addOption_gui(appStore, _("Filter kick and NMDC debug messages"), SettingsManager::FILTER_MESSAGES);
	SEUtil::addOption_gui(appStore, _("Show status icon"), SettingsManager::ALWAYS_TRAY);
	SEUtil::addOption_gui(appStore, _("Show timestamps in chat by default"), SettingsManager::TIME_STAMPS);
	SEUtil::addOption_gui(appStore, _("View status messages in main chat"), SettingsManager::STATUS_IN_CHAT);
	SEUtil::addOption_gui(appStore, _("Show joins / parts in chat by default"), SettingsManager::SHOW_JOINS);
	SEUtil::addOption_gui(appStore, _("Only show joins / parts for favorite users"), SettingsManager::FAV_SHOW_JOINS);
	SEUtil::addOption_gui(appStore, _("Sort favorite users first"), SettingsManager::SORT_FAVUSERS_FIRST);
	SEUtil::addOption_gui(appStore, _("Use OEM monospaced font for chat windows"), SettingsManager::USE_OEM_MONOFONT);
	SEUtil::addOption_gui(appStore, _("Use magnet split"), "use-magnet-split");
	SEUtil::addOption_gui(appStore, _("Use blinking status icon"), "status-icon-blink-use");
	SEUtil::addOption_gui(appStore, _("Use emoticons"), SettingsManager::USE_EMOTS);
	SEUtil::addOption_gui(appStore, _("Do not close the program, hide in the system tray"), "main-window-no-close");
	SEUtil::addOption_gui(appStore, _("Show Country in chat"), SettingsManager::GET_USER_COUNTRY);
	SEUtil::addOption_gui(appStore, _("Show IP in chat"), SettingsManager::USE_IP);
	SEUtil::addOption_gui(appStore, _("Show Free Slots in Desc"), SettingsManager::SHOW_FREE_SLOTS_DESC);
//		addOption_gui(appStore, _("Use Highlighting"), "use-highlighting");//TODO Tab
	SEUtil::addOption_gui(appStore, _("Show Close Icon in Tab"), "use-close-button");
	SEUtil::addOption_gui(appStore, _("Show send /commnads in status message"), "show-commands");
	SEUtil::addOption_gui(appStore, _("Show Server Commands as Status Messages in Chat"),SettingsManager::SERVER_COMMANDS);

	SEUtil::addOption_gui(appStore, _("Show Flags in main chat"),  SettingsManager::USE_COUNTRY_FLAG);
	SEUtil::addOption_gui(appStore, _("Use DNS in Transfers"), "use-dns");
	SEUtil::addOption_gui(appStore, _("Log Ignored Messages as STATUS mess"), SettingsManager::LOG_CHAT_B);
	SEUtil::addOption_gui(appStore, _("Do not close Tab on middle button (wheel)"), "book-three-button-disable");
	SEUtil::addOption_gui(appStore, _("Use ctrl for history in chat Books"), "key-hub-with-ctrl");
	SEUtil::addOption_gui(appStore, _("Set Hub to Bold all time when change whats in it"), "bold-all-tab");
	
	SEUtil::addOption_gui(appStore, _("Filter download by antivirus db"),SettingsManager::USE_AV_FILTER);

}

void ApearencePage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();
	SEUtil::saveOptionsView_gui(appearenceView,appStore,sm);

}
