/*
* Copyright © 2009-2018 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/
#include "favoriteusers.hh"

#include "../dcpp/ClientManager.h"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "message.hh"

using namespace std;
using namespace dcpp;

const GActionEntry FavoriteUsers::fuser_entries[] = {
//    { "add", onAddEntry_gui, NULL, NULL, NULL },
    { "delete", onRemoveItemClicked_gui, NULL, NULL, NULL },
    { "grant-slot",onGrantSlotItemClicked_gui, NULL, NULL, NULL }
};

FavoriteUsers::FavoriteUsers():
	BookEntry(Entry::FAVORITE_USERS, _("Favorite Users"), "favoriteusers")
{

	GSimpleActionGroup *group;
	group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (group), fuser_entries, G_N_ELEMENTS (fuser_entries), (gpointer)this);
	gtk_widget_insert_action_group(getContainer(),"favu" ,G_ACTION_GROUP(group));

	// Initialize favorite users list treeview
	favoriteUserView.setView(GTK_TREE_VIEW(getWidget("favoriteUserView")), TRUE, "favoriteusers");
	favoriteUserView.insertColumn(_("Auto grant slot"), G_TYPE_BOOLEAN, TreeView::BOOL, 30);
	favoriteUserView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING, 100, "Icon");
	favoriteUserView.insertColumn(_("Hub (last seen in, if offline)"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteUserView.insertColumn(_("Time last seen"), G_TYPE_STRING, TreeView::STRING, 120);
	favoriteUserView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteUserView.insertColumn(_("Nicks History (if it have CID)"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteUserView.insertColumn(_("Ignore"), G_TYPE_STRING, TreeView::STRING, 20);
	favoriteUserView.insertColumn("CID", G_TYPE_STRING, TreeView::STRING, 350);
	favoriteUserView.insertHiddenColumn("URL", G_TYPE_STRING);
	favoriteUserView.insertHiddenColumn("Icon", G_TYPE_STRING);
	favoriteUserView.insertHiddenColumn("Ign", G_TYPE_STRING);
	favoriteUserView.insertHiddenColumn("Type", G_TYPE_STRING);
	favoriteUserView.finalize();

	favoriteUserStore = gtk_list_store_newv(favoriteUserView.getColCount(), favoriteUserView.getGTypes());
	gtk_tree_view_set_model(favoriteUserView.get(), GTK_TREE_MODEL(favoriteUserStore));
	g_object_unref(favoriteUserStore);

	favoriteUserSelection = gtk_tree_view_get_selection(favoriteUserView.get());
	gtk_tree_selection_set_mode(favoriteUserSelection, GTK_SELECTION_MULTIPLE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(favoriteUserStore), favoriteUserView.col(_("Nick")), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(favoriteUserView.get(), favoriteUserView.col(_("Nick"))), TRUE);

	/* Register for mouse right button click "pressed" and "released" events on  widget*/
	GtkGesture *gesture;
	gesture = gtk_gesture_click_new ();
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
	g_signal_connect (gesture, "pressed",
                    G_CALLBACK (on_right_btn_pressed), (gpointer)this);
	g_signal_connect (gesture, "released",
                    G_CALLBACK (on_right_btn_released), (gpointer)this);
	gtk_widget_add_controller (GTK_WIDGET(favoriteUserView.get()), GTK_EVENT_CONTROLLER (gesture));

//	g_signal_connect(getWidget("browseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("sendPMItem"), "activate", G_CALLBACK(onSendPMItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("grantSlotItem"), "activate", G_CALLBACK(onGrantSlotItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("connectItem"), "activate", G_CALLBACK(onConnectItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("removeFromQueueItem"), "activate", G_CALLBACK(onRemoveFromQueueItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("descriptionItem"), "activate", G_CALLBACK(onDescriptionItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("ignoreItem"), "activate", G_CALLBACK(onIgnoreSetUserClicked_gui), (gpointer)this);

}

void FavoriteUsers::on_right_btn_pressed (GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	FavoriteUsers *FU = (FavoriteUsers*)data;

	GMenu *menu = g_menu_new ();
//	GMenuItem *menu_item_add = g_menu_item_new ("Add", "favu.add");
//	g_menu_append_item (menu, menu_item_add);
//	g_object_unref (menu_item_add);

	GMenuItem* menu_item_edit = g_menu_item_new ("Delete", "favu.delete");
	g_menu_append_item (menu, menu_item_edit);
	g_object_unref (menu_item_edit);

	GMenuItem* menu_item_conn = g_menu_item_new ("Grant Slot", "favu.grant-slot");
	g_menu_append_item (menu, menu_item_conn);
	g_object_unref (menu_item_conn);

	GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	gtk_widget_set_parent(pop, FU->getContainer());
	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
	gtk_popover_popup (GTK_POPOVER(pop));

}

void FavoriteUsers::on_right_btn_released (GtkGestureClick *gesture,
                                    int             /* n_press*/,
                                    double          /* x*/,
                                    double           /*y*/,
                                    gpointer*       /*widget*/)
{
  gtk_gesture_set_state (GTK_GESTURE (gesture),
                         GTK_EVENT_SEQUENCE_CLAIMED);
}

FavoriteUsers::~FavoriteUsers()
{
	FavoriteManager::getInstance()->removeListener(this);
}

void FavoriteUsers::show()
{
	// Initialize favorite users list
	FavoriteManager::FavoriteMap map = FavoriteManager::getInstance()->getFavoriteUsers();
	FavoriteManager::FavoriteMap::const_iterator it;

	for (it = map.begin(); it != map.end(); ++it)
	{
		GtkTreeIter iter;
		const FavoriteUser &user = it->second;
		bool online = user.getUser()->isOnline();
		string hub = online ? WulforUtil::getHubNames(user.getUser(), user.getUrl()) : user.getUrl();
		string seen = online ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen());
		string cid = user.getUser()->getCID().toBase32();
		string ignore = user.isSet(FavoriteUser::FLAG_IGNORE) ? _("Yes") : _("No");

		gtk_list_store_append(favoriteUserStore, &iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(_("Auto grant slot")), user.isSet(FavoriteUser::FLAG_GRANTSLOT) ? TRUE : FALSE,
			favoriteUserView.col(_("Nick")), user.getNick().c_str(),
			favoriteUserView.col(_("Hub (last seen in, if offline)")), hub.c_str(),
			favoriteUserView.col(_("Time last seen")), seen.c_str(),
			favoriteUserView.col(_("Description")), user.getDescription().c_str(),
			favoriteUserView.col(_("Nicks History (if it have CID)")), user.getNicks().c_str(),
			favoriteUserView.col(_("Ignore")), ignore.c_str(),
			favoriteUserView.col("CID"), cid.c_str(),
			favoriteUserView.col("URL"), user.getUrl().c_str(),
			favoriteUserView.col("Icon"), "bmdc-normal",
			favoriteUserView.col("Ign"), string(user.isSet(FavoriteUser::FLAG_IGNORE) ? "1" : "0").c_str(),
			favoriteUserView.col("Type"), "cid",
			-1);

		userIters.insert(UnMapIter::value_type(cid, iter));
	}
	
	auto nickmap = FavoriteManager::getInstance()->getFavoritesIndepentOnCid();
	for(auto nt = nickmap.begin();nt!= nickmap.end();++nt)
	{
		GtkTreeIter iter;
		FavoriteUser* u = nt->second; 
		string seen = Util::formatTime("%Y-%m-%d %H:%M", u->getLastSeen());
		
		gtk_list_store_append(favoriteUserStore,&iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(_("Auto grant slot")), u->isSet(FavoriteUser::FLAG_GRANTSLOT) ? TRUE : FALSE,
			favoriteUserView.col(_("Nick")), nt->first.c_str(),	
			favoriteUserView.col(_("Hub (last seen in, if offline)")), Util::emptyString.c_str(),
			favoriteUserView.col(_("Time last seen")), seen.c_str(),
			favoriteUserView.col(_("Description")), u->getDescription().c_str(),
			favoriteUserView.col("CID"), "n/a",
			favoriteUserView.col("URL"), "n/a",
			favoriteUserView.col("Icon"), "bmdc-normal",
			favoriteUserView.col("Type"), "nick",

		-1);
		nicksIters.insert(UnMapIter::value_type(nt->first, iter));
		
	}
	
	auto list = FavoriteManager::getInstance()->getListIp();

	for(auto it:list)
	{
		GtkTreeIter iter;
		FavoriteUser* u = it.second;
		string seen = Util::formatTime("%Y-%m-%d %H:%M", u->getLastSeen());
		
		gtk_list_store_append(favoriteUserStore,&iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(_("Auto grant slot")), u->isSet(FavoriteUser::FLAG_GRANTSLOT) ? TRUE : FALSE,
			favoriteUserView.col(_("Nick")), it.first.c_str(),	
			favoriteUserView.col(_("Hub (last seen in, if offline)")), Util::emptyString.c_str(),
			favoriteUserView.col(_("Time last seen")), seen.c_str(),
			favoriteUserView.col(_("Description")), u->getDescription().c_str(),
			favoriteUserView.col("CID"), "n/a",
			favoriteUserView.col("URL"), "n/a",
			favoriteUserView.col("Icon"), "bmdc-normal",
			favoriteUserView.col("Type"), "ip",

		-1);
	
	
	}
	FavoriteManager::getInstance()->addListener(this);
}
/*
gboolean FavoriteUsers::onKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
}
*/
/*
void FavoriteUsers::clickAction(gpointer data)
{
	//TODO:maybe..some other & UI & fav?
	switch((CActions::User)WGETI("double-click-action"))
	{
		case CActions::BROWSE:
			onBrowseItemClicked_gui(NULL, data);
			break;
		case CActions::NICK_TO_CHAT:
			break;
	    case CActions::PM_TO_NICK:
			onSendPMItemClicked_gui(NULL, data);
			break;
		case CActions::MATCH_Q:
			onMatchQueueItemClicked_gui(NULL ,data);
			break;
		case CActions::GRANT_SLOT:
			onGrantSlotItemClicked_gui(NULL, data);
			break;
		case CActions::ADD_AS_FAV:
			break;
		case CActions::GET_PARTIAL_FILELIST:
			break;
		default: break;
	}
}*/
/*
void FavoriteUsers::onBrowseItemClicked_gui(GtkMenuItem*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func3<FavoriteUsers, string, string, bool> F3;
		typedef Func2<FavoriteUsers, string, bool> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string nick = fu->favoriteUserView.getString(&iter, _("Nick"));
				string type = fu->favoriteUserView.getString(&iter, "Type");
				
				if(type == "nick")
				{
					F2 *func = new F2(fu, &FavoriteUsers::getFileListNick_client,
						nick,
						FALSE);
					WulforManager::get()->dispatchClientFunc(func);	
					return;	
				}
				if(type == "cid")
				{
					F3 *func = new F3(fu, &FavoriteUsers::getFileList_client,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"),
					FALSE);
					WulforManager::get()->dispatchClientFunc(func);
				}
				if(type == "ip")continue;//TODO	
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void FavoriteUsers::onMatchQueueItemClicked_gui(GtkMenuItem*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func3<FavoriteUsers, string, string, bool> F3;
		typedef Func2<FavoriteUsers, string, bool> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string nick = fu->favoriteUserView.getString(&iter, _("Nick"));
				string type = fu->favoriteUserView.getString(&iter, "Type");
				
				if(type == "nick")
				{
					F2 *func = new F2(fu, &FavoriteUsers::getFileListNick_client,
					nick,
					TRUE);
					WulforManager::get()->dispatchClientFunc(func);
					return;
				}
				if(type == "cid") {
				F3 *func = new F3(fu, &FavoriteUsers::getFileList_client,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"),
					TRUE);
				WulforManager::get()->dispatchClientFunc(func);
				}
				if(type == "ip") continue;
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void FavoriteUsers::onSendPMItemClicked_gui(GtkMenuItem*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

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
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string type = fu->favoriteUserView.getString(&iter, "Type");
				if(type == "nick") return;
				if(type == "ip") return;
				
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}*/

void FavoriteUsers::onGrantSlotItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func2<FavoriteUsers, string, string> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string nick = fu->favoriteUserView.getString(&iter, _("Nick"));
				string type = fu->favoriteUserView.getString(&iter, "Type");
				if(type == "nick")
				{
					FavoriteUser* f = FavoriteManager::getInstance()->getIndepentFavorite(nick);
					f->setFlag(FavoriteUser::FLAG_GRANTSLOT);
					return;	
				}
				if(type == "cid") {
					F2 *func = new F2(fu, &FavoriteUsers::grantSlot_client,
					fu->favoriteUserView.getString(&iter, "CID"),
					fu->favoriteUserView.getString(&iter, "URL"));
					WulforManager::get()->dispatchClientFunc(func);
				}
				if(type == "ip") return;
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void FavoriteUsers::onConnectItemClicked_gui(GtkMenuItem*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

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
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string type = fu->favoriteUserView.getString(&iter, "Type");
				if(type == "nick") return;
				if(type == "ip") return;
				
				WulforManager::get()->getMainWindow()->showHub_gui(fu->favoriteUserView.getString(&iter, "URL"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void FavoriteUsers::onRemoveFromQueueItemClicked_gui(GtkMenuItem*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func1<FavoriteUsers, string> F1;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string type = fu->favoriteUserView.getString(&iter, "Type");
				if(type == "nick")return;
				if(type == "ip")return;
				
				F1 *func = new F1(fu, &FavoriteUsers::removeUserFromQueue_client, fu->favoriteUserView.getString(&iter, "CID"));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void FavoriteUsers::onDescriptionItemClicked_gui(GtkMenuItem*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) == 1)
	{
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		if (list != NULL)
		{
			GtkTreeIter iter;
			GtkTreePath *path = (GtkTreePath *) list->data;
			string description, nick, cid,type;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				description = fu->favoriteUserView.getString(&iter, _("Description"));
				nick = fu->favoriteUserView.getString(&iter, _("Nick"));
				cid = fu->favoriteUserView.getString(&iter, "CID");
				type = fu->favoriteUserView.getString(&iter, "Type");
			}
			gtk_tree_path_free(path);
			g_list_free(list);

			gtk_window_set_title(GTK_WINDOW(fu->getWidget("DescriptionDialog")), nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fu->getWidget("descriptionEntry")), description.c_str());
		//	gint response = gtk_dialog_run(GTK_DIALOG(fu->getWidget("DescriptionDialog")));

			// Fix crash, if the dialog gets programmatically destroyed.
			if (response == GTK_RESPONSE_NONE)
			{
				return;
			}

			gtk_widget_hide(fu->getWidget("DescriptionDialog"));

			if (response != GTK_RESPONSE_OK)
				return;
				
			if(type == "ip") return;	

			if(type == "nick") 
			{
				//description = gtk_entry_get_text(GTK_ENTRY(fu->getWidget("descriptionEntry")));
				gtk_list_store_set(fu->favoriteUserStore, &iter, fu->favoriteUserView.col(_("Description")), description.c_str(), -1);
				typedef Func2<FavoriteUsers, string, string> F2;
				F2 *func = new F2(fu, &FavoriteUsers::setDesc_client,nick,description);
				WulforManager::get()->dispatchClientFunc(func);
				return;
			}

			if (fu->findUser_gui(cid, &iter))
			{
				//description = gtk_entry_get_text(GTK_ENTRY(fu->getWidget("descriptionEntry")));
				gtk_list_store_set(fu->favoriteUserStore, &iter, fu->favoriteUserView.col(_("Description")), description.c_str(), -1);
				typedef Func2<FavoriteUsers, string, string> F2;
				F2 *func = new F2(fu, &FavoriteUsers::setUserDescription_client, cid, description);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
	}
}
*/
void FavoriteUsers::onRemoveItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		vector<string> remove;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<FavoriteUsers, string> F1;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				string type = fu->favoriteUserView.getString(&iter, "Type");
				if(type == "nick")
				{
					string nick = fu->favoriteUserView.getString(&iter, _("Nick"));		
					FavoriteManager::getInstance()->removeFavoriteIUser(nick);
					return;
				}
				if(type == "ip")
			{//todo
			return;	
			}
					
				remove.push_back(cid);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (SETTING(CONFIRM_USER_REMOVAL))
		{
			//;
		}

		for (vector<string>::const_iterator it = remove.begin(); it != remove.end(); ++it)
		{
			F1 *func = new F1(fu, &FavoriteUsers::removeFavoriteUser_client, *it);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}
/*
void FavoriteUsers::onAutoGrantSlotToggled_gui(GtkCellRendererToggle*, gchar *path, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
	{
		string cid = fu->favoriteUserView.getString(&iter, "CID");
		string type = fu->favoriteUserView.getString(&iter, "Type");
		gboolean grant = fu->favoriteUserView.getValue<gboolean>(&iter, _("Auto grant slot"));
		grant = !grant;
		gtk_list_store_set(fu->favoriteUserStore, &iter, fu->favoriteUserView.col(_("Auto grant slot")), grant, -1);

		if(type == "ip")
			return;
		if(type == "nick")
		{
			FavoriteUser* u = FavoriteManager::getInstance()->getIndepentFavorite(fu->favoriteUserView.getString(&iter,_("Nick")));
			if(u != NULL) {
				u->setFlag(FavoriteUser::FLAG_GRANTSLOT);
				FavoriteManager::getInstance()->save();
				return;
			}	
		}
		typedef Func2<FavoriteUsers, string, bool> F2;
		F2 *func = new F2(fu, &FavoriteUsers::setAutoGrantSlot_client, cid, grant);
		WulforManager::get()->dispatchClientFunc(func);
	}
}
*/
void FavoriteUsers::getFileList_client(const string cid, const string hubUrl, bool match)
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

		if (user)
		{
			const HintedUser hintedUser(user, hubUrl);
			if (match)
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);
			else
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
		}
	}
	catch (const Exception& e)
	{
		typedef Func1<FavoriteUsers, string> F1;
		F1 *func = new F1(this, &FavoriteUsers::setStatus_gui, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FavoriteUsers::getFileListNick_client(const string nick, bool match)
{
	try
	{
		UserPtr u = ClientManager::getInstance()->findUser(nick, Util::emptyString);
		if(u)			
		{
			const HintedUser hintedUser(u,Util::emptyString);
			if (match)
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);
			else
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
		}
	}catch(const Exception& e)
	{
		typedef Func1<FavoriteUsers, string> F1;
		F1 *func = new F1(this, &FavoriteUsers::setStatus_gui, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);	
	}
}


void FavoriteUsers::grantSlot_client(const string cid, const string hubUrl)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));
		typedef Func1<FavoriteUsers, string> F1;
		F1 *func = new F1(this, &FavoriteUsers::setStatus_gui, _("Slot granted to ") + WulforUtil::getNicks(user, hubUrl));
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FavoriteUsers::removeUserFromQueue_client(const string cid)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void FavoriteUsers::setAutoGrantSlot_client(const string cid, bool grant)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->setAutoGrant(user, grant);
	}
}

void FavoriteUsers::removeFavoriteUser_client(const string cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->removeFavoriteUser(user);
	}
}

void FavoriteUsers::setUserDescription_client(const string cid, const string description)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->setUserDescription(user, description);
	}
}

void FavoriteUsers::setDesc_client(const string nick, const string desc)
{
	FavoriteUser* u = FavoriteManager::getInstance()->getIndepentFavorite(nick);
	if(u!= NULL)
	{
		u->setDescription(desc);	
	}
}

bool FavoriteUsers::findUser_gui(const string &cid, GtkTreeIter *iter)
{
	UnMapIter::const_iterator it = userIters.find(cid);

	if (it != userIters.end())
	{
		if (iter)
			*iter = it->second;

		return true;
	}

	return false;
}

bool FavoriteUsers::findNicks_gui(const string &nick, GtkTreeIter *iter)
{
	UnMapIter::const_iterator it = nicksIters.find(nick);
	if(it!= nicksIters.end())
	{
		
		if(iter)
			*iter = it->second;
		return true;
	}	
	return false;
}

void FavoriteUsers::updateFavoriteUser_gui(ParamMap params)
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
			favoriteUserView.col(_("Nick")), params["Nick"].c_str(),
			favoriteUserView.col(_("Hub (last seen in, if offline)")), params["Hub"].c_str(),
			favoriteUserView.col(_("Time last seen")), params["Time"].c_str(),
			favoriteUserView.col(_("Description")), params["Description"].c_str(),
			favoriteUserView.col(_("Ignore")), params["Ignore"].c_str(),
			favoriteUserView.col("CID"), cid.c_str(),
			favoriteUserView.col("URL"), params["URL"].c_str(),
			favoriteUserView.col("Icon"), "bmdc-normal",
			favoriteUserView.col("Ign"), params["ign"].c_str(),
			-1);

		userIters.insert(UnMapIter::value_type(cid, iter));
	}
}

void FavoriteUsers::updateFavoriteNicks_gui(ParamMap params)
{
	const string &nick = params["Nick"];
	GtkTreeIter iter;
	
	if(findNicks_gui(nick, &iter))
	{
		gtk_list_store_set(favoriteUserStore, &iter, favoriteUserView.col(_("Time last seen")), params["Time"].c_str(), -1);
	}
	else
	{
		gtk_list_store_append(favoriteUserStore, &iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(_("Nick")), params["Nick"].c_str(),
			favoriteUserView.col(_("Hub (last seen in, if offline)")), params["Hub"].c_str(),
			favoriteUserView.col(_("Time last seen")), params["Time"].c_str(),
			favoriteUserView.col(_("Description")), params["Description"].c_str(),
			favoriteUserView.col("CID"), "n/a",
			favoriteUserView.col("URL"), params["URL"].c_str(),
			favoriteUserView.col("Icon"), "bmdc-normal",//todo another icon maybe
			-1);
		nicksIters.insert(UnMapIter::value_type(nick, iter));	
	}
}

void FavoriteUsers::removeFavoriteUser_gui(const string cid)
{
	GtkTreeIter iter;

	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_remove(favoriteUserStore, &iter);
		userIters.erase(cid);
	}
}

void FavoriteUsers::removeFavoriteNicks_gui(const string nick)
{
	GtkTreeIter iter;

	if(findNicks_gui(nick, &iter))	
	{
		gtk_list_store_remove(favoriteUserStore, &iter);
		nicksIters.erase(nick);
	}
}

void FavoriteUsers::setStatus_gui(const string text)
{
	if (!text.empty())
	{
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, ("[" + Util::getShortTimeString() + "] " + text).c_str());
	}
}

void FavoriteUsers::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) noexcept
{
	ParamMap params;
	bool online = user.getUser()->isOnline();
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("Hub", online ? WulforUtil::getHubNames(user.getUser(), user.getUrl()) : user.getUrl()));//NOTE: core 0.762
	params.insert(ParamMap::value_type("Time", online ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen())));
	params.insert(ParamMap::value_type("Description", user.getDescription()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("URL", user.getUrl()));
	params.insert(ParamMap::value_type("Ignore", user.isSet(FavoriteUser::FLAG_IGNORE) ? _("Yes") : _("No")));
	params.insert(ParamMap::value_type("ign", user.isSet(FavoriteUser::FLAG_IGNORE) ? "1" : "0"));

	Func1<FavoriteUsers, ParamMap> *func = new Func1<FavoriteUsers, ParamMap>(this, &FavoriteUsers::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteUsers::on(FavoriteManagerListener::UserRemoved, const FavoriteUser &user) noexcept
{
	Func1<FavoriteUsers, string> *func = new Func1<FavoriteUsers, string>(this, &FavoriteUsers::removeFavoriteUser_gui,
		user.getUser()->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteUsers::on(FavoriteManagerListener::StatusChanged, const FavoriteUser &user) noexcept
{
	ParamMap params;
	string seen = user.getUser()->isOnline() ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen());
	params.insert(ParamMap::value_type("Time", seen));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("Ignore", user.isSet(FavoriteUser::FLAG_IGNORE) ? _("Yes") : _("No")));
	params.insert(ParamMap::value_type("ign", user.isSet(FavoriteUser::FLAG_IGNORE) ? "1" : "0"));

	Func1<FavoriteUsers, ParamMap> *func = new Func1<FavoriteUsers, ParamMap>(this, &FavoriteUsers::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
//Indepent
void FavoriteUsers::on(FavoriteManagerListener::FavoriteIAdded, const string &nick, FavoriteUser* &user) noexcept
{
	ParamMap params;
	params.insert(ParamMap::value_type("Nick", nick));
	params.insert(ParamMap::value_type("Hub", Util::emptyString));
	params.insert(ParamMap::value_type("Time", Util::formatTime("%Y-%m-%d %H:%M", user->getLastSeen())));
	params.insert(ParamMap::value_type("Description", user->getDescription()));
	params.insert(ParamMap::value_type("CID", "n/a"));
	params.insert(ParamMap::value_type("URL", Util::emptyString));
	
	Func1<FavoriteUsers, ParamMap> *func = new Func1<FavoriteUsers, ParamMap>(this, &FavoriteUsers::updateFavoriteNicks_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteUsers::on(FavoriteManagerListener::FavoriteIRemoved, const string &nick, FavoriteUser*&) noexcept
{
	Func1<FavoriteUsers, string> *func = new Func1<FavoriteUsers, string>(this, &FavoriteUsers::removeFavoriteNicks_gui,
		nick);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteUsers::on(FavoriteManagerListener::FavoriteIUpdate, const string &nick , FavoriteUser* &user) noexcept 
{ 
	ParamMap params;
	string seen = Util::formatTime("%Y-%m-%d %H:%M", user->getLastSeen());
	params.insert(ParamMap::value_type("Time", seen));
	params.insert(ParamMap::value_type("CID", user->getCid()));
	params.insert(ParamMap::value_type("Nick", nick));

	Func1<FavoriteUsers, ParamMap> *func = new Func1<FavoriteUsers, ParamMap>(this, &FavoriteUsers::updateFavoriteNicks_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
/*
void FavoriteUsers::onIgnoreSetUserClicked_gui(GtkWidget*, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func2<FavoriteUsers, string, bool> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				string cid = fu->favoriteUserView.getString(&iter, "CID");
				bool ign = fu->favoriteUserView.getString(&iter, "Ign") == "1" ? true : false ;
				if(cid == "n/a")return;

				F2 *func = new F2(fu,&FavoriteUsers::setIgnore,cid,ign);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
*/
void FavoriteUsers::setIgnore(const string cid, bool ignore)
{
	UserPtr uptr = ClientManager::getInstance()->findUser(CID(cid));

	if (uptr)
	{
		FavoriteManager::getInstance()->addFavoriteUser(uptr);
		if(ignore)
			FavoriteManager::getInstance()->setIgnore(uptr, true);
		else FavoriteManager::getInstance()->setIgnore(uptr, false);
	

		typedef Func1<FavoriteUsers, string> F1;
		F1 *func = new F1(this, &FavoriteUsers::setStatus_gui, _("Ignored User ") + WulforUtil::getNicks(uptr, Util::emptyString));
		WulforManager::get()->dispatchGuiFunc(func);

		const FavoriteUser* user = FavoriteManager::getInstance()->getFavoriteUser(uptr);
		ParamMap params;
		string seen = (*user).getUser()->isOnline() ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", (*user).getLastSeen());
		params.insert(ParamMap::value_type("Time", seen));
		params.insert(ParamMap::value_type("CID", (*user).getUser()->getCID().toBase32()));
		params.insert(ParamMap::value_type("Ignore", (*user).isSet(FavoriteUser::FLAG_IGNORE) ? _("Yes") : _("No")));
		params.insert(ParamMap::value_type("ign", (*user).isSet(FavoriteUser::FLAG_IGNORE) ? "1" : "0"));

		typedef Func1<FavoriteUsers, ParamMap> FX;
		FX *funcs = new FX(this,&FavoriteUsers::updateFavoriteUser_gui,params);
		WulforManager::get()->dispatchGuiFunc(funcs);

		FavoriteManager::getInstance()->save();

	}
}

