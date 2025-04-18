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

#ifndef USER_COMMAND_MENU_HH
#define USER_COMMAND_MENU_HH

#include <gtk/gtk.h>
#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/typedefs.h"
#include "entry.hh"

class UserCommandMenu
{
	public:
		UserCommandMenu(GMenu *userCommandMenu,GtkWidget* parent, int ctx);
		virtual ~UserCommandMenu() {}

		GMenu *getContainer() { return userCommandMenu; }
		void addHub(const std::string hub);
		void addHub(const dcpp::StringList hubs2);
		void addUser(const std::string cid);
		void addFile(const std::string cid, const std::string name,
			const int64_t size, const std::string tth);
		void addIp(const std::string ip);
		void cleanMenu_gui();
		void buildMenu_gui();

	private:
		// GUI functions
		void createSubMenu_gui(GMenuItem *&menu, std::string &command);

		// GUI callbacks
		static void onUserCommandClick_gui(GMenu *item,GVariant*, gpointer data);

		// Client functions
		void sendUserCommand_client(std::string cid, std::string commandName, std::string hub, dcpp::ParamMap params);

		GMenu *userCommandMenu;
		int ctx;
		dcpp::StringList hubs;
 		struct UCParam
		{
 			std::string cid;
 			std::string name;
 			int64_t size;
 			std::string tth;
 			std::string type;
 		};
 		std::vector<UCParam> ucParams;
 		std::vector<std::string> ips;

 		static const GActionEntry uc[];

};

#else
class UserCommandMenu;
#endif
