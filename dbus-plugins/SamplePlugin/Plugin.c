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
#endif

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <string>
#include <map>
#include <vector>

#define stricmp strcasecmp
#define strnicmp strncasecmp

/* Variables */
DCCorePtr dcpp;
subsHandle hooks[2];
GDBusProxy *proxy = NULL;

using namespace std;

typedef std::map<std::string, std::string> StringMap;
typedef StringMap::const_iterator StringMapIter;
typedef std::vector<std::string> StringList;
typedef StringList::const_iterator StringIter;

Bool onLoad(uint32_t eventId, DCCorePtr core) {
	dcpp = core;
	if(eventId == ON_INSTALL) {
		/* Default settings */
		set_cfg("MediaPlayerFormat", "playing %[artist] - %[album] - %[title]");
	}

	hooks[0] = dcpp->set_hook(HOOK_CHAT, &pluginProc, NULL);
	//hooks[1] = dcpp->set_hook(HOOK_HUBS, &pluginProc, NULL);
	
	return True;
}

Bool onUnload(uint32_t eventId) {
	//if(hooks[1]) dcpp->un_hook(hooks[1]);
	if(hooks[0]) dcpp->un_hook(hooks[0]);
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
   const char *format = get_cfg("MediaPlayerFormat");
   string f(format);
   return const_cast<gchar *>(formatParams(f,params).c_str());
}

Bool onHubEnter(dcptr_t client, const char* message) {
	if(message[0] == '/') {
		char* cmd = NULL;
		char* param = NULL;
		char* delim = strchr((char*)message, ' ');
		size_t len = (delim != NULL) ? strlen(delim) : 0;
		if(len > 1) {
			cmd = (char*)memset(malloc(delim - message), 0, delim - message);
			cmd = strncpy(cmd, &message[1], (delim - message) - 1);
			param = (char*)memset(malloc(len), 0, len);
			param = strcpy(param, &delim[1]);
		} else {
			len = strlen(message);
			cmd = (char*)memset(malloc(len), 0, len);
			cmd = strcpy(cmd, &message[1]);
		}

		if(stricmp(cmd, "help") == 0 && (param && stricmp(param, "plugins") == 0)) {
			const char* help =
				"\t\t\t Help: SamplePlugin \t\t\t\n"
				"\t /pluginhelp \t\t\t Prints info about the purpose of this plugin\n"
				"\t /plugininfo \t\t\t Prints info about the sample plugin\n"
				"\t /unhook <type> \t\t Hooks test\n"
				"\t /rehook <type> \t\t\t Hooks test\n"
				"\t /msgreg \t\t\t Message register test\n"
				"\t /send <text> \t\t\t Chat message test\n"
				"\t /np \t\t\t Now Playing with formating";

			BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, help);
			return True;
		} else if(stricmp(cmd, "pluginhelp") == 0) {
			const char* pluginhelp =
				"\t\t\t Plugin Help: DBus-like Media Plugin \t\t\t\n"
				"\t The sample plugin project is intended to both demostrate the use and test the implementation of the API\n"
				"\t as such the plugin itself does nothing useful but it can be used to verify whether an implementation works\n"
				"\t with the API or not, however, it is by no means intended to be a comprehensive testing tool for the API.\n";

			BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, pluginhelp);
			return True;
		} else if(stricmp(cmd, "plugininfo") == 0) {
			const char* info =
				"\t\t\t Plugin Info: DBus-Like Plugin \t\t\t\n"
				"\t Name: \t\t\t\t" PLUGIN_NAME "\n"
				"\t Author: \t\t\t" PLUGIN_AUTHOR "\n"
				"\t Version: \t\t\t" STRINGIZE(PLUGIN_VERSION) "\n"
				"\t Description: \t\t\t" PLUGIN_DESC "\n"
				"\t GUID/UUID: \t\t\t" PLUGIN_GUID "\n";

			BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, info);
			return True;
		} else if(stricmp(cmd, "msgreg") == 0) {
			char result[256];

			/* Test values */
			const uint32_t testMsg = dcpp->register_message(HOOK_EVENT, "testMsg");
			const uint32_t testRange = dcpp->register_range(HOOK_EVENT, "testRange", 5);
			const uint32_t testMsg2 = dcpp->register_message(HOOK_EVENT, "testMsg2");

			memset(&result, 0, sizeof(result));
			if(testMsg != dcpp->seek_message("testMsg")) {
				snprintf(result, sizeof(result), "Result: FAILED! (Test 1, testMsg: %u, testRange: %u, testMsg2: %u)", testMsg, testRange, testMsg2);
				BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, result);
				return True;
			} else if(testRange != dcpp->seek_message("testRange")) {
				snprintf(result, sizeof(result), "Result: FAILED! (Test 2, testMsg: %u, testRange: %u, testMsg2: %u)", testMsg, testRange, testMsg2);
				BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, result);
				return True;
			} else if(testMsg2 != (testRange + 5)) {
				snprintf(result, sizeof(result), "Result: FAILED! (Test 3, testMsg: %u, testRange: %u, testMsg2: %u)", testMsg, testRange, testMsg2);
				BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, result);
				return True;
			}

			snprintf(result, sizeof(result), "Result: OK! (testMsg: %u, testRange: %u, testMsg2: %u)", testMsg, testRange, testMsg2);
			BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, result);
			return True;
		} else if(stricmp(cmd, "send") == 0) {
			if(param != NULL) {
				const char* suffix = get_cfg("SendSuffix");
				size_t msgLen = len + strlen(suffix) + 1;
				char* text = (char*)memset(malloc(msgLen), 0, msgLen);

				strcat(text, param);
				text[len-1] = ' ';
				strcat(text, suffix);

				BASE_HUB_SEND_CHAT(dcpp, client, text, (strnicmp(text, "/me ", 4) == 0) ? True : False);
				free(text);
			} else {
				BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, "You must supply a parameter!");
			}
			return True;
		} else if (stricmp(cmd, "np") == 0)
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
				BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, error->message);
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
				BASE_HUB_SEND_LOCAL(dcpp, MSG_SYSTEM, client, error->message);
				g_error_free (error);
				return True;
			}
			gchar *pr = parse(var);
			
			BASE_HUB_SEND_CHAT(dcpp, client, pr, (strnicmp(pr, "/me ", 4) == 0) ? True : False);
			g_variant_unref(var);
		return True;	
		}

		free(param);
		free(cmd);
	}
	return False;
}

#ifdef _WIN32
/* Config dialog stuff */
BOOL onConfigInit(HWND hWnd) {
	char* value = (char*)get_cfg("SendSuffix");
	size_t len = strlen(value) + 1;
	TCHAR* buf = (TCHAR*)memset(malloc(len * sizeof(TCHAR)), 0, len * sizeof(TCHAR));
	dcpp->strconv(CONV_UTF8_TO_WIDE, buf, value, len);

	SetDlgItemText(hWnd, IDC_SUFFIX, buf);
	SetWindowText(hWnd, _T(PLUGIN_NAME) _T(" Settings"));

	free(buf);
	return TRUE;
}

BOOL onConfigClose(HWND hWnd, UINT wID) {
	if(wID == IDOK) {
		int len = GetWindowTextLength(GetDlgItem(hWnd, IDC_SUFFIX)) + 1;
		TCHAR* wbuf = (TCHAR*)memset(malloc(len * sizeof(TCHAR)), 0, len * sizeof(TCHAR));
		char* abuf = (char*)memset(malloc(len), 0, len);

		GetWindowText(GetDlgItem(hWnd, IDC_SUFFIX), wbuf, len);
		dcpp->strconv(CONV_WIDE_TO_UTF8, abuf, wbuf, len);
		set_cfg("SendSuffix", abuf);

		free(abuf);
		free(wbuf);
	}

	EndDialog(hWnd, wID);
	return FALSE;
}

BOOL CALLBACK configProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
#ifndef _WIN32
Bool onConfig(GtkWidget *widget) {
	GtkDialog *dialog =  GTK_DIALOG(gtk_dialog_new_with_buttons ("Setting for a Plugin",
                                         GTK_WINDOW(widget),
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
   gtk_entry_set_text(GTK_ENTRY(entry) , get_cfg("MediaPlayerFormat"));                               
    gint response  = gtk_dialog_run(dialog);
   
    if(response == GTK_RESPONSE_OK)
    {
		const gchar *format = gtk_entry_get_text(GTK_ENTRY(entry));
		set_cfg("MediaPlayerFormat", format);	
		
	}
    gtk_widget_destroy(GTK_WIDGET(dialog));                              
	return True;
}
#endif

/* Settings helpers */
const char* DCAPI get_cfg(const char* name) {
	ConfigValue val;
	memset(&val, 0, sizeof(ConfigValue));

	val.type = CFG_TYPE_STRING;
	dcpp->get_cfg(PLUGIN_GUID, name, &val);
	return val.value.str;
}

int32_t DCAPI get_cfg_int32(const char* name) {
	ConfigValue val;
	memset(&val, 0, sizeof(ConfigValue));

	val.type = CFG_TYPE_INT;
	dcpp->get_cfg(PLUGIN_GUID, name, &val);
	return val.value.int32;
}

int64_t DCAPI get_cfg_int64(const char* name) {
	ConfigValue val;
	memset(&val, 0, sizeof(ConfigValue));

	val.type = CFG_TYPE_INT64;
	dcpp->get_cfg(PLUGIN_GUID, name, &val);
	return val.value.int64;
}

void DCAPI set_cfg(const char* name, const char* value) {
	ConfigValue val;
	memset(&val, 0, sizeof(ConfigValue));

	val.type = CFG_TYPE_STRING;
	val.value.str = value;
	dcpp->set_cfg(PLUGIN_GUID, name, &val);
}

void DCAPI set_cfg_int32(const char* name, int32_t value) {
	ConfigValue val;
	memset(&val, 0, sizeof(ConfigValue));

	val.type = CFG_TYPE_INT;
	val.value.int32 = value;
	dcpp->set_cfg(PLUGIN_GUID, name, &val);
}

void DCAPI set_cfg_int64(const char* name, int64_t value) {
	ConfigValue val;
	memset(&val, 0, sizeof(ConfigValue));

	val.type = CFG_TYPE_INT;
	val.value.int64 = value;
	dcpp->set_cfg(PLUGIN_GUID, name, &val);
}

/* Plugin event processing (listeners could have own functions) */
Bool DCAPI pluginProc(uint32_t eventId, dcptr_t pData) {
	switch(eventId) {
		case ON_INSTALL:
		case ON_LOAD:
			return onLoad(eventId, (DCCorePtr)pData);
		case ON_UNINSTALL:
		case ON_UNLOAD:
			return onUnload(eventId);
		case HUB_ONLINE:
			return False;
		case CHAT_OUT:
		{
			ClientDataPtr cmd = (ClientDataPtr)pData;
			return onHubEnter(cmd->object, cmd->data);
		}
		case ON_CONFIGURE:
			return onConfig((GtkWidget*)pData);
		default: return False;
	}
}
