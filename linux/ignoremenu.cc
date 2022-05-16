/*
 * Copyright © 2020
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

#include "settingsmanager.hh"
#include "GuiUtil.hh"
#include <glib/gi18n.h>
#include "../dcpp/Util.h"
#include "ignoremenu.hh"

#include "IgnoreTempManager.hh"


using namespace std;
using namespace dcpp;

void IgnoreMenu::cleanMenu_gui()
{
//	gtk_container_foreach(GTK_CONTAINER(appsPreviewMenu), (GtkCallback)gtk_widget_destroy, NULL);
}

void IgnoreMenu::buildMenu_gui(const string &nick,const string& cid,const string& ip)
{
	//0 mean disable
	GtkWidget* itemApp,*itemApp1,*itemApp2;
	for(int i = 1;i != 50;++i)
	{
		char buf[512];
		sprintf(buf,"For %d Minutes",i);
		
	//	itemApp = gtk_menu_item_new_with_label(buf);
		
	//	gtk_menu_shell_append(GTK_MENU_SHELL(appsPreviewMenu), itemApp);

		//g_signal_connect(itemApp, "activate", G_CALLBACK(onClicked_gui), (gpointer) this);

		g_object_set_data_full(G_OBJECT(itemApp), "time", g_strdup(Util::toString(i).c_str()), g_free);
		g_object_set_data_full(G_OBJECT(itemApp), "nick", g_strdup(nick.c_str()), g_free);
		
		
		
		//itemApp1 = gtk_menu_item_new_with_label((string(buf)+" CID").c_str());
		
		//gtk_menu_shell_append(GTK_MENU_SHELL(appsPreviewMenu), itemApp1);

		//g_signal_connect(itemApp1, "activate", G_CALLBACK(onClickedCid_gui), (gpointer) this);

		g_object_set_data_full(G_OBJECT(itemApp1), "time", g_strdup(Util::toString(i).c_str()), g_free);
		g_object_set_data_full(G_OBJECT(itemApp1), "cid", g_strdup(cid.c_str()), g_free);	

		
		//itemApp2 = gtk_menu_item_new_with_label((string(buf)+" IP Address").c_str());
		
		//gtk_menu_shell_append(GTK_MENU_SHELL(appsPreviewMenu), itemApp2);

		//g_signal_connect(itemApp2, "activate", G_CALLBACK(onClickedIp_gui), (gpointer) this);

		g_object_set_data_full(G_OBJECT(itemApp2), "time", g_strdup(Util::toString(i).c_str()), g_free);
		g_object_set_data_full(G_OBJECT(itemApp2), "ip", g_strdup(ip.c_str()), g_free);
		
		
	}
}
/*
void IgnoreMenu::onClicked_gui(GtkMenuItem *menuItem, gpointer )
{
	int64_t time = Util::toInt((gchar *) g_object_get_data(G_OBJECT(menuItem), "time"));
	string nick = (gchar *) g_object_get_data(G_OBJECT(menuItem), "nick");
	IgnoreTempManager::getInstance()->addNickIgnored(nick,time);
	
}

void IgnoreMenu::onClickedIp_gui(GtkMenuItem *menuItem, gpointer )
{
	int64_t time = Util::toInt((gchar *) g_object_get_data(G_OBJECT(menuItem), "time"));
	string ip = (gchar *) g_object_get_data(G_OBJECT(menuItem), "ip");
	IgnoreTempManager::getInstance()->addIpIgnored(ip,time);
}

void IgnoreMenu::onClickedCid_gui(GtkMenuItem *menuItem, gpointer )
{
	int64_t time = Util::toInt((gchar *) g_object_get_data(G_OBJECT(menuItem), "time"));
	string cid = (gchar *) g_object_get_data(G_OBJECT(menuItem), "cid");
	IgnoreTempManager::getInstance()->addCidIgnored(cid,time);
}
*/
