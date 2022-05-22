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

#include "SharingPage.hh"
#include "definitons.hh"
#include "seUtil.hh"
#include "../linux/treeview.hh"
#include "../dcpp/format.h"
#include "../dcpp/ShareManager.h"
#include "../dcpp/Util.h"

using namespace std;
using namespace dcpp;

const char* SharingPage::page_name = "Sharing";

void SharingPage::show(GtkWidget *parent, GtkWidget *old)
{
	GtkWidget *scroll = sw_new;
	shareView = TreeView (); 
	shareView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
	shareView.insertColumn(_("Virtual Name"), G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn(_("Directory"), G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn(_("Size"), G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	shareView.finalize();
	shareStore = gtk_list_store_newv(shareView.getColCount(), shareView.getGTypes());
	gtk_tree_view_set_model(shareView.get(), GTK_TREE_MODEL(shareStore));
	shareView.setSortColumn_gui(_("Size"), "Real Size");
	//gtk_container_add(GTK_CONTAINER(scroll),GTK_WIDGET(shareView.get()));

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_box_append(GTK_BOX(box),scroll);

	button_add = gtk_button_new_with_label("Add");
	button_rem = gtk_button_new_with_label("Remove");
//	button_edit = gtk_button_new_with_label("Remove");
	grid = gtk_grid_new();	
	gtk_grid_attach(GTK_GRID(grid),button_add,0,1,1,1);
	gtk_grid_attach(GTK_GRID(grid),button_rem,1,1,1,1);
//	gtk_grid_attach(GTK_GRID(grid),button_edit,2,0,1,1);
	labelShareSize = gtk_label_new("");
	gtk_grid_attach(GTK_GRID(grid),labelShareSize,2,2,1,1);
	
	gtk_box_append(GTK_BOX(box),grid);

//	if(old != NULL)
//		gtk_container_remove(GTK_CONTAINER(parent), old );
//	gtk_container_add(GTK_CONTAINER(parent),box);
	SEUtil::reAddItemCo(parent,old,box);

	//g_signal_connect(shareView.get(), "button-release-event", G_CALLBACK(onShareButtonReleased_gui), (gpointer)this);
	g_signal_connect(button_add, "clicked", G_CALLBACK(onAddShare_gui), (gpointer)this);
	g_signal_connect(button_rem, "clicked", G_CALLBACK(onRemoveShare_gui), (gpointer)this);
	gtk_widget_set_sensitive(button_rem, FALSE);
	updateShares_gui();

}
/*
gboolean SharingPage::onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	SharingPage *s = (SharingPage *)data;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (gtk_tree_selection_count_selected_rows(selection) == 0)
		gtk_widget_set_sensitive(s->button_rem, FALSE);
	else
		gtk_widget_set_sensitive(s->button_rem, TRUE);

	return FALSE;
}
*/
void SharingPage::onAddShare_gui(GtkWidget *widget, gpointer data)
{
	SharingPage *s = (SharingPage*)data;
	GtkWidget* fileDialog = b_file_dialog_widget("Open Directory");
	int response = -1;
 	//gint response = gtk_dialog_run(GTK_DIALOG(fileDialog));
	//gtk_widget_hide(fileDialog);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp ="r"; //gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(fileDialog));
		if (temp)
		{
			string path = temp;
			g_free(temp);

			if (path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;

			GtkWidget* dialog = gtk_dialog_new_with_buttons ("Favorite name",
                                      NULL,
                                     (GtkDialogFlags)(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
                                      _("_OK"),
                                      GTK_RESPONSE_OK,
                                      _("_Cancel"),
                                      GTK_RESPONSE_CANCEL,
                                      NULL);

			GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
			GtkWidget *entry = gtk_entry_new();
			GtkWidget *label = gtk_label_new("");
			gtk_box_append(GTK_BOX(box),label);
			gtk_box_append(GTK_BOX(box),entry);
			gtk_window_set_title(GTK_WINDOW(dialog), _("Virtual name"));
			gtk_editable_set_text(GTK_EDITABLE(entry), "");
			gtk_label_set_markup(GTK_LABEL(label), _("<b>Name under which the others see the directory</b>"));
			gtk_widget_show(box);
			response = -1;//gtk_dialog_run(GTK_DIALOG(dialog));
			string name = gtk_editable_get_text(GTK_EDITABLE(entry));
			gtk_widget_hide(dialog);

			if (response == GTK_RESPONSE_OK)
			{
				try
				{
					ShareManager::getInstance()->addDirectory(path, name);
				}
				catch (const ShareException &e)
				{
				//	s->showErrorDialog(e.getError());
					return;//should not update GUI if any Share* exception hapened
				}
				catch(...){g_print("Some other exception");}
				
				s->addShare_gui(path, name);
			}
		}
	}
}

void SharingPage::onRemoveShare_gui(GtkWidget *widget, gpointer data)
{
	SharingPage *s = (SharingPage *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string path = s->shareView.getString(&iter, _("Directory"));
		gtk_list_store_remove(s->shareStore, &iter);
		gtk_widget_set_sensitive(s->button_rem, FALSE);

		ShareManager::getInstance()->removeDirectory(path);
	}
}


void SharingPage::updateShares_gui()
{
	GtkTreeIter iter;
	int64_t size = 0;
	string vname;

	gtk_list_store_clear(shareStore);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it)
	{
		size = ShareManager::getInstance()->getShareSize(it->second);

		if (size == -1 && !SETTING(SHARE_HIDDEN))
		{
			vname = _("[HIDDEN SHARE] ") + it->first;
			size = 0;
		} else
			vname = it->first;

		gtk_list_store_append(shareStore, &iter);
		gtk_list_store_set(shareStore, &iter,
			shareView.col(_("Virtual Name")), vname.c_str(),
			shareView.col(_("Directory")), it->second.c_str(),
			shareView.col(_("Size")), Util::formatBytes(size).c_str(),
			shareView.col("Real Size"), size,
			-1);
	}

	string text = _("Total size: ") + Util::formatBytes(ShareManager::getInstance()->getShareSize());
	gtk_label_set_text(GTK_LABEL(labelShareSize), text.c_str());
}

void SharingPage::addShare_gui(string path, string name)
{
	int64_t size = ShareManager::getInstance()->getShareSize(path);
	GtkTreeIter iter;
	gtk_list_store_append(shareStore, &iter);
	gtk_list_store_set(shareStore, &iter,
		shareView.col(_("Virtual Name")), name.c_str(),
		shareView.col(_("Directory")), path.c_str(),
		shareView.col(_("Size")), Util::formatBytes(size).c_str(),
		shareView.col("Real Size"), size,
		-1);
}
