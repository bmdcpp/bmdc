/* Parts of Code 2010-2011 (C) Mank (freedcpp@seznam.cz)*/
/* Part of Code (C) 2010 troll.freedcpp*/
#include "favoritehubs.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

#include <dcpp/FavHubGroup.h>
#include <dcpp/RawManager.h>

using namespace std;
using namespace dcpp;

FavoriteHubs::FavoriteHubs():
    BookEntry(Entry::FAVORITE_HUBS,_("Favorite Hubs"),"favoritehubs.glade")
{
    // Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("favoriteHubsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_widget_set_sensitive(getWidget("entryNick"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryUserDescription"), FALSE);
	gtk_widget_set_sensitive(getWidget("comboboxCharset"), FALSE);
	gtk_widget_set_sensitive(getWidget("buttonGroups"), TRUE);
	
	vector<string> &charsets = WulforUtil::getCharsets();
	WulforUtil::drop_combo(getWidget("comboboxCharset"), charsets);
	
	favoriteView.setView(GTK_TREE_VIEW(getWidget("favoriteView")));
	favoriteView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 200);
	favoriteView.insertColumn(_("Address"), G_TYPE_STRING, TreeView::STRING, 100);
    favoriteView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 200);
    favoriteView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::STRING, 100);
    favoriteView.insertColumn(_("User Description"), G_TYPE_STRING, TreeView::STRING, 100);
    favoriteView.insertColumn(_("Password"), G_TYPE_STRING,TreeView::STRING, 100);
    favoriteView.insertColumn(_("Encoding"), G_TYPE_STRING, TreeView::STRING, 50);
    favoriteView.insertColumn(_("Extra Info"), G_TYPE_STRING, TreeView::STRING, 60);
	favoriteView.insertHiddenColumn("Group", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Hide", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Log", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Clients", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Filelists", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("onConnects", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Mode", G_TYPE_INT);
    favoriteView.insertHiddenColumn("IP", G_TYPE_STRING);
    favoriteView.insertHiddenColumn("Action", G_TYPE_INT);
    favoriteView.insertHiddenColumn("HidePassword", G_TYPE_STRING);
    favoriteView.finalize();
    favoriteStore = gtk_tree_store_newv(favoriteView.getColCount(),favoriteView.getGTypes());
    gtk_tree_view_set_model(favoriteView.get(),GTK_TREE_MODEL(favoriteStore));
    g_object_unref(favoriteStore);
    favoriteSel = gtk_tree_view_get_selection(favoriteView.get());

    ///Actions
    actionView.setView(GTK_TREE_VIEW(getWidget("treeviewActions")));
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
    
    // Initialize favorite hub groups list treeview
	groupView.setView(GTK_TREE_VIEW(getWidget("treeviewgrp")));
	groupView.insertColumn(_("Group name"), G_TYPE_STRING, TreeView::STRING, 150);
	groupView.insertColumn(_("Private"), G_TYPE_STRING, TreeView::STRING, 100);
	groupView.insertColumn(_("Connect"), G_TYPE_STRING, TreeView::STRING, 100);
	groupView.insertHiddenColumn("Private hub", G_TYPE_INT);
	groupView.insertHiddenColumn("Connect hub", G_TYPE_INT);
	groupView.finalize();
	groupStore = gtk_list_store_newv(groupView.getColCount(), groupView.getGTypes());
	gtk_tree_view_set_model(groupView.get(), GTK_TREE_MODEL(groupStore));
	g_object_unref(groupStore);
	gtk_tree_view_set_fixed_height_mode(groupView.get(), TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(groupStore), groupView.col(_("Group name")), GTK_SORT_ASCENDING);
	groupselection = gtk_tree_view_get_selection(groupView.get());
    //Callbacks
    g_signal_connect(getWidget("buttonNew"), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
    g_signal_connect(getWidget("buttonProperties"), "clicked", G_CALLBACK(onEditEntry_gui), (gpointer)this);
    g_signal_connect(getWidget("buttonRemove"), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
    g_signal_connect(getWidget("buttonConnect"), "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);
    
    g_signal_connect(getWidget("addMenuItem"), "activate", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("connectMenuItem"), "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesMenuItem"), "activate", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("removeMenuItem"), "activate", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	
	g_signal_connect(favoriteView.get(),"button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	
	///Fav Dialog
	g_signal_connect(getWidget("checkbuttonEncoding"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("comboboxCharset"));
	g_signal_connect(getWidget("checkbuttonNick"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryNick"));
	g_signal_connect(getWidget("checkbuttonUserDescription"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryUserDescription"));
	///Modes
	g_signal_connect(GTK_TOGGLE_BUTTON(getWidget("radiobuttonmode1")), "toggled", G_CALLBACK(onToggledMode_d_gui), this);
	g_signal_connect(GTK_TOGGLE_BUTTON(getWidget("radiobuttonmode2")), "toggled", G_CALLBACK(onToggledMode_p_gui), this);
	g_signal_connect(GTK_TOGGLE_BUTTON(getWidget("radiobuttonmode3")), "toggled", G_CALLBACK(onToggledMode_a_gui), this);
	///actions
	g_signal_connect(arenderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	///Groups
	g_signal_connect(getWidget("buttonAddGrp"), "clicked", G_CALLBACK(onAddGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonUpdate"), "clicked", G_CALLBACK(onUpdateGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("button1Remove"), "clicked", G_CALLBACK(onRemoveGroupClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonGroups"), "clicked", G_CALLBACK(onManageGroupsClicked_gui), (gpointer)this);
	g_signal_connect(groupView.get(), "button-release-event", G_CALLBACK(onGroupsButtonReleased_gui), (gpointer)this);
	g_signal_connect(groupView.get(), "key-release-event", G_CALLBACK(onGroupsKeyReleased_gui), (gpointer)this);
	
}

FavoriteHubs::~FavoriteHubs()
{
    FavoriteManager::getInstance()->removeListener(this);
}

void FavoriteHubs::show()
{
    initialze_client();
    FavoriteManager::getInstance()->addListener(this);
}

void FavoriteHubs::initialze_client()
{
  	FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

    GtkTreeIter iter,fiter;
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("groupsComboBox"))));
  
    for (FavHubGroups::const_iterator iq = favHubGroups.begin(); iq != favHubGroups.end(); ++iq)
	{
		// favorite hub properties combo box groups
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, iq->first.c_str(), -1);
		GroupsIter.insert(FavHubGroupsIter::value_type(iq->first, iter));
        //thinking
        gtk_tree_store_append(favoriteStore,&fiter,NULL);
        gtk_tree_store_set(favoriteStore,&fiter,
                           favoriteView.col(_("Name")), iq->first.c_str(),
                          -1);

		GtkTreeIter piter;
        const FavoriteHubEntryList& flist = FavoriteManager::getInstance()->getFavoriteHubs(iq->first);
        
        for(FavoriteHubEntryList::const_iterator it=flist.begin();it!=flist.end();++it)
        {

			string pass= (*it)->getPassword().empty()  ? "" : string(8,'*');

            gtk_tree_store_append(favoriteStore,&piter,&fiter);
            gtk_tree_store_set(favoriteStore,&piter,
                                favoriteView.col(_("Name")), (*it)->getName().c_str(),
                                favoriteView.col(_("Address")), (*it)->getServer().c_str(),
                                favoriteView.col(_("Description")), (*it)->getDescription().c_str(),
                                favoriteView.col(_("Nick")), (*it)->getNick().c_str(),
								favoriteView.col(_("User Description")), (*it)->getUserDescription().c_str(),
								favoriteView.col(_("Password")), pass.c_str(),
								favoriteView.col(_("Encoding")), (*it)->getEncoding().c_str(),
								favoriteView.col(_("Extra Info")), (*it)->getChatExtraInfo().c_str(),
								favoriteView.col("Group"), iq->first.c_str(),
								favoriteView.col("Hide"), (*it)->getHideShare() ? "1" : "0",
								favoriteView.col("Log"), (*it)->getLogChat() ? "1" : "0",
								favoriteView.col("Clients"), (*it)->getCheckClients() ? "1" : "0",
								favoriteView.col("Filelists"), (*it)->getCheckFilelists() ? "1" : "0",
								favoriteView.col("onConnects"), (*it)->getCheckOnConnect() ? "1" : "0",
								favoriteView.col("Mode"), (*it)->getMode(),
								favoriteView.col("IP"), (*it)->getIp().c_str(),
								favoriteView.col("HidePassword"), (*it)->getPassword().c_str(),
								-1);
					Iters *iw = new Iters();
					iw->main = fiter;
					iw->child = piter;
					faviters.insert(FavIter::value_type((*it)->getServer(),iw));
        }

    }
    
    const FavoriteHubEntryList& flist = FavoriteManager::getInstance()->getFavoriteHubs();
    for(FavoriteHubEntryList::const_iterator it = flist.begin();it!=flist.end();++it)
    {
		if((*it)->getGroup() == "" || (*it)->getGroup() == _("Default") || (*it)->getGroup() == "Default")
		{
		
			string pass= (*it)->getPassword().empty()  ? "" : string(8,'*');
				
			gtk_tree_store_append(favoriteStore,&fiter,NULL);
            gtk_tree_store_set(favoriteStore,&fiter,
                                favoriteView.col(_("Name")), (*it)->getName().c_str(),
                                favoriteView.col(_("Address")), (*it)->getServer().c_str(),
                                favoriteView.col(_("Description")), (*it)->getDescription().c_str(),
                                favoriteView.col(_("Nick")), (*it)->getNick().c_str(),
								favoriteView.col(_("User Description")), (*it)->getUserDescription().c_str(),
								favoriteView.col(_("Password")), pass.c_str(),
								favoriteView.col(_("Encoding")), (*it)->getEncoding().c_str(),
								favoriteView.col(_("Extra Info")), (*it)->getChatExtraInfo().c_str(),
								favoriteView.col("Group"), "",
								favoriteView.col("Hide"), (*it)->getHideShare() ? "1" : "0",
								favoriteView.col("Log"), (*it)->getLogChat() ? "1" : "0",
								favoriteView.col("Clients"), (*it)->getCheckClients() ? "1" : "0",
								favoriteView.col("Filelists"), (*it)->getCheckFilelists() ? "1" : "0",
								favoriteView.col("onConnects"), (*it)->getCheckOnConnect() ? "1" : "0",
								favoriteView.col("Mode"), (*it)->getMode(),
								favoriteView.col("IP"), (*it)->getIp().c_str(),
								favoriteView.col("HidePassword"), (*it)->getPassword().c_str(),
								-1);
			Iters *iw = new Iters();
			iw->main = fiter;
			
			faviters.insert(FavIter::value_type((*it)->getServer(),iw));
			
		}
	}
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
						actionView.col(_("Name")),rname.c_str(),
						actionView.col(_("Enabled")),(*i).getEnabled() ? TRUE : FALSE,
						actionView.col("ISRAW"),TRUE,
						actionView.col("ID"),(*i).getId(),
						-1);
				
		}
	}
}

void FavoriteHubs::initFavHubGroupsDialog_gui()
{
	FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

	GtkTreeIter iter;
	gtk_list_store_clear(groupStore);
	gtk_entry_set_text(GTK_ENTRY(getWidget("nameGroupEntry")), "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("privateHubsCheckButton")), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("connectAllHubsCheckButton")), FALSE);

	for (FavHubGroups::const_iterator i = favHubGroups.begin(); i != favHubGroups.end(); ++i)
	{
		// favorite hub groups list
		gtk_list_store_append(groupStore, &iter);
		gtk_list_store_set(groupStore, &iter,
			groupView.col(_("Group name")), i->first.c_str(),
			groupView.col(_("Private")), i->second.priv ? _("Yes") : _("No"),
			groupView.col(_("Connect")), i->second.connect ? _("Yes") : _("No"),
			groupView.col("Private hub"), i->second.priv,
			groupView.col("Connect hub"), i->second.connect,
			-1);
	}
}

void FavoriteHubs::addEntry_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_tree_store_append(favoriteStore, &iter,NULL);
	editEntry_gui(params, &iter);
}

void FavoriteHubs::editEntry_gui(StringMap &params, GtkTreeIter *iter)
{
	string password = params["Password"].empty() ? "" : string(8, '*');
	
	gtk_tree_store_set(favoriteStore, iter,
		favoriteView.col(_("Name")), params["Name"].c_str(),
		favoriteView.col(_("Description")), params["Description"].c_str(),
		favoriteView.col(_("Nick")), params["Nick"].c_str(),
		favoriteView.col(_("Password")), password.c_str(),
		favoriteView.col("HidePassword"), params["Password"].c_str(),
		favoriteView.col(_("Address")), params["Address"].c_str(),
		favoriteView.col(_("User Description")), params["User Description"].c_str(),
		favoriteView.col(_("Encoding")), params["Encoding"].c_str(),
		favoriteView.col("Group"), params["Group"].c_str(),
		favoriteView.col("Action"), 0,
		favoriteView.col(_("Extra Info")), params["ExtraInfo"].c_str(),
		favoriteView.col("Hide"), params["HideShare"].c_str(),
		favoriteView.col("Log"), params["Log"].c_str(),
		favoriteView.col("Clients"), params["CL"].c_str(),
		favoriteView.col("Filelists"), params["FL"].c_str(),
		favoriteView.col("onConnects"), params["CHAT"].c_str(),
		favoriteView.col("Mode"), Util::toInt(params["Mode"]),
		favoriteView.col("IP"), params["IP"].c_str(),
		-1);

}

void FavoriteHubs::removeEntry_gui(string address,string group)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		
		if(favoriteView.getString(&iter, "Group") == group)
		{
			GtkTreeIter child;
			gboolean gvalid = gtk_tree_model_iter_children(GTK_TREE_MODEL(favoriteStore),&child,&iter);
			while(gvalid)
			{
					if(favoriteView.getString(&child, _("Address")) == address)
					{
						gtk_tree_store_remove(favoriteStore,&child);
					}
					
				gvalid = gtk_tree_model_iter_next(m,&child);	
			}
		}
		
		if (favoriteView.getString(&iter, _("Address")) == address)
		{
			gtk_tree_store_remove(favoriteStore, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FavoriteHubs::onAddEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	const string emptyString = "";
	
	StringMap params;
	params["Name"] = emptyString;
	params["Address"] = emptyString;
	params["Description"] = emptyString;
	params["Nick"] = emptyString;
	params["Password"] = emptyString;
	params["Group"] =
	params["User Description"] = emptyString;
	params["Encoding"] = emptyString;
	params["HideShare"] = "0";
	params["Log"] = "0";
	params["FL"] = "0";
	params["CL"] = "0";
	params["CHAT"] = "0";
	params["ExtraInfo"] = emptyString;
	params["Mode"] = "0";
	params["IP"] = "0.0.0.0";
	
	bool isOk = fh->showFavoriteHubDialog_gui(params,fh);
	
	if(isOk)
	{
		typedef Func1<FavoriteHubs,StringMap> F1;
		F1 *func = new F1(fh,&FavoriteHubs::addEntry_client,params);
		WulforManager::get()->dispatchClientFunc(func);	
	}
}

void FavoriteHubs::onEditEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(fh->favoriteSel, NULL, &iter))
		return;
		
	StringMap params;
	params["Name"] = fh->favoriteView.getString(&iter, _("Name"));
	params["Address"] = fh->favoriteView.getString(&iter, _("Address"));//trsl??
	params["Description"] = fh->favoriteView.getString(&iter, _("Description"));
	params["Nick"] = fh->favoriteView.getString(&iter, _("Nick"));
	params["Password"] = fh->favoriteView.getString(&iter, "HidePassword");
	params["Group"] =fh->favoriteView.getString(&iter, "Group");
	params["User Description"] = fh->favoriteView.getString(&iter, _("User Description"));
	params["Encoding"] = fh->favoriteView.getString(&iter, _("Encoding"));
	params["HideShare"] = fh->favoriteView.getString(&iter, "Hide");
	params["Log"] = fh->favoriteView.getString(&iter, "Log");
	params["FL"] = fh->favoriteView.getString(&iter, "Filelists");
	params["CL"] = fh->favoriteView.getString(&iter, "Clients");
	params["CHAT"] = fh->favoriteView.getString(&iter, "onConnects");
	params["ExtraInfo"] = fh->favoriteView.getString(&iter, _("Extra Info"));
	params["Mode"] = Util::toString(fh->favoriteView.getValue<gint>(&iter, "Mode"));
	params["IP"] = fh->favoriteView.getString(&iter, "IP");
	
	bool isOk = fh->showFavoriteHubDialog_gui(params,fh);
	if(isOk)
	{
		string address = fh->favoriteView.getString(&iter, _("Address"));
		fh->editEntry_gui(params,&iter);
		
		typedef Func2<FavoriteHubs,string, StringMap> F2;
		F2 *func = new F2(fh,&FavoriteHubs::editEntry_client,address,params);
		WulforManager::get()->dispatchClientFunc(func);
		
		
	}
}

void FavoriteHubs::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
		FavoriteHubs *fh = (FavoriteHubs *)data;
		GtkTreeIter iter;
		if (gtk_tree_selection_get_selected(fh->favoriteSel, NULL, &iter))
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

	if (gtk_tree_selection_get_selected(fh->favoriteSel, NULL, &iter))
		WulforManager::get()->getMainWindow()->showHub_gui(
			fh->favoriteView.getString(&iter, _("Address")),
			fh->favoriteView.getString(&iter, _("Encoding")));
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
	
	if (!gtk_tree_selection_get_selected(fh->favoriteSel, NULL, &iter))
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
	GtkTreeModel *tmodel;

	if (gtk_tree_selection_get_selected(fh->favoriteSel, &tmodel, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);

		if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			GtkTreeViewColumn *column;
			gtk_tree_view_get_cursor(fh->favoriteView.get(), NULL, &column);
			if (column)
			{
				WulforManager::get()->getMainWindow()->showHub_gui(
					fh->favoriteView.getString(&iter, _("Address")),
					fh->favoriteView.getString(&iter, _("Encoding")));
			}

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

void FavoriteHubs::popupMenu_gui()
{
	if (!gtk_tree_selection_get_selected(favoriteSel, NULL, NULL))
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

void FavoriteHubs::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fh->actionStore), &iter, path))
	{
		string address = fh->actionView.getString(&iter, _("Name"));
		bool fixed = fh->actionView.getValue<gboolean>(&iter, _("Enabled"));
		fixed = !fixed;
		gtk_tree_store_set(fh->actionStore, &iter, fh->actionView.col(_("Enabled")), fixed, -1);
	}
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

void FavoriteHubs::onToggledMode_a_gui(GtkToggleButton *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	gboolean tmp = gtk_toggle_button_get_active(widget);
	fh->mode.isDef = FALSE;
	fh->mode.active = tmp;
	fh->mode.pasive = FALSE;
	gtk_widget_set_sensitive(fh->getWidget("entryip1"),TRUE);
	fh->mode.ip = string(gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryip1"))));
}

void FavoriteHubs::onToggledMode_p_gui(GtkToggleButton *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	gboolean tmp = gtk_toggle_button_get_active(widget);
	fh->mode.isDef = FALSE;
	fh->mode.active = FALSE;
	fh->mode.pasive = tmp;
	gtk_widget_set_sensitive(fh->getWidget("entryip1"), FALSE);
}

void FavoriteHubs::onToggledMode_d_gui(GtkToggleButton *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	gboolean tmp = gtk_toggle_button_get_active(widget);
	fh->mode.isDef = tmp;
	fh->mode.active = FALSE;
	fh->mode.pasive = FALSE;
	gtk_widget_set_sensitive(fh->getWidget("entryip1"), FALSE);
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
	GtkTreeModel *m = GTK_TREE_MODEL(fh->groupStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		string name = fh->groupView.getString(&iter, _("Group name"));

		if (group == name)
		{
			showErrorDialog_gui(_("Another group with the name already exists"), fh);
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}

	bool connect_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")));
	bool private_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("privateHubsCheckButton")));

	gtk_list_store_append(fh->groupStore, &iter);
	gtk_list_store_set(fh->groupStore, &iter,
		fh->groupView.col(_("Group name")), group.c_str(),
		fh->groupView.col(_("Private")), private_hub ? _("Yes") : _("No"),
		fh->groupView.col(_("Connect")), connect_hub ? _("Yes") : _("No"),
		fh->groupView.col("Private hub"), private_hub,
		fh->groupView.col("Connect hub"), connect_hub,
		-1);
}

void FavoriteHubs::onRemoveGroupClicked_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->groupselection, NULL, &iter))
	{
		string group = fh->groupView.getString(&iter, _("Group name"));

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
			gtk_list_store_remove(fh->groupStore, &iter);
//			fh->setFavoriteHubs_gui(TRUE, group);
		}
		else if (response == GTK_RESPONSE_NO)
		{
			gtk_list_store_remove(fh->groupStore, &iter);
//			fh->setFavoriteHubs_gui(FALSE, group);
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
				params["Address"] = favoriteView.getString(&iter, _("Address"));
				params["Description"] = favoriteView.getString(&iter, _("Description"));
				params["Nick"] = favoriteView.getString(&iter, _("Nick"));
				params["Password"] = favoriteView.getString(&iter, "HidePassword");
				params["User Description"] = favoriteView.getString(&iter, _("User Description"));
				params["Encoding"] = favoriteView.getString(&iter, _("Encoding"));
				params["HideShare"] = favoriteView.getString(&iter, "Hide");
				params["Log"] = favoriteView.getString(&iter, "Log");
				params["FL"] = favoriteView.getString(&iter, "Filelists");
				params["CL"] = favoriteView.getString(&iter, "Clients");
				params["CHAT"] = favoriteView.getString(&iter, "onConnects");
				params["ExtraInfo"] = favoriteView.getString(&iter, "Extra Info");
				params["Mode"] = favoriteView.getString(&iter, "Mode");
				params["IP"] = favoriteView.getString(&iter, "IP");
				params["Group"] = Util::emptyString;

				editEntry_gui(params, &iter);

				typedef Func2<FavoriteHubs, string, StringMap> F2;
				F2 *func = new F2(this, &FavoriteHubs::editEntry_client, address, params);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
		gtk_tree_store_set(favoriteStore, &iter, favoriteView.col("Action"), 0, -1);

		valid = gtk_tree_model_iter_next(m, &iter);
	}
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

void FavoriteHubs::saveFavHubGroups()
{
	GtkTreeIter iter, it;
	GtkTreeModel *m = GTK_TREE_MODEL(groupStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("groupsComboBox"))));
	gtk_list_store_clear(store);
	gtk_list_store_append(store, &it);
	gtk_list_store_set(store, &it, 0, _("Default"), -1);
	GroupsIter.clear();

	FavHubGroups favHubGroups;

	while (valid)
	{
		string group = groupView.getString(&iter, _("Group name"));

		// favorite hub properties combo box groups
		gtk_list_store_append(store, &it);
		gtk_list_store_set(store, &it, 0, group.c_str(), -1);
		GroupsIter.insert(FavHubGroupsIter::value_type(group, it));

		bool private_hub = groupView.getValue<int>(&iter, "Private hub");
		bool connect_hub = groupView.getValue<int>(&iter, "Connect hub");

		FavHubGroupProperties p;
		p.connect = connect_hub;
		p.priv = private_hub;

		favHubGroups.insert(FavHubGroup(group, p));

		valid = gtk_tree_model_iter_next(m, &iter);
	}
	FavoriteManager::getInstance()->setFavHubGroups(favHubGroups);
	FavoriteManager::getInstance()->save();
}

void FavoriteHubs::onUpdateGroupClicked_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	GtkTreeIter iter;
	string group = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")));

	if (gtk_tree_selection_get_selected(fh->groupselection, NULL, &iter) && !group.empty())
	{
		bool connect_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")));
		bool private_hub = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("privateHubsCheckButton")));

		gtk_list_store_set(fh->groupStore, &iter,
			fh->groupView.col(_("Group name")), group.c_str(),
			fh->groupView.col(_("Private")), private_hub ? _("Yes") : _("No"),
			fh->groupView.col(_("Connect")), connect_hub ? _("Yes") : _("No"),
			fh->groupView.col("Private hub"), private_hub,
			fh->groupView.col("Connect hub"), connect_hub,
			-1);
	}
}

gboolean FavoriteHubs::onGroupsKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->groupselection, NULL, &iter))
	{
		if (event->keyval == GDK_Up || event->keyval == GDK_Down)
		{
			string group = fh->groupView.getString(&iter, _("Group name"));
			gboolean priv = fh->groupView.getValue<gboolean>(&iter, "Private hub");
			gboolean con = fh->groupView.getValue<gboolean>(&iter, "Connect hub");

			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("privateHubsCheckButton")), priv);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")), con);
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")), group.c_str());
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
		if (gtk_tree_selection_get_selected(fh->groupselection, NULL, &iter))
		{
			string group = fh->groupView.getString(&iter, _("Group name"));
			gboolean priv = fh->groupView.getValue<gboolean>(&iter, "Private hub");
			gboolean con = fh->groupView.getValue<gboolean>(&iter, "Connect hub");

			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("privateHubsCheckButton")), priv);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("connectAllHubsCheckButton")), con);
			gtk_entry_set_text(GTK_ENTRY(fh->getWidget("nameGroupEntry")), group.c_str());
		}
	}
	return FALSE;
}


bool FavoriteHubs::showFavoriteHubDialog_gui(StringMap &params, FavoriteHubs *fh)
{
	fh->initActions();
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryName")), params["Name"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAddress")), params["Address"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescription")), params["Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNick")), params["Nick"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryPassword")), params["Password"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryUserDescription")), params["User Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("comboboxCharset")), params["Encoding"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryExtraInfo")), params["ExtraInfo"].c_str());
	
	//Modes
	if(params["Mode"] == "0" || params["Mode"] == "-1")
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("radiobuttonmode1")),TRUE);    
		fh->mode.isDef = TRUE;
		fh->mode.active = FALSE;
		fh->mode.pasive = FALSE;
		fh->mode.ip = "0.0.0.0";
		gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryip1")), params["IP"].c_str());
	}
	else if (params["Mode"] == "1")
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("radiobuttonmode2")),TRUE);    
		fh->mode.isDef = FALSE;
		fh->mode.active = TRUE;
		fh->mode.pasive = FALSE;
		fh->mode.ip = params["IP"];
		gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryip1")), params["IP"].c_str());
	}
	else if (params["Mode"] == "2")
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("radiobuttonmode3")),TRUE);    
		fh->mode.isDef = TRUE;
		fh->mode.active = FALSE;
		fh->mode.pasive = FALSE;
		fh->mode.ip = "0.0.0.0";
		gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryip1")), params["IP"].c_str());
	}
	///Groups
	FavHubGroupsIter::const_iterator it = fh->GroupsIter.find(params["Group"]);
	if (it != fh->GroupsIter.end())
	{
		GtkTreeIter iter = it->second;
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(fh->getWidget("groupsComboBox")), &iter);
	}
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(fh->getWidget("groupsComboBox")), 0);
	///END
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
	
	// Set the HideShare checkbox
	gboolean hides =(((params["HideShare"] == "1") ? TRUE : FALSE) || params["HideShare"].empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonHideShare")), hides);

	// Set the log checkbox
	gboolean logs = (((params["Log"] == "1") ? TRUE : FALSE) || params["Log"].empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonLogChat")), logs);

	// Set the client check checkbox
	gboolean cl = (((params["CL"] == "1") ? TRUE : FALSE) || params["CL"].empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkcli")), cl);

	// Set the filelist check checkbox
	gboolean fl = (((params["FL"] == "1") ? TRUE : FALSE) || params["FL"].empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkFL")), fl);
	// Set the at conections checkbox
	gboolean atcon = (((params["CHAT"] == "1") ? TRUE : FALSE) || params["CHAT"].empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkatconn")), atcon);
	
	fh->setRawActions_gui(fh,params);
	
	// Show the dialog
	gint response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));
	
	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;
	
	while(response == GTK_RESPONSE_OK)
	{
			params["Name"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryName")));
			params["Address"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryAddress")));
			params["Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescription")));
			params["Password"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryPassword")));
			params["HideShare"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonHideShare"))) ? "1" : "0";
			params["Log"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonLogChat"))) ? "1" : "0";
			params["CL"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkcli"))) ? "1" : "0";
			params["FL"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkFL"))) ? "1" : "0";
			params["CHAT"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkatconn"))) ? "1" : "0";
			params["ExtraInfo"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryExtraInfo")));
			params["IP"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryip1")));
			
			if(fh->mode.isDef)
			{
				params["Mode"] = "0";	
				
			}
			else if(fh->mode.active)
			{
				params["Mode"] = "1";	
				
			}else if(fh->mode.pasive)
			{
				params["Mode"] = "2";	
			}
		
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonNick"))))
			{
				params["Nick"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNick")));
			}

			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonUserDescription"))))
			{
				params["User Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryUserDescription")));
			}
			/* Encoding */
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonEncoding"))))
			{
				gchar *enco = gtk_combo_box_get_active_text(GTK_COMBO_BOX(fh->getWidget("comboboxCharset")));
				params["Encoding"] = string(enco);
				g_free(enco);
			}
			
			/* groups */
			gchar *group = gtk_combo_box_get_active_text(GTK_COMBO_BOX(fh->getWidget("groupsComboBox")));
			params["Group"] = string(group);
			g_free(group);
			
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

void FavoriteHubs::setRawActions_gui(FavoriteHubs *fh,StringMap params)
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

bool FavoriteHubs::findHub_gui(const string &server, GtkTreeIter *par, GtkTreeIter *child)
{
	FavIter::const_iterator it = faviters.find(server);

	if (it != faviters.end())
	{
		
		if(par)
			*par = it->second->main;
		
		if (child)
			*child = it->second->child;

		return TRUE;
	}

	return FALSE;
}

///Clients Func

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
		entry.setHideShare(Util::toInt(params["HideShare"]));
		entry.setLogChat(Util::toInt(params["Log"]));
		//CMD
		entry.setCheckClients(Util::toInt(params["CL"]));
		entry.setCheckFilelists(Util::toInt(params["FL"]));
		entry.setCheckOnConnect(Util::toInt(params["CHAT"]));
		///ExtraInfo
		entry.setChatExtraInfo(params["ExtraInfo"]);
		///Mode
		entry.setMode(Util::toInt(params["Mode"]));
		entry.setIp(params["IP"]);
		///Groups
		entry.setGroup(params["Group"]);
		FavoriteManager::getInstance()->addFavorite(entry);
}

void FavoriteHubs::editEntry_client(string address,StringMap params)
{
		FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);
		if (entry)
		{

			entry->setName(params["Name"]);
			entry->setServer(params["Address"]);
			entry->setDescription(params["Description"]);
			entry->setEncoding(params["Encoding"]);
			entry->setNick(params["Nick"]);
			entry->setPassword(params["Password"]);
			entry->setUserDescription(params["User Description"]);
			entry->setHideShare(Util::toInt(params["HideShare"]));
			entry->setLogChat(Util::toInt(params["Log"]));
			entry->setGroup(params["Group"]);
			//CMD
			entry->setCheckClients(Util::toInt(params["CL"]));
			entry->setCheckFilelists(Util::toInt(params["FL"]));
			entry->setCheckOnConnect(Util::toInt(params["CHAT"]));
			entry->setChatExtraInfo(params["ExtraInfo"]);
			entry->setMode(Util::toInt(params["Mode"]));
			entry->setIp(params["IP"]);

			FavoriteManager::getInstance()->save();
		}
}

void FavoriteHubs::removeEntry_client(string address)
{

	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
		FavoriteManager::getInstance()->removeFavorite(entry);

}

void FavoriteHubs::getParamsFav(const FavoriteHubEntry *entry,StringMap &params)
{
	params["Name"] = entry->getName();
	params["Address"] = entry->getServer();
	params["Nick"] = entry->getNick();
	params["Pass"] = entry->getPassword();
	params["Desc"] = entry->getDescription();
	params["HideShare"] = entry->getHideShare() ? "1" : "0";
	params["Log"] = entry->getLogChat() ? "1" : "0";
	params["User Description"] = entry->getUserDescription();
	params["Encoding"]= entry->getEncoding();
	params["Group"] = entry->getGroup();
	params["FL"] = entry->getCheckFilelists() ? "1" : "0";
	params["CL"] = entry->getCheckClients() ? "1" : "0";
	params["CHAT"] = entry->getCheckOnConnect() ? "1" : "0";
	params["ExtraInfo"] = entry->getChatExtraInfo();
	params["Mode"] = Util::toString(entry->getMode());
	params["IP"] = entry->getIp();

}

void FavoriteHubs::on(dcpp::FavoriteManagerListener::FavoriteAdded, const dcpp::FavoriteHubEntryPtr entry) throw() 
{ 
	StringMap params;
	getParamsFav(entry,params);
	
	typedef Func1<FavoriteHubs,StringMap> F1;
	F1 *func = new F1(this,&FavoriteHubs::addEntry_gui,params);
	WulforManager::get()->dispatchGuiFunc(func);	
}
void FavoriteHubs::on(dcpp::FavoriteManagerListener::FavoriteRemoved, const dcpp::FavoriteHubEntryPtr entry) throw()
{
	string address = (*entry).getServer();
	string group = (*entry).getGroup();
	
	typedef Func2<FavoriteHubs,string, string> F2;
	F2 *func = new F2(this,&FavoriteHubs::removeEntry_gui, address, group);
	WulforManager::get()->dispatchGuiFunc(func);
	
}

/*this is a pop menu*/
void FavoriteHubs::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()),closeMenuItem);
    gtk_widget_show(closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate", G_CALLBACK(onCloseItem),this);

}

void FavoriteHubs::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}
