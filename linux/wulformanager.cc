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
#define USE_NEW_SETTINGS 1
#include "wulformanager.hh"
#include "GuiUtil.hh"

#include <iostream>
#include <vector>
#include <string>
#include <glib/gi18n.h>
#include "hashdialog.hh"

#ifdef USE_NEW_SETTINGS
#include "../settings/SettingsDialog.hh"
#else
#include "settingsdialog.hh"
#endif

using namespace std;
using namespace dcpp;

WulforManager *WulforManager::manager = NULL;
string WulforManager::argv1;
char** WulforManager::argv;
int WulforManager::argc;

int WulforManager::start(int _argc, char **_argv)
{
	argc = _argc;
	argv = _argv;
	if (argc > 1)
	{
		argv1 = _argv[1];
	}

	int status;
	// Create WulforManager
	dcassert(!manager);
	manager = new WulforManager();
	status = manager->createMainWindow();
	return status;
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
	// Initialize sempahore variables
	g_rw_lock_init(&entryMutex);
	// Determine path to data files

//	const gchar* const* g_path = g_get_system_data_dirs();
//path = string(_DATADIR) + "bmdc" + G_DIR_SEPARATOR_S;
	path = ".";

   	// Set the custom icon search path so GTK+ can find our icons
   const string iconPath = path + G_DIR_SEPARATOR_S + "icons";
   const string themes = path + G_DIR_SEPARATOR_S + "themes";

	GtkIconTheme* icon_theme = gtk_icon_theme_get_for_display (gdk_display_get_default());
   gtk_icon_theme_add_search_path(icon_theme , iconPath.c_str());
}

WulforManager::~WulforManager()
{
	g_rw_lock_clear(&entryMutex);
   
}

int WulforManager::createMainWindow()
{
	int status;

    application = gtk_application_new ("org.bmdcteam.bmdc", G_APPLICATION_FLAGS_NONE); //TODO::not-deprectaed
    g_signal_connect (application, "activate", G_CALLBACK (activate), (gpointer)this);
    g_signal_connect (application, "shutdown", G_CALLBACK (shutdown), (gpointer)this);
	status = g_application_run (G_APPLICATION (application), argc, argv);
	return status;
}

void WulforManager::shutdown(GtkApplication* app,
          gpointer        data)
{
	WulforSettingsManager::deleteInstance();
	std::cout << _("Shutting down dcpp client...") << std::endl;
	try{
	dcpp::shutdown();
}catch(...){ }
	std::cout << _("Quit...") << std::endl;
}          

void WulforManager::activate(GtkApplication* app,
          gpointer        data)
{
	GtkWidget* window = gtk_application_window_new (app);
	WulforManager* w = (WulforManager*)data;
	w->mainWin = new MainWindow(window);
	w->mainWin->show();
}

void WulforManager::deleteMainWindow()
{
	// response dialogs: hash, settings
	DialogEntry *hashDialogEntry = getHashDialog_gui();
#ifndef USE_NEW_SETTINGS
	DialogEntry *settingsDialogEntry = getSettingsDialog_gui();
#endif
	if (hashDialogEntry != NULL)
	{
		gtk_dialog_response(GTK_DIALOG(hashDialogEntry->getContainer()), GTK_RESPONSE_OK);
	}
#ifndef USE_NEW_SETTINGS
	if (settingsDialogEntry != NULL)
	{
		dynamic_cast<Settings*>(settingsDialogEntry)->response_gui();
	}
#endif
	mainWin->remove();
	mainWin = NULL;
}
void WulforManager::dispatchGuiFunc(FuncBase *func)
{
    g_idle_add((GSourceFunc)(func)->call_,(gpointer)func);
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

GtkWidget* WulforManager::openSettingsDialog_gui()
{
	SettingsDialog *s = new SettingsDialog();
	return s->getContainer();
}

DialogEntry *WulforManager::getHashDialog_gui()
{
	return getDialogEntry_gui(Util::toString(Entry::HASH_DIALOG) + ":");
}

DialogEntry *WulforManager::getSettingsDialog_gui()
{
	return getDialogEntry_gui(Util::toString(Entry::SETTINGS_DIALOG) + ":");
}
