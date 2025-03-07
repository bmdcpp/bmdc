/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "entry.hh"
#include "wulformanager.hh"

class ShortCuts: public Entry
{
	public:
	ShortCuts():
	Entry(Entry::SHORTCUTS,"shortcuts")
	{
		
	
	}
	~ShortCuts() {}
	
	GtkWidget* getContainer() {
		return getWidget("shortcuts-bmdc");
	}
	
	void show()
	{
		GtkWidget *overlay = GTK_WIDGET (getWidget("shortcuts-bmdc"));
		gtk_window_set_transient_for (GTK_WINDOW (overlay), GTK_WINDOW (WulforManager::get()->getMainWindow()->getContainer()));
		g_object_set (overlay, "view-name", NULL, NULL);
		gtk_widget_show (overlay);
	}
	
};
