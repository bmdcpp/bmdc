/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _IGNORE_MENU_HH
#define _IGNORE_MENU_HH

class IgnoreMenu
{
	public:
		IgnoreMenu(GtkWidget *menu) : appsPreviewMenu(menu) {}
		~IgnoreMenu() {}

		void cleanMenu_gui();
		void buildMenu_gui(const std::string &nick,const std::string& cid, const std::string& ip);

	private:

		GtkWidget *appsPreviewMenu;
};

#else
class IgnoreMenu;
#endif
