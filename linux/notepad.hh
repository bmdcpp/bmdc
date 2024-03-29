//      Copyright BMDC 2025
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#ifndef _BMDC_NOTEPAD_HH
#define _BMDC_NOTEPAD_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"

#include "bookentry.hh"
#include "treeview.hh"

class Notepad:
	public BookEntry
{
	public:
		Notepad();
		virtual ~Notepad();
		virtual void show();

	private:
		void ini_client();
		void add_gui(std::string file);

		GtkTextBuffer *buffer;
};

#else
class notepad;
#endif
