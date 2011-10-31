//      System.hh
//      
//      Copyright 2011 Mank <Mank1@seznam.cz>
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
//      
//      

#ifndef SYSTEM_HH
#define SYSTEM_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/LogManager.h>
#include <dcpp/LogManagerListener.h>

#include "bookentry.hh"
#include "treeview.hh"


class systemlog:
	public BookEntry, private dcpp::LogManagerListener
{
	public:
		systemlog();
		virtual ~systemlog();
		virtual void show();

	private:
		//GUI
		static void onClearButton(GtkWidget *widget, gpointer data);
		// Client functions
		void ini_client();
		//GUI FCE
		void add_gui(time_t t,std::string file);
		static void onScroll_gui(GtkAdjustment *adjustment, gpointer data);
		static void onResize_gui(GtkAdjustment *adjustment, gpointer data);

		// LogManagerListener
		virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string& message) throw();

		GtkTextBuffer *buffer;
		GtkTextIter iter;
		GtkTextMark *sysMark;
		static const int maxLines = 1000;
		bool scrollToBottom;

};

#else
class systemlog;
#endif
