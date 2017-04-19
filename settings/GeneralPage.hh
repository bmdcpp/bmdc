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

#ifndef _GENERAL_PAGE_
#define _GENERAL_PAGE_
#include "SettingsPage.hh"
#include <vector>
#include <string>

class GeneralPage: public SettingsPage
{
	public:
		const char* get_name_page(){ return page_name; }
		GeneralPage();
		~GeneralPage(){};
		void show(GtkWidget *parent, GtkWidget* old);
		void write();
		GtkWidget* getTopWidget() {return box_grid;}
	private:
		GtkWidget *entry_nick,*entry_desc,*entry_email,
		*box_grid;
		GtkComboBoxText *conn,*codepage;
		static const char* page_name;
		std::vector<std::string> connectionSpeeds;

};
#else
class GeneralPage;
#endif
