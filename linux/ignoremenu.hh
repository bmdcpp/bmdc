/*
 * Copyright © 2017
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
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
		static void onClicked_gui(GtkMenuItem *menuItem, gpointer data);
		static void onClickedIp_gui(GtkMenuItem *menuItem, gpointer data);
		static void onClickedCid_gui(GtkMenuItem *menuItem, gpointer data);
};

#else
class IgnoreMenu;
#endif
