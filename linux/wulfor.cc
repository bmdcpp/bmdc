/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2016 BMDC
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
#ifndef _WIN32
#include <glib/gi18n.h>
#include "bacon-message-connection.hh"
#endif
#ifdef _WIN32
#undef USE_STACKTRACE
#endif
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "Splash.hh"
#include "version.hh"
#include <iostream>
#include <iterator>
#include <fstream>
#include <signal.h>
#include "stacktrace.hh"

#define GUI_LOCALE_DIR _DATADIR PATH_SEPARATOR_STR "locale"
#ifndef _WIN32
BaconMessageConnection *connection = NULL;
#endif
void receiver(const char *link, gpointer )
{
	g_return_if_fail(link != NULL);
	WulforManager::get()->onReceived_gui(link);
}

using namespace std;
using namespace dcpp;

static bool m_crash = false;

void handle_crash(int )
{
    if(m_crash)
        abort();

    m_crash = true;

    std::cerr << "pid: " << getpid() << std::endl;
#ifndef _WIN32
#if USE_STACKTRACE
    cow::StackTrace trace;
    trace.generate_frames();
    std::copy(trace.begin(), trace.end(),
        std::ostream_iterator<cow::StackFrame>(std::cerr, "\n"));

	string stackPath = Util::getPath(Util::PATH_USER_CONFIG) + "exceptioninfo.txt";
	std::ofstream f;
	f.open(stackPath.c_str(),ios_base::out | ios_base::app);
	std::copy(trace.begin(), trace.end(),
		std::ostream_iterator<cow::StackFrame>(f, "\n"));
	f << "\nGTK Version: \n";
	printf("Version");
	f << gtk_get_major_version();
	f << ".";
	f << gtk_get_minor_version();
	f << ".";
	f << gtk_get_micro_version();
	printf("\nCompiler version\n");
	f << "\n Compiler Version: \n";
	#ifdef __clang__
	f << "clang " __clang_version__;
	#elif defined(__GNUC__)
	f << "gcc " __VERSION__;
	#endif
	f << "\n";
	printf("Close");
	f.close();
	
	std::cout << "\nException info to be posted on the bug tracker has also been saved in " + stackPath << std::endl;
#else
    std::cerr << "Stacktrace is not enabled\n";
#endif

#endif
	return exit(0);
}

int main(int argc, char *argv[])
{
	#ifndef _WIN32
	// Initialize i18n support
	bindtextdomain(GUI_LOCALE_PACKAGE, GUI_LOCALE_DIR);
	textdomain(GUI_LOCALE_PACKAGE);
	bind_textdomain_codeset(GUI_LOCALE_PACKAGE, "UTF-8");
	
	connection = bacon_message_connection_new(GUI_PACKAGE);

	if (connection != NULL) {
		dcdebug("bmdc: connection yes...\n");
	}else {
		dcdebug("bmdc: connection no...\n");
	}
	// Check if profile is locked
	if (WulforUtil::profileIsLocked())
	{
		if (!bacon_message_connection_get_is_server(connection))
		{
			dcdebug("bmdc: is client...\n");

			if (argc > 1)
			{
				dcdebug("bmdc: send %s\n", argv[1]);
				bacon_message_connection_send(connection, argv[1]);
			}
		}

		bacon_message_connection_free(connection);

		return 1;
	}

	if (bacon_message_connection_get_is_server(connection))
	{
		dcdebug("bmdc: is server...\n");
		bacon_message_connection_set_callback(connection, receiver, NULL);
	}
	#endif
	// Start the DC++ client core

	dcpp::Util::initialize();
   
	gtk_init(&argc, &argv);

	Splash* sp = new Splash();
	sp->show();
	dcpp::startup();
	try{
		dcpp::load([sp](const string& str){ sp->setText(str); sp->update(); },
		[sp](const float& str){ sp->setPercentage(str); sp->update(); }  );
	}catch(...){
	///	
	}
	sp->destroy();
	delete sp;
	try {
	dcpp::TimerManager::getInstance()->start();
	}catch(...){
	///	
	}
	g_set_application_name("BMDC++");
	WulforSettingsManager::newInstance();
	#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
	signal(SIGSEGV, handle_crash);
	signal(SIGINT,  handle_crash);
	signal(SIGILL,  handle_crash);
	signal(SIGFPE,  handle_crash);
	signal(SIGABRT, handle_crash);
	signal(SIGTERM, handle_crash);
	#endif
	WulforManager::start(argc, argv);
	gtk_main();
	#ifndef _WIN32
	bacon_message_connection_free(connection);
	#endif
	WulforManager::stop();
	WulforSettingsManager::deleteInstance();

	std::cout << _("Shutting down dcpp client...") << std::endl;
	try{
	dcpp::shutdown();
}catch(...){}
	std::cout << _("Quit...") << std::endl;

	return 0;
}

