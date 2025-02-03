/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _CHAT_SETTINGS_PAGE_
#define _CHAT_SETTINGS_PAGE_
#include <gtk/gtk.h>
#include <string>
#include "../linux/settingsmanager.hh"
#include "SettingsPage.hh"
#include "../linux/treeview.hh"
/*-----------------------------------------------------------------------------*/
class ChatPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page(){ return page_name;}
		virtual void write();
		GtkWidget* getTopWidget(){return box;}
	private:
		static const char* page_name;
		TreeView textStyleView;
		GtkListStore* textStyleStore;
		GtkWidget *box, *toggle_autors, *textView;
		GtkTextBuffer* textStyleBuffer;
		GtkWidget *bFore,*bBack,*bStyle,*bBW,*bDef,*bBaAll;
		void selectTextColor_gui(const int select);
		void selectTextStyle_gui(const int select);
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const std::string &name,
		const std::string &key1, const std::string &key2, const std::string &key3, const std::string &key4);
		static void onTextColorForeClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextColorBackClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextColorBWClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextStyleClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextStyleDefaultClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextBackGroundChat(GtkWidget *widget , gpointer data);

};
#else
class ChatPage;
#endif
/*-------------------------------------------------------------------------------*/
