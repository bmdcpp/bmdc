/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <glib/gi18n.h>

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/UPnPManager.h>//NOTE: core 0.762

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "Splash.hh"
#include "version.hh"
#include "upnpc.hh"//NOTE: core 0.762
#include <iostream>
#include <signal.h>

#define GUI_LOCALE_DIR _DATADIR PATH_SEPARATOR_STR "locale"


void receiver(const char *link, gpointer data)
{
	g_return_if_fail(link != NULL);
	WulforManager::get()->onReceived_gui(link);
}

void callBack(void* x, const std::string& a)
{
	std::cout << "Loading: " << a << std::endl;
	Splash *sp = (Splash *)x;
	sp->setText(a);
	sp->update();
}

int main(int argc, char *argv[])
{
	// Initialize i18n support
	bindtextdomain(GUI_PACKAGE, GUI_LOCALE_DIR);
	textdomain(GUI_PACKAGE);
	bind_textdomain_codeset(GUI_PACKAGE, "UTF-8");

	// Check if profile is locked
	if (WulforUtil::profileIsLocked())
	{
		cout << _("No More That one Instance\n");
		return 1;
	}

	// Start the DC++ client core
	dcpp::Util::initialize();//NOTE: core 0.762
	gtk_init(&argc, &argv);
	Splash* sp = new Splash();
	sp->show();
	dcpp::startup(callBack, (void*)sp);
	sp->destroy();
	dcpp::UPnPManager::getInstance()->addImplementation(new UPnPc());//NOTE: core 0.762
	dcpp::TimerManager::getInstance()->start();

	g_thread_init(NULL);
	gdk_threads_init();
	glade_init();
	g_set_application_name("BMDC++");

	signal(SIGPIPE, SIG_IGN);

	WulforSettingsManager::newInstance();
	WulforManager::start(argc, argv);
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	WulforManager::stop();
	WulforSettingsManager::deleteInstance();

	std::cout << _("Shutting down dcpp client...") << std::endl;
	dcpp::shutdown();
	std::cout << _("Quit...") << std::endl;

	return 0;
}

