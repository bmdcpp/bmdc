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

#ifndef _BMDC_MANAGER_HH
#define _BMDC_MANAGER_HH

#include <gtk/gtk.h>
#include <glib.h>
#include <string>

#include "dialogentry.hh"
#include "func.hh"
#include "mainwindow.hh"

#include "gtk-fixies.hh"

class WulforManager
{
	public:
		static int start(int argc, char **argv);
		static void stop();
		static WulforManager *get();

		WulforManager();
		~WulforManager();

		std::string getURL();
		std::string getPath() const;
		MainWindow *getMainWindow();
		void deleteMainWindow();
		void dispatchGuiFunc(FuncBase *func);
		void dispatchClientFunc(FuncBase *func);

		void insertEntry_gui(Entry *entry);
		void deleteEntry_gui(Entry *entry);
		bool isEntry_gui(Entry *entry);

		// DialogEntry functions
		gint openHashDialog_gui();
		GtkWidget* openSettingsDialog_gui();
		
		DialogEntry *getHashDialog_gui();
		
		DialogEntry *getSettingsDialog_gui();

		void onReceived_gui(const std::string& link);
        GApplication* getApplication() { return G_APPLICATION(application);}
	private:
		// argv[1] from main
		static std::string argv1;

		// MainWindow-related functions
		int createMainWindow();
		static void activate(GtkApplication* app,
          gpointer        user_data);
		static void shutdown(GtkApplication* app,
          gpointer        user_data);

		// Entry functions
		DialogEntry *getDialogEntry_gui(const std::string &id);

		static WulforManager *manager;
		MainWindow *mainWin;
		std::string path;
		std::unordered_map<std::string, Entry *> entries;
		GRWLock entryMutex;
        GtkApplication *application;
        static int argc;
        static char** argv;

};

#else
class WulforManager;
#endif
