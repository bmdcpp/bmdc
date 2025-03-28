/*
* Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _HASH_HH
#define _HASH_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/format.h"
#include "../dcpp/TimerManager.h"

#include "dialogentry.hh"

class Hash:
	public DialogEntry,
	public dcpp::TimerManagerListener
{
	public:
		Hash(GtkWindow* parent = NULL);
		~Hash();

	private:
		using dcpp::TimerManagerListener::on;
		// GUI functions
		void updateStats_gui(std::string file, uint64_t bytes, size_t files, uint32_t tick);

		// Client callbacks
		virtual void on(dcpp::TimerManagerListener::Second, uint64_t tics) throw();

		// GUI callback
		static void onPauseHashing_gui(GtkWidget *widget, gpointer data);

		uint64_t startBytes;
		size_t startFiles;
		uint32_t startTime;
};

#else
class Hash;
#endif
