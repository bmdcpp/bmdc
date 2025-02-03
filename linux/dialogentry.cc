/*
* Copyright © 2004-2018 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dialogentry.hh"
#include "wulformanager.hh"

using namespace std;

DialogEntry::DialogEntry(const EntryType type, const string &glade, GtkWindow*):
	Entry(type, glade),
	responseID(GTK_RESPONSE_NONE)
{
	
}

DialogEntry::~DialogEntry()
{
	
}

GtkWidget* DialogEntry::getContainer()
{
	return getWidget("dialog");
}

gint DialogEntry::run()
{
	gtk_widget_show(getWidget("dialog"));

	return GTK_RESPONSE_OK;
}

gint DialogEntry::getResponseID() const
{
	return GTK_RESPONSE_OK;
}
