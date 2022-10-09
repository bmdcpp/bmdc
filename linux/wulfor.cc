/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2024 BMDC
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
#include "../dcpp/format.h"
#ifndef _WIN32
#include <glib/gi18n.h>
#endif
#ifdef _WIN32
#undef USE_STACKTRACE
#endif
#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "settingsmanager.hh"
#include "IgnoreTempManager.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "Splash.hh"
#include "version.hh"
#include <iostream>
#include <iterator>
#include <fstream>
#include <signal.h>
#include "stacktrace.hh"

#define GUI_LOCALE_DIR _DATADIR PATH_SEPARATOR_STR "locale"

using namespace std;
using namespace dcpp;

static bool b_crash = false;

void handle_crash(int )
{
    if(b_crash)
        abort();

    b_crash = true;

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
	f << "\nGTK Version: " << std::endl;
	f << gtk_get_major_version();
	f << ".";
	f << gtk_get_minor_version();
	f << ".";
	f << gtk_get_micro_version();
	f << std::endl;
	f << " Compiler Version: " << std::endl;
	#ifdef __clang__
	f << "clang " __clang_version__;
	#elif defined(__GNUC__)
	f << "gcc " __VERSION__;
	#endif
	f << std::endl;
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
	// Initialize i18n support
	bindtextdomain(GUI_LOCALE_PACKAGE, GUI_LOCALE_DIR);
	textdomain(GUI_LOCALE_PACKAGE);
	bind_textdomain_codeset(GUI_LOCALE_PACKAGE, "UTF-8");

	dcpp::Util::PathsMap map;
	string home = string(g_get_home_dir ()) + "/.bmdc++/";
	map[dcpp::Util::PATH_GLOBAL_CONFIG] = home;
	if(argc >= 1) {
		if(argv[1] != NULL) {
			map[dcpp::Util::PATH_GLOBAL_CONFIG] = string(argv[1]);
			map[dcpp::Util::PATH_USER_CONFIG] = string(argv[1]);
			map[dcpp::Util::PATH_DOWNLOADS] = string(argv[1]);
			}
	}
	// Start the DC++ client core
	if(map.size() > 1) {
		dcpp::Util::initialize(map);
	} else
		dcpp::Util::initialize();

	gtk_init();

	Splash* pSplash = new Splash();
	pSplash->show();
	dcpp::startup();
	try{
		dcpp::load([pSplash](const string& str){ pSplash->setText(str); pSplash->update(); },
		[pSplash](const float& str){ pSplash->setPercentage(str); pSplash->update(); }  );
	}catch(...){
	///
	}
	pSplash->destroy();
	delete pSplash;
	try {
	dcpp::TimerManager::getInstance()->start();
	}catch(...){
	///
	}
	g_set_application_name("BMDC++");
	WulforSettingsManager::newInstance();
	IgnoreTempManager::newInstance();
	#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
	signal(SIGSEGV, handle_crash);
	signal(SIGINT,  handle_crash);
	signal(SIGILL,  handle_crash);
	signal(SIGFPE,  handle_crash);
	signal(SIGABRT, handle_crash);
	signal(SIGTERM, handle_crash);
	#endif
	int status = WulforManager::start(argc, argv);
	return status;
}

