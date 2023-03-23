/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2025 BMDC
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

#include "favoritehubs.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "FavoriteHubDialog.hh"
#include "AboutConfigFav.hh"

using namespace std;
using namespace dcpp;

const GActionEntry FavoriteHubs::win_entries[] = {
    { "add", onAddEntry_gui, NULL, NULL, NULL },
    { "edit", onEditEntry_gui, NULL, NULL, NULL },
    { "connect",onConnect_gui, NULL, NULL, NULL },
    { "remove", onRemoveEntry_gui, NULL, NULL, NULL },
    { "copy-url", onCopyAddress, NULL, NULL, NULL},
    { "advanced-settings", onAdvancedSettings, NULL, NULL, NULL}
	};

FavoriteHubs::FavoriteHubs():
	BookEntry(Entry::FAVORITE_HUBS, _("Favorite Hubs"), "favoritehubs")
{
	GSimpleActionGroup *group;
	group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (group), win_entries, G_N_ELEMENTS (win_entries), (gpointer)this);
	gtk_widget_insert_action_group(getContainer(),"fav" ,G_ACTION_GROUP(group));

	// Initialize favorite hub list treeview
	favoriteView.setView(GTK_TREE_VIEW(getWidget("favoriteView")), TRUE, "favoritehubs");
	favoriteView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 50);
	favoriteView.insertColumn(_("Address"), G_TYPE_STRING, TreeView::STRING, 150);
	favoriteView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::STRING, 60);
	favoriteView.insertColumn(_("Password"), G_TYPE_STRING, TreeView::STRING, 10);
	favoriteView.insertColumn(_("User Description"), G_TYPE_STRING, TreeView::STRING, 50);
	favoriteView.insertColumn(_("Encoding"), G_TYPE_STRING, TreeView::STRING, 80);
//	favoriteView.insertColumn(_("Group"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::STRING, 60);
	favoriteView.insertHiddenColumn("Hidden Password", G_TYPE_STRING);
	favoriteView.insertHiddenColumn("FavPointer",G_TYPE_POINTER);
	favoriteView.insertHiddenColumn("Action", G_TYPE_INT);
	favoriteView.finalize();
	favoriteStore = gtk_list_store_newv(favoriteView.getColCount(), favoriteView.getGTypes());
	gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));
	g_object_unref(favoriteStore);
	favoriteSelection = gtk_tree_view_get_selection(favoriteView.get());
	gtk_tree_view_set_search_column(favoriteView.get(), favoriteView.col(_("Name")));
	gtk_tree_selection_set_mode (favoriteSelection, GTK_SELECTION_SINGLE);

	// Initialize favorite hub groups list treeview
/*	groupsView.setView(GTK_TREE_VIEW(getWidget("groupsTreeView")));
	groupsView.insertColumn(_("Group name"), G_TYPE_STRING, TreeView::STRING, 150);
	groupsView.insertColumn(_("Connect"), G_TYPE_STRING, TreeView::STRING, 100);
	groupsView.insertHiddenColumn("Nick", G_TYPE_STRING);
	groupsView.insertHiddenColumn("eMail", G_TYPE_STRING);
	groupsView.insertHiddenColumn("Desc", G_TYPE_STRING);
	groupsView.insertHiddenColumn("Parts", G_TYPE_STRING);
	groupsView.insertHiddenColumn("FavParts", G_TYPE_STRING);
	groupsView.insertHiddenColumn("LogChat", G_TYPE_BOOLEAN);
	groupsView.insertHiddenColumn("AwayMessage", G_TYPE_STRING);
	groupsView.insertHiddenColumn("Connect hub", G_TYPE_BOOLEAN);
	groupsView.finalize();
/*
	groupsStore = gtk_list_store_newv(groupsView.getColCount(), groupsView.getGTypes());
	gtk_tree_view_set_model(groupsView.get(), GTK_TREE_MODEL(groupsStore));
	g_object_unref(groupsStore);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(groupsStore), groupsView.col(_("Group name")), GTK_SORT_ASCENDING);
	groupsSelection = gtk_tree_view_get_selection(groupsView.get());
*/
	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("buttonNew"), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonConnect"), "clicked", G_CALLBACK(onConnect_gui_b), (gpointer)this);
	g_signal_connect(getWidget("buttonProperties"), "clicked", G_CALLBACK(onEditEntry_gui_b), (gpointer)this);
	g_signal_connect(getWidget("buttonRemove"), "clicked", G_CALLBACK(onRemoveEntry_gui_b), (gpointer)this);
/*
	g_signal_connect(getWidget("addGroupButton"), "clicked", G_CALLBACK(onAddGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("updateGroupButton"), "clicked", G_CALLBACK(onUpdateGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeGroupButton"), "clicked", G_CALLBACK(onRemoveGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("manageGroupsButton"), "clicked", G_CALLBACK(onManageGroupsClicked_gui), (gpointer)this);
	g_signal_connect(groupsView.get(), "button-release-event", G_CALLBACK(onGroupsButtonReleased_gui), (gpointer)this);
	g_signal_connect(groupsView.get(), "key-release-event", G_CALLBACK(onGroupsKeyReleased_gui), (gpointer)this);
*/

/* Register for mouse right button click "pressed" and "released" events on  widget*/
	GtkGesture *gesture;
  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
  g_signal_connect (gesture, "pressed",
                    G_CALLBACK (on_right_btn_pressed), (gpointer)this);
  g_signal_connect (gesture, "released",
                    G_CALLBACK (on_right_btn_released), (gpointer)this);
  gtk_widget_add_controller (GTK_WIDGET(favoriteView.get()), GTK_EVENT_CONTROLLER (gesture));
//-------keys stuff
  GtkEventController* keys = gtk_event_controller_key_new ();

  g_signal_connect (keys, "key-pressed",
                    G_CALLBACK (key_pressed_gui), (gpointer)this);
  g_signal_connect (keys, "key-released",
                    G_CALLBACK (key_released_gui), (gpointer)this);
  gtk_widget_add_controller (GTK_WIDGET(favoriteView.get()), GTK_EVENT_CONTROLLER (keys));

}

gboolean FavoriteHubs::key_pressed_gui ( GtkEventControllerKey* self,  guint keyval,  guint keycode,  GdkModifierType state,  gpointer data )
{
	g_debug( "key_pressed");
	if( keyval == GDK_KEY_Return  ) {
			onConnect_gui_b(NULL, data);
			return TRUE;
	}
	return FALSE;
}

void FavoriteHubs::key_released_gui (  GtkEventControllerKey* self,  guint keyval,  guint keycode,  GdkModifierType state,  gpointer user_data )
{
	g_debug( "key_released");
	
}

void FavoriteHubs::on_right_btn_pressed (GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	FavoriteHubs *FH = (FavoriteHubs*)data;

	GMenu *menu = g_menu_new ();
	GMenuItem *menu_item_add = g_menu_item_new ("Add", "fav.add");
	g_menu_append_item (menu, menu_item_add);
	g_object_unref (menu_item_add);

	GMenuItem* menu_item_edit = g_menu_item_new ("Edit", "fav.edit");
	g_menu_append_item (menu, menu_item_edit);
	g_object_unref (menu_item_edit);

	GMenuItem* menu_item_conn = g_menu_item_new ("Connect", "fav.connect");
	g_menu_append_item (menu, menu_item_conn);
	g_object_unref (menu_item_conn);

	GMenuItem* menu_item_copy = g_menu_item_new ("Copy URL", "fav.copy-url");
	g_menu_append_item (menu, menu_item_copy);
	g_object_unref (menu_item_copy);

	GMenuItem* menu_item_rem = g_menu_item_new ("Remove", "fav.remove");
	g_menu_append_item (menu, menu_item_rem);
	g_object_unref (menu_item_rem);

	GMenuItem* menu_item_adv = g_menu_item_new ("Edit Advanced Settings", "fav.advanced-settings");
	g_menu_append_item (menu, menu_item_adv);
	g_object_unref (menu_item_adv);

	GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	gtk_widget_set_parent(pop, FH->getContainer());
	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
	gtk_popover_popup (GTK_POPOVER(pop));

}

void FavoriteHubs::on_right_btn_released (GtkGestureClick *gesture,
                                    int             /* n_press*/,
                                    double          /* x*/,
                                    double           /*y*/,
                                    gpointer*       /*widget*/)
{
  gtk_gesture_set_state (GTK_GESTURE (gesture),
                         GTK_EVENT_SEQUENCE_CLAIMED);
}

FavoriteHubs::~FavoriteHubs()
{
	FavoriteManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);
}

void FavoriteHubs::show()
{
	initializeList_client();
	FavoriteManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);
}

void FavoriteHubs::addEntry_gui(FavoriteHubEntry* entry)
{
	GtkTreeIter iter;
	gtk_list_store_append(favoriteStore, &iter);
	editEntry_gui(entry,&iter);
	HubsIter.insert(UnMapIter::value_type(entry->getServer(),iter));
}

void FavoriteHubs::editEntry_gui(FavoriteHubEntry* entry, GtkTreeIter *iter)
{
	string password = entry->getPassword().empty() ? "" : string(6, '*');

	gtk_list_store_set(favoriteStore, iter,
		favoriteView.col(_("Name")), entry->getName().c_str(),
		favoriteView.col(_("Description")), entry->getHubDescription().c_str(),
		favoriteView.col(_("Nick")),  entry->get(SettingsManager::NICK,SETTING(NICK)).c_str(),
		favoriteView.col(_("Password")), password.c_str(),
		favoriteView.col("Hidden Password"), entry->getPassword().c_str(),
		favoriteView.col(_("Address")), entry->getServer().c_str(),
		favoriteView.col(_("User Description")), entry->get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION)).c_str(),
		favoriteView.col(_("Encoding")), entry->getEncoding().c_str(),
		favoriteView.col(_("Group")), entry->getGroup().c_str(),
		favoriteView.col(_("Status")), ClientManager::getInstance()->isHubConnected(entry->getServer()) ? _("Online") : _("Offline"),
		favoriteView.col("FavPointer"), entry,
		favoriteView.col("Action"), 0,
		-1);
}

void FavoriteHubs::removeEntry_gui(string address)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (favoriteView.getString(&iter, _("Address")) == address)
		{
			gtk_list_store_remove(favoriteStore, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

bool FavoriteHubs::showErrorDialog_gui(const string description, FavoriteHubs *fh)
{
//@todo: info bar?
	GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(fh->getWidget("favoriteHubsDialog")),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());

	return true;
}

void FavoriteHubs::popupMenu_gui()
{
	if (!gtk_tree_selection_get_selected(favoriteSelection, NULL, NULL))
	{
		gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("removeMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("connectMenuItem"), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("removeMenuItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("connectMenuItem"), TRUE);
	}
}

gboolean FavoriteHubs::onButtonPressed_gui(GtkWidget*, GdkEvent *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	fh->previous = gdk_event_get_event_type(event);
	return FALSE;
}

gboolean FavoriteHubs::onButtonReleased_gui(GtkWidget*, GdkEvent*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);
	}

	return FALSE;
}
/*
gboolean FavoriteHubs::onKeyReleased_gui(GtkWidget* widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);

		if (event->keyval == GDK_KEY_Return)
		{
			WulforManager::get()->getMainWindow()->showHub_gui(
				fh->favoriteView.getString(&iter, _("Address")),
				fh->favoriteView.getString(&iter, _("Encoding")));
		}
		else if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_BackSpace)
		{
			fh->onRemoveEntry_gui(widget, data);
		}
		else if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
		{
			fh->popupMenu_gui();
		}
	}

	return FALSE;
}
*/
void FavoriteHubs::onAddEntry_gui(GtkWidget*, gpointer data)
{

	FavoriteHubs *fh = (FavoriteHubs *)data;
	FavoriteHubEntry entry;
	FavoriteHubDialog *f = new FavoriteHubDialog(&entry);
	
	if(fh->checkAddys(entry.getServer()))
	{
		fh->showErrorDialog_gui(_("Duplicate entries are not allowed"),fh);
	}

}

bool FavoriteHubs::checkAddys(const string url)
{
	string tmp = url;
	size_t needle = tmp.find("dchub://");
	if(needle == string::npos)
		return false;

	string newhubaddy = tmp.substr(needle);
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (favoriteView.getString(&iter, _("Address")) == newhubaddy)
		{
			return true;//not add to 
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}

	return false;
}

void FavoriteHubs::onEditEntry_gui(GtkWidget*,GVariant *, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if ( gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter) )
	{	

		FavoriteHubEntry* entry = (FavoriteHubEntry *)fh->favoriteView.getValue<gpointer>(&iter, "FavPointer");
		FavoriteHubDialog* f = new FavoriteHubDialog(entry,true);
		{
			fh->editEntry_gui(f->getEntryFav(),&iter);
		}
	}
}

void FavoriteHubs::onManageGroupsClicked_gui(GtkWidget*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

//	GtkWidget *dialog = fh->getWidget("FavoriteHubGroupsDialog");
//	fh->initFavHubGroupsDialog_gui();
//	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//	if (response == GTK_RESPONSE_OK)
//	{
//		fh->updateFavHubGroups_gui(true);
//		fh->saveFavHubGroups();
//	}
	//else
	//	fh->updateFavHubGroups_gui(false);
}

void FavoriteHubs::initFavHubGroupsDialog_gui()
{
	FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

	GtkTreeIter iter;
	gtk_list_store_clear(groupsStore);
	gtk_editable_set_text(GTK_EDITABLE(getWidget("nameGroupEntry")), "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("connectAllHubsCheckButton")), FALSE);

	for (auto i = favHubGroups.begin(); i != favHubGroups.end(); ++i)
	{
		// favorite hub groups list
		bool showJoins = i->second.get(SettingsManager::SHOW_JOINS, SETTING(SHOW_JOINS));
		bool FavShowJoins = i->second.get(SettingsManager::FAV_SHOW_JOINS, SETTING(FAV_SHOW_JOINS));
		bool log = i->second.get(SettingsManager::LOG_CHAT_B, SETTING(LOG_CHAT_B));
		bool connect = i->second.getAutoConnect();

		gtk_list_store_append(groupsStore, &iter);
		gtk_list_store_set(groupsStore, &iter,
			groupsView.col(_("Group name")), i->first.c_str(),
			groupsView.col(_("Connect")), connect == 1 ? _("Yes") : _("No"),
			groupsView.col("Nick"), i->second.get(SettingsManager::NICK,SETTING(NICK)).c_str(),
			groupsView.col("eMail"), i->second.get(SettingsManager::EMAIL,SETTING(EMAIL)).c_str(),
			groupsView.col("Desc"), i->second.get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION)).c_str(),
			groupsView.col("Parts"), Util::toString((int)showJoins).c_str() ,//todo
			groupsView.col("FavParts"),Util::toString((int)FavShowJoins).c_str(),
			groupsView.col("Connect hub"), (gboolean)connect,
			groupsView.col("LogChat"), (gboolean)log ,
			groupsView.col("AwayMessage"), i->second.get(SettingsManager::DEFAULT_AWAY_MESSAGE,SETTING(DEFAULT_AWAY_MESSAGE)).c_str(),
			-1);
	}
}

void FavoriteHubs::onRemoveEntry_gui(GtkWidget*,GVariant*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		/*if (SETTING(CONFIRM_HUB_REMOVAL))
		{
			string name = fh->favoriteView.getString(&iter, _("Name")).c_str();
			GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete favorite hub \"%s\"?"), name.c_str());

			gtk_dialog_add_buttons(GTK_DIALOG(dialog), "_Cancel", GTK_RESPONSE_CANCEL, "_Yes", GTK_RESPONSE_YES, NULL);

//			gint response = gtk_dialog_run(GTK_DIALOG(dialog));

//			if (response != GTK_RESPONSE_YES)
//				return;
		}*/
		string address = fh->favoriteView.getString(&iter, _("Address"));
		typedef Func1<FavoriteHubs, string> F1;
		F1 *func = new F1(fh, &FavoriteHubs::removeEntry_client, address);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onCopyAddress(GtkWidget*,GVariant*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		string address = fh->favoriteView.getString(&iter, _("Address"));

		GValue value = G_VALUE_INIT;
		g_value_init (&value, G_TYPE_STRING);
		g_value_set_string (&value, address.c_str());
		// Store the value in the clipboard object
		GdkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET(fh->favoriteView.get()));
		gdk_clipboard_set_value (clipboard, &value);

		g_value_unset (&value);

	}
}

void FavoriteHubs::onConnect_gui(GtkButton*,GVariant*,  gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		WulforManager::get()->getMainWindow()->showHub_gui(
			fh->favoriteView.getString(&iter, _("Address")),
			fh->favoriteView.getString(&iter, _("Encoding")));
}

void FavoriteHubs::onAddGroupClicked_gui(GtkWidget*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	const string group = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("nameGroupEntry")));

	if (group.empty())
	{
		showErrorDialog_gui(_("You must enter a name"), fh);
		return;
	}

	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(fh->groupsStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		string name = fh->groupsView.getString(&iter, _("Group name"));

		if (group == name)
		{
			showErrorDialog_gui(_("Another group with the name already exists"), fh);
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}

	bool connect_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")));
	bool log_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")));
	string nick = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryNickGroup")));
	string email = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryEmailGroup")));
	string desc = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryDescGroup")));
	int favShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")));
	int ShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoin")));
	string awayMsg = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryAwayGroup")));

	gtk_list_store_append(fh->groupsStore, &iter);
	gtk_list_store_set(fh->groupsStore, &iter,
		fh->groupsView.col(_("Group name")), group.c_str(),
		fh->groupsView.col(_("Connect")), connect_hub ? _("Yes") : _("No"),
		fh->groupsView.col("Nick"), nick.c_str(),
		fh->groupsView.col("eMail"), email.c_str(),
		fh->groupsView.col("Desc"), desc.c_str(),
		fh->groupsView.col("Parts"), ShowJoins == 1 ? "1" : ( ShowJoins >= 2 ? "2" : "0"),
		fh->groupsView.col("FavParts"), favShowJoins == 1 ? "1" : ( favShowJoins >= 2 ? "2" : "0"),
		fh->groupsView.col("LogChat"), (gboolean)log_hub,
		fh->groupsView.col("Connect hub"), (gboolean)connect_hub,
		fh->groupsView.col("AwayMessage"), awayMsg.c_str(),
		-1);
}

void FavoriteHubs::onRemoveGroupClicked_gui(GtkWidget*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter))
	{
		string group = fh->groupsView.getString(&iter, _("Group name"));

		GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_NONE,
			_("If you select 'Yes', all of these hubs are going to be deleted!\nIf you select 'No', these hubs will simply be moved to the main default group."));
		gtk_dialog_add_buttons(GTK_DIALOG(dialog), "_Cancel", GTK_RESPONSE_CANCEL, "_Yes",
			GTK_RESPONSE_YES, "_No", GTK_RESPONSE_NO, NULL);

//		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
//		if (response == GTK_RESPONSE_YES)
//		{
//			gtk_list_store_remove(fh->groupsStore, &iter);
//			fh->setFavoriteHubs_gui(TRUE, group);
//		}
//		else if (response == GTK_RESPONSE_NO)
//		{
//			gtk_list_store_remove(fh->groupsStore, &iter);
//			fh->setFavoriteHubs_gui(FALSE, group);
//		}
	}
}

void FavoriteHubs::updateFavHubGroups_gui(bool updated)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (updated)
		{
			string address = favoriteView.getString(&iter, _("Address"));
			int action = favoriteView.getValue<int>(&iter, "Action");

			if (action == 1)
			{
				// remove hub entry
				typedef Func1<FavoriteHubs, string> F1;
				F1 *func = new F1(this, &FavoriteHubs::removeEntry_client, address);
				WulforManager::get()->dispatchClientFunc(func);
			}
			else if (action == 2)
			{
				// moved hub entry to default group
				FavoriteHubEntry* entry = (FavoriteHubEntry*)favoriteView.getValue<gpointer>(&iter,"FavPointer");
				entry->setGroup(Util::emptyString);
				editEntry_gui(entry,&iter);
			}
		}
		gtk_list_store_set(favoriteStore, &iter, favoriteView.col("Action"), 0, -1);

		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FavoriteHubs::saveFavHubGroups()
{
	GtkTreeIter iter, it;
	GtkTreeModel *m = GTK_TREE_MODEL(groupsStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	GroupsIter.clear();

	FavHubGroups favHubGroups;

	while (valid)
	{
		string group = groupsView.getString(&iter, _("Group name"));

		GroupsIter.insert(UnMapIter::value_type(group, it));
		HubSettings p;

		bool log_hub = groupsView.getString(&iter, "LogChat") == "1" ? true : false;
		bool connect_hub = groupsView.getValue<gboolean>(&iter, "Connect hub");
		string nick = groupsView.getString(&iter, "Nick");
		string email = groupsView.getString(&iter, "eMail");
		string desc = groupsView.getString(&iter, "Desc");
		int favShowJoins = Util::toInt(groupsView.getString(&iter,"FavParts"));
		int showJoins = Util::toInt(groupsView.getString(&iter,"Parts"));
		string awayMsg = groupsView.getString(&iter, "AwayMessage");

		p.set(SettingsManager::NICK, nick);
		p.set(SettingsManager::EMAIL,email);
		p.set(SettingsManager::DESCRIPTION, desc);
		p.set(SettingsManager::FAV_SHOW_JOINS, favShowJoins);
		p.set(SettingsManager::SHOW_JOINS, showJoins);
		p.set(SettingsManager::LOG_CHAT_B, log_hub);
		p.setAutoConnect(connect_hub);
		p.set(SettingsManager::DEFAULT_AWAY_MESSAGE, awayMsg);

		favHubGroups.insert(FavHubGroup(group, p));

		valid = gtk_tree_model_iter_next(m, &iter);
	}
	FavoriteManager::getInstance()->setFavHubGroups(favHubGroups);
	FavoriteManager::getInstance()->save();
}

void FavoriteHubs::setFavoriteHubs_gui(bool remove, const string &group)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		string grp = favoriteView.getString(&iter, _("Group"));

		if (group == grp)
		{
			if (remove)
			{
				// remove hub entry
				gtk_list_store_set(favoriteStore, &iter, favoriteView.col("Action"), 1, -1);
			}
			else
			{
				// moved hub entry to default group
				gtk_list_store_set(favoriteStore, &iter, favoriteView.col("Action"), 2, -1);
			}
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FavoriteHubs::onUpdateGroupClicked_gui(GtkWidget*, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	GtkTreeIter iter;
	string group = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("nameGroupEntry")));

	if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter) && !group.empty())
	{
		bool connect_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")));
		bool log_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")));
		string nick = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryNickGroup")));
		string email = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryEmailGroup")));
		string desc = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryDescGroup")));
		int favShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")));
		int ShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoin")));
		string awayMsg = gtk_editable_get_text(GTK_EDITABLE(fh->getWidget("entryAwayGroup")));

		gtk_list_store_set(fh->groupsStore, &iter,
			fh->groupsView.col(_("Group name")), group.c_str(),
			fh->groupsView.col(_("Connect")), connect_hub ? _("Yes") : _("No"),
			fh->groupsView.col("Nick"), nick.c_str(),
			fh->groupsView.col("eMail"), email.c_str(),
			fh->groupsView.col("Desc"), desc.c_str(),
			fh->groupsView.col("Parts"), ShowJoins == 1 ? "1" : ( ShowJoins >= 2 ? "2" : "0"),
			fh->groupsView.col("FavParts"), favShowJoins == 1 ? "1" : ( favShowJoins >= 2 ? "2" : "0"),
			fh->groupsView.col("LogChat"), (gboolean)log_hub,
			fh->groupsView.col("Connect hub"), connect_hub,
			fh->groupsView.col("AwayMessage"), awayMsg.c_str(),
			-1);
	}
}
/*
gboolean FavoriteHubs::onGroupsKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter))
	{
		if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
		{
			string group = fh->groupsView.getString(&iter, _("Group name"));
			gboolean con = fh->groupsView.getValue<gboolean>(&iter, "Connect hub");
			string nick = fh->groupsView.getString(&iter,"Nick");
			string desc = fh->groupsView.getString(&iter, "Desc");
			string email = fh->groupsView.getString(&iter, "eMail");
			string parts = fh->groupsView.getString(&iter, "Parts");
			string favParts = fh->groupsView.getString(&iter, "FavParts");
			gboolean log_chat = fh->groupsView.getString(&iter, "LogChat") == "1" ? TRUE : FALSE;
			string awayMsg = fh->groupsView.getString(&iter, "AwayMessage");

			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")), con);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")), log_chat);
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")), group.c_str());

			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNickGroup")),nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescGroup")),desc.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryEmailGroup")),email.c_str());
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoin")),Util::toInt(parts));
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")),Util::toInt(favParts));

			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAwayGroup")),awayMsg.c_str());
		}
		else if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_BackSpace)
		{
			fh->onRemoveGroupClicked_gui(NULL, data);
		}
	}
	return FALSE;
}
*//*
gboolean FavoriteHubs::onGroupsButtonReleased_gui(GtkWidget*, GdkEvent *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	//if ( button == GDK_BUTTON_SECONDARY || button  == GDK_BUTTON_PRIMARY)
	{
		if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter))
		{
/*			string group = fh->groupsView.getString(&iter, _("Group name"));
			gboolean log_chat = fh->groupsView.getString(&iter, "LogChat") == "1" ? TRUE : FALSE;
			gboolean con = fh->groupsView.getValue<gboolean>(&iter, "Connect hub");
			string nick = fh->groupsView.getString(&iter,"Nick");
			string desc = fh->groupsView.getString(&iter, "Desc");
			string email = fh->groupsView.getString(&iter, "eMail");
			string parts = fh->groupsView.getString(&iter, "Parts");
			string favParts = fh->groupsView.getString(&iter, "FavParts");
			string awayMsg = fh->groupsView.getString(&iter, "AwayMessage");

			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")), log_chat);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")), con);
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")), group.c_str());

			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNickGroup")),nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescGroup")),desc.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryEmailGroup")),email.c_str());
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoin")),Util::toInt(parts));
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")),Util::toInt(favParts));

			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAwayGroup")),awayMsg.c_str());*/
	/*	}
	}
	return FALSE;
}
*/
void FavoriteHubs::edit_online_status(const string url,bool online)
{
	auto it = HubsIter.find(url);
	GtkTreeIter iter;

	if(it != HubsIter.end())
		 iter = it->second;

	gtk_list_store_set(favoriteStore,&iter,
		favoriteView.col(_("Status")), online ? _("Online") : _("Offline"),-1);

}

void FavoriteHubs::initializeList_client()
{
	const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

	for (auto it = fl.begin(); it != fl.end(); ++it)
	{
		addEntry_gui(*it);
	}
}

void FavoriteHubs::removeEntry_client(string address)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		FavoriteManager::getInstance()->removeFavorite(entry);
	}
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntryPtr entry) noexcept
{
	typedef Func1<FavoriteHubs, FavoriteHubEntry*> F1;
	F1 *func = new F1(this, &FavoriteHubs::addEntry_gui,entry);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntryPtr entry) noexcept
{
	typedef Func1<FavoriteHubs, string> F1;
	F1 *func = new F1(this, &FavoriteHubs::removeEntry_gui, entry->getServer());
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteHubs::on(ClientManagerListener::ClientConnected, Client* c) noexcept
{
	typedef Func2<FavoriteHubs,string,bool> F2;
	F2 *func = new F2(this,&FavoriteHubs::edit_online_status,c->getHubUrl(), true);
	WulforManager::get()->dispatchGuiFunc(func);

}
void FavoriteHubs::on(ClientManagerListener::ClientDisconnected, Client* c) noexcept
{

	typedef Func2<FavoriteHubs,string,bool> F2;
	F2 *func = new F2(this,&FavoriteHubs::edit_online_status,c->getHubUrl(), false);
	WulforManager::get()->dispatchGuiFunc(func);

}

void FavoriteHubs::onAdvancedSettings(GtkWidget*, GVariant*, gpointer data)
{
	FavoriteHubs* fh = (FavoriteHubs*)data;

	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		WulforManager::get()->getMainWindow()->showBook(Entry::ABOUT_CONFIG,
		new AboutConfigFav((FavoriteHubEntry*)fh->favoriteView.getValue<FavoriteHubEntry*>(&iter, _("FavPointer"))));

}

