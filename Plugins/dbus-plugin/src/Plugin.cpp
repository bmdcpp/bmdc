/*
 * Copyright (C) 2012-2014 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdafx.h"
#include "Plugin.h"

/* Include plugin SDK helpers. There are more interfaces available that can be included in the same
fashion (check the pluginsdk directory). */
#include <pluginsdk/Config.h>
#include <pluginsdk/Core.h>
#include <pluginsdk/Hooks.h>
#include <pluginsdk/Hubs.h>
#include <pluginsdk/Logger.h>
#include <pluginsdk/Util.h>

#ifndef __cplusplus
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#else
# include <cstdio>
# include <cstdlib>
# include <cstring>
#endif
#ifdef _WIN32
# include "resource.h"
# ifdef _MSC_VER
#  define snprintf _snprintf
#  define snwprintf _snwprintf
# endif
#elif __GNUC__
# define stricmp strcasecmp
# define strnicmp strncasecmp
#else
# error No supported compiler found
#endif
//custom headers
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <string>
#include <map>
#include <vector>

/* Plugin SDK helpers are in the "dcapi" namespace; ease their calling. */
using dcapi::Config;
using dcapi::Core;
using dcapi::Hooks;
using dcapi::Hubs;
using dcapi::Logger;
using dcapi::Util;

static const string commandName = "Configure media broadcasts";
Plugin* Plugin::instance = NULL;
Plugin::Plugin() {
}

Plugin::~Plugin() {
	Hooks::clear();
}
Bool DCAPI Plugin::main(PluginState state, DCCorePtr core, dcptr_t) {

	switch(state) {
	case ON_INSTALL:
	case ON_LOAD:
	case ON_LOAD_RUNTIME:
		{
			instance = new Plugin();
			return instance->onLoad(core, state == ON_INSTALL, state == ON_INSTALL || state == ON_LOAD_RUNTIME) ? True : False;
		}

	case ON_UNINSTALL:
	case ON_UNLOAD:
		{
			delete instance;
			instance = 0;
			return True;
		}

	default:
		{
			return False;
		}
	}
}

bool Plugin::onLoad(DCCorePtr core, bool install, bool runtime) {
	/* Initialization phase. Initiate additional interfaces that you may have included from the
	plugin SDK. */
	Core::init(core);
	if(!Config::init(PLUGIN_GUID) || !Hooks::init() || !Hubs::init() || !Logger::init() || !Util::init()) {
		return false;
	}

	if(install) {
		Config::setConfig("MediaPlayerFormat","playing %[artist] - %[album] - %[title]");
		Logger::log("The MediaPlayer plugin has been installed; check the \"" + commandName + "\" command.");
	}

	if(runtime) {
		// If the plugin doesn't support being enabled at runtime, cancel its loading here.
	}

	// Start the plugin logic here; add hooks with functions from the Hooks interface.
	addHooks();
	return true;
}

void Plugin::addHooks() {
	Hooks::UI::onChatCommand([this](HubDataPtr hHub, CommandDataPtr cmd, bool&) { return onChatCommand(hHub, cmd); });
	Hooks::UI::onChatCommandPM([this](UserDataPtr hUser, CommandDataPtr cmd, bool&) { return onChatCommandPM(hUser, cmd); });
}

bool Plugin::onChatCommand(HubDataPtr hub, CommandDataPtr cmd) {
	string result = "";
	bool failed = false;

	if(parseCommand(cmd->command, cmd->params, result, failed)) {
		if(!failed) {
			Bool thirdPerson = False;
			if(strnicmp(result.c_str(), "/me ", 4) == 0) {
				thirdPerson = True;
				result = result.substr(4);
			}
			Hubs::handle()->send_message(hub, result.c_str(), thirdPerson);
		} else {
			Hubs::handle()->local_message(hub, result.c_str(), MSG_SYSTEM);
		}
		return true;
	} else {
		return false;
	}


}

bool Plugin::onChatCommandPM(UserDataPtr user, CommandDataPtr cmd) {
	string result = "";
	bool failed = false;

	if(parseCommand(cmd->command, cmd->params, result, failed)) {
		if(!failed) {
			Bool thirdPerson = False;
			if(strnicmp(result.c_str(), "/me ", 4) == 0) {
				thirdPerson = True;
				result = result.substr(4);
			}
			Hubs::handle()->send_private_message(user, result.c_str(), thirdPerson);
		} else {
			return false; // Will let it be for now until a better solution comes up
		}
		return true;
	} else {
		return false;
	}
}

bool Plugin::parseCommand(const char* cmd, const char* param, string& result, bool& status) {
	if(stricmp(cmd, "help") == 0) {
		if(stricmp(param, "plugins") == 0) {
			result =
				"\t\t\t Help: MediaPlayer plugin \t\t\t\n"
				"\t /np \t\t\t\t Broadcast what's currently playing in \n";
			status = true;
			return true;
		}
	} else if(stricmp(cmd, "np") == 0) {
		result = getSpam(status);
		return true;
	}
	return false;
}

string Plugin::getSpam(bool& status)
{
		GError *error = NULL;
		GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;
		if(proxy == NULL){

		proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                         flags,
                                         NULL, /* GDBusInterfaceInfo */
                                         "org.mpris.guayadeque",
                                         "/Player",
                                         "org.freedesktop.MediaPlayer",
                                         NULL, /* GCancellable */
                                         &error);

			if (proxy == NULL || error != NULL)
			{
				g_printerr ("Error creating proxy: %s\n", error->message);
				status = true;
				g_error_free (error);
				return error->message;
			}
		}
				GVariant *var = NULL;
				var = g_dbus_proxy_call_sync (proxy,
                   "GetMetadata",
                   NULL,
                   G_DBUS_CALL_FLAGS_NONE,
                   -1,
                   NULL,
                   &error);

			if( (var == NULL) && error != NULL)
			{
				g_printerr ("Error call on proxy: %s\n", error->message);
				g_error_free (error);
				status = true;
				return error->message;
			}
			//string format = Config::getConfig("MediaPlayerFormat");
			string format = "playing %[artist] - %[album] - %[title] - %[genre]";
			 printf("\n%s\n", format.c_str());
			string pr = parse(var,format);
			g_variant_unref(var);
		return (pr);
}

string Plugin::parse(GVariant *var,string suffix)
{
	StringMap params;

	if (g_variant_n_children (var) > 0)
	{
		 GVariantIter *iter;
		 const gchar *key;
		 GVariant *value;

		 g_variant_get (var,
                     "(a{sv})",
                     &iter);
		while (g_variant_iter_loop (iter, "{&sv}", &key, &value))
        {
		  gchar *value_str = NULL;
          value_str = g_variant_print (value, TRUE);
          g_print ("  %s -> %s\n", key, value_str);
		  string tmp(value_str);
		  string tmp2 = tmp.substr(1,tmp.length()-2);
		  params[key] = (key[0] == 'y') ? tmp : tmp2;
          g_free (value_str);
        }
      g_variant_iter_free (iter);
	}
   return formatParams(suffix,params);
}

