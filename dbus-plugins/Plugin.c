/*
 * Copyright (C) 2012 Jacek Sieka, arnetheduck on gmail point com
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

/* Variables */
DCCorePtr dcpp;

DCHooksPtr hooks;
DCConfigPtr config;
DCLogPtr logging;

DCHubPtr hub;
DCUtilsPtr utils = NULL;
GDBusProxy *proxy = NULL;

using namespace std;

typedef std::map<std::string, std::string> StringMap;
typedef StringMap::const_iterator StringMapIter;
typedef std::vector<std::string> StringList;
typedef StringList::const_iterator StringIter;

/* Hook subscription store */
#define HOOKS_SUBSCRIBED 2

const char* hookGuids[HOOKS_SUBSCRIBED] = {
	HOOK_UI_PROCESS_CHAT_CMD,
	HOOK_HUB_ONLINE
};

DCHOOK hookFuncs[HOOKS_SUBSCRIBED] = {
	&onHubEnter,
	&onHubOnline
};

subsHandle subs[HOOKS_SUBSCRIBED];

Bool onLoad(uint32_t eventId, DCCorePtr core) {
	uint32_t i = 0;
	dcpp = core;

	hooks = (DCHooksPtr)core->query_interface(DCINTF_HOOKS, DCINTF_HOOKS_VER);
	config = (DCConfigPtr)core->query_interface(DCINTF_CONFIG, DCINTF_CONFIG_VER);
	logging = (DCLogPtr)core->query_interface(DCINTF_LOGGING, DCINTF_LOGGING_VER);

	hub = (DCHubPtr)core->query_interface(DCINTF_DCPP_HUBS, DCINTF_DCPP_HUBS_VER);
#ifdef _WIN32
	utils = (DCUtilsPtr)core->query_interface(DCINTF_DCPP_UTILS, DCINTF_DCPP_UTILS_VER);
#endif

	if(eventId == ON_INSTALL) {
		/* Default settings */
		set_cfg("SendSuffix", "<DC++ Plugins Test>");
		set_cfg("MediaPlayerFormat", "playing %[artist] - %[album] - %[title]");
	}

	while(i < HOOKS_SUBSCRIBED) {
		subs[i] = hooks->bind_hook(hookGuids[i], hookFuncs[i], NULL);
		++i;
	}

	return True;
}

Bool onUnload() {
	uint32_t i = 0;
	while(i < HOOKS_SUBSCRIBED) {
		if(subs[i]) hooks->release_hook(subs[i]);
		++i;
	}
	return True;
}

string fixedftime(const string& format, struct tm* t) {
	string ret = format;
	const char codes[] = "aAbBcdHIjmMpSUwWxXyYzZ%";

	char tmp[4];
	tmp[0] = '%';
	tmp[1] = tmp[2] = tmp[3] = 0;

	StringMap sm;
	static const size_t BUF_SIZE = 1024;
	string buf;
	buf.resize(BUF_SIZE);
	for(size_t i = 0; i < strlen(codes); ++i) {
		tmp[1] = codes[i];
		tmp[2] = 0;
		strftime(&buf[0], BUF_SIZE-1, tmp, t);
		sm[tmp] = &buf[0];

		tmp[1] = '#';
		tmp[2] = codes[i];
		strftime(&buf[0], BUF_SIZE-1, tmp, t);
		sm[tmp] = &buf[0]; 
	}

	for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
		for(string::size_type j = ret.find(i->first); j != string::npos; j = ret.find(i->first, j)) {
			ret.replace(j, i->first.length(), i->second);
			j += i->second.length() - i->first.length();
		}
	}

	return ret;
}

string formatTime(const string &msg, const time_t t) {
	if (!msg.empty()) {
		size_t bufsize = msg.size() + 256;
		struct tm* loc = localtime(&t);

		if(!loc) {
			return "";
		}
		string buf(bufsize, 0);
		buf.resize(strftime(&buf[0], buf.size()-1, msg.c_str(), loc));

		if(buf.empty()) {
			return fixedftime(msg, loc);
		}

		return buf;
	}
	return "";
}

string formatParams(const string& format, StringMap& params) {
	string result = format;
	string::size_type i, j, k;
	i = 0;
	while (( j = result.find("%[", i)) != string::npos) {
		if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
			break;
		}
		string name = result.substr(j + 2, k - j - 2);
		StringMapIter smi = params.find(name);
		if(smi == params.end()) {
			result.erase(j, k-j + 1);
			i = j;
		} else {
			if(smi->second.find_first_of("%\\./") != string::npos) {
				string tmp = smi->second;	// replace all % in params with %% for strftime
				string::size_type m = 0;
				while(( m = tmp.find('%', m)) != string::npos) {
					tmp.replace(m, 1, "%%");
					m+=2;
				}

				result.replace(j, k-j + 1, tmp);
				i = j + tmp.size();
			} else {
				result.replace(j, k-j + 1, smi->second);
				i = j + smi->second.size();
			}
		}
	}

	result = formatTime(result, time(NULL));
	return result;
}

static gchar *parse(GVariant *var)
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
		  gchar *value_str;
          value_str = g_variant_print (value, TRUE);
          g_print ("      %s -> %s\n", key, value_str);
		  string tmp(value_str);
		  string tmp2 = tmp.substr(1,tmp.length()-2);
		  params[key] = tmp2; //value_str;
          g_free (value_str);
        }
      g_variant_iter_free (iter);
	}
   const char *format = get_cfg("MediaPlayerFormat")->value;
   string f(format);
   return const_cast<gchar *>(formatParams(f,params).c_str());
}

/* Event handlers */
Bool DCAPI onHubEnter(dcptr_t pObject, dcptr_t pData, Bool* bBreak) {
	HubDataPtr hHub = (HubDataPtr)pObject;
	CommandDataPtr cmd = (CommandDataPtr)pData;

	if(cmd->isPrivate)
		return False;

	if(stricmp(cmd->command, "help") == 0 && stricmp(cmd->params, "plugins") == 0) {
		const char* help =
			"\t\t\t Help: SamplePlugin \t\t\t\n"
			"\t /pluginhelp \t\t\t Prints info about the purpose of this plugin\n"
			"\t /plugininfo \t\t\t Prints info about the sample plugin\n"
			"\t /unhook <index> \t\t Hooks test\n"
			"\t /rehook <index> \t\t Hooks test\n"
			"\t /send <text> \t\t\t Chat message test\n"
			"\t /np \t\t\t Now Playing with formating";

		hub->local_message(hHub, help, MSG_SYSTEM);
		return True;
	} else if(stricmp(cmd->command, "pluginhelp") == 0) {
		const char* pluginhelp =
			"\t\t\t Plugin Help: SamplePlugin \t\t\t\n"
			"\t The sample plugin project is intended to both demostrate the use and test the implementation of the API\n"
			"\t as such the plugin itself does nothing useful but it can be used to verify whether an implementation works\n"
			"\t with the API or not, however, it is by no means intended to be a comprehensive testing tool for the API.\n";

		hub->local_message(hHub, pluginhelp, MSG_SYSTEM);
		return True;
	} else if(stricmp(cmd->command, "plugininfo") == 0) {
		const char* info =
			"\t\t\t Plugin Info: SamplePlugin \t\t\t\n"
			"\t Name: \t\t\t\t" PLUGIN_NAME "\n"
			"\t Author: \t\t\t" PLUGIN_AUTHOR "\n"
			"\t Version: \t\t\t" STRINGIZE(PLUGIN_VERSION) "\n"
			"\t Description: \t\t\t" PLUGIN_DESC "\n"
			"\t GUID/UUID: \t\t\t" PLUGIN_GUID "\n";

		hub->local_message(hHub, info, MSG_SYSTEM);
		return True;
	} else if(stricmp(cmd->command, "unhook") == 0) {
		/* Unhook test */
		if(strlen(cmd->params) == 0) {
			hub->local_message(hHub, "You must supply a parameter!", MSG_SYSTEM);
		} else {
			uint32_t subIdx = atoi(cmd->params);
			if((subIdx < HOOKS_SUBSCRIBED) && subs[subIdx]) {
				hooks->release_hook(subs[subIdx]);
				subs[subIdx] = 0;
				hub->local_message(hHub, "Hooking changed...", MSG_SYSTEM);
			}
		}
		return True;
	} else if(stricmp(cmd->command, "rehook") == 0) {
		/* Rehook test */
		if(strlen(cmd->params) == 0) {
			hub->local_message(hHub, "You must supply a parameter!", MSG_SYSTEM);
		} else {
			uint32_t subIdx = atoi(cmd->params);
			if((subIdx < HOOKS_SUBSCRIBED) && !subs[subIdx]) {
				subs[subIdx] = hooks->bind_hook(hookGuids[subIdx], hookFuncs[subIdx], NULL);
				hub->local_message(hHub, "Hooking changed...", MSG_SYSTEM);
			}
		}
		return True;
	} else if(stricmp(cmd->command, "send") == 0) {
		size_t len = strlen(cmd->params);
		if(len > 0) {
			ConfigStrPtr suffix = get_cfg("SendSuffix");
			size_t msgLen = len + strlen(suffix->value) + 2;
			char* text = (char*)memset(malloc(msgLen), 0, msgLen);

			strcat(text, cmd->params);
			text[len] = ' ';
			strcat(text, suffix->value);

			hub->send_message(hHub, text, (strnicmp(text, "/me ", 4) == 0) ? True : False);

			free(text);
			config->release((ConfigValuePtr)suffix);
		} else {
			hub->local_message(hHub, "You must supply a parameter!", MSG_SYSTEM);
		}
		return True;
	}
	else if (stricmp(cmd->command, "np") == 0)
	{
			GError *error = NULL;
			GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;
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
				hub->local_message(hHub,error->message, MSG_SYSTEM);
				g_error_free (error);
				return True;
			}
				GVariant *var = NULL;
				error = NULL;
  
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
				hub->local_message(hHub,error->message, MSG_SYSTEM);
				g_error_free (error);
				return True;
			}
			gchar *pr = parse(var);
			
			hub->send_message(hHub, pr, (strnicmp(pr, "/me ", 4) == 0) ? True : False);
			g_variant_unref(var);
		return True;	
	}
	return False;
}

Bool DCAPI onHubOnline(dcptr_t pObject, dcptr_t pData, Bool* bBreak) {
	HubDataPtr hHub = (HubDataPtr)pObject;

	char* buf = (char*)memset(malloc(256), 0, 256);
	snprintf(buf, 256, "*** %s connected! (%s)", hHub->url, (hHub->protocol == PROTOCOL_ADC ? "adc" : "nmdc"));

	logging->log(buf);
	free(buf);

	return False;
}

#ifdef _WIN32
/* Config dialog stuff */
BOOL onConfigInit(HWND hWnd) {
	ConfigStrPtr value = get_cfg("SendSuffix");
	size_t len = strlen(value->value) + 1;
	TCHAR* buf = (TCHAR*)memset(malloc(len * sizeof(TCHAR)), 0, len * sizeof(TCHAR));

	utils->utf8_to_wcs(buf, value->value, len);
	config->release((ConfigValuePtr)value);
	value = NULL;

	SetDlgItemText(hWnd, IDC_SUFFIX, buf);
	SetWindowText(hWnd, _T(PLUGIN_NAME) _T(" Settings"));

	free(buf);
	return TRUE;
}

BOOL onConfigClose(HWND hWnd, UINT wID) {
	if(wID == IDOK) {
		int len = GetWindowTextLength(GetDlgItem(hWnd, IDC_SUFFIX)) + 1;
		TCHAR* wbuf = (TCHAR*)memset(malloc(len * sizeof(TCHAR)), 0, len * sizeof(TCHAR));
		char* value = (char*)memset(malloc(len), 0, len);

		GetWindowText(GetDlgItem(hWnd, IDC_SUFFIX), wbuf, len);
		utils->wcs_to_utf8(value, wbuf, len);
		set_cfg("SendSuffix", value);

		free(value);
		free(wbuf);
	}

	EndDialog(hWnd, wID);
	return FALSE;
}

BOOL CALLBACK configProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch(uMsg) {
		case WM_INITDIALOG:
			return onConfigInit(hWnd);
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
				case IDCLOSE:
					return onConfigClose(hWnd, LOWORD(wParam));
			}
		}
	}
	return FALSE;
}
#endif
#ifdef _WIN32
Bool onConfig(dcptr_t hWnd) {

	DialogBox(hInst, MAKEINTRESOURCE(IDD_PLUGINDLG), (HWND)hWnd, (DLGPROC)&configProc);
	return True;
}
#endif
#ifndef _WIN32
Bool onConfig(dcptr_t widget) {
	GtkDialog *dialog =  GTK_DIALOG(gtk_dialog_new_with_buttons ("Setting for a Plugin",
                                         GTK_WINDOW((GtkWidget*)widget),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_CANCEL,
                                         NULL));
   GtkWidget *content_area = gtk_dialog_get_content_area (dialog);                              
   GtkWidget *entry = gtk_entry_new();
   GtkWidget *label = gtk_label_new("Format string: ");
   gtk_container_add(GTK_CONTAINER(content_area),label);
   gtk_container_add(GTK_CONTAINER(content_area), entry);
   gtk_widget_show(entry); 
   gtk_widget_show(label);
   gtk_entry_set_text(GTK_ENTRY(entry) , get_cfg("MediaPlayerFormat")->value);                               
    gint response  = gtk_dialog_run(dialog);
   
    if(response == GTK_RESPONSE_OK)
    {
		const gchar *format = gtk_entry_get_text(GTK_ENTRY(entry));
		set_cfg("MediaPlayerFormat", (char*)format);	
		
	}
    gtk_widget_destroy(GTK_WIDGET(dialog));                              
	return True;
}
#endif

/* Settings helpers */
ConfigStrPtr DCAPI get_cfg(const char* name) {
	return (ConfigStrPtr)config->get_cfg(PLUGIN_GUID, name, CFG_TYPE_STRING);
}

ConfigIntPtr DCAPI get_cfg_int(const char* name) {
	return (ConfigIntPtr)config->get_cfg(PLUGIN_GUID, name, CFG_TYPE_INT);
}

ConfigInt64Ptr DCAPI get_cfg_int64(const char* name) {
	return (ConfigInt64Ptr)config->get_cfg(PLUGIN_GUID, name, CFG_TYPE_INT64);
}

void DCAPI set_cfg(const char* name, const char* value) {
	ConfigStr val;
	memset(&val, 0, sizeof(ConfigStr));

	val.type = CFG_TYPE_STRING;
	val.value = value;
	config->set_cfg(PLUGIN_GUID, name, (ConfigValuePtr)&val);
}

void DCAPI set_cfg_int(const char* name, int32_t value) {
	ConfigInt val;
	memset(&val, 0, sizeof(ConfigInt));

	val.type = CFG_TYPE_INT;
	val.value = value;
	config->set_cfg(PLUGIN_GUID, name, (ConfigValuePtr)&val);
}

void DCAPI set_cfg_int64(const char* name, int64_t value) {
	ConfigInt64 val;
	memset(&val, 0, sizeof(ConfigInt64));

	val.type = CFG_TYPE_INT64;
	val.value = value;
	config->set_cfg(PLUGIN_GUID, name, (ConfigValuePtr)&val);
}

/* Plugin main function */
Bool DCAPI pluginMain(PluginState state, DCCorePtr core, dcptr_t pData) {
	switch(state) {
		case ON_INSTALL:
		case ON_LOAD:
			return onLoad(state, core);
		case ON_UNINSTALL:
		case ON_UNLOAD:
			return onUnload();
		case ON_CONFIGURE:
			return onConfig(pData);
		default: return False;
	}
}
