// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
// 

#include <string>
#include <dcpp/SettingsManager.h>
#include <linux/settingsmanager.hh>
#include <linux/treeview.hh>

class SEUtil
{
	public:
	static void reAddItemCo(GtkWidget* parent,GtkWidget* old,GtkWidget* box)
	{
		if((GTK_IS_WIDGET(old) == TRUE) && (gtk_widget_get_realized (old) == TRUE))
				gtk_container_remove(GTK_CONTAINER(parent), old );
		gtk_container_add(GTK_CONTAINER(parent),box);
	}
	
/* Creates a generic checkbox-based options GtkTreeView */	
	static void createOptionsView_gui(TreeView &treeView,GtkListStore *&store)
{
	// Create the view
	treeView.setView(GTK_TREE_VIEW(gtk_tree_view_new ()));
	treeView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	treeView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, -1);
	treeView.insertHiddenColumn("Core Setting", G_TYPE_INT);
	treeView.insertHiddenColumn("UI Setting", G_TYPE_STRING);
	treeView.finalize();

	// Create the store
	store = gtk_list_store_newv(treeView.getColCount(), treeView.getGTypes());
	gtk_tree_view_set_model(treeView.get(), GTK_TREE_MODEL(store));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), treeView.col(_("Name")), GTK_SORT_ASCENDING);

	// Connect the signal handlers
	g_signal_connect(treeView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(onOptionsViewToggled_gui), (gpointer)store);
}

/* Adds a core option */
static void addOption_gui(GtkListStore *store, char* name, dcpp::SettingsManager::BoolSetting setting)
{
	//g_print("Add");
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, dcpp::SettingsManager::getInstance()->get(setting),
		1, name,
		2, (int)setting,
		3, "",
		-1);
}


/* Adds a custom UI specific option */

static void addOption_gui(GtkListStore *store, const std::string &name, const std::string &setting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, WGETI(setting),
		1, name.c_str(),
		2, -2,
		3, setting.c_str(),
		-1);
}

/* Saves the core or UI values stored in the options GtkTreeView */

static void saveOptionsView_gui(TreeView &treeView,GtkListStore* &store, dcpp::SettingsManager *sm)
{
	GtkTreeIter iter;
	GtkTreeModel *m = gtk_tree_view_get_model(treeView.get());
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		gboolean toggled = treeView.getValue<gboolean>(&iter, _("Use"));
		gint coreSetting = treeView.getValue<gint>(&iter, "Core Setting");

		// If core setting has been set to a valid value
		if (coreSetting >= 0)
		{
			sm->set((dcpp::SettingsManager::BoolSetting)coreSetting, toggled);
		}
		else
		{
			std::string uiSetting = treeView.getString(&iter, "UI Setting");
			WSET(std::string(uiSetting), toggled);
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

	static void onOptionsViewToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	GtkTreeIter iter;
	GtkListStore *store = (GtkListStore *)data;
	GtkTreeModel *model = GTK_TREE_MODEL(store);

	if (gtk_tree_model_get_iter_from_string(model, &iter, path))
	{
		gboolean fixed;
		gtk_tree_model_get(model, &iter, 0, &fixed, -1);
		gtk_list_store_set(store, &iter, 0, !fixed, -1);
	}
}

};
