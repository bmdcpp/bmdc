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

#include "search.hh"

#include "../dcpp/FavoriteManager.h"
#include "../dcpp/QueueManager.h"
#include "../dcpp/ShareManager.h"
#include "../dcpp/StringTokenizer.h"
#include "../dcpp/Text.h"
#include "../dcpp/UserCommand.h"
#include "../dcpp/Client.h"
#include "../dcpp/AVManager.h"
#include "../dcpp/GeoManager.h"
#include "../dcpp/format.h"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "settingsmanager.hh"
#include "gtk-fixies.hh"

using namespace std;
using namespace dcpp;
//no need as static var
#define EN_STRING 0

Search::Search(const string& str):
	BookEntry(Entry::SEARCH, _("Search: "), "search", str.empty() ? generateID() : str),
	previousGrouping(NOGROUPING)
{
	setSearchButtons(true);

	/* set up completion */
	//completion = gtk_entry_completion_new();
	//gtk_entry_completion_set_text_column(completion, EN_STRING);
	//gtk_entry_set_completion(GTK_ENTRY(getWidget("SearchEntry")), completion);
	//g_signal_connect(G_OBJECT (completion), "match-selected", G_CALLBACK (on_match_select_entry), (gpointer)this);

	/* Create the ListStore set it as the model of the entrycompletion */
	/*emodel = gtk_list_store_new(1, G_TYPE_STRING);
	StringTokenizer<string> st(WGETS("last-searchs"),';');
	GtkTreeIter eiter;

	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i)
	{
		gtk_list_store_append(emodel, &eiter);
		gtk_list_store_set(emodel, &eiter, EN_STRING, i->c_str(), -1);
	}

	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(emodel));
*/
	// Configure the dialog
	File::ensureDirectory(SETTING(DOWNLOAD_DIRECTORY));

	// Initialize check button options.
	onlyFree = SETTING(SEARCH_ONLY_FREE_SLOTS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonSlots")), onlyFree);
	gtk_widget_set_sensitive(GTK_WIDGET(getWidget("checkbuttonSlots")), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(getWidget("checkbuttonShared")), FALSE);

	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxSize")), 1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxUnit")), 2);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxFile")), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxGroupBy")), (int)NOGROUPING);

	// Initialize hub list treeview
	hubView.setView(GTK_TREE_VIEW(getWidget("treeviewHubs")));
	hubView.insertColumn("Search", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	hubView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
	hubView.insertHiddenColumn("Url", G_TYPE_STRING);
	hubView.finalize();
	hubStore = gtk_list_store_newv(hubView.getColCount(), hubView.getGTypes());
	gtk_tree_view_set_model(hubView.get(), GTK_TREE_MODEL(hubStore));
	g_object_unref(hubStore);

	// Initialize search result treeview
	resultView.setView(GTK_TREE_VIEW(getWidget("treeviewResult")), TRUE, "search");
	resultView.insertColumn(_("Filename"), G_TYPE_STRING, TreeView::PIXBUF_STRING, 250, "Icon");
	resultView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::STRING, 100);
	resultView.insertColumn(_("Type"), G_TYPE_STRING, TreeView::STRING, 65);
	resultView.insertColumn(_("Size"), G_TYPE_STRING, TreeView::STRING, 80);
	resultView.insertColumn(_("Path"), G_TYPE_STRING, TreeView::STRING, 100);
	resultView.insertColumn(_("Slots"), G_TYPE_STRING, TreeView::STRING, 50);
	resultView.insertColumn(_("Connection"), G_TYPE_STRING, TreeView::STRING, 90);
	resultView.insertColumn(_("Hub"), G_TYPE_STRING, TreeView::STRING, 150);
	resultView.insertColumn(_("Exact Size"), G_TYPE_STRING, TreeView::STRING, 80);
	resultView.insertColumn("Country", G_TYPE_STRING, TreeView::PIXBUF_STRING, 100, "Pixbuf");
	resultView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 100);
	resultView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 125);
	resultView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	resultView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	resultView.insertHiddenColumn("Slots Order", G_TYPE_INT);
	resultView.insertHiddenColumn("File Order", G_TYPE_STRING);
	resultView.insertHiddenColumn("Hub URL", G_TYPE_STRING);
	resultView.insertHiddenColumn("CID", G_TYPE_STRING);
	resultView.insertHiddenColumn("Shared", G_TYPE_BOOLEAN);
	resultView.insertHiddenColumn("Grouping String", G_TYPE_STRING);
	resultView.insertHiddenColumn("Free Slots", G_TYPE_INT);
	resultView.insertHiddenColumn("Pixbuf", GDK_TYPE_PIXBUF);
	resultView.finalize();
	resultStore = gtk_tree_store_newv(resultView.getColCount(), resultView.getGTypes());
	searchFilterModel = gtk_tree_model_filter_new(GTK_TREE_MODEL(resultStore), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(searchFilterModel), &Search::searchFilterFunc_gui, (gpointer)this, NULL);
	sortedFilterModel = gtk_tree_model_sort_new_with_model(searchFilterModel);
	gtk_tree_view_set_model(resultView.get(), sortedFilterModel);
	g_object_unref(resultStore);
	g_object_unref(searchFilterModel);
	g_object_unref(sortedFilterModel);
	selection = gtk_tree_view_get_selection(resultView.get());
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	resultView.setSortColumn_gui(_("Size"), "Real Size");
	resultView.setSortColumn_gui(_("Exact Size"), "Real Size");
	resultView.setSortColumn_gui(_("Slots"), "Slots Order");
	resultView.setSortColumn_gui(_("Filename"), "File Order");
	//
	resultView.setSelection(selection);
//	resultView.buildCopyMenu(getWidget("CopyMenu"));
	//..
	g_object_set(G_OBJECT(resultView.get()),"has-tooltip", TRUE, NULL);
	g_signal_connect(resultView.get(), "query-tooltip", G_CALLBACK(onResultView_gui), (gpointer)this);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (resultView.get())), "changed", G_CALLBACK (selection_changed_result_gui), GTK_WIDGET(resultView.get()));
	/* Set a tooltip on the column */
	set_Header_tooltip_gui();

	// Initialize the user command menu
	//userCommandMenu = new UserCommandMenu(getWidget("usercommandMenu"), ::UserCommand::CONTEXT_SEARCH);

	// Initialize search types
	GtkTreeIter iter;
	GtkComboBox *combo_box = GTK_COMBO_BOX(getWidget("comboboxFile"));
	GtkTreeModel *model = gtk_combo_box_get_model(combo_box);
	GtkListStore *store = GTK_LIST_STORE(model);

	// Predefined
	for(int i = SearchManager::TYPE_ANY; i != SearchManager::TYPE_LAST; ++i)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, SearchManager::getTypeStr(i), -1);
	}

	// Customs
	for(auto& i: SettingsManager::getInstance()->getSearchTypes()) {
		if( !(i.first.size() == 1 || i.first[0] <= '1' || i.first[0] >= '6')) { //Custom type
			string type = i.first;
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, 0, type.c_str(), -1);
		}
	}
	gtk_combo_box_set_active(combo_box, 0);

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("checkbuttonFilter"), "toggled", G_CALLBACK(onFilterButtonToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonSlots"), "toggled", G_CALLBACK(onSlotsButtonToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonShared"), "toggled", G_CALLBACK(onSharedButtonToggled_gui), (gpointer)this);
	g_signal_connect(hubView.getCellRenderOf("Search"), "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
//	g_signal_connect(resultView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
//	g_signal_connect(resultView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	//g_signal_connect(getWidget("SearchEntry"), "key-press-event", G_CALLBACK(onSearchEntryKeyPressed_gui), (gpointer)this);
	//g_signal_connect(getWidget("SearchEntry"), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	//g_signal_connect(getWidget("entrySize"), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonSearch"), "clicked", G_CALLBACK(onSearchButtonClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("downloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("downloadWholeDirItem"), "activate", G_CALLBACK(onDownloadDirClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("searchByTTHItem"), "activate", G_CALLBACK(onSearchByTTHClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyMagnetClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("getFileListItem"), "activate", G_CALLBACK(onGetFileListClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("sendPrivateMessageItem"), "activate", G_CALLBACK(onPrivateMessageClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("addToFavoritesItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("grantExtraSlotItem"), "activate", G_CALLBACK(onGrantExtraSlotClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("removeUserFromQueueItem"), "activate", G_CALLBACK(onRemoveUserFromQueueClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxSize"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxUnit"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxFile"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxGroupBy"), "changed", G_CALLBACK(onGroupByComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("checkop"), "toggled", G_CALLBACK(onCheckOp_gui), (gpointer)this);
}

Search::~Search()
{
	ClientManager::getInstance()->removeListener(this);
	SearchManager::getInstance()->removeListener(this);
}

void Search::show()
{
	initHubs_gui();
	ClientManager::getInstance()->addListener(this);
	SearchManager::getInstance()->addListener(this);
}

void Search::setColorsRows()
{
	setColorRow(_("Filename"));
	setColorRow(_("Nick"));
	setColorRow(_("Type"));
	setColorRow(_("Size"));
	setColorRow(_("Path"));
	setColorRow(_("Slots"));
	setColorRow(_("Connection"));
	setColorRow(_("Hub"));
	setColorRow(_("Exact Size"));
	setColorRow("Country");
	setColorRow("IP");
	setColorRow("TTH");
}

void Search::setColorRow(string cell)
{

	if(resultView.getCellRenderOf(cell) != NULL)
		gtk_tree_view_column_set_cell_data_func(resultView.getColumn(cell),
								resultView.getCellRenderOf(cell),
								Search::makeColor,
								(gpointer)this,
								NULL);
	
	if(resultView.getCellRenderOf2(cell) != NULL)
		gtk_tree_view_column_set_cell_data_func(resultView.getColumn(cell),
								resultView.getCellRenderOf2(cell),
								Search::makeColor,
								(gpointer)this,
								NULL);
}

void Search::makeColor(GtkTreeViewColumn *column,GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter,gpointer data)
{
		Search* se = (Search *)data;
		if(se == NULL) return;
		if(model == NULL)
			return;
		if(iter == NULL)
			return;
		if(column == NULL)
			return;
		if(cell == NULL)
			return;
	string color = string();
	string nick = se->resultView.getString(iter,_("Nick"),model);
	string ip = se->resultView.getString(iter,"IP",model);

	if(!nick.empty() &&  AVManager::getInstance()->isNickVirused(nick))
	{
		if( AVManager::getInstance()->isIpVirused(ip) == true)
			color = "red";//hardcode for now
	}
	if(!ip.empty() && AVManager::getInstance()->isIpVirused(ip))
	{
		AVManager::AVEntry entry = AVManager::getInstance()->getEntryByIP(ip);
		if(entry.sNick == nick)
			color = "red";
	}
	if(!color.empty())
		g_object_set(cell,"cell-background-set",TRUE,"cell-background",color.c_str(),NULL);
}


void Search::putValue_gui(const string &str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type)
{
	gtk_editable_set_text(GTK_EDITABLE(getWidget("SearchEntry")), str.c_str());
	gtk_editable_set_text(GTK_EDITABLE(getWidget("entrySize")), Util::toString(size).c_str());
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxSize")), (int)mode);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxFile")), (int)type);

	search_gui();
}

void Search::initHubs_gui()
{
	ClientManager::getInstance()->lock();

	const ClientManager::ClientList clients = ClientManager::getInstance()->getClients();

	Client *client = nullptr;
	for (auto it = clients.begin(); it != clients.end(); ++it)
	{
		client = (*it).second;
		if (client->isConnected())
			addHub_gui(client->getHubName(), client->getHubUrl());
	}
}

void Search::addHub_gui(string name, string url)
{
	GtkTreeIter iter;
	gtk_list_store_append(hubStore, &iter);
	gtk_list_store_set(hubStore, &iter,
		hubView.col("Search"), TRUE,
		hubView.col("Name"), name.empty() ? url.c_str() : name.c_str(),
		hubView.col("Url"), url.c_str(),
		-1);
}

void Search::modifyHub_gui(string name, string url, bool op /*= true*/)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(hubStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (url == hubView.getString(&iter, "Url"))
		{
			gtk_list_store_set(hubStore, &iter,
				hubView.col("Search"), op,
				hubView.col("Name"), name.empty() ? url.c_str() : name.c_str(),
				hubView.col("Url"), url.c_str(),
				-1);
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Search::removeHub_gui(string url)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(hubStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (url == hubView.getString(&iter, "Url"))
		{
			gtk_list_store_remove(hubStore, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Search::popupMenu_gui()
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(selection, NULL);
	guint count = g_list_length(list);

	if (count == 1)
	{
		path = (GtkTreePath*)list->data; // This will be freed later

		// If it is a parent effectively more than one row is selected
		if (gtk_tree_model_get_iter(sortedFilterModel, &iter, path) &&
		    gtk_tree_model_iter_has_child(sortedFilterModel, &iter))
		{
			gtk_widget_set_sensitive(getWidget("searchByTTHItem"), FALSE);
		}
		else
		{
			gtk_widget_set_sensitive(getWidget("searchByTTHItem"), TRUE);
		}
	}
	else if (count > 1)
	{
		gtk_widget_set_sensitive(getWidget("searchByTTHItem"), FALSE);
	}

	GtkWidget *menuItem;
	string tth;

	// Clean menus
//	gtk_container_foreach(GTK_CONTAINER(getWidget("downloadMenu")), (GtkCallback)gtk_widget_destroy, NULL);
//	gtk_container_foreach(GTK_CONTAINER(getWidget("downloadDirMenu")), (GtkCallback)gtk_widget_destroy, NULL);
	userCommandMenu->cleanMenu_gui();

	// Build "Download to..." submenu

	// Add favorite download directories
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); i++)
		{
//			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
//			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
//			g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteClicked_gui), (gpointer)this);
//			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
		}
//		menuItem = gtk_separator_menu_item_new();
//		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
	}

	// Add Browse item
//	menuItem = gtk_menu_item_new_with_label(_("Browse..."));
//	g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
//	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);

	// Add search results with the same TTH to menu
	bool firstTTH = true,	hasTTH = false;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(sortedFilterModel, &iter, path))
		{
			userCommandMenu->addHub(resultView.getString(&iter, "Hub URL"));
			userCommandMenu->addFile(resultView.getString(&iter, "CID"),
				resultView.getString(&iter, _("Filename")),
				resultView.getValue<int64_t>(&iter, "Real Size"),
				resultView.getString(&iter, "TTH"));

			if (firstTTH)
			{
				tth = resultView.getString(&iter, "TTH");
				firstTTH = false;
				hasTTH = true;
			}
			else if (hasTTH)
			{
				if (tth.empty() || tth != resultView.getString(&iter, "TTH"))
					hasTTH = false; // Can't break here since we have to free all the paths
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	if (hasTTH)
	{
		StringList targets;
		targets = QueueManager::getInstance()->getTargets(TTHValue(tth));

		if (targets.size() > static_cast<size_t>(0))
		{
///			menuItem = gtk_separator_menu_item_new();
	//		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
			for (StringIter i = targets.begin(); i != targets.end(); ++i)
			{
	//			menuItem = gtk_menu_item_new_with_label(i->c_str());
	//			g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadToMatchClicked_gui), (gpointer)this);
	//			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
			}
		}
	}

	// Build "Download whole directory to..." submenu

	spl.clear();
	spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); i++)
		{
	//		menuItem = gtk_menu_item_new_with_label(i->second.c_str());
	//		g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
	//		g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteDirClicked_gui), (gpointer)this);
	//		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadDirMenu")), menuItem);
		}
	//	menuItem = gtk_separator_menu_item_new();
	//	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadDirMenu")), menuItem);
	
	}
	//	menuItem = gtk_menu_item_new_with_label(_("Browse..."));
	//	g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadDirToClicked_gui), (gpointer)this);
	//	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadDirMenu")), menuItem);
		
	
	// Build user command menu
//	userCommandMenu->buildMenu_gui();
//	gtk_menu_popup(GTK_MENU(getWidget("mainMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());

}

void Search::setStatus_gui(string statusBar, string text)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
}

void Search::search_gui()
{
	StringList clients;
	GtkTreeIter iter;

	string text = gtk_editable_get_text(GTK_EDITABLE(getWidget("SearchEntry")));
	if (text.empty())
		return;

	WSET("last-searchs", WGETS("last-searchs") + ";" + text );
	GtkTreeIter eiter;
	gtk_list_store_append(emodel, &eiter);
	gtk_list_store_set(emodel, &eiter,
							EN_STRING, text.c_str(),
							-1);

	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(hubStore), &iter);
 	while (valid)
 	{
 		if (hubView.getValue<gboolean>(&iter, "Search"))
 			clients.push_back(hubView.getString(&iter, "Url"));
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(hubStore), &iter);
	}

	if (clients.size() < 1)
		return;

	double lsize = Util::toDouble(gtk_editable_get_text(GTK_EDITABLE(getWidget("entrySize"))));

	switch (gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxUnit"))))
	{
		case 1:
			lsize *= 1024.0;
			break;
		case 2:
			lsize *= 1024.0 * 1024.0;
			break;
		case 3:
			lsize *= 1024.0 * 1024.0 * 1024.0;
			break;
	}

	gtk_tree_store_clear(resultStore);
	results.clear();
	int64_t llsize = static_cast<int64_t>(lsize);
	searchlist = StringTokenizer<string>(text, ' ').getTokens();

	// Strip out terms beginning with -
	text.clear();
	for (StringList::const_iterator si = searchlist.begin(); si != searchlist.end(); ++si)
		if ((*si)[0] != '-')
			text += *si + ' ';
	text = text.substr(0, std::max(text.size(), static_cast<string::size_type>(1)) - 1);

	SearchManager::SizeModes mode((SearchManager::SizeModes)gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxSize"))));
	if (llsize == 0)
		mode = SearchManager::SIZE_DONTCARE;

	int ftype = gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxFile")));

	string ftypeStr;
	if (ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_LAST)
	{
		ftypeStr = SearchManager::getInstance()->getTypeStr(ftype);
	}
	else
	{
		gchar *tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxFile")));
		ftypeStr = tmp;
		g_free(tmp);
		ftype = SearchManager::TYPE_ANY;
	}

	// Get ADC searchtype extensions if any is selected
	StringList exts;
	try
	{
		if (ftype == SearchManager::TYPE_ANY)
		{
			// Custom searchtype
			exts = SettingsManager::getInstance()->getExtensions(ftypeStr);
		}
		else if (ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_DIRECTORY)
		{
			// Predefined searchtype
			exts = SettingsManager::getInstance()->getExtensions(string(1, '0' + ftype));
		}
	}
	catch (const SearchTypeException&)
	{
		ftype = SearchManager::TYPE_ANY;
	}

	isHash = (ftype == SearchManager::TYPE_TTH);

	droppedResult = 0;
	searchHits = 0;
	setStatus_gui("statusbar1", _("Searching for ") + text + " ...");
	setStatus_gui("statusbar2", _("0 items"));
	setStatus_gui("statusbar3", _("0 filtered"));
	setLabel_gui(_("Search: ") + text);

	if (SearchManager::getInstance()->okToSearch())
	{
		dcdebug("Sent ADC extensions : %s\n",Util::toString(";", exts).c_str());
		SearchManager::getInstance()->search(clients, text, llsize, (SearchManager::TypeModes)ftype, mode, "manual", exts);//NOTE: core 0.770

		if (SETTING(CLEAR_SEARCH)) // Only clear if the search was sent.
			gtk_editable_set_text(GTK_EDITABLE(getWidget("SearchEntry")), "");
	}
	else
	{
		int32_t waitFor = SearchManager::getInstance()->timeToSearch();
		string line = _("Searching too soon, retry in ") + Util::formatSeconds(waitFor) + " s";
		setStatus_gui("statusbar1", line);
		setStatus_gui("statusbar2", "");
		setStatus_gui("statusbar3", "");
	}
}

void Search::addResult_gui(const SearchResultPtr result)
{
	// Check that it's not a duplicate and find parent for grouping
	GtkTreeIter iter;
	GtkTreeIter parent;
	GtkTreeIter child;
	GtkTreeModel *m = GTK_TREE_MODEL(resultStore);
	bool foundParent = false;
	bool createParent = false;

	vector<SearchResultPtr> &existingResults = results[result->getUser()->getCID().toBase32()];
	for (vector<SearchResultPtr>::iterator it = existingResults.begin(); it != existingResults.end(); ++it)
	{
		// Check if it's a duplicate
		if (result->getFile() == (*it)->getFile())
			return;
	}
	existingResults.push_back(result);

	dcpp::StringMap resultMap;
	parseSearchResult_gui(result, resultMap);

	// Find grouping parent
	GroupType groupBy = (GroupType)gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxGroupBy")));
	string groupColumn = getGroupingColumn(groupBy);
	string groupStr = resultMap[groupColumn];
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid && groupBy != NOGROUPING && !foundParent && !groupStr.empty())
	{
		if (resultView.getString(&iter, "Grouping String", m) == groupStr)
		{
			// Parent row
			if (gtk_tree_model_iter_has_child(m, &iter))
			{
				parent = iter;
				foundParent = TRUE;
			}
			// If two rows that match the grouping criteria
			// are found, group them under a new parent row
			else if (!foundParent)
			{
				child = iter;
				createParent = TRUE;
				foundParent = TRUE;
			}
		}

		valid = WulforUtil::getNextIter_gui(m, &iter);
	}

	// Move top level row to be under newly created grouping parent.
	// This needs to be done outside of the loop so that we don't modify the
	// tree until after the duplication check.
	if (createParent)
	{
//		GdkPixbuf* buf = gtk_icon_theme_load_icon(WulforUtil::icon_theme,"dnd-multiple",GTK_ICON_SIZE_MENU,GTK_ICON_LOOKUP_USE_BUILTIN,NULL);
		// Insert the new parent row
		gtk_tree_store_insert_with_values(resultStore, &parent, NULL, -1,
//				resultView.col("Icon"),buf ,
				resultView.col("Grouping String"), groupStr.c_str(),
				-1);

		// Move the row to be a child of the new parent
		WulforUtil::copyRow_gui(resultStore, &child, &parent);
		gtk_tree_store_remove(resultStore, &child);
	}

	// Have to use insert with values since appending would cause searchFilterFunc to be
	// called with empty row which in turn will cause assert failure in treeview::getString
	gtk_tree_store_insert_with_values(resultStore, &iter, foundParent ? &parent : NULL, -1,
		resultView.col(_("Nick")), resultMap[_("Nick")].c_str(),
		resultView.col(_("Filename")), resultMap[_("Filename")].c_str(),
		resultView.col(_("Slots")), resultMap["Slots"].c_str(),
		resultView.col(_("Size")), resultMap[_("Size")].c_str(),
		resultView.col(_("Path")), resultMap[_("Path")].c_str(),
		resultView.col(_("Type")), resultMap[_("Type")].c_str(),
		resultView.col(_("Connection")), resultMap[_("Connection")].c_str(),
		resultView.col(_("Hub")), resultMap[_("Hub")].c_str(),
		resultView.col(_("Exact Size")), resultMap["Exact Size"].c_str(),
		resultView.col("Country"), GeoManager::getInstance()->getCountry(resultMap["IP"]).c_str(),
		resultView.col("IP"), resultMap["IP"].c_str(),
		resultView.col("TTH"), resultMap["TTH"].c_str(),
		resultView.col("Icon"), WulforUtil::loadIconShare(resultMap["Icon"]),
		resultView.col("File Order"), resultMap["File Order"].c_str(),
		resultView.col("Real Size"), Util::toInt64(resultMap["Real Size"]),
		resultView.col("Slots Order"), Util::toInt(resultMap["Slots Order"]),
		resultView.col("Hub URL"), resultMap["Hub URL"].c_str(),
		resultView.col("CID"), resultMap["CID"].c_str(),
		resultView.col("Shared"), Util::toInt(resultMap["Shared"]),
		resultView.col("Grouping String"), groupStr.c_str(),
		resultView.col("Free Slots"), Util::toInt(resultMap["Free Slots"]),
		resultView.col("Pixbuf"), WulforUtil::LoadCountryPixbuf(GeoManager::getInstance()->getCountryAbbrevation(resultMap["IP"])),
		-1);

	if (foundParent)
		updateParentRow_gui(&parent, &iter);

	++searchHits;
	setStatus_gui("statusbar2", Util::toString(searchHits) + _(" items"));

	if (SETTING(BOLD_SEARCH))
		setBold_gui();

	setColorsRows();
}

void Search::updateParentRow_gui(GtkTreeIter *parent, GtkTreeIter *child)
{
	// Let's make sure we really have children...
	gint children = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(resultStore), parent);
	dcassert(children != 0);

	string groupStr = resultView.getString(parent, "Grouping String", GTK_TREE_MODEL(resultStore));
	string groupDesc = "[" + Util::toString(children) + "] " + groupStr;
	gtk_tree_store_set(resultStore, parent, resultView.col(_("Filename")), groupDesc.c_str(), -1);

	if (child == NULL)
		return;

	GroupType groupType = (GroupType)gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxGroupBy")));

	switch (groupType)
	{
		case NOGROUPING:
			break;
		case FILENAME:
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col("File Order"));
			break;
		case FILEPATH:
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Path")));
			break;
		case SIZE:
		{
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Exact Size")));
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Size")));
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col("Real Size"));
			break;
		}
		case CONNECTION:
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Connection")));
			break;
		case TTH:
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col("TTH"));
			break;
		case NICK:
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Nick")));
			break;
		case HUB:
		{
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Hub")));
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col("Hub URL"));
			break;
		}
		case TYPE:
			WulforUtil::copyValue_gui(resultStore, child, parent, resultView.col(_("Type")));
			break;
		default:
			///@todo: throw an exception
			break;
	}
}

void Search::ungroup_gui()
{
	GtkTreeIter iter;
	gint position = 0;
	GtkTreeModel *m = GTK_TREE_MODEL(resultStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		// Ungroup parent rows and remove them
		if (gtk_tree_model_iter_has_child(m, &iter))
		{
			GtkTreeIter child = iter;
			valid = WulforUtil::getNextIter_gui(m, &child, TRUE, FALSE);

			// Move all children out from under the old grouping parent
			while (valid)
			{
				WulforUtil::copyRow_gui(resultStore, &child, NULL, position++);
				valid = gtk_tree_store_remove(resultStore, &child);
			}

			// Delete the parent row
			valid = gtk_tree_store_remove(resultStore, &iter);
		}
		else // Non-parent row
		{
			++position;
			valid = WulforUtil::getNextIter_gui(m, &iter);
		}
	}
}

void Search::regroup_gui()
{
	unordered_map<string, GtkTreeIter> iterMap; // Maps group string -> parent tree iter
	GtkTreeIter iter;
	gint position = 0;
	GtkTreeModel *m = GTK_TREE_MODEL(resultStore);
	GroupType groupBy = (GroupType)gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxGroupBy")));
	string groupColumn = getGroupingColumn(groupBy);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		string groupStr;
		if (!groupColumn.empty())
			groupStr = resultView.getString(&iter, groupColumn, m);

		// Don't add parent rows
		if (gtk_tree_model_iter_has_child(m, &iter) || groupStr.empty())
		{
			++position;
			valid = WulforUtil::getNextIter_gui(m, &iter);
			continue;
		}

		unordered_map<string, GtkTreeIter>::iterator mapIter = iterMap.find(groupStr);

		// New non-parent, top-level item
		if (mapIter == iterMap.end())
		{
			++position;
			iterMap[groupStr] = iter;
			valid = WulforUtil::getNextIter_gui(m, &iter);
		}
		else // Insert as a child under the grouping parent
		{
			GtkTreeIter parent = mapIter->second;
			GtkTreeIter groupParent = mapIter->second;

			// If this is the first child to be appended, create a new parent row.
			if (!gtk_tree_model_iter_has_child(GTK_TREE_MODEL(resultStore), &groupParent))
			{
//			GdkPixbuf* buf = gtk_icon_theme_load_icon(WulforUtil::icon_theme,"dnd-multiple",GTK_ICON_SIZE_MENU,GTK_ICON_LOOKUP_USE_BUILTIN,NULL);

			gtk_tree_store_insert_with_values(resultStore, &parent, NULL, position,
//					resultView.col("Icon"), buf,
					resultView.col("Grouping String"), groupStr.c_str(),
					-1);

				// Move the previously top-level row to be under the new parent
				GtkTreeIter child = WulforUtil::copyRow_gui(resultStore, &groupParent, &parent);
				gtk_tree_store_set(resultStore, &child, resultView.col("Grouping String"), groupStr.c_str(), -1);
				gtk_tree_store_remove(resultStore, &groupParent);
				updateParentRow_gui(&parent, &child);

				mapIter->second = parent;
			}

			// Insert the row as a child
			GtkTreeIter child = WulforUtil::copyRow_gui(resultStore, &iter, &parent);
			gtk_tree_store_set(resultStore, &child, resultView.col("Grouping String"), groupStr.c_str(), -1);
			valid = gtk_tree_store_remove(resultStore, &iter);
			updateParentRow_gui(&parent);
		}
	}
}

/*
 * We can't rely on the string from the text box since it will be internationalized.
 */
string Search::getGroupingColumn(GroupType groupBy)
{
	string column;

	switch (groupBy)
	{
		case Search::NOGROUPING:
			break;
		case Search::FILENAME:
			column = _("Filename");
			break;
		case Search::FILEPATH:
			column = _("Path");
			break;
		case Search::SIZE:
			column = _("Size");
			break;
		case Search::CONNECTION:
			column = _("Connection");
			break;
		case Search::TTH:
			column = "TTH";
			break;
		case Search::NICK:
			column = _("Nick");
			break;
		case Search::HUB:
			column = _("Hub");
			break;
		case Search::TYPE:
			column = _("Type");
			break;
		default:
			///@todo: throw an exception
			break;
	}

	return column;
}
/*
gboolean Search::onFocusIn_gui(GtkWidget*, GdkEventFocus*, gpointer data)
{
	Search *s = (Search *)data;
	gtk_widget_grab_focus(s->getWidget("SearchEntry"));

	return TRUE;
}
/*
gboolean Search::onButtonPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	Search *s = (Search *)data;
	s->oldEventType = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(s->resultView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(s->selection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}
/*
gboolean Search::onButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	Search *s = (Search *)data;
	gint count = gtk_tree_selection_count_selected_rows(s->selection);

	if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
		s->popupMenu_gui();
	else if (count == 1 && s->oldEventType == GDK_2BUTTON_PRESS && event->button == 1)
		s->onDownloadClicked_gui(NULL, data);

	return FALSE;
}
/*
gboolean Search::onKeyReleased_gui(GtkWidget* widget, GdkEventKey *event, gpointer data)
{
	Search *s = (Search *)data;
	if (widget == GTK_WIDGET(s->resultView.get()))
	{
		gint count = gtk_tree_selection_count_selected_rows(s->selection);

		if (count > 0)
		{
			if (event->keyval == GDK_KEY_Return)
				s->onDownloadClicked_gui(NULL, data);
			else if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_BackSpace)
				s->onRemoveClicked_gui(NULL, data);
			else if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
				s->popupMenu_gui();
		}
	}
	else
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
			gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
	}

	return FALSE;
}*/
/*
gboolean Search::onSearchEntryKeyPressed_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	Search *s = (Search *)data;

	if (event->keyval == GDK_KEY_Return)
	{
		s->search_gui();
	}

	return FALSE;
}
*/
void Search::onComboBoxChanged_gui(GtkWidget* , gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}

void Search::onGroupByComboBoxChanged_gui(GtkWidget *comboBox, gpointer data)
{
	Search *s = (Search*)data;
	GroupType groupBy = (GroupType)gtk_combo_box_get_active(GTK_COMBO_BOX(comboBox));

	s->ungroup_gui();

	if (groupBy != NOGROUPING)
	{
		gtk_widget_set_sensitive(s->getWidget("checkbuttonFilter"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonSlots"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonShared"), FALSE);
		s->regroup_gui();
	}
	else
	{
		gtk_widget_set_sensitive(s->getWidget("checkbuttonFilter"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonSlots"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonShared"), FALSE);
	}
}

void Search::onSearchButtonClicked_gui(GtkWidget*, gpointer data)
{
	Search *s = (Search *)data;
	s->search_gui();
}

void Search::onFilterButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
	Search *s = (Search *)data;
	GtkComboBox *comboBox = GTK_COMBO_BOX(s->getWidget("comboboxGroupBy"));

	// Disable grouping when filtering within local results
	if (gtk_toggle_button_get_active(button))
	{
		s->previousGrouping = (GroupType)gtk_combo_box_get_active(comboBox);
		gtk_combo_box_set_active(comboBox, (int)NOGROUPING);
		gtk_widget_set_sensitive(GTK_WIDGET(comboBox), FALSE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonSlots"), TRUE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonShared"), TRUE);
	}
	else
	{
		gtk_combo_box_set_active(comboBox, (int)s->previousGrouping);
		gtk_widget_set_sensitive(GTK_WIDGET(comboBox), TRUE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonSlots"), FALSE);
		gtk_widget_set_sensitive(s->getWidget("checkbuttonShared"), FALSE);

	}

	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}

void Search::onSlotsButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
	Search *s = (Search *)data;

	s->onlyFree = gtk_toggle_button_get_active(button);
	if (s->onlyFree != SETTING(SEARCH_ONLY_FREE_SLOTS))
		SettingsManager::getInstance()->set(SettingsManager::SEARCH_ONLY_FREE_SLOTS, s->onlyFree);

	// Refilter current view only if "Search within local results" is enabled
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}

void Search::onSharedButtonToggled_gui(GtkToggleButton*, gpointer data)
{
	Search *s = (Search *)data;

	// Refilter current view only if "Search within local results" is enabled
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}

void Search::onToggledClicked_gui(GtkCellRendererToggle*, gchar *path, gpointer data)
{
	Search *s = (Search *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->hubStore), &iter, path))
	{
		gboolean toggled = s->hubView.getValue<gboolean>(&iter, "Search");
		gtk_list_store_set(s->hubStore, &iter, s->hubView.col("Search"), !toggled, -1);
	}

	// Refilter current view only if "Search within local results" is enabled
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}
/*
void Search::onDownloadClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		typedef Func6<Search, string, string, string, int64_t, string, string> F6;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
					{
						string cid = s->resultView.getString(&iter, "CID");
						string filename = s->resultView.getString(&iter, _("Path"));
						filename += s->resultView.getString(&iter, _("Filename"));
						int64_t size = s->resultView.getValue<int64_t>(&iter, "Real Size");
						string tth = s->resultView.getString(&iter, "TTH");
						string hubUrl = s->resultView.getString(&iter, "Hub URL");
						F6 *func = new F6(s, &Search::download_client, target, cid, filename, size, tth, hubUrl);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;
	string fav = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));

	if (!fav.empty() && gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func6<Search, string, string, string, int64_t, string, string> F6;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
					{
						string cid = s->resultView.getString(&iter, "CID");
						string filename = s->resultView.getString(&iter, _("Path"));
						filename += s->resultView.getString(&iter, _("Filename"));
						int64_t size = s->resultView.getValue<int64_t>(&iter, "Real Size");
						string tth = s->resultView.getString(&iter, "TTH");
						string hubUrl = s->resultView.getString(&iter, "Hub URL");
						F6 *func = new F6(s, &Search::download_client, fav, cid, filename, size, tth, hubUrl);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onDownloadToClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(s->getWidget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		int count = gtk_tree_selection_count_selected_rows(s->selection);
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));

		if (temp && count > 0)
		{
			string target = Text::toUtf8(temp);
			g_free(temp);

			if (target[target.length() - 1] != PATH_SEPARATOR)
				target += PATH_SEPARATOR;

			GtkTreeIter iter;
			GtkTreePath *path;
			GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
			typedef Func6<Search, string, string, string, int64_t, string, string> F6;

			for (GList *i = list; i; i = i->next)
			{
				path = (GtkTreePath *)i->data;
				if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
				{
					bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

					do
					{
						if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
						{
							string cid = s->resultView.getString(&iter, "CID");
							string filename = s->resultView.getString(&iter, _("Path"));
							filename += s->resultView.getString(&iter, _("Filename"));
							int64_t size = s->resultView.getValue<int64_t>(&iter, "Real Size");
							string tth = s->resultView.getString(&iter, "TTH");
							string hubUrl = s->resultView.getString(&iter, "Hub URL");
							F6 *func = new F6(s, &Search::download_client, target, cid, filename, size, tth, hubUrl);
							WulforManager::get()->dispatchClientFunc(func);
						}
					}
					while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
				}
				gtk_tree_path_free(path);
			}
			g_list_free(list);
		}
	}
}
/*
void Search::onDownloadToMatchClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string fileName = WulforUtil::getTextFromMenu(item);
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func5<Search, string, string, int64_t, string, string> F5;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
					{
						string cid = s->resultView.getString(&iter, "CID");
						int64_t size = s->resultView.getValue<int64_t>(&iter, "Real Size");
						string tth = s->resultView.getString(&iter, "TTH");
						string hubUrl = s->resultView.getString(&iter, "Hub URL");
						F5 *func = new F5(s, &Search::addSource_client, fileName, cid, size, tth, hubUrl);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onDownloadDirClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		typedef Func4<Search, string, string, string, string> F4;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
					{
						string cid = s->resultView.getString(&iter, "CID");
						string filename = s->resultView.getString(&iter, _("Path"));
						filename += s->resultView.getString(&iter, _("Filename"));
						string hubUrl = s->resultView.getString(&iter, "Hub URL");
						F4 *func = new F4(s, &Search::downloadDir_client, target, cid, filename, hubUrl);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;
	string fav = (gchar *)g_object_get_data(G_OBJECT(item), "fav");

	if (!fav.empty() && gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func4<Search, string, string, string, string> F4;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
					{
						string cid = s->resultView.getString(&iter, "CID");
						string filename = s->resultView.getString(&iter, _("Path"));
						filename += s->resultView.getString(&iter, _("Filename"));
						string hubUrl = s->resultView.getString(&iter, "Hub URL");
						F4 *func = new F4(s, &Search::downloadDir_client, fav, cid, filename, hubUrl);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onDownloadDirToClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(s->getWidget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		int count = gtk_tree_selection_count_selected_rows(s->selection);
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));

		if (temp && count > 0)
		{
			string target = Text::toUtf8(temp);
			g_free(temp);

			if (target[target.length() - 1] != PATH_SEPARATOR)
				target += PATH_SEPARATOR;

			GtkTreeIter iter;
			GtkTreePath *path;
			GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
			typedef Func4<Search, string, string, string, string> F4;

			for (GList *i = list; i; i = i->next)
			{
				path = (GtkTreePath *)i->data;
				if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
				{
					bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

					do
					{
						if (!gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter))
						{
							string cid = s->resultView.getString(&iter, "CID");
							string filename = s->resultView.getString(&iter, _("Path"));
							filename += s->resultView.getString(&iter, _("Filename"));
							string hubUrl = s->resultView.getString(&iter, "Hub URL");
							F4 *func = new F4(s, &Search::downloadDir_client, target, cid, filename, hubUrl);
							WulforManager::get()->dispatchClientFunc(func);
						}
					}
					while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
				}
				gtk_tree_path_free(path);
			}
			g_list_free(list);
		}
	}
}
/*
void Search::onSearchByTTHClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				string tth = s->resultView.getString(&iter, "TTH");
				if (!tth.empty())
				{
					SearchEntry *ns = WulforManager::get()->getMainWindow()->addSearch_gui();
					ns->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onGetFileListClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func4<Search, string, string, bool, string> F4;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					string cid = s->resultView.getString(&iter, "CID");
					string dir = s->resultView.getString(&iter, _("Path"));
					string hubUrl = s->resultView.getString(&iter, "Hub URL");
					F4 *func = new F4(s, &Search::getFileList_client, cid, dir, FALSE, hubUrl);
					WulforManager::get()->dispatchClientFunc(func);
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onMatchQueueClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func4<Search, string, string, bool, string> F4;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					string cid = s->resultView.getString(&iter, "CID");
					string hubUrl = s->resultView.getString(&iter, "Hub URL");
					F4 *func = new F4(s, &Search::getFileList_client, cid, "", TRUE, hubUrl);
					WulforManager::get()->dispatchClientFunc(func);
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onPrivateMessageClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					string cid = s->resultView.getString(&iter, "CID");
					string hubUrl = s->resultView.getString(&iter, "Hub URL");
					if (!cid.empty())
						WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, cid, hubUrl);
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Search::onAddFavoriteUserClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func1<Search, string> F1;
		F1 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					cid = s->resultView.getString(&iter, "CID");
					func = new F1(s, &Search::addFavUser_client, cid);
					WulforManager::get()->dispatchClientFunc(func);
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onGrantExtraSlotClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func2<Search, string, string> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					string cid = s->resultView.getString(&iter, "CID");
					string hubUrl = s->resultView.getString(&iter, "Hub URL");
					F2 *func = new F2(s, &Search::grantSlot_client, cid, hubUrl);
					WulforManager::get()->dispatchClientFunc(func);
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onRemoveUserFromQueueClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func1<Search, string> F1;
		F1 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					cid = s->resultView.getString(&iter, "CID");
					func = new F1(s, &Search::removeSource_client, cid);
					WulforManager::get()->dispatchClientFunc(func);
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

// Removing a row from treeStore still leaves the SearchResultPtr to results map. This way if a duplicate
// result comes in later it won't be readded, before the results map is cleared with a new search.
void Search::onRemoveClicked_gui(GtkMenuItem*, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		GtkTreeIter iter;
		GtkTreeIter filterIter;
		GtkTreePath *path;
		vector<GtkTreeIter> remove;
		GList *list = g_list_reverse(gtk_tree_selection_get_selected_rows(s->selection, NULL));

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				// Remove the top-level node and it will remove any children nodes (if applicable)
				gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(s->sortedFilterModel), &filterIter, &iter);
				gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(s->searchFilterModel), &iter, &filterIter);

				// обходим функцию gtk_tree_store_remove(s->resultStore, &iter), т.к пути меняются
				remove.push_back(iter);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		// удаляем
		for (vector<GtkTreeIter>::const_iterator it = remove.begin(); it != remove.end(); ++it)
		{
			iter = *it;
			gtk_tree_store_remove(s->resultStore, &iter);
		}
	}
}

void Search::onCopyMagnetClicked_gui(GtkMenuItem* , gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		int64_t size;
		string magnets, magnet, filename, tth;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				bool parent = gtk_tree_model_iter_has_child(s->sortedFilterModel, &iter);

				do
				{
					filename = s->resultView.getString(&iter, _("Filename"));
					size = s->resultView.getValue<int64_t>(&iter, "Real Size");
					tth = s->resultView.getString(&iter, "TTH");
					magnet = WulforUtil::makeMagnet(filename, size, tth);

					if (!magnet.empty())
					{
						if (!magnets.empty())
							magnets += '\n';
						magnets += magnet;
					}
				}
				while (parent && WulforUtil::getNextIter_gui(s->sortedFilterModel, &iter, TRUE, FALSE));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!magnets.empty())
			gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), magnets.c_str(), magnets.length());
	}
}
*/
void Search::onCheckOp_gui(GtkToggleButton*, gpointer data)
{
	Search *s = (Search *)data;
	ClientManager::getInstance()->lock();

	ClientManager::ClientList clients = ClientManager::getInstance()->getClients();

	Client *client = NULL;
	for (auto it = clients.begin(); it != clients.end(); ++it)
	{
		client = (*it).second;
		if (client->isConnected())
		{
			s->modifyHub_gui(client->getHubName(),client->getHubUrl(),client->getMyIdentity().isOp());
		}
	}
}

void Search::parseSearchResult_gui(SearchResultPtr result, StringMap &resultMap)
{
	if (result->getType() == SearchResult::TYPE_FILE)
	{
		string file = WulforUtil::linuxSeparator(result->getFile());
		if (file.rfind('/') == tstring::npos)
		{
			resultMap[_("Filename")] = file;
		}
		else
		{
			resultMap[_("Filename")] = Util::getFileName(file);
			resultMap[_("Path")] = Util::getFilePath(file);
		}

		resultMap["File Order"] = "f" + resultMap[_("Filename")];
		resultMap[_("Type")] = Util::getFileExt(resultMap[_("Filename")]);
		if (!resultMap[_("Type")].empty() && resultMap[_("Type")][0] == '.')
			resultMap[_("Type")].erase(0, 1);
		resultMap[_("Size")] = Util::formatBytes(result->getSize());
		resultMap["Exact Size"] = Util::formatExactSize(result->getSize());
		resultMap["Icon"] = Util::getFileExt(file);
		resultMap["Shared"] = Util::toString(ShareManager::getInstance()->isTTHShared(result->getTTH()));
	}
	else
	{
		string path = WulforUtil::linuxSeparator(result->getFile());
		resultMap[_("Filename")] = WulforUtil::linuxSeparator(result->getFileName());
		resultMap[_("Path")] = Util::getFilePath(path.substr(0, path.length() - 1)); // getFilePath just returns path unless we chop the last / off
		if (resultMap[_("Path")].find("/") == string::npos)
			resultMap[_("Path")] = "";
		resultMap["File Order"] = "d" + resultMap[_("Filename")];
		resultMap[_("Type")] = _("Directory");
		resultMap["Icon"] = "directory";
		resultMap["Shared"] = "0";
		if (result->getSize() > 0)
		{
			resultMap[_("Size")] = Util::formatBytes(result->getSize());
			resultMap["Exact Size"] = Util::formatExactSize(result->getSize());
		}
	}

	resultMap[_("Nick")] = WulforUtil::getNicks(result->getUser(), result->getHubURL());
	resultMap["CID"] = result->getUser()->getCID().toBase32();
	resultMap["Slots"] = result->getSlotString();
	resultMap[_("Connection")] = ClientManager::getInstance()->getConnection(result->getUser()->getCID());
	resultMap[_("Hub")] = result->getHubName().empty() ? result->getHubURL().c_str() : result->getHubName().c_str();
	resultMap["Hub URL"] = result->getHubURL();
	resultMap["IP"] = result->getIP();
	resultMap["Real Size"] = Util::toString(result->getSize());
	if (result->getType() == SearchResult::TYPE_FILE)
		resultMap["TTH"] = result->getTTH().toBase32();

	// assumption: total slots is never above 999
	resultMap["Slots Order"] = Util::toString(-1000 * result->getFreeSlots() - result->getSlots());
	resultMap["Free Slots"] = Util::toString(result->getFreeSlots());
	
}

void Search::download_client(string target, string cid, string filename, int64_t size, string tth, string hubUrl)
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user == NULL)
			return;

		// Only files have a TTH
		if (!tth.empty())
		{
			string subdir = Util::getFileName(filename);
			QueueManager::getInstance()->add(target + subdir, size, TTHValue(tth), HintedUser(user, hubUrl));//NOTE: core 0.762
		}
		else
		{
			string dir = WulforUtil::windowsSeparator(filename);
			QueueManager::getInstance()->addDirectory(dir, HintedUser(user, hubUrl), target);//NOTE: core 0.762
		}
	}
	catch (const Exception&)
	{
	}
}

void Search::downloadDir_client(string target, string cid, string filename, string hubUrl)
{
	try
	{
		string dir;

		// If it's a file (directories are assumed to end in '/')
		if (filename[filename.length() - 1] != PATH_SEPARATOR)
		{
			dir = WulforUtil::windowsSeparator(Util::getFilePath(filename));
		}
		else
		{
			dir = WulforUtil::windowsSeparator(filename);
		}

		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user != NULL)
		{
			QueueManager::getInstance()->addDirectory(dir, HintedUser(user, hubUrl), target);//NOTE: core 0.762
		}
	}
	catch (const Exception&)
	{
	}
}

void Search::addSource_client(string source, string cid, int64_t size, string tth, string hubUrl)
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (!tth.empty() && user != NULL)
		{
			QueueManager::getInstance()->add(source, size, TTHValue(tth), HintedUser(user, hubUrl));//NOTE: core 0.762
		}
	}
	catch (const Exception&)
	{
	}
}

void Search::getFileList_client(string cid, string dir, bool match, string hubUrl)
{
	if (!cid.empty())
	{
		try
		{
			UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
			if (user)
			{
				QueueItem::FileFlags flags;
				if (match)
					flags = QueueItem::FLAG_MATCH_QUEUE;
				else
					flags = QueueItem::FLAG_CLIENT_VIEW;

				QueueManager::getInstance()->addList(HintedUser(user, hubUrl), flags, dir);//NOTE: core 0.762
			}
		}
		catch (const Exception&)
		{
		}
	}
}

void Search::grantSlot_client(string cid, string hubUrl)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
		{
			UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));//NOTE: core 0.762
		}
	}
}

void Search::addFavUser_client(string cid)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void Search::removeSource_client(string cid)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void Search::on(ClientManagerListener::ClientConnected, Client *client) throw()
{
	if (client)
	{
		typedef Func2<Search, string, string> F2;
		F2 *func = new F2(this, &Search::addHub_gui, client->getHubName(), client->getHubUrl());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Search::on(ClientManagerListener::ClientUpdated, Client *client) throw()
{
	if (client)
	{
		typedef Func3<Search, string, string,bool> F3;
		F3 *func = new F3(this, &Search::modifyHub_gui, client->getHubName(), client->getHubUrl(),TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Search::on(ClientManagerListener::ClientDisconnected, Client *client) throw()
{
	if (client)
	{
		typedef Func1<Search, string> F1;
		F1 *func = new F1(this, &Search::removeHub_gui, client->getHubUrl());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Search::on(SearchManagerListener::SR, const SearchResultPtr& result) throw()
{
	if (searchlist.empty() || result == NULL)
		return;

	typedef Func2<Search, string, string> F2;

	if (isHash)
	{
		if (result->getType() != SearchResult::TYPE_FILE || TTHValue(searchlist[0]) != result->getTTH())
		{
			++droppedResult;
			F2 *func = new F2(this, &Search::setStatus_gui, "statusbar3", Util::toString(droppedResult) + _(" filtered"));
			WulforManager::get()->dispatchGuiFunc(func);
			return;
		}
	}
	else
	{
		for (TStringIter i = searchlist.begin(); i != searchlist.end(); ++i)
		{
			if ((*i->begin() != '-' && Util::findSubString(result->getFile(), *i) == (string::size_type)-1) ||
			    (*i->begin() == '-' && i->size() != 1 && Util::findSubString(result->getFile(), i->substr(1)) != (string::size_type)-1))
			{
				++droppedResult;
				F2 *func = new F2(this, &Search::setStatus_gui, "statusbar3", Util::toString(droppedResult) + _(" dropped"));
				WulforManager::get()->dispatchGuiFunc(func);
				return;
			}
		}
	}

	typedef Func1<Search, SearchResultPtr> F1;
	F1 *func = new F1(this, &Search::addResult_gui, result);
	WulforManager::get()->dispatchGuiFunc(func);
}

// Filtering causes Gtk-CRITICAL assertion failure, when last item is removed
// see. http://bugzilla.gnome.org/show_bug.cgi?id=464173
gboolean Search::searchFilterFunc_gui(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	Search *s = (Search *)data;
	dcassert(model == GTK_TREE_MODEL(s->resultStore));

	// Enabler filtering only if search within local results is checked
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		return TRUE;

	// Grouping shouldn't be enabled while filtering, but just in case...
	if (gtk_tree_model_iter_has_child(model, iter))
		return TRUE;

	string hub = s->resultView.getString(iter, "Hub URL", model);
	GtkTreeIter hubIter;
	bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(s->hubStore), &hubIter);
	while (valid)
	{
		if (hub == s->hubView.getString(&hubIter, "Url"))
		{
			if (!s->hubView.getValue<gboolean>(&hubIter, "Search"))
				return FALSE;
			else
				break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->hubStore), &hubIter);
	}

	// Filter based on free slots.
	gint freeSlots = s->resultView.getValue<gint>(iter, "Free Slots", model);
	if (s->onlyFree && freeSlots < 1)
		return FALSE;

	// Hide results already in share
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonShared"))) &&
		s->resultView.getValue<gboolean>(iter, "Shared", model) == TRUE)
		return FALSE;

	// Filter based on search terms.
	/*string filter = Text::toLower(gtk_entry_get_text(GTK_ENTRY(s->getWidget("SearchEntry")/*searchEntry*//*)));
	/*TStringList filterList = StringTokenizer<tstring>(filter, ' ').getTokens();
	string filename = Text::toLower(s->resultView.getString(iter, _("Filename"), model));
	string path = Text::toLower(s->resultView.getString(iter, _("Path"), model));
	for (TStringList::const_iterator term = filterList.begin(); term != filterList.end(); ++term)
	{
		if ((*term)[0] == '-')
		{
			if (filename.find((*term).substr(1)) != string::npos)
				return FALSE;
			else if (path.find((*term).substr(1)) != string::npos)
				return FALSE;
		}
		else if (filename.find(*term) == string::npos && path.find(*term) == string::npos)
			return FALSE;
	}

	// Filter based on file size.
	double filterSize = Util::toDouble(gtk_entry_get_text(GTK_ENTRY(s->getWidget("entrySize"))));
	if (filterSize > 0)
	{
		switch (gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("comboboxUnit"))))
		{
			case 1:
				filterSize *= 1024.0;
				break;
			case 2:
				filterSize *= 1024.0 * 1024.0;
				break;
			case 3:
				filterSize *= 1024.0 * 1024.0 * 1024.0;
				break;
		}

		int64_t size = s->resultView.getValue<int64_t>(iter, "Real Size", model);

		switch (gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("comboboxSize"))))
		{
			case 0:
				if (size != filterSize)
					return FALSE;
				break;
			case 1:
				if (size < filterSize)
					return FALSE;
				break;
			case 2:
				if (size > filterSize)
					return FALSE;
		}
	}

	int type = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("comboboxFile")));
	if (type != SearchManager::TYPE_ANY/* && type != SearchManager::getTypeStr(filename)*//*)
	/*	return FALSE;
*/
	return TRUE;
}

gboolean Search::on_match_select_entry(GtkEntryCompletion*,GtkTreeModel *model, GtkTreeIter *iter, gpointer )
{
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, EN_STRING, &value);
	dcdebug("You have selected %s\n", g_value_get_string(&value));
	g_value_unset(&value);
	return FALSE;
}

gboolean Search::onResultView_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data)
{
	Search* s = (Search*)data;
	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(widget);
	GtkTreeModel *model = gtk_tree_view_get_model (view);
	GtkTreePath *path = NULL;
	g_autofree gchar *filename,*nick,*type,*size,*ppath,*slots,*con,*hub,*exsize,*country,*ip,*tth;
	

//	if(!gtk_tree_view_get_tooltip_context (view, &x, &y,
//					  keyboard_tip,
//					  &model, &path, &iter))
//		return FALSE;

/*	gtk_tree_model_get (model,&iter,
	                    s->resultView.col(_("Filename")),&filename,
	                    s->resultView.col(_("Nick")),&nick,
	                    s->resultView.col(_("Type")),&type,
	                    s->resultView.col(_("Size")),&size,
	                    s->resultView.col(_("Path")),&ppath,
	                    s->resultView.col(_("Slots")),&slots,
	                    s->resultView.col(_("Connection")),&con,
	                    s->resultView.col(_("Hub")),&hub,
	                    s->resultView.col(_("Exact Size")),&exsize,
	                    s->resultView.col("Country"),&country,
	                    s->resultView.col("IP"),&ip,
	                    s->resultView.col("TTH"),&tth,
						-1);
	char buffer[1000];
	g_snprintf(buffer,1000," Filename: %s\n Nick: %s\n Type: %s\n Size: %s\n Path: %s\n Slots: %s\n Connection: %s\n Hub: %s\n Exact Size: %s\n Country: %s\n IP: %s\n TTH: %s\n",
					filename, nick,type,size,ppath,slots,con,hub,exsize,country,ip,tth);

	gtk_tooltip_set_text (_tooltip, buffer);

	gtk_tree_view_set_tooltip_row (view, _tooltip, path);

	gtk_tree_path_free (path);
	/*g_free(filename);
	g_free(nick);
	g_free(type);
	g_free(size);
	g_free(ppath);
	g_free(slots);
	g_free(con);
	g_free(hub);
	g_free(exsize);
	g_free(country);
	g_free(ip);
	g_free(tth);*/
	return TRUE;
}

void Search::selection_changed_result_gui(GtkTreeSelection*, GtkWidget *tree_view)
{
	gtk_widget_trigger_tooltip_query (tree_view);
}

void Search::columnHeader(int num, string name)
{
	GtkTreeViewColumn *col = gtk_tree_view_get_column (resultView.get(), num);
	gtk_tree_view_column_set_clickable (col, TRUE);
	g_object_set (gtk_tree_view_column_get_button(col), "tooltip-text", name.c_str(), NULL);
}

void Search::set_Header_tooltip_gui()
{
	columnHeader(resultView.col(_("Filename")), _("Filename"));
	columnHeader(resultView.col(_("Nick")), _("Nick"));
	columnHeader(resultView.col(_("Type")), _("Type"));
	columnHeader(resultView.col(_("Size")), _("Size"));
	columnHeader(resultView.col(_("Path")), _("Path"));
	columnHeader(resultView.col(_("Slots")), _("Slots"));
	columnHeader(resultView.col(_("Connection")), _("Connection"));
	columnHeader(resultView.col(_("Hub")), _("Hub"));
	columnHeader(resultView.col(_("Exact Size")), _("Exact Size"));
	columnHeader(resultView.col("Country"), "Country");
	columnHeader(resultView.col("IP"), "IP");
	columnHeader(resultView.col("TTH"), "TTH");

}

void Search::onCloseItem(gpointer data)
{
	Search *entry = (Search *)data;
    if(entry)
        WulforManager::get()->getMainWindow()->getSearchEntry()->removeBookEntry_gui(entry);
}

void Search::onAddItem(gpointer )
{
	BookEntry* entry = new Search(string());
    if(entry)
        WulforManager::get()->getMainWindow()->getSearchEntry()->addBookEntry_gui(entry);
}

GMenu *Search::createmenu()
{
	GMenu* menu = g_menu_new();
	GMenuItem* item = g_menu_item_new("Search", NULL);
	
//	if(isMenuCreated) {
	
//		GtkWidget *item = BookEntry::createItemFirstMenu();
//		menu =  gtk_menu_new();
//		GtkWidget *addSearchTab = gtk_menu_item_new_with_label(_("Add Search Tab"));
//		GtkWidget *close = gtk_menu_item_new_with_label(_("Close"));
//		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
//		gtk_menu_shell_append(GTK_MENU_SHELL(menu),close);
//		gtk_menu_shell_append(GTK_MENU_SHELL(menu),addSearchTab);
//		gtk_widget_show(close);
//		gtk_widget_show(addSearchTab);
//		gtk_widget_show(item);
//		gtk_widget_show_all(menu);
//		g_signal_connect_swapped(close, "activate", G_CALLBACK(onCloseItem), (gpointer)this);
//		g_signal_connect_swapped(addSearchTab, "activate", G_CALLBACK(onAddItem), (gpointer)this);
//		isMenuCreated = false;
//	}
//	return menu;
	return menu;
}
