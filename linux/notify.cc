/*
 * Copyright © 2009-2018 Leliksan Floyd <leliksan@Quadrafon2>
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/


#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "settingsmanager.hh"
#include "../dcpp/Text.h"
#include "notify.hh"

using namespace std;
using namespace dcpp;

Notify *Notify::notify = NULL;

void Notify::start()
{
	dcassert(!notify);
	notify = new Notify();
}

void Notify::stop()
{
	dcassert(notify);
	delete notify;
}

Notify* Notify::get()
{
	dcassert(notify);
	return notify;
}

void Notify::init()
{
    application = WulforManager::get()->getApplication();
    
    static GActionEntry actions[] = {
    { .name = "launch", .activate = onAction, .parameter_type = "s", .state = NULL , .change_state = NULL , .padding = {0,0,0} }
    };

    g_action_map_add_action_entries (G_ACTION_MAP (application),
                                 actions, G_N_ELEMENTS (actions),
                                 application);
}

void Notify::finalize() 
{

}
/*
void Notify::setCurrIconSize(const int size)
{
	currIconSize = size;

	switch (size)
	{
		case x16:
			icon_width = icon_height = 16; // 16x16
			break;

		case x22:
			icon_width = icon_height = 22; // 22x22
			break;

		case x24:
			icon_width = icon_height = 24; // 24x24
			break;

		case x32:
			icon_width = icon_height = 32; // 32x32
			break;

		case x36:
			icon_width = icon_height = 36; // 36x36
			break;

		case x48:
			icon_width = icon_height = 48; // 48x48
			break;

		case x64:
			icon_width = icon_height = 64; // 64x64
			break;

		case DEFAULT:
			currIconSize = DEFAULT;
			break;

		default:
			currIconSize = DEFAULT;
			WSET("notify-icon-size", DEFAULT);
	}
}
*/
void Notify::showNotify(const string head, const string body, TypeNotify notify)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	switch (notify)
	{
		case DOWNLOAD_FINISHED:

			if (wsm->getInt("notify-download-finished-use"))
			{

				bAction = true;//set action
				showNotify(wsm->getString("notify-download-finished-title"), head, body,
					wsm->getString("notify-download-finished-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_NORMAL);
			}

			break;

		case DOWNLOAD_FINISHED_USER_LIST:

			if (wsm->getInt("notify-download-finished-ul-use"))
			showNotify(wsm->getString("notify-download-finished-ul-title"), head, body,
				wsm->getString("notify-download-finished-ul-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_LOW);
			break;

		case PRIVATE_MESSAGE:

			if (wsm->getInt("notify-private-message-use"))
			showNotify(wsm->getString("notify-private-message-title"), head, body,
				wsm->getString("notify-private-message-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_NORMAL);
			break;

		case HUB_CONNECT:

			if (wsm->getInt("notify-hub-connect-use"))
			showNotify(wsm->getString("notify-hub-connect-title"), head, body,
				wsm->getString("notify-hub-connect-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_NORMAL);
			break;

		case HUB_DISCONNECT:

			if (wsm->getInt("notify-hub-disconnect-use"))
			showNotify(wsm->getString("notify-hub-disconnect-title"), head, body,
				wsm->getString("notify-hub-disconnect-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_URGENT);
			break;

		case FAVORITE_USER_JOIN:

			if (wsm->getInt("notify-fuser-join"))
			showNotify(wsm->getString("notify-fuser-join-title"), head, body,
				wsm->getString("notify-fuser-join-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_NORMAL);
			break;

		case FAVORITE_USER_QUIT:

			if (wsm->getInt("notify-fuser-quit"))
			showNotify(wsm->getString("notify-fuser-quit-title"), head, body,
				wsm->getString("notify-fuser-quit-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_NORMAL);
			break;
		case HIGHLITING:
			if (wsm->getInt("notify-high-use"))
				showNotify(wsm->getString("notify-high-title"), head , body,
						wsm->getString("notify-high-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_LOW);
			break;
		case HUB_CHAT:
			if (wsm->getInt("notify-hub-chat-use"))
				showNotify(wsm->getString("notify-hub-chat-title"), head , body,
						wsm->getString("notify-hub-chat-icon"), wsm->getInt("notify-icon-size"), G_NOTIFICATION_PRIORITY_NORMAL);
			break;
		default: break;
	}
}
void Notify::showNotify(const string title, const string head, const string body, const string icon, const int /*iconSize*/, GNotificationPriority urgency)
{

//@ only title is Fatal	
	if(title.empty())
		return;
	
	g_autofree gchar *esc_title = g_markup_escape_text(g_filename_to_utf8(title.c_str(),-1,NULL,NULL,NULL), -1);
	g_autofree gchar *esc_body = g_markup_escape_text(g_filename_to_utf8(Util::getFileName(body).c_str(),-1,NULL,NULL,NULL), -1);
	string message = head + esc_body;

    GFile* ficon = g_file_new_for_path(icon.c_str());
    GIcon *gicon = g_file_icon_new (ficon);
    GNotification *notification = g_notification_new (esc_title);
	g_notification_set_body (notification, esc_body);
	g_notification_set_icon (notification, gicon);
    if(bAction) {
        g_notification_add_button_with_target (notification, "Open Folder", "app.launch", "s", (string("file:///")+Util::getFilePath(body)).c_str());
        g_notification_add_button_with_target (notification, "Open File", "app.launch", "s", (string("file:///")+body).c_str());
    } 
    g_notification_set_priority (notification,urgency);          
	g_application_send_notification (application, NULL, notification);
    g_object_unref (gicon);
	g_object_unref (notification);
    bAction = false;
}

void Notify::onAction(GSimpleAction*, GVariant* var, gpointer)
{

	string target = g_variant_get_string (var, NULL);

	if (!target.empty())
		WulforUtil::openURI(target);
}


