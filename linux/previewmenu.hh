/*
* Copyright © 2009-2017 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _PREVIEW_MENU_HH
#define _PREVIEW_MENU_HH

class PreviewMenu
{
	public:
		PreviewMenu(GtkWidget *menu) : appsPreviewMenu(menu) {}
		~PreviewMenu() {}

		void cleanMenu_gui();
		bool buildMenu_gui(const std::string &target);

	private:

		GtkWidget *appsPreviewMenu;
		//static void onPreviewAppClicked_gui(GtkMenuItem *menuItem, gpointer data);
};

#else
class PreviewMenu;
#endif
