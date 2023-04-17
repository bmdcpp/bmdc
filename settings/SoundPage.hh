
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

#ifndef _SOUND_PAGE_
#define _SOUND_PAGE_
#include <gtk/gtk.h>
#include <string>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"
#include "../linux/settingsmanager.hh"
/*-----------------------------------------------------------------------------*/
class SoundPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget *old);
		const char* get_name_page()
		{ return name_page;}
		virtual void write(){}
		virtual GtkWidget* getTopWidget(){return box;}
	private:
		static const char* name_page;
		GtkListStore *soundStore;
		TreeView soundView;
		GtkWidget *button_play,*button_browse,*grid,*box;
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const std::string &name, const std::string &key1, const std::string &key2);
		static void onSoundFileBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onSoundPlayButton_gui(GtkWidget *widget, gpointer data);
};
#else
class SoundPage;
#endif
/*-------------------------------------------------------------------------------*/
