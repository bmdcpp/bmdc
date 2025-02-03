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

#ifndef _BMDC_DIALOG_ENTRY_HH
#define _BMDC_DIALOG_ENTRY_HH

#include "entry.hh"

class DialogEntry : public Entry
{
	public:
		DialogEntry()  {}
		DialogEntry(const EntryType type, const std::string &glade, GtkWindow*);
		virtual ~DialogEntry();

		GtkWidget *getContainer();
		gint run();
		gint getResponseID() const;

	private:
		gint responseID;
};

#else
class DialogEntry;
#endif
