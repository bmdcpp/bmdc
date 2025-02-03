/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _OAPP_PAGE_
#define _OAPP_PAGE_
#include <gtk/gtk.h>
#include <vector>
#include <string>
#include "SettingsPage.hh"
/*-----------------------------------------------------------------------------*/
class OApearencePage: public SettingsPage
{
	public:
		OApearencePage();
		~OApearencePage(){ }
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page()
		{ return page_name;};
		void write();
		GtkWidget* getTopWidget(){return grid;}
	private:
		std::string getNameAction(int num);
		static const char* page_name;
		GtkWidget *entry_country,*entry_timestamp,
		*entry_away, *entry_ripe,*entry_ratio,*entry_chat_info,
		*grid;
		GtkComboBoxText *actionUL;
		std::vector<std::string> usersAction;

};
#else
class OApearencePage;
#endif
/*-------------------------------------------------------------------------------*/
