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

/**/
#include "WindowPage.hh"
#include "seUtil.hh"
#include <dcpp/SettingsManager.h>

using namespace dcpp;

const char* WindowPage::page_name = "→ Windows";

void WindowPage::show(GtkWidget *parent, GtkWidget* old)
{
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	GtkWidget *frame = gtk_frame_new("Auto-Open");
	GtkWidget *scroll = gtk_scrolled_window_new();
	// Auto-open
	windowOpenView = TreeView();
	SEUtil::createOptionsView_gui(windowOpenView, windowStore1);

	//gtk_container_add(GTK_CONTAINER(scroll),GTK_WIDGET(windowOpenView.get()));
	//gtk_container_add(GTK_CONTAINER(frame),scroll);
	gtk_box_append(GTK_BOX(box),frame);

	SEUtil::addOption_gui(windowStore1, _("Public Hubs"), "open-public");
	SEUtil::addOption_gui(windowStore1, _("Favorite Hubs"), "open-favorite-hubs");
	SEUtil::addOption_gui(windowStore1, _("Download Queue"), "open-queue");
	SEUtil::addOption_gui(windowStore1, _("Finished Downloads"), "open-finished-downloads");
	SEUtil::addOption_gui(windowStore1, _("Finished Uploads"), "open-finished-uploads");
	SEUtil::addOption_gui(windowStore1, _("Favorite Users"), "open-favorite-users");
	SEUtil::addOption_gui(windowStore1, _("Search Spy"), "open-search-spy");
	SEUtil::addOption_gui(windowStore1, _("Notepad"), "open-notepad");
	SEUtil::addOption_gui(windowStore1, _("System Tab"), "open-system");
	SEUtil::addOption_gui(windowStore1, _("Upload Queue Tab"), "open-upload-queue");
	// Window options
	windowView2 = TreeView();
	GtkWidget *frame2 = gtk_frame_new("Window Settings");
	GtkWidget *scroll2 = gtk_scrolled_window_new();	
	SEUtil::createOptionsView_gui(windowView2, windowStore2);
//	gtk_container_add(GTK_CONTAINER(scroll2),GTK_WIDGET(windowView2.get()));
//	gtk_container_add(GTK_CONTAINER(frame2),scroll2);
	gtk_box_append(GTK_BOX(box),frame2);

	SEUtil::addOption_gui(windowStore2, _("Open file list window in the background"), SettingsManager::POPUNDER_FILELIST);
	SEUtil::addOption_gui(windowStore2, _("Open new private messages from other users in the background"), SettingsManager::POPUNDER_PM);
	SEUtil::addOption_gui(windowStore2, _("Open new window when using /join"), SettingsManager::JOIN_OPEN_NEW_WINDOW);
	SEUtil::addOption_gui(windowStore2, _("Ignore private messages from the hub"), SettingsManager::IGNORE_HUB_PMS);
	SEUtil::addOption_gui(windowStore2, _("Ignore private messages from bots"), SettingsManager::IGNORE_BOT_PMS);
	SEUtil::addOption_gui(windowStore2, _("Popup box to input password for hubs"), SettingsManager::PROMPT_PASSWORD);
	windowView3 = TreeView();
	// Confirmation dialog
	GtkWidget* frame3 = gtk_frame_new("Confrimation Option");
	GtkWidget *scroll3 = gtk_scrolled_window_new();
	SEUtil::createOptionsView_gui(windowView3, windowStore3);
//	gtk_container_add(GTK_CONTAINER(scroll3),GTK_WIDGET(windowView3.get()));
//	gtk_container_add(GTK_CONTAINER(frame3),scroll3);
	gtk_box_append(GTK_BOX(box),frame3);

	SEUtil::addOption_gui(windowStore3, _("Confirm application exit"), SettingsManager::CONFIRM_EXIT);
	SEUtil::addOption_gui(windowStore3, _("Confirm favorite hub removal"), SettingsManager::CONFIRM_HUB_REMOVAL);
	/// @todo: Uncomment when implemented
	//SEUtil::addOption_gui(windowStore3, _("Confirm item removal in download queue"), SettingsManager::CONFIRM_ITEM_REMOVAL);
	SEUtil::reAddItemCo(parent,old,box);
}

void WindowPage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();
	SEUtil::saveOptionsView_gui(windowOpenView,windowStore1,sm);
	SEUtil::saveOptionsView_gui(windowView2,windowStore2,sm);
	SEUtil::saveOptionsView_gui(windowView3,windowStore3,sm);
}
