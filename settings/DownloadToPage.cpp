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

#include "DownloadToPage.hh"
#include "definitons.hh"
#include "seUtil.hh"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/typedefs.h"
#include "../dcpp/Text.h"
#include "../dcpp/FavoriteManager.h"

using namespace std;
using namespace dcpp;

const char* DownloadToPage::page_name = "→ Download To";

void DownloadToPage::show(GtkWidget *parent, GtkWidget* old)
{
		GtkTreeIter iter;
		box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		//GtkWidget *scrolled = gtk_scrolled_window_new(NULL,NULL);
		buttonAdd = gtk_button_new_with_label("Add");
		buttonRem = gtk_button_new_with_label("Remove");
		downloadToView = TreeView();
		g_signal_connect(buttonAdd, "clicked", G_CALLBACK(onAddFavorite_gui), (gpointer)this);
		g_signal_connect(buttonRem, "clicked", G_CALLBACK(onRemoveFavorite_gui), (gpointer)this);
		downloadToView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
		downloadToView.insertColumn(_("Favorite Name"), G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.insertColumn(_("Directory"), G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.finalize();
		downloadToStore = gtk_list_store_newv(downloadToView.getColCount(), downloadToView.getGTypes());
		gtk_tree_view_set_model(downloadToView.get(), GTK_TREE_MODEL(downloadToStore));

		//g_signal_connect(downloadToView.get(), "button-release-event", G_CALLBACK(onFavoriteButtonReleased_gui), (gpointer)this);
		gtk_box_append(GTK_BOX(box),GTK_WIDGET(downloadToView.get()));
		//gtk_box_pack_start(GTK_BOX(box),scrolled,TRUE,TRUE,0);
		grid = gtk_grid_new();
		gtk_grid_attach(GTK_GRID(grid),buttonAdd,0,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),buttonRem,1,0,1,1);
		gtk_box_append(GTK_BOX(box),grid);

		SEUtil::reAddItemCo(parent,old,box);
		gtk_widget_set_sensitive(buttonRem, FALSE);

		dcpp::StringPairList directories = dcpp::FavoriteManager::getInstance()->getFavoriteDirs();
		for (auto j = directories.begin(); j != directories.end(); ++j)
		{
			gtk_list_store_append(downloadToStore, &iter);
			gtk_list_store_set(downloadToStore, &iter,
				downloadToView.col(_("Favorite Name")), j->second.c_str(),
				downloadToView.col(_("Directory")), j->first.c_str(),
				-1);
		}

}

void DownloadToPage::onAddFavorite_gui(GtkWidget *widget, gpointer data)
{
	DownloadToPage *s = (DownloadToPage *)data;
	GtkWidget* fileDialog = b_file_dialog_widget("Open Directory");

	gint response = -1;//gtk_dialog_run(GTK_DIALOG(fileDialog));
	//gtk_widget_hide(fileDialog);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = nullptr;// = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(fileDialog));
		if (temp)
		{
			string path = Text::toUtf8(temp);
			//g_free(temp);
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
			gtk_window_set_title(GTK_WINDOW(dialog), _("Favorite name"));
			gtk_editable_set_text(GTK_EDITABLE(entry), "");
			gtk_label_set_markup(GTK_LABEL(label), _("<b>Under what name you see the directory</b>"));
//			gtk_widget_show_all(box);
			response = -1;//gtk_dialog_run(GTK_DIALOG(dialog));
//			gtk_widget_hide(dialog);

			if (response == GTK_RESPONSE_OK)
			{
				string name = gtk_editable_get_text(GTK_EDITABLE(entry));
				if (path[path.length() - 1] != PATH_SEPARATOR)
					path += PATH_SEPARATOR;

				if (!name.empty() && FavoriteManager::getInstance()->addFavoriteDir(path, name))
				{
					GtkTreeIter iter;
					gtk_list_store_append(s->downloadToStore, &iter);
					gtk_list_store_set(s->downloadToStore, &iter,
						s->downloadToView.col(_("Favorite Name")), name.c_str(),
						s->downloadToView.col(_("Directory")), path.c_str(),
						-1);
				}
				else
				{
				//	s->showErrorDialog(_("Directory or favorite name already exists"));
				}
			}
		}
	}
}

void DownloadToPage::onRemoveFavorite_gui(GtkWidget *widget, gpointer data)
{
	DownloadToPage *s = (DownloadToPage *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string path = s->downloadToView.getString(&iter, _("Directory"));
		if (FavoriteManager::getInstance()->removeFavoriteDir(path))
		{
			gtk_list_store_remove(s->downloadToStore, &iter);
			gtk_widget_set_sensitive(s->buttonRem, FALSE);
		}
	}
}
/*
gboolean DownloadToPage::onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadToPage *s = (DownloadToPage *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		gtk_widget_set_sensitive(s->buttonRem, TRUE);
	else
		gtk_widget_set_sensitive(s->buttonRem, FALSE);

	return FALSE;
}
*/
