/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _SETTINGS_PAGE_
#define _SETTINGS_PAGE_
#include <gtk/gtk.h>
/*----------------------------Meta---------------------------------------------*/
class SettingsPage
{
	public:
	SettingsPage() { }
	virtual ~SettingsPage() { }
	virtual void show(GtkWidget *parent, GtkWidget* old) = 0;
	virtual GtkWidget* getTopWidget() = 0;
	virtual const char* get_name_page() = 0;
	virtual void write() { }
};
#else
class SettingsPage;
#endif
/*------------------------------------------------------------------------------*/
