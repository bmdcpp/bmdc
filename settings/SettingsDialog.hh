/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _SETTINGS_DIALOG_
#define _SETTINGS_DIALOG_
#include <gtk/gtk.h>
#include "SettingsPage.hh"

class SettingsDialog
{
	public:
		SettingsDialog();
		~SettingsDialog();
		GtkWidget *getContainer() { return dia; }
		void run();
		std::string& getID() { return id;}
	private:
		SettingsPage* pages[17];
		GtkWidget* dialogWin,*dia ,
		* mainBox,	* statusBox,
		* okButton,*stButton,
		* paned,*tree,*stack,
		* containBox;
		GtkListStore *store;
		GtkTreeModel *model;
		GtkTreeSelection *tree_sel;
		gint previous_page;
		gint actual_page;
		std::string id;
		int m_num;
	public:	
		void close() { 
			for(int i = 0; i <= m_num; ++i) {
				if(pages[i] != NULL) {
					pages[i]->write();
				}
			}
		}
	private:	
		static void onOkButton(GtkWidget *widget, gpointer data);
		static void onStButton(GtkWidget *widget, gpointer data);
};
#endif
