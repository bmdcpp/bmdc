/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "DownloadsPage.hh"
#include "definitons.hh"
#include "seUtil.hh"
#include "../dcpp/SettingsManager.h"

using namespace dcpp;

const char* DownloadsPage::name_page = "Downloads";

void DownloadsPage::show(GtkWidget *parent, GtkWidget* old)
{
	box_grid = gtk_grid_new();
	entry_down_path = gen;
	entry_down_path_temp = gen;
	button_browse = gtk_button_new_with_label(_("Browse..."));
	button_browse_temp = gtk_button_new_with_label(_("Browse..."));
	
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Download Path: ")),0,0,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),entry_down_path,1,0,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),button_browse,2,0,1,1);
	/*temp*/
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Temp Download Path: ")),0,1,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),entry_down_path_temp,1,1,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),button_browse_temp,2,1,1,1);
	
	spin_max_down_speed = gtk_spin_button_new_with_range(0,100000000000,1);
	
	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Max Download Speed: ")),0,2,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),spin_max_down_speed,1,2,1,1);
	
	spin_slots_d = gtk_spin_button_new_with_range(0,100,1);

	gtk_grid_attach(GTK_GRID(box_grid),gtk_label_new(_("Max Download Slots: ")),0,3,1,1);
	gtk_grid_attach(GTK_GRID(box_grid),spin_slots_d,1,3,1,1);
	
	gtk_editable_set_text(GTK_EDITABLE(entry_down_path), SETTING(DOWNLOAD_DIRECTORY).c_str());
	gtk_editable_set_text(GTK_EDITABLE(entry_down_path_temp), SETTING(TEMP_DOWNLOAD_DIRECTORY).c_str());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_slots_d), (double)SETTING(DOWNLOAD_SLOTS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_max_down_speed), (double)SETTING(MAX_DOWNLOAD_SPEED));
	
	g_signal_connect(button_browse, "clicked", G_CALLBACK(onBrowseFinished_gui), (gpointer)this);
	g_signal_connect(button_browse_temp, "clicked", G_CALLBACK(onBrowseUnfinished_gui), (gpointer)this);

}

void DownloadsPage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();
	
	// Downloads
	const gchar* s_path = gtk_editable_get_text(GTK_EDITABLE(entry_down_path));
	if(s_path) {
		string path = s_path;
		if (path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::DOWNLOAD_DIRECTORY, path);
	}
	const gchar* s_path2 = gtk_editable_get_text(GTK_EDITABLE(entry_down_path_temp));
	if(s_path2) {
		string path = s_path2;
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, path);
	}	
	sm->set(SettingsManager::DOWNLOAD_SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_slots_d)));
	sm->set(SettingsManager::MAX_DOWNLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_max_down_speed)));

}

void DownloadsPage::onBrowseFinished_gui(GtkWidget *widget, gpointer data)
{
	DownloadsPage *s = (DownloadsPage *)data;
	/*GtkWidget* dialog = b_file_dialog_widget(_("Open Directory"));
		
 	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
		if (path)
		{
			gtk_editable_set_text(GTK_EDITABLE(s->entry_down_path), Text::toUtf8(path).c_str());
			g_free(path);
		}
	}*/
}

void DownloadsPage::onBrowseUnfinished_gui(GtkWidget *widget, gpointer data)
{
	DownloadsPage *s = (DownloadsPage *)data;
	/*GtkWidget* dialog = b_file_dialog_widget(_("Open Directory"));
	
 	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
		if (path)
		{
			gtk_editable_set_text(GTK_EDITABLE(s->entry_down_path_temp),  Text::toUtf8(path).c_str());
			g_free(path);
		}
	}*/
}
