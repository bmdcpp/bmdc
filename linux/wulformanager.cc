/*
 * Copyright © 2004-2015 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2015 Mank <freedcpp at seznam dot cz>
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

#include "wulformanager.hh"
#include "WulforUtil.hh"

#include <iostream>
#include <glib/gi18n.h>
#include "hashdialog.hh"
#include "settingsdialog.hh"

using namespace std;
using namespace dcpp;

WulforManager *WulforManager::manager = NULL;
string WulforManager::argv1;

void WulforManager::start(int argc, char **argv)
{
	if (argc > 1)
	{
		argv1 = argv[1];
	}

	// Create WulforManager
	dcassert(!manager);
	manager = new WulforManager();
	manager->createMainWindow();
}

void WulforManager::stop()
{
	dcassert(manager);
	delete manager;
	manager = NULL;
}

WulforManager *WulforManager::get()
{
	dcassert(manager);
	return manager;
}

WulforManager::WulforManager():
mainWin(NULL)
{
	abort = false;

	// Initialize sempahore variables
	g_rw_lock_init(&entryMutex);
	// Determine path to data files
	path = string(_DATADIR) + G_DIR_SEPARATOR_S + g_get_prgname();
	if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
	{
		cerr << path << " is inaccessible, falling back to current directory instead.\n";
		path = ".";
	}

	// Set the custom icon search path so GTK+ can find our icons
	const string iconPath = path + G_DIR_SEPARATOR_S + "icons";
	const string themes = path + G_DIR_SEPARATOR_S + "themes";
	GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
	gtk_icon_theme_append_search_path(iconTheme, iconPath.c_str());
	gtk_icon_theme_append_search_path(iconTheme, themes.c_str());
}

WulforManager::~WulforManager()
{
	abort = TRUE;
	g_rw_lock_clear(&entryMutex);
}

void WulforManager::createMainWindow()
{
	dcassert(!mainWin);
	mainWin = new MainWindow();
	WulforManager::insertEntry_gui(mainWin);
	mainWin->show();
}

void WulforManager::deleteMainWindow()
{
	// response dialogs: hash, settings
	DialogEntry *hashDialogEntry = getHashDialog_gui();
	DialogEntry *settingsDialogEntry = getSettingsDialog_gui();

	if (hashDialogEntry != NULL)
	{
		gtk_dialog_response(GTK_DIALOG(hashDialogEntry->getContainer()), GTK_RESPONSE_OK);
	}
	if (settingsDialogEntry != NULL)
	{
		dynamic_cast<Settings*>(settingsDialogEntry)->response_gui();
	}

	mainWin->remove();
	mainWin = NULL;
	gtk_main_quit();
}

static void xnotify(gpointer data)
{ delete (FuncBase*)data; }

void WulforManager::dispatchGuiFunc(FuncBase *func)
{
	g_idle_add_full ((gint)G_PRIORITY_DEFAULT_IDLE,(GSourceFunc)((func)->call_),func, xnotify);
}

void WulforManager::dispatchClientFunc(FuncBase *func)
{
	func->call();
}

MainWindow *WulforManager::getMainWindow()
{
	dcassert(mainWin);
	return mainWin;
}

string WulforManager::getURL()
{
	return argv1;
}

string WulforManager::getPath() const
{
	return path;
}

void WulforManager::insertEntry_gui(Entry *entry)
{
	g_rw_lock_writer_lock(&entryMutex);
	entries[entry->getID()] = entry;
	g_rw_lock_writer_unlock(&entryMutex);
}

void WulforManager::deleteEntry_gui(Entry *entry)
{
	const string &id = entry->getID();

	// Remove the bookentry from the list.
	g_rw_lock_writer_lock(&entryMutex);
	entries.erase(id);
	g_rw_lock_writer_unlock(&entryMutex);

	delete entry;
	entry = NULL;
}

bool WulforManager::isEntry_gui(Entry *entry)
{
	g_rw_lock_writer_lock(&entryMutex);

	unordered_map<string, Entry *>::const_iterator it = find_if(entries.begin(), entries.end(),
		CompareSecond<string, Entry *>(entry));

	if (it == entries.end())
		entry = NULL;

	g_rw_lock_writer_unlock(&entryMutex);

	return (entry != NULL);
}

DialogEntry* WulforManager::getDialogEntry_gui(const string &id)
{
	DialogEntry *ret = NULL;

	g_rw_lock_reader_lock(&entryMutex);
	if (entries.find(id) != entries.end())
		ret = dynamic_cast<DialogEntry *>(entries[id]);
	g_rw_lock_reader_unlock(&entryMutex);

	return ret;
}

void WulforManager::onReceived_gui(const string& link)
{
	dcassert(mainWin);

	if (WulforUtil::isHubURL(link) && SETTING(URL_HANDLER))
		mainWin->showHub_gui(link);

	else if (WulforUtil::isMagnet(link) && SETTING(MAGNET_REGISTER))
		mainWin->actionMagnet_gui(link);
}

gint WulforManager::openHashDialog_gui()
{
	Hash *h = new Hash();
	gint response = h->run();

	return response;
}

gint WulforManager::openSettingsDialog_gui()
{
	Settings *s = new Settings();
	gint response = s->run();

	return response;
}

DialogEntry *WulforManager::getHashDialog_gui()
{
	return getDialogEntry_gui(Util::toString(Entry::HASH_DIALOG) + ":");
}

DialogEntry *WulforManager::getSettingsDialog_gui()
{
	return getDialogEntry_gui(Util::toString(Entry::SETTINGS_DIALOG) + ":");
}
