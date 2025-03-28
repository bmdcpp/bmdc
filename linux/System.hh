/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_SYSTEM_HH
#define _BMDC_SYSTEM_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/LogManager.h"
#include "../dcpp/LogManagerListener.h"

#include "bookentry.hh"
#include "treeview.hh"

class SystemLog:
	public BookEntry, private dcpp::LogManagerListener
{
	private:
		using dcpp::LogManagerListener::on;
	public:
		SystemLog();
		virtual ~SystemLog();
		virtual void show();
		void preferences_gui();

	private:
		// Client functions
		void ini_client();
		//Gui functions
		void add_gui(time_t t,std::string message,int sev = dcpp::LogManager::Sev::LOW);
		static void onScroll_gui(GtkAdjustment *adjustment, gpointer data);
		static void onResize_gui(GtkAdjustment *adjustment, gpointer data);
		static void onClearButton(GtkWidget*, gpointer data);
		// LogManagerListener
		virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string& message,int sev) noexcept;

		GtkTextBuffer *buffer;
		GtkTextIter iter;
		GtkTextMark *sysMark;
		static const int maxLines = 1000;
		bool scrollToBottom;
		GdkPaintable* getImageSev(int sev);

};

#else
class systemlog;
#endif
