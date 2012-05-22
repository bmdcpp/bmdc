/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2012 Mank freedcpp@seznam.cz
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
#include "WulforUtil.hh"
#include <dcpp/RawManager.h>

using namespace std;
using namespace dcpp;

FavoriteHubs::FavoriteHubs():
	BookEntry(Entry::FAVORITE_HUBS, _("Favorite Hubs"), "favoritehubs.glade")
{
	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("favoriteHubsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_widget_set_sensitive(getWidget("comboboxCharset"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryNick"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryUserDescription"), FALSE);

	// menu
	g_object_ref_sink(getWidget("menu"));

	gtk_window_set_transient_for(GTK_WINDOW(getWidget("favoriteHubsDialog")), GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(getWidget("favoriteHubsDialog")), TRUE);

	gtk_window_set_transient_for(GTK_WINDOW(getWidget("FavoriteHubGroupsDialog")),
		GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));

	// Fill the charset drop-down list in edit fav hub dialog.
	vector<string> &charsets = WulforUtil::getCharsets();
	for (vector<string>::const_iterator it = charsets.begin(); it != charsets.end(); ++it)
		gtk_combo_box_append_text(GTK_COMBO_BOX(getWidget("comboboxCharset")), it->c_str());

	// Initialize favorite hub list treeview
	favoriteView.setView(GTK_TREE_VIEW(getWidget("favoriteView")), TRUE, "favoritehubs");
	favoriteView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 200);
	favoriteView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 200);
	favoriteView.insertColumn(_("Address"), G_TYPE_STRING, TreeView::STRING, 175);
	favoriteView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn(_("Password"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn(_("User Description"), G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertColumn(_("Encoding"), G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertColumn(_("Group"), G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertHiddenColumn("Hidden Password", G_TYPE_STRING);
	favoriteView.insertHiddenColumn("Hide", G_TYPE_INT);
	favoriteView.insertHiddenColumn("Clients", G_TYPE_INT);
    favoriteView.insertHiddenColumn("Filelists", G_TYPE_INT);
    favoriteView.insertHiddenColumn("onConnect", G_TYPE_INT);
    favoriteView.insertHiddenColumn("Mode", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Auto Connect", G_TYPE_INT);
    favoriteView.insertHiddenColumn("IP", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("ExtraInfo", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Protected", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("eMail", G_TYPE_STRING);//
    favoriteView.insertHiddenColumn("Parts", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("FavParts", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("LogChat", G_TYPE_STRING);
	favoriteView.insertHiddenColumn("Action", G_TYPE_INT);
	favoriteView.finalize();
	favoriteStore = gtk_list_store_newv(favoriteView.getColCount(), favoriteView.getGTypes());
	gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));
	g_object_unref(favoriteStore);
	gtk_tree_view_set_fixed_height_mode(favoriteView.get(), TRUE);
	favoriteSelection = gtk_tree_view_get_selection(favoriteView.get());
	gtk_tree_view_set_search_column(favoriteView.get(), favoriteView.col(_("Name")));

	// Initialize favorite hub groups list treeview
	groupsView.setView(GTK_TREE_VIEW(getWidget("groupsTreeView")));
	groupsView.insertColumn(_("Group name"), G_TYPE_STRING, TreeView::STRING, 150);
//	groupsView.insertColumn(_("Private"), G_TYPE_STRING, TreeView::STRING, 100);
	groupsView.insertColumn(_("Connect"), G_TYPE_STRING, TreeView::STRING, 100);
//	groupsView.insertHiddenColumn("Private hub", G_TYPE_INT);
	groupsView.insertHiddenColumn("Nick", G_TYPE_STRING);
	groupsView.insertHiddenColumn("eMail", G_TYPE_STRING);
	groupsView.insertHiddenColumn("Desc", G_TYPE_STRING);
	groupsView.insertHiddenColumn("Parts", G_TYPE_STRING);
	groupsView.insertHiddenColumn("FavParts", G_TYPE_STRING);
	groupsView.insertHiddenColumn("LogChat", G_TYPE_STRING);
	groupsView.insertHiddenColumn("Connect hub", G_TYPE_INT);
	groupsView.finalize();

	groupsStore = gtk_list_store_newv(groupsView.getColCount(), groupsView.getGTypes());
	gtk_tree_view_set_model(groupsView.get(), GTK_TREE_MODEL(groupsStore));
	g_object_unref(groupsStore);
	gtk_tree_view_set_fixed_height_mode(groupsView.get(), TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(groupsStore), groupsView.col(_("Group name")), GTK_SORT_ASCENDING);
	groupsSelection = gtk_tree_view_get_selection(groupsView.get());
	
	///Actions
    actionView.setView(GTK_TREE_VIEW(getWidget("rawview")));
    actionView.insertColumn(_("Name"), G_TYPE_STRING,TreeView::STRING,100);
    actionView.insertColumn(_("Enabled"), G_TYPE_BOOLEAN, TreeView::BOOL,100);
    actionView.insertHiddenColumn("ISRAW", G_TYPE_BOOLEAN);
    actionView.insertHiddenColumn("ID", G_TYPE_INT);
    actionView.finalize();
    actionStore = gtk_tree_store_newv(actionView.getColCount(),actionView.getGTypes());
    gtk_tree_view_set_model(actionView.get(),GTK_TREE_MODEL(actionStore));
    g_object_unref(actionStore);
    actionSel = gtk_tree_view_get_selection(actionView.get());

    GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(actionView.get(), actionView.col(_("Enabled"))));
	GtkCellRenderer *arenderer = (GtkCellRenderer *)g_list_nth_data(list, 0);
	g_list_free(list);

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("buttonNew"), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonConnect"), "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonProperties"), "clicked", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonRemove"), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("addMenuItem"), "activate", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("connectMenuItem"), "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesMenuItem"), "activate", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("removeMenuItem"), "activate", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonEncoding"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("comboboxCharset"));
	g_signal_connect(getWidget("checkbuttonNick"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryNick"));
	g_signal_connect(getWidget("checkbuttonUserDescription"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryUserDescription"));
	g_signal_connect(getWidget("addGroupButton"), "clicked", G_CALLBACK(onAddGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("updateGroupButton"), "clicked", G_CALLBACK(onUpdateGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeGroupButton"), "clicked", G_CALLBACK(onRemoveGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("manageGroupsButton"), "clicked", G_CALLBACK(onManageGroupsClicked_gui), (gpointer)this);
	g_signal_connect(groupsView.get(), "button-release-event", G_CALLBACK(onGroupsButtonReleased_gui), (gpointer)this);
	g_signal_connect(groupsView.get(), "key-release-event", G_CALLBACK(onGroupsKeyReleased_gui), (gpointer)this);
	
	g_signal_connect(arenderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
}

FavoriteHubs::~FavoriteHubs()
{
	FavoriteManager::getInstance()->removeListener(this);

	gtk_widget_destroy(getWidget("favoriteHubsDialog"));
	gtk_widget_destroy(getWidget("FavoriteHubGroupsDialog"));
	g_object_unref(getWidget("menu"));
}

void FavoriteHubs::show()
{
	initializeList_client();
	FavoriteManager::getInstance()->addListener(this);
}

void FavoriteHubs::addEntry_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_list_store_append(favoriteStore, &iter);
	editEntry_gui(params, &iter);
}

void FavoriteHubs::editEntry_gui(StringMap &params, GtkTreeIter *iter)
{
	string password = params["Password"].empty() ? "" : string(8, '*');

	gtk_list_store_set(favoriteStore, iter,
		favoriteView.col(_("Name")), params["Name"].c_str(),
		favoriteView.col(_("Description")), params["Description"].c_str(),
		favoriteView.col(_("Nick")), params["Nick"].c_str(),
		favoriteView.col(_("Password")), password.c_str(),
		favoriteView.col("Hidden Password"), params["Password"].c_str(),
		favoriteView.col(_("Address")), params["Address"].c_str(),
		favoriteView.col(_("User Description")), params["User Description"].c_str(),
		favoriteView.col(_("Encoding")), params["Encoding"].c_str(),
		favoriteView.col(_("Group")), params["Group"].c_str(),
		favoriteView.col("Mode"), params["Mode"].c_str(),
		favoriteView.col("IP"), params["IP"].c_str(),
		favoriteView.col("ExtraInfo"), params["ExtraInfo"].c_str(),
		favoriteView.col("Hide"), Util::toInt(params["Hide"]),
		favoriteView.col("Auto Connect"), Util::toInt(params["Auto Connect"]),
		favoriteView.col("Clients"), Util::toInt(params["Clients"]),
		favoriteView.col("Filelists"), Util::toInt(params["Filelists"]),
		favoriteView.col("onConnect"), Util::toInt(params["OnConnect"]),
		favoriteView.col("Protected"), params["Protected"].c_str(),
		favoriteView.col("eMail"), params["eMail"].c_str(),
		favoriteView.col("Parts"), params["Parts"].c_str(),
		favoriteView.col("FavParts"), params["FavParts"].c_str(),
		favoriteView.col("LogChat"), params["LogChat"].c_str(),
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

bool FavoriteHubs::showErrorDialog_gui(const string &description, FavoriteHubs *fh)
{
	GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(fh->getWidget("favoriteHubsDialog")),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;

	gtk_widget_destroy(dialog);

	return TRUE;
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
	gtk_menu_popup(GTK_MENU(getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean FavoriteHubs::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	fh->previous = event->type;
	return FALSE;
}

gboolean FavoriteHubs::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
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

		if (fh->previous == GDK_BUTTON_PRESS && event->button == 3)
		{
			fh->popupMenu_gui();
		}
		else if (fh->previous == GDK_2BUTTON_PRESS && event->button == 1)
		{
			WulforManager::get()->getMainWindow()->showHub_gui(
				fh->favoriteView.getString(&iter, _("Address")),
				fh->favoriteView.getString(&iter, _("Encoding")));
		}
	}

	return FALSE;
}

gboolean FavoriteHubs::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);

		if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			WulforManager::get()->getMainWindow()->showHub_gui(
				fh->favoriteView.getString(&iter, _("Address")),
				fh->favoriteView.getString(&iter, _("Encoding")));
		}
		else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			fh->onRemoveEntry_gui(widget, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			fh->popupMenu_gui();
		}
	}

	return FALSE;
}

void FavoriteHubs::onAddEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	StringMap params;
	params["Name"] = Util::emptyString;
	params["Address"] = Util::emptyString;
	params["Description"] = Util::emptyString;
	params["Nick"] = Util::emptyString;
	params["Password"] = Util::emptyString;
	params["User Description"] = Util::emptyString;
	params["Encoding"] = Util::emptyString;
	params["Group"] = Util::emptyString;
	params["IP"] = Util::emptyString;
	params["Mode"] = "0";
	params["Auto Connect"] = "0";
	params["Hide"] = "0";
	params["Clients"] = "0";
	params["Filelists"] = "0";
	params["OnConnect"] = "0";
	params["ExtraInfo"] = Util::emptyString;
	params["Protected"] = Util::emptyString;
	params["eMail"] = Util::emptyString;
	params["Parts"] = Util::emptyString;
	params["FavParts"] = Util::emptyString;
	params["LogChat"] = Util::emptyString;

	bool updatedEntry = fh->showFavoriteHubDialog_gui(params, fh);

	if (updatedEntry)
	{
		typedef Func1<FavoriteHubs, StringMap> F1;
		F1 *func = new F1(fh, &FavoriteHubs::addEntry_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onEditEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		return;

	StringMap params;
	params["Name"] = fh->favoriteView.getString(&iter, _("Name"));
	params["Address"] = fh->favoriteView.getString(&iter, _("Address"));
	params["Description"] = fh->favoriteView.getString(&iter, _("Description"));
	params["Nick"] = fh->favoriteView.getString(&iter, _("Nick"));
	params["Password"] = fh->favoriteView.getString(&iter, "Hidden Password");
	params["User Description"] = fh->favoriteView.getString(&iter, _("User Description"));
	params["Encoding"] = fh->favoriteView.getString(&iter, _("Encoding"));
	params["Group"] = fh->favoriteView.getString(&iter, _("Group"));
	params["Mode"] = fh->favoriteView.getString(&iter, "Mode");
	params["Hide"] = Util::toString(fh->favoriteView.getValue<gint>(&iter, "Hide"));
	params["Auto Connect"] = Util::toString(fh->favoriteView.getValue<gint>(&iter, "Auto Connect"));
	params["Clients"] = Util::toString(fh->favoriteView.getValue<gint>(&iter, "Clients"));
	params["Filelists"] = Util::toString(fh->favoriteView.getValue<gint>(&iter, "Filelists"));;
	params["OnConnect"] = Util::toString(fh->favoriteView.getValue<gint>(&iter, "onConnect"));
	params["ExtraInfo"] = fh->favoriteView.getString(&iter, "ExtraInfo");
	params["Protected"] = fh->favoriteView.getString(&iter, "Protected");
	params["eMail"] = fh->favoriteView.getString(&iter, "eMail");
	params["Parts"] = fh->favoriteView.getString(&iter, "Parts");
	params["FavParts"] = fh->favoriteView.getString(&iter, "FavParts");
	params["LogChat"] = fh->favoriteView.getString(&iter, "LogChat");

	bool entryUpdated = showFavoriteHubDialog_gui(params, fh);

	if (entryUpdated)
	{
		string address_old = fh->favoriteView.getString(&iter, _("Address"));
		string address_new = params["Address"];

		if (fh->checkEntry_gui(address_old, address_new))
		{
			fh->editEntry_gui(params, &iter);

			typedef Func2<FavoriteHubs, string, StringMap> F2;
			F2 *func = new F2(fh, &FavoriteHubs::editEntry_client, address_old, params);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

bool FavoriteHubs::checkEntry_gui(string address_old, string address_new)
{
	if (address_old == address_new)
		return TRUE;

	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (favoriteView.getString(&iter, _("Address")) == address_new)
		{
			return FALSE;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}

	return TRUE;
}

void FavoriteHubs::initActions()
{
	GtkTreeIter toplevel;

	gtk_tree_store_clear(actionStore);

	const Action::ActionList& list = RawManager::getInstance()->getActions();

	for(Action::ActionList::const_iterator it = list.begin();it!= list.end();++it)
	{
		const string& name = (*it)->getName();

		gtk_tree_store_append(actionStore,&toplevel,NULL);
		gtk_tree_store_set(actionStore,&toplevel,
						actionView.col(_("Name")), name.c_str(),
						actionView.col(_("Enabled")), (*it)->getEnabled() ? TRUE : FALSE,
						actionView.col("ISRAW"), FALSE,
						actionView.col("ID"), (*it)->getId(),
						-1);

		GtkTreeIter child;

		for(Action::RawsList::const_iterator i = (*it)->raw.begin(); i != (*it)->raw.end(); ++i)
		{
			string rname = (*i).getName();
			gtk_tree_store_append(actionStore,&child,&toplevel);
			gtk_tree_store_set(actionStore,&child,
						actionView.col(_("Name")), rname.c_str(),
						actionView.col(_("Enabled")), (*i).getEnabled() ? TRUE : FALSE,
						actionView.col("ISRAW"), TRUE,
						actionView.col("ID"), (*i).getId(),
						-1);
		}
	}
}

void FavoriteHubs::setRawActions_gui(FavoriteHubs *fh, StringMap params)
{

	GtkTreeIter iter;

	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fh->actionStore), &iter);//??
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(params["Address"]);

	while(valid)
	{
		gint ida = actionView.getValue<gint>(&iter, "ID");
		gboolean isRaw = actionView.getValue<gboolean>(&iter, "ISRAW");
		bool isActive = FavoriteManager::getInstance()->getEnabledAction(&(*entry), ida);
		gtk_tree_store_set (actionStore, &iter,actionView.col(_("Enabled")), isActive, -1);

		if(!isRaw)
		{
			GtkTreeIter child;
			gboolean cvalid = gtk_tree_model_iter_children(GTK_TREE_MODEL(fh->actionStore), &child, &iter);
			while(cvalid)
			{
				gint idr = actionView.getValue<gint>(&child, "ID");
				bool isActive = FavoriteManager::getInstance()->getEnabledRaw(&(*entry), ida, idr);
				gtk_tree_store_set (fh->actionStore, &child,actionView.col(_("Enabled")), isActive, -1);

				cvalid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fh->actionStore), &child);
			}
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fh->actionStore), &iter);
	}
}

void FavoriteHubs::setRawActions_client(FavoriteHubs *fh, StringMap params)
{
	GtkTreeIter iter;

	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fh->actionStore), &iter);
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(params["Address"]);

	while(valid)
	{
		gint ida = actionView.getValue<gint>(&iter, "ID");
		gboolean isRaw = actionView.getValue<gboolean>(&iter, "ISRAW");
		gboolean active = actionView.getValue<gboolean>(&iter, _("Enabled"));
		if(active)
		{
			FavoriteManager::getInstance()->setEnabledAction(&(*entry), ida, true);
		}

		if(!isRaw)
		{
			GtkTreeIter child;
			gboolean cvalid = gtk_tree_model_iter_children(GTK_TREE_MODEL(fh->actionStore), &child, &iter);
			while(cvalid)
			{
				gint idr = actionView.getValue<gint>(&child, "ID");
				gboolean active = actionView.getValue<gboolean>(&child, _("Enabled"));
				if(active)
				{
                   FavoriteManager::getInstance()->setEnabledRaw(&(*entry), ida, idr, true);
				}
				cvalid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fh->actionStore), &child);
			}
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fh->actionStore), &iter);
	}
}

bool FavoriteHubs::showFavoriteHubDialog_gui(StringMap &params, FavoriteHubs *fh)
{
	fh->initActions();
	// Populate the dialog with initial values
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryName")), params["Name"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAddress")), params["Address"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescription")), params["Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNick")), params["Nick"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryPassword")), params["Password"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryUserDescription")), params["User Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("comboboxentryCharset")), params["Encoding"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryIp")), params["IP"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryExtraInfo")), params["ExtraInfo"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryprotected")), params["Protected"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryeMail")), params["eMail"].c_str());
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(fh->getWidget("comboboxMode")), Util::toInt64(params["Mode"]));
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(fh->getWidget("comboboxParts")), Util::toInt64(params["Parts"]));
	gtk_combo_box_set_active(GTK_COMBO_BOX(fh->getWidget("comboboxFavParts")), Util::toInt64(params["FavParts"]));

	FavHubGroupsIter::const_iterator it = fh->GroupsIter.find(params["Group"]);
	if (it != fh->GroupsIter.end())
	{
		GtkTreeIter iter = it->second;
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(fh->getWidget("groupsComboBox")), &iter);
	}
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(fh->getWidget("groupsComboBox")), 0);

	// Set the override default encoding checkbox. Check for "Global hub default"
	// for backwards compatability w/ 1.0.3. Should be removed at some point.
	gboolean overrideEncoding = !(params["Encoding"].empty() || params["Encoding"] == "Global hub default");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonEncoding")), overrideEncoding);

	// Set the override default nick checkbox
	gboolean overrideNick = !(params["Nick"].empty() || params["Nick"] == SETTING(NICK));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonNick")), overrideNick);

	// Set the override default user description checkbox
	gboolean overrideUserDescription = !(params["User Description"].empty() || params["User Description"] == SETTING(DESCRIPTION));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonUserDescription")), overrideUserDescription);
	
	// Set the auto connect checkbox
	gboolean autoConnect = params["Auto Connect"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonAutoConnect")), autoConnect);
	/*/*/
	gboolean hide = params["Hide"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkHide")), hide);
	
	gboolean fl = params["Filelists"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkFilelist")), fl);
	
	gboolean cl = params["Clients"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkClients")), cl);
	
	gboolean con = params["OnConnect"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkoncon")), con);
	
	gboolean log = params["LogChat"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkLog")), log);
	
	fh->setRawActions_gui(fh,params);
	// Show the dialog
	gint response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;

	while (response == GTK_RESPONSE_OK)
	{
		params.clear();
		params["Name"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryName")));
		params["Address"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryAddress")));
		params["Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescription")));
		params["Password"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryPassword")));
		params["Group"] = Util::emptyString;
		params["ExtraInfo"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryExtraInfo")));
		params["IP"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryIp")));
		params["Protected"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryprotected")));
		
		params["Mode"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxMode"))));
		params["Auto Connect"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonAutoConnect"))) ? "1" : "0";
		
		params["LogChat"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkLog"))) ? "1" : "0";
		
		params["Hide"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkHide"))) ? "1" : "0";
		params["OnConnect"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkoncon"))) ? "1" : "0";
		params["Filelists"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkFilelist"))) ? "1" : "0";
		params["Clients"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkClients"))) ? "1" : "0";
		params["eMail"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryeMail")));
		params["Parts"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxParts"))));
		params["FavParts"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxFavParts"))));
		
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("groupsComboBox"))) != 0)
		{
			gchar *group = gtk_combo_box_get_active_text(GTK_COMBO_BOX(fh->getWidget("groupsComboBox")));
			params["Group"] = string(group);
			g_free(group);
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonEncoding"))))
		{
			gchar *encoding = gtk_combo_box_get_active_text(GTK_COMBO_BOX(fh->getWidget("comboboxCharset")));
			params["Encoding"] = string(encoding);
			g_free(encoding);
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonNick"))))
		{
			params["Nick"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNick")));
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonUserDescription"))))
		{
			params["User Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryUserDescription")));
		}
		
		fh->setRawActions_client(fh,params);

		if (params["Name"].empty() || params["Address"].empty())
		{
			if (showErrorDialog_gui(_("The name and address fields are required"), fh))
			{
				response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));

				// Fix crash, if the dialog gets programmatically destroyed.
				if (response == GTK_RESPONSE_NONE)
					return FALSE;
			}
			else
				return FALSE;
		}
		else
		{
			gtk_widget_hide(fh->getWidget("favoriteHubsDialog"));
			return TRUE;
		}
	}

	gtk_widget_hide(fh->getWidget("favoriteHubsDialog"));
	return FALSE;
}

void FavoriteHubs::onManageGroupsClicked_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	GtkWidget *dialog = fh->getWidget("FavoriteHubGroupsDialog");
	fh->initFavHubGroupsDialog_gui();
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(dialog);

	if (response == GTK_RESPONSE_OK)
	{
		fh->updateFavHubGroups_gui(true);
		fh->saveFavHubGroups();
	}
	else
		fh->updateFavHubGroups_gui(false);
}

void FavoriteHubs::initFavHubGroupsDialog_gui()
{
	FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

	GtkTreeIter iter;
	gtk_list_store_clear(groupsStore);
	gtk_entry_set_text(GTK_ENTRY(getWidget("nameGroupEntry")), "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("connectAllHubsCheckButton")), FALSE);

	for (FavHubGroups::const_iterator i = favHubGroups.begin(); i != favHubGroups.end(); ++i)
	{
		// favorite hub groups list
		gtk_list_store_append(groupsStore, &iter);
		gtk_list_store_set(groupsStore, &iter,
			groupsView.col(_("Group name")), i->first.c_str(),
			groupsView.col(_("Connect")), i->second.getConnect() ? _("Yes") : _("No"),
			groupsView.col("Nick"), i->second.getNick().c_str(),
			groupsView.col("eMail"), i->second.getEmail().c_str(),
			groupsView.col("Desc"), i->second.getDescription().c_str(),
			groupsView.col("Parts"), toInt(i->second.showJoins) == 1 ? "1" : (toInt(i->second.showJoins) >= 2 ? "2" : "0"),
			groupsView.col("FavParts"), toInt(i->second.favShowJoins) == 1 ? "1" : (toInt(i->second.showJoins) >= 2 ? "2" : "0" ),
			groupsView.col("Connect hub"), i->second.getConnect(),
			groupsView.col("LogChat"), i->second.getLogChat() ? "1" : "0",
			-1);
			//Parts 1 = Enable 2 = disable 0 = def
	}
}

void FavoriteHubs::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		if (BOOLSETTING(CONFIRM_HUB_REMOVAL))
		{
			string name = fh->favoriteView.getString(&iter, _("Name")).c_str();
			GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete favorite hub \"%s\"?"), name.c_str());
			gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
			gint response = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			if (response != GTK_RESPONSE_YES)
				return;
		}

		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), FALSE);

		string address = fh->favoriteView.getString(&iter, _("Address"));

		typedef Func1<FavoriteHubs, string> F1;
		F1 *func = new F1(fh, &FavoriteHubs::removeEntry_client, address);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onConnect_gui(GtkButton *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		WulforManager::get()->getMainWindow()->showHub_gui(
			fh->favoriteView.getString(&iter, _("Address")),
			fh->favoriteView.getString(&iter, _("Encoding")));
}

void FavoriteHubs::onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
	GtkWidget *widget = (GtkWidget*)data;
	bool override = gtk_toggle_button_get_active(button);

	gtk_widget_set_sensitive(widget, override);

	if (override)
	{
		gtk_widget_grab_focus(widget);
	}
}

void FavoriteHubs::onAddGroupClicked_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	const string group = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")));

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
	string nick = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNickGroup")));
	string email = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryEmailGroup")));
	string desc = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescGroup")));
	int favShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")));
	int ShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoin")));

	gtk_list_store_append(fh->groupsStore, &iter);
	gtk_list_store_set(fh->groupsStore, &iter,
		fh->groupsView.col(_("Group name")), group.c_str(),
		fh->groupsView.col(_("Connect")), connect_hub ? _("Yes") : _("No"),
		fh->groupsView.col("Nick"), nick.c_str(),
		fh->groupsView.col("eMail"), email.c_str(),
		fh->groupsView.col("Desc"), desc.c_str(),
		fh->groupsView.col("Parts"), ShowJoins == 1 ? "1" : ( ShowJoins >= 2 ? "2" : "0"),
		fh->groupsView.col("FavParts"), favShowJoins == 1 ? "1" : ( favShowJoins >= 2 ? "2" : "0"),
		fh->groupsView.col("LogChat"), log_hub,
		fh->groupsView.col("Connect hub"), connect_hub,
		-1);
}

void FavoriteHubs::onRemoveGroupClicked_gui(GtkWidget *widget, gpointer data)
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

		gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_YES,
			GTK_RESPONSE_YES, GTK_STOCK_NO, GTK_RESPONSE_NO, NULL);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_NO, GTK_RESPONSE_CANCEL, -1);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		// if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return;

		if (response == GTK_RESPONSE_YES)
		{
			gtk_list_store_remove(fh->groupsStore, &iter);
			fh->setFavoriteHubs_gui(TRUE, group);
		}
		else if (response == GTK_RESPONSE_NO)
		{
			gtk_list_store_remove(fh->groupsStore, &iter);
			fh->setFavoriteHubs_gui(FALSE, group);
		}
		gtk_widget_destroy(dialog);
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
				StringMap params;
				params["Name"] = favoriteView.getString(&iter, _("Name"));
				params["Address"] = address;
				params["Description"] = favoriteView.getString(&iter, _("Description"));
				params["Nick"] = favoriteView.getString(&iter, _("Nick"));
				params["Password"] = favoriteView.getString(&iter, "Hidden Password");
				params["User Description"] = favoriteView.getString(&iter, _("User Description"));
				params["Encoding"] = favoriteView.getString(&iter, _("Encoding"));
				params["Group"] = Util::emptyString;
				params["Mode"] = favoriteView.getString(&iter, "Mode");
				params["Hide"] = Util::toString(favoriteView.getValue<gint>(&iter, "Hide"));
				params["Auto Connect"] = Util::toString(favoriteView.getValue<gint>(&iter, "Auto Connect"));
				params["Clients"] = Util::toString(favoriteView.getValue<gint>(&iter, "Clients"));
				params["Filelists"] = Util::toString(favoriteView.getValue<gint>(&iter, "Filelists"));;
				params["OnConnect"] = Util::toString(favoriteView.getValue<gint>(&iter, "onConnect"));
				params["ExtraInfo"] = favoriteView.getString(&iter, "ExtraInfo");
				params["IP"] = favoriteView.getString(&iter, "IP");
				params["LogChat"] = favoriteView.getString(&iter, "LogChat");

				editEntry_gui(params, &iter);

				typedef Func2<FavoriteHubs, string, StringMap> F2;
				F2 *func = new F2(this, &FavoriteHubs::editEntry_client, address, params);
				WulforManager::get()->dispatchClientFunc(func);
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

	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("groupsComboBox"))));
	gtk_list_store_clear(store);
	gtk_list_store_append(store, &it);
	gtk_list_store_set(store, &it, 0, _("Default"), -1);
	GroupsIter.clear();

	FavHubGroups favHubGroups;

	while (valid)
	{
		string group = groupsView.getString(&iter, _("Group name"));

		// favorite hub properties combo box groups
		gtk_list_store_append(store, &it);
		gtk_list_store_set(store, &it, 0, group.c_str(), -1);
		GroupsIter.insert(FavHubGroupsIter::value_type(group, it));

		bool log_hub = groupsView.getString(&iter, "LogChat") == "1" ? TRUE : FALSE; 
		bool connect_hub = groupsView.getValue<int>(&iter, "Connect hub");
		string nick = groupsView.getString(&iter, "Nick");
		string email = groupsView.getString(&iter, "eMail");
		string desc = groupsView.getString(&iter, "Desc");
		tribool favShowJoins = to3bool(Util::toInt(groupsView.getString(&iter,"FavParts")));
		tribool showJoins = to3bool(Util::toInt(groupsView.getString(&iter,"Parts")));

		HubSettings p;
		//p.connect = connect_hub;
		//p.priv = private_hub;
		p.setNick(nick);
		p.setEmail(email);
		p.setDescription(desc);
		p.favShowJoins = favShowJoins;
		p.showJoins = showJoins;
		p.setLogChat(log_hub);
		p.setConnect(connect_hub);
		
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

void FavoriteHubs::onUpdateGroupClicked_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	GtkTreeIter iter;
	string group = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")));

	if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter) && !group.empty())
	{
		bool connect_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")));
		bool log_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")));
		string nick = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNickGroup")));
		string email = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryEmailGroup")));
		string desc = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescGroup")));
		int favShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")));
		int ShowJoins = gtk_combo_box_get_active(GTK_COMBO_BOX(fh->getWidget("comboboxJoin")));
		
		gtk_list_store_set(fh->groupsStore, &iter,
			fh->groupsView.col(_("Group name")), group.c_str(),
			fh->groupsView.col(_("Connect")), connect_hub ? _("Yes") : _("No"),
			fh->groupsView.col("Nick"), nick.c_str(),
			fh->groupsView.col("eMail"), email.c_str(),
			fh->groupsView.col("Desc"), desc.c_str(),
			fh->groupsView.col("Parts"), ShowJoins == 1 ? "1" : ( ShowJoins >= 2 ? "2" : "0"),
			fh->groupsView.col("FavParts"), favShowJoins == 1 ? "1" : ( favShowJoins >= 2 ? "2" : "0"),
			fh->groupsView.col("LogChat"), log_hub ? "1" : "0",
			fh->groupsView.col("Connect hub"), connect_hub,
			-1);
	}
}

gboolean FavoriteHubs::onGroupsKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter))
	{
		if (event->keyval == GDK_Up || event->keyval == GDK_Down)
		{
			string group = fh->groupsView.getString(&iter, _("Group name"));
			gboolean con = fh->groupsView.getValue<gboolean>(&iter, "Connect hub");
			string nick = fh->groupsView.getString(&iter,"Nick");
			string desc = fh->groupsView.getString(&iter, "Desc");
			string email = fh->groupsView.getString(&iter, "eMail");
			string parts = fh->groupsView.getString(&iter, "Parts");
			string favParts = fh->groupsView.getString(&iter, "FavParts");
			gboolean log_chat = fh->groupsView.getString(&iter, "LogChat") == "1" ? TRUE : FALSE;

			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")), con);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")), log_chat);
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")), group.c_str());
			
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNickGroup")),nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescGroup")),desc.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryEmailGroup")),email.c_str());
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoin")),Util::toInt(parts));
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")),Util::toInt(favParts));
		}
		else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			fh->onRemoveGroupClicked_gui(NULL, data);
		}
	}
	return FALSE;
}

gboolean FavoriteHubs::onGroupsButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (event->button == 3 || event->button == 1)
	{
		if (gtk_tree_selection_get_selected(fh->groupsSelection, NULL, &iter))
		{
			string group = fh->groupsView.getString(&iter, _("Group name"));
			gboolean log_chat = fh->groupsView.getString(&iter, "LogChat") == "1" ? TRUE : FALSE;
			gboolean con = fh->groupsView.getValue<gboolean>(&iter, "Connect hub");
			string nick = fh->groupsView.getString(&iter,"Nick");
			string desc = fh->groupsView.getString(&iter, "Desc");
			string email = fh->groupsView.getString(&iter, "eMail");
			string parts = fh->groupsView.getString(&iter, "Parts");
			string favParts = fh->groupsView.getString(&iter, "FavParts");
			
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("LogCheckButton")), log_chat);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")), con);
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")), group.c_str());
			
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNickGroup")),nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescGroup")),desc.c_str());
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryEmailGroup")),email.c_str());
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoin")),Util::toInt(parts));
			gtk_combo_box_set_active (GTK_COMBO_BOX(fh->getWidget("comboboxJoinFav")),Util::toInt(favParts));
		}
	}
	return FALSE;
}

void FavoriteHubs::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fh->actionStore), &iter, path))
	{
		string nane = fh->actionView.getString(&iter, _("Name"));
		bool fixed = fh->actionView.getValue<gboolean>(&iter, _("Enabled"));
		fixed = !fixed;
		gtk_tree_store_set(fh->actionStore, &iter, fh->actionView.col(_("Enabled")), fixed, -1);
	}
}

void FavoriteHubs::initializeList_client()
{
	StringMap params;
	typedef Func1<FavoriteHubs, StringMap> F1;
	const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();
	FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("groupsComboBox"))));

	for (FavHubGroups::const_iterator i = favHubGroups.begin(); i != favHubGroups.end(); ++i)
	{
		// favorite hub properties combo box groups
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, i->first.c_str(), -1);
		GroupsIter.insert(FavHubGroupsIter::value_type(i->first, iter));
	}

	for (FavoriteHubEntryList::const_iterator it = fl.begin(); it != fl.end(); ++it)
	{
		getFavHubParams_client(*it, params);
		addEntry_gui(params);
	}
}

void FavoriteHubs::getFavHubParams_client(const FavoriteHubEntry *entry, StringMap &params)
{
	params["Name"] = entry->getName();
	params["Description"] = entry->getDescription();
	params["Nick"] = entry->getNick(); 
	params["Password"] = entry->getPassword();
	params["Address"] = entry->getServer();
	params["User Description"] = entry->getUserDescription();
	params["Encoding"] = entry->getEncoding();
	params["Group"] = entry->getGroup();
	params["Filelists"] = entry->getCheckFilelists() ? "1" : "0";
	params["Clients"] = entry->getCheckClients() ? "1" : "0";
	params["OnConnect"] = entry->getCheckAtConn() ? "1" : "0";
	params["ExtraInfo"] = entry->getChatExtraInfo();
	params["Mode"] = Util::toString(entry->getMode());
	params["IP"] = entry->getIp();
	params["Hide"] = entry->getHideShare() ? "1" : "0";
	params["Auto Connect"] = entry->getAutoConnect() ? "1" : "0";
	params["Protected"] = entry->getProtectUsers();
	params["eMail"] = entry->getEmail();
	params["Parts"] = toInt(entry->showJoins) == 1 ? "1" : (toInt(entry->showJoins) >= 2 ? "2" : "0");
	params["FavParts"] = toInt(entry->favShowJoins) == 1 ? "1" : (toInt(entry->favShowJoins) >= 2 ? "2" : "0");
	params["LogChat"] = entry->getLogChat() ? "1" : "0";
	
}

void FavoriteHubs::addEntry_client(StringMap params)
{
	FavoriteHubEntry entry;
	entry.setName(params["Name"]);
	entry.setServer(params["Address"]);
	entry.setDescription(params["Description"]);
	entry.setNick(params["Nick"]);
	entry.setPassword(params["Password"]);
	entry.setUserDescription(params["User Description"]);
	entry.setEncoding(params["Encoding"]);
	entry.setGroup(params["Group"]);
	entry.setHideShare(Util::toInt(params["Hide"]));
	entry.setMode(Util::toInt(params["Mode"]));
	entry.setIp(params["IP"]);
	entry.setEmail(params["eMail"]);
	entry.showJoins = to3bool(Util::toInt(params["Parts"]));
	entry.favShowJoins = to3bool(Util::toInt(params["FavParts"]));
	
	entry.setCheckClients(Util::toInt(params["Clients"]));
	entry.setCheckFilelists(Util::toInt(params["Filelists"]));
	entry.setCheckAtConn(Util::toInt(params["OnConnect"]));
	entry.setChatExtraInfo(params["ExtraInfo"]);
	entry.setAutoConnect(Util::toInt(params["Auto Connect"]));
	entry.setProtectUsers(params["Protected"]);
	
	entry.setLogChat(Util::toInt(params["LogChat"]));

	FavoriteManager::getInstance()->addFavorite(entry);

	const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
	WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);
}

void FavoriteHubs::editEntry_client(string address, StringMap params)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		entry->setName(params["Name"]);
		entry->setServer(params["Address"]);
		entry->setDescription(params["Description"]);
		entry->setNick(params["Nick"]);
		entry->setPassword(params["Password"]);
		entry->setUserDescription(params["User Description"]);
		entry->setEncoding(params["Encoding"]);
		entry->setGroup(params["Group"]);
		entry->setMode(Util::toInt(params["Mode"]));
		entry->setIp(params["IP"]);
		entry->setHideShare(Util::toInt(params["Hide"]));
		
		entry->setEmail(params["eMail"]);
		tribool showjoin = to3bool(Util::toInt(params["Parts"]));
		entry->showJoins = showjoin;
		tribool favJoin = to3bool(Util::toInt(params["FavParts"]));
		entry->favShowJoins = favJoin;
		
		entry->setCheckClients(Util::toInt(params["Clients"]));
		entry->setCheckFilelists(Util::toInt(params["Filelists"]));
		entry->setCheckAtConn(Util::toInt(params["OnConnect"]));
		entry->setChatExtraInfo(params["ExtraInfo"]);
		entry->setAutoConnect(Util::toInt(params["Auto Connect"]));
		entry->setProtectUsers(params["Protected"]);
		
		entry->setLogChat(Util::toInt(params["LogChat"]));

		FavoriteManager::getInstance()->save();

		const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
		WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);
	}
}

void FavoriteHubs::removeEntry_client(string address)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		FavoriteManager::getInstance()->removeFavorite(entry);

		const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
		WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);
	}
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntryPtr entry) throw()
{
	StringMap params;
	getFavHubParams_client(entry, params);

	typedef Func1<FavoriteHubs, StringMap> F1;
	F1 *func = new F1(this, &FavoriteHubs::addEntry_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntryPtr entry) throw()
{
	typedef Func1<FavoriteHubs, string> F1;
	F1 *func = new F1(this, &FavoriteHubs::removeEntry_gui, entry->getServer());
	WulforManager::get()->dispatchGuiFunc(func);
}
