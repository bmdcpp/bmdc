//      recenthub.cc
//
//      Copyright 2010-2011 Mank <freedcpp@seznam.cz>
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


#include "recenthub.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "hub.hh"
#include <dcpp/FavoriteManager.h>
#include <dcpp/ClientManager.h>
#include "settingsmanager.hh"

using namespace std;
using namespace dcpp;

RecentTab::RecentTab():
BookEntry(Entry::RECENT,_("Recent Hubs"),"recenthub.glade")
{
		// Configure the dialog
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("DescriptionDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

		// Initialize favorite users list treeview
		recentView.setView(GTK_TREE_VIEW(getWidget("favoriteUserView")));
		recentView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
		recentView.insertColumn(_("Server"), G_TYPE_STRING, TreeView::STRING, 200);
		recentView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 120);
		recentView.insertColumn(_("Users"), G_TYPE_STRING, TreeView::STRING, 100);
		recentView.insertColumn(_("Shared"), G_TYPE_STRING, TreeView::STRING, 100);
		recentView.finalize();
		recentStore = gtk_list_store_newv(recentView.getColCount(), recentView.getGTypes());
		gtk_tree_view_set_model(recentView.get(), GTK_TREE_MODEL(recentStore));
		g_object_unref(recentStore);
		gtk_tree_view_set_fixed_height_mode(recentView.get(), TRUE);
		recentSelection = gtk_tree_view_get_selection(recentView.get());

		/* CONECT TO SIGNAL */

		g_signal_connect(getWidget("connectItem"), "activate", G_CALLBACK(onConnectItemClicked_gui), (gpointer)this);
		g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItemClicked_gui), (gpointer)this);
		g_signal_connect(recentView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
		g_signal_connect(recentView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
		g_signal_connect(recentView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

}

RecentTab::~RecentTab()
{
	FavoriteManager::getInstance()->save();
	FavoriteManager::getInstance()->removeListener(this);
}

void RecentTab::show()
{
	const RecentHubEntry::List& fl=FavoriteManager::getInstance()->getRecentHubs();
	for(RecentHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i)
	{
		GtkTreeIter iter;

		gtk_list_store_append(recentStore, &iter);
		gtk_list_store_set(recentStore, &iter,
			recentView.col(_("Name")), (*i)->getName().c_str(),
			recentView.col(_("Server")), (*i)->getServer().c_str(),
			recentView.col(_("Description")), (*i)->getDescription().c_str(),
			recentView.col(_("Users")), (*i)->getUsers().c_str(),
			recentView.col(_("Shared")),Util::formatBytes((*i)->getShared()).c_str(),
			-1);
		userIters.insert(UserIters::value_type((*i)->getServer(), iter));

	}
	FavoriteManager::getInstance()->addListener(this);
}

gboolean RecentTab::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	RecentTab *fu = (RecentTab *)data;

	if (gtk_tree_selection_count_selected_rows(fu->recentSelection) > 0)
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

gboolean RecentTab::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	RecentTab *fu = (RecentTab *)data;
	fu->previous = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;

		if (gtk_tree_view_get_path_at_pos(fu->recentView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(fu->recentSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean RecentTab::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	RecentTab *fu = (RecentTab *)data;

	if (gtk_tree_selection_count_selected_rows(fu->recentSelection) > 0)
	{
		if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			gtk_menu_popup(GTK_MENU(fu->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void RecentTab::onConnectItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	RecentTab *fu = (RecentTab *)data;

	if (gtk_tree_selection_count_selected_rows(fu->recentSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->recentSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->recentStore), &iter, path))
			{
				WulforManager::get()->getMainWindow()->showHub_gui(fu->recentView.getString(&iter, "Server"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void RecentTab::onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	RecentTab *fu = (RecentTab *)data;

	if (gtk_tree_selection_count_selected_rows(fu->recentSelection) > 0)
	{
		ParamMap params;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<RecentTab, string> F1;
		GList *list = gtk_tree_selection_get_selected_rows(fu->recentSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->recentStore), &iter, path))
			{
				params.insert(ParamMap::value_type(fu->recentView.getString(&iter, "Server"),
					fu->recentView.getString(&iter, _("Name"))));
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
				_("Are you sure you want to delete recent hub(s)?"));
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
			F1 *func = new F1(fu, &RecentTab::removeFavoriteUser_client, it->first);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void RecentTab::removeFavoriteUser_client(const string adr)
{
	RecentHubEntry *r = FavoriteManager::getInstance()->getRecentHubEntry(adr);
	if(r)
	{
		FavoriteManager::getInstance()->removeRecent(r);
	}
}

bool RecentTab::findUser_gui(const string &cid, GtkTreeIter *iter)
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

void RecentTab::updateFavoriteUser_gui(ParamMap params)
{
	const string &server = params["Server"];
	GtkTreeIter iter;

	if (findUser_gui(server, &iter))
	{
		gtk_list_store_set(recentStore, &iter,
				recentView.col(_("Description")), params["Description"].c_str(),
				recentView.col(_("Name")), params["Name"].c_str(),
				recentView.col(_("Shared")), params["Shared"].c_str(),
				recentView.col(_("Users")), params["Users"].c_str(),
				 -1);
	}
	else
	{
		gtk_list_store_append(recentStore, &iter);
		gtk_list_store_set(recentStore, &iter,
			recentView.col(_("Name")), params["Name"].c_str(),
			recentView.col(_("Server")), params["Server"].c_str(),
			recentView.col(_("Users")), params["Users"].c_str(),
			recentView.col(_("Description")), params["Description"].c_str(),
			recentView.col(_("Shared")), params["Shared"].c_str(),
			-1);

		userIters.insert(UserIters::value_type(server, iter));
	}
}

void RecentTab::removeFavoriteUser_gui(const string cid)
{
	GtkTreeIter iter;

	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_remove(recentStore, &iter);
		userIters.erase(cid);
	}
}



void RecentTab::on(FavoriteManagerListener::RecentUpdated, const RecentHubEntry *entry) throw()
{
	ParamMap params;
	params.insert(ParamMap::value_type("Name", entry->getName()));
	params.insert(ParamMap::value_type("Server",entry->getServer()));
	params.insert(ParamMap::value_type("Description", entry->getDescription()));
	params.insert(ParamMap::value_type("Users", entry->getUsers() ));
	params.insert(ParamMap::value_type("Shared", Util::formatBytes(entry->getShared())));

	Func1<RecentTab, ParamMap> *func = new Func1<RecentTab, ParamMap>(this, &RecentTab::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void RecentTab::on(FavoriteManagerListener::RecentRemoved, const RecentHubEntry *entry) throw()
{
	Func1<RecentTab, string> *func = new Func1<RecentTab, string>(this, &RecentTab::removeFavoriteUser_gui,
		entry->getServer());
	WulforManager::get()->dispatchGuiFunc(func);
}

void RecentTab::on(FavoriteManagerListener::RecentAdded, const RecentHubEntry *entry) throw()
{
	ParamMap params;
	params.insert(ParamMap::value_type("Name", entry->getName()));
	params.insert(ParamMap::value_type("Server",entry->getServer()));
	params.insert(ParamMap::value_type("Description", entry->getDescription()));
	params.insert(ParamMap::value_type("Users", entry->getUsers() ));
	params.insert(ParamMap::value_type("Shared", Util::formatBytes(entry->getShared())));

	Func1<RecentTab, ParamMap> *func = new Func1<RecentTab, ParamMap>(this, &RecentTab::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

/*this is a pop menu*/
void RecentTab::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()),closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate",G_CALLBACK(onCloseItem),this);

}

void RecentTab::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}
