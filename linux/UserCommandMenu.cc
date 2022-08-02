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

#include "UserCommandMenu.hh"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/UserCommand.h"
#include "../dcpp/ClientManager.h"
#include "wulformanager.hh"
#include "GuiUtil.hh"

using namespace std;
using namespace dcpp;

static const GActionEntry UserCommandMenu::uc[]
{{"send",onUserCommandClick_gui,NULL,NULL,NULL}};

UserCommandMenu::UserCommandMenu(GMenu *userCommandMenu,GtkWidget* parent , int ctx):
	userCommandMenu(userCommandMenu),
	ctx(ctx)
{
	//@TODO: non-deprecated things
	GSimpleActionGroup* simple = g_simple_action_group_new ();
	g_simple_action_group_add_entries(simple, uc, G_N_ELEMENTS (uc), (gpointer)this);
	gtk_widget_insert_action_group(parent,"uc" ,G_ACTION_GROUP(simple));
	
}

void UserCommandMenu::addHub(const string hub)
{
	hubs.clear();
	hubs.push_back(hub);
}

void UserCommandMenu::addHub(const StringList hubs2)
{
	hubs.clear();
	hubs.insert(hubs.end(), hubs2.begin(), hubs2.end());
}

void UserCommandMenu::addUser(const string cid)
{
	UCParam u;
	u.cid = cid;
	ucParams.push_back(u);
}

void UserCommandMenu::addIp(const string ip)
{
	ips.clear();
	ips.push_back(ip);
}

void UserCommandMenu::addFile(const string cid, const string name,
	const int64_t size, const string tth)
{
	UCParam u;
	u.cid = cid;
	u.name = name;
	u.size = size;
	u.tth = tth;
	if (u.tth.empty())
		u.type = _("Directory");
	else
		u.type = _("File");
	ucParams.push_back(u);
}

void UserCommandMenu::cleanMenu_gui()
{
	hubs.clear();
	ucParams.clear();
	ips.clear();
}

void UserCommandMenu::buildMenu_gui()
{
	UserCommand::List userCommandList = FavoriteManager::getInstance()->getUserCommands(ctx, hubs);

	GMenuItem *menuItem;
	GMenu *menu = userCommandMenu;
	bool separator = false; // tracks whether last menu item was a separator

	for (UserCommand::List::iterator i = userCommandList.begin(); i != userCommandList.end(); ++i)
	{
		UserCommand& uc = *i;

		// Add line separator only if it's not a duplicate
		/*if (uc.getType() == UserCommand::TYPE_SEPARATOR && !separator)
		{
			menuItem = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
			separator = true;
		}
		else*/ if (uc.getType() == UserCommand::TYPE_RAW || uc.getType() == UserCommand::TYPE_RAW_ONCE)
		{
			string command = uc.getName();
			separator = false;
			menu = userCommandMenu;

			// Append the user command to the sub menu
			menuItem = g_menu_item_new(command.c_str() , "uc.send");
			createSubMenu_gui(menuItem, command);
			g_object_set_data_full(G_OBJECT(menuItem), "name", g_strdup(uc.getName().c_str()), g_free);
			g_object_set_data_full(G_OBJECT(menuItem), "command", g_strdup(uc.getCommand().c_str()), g_free);
			g_object_set_data_full(G_OBJECT(menuItem), "hub", g_strdup(uc.getHub().c_str()), g_free);
			g_menu_append_item(menu , menuItem);
		}
	}
}

void UserCommandMenu::createSubMenu_gui(GMenuItem *&menu, string &command)
{
	string::size_type i = 0;
	GMenuItem *menuItem;

	// Create subfolders based on path separators in the command
	while ((i = command.find('/')) != string::npos)
	{
		bool createSubmenu = true;
//		GList *menuItems = gtk_container_get_children(GTK_CONTAINER(menu));

		// Search for the sub menu to append the command to
//		for (GList *iter = menuItems; iter; iter = iter->next)
//		{
//			GtkMenuItem *item = (GtkMenuItem *)iter->data;
//			if (gtk_menu_item_get_submenu(item) && WulforUtil::getTextFromMenu(item) == command.substr(0, i))
//			{
//				menu = gtk_menu_item_get_submenu(item);
//				createSubmenu = false;
//				break;
//			}
//		}
//		g_list_free(menuItems);

		// Couldn't find existing sub menu, so we create one
		if (createSubmenu)
		{
			GMenu *subMenu = g_menu_new();
			menuItem = g_menu_item_new(command.substr(0, i).c_str() ,NULL);
			g_menu_append_item(subMenu , menuItem);
			g_menu_item_set_submenu(menu,G_MENU_MODEL(subMenu));
			
		}

		command = command.substr(++i);
	}
}

void UserCommandMenu::onUserCommandClick_gui(GMenu *item,GVariant*, gpointer data)
{
	UserCommandMenu *ucm = reinterpret_cast<UserCommandMenu *>(data);
	string command = (gchar *)g_object_get_data(G_OBJECT(item), "command");
	ParamMap params;
//	if (MainWindow::getUserCommandLines_gui(command/*uc*//*, params))
	{
		string commandName = (gchar *)g_object_get_data(G_OBJECT(item), "name");
		string hub = (gchar *)g_object_get_data(G_OBJECT(item), "hub");

 		for (vector<UCParam>::iterator i = ucm->ucParams.begin(); i != ucm->ucParams.end(); ++i)
  		{
			if (!i->name.empty() && !i->type.empty())
			{
				params["type"] = i->type;
	 			params["fileFN"] = i->name;
	 			params["fileSI"] = Util::toString(i->size);
	 			params["fileSIshort"] = Util::formatBytes(i->size);
	 			params["fileTR"] = i->tth;
	 			params["fileMN"] = WulforUtil::makeMagnet(i->name, i->size, i->tth);
	 			params["file"] = params["fileFN"];
	 			params["filesize"] = params["fileSI"];
	 			params["filesizeshort"] = params["fileSIshort"];
	 			params["tth"] = params["fileTR"];
			}

			ucm->sendUserCommand_client(
				i->cid, commandName, hub, params);
		}

		for(auto i= ucm->ips.begin(); i!= ucm->ips.end(); ++i)
		{
			string cid = ClientManager::getInstance()->getMe()->getCID().toBase32();
			params["cmdIP"] = *i;
			ucm->sendUserCommand_client(
				cid, commandName, hub, params);
		}
	}
}

void UserCommandMenu::sendUserCommand_client(string cid, string commandName, string hub, ParamMap params)
{
	if (!cid.empty() && !commandName.empty())
	{
		int id = FavoriteManager::getInstance()->findUserCommand(commandName, hub);
		UserCommand uc;

		if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
			return;

		UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hub);
		if (user)
		{
                	ClientManager::getInstance()->userCommand(HintedUser(user, hub), uc, params, true);
		}
	}
}
