//
//      Copyright 2011 - 2025 BMDC
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
//

#include "uploadqueue.hh"

#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/Client.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/QueueManager.h"
#include "../dcpp/UploadManager.h"

#include "wulformanager.hh"
#include "GuiUtil.hh"

using namespace std;
using namespace dcpp;
//0->padding
const GActionEntry UploadQueue::win_entries[] = {
    { "grant-slot", onGrantSlotItemClicked_gui, NULL, NULL, NULL , 0 },
    { "favorite-user", onFavoriteUserAddItemClicked_gui , NULL, NULL, NULL, 0 },
    { "remove-item",  onRemoveItem_gui , NULL, NULL, NULL , 0 },
    { "pm-item", onSendPMItemClicked_gui , NULL, NULL, NULL, 0 },
    { "browse-item", onBrowseItemClicked_gui , NULL, NULL, NULL , 0 },
};

UploadQueue::UploadQueue():
BookEntry(Entry::UPLOADQUEUE, _("Upload Queue"), "uploadqueue"),
selection(NULL)
{

	GSimpleActionGroup* simple = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (simple), win_entries, G_N_ELEMENTS (win_entries), (gpointer)this);
	gtk_widget_insert_action_group(getWidget("viewUsers"), "UploadQueue" ,G_ACTION_GROUP(simple));

	users.setView(GTK_TREE_VIEW(getWidget("viewUsers")));
	users.insertColumn("User", G_TYPE_STRING, TreeView::ICON_STRING, 100, "Icon");
	users.insertColumn("File", G_TYPE_STRING, TreeView::STRING, 300);
	users.insertColumn("Hub", G_TYPE_STRING, TreeView::STRING, 90);
	users.insertColumn("CID", G_TYPE_STRING, TreeView::STRING, 80);
	users.insertHiddenColumn("Icon", G_TYPE_STRING);
	users.finalize();

	store = gtk_list_store_newv(users.getColCount(),users.getGTypes());
	gtk_tree_view_set_model(users.get(), GTK_TREE_MODEL(store));
	g_object_unref(store);

	selection = gtk_tree_view_get_selection(users.get());
	/*
	g_signal_connect(users.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	*/
	GtkGesture *gesture;
	gesture = gtk_gesture_click_new ();
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
	g_signal_connect (gesture, "pressed",
                    G_CALLBACK (onWidgetPressed), (gpointer)this);
	g_signal_connect (gesture, "released",
                    G_CALLBACK (on_widget_right_btn_released), (gpointer)this);
	gtk_widget_add_controller (GTK_WIDGET(users.get()), GTK_EVENT_CONTROLLER (gesture));

}

void UploadQueue::onWidgetPressed (GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	
	UploadQueue *FH = (UploadQueue*)data;
	g_debug ("on_inner_widget_right_btn_pressed() called\n");

	GMenu *menu = g_menu_new ();
	GMenuItem *menu_item_add = g_menu_item_new ("Grant Slot", "UploadQueue.grant-slot");
	g_menu_append_item (menu, menu_item_add);
	g_object_unref (menu_item_add);

	GMenuItem* menu_item_edit = g_menu_item_new ("Browse Filelist", "UploadQueue.browse-fl");
	g_menu_append_item (menu, menu_item_edit);
	g_object_unref (menu_item_edit);

	GMenuItem* menu_item_conn = g_menu_item_new ("Add Favorite User", "UploadQueue.favorite-user");
	g_menu_append_item (menu, menu_item_conn);
	g_object_unref (menu_item_conn);

	GMenuItem* menu_item_copy = g_menu_item_new ("Remove Item", "UploadQueue.remove-item");
	g_menu_append_item (menu, menu_item_copy);
	g_object_unref (menu_item_copy);

	GMenuItem* menu_item_rem = g_menu_item_new ("Send Private Message", "UploadQueue.pm-item");
	g_menu_append_item (menu, menu_item_rem);
	g_object_unref (menu_item_rem);

	GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	gtk_widget_set_parent(pop, FH->getContainer());
	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
	gtk_popover_popup (GTK_POPOVER(pop));

}

void UploadQueue::on_widget_right_btn_released (GtkGestureClick *gesture,
                                    int             /* n_press*/,
                                    double          /* x*/,
                                    double           /*y*/,
                                    GtkWidget*       /*widget*/)
{
  g_debug ("on_inner_widget_right_btn_released() called\n");

  gtk_gesture_set_state (GTK_GESTURE (gesture),
                         GTK_EVENT_SEQUENCE_CLAIMED);
}

UploadQueue::~UploadQueue()
{
	UploadManager::getInstance()->removeListener(this);
	gtk_list_store_clear(store);
}

void UploadQueue::show()
{
	UploadManager::getInstance()->addListener(this);
	intilaize_client();
}

void UploadQueue::intilaize_client()
{
// Load queue
	const HintedUserList _users = UploadManager::getInstance()->getWaitingUsers();
	UploadManager *up = UploadManager::getInstance();
	for(HintedUserList::const_iterator uit = _users.begin(); uit != _users.end(); ++uit) 
	{
		
		const UploadManager::FileSet f = up->getWaitingUserFiles(((*uit).user));
		StringMap params;
		
		for(auto fit = f.begin(); fit!= f.end();++fit)
		{
			GtkTreeIter iter;
			getParams(*fit,(*uit).user, params);
			addFile(params, &iter);
		}

	}
}

void UploadQueue::getParams(const string& file, UserPtr user, StringMap &params)
{
	params["file"]  = file;
	params["Nick"]  = WulforUtil::getNicks(user->getCID(), string());
	params["CID"]   = user->getCID().toBase32();
	params["hub"]	= WulforUtil::getHubNames(user->getCID(), string());
}

void UploadQueue::addFile(StringMap &params,GtkTreeIter *iter)
{

	gtk_list_store_append(store,iter);
	gtk_list_store_set(store, iter,
				users.col("User"), params["Nick"].c_str(),
				users.col("File"), params["file"].c_str(),
				users.col("Hub") , params["hub"].c_str(),
				users.col("CID"),  params["CID"].c_str(),
				users.col("Icon"), "bmdc-normal",
				-1);
				
	mapUsers.insert(UnMapIter::value_type(params["CID"], *iter));
}

void UploadQueue::AddFile_gui(StringMap params)
{
	GtkTreeIter iter;
	g_autofree gchar *cpfile = NULL;
	unordered_map<string,GtkTreeIter>::iterator it = mapUsers.find(params["CID"]);
	if(it != mapUsers.end())
	{
		iter = it->second;
		gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,
								1,&cpfile,-1);
		params["file"] += string(cpfile);

	}
	addFile(params,&iter);
}

void UploadQueue::removeUser(const string &cid)
{
	GtkTreeIter iter;
	unordered_map<string, GtkTreeIter>::iterator it = mapUsers.find(cid);
	if(it != mapUsers.end())
	{
		iter = it->second;
		gtk_list_store_remove(store,&iter);
		mapUsers.erase(it);

	}
}

void UploadQueue::onGrantSlotItemClicked_gui(GtkWidget *widget,GVariant  *, gpointer data)
{
	UploadQueue *qp = (UploadQueue *)data;

	if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path = NULL;
		GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
		typedef Func1<UploadQueue, const string&> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
			{
				F2 *func = new F2(qp, &UploadQueue::grantSlot_client,
					(qp->users.getString(&iter, "CID")));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void UploadQueue::onRemoveItem_gui(GtkWidget *widget,GVariant  *, gpointer data)
{
	UploadQueue *qp = (UploadQueue *)data;

	if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path = NULL;
		GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
		typedef Func1<UploadQueue, const string&> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
			{
				F2 *func = new F2(qp, &UploadQueue::removeUploadFromQueue,
					qp->users.getString(&iter, "CID"));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void UploadQueue::onSendPMItemClicked_gui(GtkWidget *widget,GVariant  *, gpointer data)
{
	UploadQueue *qp = (UploadQueue *)data;

	if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path = NULL;
		GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
			{
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN,
					qp->users.getString(&iter, "CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void UploadQueue::onBrowseItemClicked_gui(GtkWidget *widget,GVariant  *, gpointer data)
{
	UploadQueue *qp = (UploadQueue *)data;

	if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path = NULL;
		GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
		typedef Func1<UploadQueue, const string&> F1;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
			{
				F1 *func = new F1(qp, &UploadQueue::getFileList_client,
				qp->users.getString(&iter, "CID"));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void UploadQueue::onFavoriteUserAddItemClicked_gui(GtkWidget *widget,GVariant*, gpointer data)
{
	UploadQueue *qp = (UploadQueue *)data;

	if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path = NULL;
		GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
		typedef Func1<UploadQueue, const string&> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
			{
				F2 *func = new F2(qp, &UploadQueue::addFavoriteUser_client,
					qp->users.getString(&iter, "CID"));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
gboolean UploadQueue::onKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	UploadQueue *qp = (UploadQueue *)data;

	if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
	{
		if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
		{
		     gtk_menu_popup_at_pointer(GTK_MENU(qp->getWidget("menu")),NULL);
		}
	}

	return FALSE;
}
*/
void UploadQueue::grantSlot_client(const string &cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(user, dcpp::Util::emptyString));
	}
}

void UploadQueue::removeUploadFromQueue(const string &cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
          UploadManager::getInstance()->clearUserFiles(user);
	}
}

void UploadQueue::getFileList_client(const string &cid)
{
	try {
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if(user)
		{
			HintedUser hintedUser(user, dcpp::Util::emptyString);
			QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
		}
	}catch(...)
	{ 
		//... for now ignore it
	}

}

void UploadQueue::addFavoriteUser_client(const string &cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void UploadQueue::on(dcpp::UploadManagerListener::WaitingAddFile, const HintedUser& hUser, const string& file) noexcept
{
	StringMap params;
	getParams(file,hUser.user,params);
	typedef Func1<UploadQueue, StringMap> F1;
	F1 *func = new F1(this,&UploadQueue::AddFile_gui,params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void UploadQueue::on(dcpp::UploadManagerListener::WaitingRemoveUser, const HintedUser& user) noexcept
{
	typedef Func1<UploadQueue, const string& > F1;
	F1 *func = new F1(this, &UploadQueue::removeUser,user.user->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);
}
