/*
 * Copyright © 2004-2013 Jens Oknelid, paskharen@gmail.com
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

#ifndef _WULFOR_MANAGER_HH
#define _WULFOR_MANAGER_HH

#include <gtk/gtk.h>
#include <glib.h>
#include <string>

#include "dialogentry.hh"
#include "func.hh"
#include "mainwindow.hh"

// @because GtkFactory -> GtkIconTheme and stock -> icon_name
#if GTK_CHECK_VERSION(3,9,0)
	#define GTK_STOCK_CANCEL "_Cancel"
	#define GTK_STOCK_REMOVE "list-remove"
	#define GTK_STOCK_OPEN "_Open"
	#define GTK_STOCK_OK "_OK"
	#define GTK_STOCK_NO "_No"
	#define GTK_STOCK_YES "_Yes"
	#define GTK_STOCK_DIALOG_QUESTION "dialog-question"
	#define GTK_STOCK_FIND  "edit-find"
	#define GTK_STOCK_FILE "text-x-generic"
	#define GTK_STOCK_DIRECTORY "folder"
	#define GTK_STOCK_GO_DOWN "go-down"
	#define GTK_STOCK_GO_UP "go-up"
	#define GTK_STOCK_HOME "go-home"
	#define GTK_STOCK_CONVERT "convert"
	#define GTK_STOCK_PREFERENCES  "preferences-system"
	#define GTK_STOCK_NETWORK "network-workgroup"
	#define GTK_STOCK_CONNECT "gtk-connect"
	#define GTK_STOCK_QUIT "application-exit"
#endif

class WulforManager
{
	public:
		static void start(int argc, char **argv);
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
		gint openSettingsDialog_gui();
		DialogEntry *getHashDialog_gui();
		DialogEntry *getSettingsDialog_gui();

		void onReceived_gui(const std::string& link);

	private:
		// argv[1] from main
		static std::string argv1;

		// MainWindow-related functions
		void createMainWindow();

		// Entry functions
		DialogEntry *getDialogEntry_gui(const std::string &id);

		// Thread-related functions
		static gpointer threadFunc_client(gpointer data);
		void processClientQueue();

		static WulforManager *manager;
		MainWindow *mainWin;
		std::string path;
		std::deque<FuncBase *> clientFuncs;
		std::unordered_map<std::string, Entry *> entries;
		gint clientCondValue;
		GCond clientCond;
		GMutex clientCondMutex;
		GMutex clientCallMutex;
		GMutex clientQueueMutex;
		GRWLock entryMutex;
		GThread *clientThread;
		bool abort;
};

#else
class WulforManager;
#endif
