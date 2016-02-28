/*
 * Copyright © 2009-2012 freedcpp, http://code.google.com/p/freedcpp
 * Copyright © 2011-2016 Parts (CMD supports) of Code BMDC++ , https://launchpad.net/bmdc++
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

#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "adlsearch.hh"

using namespace std;
using namespace dcpp;

SearchADL::SearchADL():
	BookEntry(Entry::SEARCH_ADL, _("ADL Search"), "adlsearch"),
	sens(TRUE),	acts(TRUE),
	forbid(TRUE)
{
	#if !GTK_CHECK_VERSION(3,12,0)		
	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("ADLSearchDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	#endif
	g_object_ref_sink(getWidget("menu"));

	// Fill drop down actions
	auto action = WulforUtil::getActions();
	WulforUtil::drop_combo(getWidget("comboboxAction"),action);
	
	// Initialize search list treeview
	searchADLView.setView(GTK_TREE_VIEW(getWidget("searchADLView")));
	searchADLView.insertColumn(_("Enabled"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	searchADLView.insertColumn(_("Search String"), G_TYPE_STRING, TreeView::STRING, 100);
	searchADLView.insertColumn(_("Source Type"), G_TYPE_STRING, TreeView::STRING, 90);
	searchADLView.insertColumn(_("Destination Directory"), G_TYPE_STRING, TreeView::STRING, 90);
	searchADLView.insertColumn(_("Min Size"), G_TYPE_STRING, TreeView::STRING, 100);
	searchADLView.insertColumn(_("Max Size"), G_TYPE_STRING, TreeView::STRING, 100);
	searchADLView.insertColumn(_("Forbiden File"), G_TYPE_STRING, TreeView::STRING,80);
	searchADLView.insertColumn(_("Comment"), G_TYPE_STRING, TreeView::STRING, 100);

	searchADLView.insertHiddenColumn("Download Matches", G_TYPE_BOOLEAN);
	searchADLView.insertHiddenColumn("MinSize", G_TYPE_INT64);
	searchADLView.insertHiddenColumn("MaxSize", G_TYPE_INT64);
	searchADLView.insertHiddenColumn("SourceType", G_TYPE_INT);
	searchADLView.insertHiddenColumn("SizeType", G_TYPE_INT);
	///CMD
	searchADLView.insertHiddenColumn("AdlsPoint", G_TYPE_INT);
	searchADLView.insertHiddenColumn("ARaw", G_TYPE_INT);
	searchADLView.insertHiddenColumn("FromFav", G_TYPE_BOOLEAN);
	searchADLView.insertHiddenColumn("OverRideP", G_TYPE_BOOLEAN);
	searchADLView.insertHiddenColumn("KickString",G_TYPE_STRING);
	searchADLView.finalize();

	searchADLStore = gtk_list_store_newv(searchADLView.getColCount(), searchADLView.getGTypes());
	gtk_tree_view_set_model(searchADLView.get(), GTK_TREE_MODEL(searchADLStore));
	g_object_unref(searchADLStore);

	searchADLSelection = gtk_tree_view_get_selection(searchADLView.get());
	gtk_tree_selection_set_mode(searchADLSelection, GTK_SELECTION_SINGLE);

	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Enabled"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Search String"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Source Type"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Destination Directory"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Min Size"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Max Size"))), FALSE);

	g_signal_connect( searchADLView.getCellRenderOf(_("Enabled")) , "toggled", G_CALLBACK(onActiveToggled_gui), (gpointer)this);

	g_signal_connect(getWidget("addItem"), "activate", G_CALLBACK(onAddClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesItem"), "activate", G_CALLBACK(onPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveClicked_gui), (gpointer)this);

	g_signal_connect(getWidget("addButton"), "clicked", G_CALLBACK(onAddClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesButton"), "clicked", G_CALLBACK(onPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("moveUpButton"), "clicked", G_CALLBACK(onMoveUpClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("moveDownButton"), "clicked", G_CALLBACK(onMoveDownClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeButton"), "clicked", G_CALLBACK(onRemoveClicked_gui), (gpointer)this);

	g_signal_connect(searchADLView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(searchADLView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(searchADLView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

	g_signal_connect(getWidget("checkoveride1"), "toggled", G_CALLBACK(onToggleOveride), (gpointer)this);
	g_signal_connect(getWidget("checkFromFav"), "toggled", G_CALLBACK(onToggleActions), (gpointer)this);

	g_signal_connect(getWidget("checkForbiden"), "toggled", G_CALLBACK(onToggleForb), (gpointer)this);

	g_signal_connect(getWidget("sourceTypeComboBox"),"changed", G_CALLBACK(onChangeCombo),(gpointer)this);

	gtk_widget_set_sensitive(getWidget("checkFromFav"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryKick"), FALSE);
	gtk_widget_set_sensitive(getWidget("checkAction"), FALSE);
	gtk_widget_set_sensitive(getWidget("comboboxAction"), FALSE);
	gtk_widget_set_sensitive(getWidget("spinbuttonPoints"), FALSE);
	gtk_widget_set_sensitive(getWidget("checkoveride1"), FALSE);
	gtk_widget_set_sensitive(getWidget("checkcasesensitive"), FALSE);
}

SearchADL::~SearchADL()
{
	ADLSearchManager::getInstance()->save();
	gtk_widget_destroy(getWidget("ADLSearchDialog"));
	g_object_unref(getWidget("menu"));
}

void SearchADL::show()
{
	// initialize searches list
	string minSize, maxSize;
	ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
	
	for (ADLSearchManager::SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i)
	{
		GtkTreeIter iter;
		ADLSearch &search = *i;
		minSize = search.minFileSize >= 0 ? Util::toString(search.minFileSize) + " " +
			search.SizeTypeToString(search.typeFileSize) : "";
		maxSize = search.maxFileSize >= 0 ? Util::toString(search.maxFileSize) + " " +
			search.SizeTypeToString(search.typeFileSize) : "";

		gtk_list_store_append(searchADLStore, &iter);
		gtk_list_store_set(searchADLStore, &iter,
			searchADLView.col(_("Enabled")), search.isActive,
			searchADLView.col(_("Search String")), search.searchString.c_str(),
			searchADLView.col(_("Source Type")), search.SourceTypeToString(search.sourceType).c_str(),
			searchADLView.col(_("Destination Directory")), search.destDir.c_str(),
			searchADLView.col(_("Min Size")), minSize.c_str(),
			searchADLView.col(_("Max Size")), maxSize.c_str(),
			searchADLView.col(_("Forbiden File")), search.isForbidden ? _("1") : _("0"),
			searchADLView.col(_("Comment")), search.adlsComment.c_str(),
			searchADLView.col("Download Matches"), search.isAutoQueue,
			searchADLView.col("MinSize"), search.minFileSize,
			searchADLView.col("MaxSize"), search.maxFileSize,
			searchADLView.col("SourceType"), search.sourceType,
			searchADLView.col("SizeType"), search.typeFileSize,
			searchADLView.col("AdlsPoint"), search.adlsPoints,
			searchADLView.col("ARaw"), search.adlsRaw,
			searchADLView.col("FromFav"), search.fromFavs,
			searchADLView.col("OverRideP"),search.overRidePoints,
			searchADLView.col("KickString"), search.kickString.c_str(),
			-1);
	}
}

void SearchADL::onRemoveClicked_gui(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	GtkTreeIter iter;
	
	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (i < collection.size())
		{
			collection.erase(collection.begin() + i);
			gtk_list_store_remove(s->searchADLStore, &iter);
		}
	}
}

void SearchADL::onAddClicked_gui(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	ADLSearch search;
	
	if (showPropertiesDialog_gui(search, false, s))
	{
		GtkTreeIter iter;
		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;

		if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
		{
			gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
			SearchType i = (SearchType)Util::toInt(p);
			g_free(p);

			if (i < collection.size())
			{
				GtkTreeIter it;
				collection.insert(collection.begin() + i, search);
				gtk_list_store_insert_before(s->searchADLStore, &it, &iter);
				s->setSearch_gui(search, &it);
			}
		}
		else
		{
			collection.push_back(search);
			gtk_list_store_append(s->searchADLStore, &iter);
			s->setSearch_gui(search, &iter);
		}
	}
}

void SearchADL::onPropertiesClicked_gui(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	GtkTreeIter iter;
	
	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
	{
		ADLSearch search;
		
		if (showPropertiesDialog_gui(search, true, s))
		{
			gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
			SearchType i = (SearchType)Util::toInt(p);
			g_free(p);

			ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
			if (i < collection.size())
			{
				collection[i] = search;
				s->setSearch_gui(search, &iter);
			}
		}
	}
}

void SearchADL::setSearch_gui(ADLSearch &search, GtkTreeIter *iter)
{
	string minSize = search.minFileSize >= 0 ? Util::toString(search.minFileSize) + " " +
		search.SizeTypeToString(search.typeFileSize) : "";
	string maxSize = search.maxFileSize >= 0 ? Util::toString(search.maxFileSize) + " " +
		search.SizeTypeToString(search.typeFileSize) : "";

	gtk_list_store_set(searchADLStore, iter,
		searchADLView.col(_("Enabled")), search.isActive,
		searchADLView.col(_("Search String")), search.searchString.c_str(),
		searchADLView.col(_("Source Type")), search.SourceTypeToString(search.sourceType).c_str(),
		searchADLView.col(_("Destination Directory")), search.destDir.c_str(),
		searchADLView.col(_("Min Size")), minSize.c_str(),
		searchADLView.col(_("Max Size")), maxSize.c_str(),
		searchADLView.col(_("Forbiden File")), search.isForbidden ? _("1") : _("0"),
		searchADLView.col(_("Comment")), search.adlsComment.c_str(),
		searchADLView.col("Download Matches"), search.isAutoQueue,
		searchADLView.col("MinSize"), search.minFileSize,
		searchADLView.col("MaxSize"), search.maxFileSize,
		searchADLView.col("SourceType"), search.sourceType,
		searchADLView.col("SizeType"), search.typeFileSize,
		searchADLView.col("AdlsPoint"), search.adlsPoints,
		searchADLView.col("ARaw"), search.adlsRaw,
		searchADLView.col("FromFav"), search.fromFavs,
		searchADLView.col("OverRideP"), search.overRidePoints,
		searchADLView.col("KickString"),search.kickString.c_str(),
		-1);
}

bool SearchADL::showPropertiesDialog_gui(ADLSearch &search, bool edit, SearchADL *s)
{
	GtkTreeIter iter;
	if (edit && !gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
		return false;

	string searchString, destDir;
	gboolean enabledCheck, matchesCheck;
	string isForbid, commentStr, kickStr;
	gint sizeType, sourceType;
	gdouble minSize, maxSize;
	int raw = 0 ,point;
	gboolean isFav;
	bool isFavs;
	bool overide = false;

	if (edit)
	{
		searchString = s->searchADLView.getString(&iter, _("Search String"));
		destDir = s->searchADLView.getString(&iter, _("Destination Directory"));
		minSize = s->searchADLView.getValue<int64_t>(&iter, "MinSize");
		maxSize = s->searchADLView.getValue<int64_t>(&iter, "MaxSize");
		sourceType = s->searchADLView.getValue<gint>(&iter, "SourceType");
		sizeType = s->searchADLView.getValue<gint>(&iter, "SizeType");
		enabledCheck = s->searchADLView.getValue<gboolean>(&iter, _("Enabled"));
		matchesCheck = s->searchADLView.getValue<gboolean>(&iter, "Download Matches");
		///CMD
		isForbid = s->searchADLView.getString(&iter, _("Forbiden File"));
		commentStr = s->searchADLView.getString(&iter, _("Comment"));
		raw = s->searchADLView.getValue<gint>(&iter, "ARaw");
		isFav = s->searchADLView.getValue<gboolean>(&iter, "FromFav");
		kickStr = s->searchADLView.getString(&iter, "KickString");
		point = s->searchADLView.getValue<gint>(&iter, "AdlsPoint");
		overide = s->searchADLView.getValue<gboolean>(&iter, "OverRideP");

		// set text
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("searchStringEntry")), searchString.c_str());
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), destDir.c_str());

		// set size
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("minFileSizeSpinButton")), minSize);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("maxFileSizeSpinButton")), maxSize);

		// set types
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sizeTypeComboBox")), sizeType);
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")), sourceType);

		// set checks
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("enabledCheckButton")), enabledCheck);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("downloadMatchesCheckButton")), matchesCheck);
		///CMD
		//Forbiden
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkForbiden")), (isForbid == _("1")) ? TRUE : FALSE);
		//Comment
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryComment")), commentStr.c_str());
		//raw
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("comboboxAction")), (gint)(s->find_rawInt(raw)) );
		//FromFav
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkFromFav")), isFav);
		//KickSTr
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryKick")), kickStr.c_str());
		//Points
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("spinbuttonPoints")),(gint)point);
		//Overide
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkoveride1")),overide);

	}
	else
	{
		// set text default
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("searchStringEntry")), search.searchString.c_str());
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), search.destDir.c_str());

		// set size default
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("minFileSizeSpinButton")), search.minFileSize);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("maxFileSizeSpinButton")), search.maxFileSize);

		// set type default
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sizeTypeComboBox")), search.typeFileSize);
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")), search.sourceType);

		// set checks default
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("enabledCheckButton")), search.isActive);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("downloadMatchesCheckButton")), search.isAutoQueue);
		///CMD
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkForbiden")), FALSE);
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryComment")), "");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkFromFav")), FALSE);
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("entryKick")), "");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("spinbuttonPoints")),(gint)0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("checkoveride1")),false);
	}

	GtkWidget *dialog = s->getWidget("ADLSearchDialog");
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return false;

	gtk_widget_hide(dialog);

	if (response != GTK_RESPONSE_OK)
		return false;

	// set search
	enabledCheck = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("enabledCheckButton")));
	matchesCheck = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("downloadMatchesCheckButton")));
	searchString = gtk_entry_get_text(GTK_ENTRY(s->getWidget("searchStringEntry")));
	destDir = gtk_entry_get_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")));
	minSize = gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("minFileSizeSpinButton")));
	maxSize = gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("maxFileSizeSpinButton")));
	sizeType = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("sizeTypeComboBox")));
	sourceType = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")));
	///CMD
	gchar *tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->getWidget("comboboxAction")));
	if(tmp) {
		raw = s->find_raw(tmp);
	}
	g_free(tmp);
	isFavs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkFromFav")));
	kickStr = gtk_entry_get_text(GTK_ENTRY(s->getWidget("entryKick")));
	point = (int)gtk_spin_button_get_value (GTK_SPIN_BUTTON(s->getWidget("spinbuttonPoints")));
	
	if(gtk_widget_is_sensitive(s->getWidget("checkoveride1")))
		overide = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkoveride1")));

	search.isActive = enabledCheck;
	search.isAutoQueue = matchesCheck;
	search.searchString = searchString;
	search.destDir = destDir;
	search.minFileSize = minSize;
	search.maxFileSize = maxSize;
	search.sourceType = (ADLSearch::SourceType)sourceType;
	search.typeFileSize = (ADLSearch::SizeType)sizeType;
	///CMD
	search.isForbidden = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkForbiden"))) ? true : false;
	search.adlsComment = gtk_entry_get_text(GTK_ENTRY(s->getWidget("entryComment")));
	search.adlsRaw = raw;
	search.fromFavs = isFavs;
	search.kickString = kickStr;
	search.adlsPoints = point;
	search.overRidePoints = overide;

	return true;
}

void SearchADL::onMoveUpClicked_gui(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(s->searchADLStore);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->searchADLView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &current);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (i == 0 || !(i < collection.size()))
			return;

		bool swap = false;
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
		{
			swap = true;
			gtk_list_store_swap(s->searchADLStore, &current, &prev);
		}
		gtk_tree_path_free(path);

		if (swap)
			std::swap(collection[i], collection[i - 1]);
	}
}

void SearchADL::onMoveDownClicked_gui(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	GtkTreeIter current, next;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->searchADLView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &current);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (collection.empty() || !(i < collection.size() - 1))
			return;

		bool swap = false;
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->searchADLStore), &next))
		{
			swap = true;
			gtk_list_store_swap(s->searchADLStore, &current, &next);
		}

		if (swap)
			std::swap(collection[i], collection[i + 1]);
	}
}

void SearchADL::onActiveToggled_gui(GtkCellRendererToggle* , gchar *path, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->searchADLStore), &iter, path))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (i < collection.size())
		{
			gboolean active = s->searchADLView.getValue<gboolean>(&iter, _("Enabled"));
			active = !active;
			gtk_list_store_set(s->searchADLStore, &iter, s->searchADLView.col(_("Enabled")), active, -1);

			ADLSearch search = collection[i];
			search.isActive = active;
			collection[i] = search;
		}
	}
}

gboolean SearchADL::onButtonPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);
	s->previous = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;

		if (gtk_tree_view_get_path_at_pos(s->searchADLView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(s->searchADLSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean SearchADL::onButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, NULL))
	{
		if (event->button == 1 && s->previous == GDK_2BUTTON_PRESS)
		{
			// show dialog
			onPropertiesClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			// show menu
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean SearchADL::onKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, NULL))
	{
		if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_BackSpace)
		{
			s->onRemoveClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}
/* Util funtions */
int SearchADL::find_raw(string rawString)
{
	if(rawString.empty())
		return 0;
	int raw = 0;
	
	auto act = WulforUtil::getActions();
	
	for (auto it = act.begin(); it != act.end(); ++it)
	{
		if(it->first == rawString)
			raw = it->second;
	}
	
  return raw;
}

int SearchADL::find_rawInt(int raw)
{
	if(raw == 0)
		return 0;
		
	int _raw = 0;
	int i = 0;
	auto act = WulforUtil::getActions();
	for (auto it = act.begin(); it != act.end(); ++it)
	{
		i++;
		if(it->second == raw)
		{ _raw =(i-1);break; }
	}
  return _raw;
}
//end
void SearchADL::onToggleOveride(GtkWidget*, gpointer data) 
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	gtk_widget_set_sensitive(s->getWidget("checkFromFav"), s->sens);
	gtk_widget_set_sensitive(s->getWidget("entryKick"), s->sens);
	gtk_widget_set_sensitive(s->getWidget("checkAction"), s->sens);
	gtk_widget_set_sensitive(s->getWidget("comboboxAction"), FALSE);
	s->sens = !s->sens;
}

void SearchADL::onToggleActions(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);

	gtk_widget_set_sensitive(s->getWidget("comboboxAction"), s->acts);
	gtk_widget_set_sensitive(s->getWidget("checkAction"), s->acts);
	s->acts = !s->acts;
}

void SearchADL::onChangeCombo(GtkWidget *widget, gpointer data)
{
    SearchADL *s = reinterpret_cast<SearchADL *>(data);

    gint type = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

    if(!s->forbid) {
		
		switch(type) {
    case 0:
        gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")),"Forbidden Files");
        break;
    case 1:
         gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")),"Forbidden Directories");
         break;
    case 2:
        gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")),"Forbidden Full Paths");
        break;
    case 3:
        gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")),"Forbidden TTHS");
        break;
    default: return;
	}
	
  }
  
}

void SearchADL::onToggleForb(GtkWidget*, gpointer data)
{
	SearchADL *s = reinterpret_cast<SearchADL *>(data);
	string tmp;
	gint type;

	gtk_widget_set_sensitive(s->getWidget("checkcasesensitive"), s->forbid);
	gtk_widget_set_sensitive(s->getWidget("checkoveride1"), s->forbid);
	gtk_widget_set_sensitive(s->getWidget("spinbuttonPoints"), s->forbid);

	tmp = string(gtk_entry_get_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry"))));
	if( (tmp != "Forbidden Files") || (tmp != "Forbidden TTHS") || (tmp != "Forbidden Directories"))
	{
		//todo check this...
        type = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")));
        switch(type)
        {
        case 0:
           gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), "Forbidden Files");
           gtk_widget_set_sensitive(s->getWidget("destinationDirectoryEntry"), !s->forbid);
           break;
        case 1:
           gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), "Forbidden Directories");
           gtk_widget_set_sensitive(s->getWidget("destinationDirectoryEntry"), !s->forbid);
           break;
        case 2:
           gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")),"Forbidden Full Paths");
           gtk_widget_set_sensitive(s->getWidget("destinationDirectoryEntry"), !s->forbid);
           break;
        case 3:
           gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), "Forbidden TTHS");
           gtk_widget_set_sensitive(s->getWidget("destinationDirectoryEntry"), !s->forbid);
           break;
        default: gtk_widget_set_sensitive(s->getWidget("destinationDirectoryEntry"), !s->forbid);;
		}

	}
	else gtk_widget_set_sensitive(s->getWidget("destinationDirectoryEntry"), !s->forbid);

	s->forbid = !s->forbid;

}
