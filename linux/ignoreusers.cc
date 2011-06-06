//      ignoreusers.cpp
//
//      Copyright 2009 Mank <mank@no-ip.sk>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include "ignoreusers.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "privatemessage.hh"
#include "hub.hh"

#include <dcpp/FavoriteManager.h>
#include <dcpp/ClientManager.h>
#include <dcpp/FavoriteUser.h>

#include "settingsmanager.hh"

using namespace std;
using namespace dcpp;

ignoreusers::ignoreusers():
BookEntry(Entry::IGNORE_USERS,_("Ignore Users"),"ignoreeusers.glade")
{
		// Configure the dialog
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("DescriptionDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

		// Initialize favorite users list treeview
		favoriteUserView.setView(GTK_TREE_VIEW(getWidget("favoriteUserView")), TRUE, "favoriteusers");
		favoriteUserView.insertColumn(N_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING, 100, "Icon");
		favoriteUserView.insertColumn(N_("Hub (last seen in, if offline)"), G_TYPE_STRING, TreeView::STRING, 200);
		favoriteUserView.insertColumn(N_("Time last seen"), G_TYPE_STRING, TreeView::STRING, 120);
		favoriteUserView.insertColumn(N_("Description"), G_TYPE_STRING, TreeView::STRING, 100);
		favoriteUserView.insertColumn("CID", G_TYPE_STRING, TreeView::STRING, 350);
		favoriteUserView.insertHiddenColumn("URL", G_TYPE_STRING);
		favoriteUserView.insertHiddenColumn("Icon", G_TYPE_STRING);
		favoriteUserView.finalize();
		favoriteUserStore = gtk_list_store_newv(favoriteUserView.getColCount(), favoriteUserView.getGTypes());
		gtk_tree_view_set_model(favoriteUserView.get(), GTK_TREE_MODEL(favoriteUserStore));
		g_object_unref(favoriteUserStore);
		gtk_tree_view_set_fixed_height_mode(favoriteUserView.get(), TRUE);
		favoriteUserSelection = gtk_tree_view_get_selection(favoriteUserView.get());
		
		// Treat "Nick" as the default col instead of "Auto Connect"
		gtk_tree_view_set_search_column(favoriteUserView.get(), favoriteUserView.col(N_("Nick")));
		GtkTreeViewColumn *column = gtk_tree_view_get_column(favoriteUserView.get(), favoriteUserView.col(N_("Nick")));
		gtk_widget_grab_focus(column->button);

		/*conect to signals*/

		g_signal_connect(getWidget("browseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueItemClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("sendPMItem"), "activate", G_CALLBACK(onSendPMItemClicked_gui), (gpointer)this);

		g_signal_connect(getWidget("connectItem"), "activate", G_CALLBACK(onConnectItemClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("removeFromQueueItem"), "activate", G_CALLBACK(onRemoveFromQueueItemClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("descriptionItem"), "activate", G_CALLBACK(onDescriptionItemClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItemClicked_gui), (gpointer)this);
		g_signal_connect(favoriteUserView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
		g_signal_connect(favoriteUserView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
		g_signal_connect(favoriteUserView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

}

ignoreusers::~ignoreusers()
{
	FavoriteManager::getInstance()->save();
	FavoriteManager::getInstance()->removeListener(this);
}

void ignoreusers::show()
{
	FavoriteManager::IgnoredMap map = FavoriteManager::getInstance()->getIgnoredUsers();
	FavoriteManager::IgnoredMap::const_iterator it;

	for (it = map.begin(); it != map.end(); ++it)
	{
		GtkTreeIter iter;
		const FavoriteUser &user= it->second;
		bool online = user.getUser()->isOnline();
		string hub = online ? WulforUtil::getHubNames(user.getUser(), user.getUrl()) : user.getUrl();//NOTE: core 0.762
		string seen = online ? N_("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen());
		string cid = user.getUser()->getCID().toBase32();

		gtk_list_store_append(favoriteUserStore, &iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(N_("Nick")), user.getNick().c_str(),
			favoriteUserView.col(N_("Hub (last seen in, if offline)")), hub.c_str(),
			favoriteUserView.col(N_("Time last seen")), seen.c_str(),
			favoriteUserView.col(N_("Description")), user.getDescription().c_str(),
			favoriteUserView.col("CID"), cid.c_str(),
			favoriteUserView.col("URL"), user.getUrl().c_str(),
			favoriteUserView.col("Icon"), "bmdc-normal",
			-1);

		userIters.insert(UserIters::value_type(cid, iter));
	}
	FavoriteManager::getInstance()->addListener(this);
}

gboolean ignoreusers::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			fu->onRemoveItemClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(fu->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean ignoreusers::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
ignoreusers *fu = (ignoreusers *)data;
	fu->previous = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;

		if (gtk_tree_view_get_path_at_pos(fu->favoriteUserView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(fu->favoriteUserSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean ignoreusers::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		if (event->button == 1 && fu->previous == GDK_2BUTTON_PRESS)
		{
			if (WGETB("pm"))

				fu->onSendPMItemClicked_gui(NULL, data);
			else
				fu->onBrowseItemClicked_gui(NULL, data);
		}
		else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
		{
			if (WGETB("pm"))

				fu->onBrowseItemClicked_gui(NULL, data);
			else
				fu->onSendPMItemClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			gtk_menu_popup(GTK_MENU(fu->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void ignoreusers::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func3<ignoreusers, string, string, bool> F3;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F3 *func = new F3(fu, &ignoreusers::getFileList_client,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"),
					FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void ignoreusers::onMatchQueueItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func3<ignoreusers, string, string, bool> F3;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F3 *func = new F3(fu, &ignoreusers::getFileList_client,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"),
					TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void ignoreusers::onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void ignoreusers::onConnectItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				WulforManager::get()->getMainWindow()->showHub_gui(fu->favoriteUserView.getString(&iter, "URL"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void ignoreusers::onRemoveFromQueueItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func1<ignoreusers, string> F1;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F1 *func = new F1(fu, &ignoreusers::removeUserFromQueue_client, fu->favoriteUserView.getString(&iter, "CID"));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void ignoreusers::onDescriptionItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) == 1)
	{
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		if (list != NULL)
		{
			GtkTreeIter iter;
			GtkTreePath *path = (GtkTreePath *) list->data;
			string description, nick, cid;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				description = fu->favoriteUserView.getString(&iter, N_("Description"));
				nick = fu->favoriteUserView.getString(&iter, N_("Nick"));
				cid = fu->favoriteUserView.getString(&iter, "CID");
			}
			gtk_tree_path_free(path);
			g_list_free(list);

			gtk_window_set_title(GTK_WINDOW(fu->getWidget("DescriptionDialog")), nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fu->getWidget("descriptionEntry")), description.c_str());
			gint response = gtk_dialog_run(GTK_DIALOG(fu->getWidget("DescriptionDialog")));

			// Fix crash, if the dialog gets programmatically destroyed.
			if (response == GTK_RESPONSE_NONE)
			{
				return;
			}

			gtk_widget_hide(fu->getWidget("DescriptionDialog"));

			if (response != GTK_RESPONSE_OK)
				return;

			if (fu->findUser_gui(cid, &iter))
			{
				description = gtk_entry_get_text(GTK_ENTRY(fu->getWidget("descriptionEntry")));
				gtk_list_store_set(fu->favoriteUserStore, &iter, fu->favoriteUserView.col(N_("Description")), description.c_str(), -1);

				typedef Func2<ignoreusers, string, string> F2;
				F2 *func = new F2(fu, &ignoreusers::setUserDescription_client, cid, description);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
	}
}

void ignoreusers::onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	ignoreusers *fu = (ignoreusers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		ParamMap params;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<ignoreusers, string> F1;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				params.insert(ParamMap::value_type(fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, N_("Nick"))));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (BOOLSETTING(CONFIRM_USER_REMOVAL))
		{
			GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_NONE,
				_("Are you sure you want to delete ignore user(s)?"));
			gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE,
				GTK_RESPONSE_YES, NULL);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
			gint response = gtk_dialog_run(GTK_DIALOG(dialog));

			// Widget failed if the dialog gets programmatically destroyed.
			if (response == GTK_RESPONSE_NONE)
				return;

			gtk_widget_hide(dialog);

			if (response != GTK_RESPONSE_YES)
				return;
		}

		for (ParamMap::const_iterator it = params.begin(); it != params.end(); ++it)
		{
			F1 *func = new F1(fu, &ignoreusers::removeFavoriteUser_client, it->first);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void ignoreusers::getFileList_client(const string cid, const string hubUrl, bool match)
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

		if (user)
		{
			const HintedUser hintedUser(user, hubUrl);//NOTE: core 0.762
			if (match)

				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);//NOTE: core 0.762
			else
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
		}
	}
	catch (const Exception& e)
	{
		typedef Func1<ignoreusers, string> F1;
		F1 *func = new F1(this, &ignoreusers::setStatus_gui, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ignoreusers::removeUserFromQueue_client(const string cid)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void ignoreusers::removeFavoriteUser_client(const string cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->removeIgnoredUser(user);
	}
}

void ignoreusers::setUserDescription_client(const string cid, const string description)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->setIgnUserDescription(user, description);
	}
}

bool ignoreusers::findUser_gui(const string &cid, GtkTreeIter *iter)
{
	UserIters::const_iterator it = userIters.find(cid);

	if (it != userIters.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}

void ignoreusers::updateFavoriteUser_gui(ParamMap params)
{
	const string &cid = params["CID"];
	GtkTreeIter iter;

	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_set(favoriteUserStore, &iter, favoriteUserView.col(_("Time last seen")), params["Time"].c_str(), -1);
	}
	else
	{
		gtk_list_store_append(favoriteUserStore, &iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(N_("Nick")), params["Nick"].c_str(),
			favoriteUserView.col(N_("Hub (last seen in, if offline)")), params["Hub"].c_str(),
			favoriteUserView.col(N_("Time last seen")), params["Time"].c_str(),
			favoriteUserView.col(N_("Description")), params["Description"].c_str(),
			favoriteUserView.col("CID"), cid.c_str(),
			favoriteUserView.col("URL"), params["URL"].c_str(),
			favoriteUserView.col("Icon"), "bmdc-normal",
			-1);

		userIters.insert(UserIters::value_type(cid, iter));
	}
}

void ignoreusers::removeFavoriteUser_gui(const string cid)
{
	GtkTreeIter iter;

	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_remove(favoriteUserStore, &iter);
		userIters.erase(cid);
	}
}

void ignoreusers::setStatus_gui(const string text)
{
	if (!text.empty())
	{
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, ("[" + Util::getShortTimeString() + "] " + text).c_str());
	}
}

void ignoreusers::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) throw()
{
	ParamMap params;
	bool online = user.getUser()->isOnline();
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("Hub", online ? WulforUtil::getHubNames(user.getUser(), user.getUrl()) : user.getUrl()));//NOTE: core 0.762
	params.insert(ParamMap::value_type("Time", online ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen())));
	params.insert(ParamMap::value_type("Description", user.getDescription()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("URL", user.getUrl()));

	Func1<ignoreusers, ParamMap> *func = new Func1<ignoreusers, ParamMap>(this, &ignoreusers::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void ignoreusers::on(FavoriteManagerListener::UserRemoved, const FavoriteUser &user) throw()
{
	Func1<ignoreusers, string> *func = new Func1<ignoreusers, string>(this, &ignoreusers::removeFavoriteUser_gui,
		user.getUser()->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);
}

void ignoreusers::on(FavoriteManagerListener::StatusChanged, const FavoriteUser &user) throw()
{
	ParamMap params;
	string seen = user.getUser()->isOnline() ? N_("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen());
	params.insert(ParamMap::value_type("Time", seen));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

	Func1<ignoreusers, ParamMap> *func = new Func1<ignoreusers, ParamMap>(this, &ignoreusers::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

/*this is a generic pop menu*/
void ignoreusers::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()),closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate",G_CALLBACK(onCloseItem),this);
}

void ignoreusers::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}
