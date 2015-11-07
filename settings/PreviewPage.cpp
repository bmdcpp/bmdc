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

#include "PreviewPage.hh"
#include "definitons.hh"
#include "seUtil.hh"
#include <dcpp/format.h>
#include <linux/treeview.hh>
#include <linux/settingsmanager.hh>

using namespace std;

const char* PreviewPage::page_name = "→ Preview";

void PreviewPage::show(GtkWidget *parent, GtkWidget* old)
{
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	GtkWidget* scroll = gtk_scrolled_window_new(NULL,NULL);

	previewAppView = TreeView();
	previewAppView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
	previewAppView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, -1);
	previewAppView.insertColumn(_("Application"), G_TYPE_STRING, TreeView::STRING, -1);
	previewAppView.insertColumn(_("Extensions"), G_TYPE_STRING, TreeView::STRING, -1);
	previewAppView.finalize();
	previewAppToStore = gtk_list_store_newv(previewAppView.getColCount(), previewAppView.getGTypes());
	gtk_tree_view_set_model(previewAppView.get(), GTK_TREE_MODEL(previewAppToStore));
		
	gtk_container_add(GTK_CONTAINER(scroll),GTK_WIDGET(previewAppView.get()));
	gtk_box_pack_start(GTK_BOX(box),scroll,TRUE,TRUE,0);
	grid = gtk_grid_new();
	GtkWidget *addButton = gtk_button_new_with_label("Add");
	gtk_grid_attach(GTK_GRID(grid),addButton,0,0,1,1);
	GtkWidget *remButton = gtk_button_new_with_label("Remove");
	gtk_grid_attach(GTK_GRID(grid),remButton,1,0,1,1);
	GtkWidget *upButton = gtk_button_new_with_label("Apply");
	gtk_grid_attach(GTK_GRID(grid),upButton,2,0,1,1);
		
	g_signal_connect(addButton, "clicked", G_CALLBACK(onPreviewAdd_gui), (gpointer)this);
	g_signal_connect(remButton, "clicked", G_CALLBACK(onPreviewRemove_gui), (gpointer)this);
	g_signal_connect(upButton, "clicked", G_CALLBACK(onPreviewApply_gui), (gpointer)this);
	g_signal_connect(previewAppView.get(), "key-release-event", G_CALLBACK(onPreviewKeyReleased_gui), (gpointer)this);
	g_signal_connect(previewAppView.get(), "button-release-event", G_CALLBACK(onPreviewButtonReleased_gui), (gpointer)this);

	gtk_box_pack_start(GTK_BOX(box),grid,TRUE,TRUE,0);

	gtk_widget_set_sensitive(addButton, TRUE);
	gtk_widget_set_sensitive(remButton, TRUE);
	gtk_widget_set_sensitive(upButton, TRUE);
		/*update*/
	GtkWidget *frame = gtk_frame_new(_("Info"));
	GtkWidget *grid2 = gtk_grid_new();
	entry_name = gen;
	entry_type = gen;
	entry_app  = gen;
	
	gtk_grid_attach(GTK_GRID(grid2),gtk_label_new("Name"),0,0,1,1);
	gtk_grid_attach(GTK_GRID(grid2),entry_name,1,0,1,1);

	gtk_grid_attach(GTK_GRID(grid2),gtk_label_new("Application"),0,1,1,1);
	gtk_grid_attach(GTK_GRID(grid2),entry_app,1,1,1,1);

	gtk_grid_attach(GTK_GRID(grid2),gtk_label_new("Types"),0,2,1,1);
	gtk_grid_attach(GTK_GRID(grid2),entry_type,1,2,1,1);
	gtk_container_add(GTK_CONTAINER(frame),grid2);
	gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,0);
	infoLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(box),infoLabel,TRUE,TRUE,0);
		
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	const PreviewApp::List &Apps = wsm->getPreviewApps();

		// add default applications players
		if (Apps.empty())
		{
			wsm->addPreviewApp("Xine player", "xine --no-logo --session volume=50", "avi; mov; vob; mpg; mp3");
			wsm->addPreviewApp("Kaffeine player", "kaffeine -p", "avi; mov; mpg; vob; mp3");
			wsm->addPreviewApp("Mplayer player", "mplayer", "avi; mov; vob; mp3");
			wsm->addPreviewApp("Amarok player", "amarok", "mp3");
		}
		GtkTreeIter iter;
		for (auto item = Apps.begin(); item != Apps.end(); ++item)
		{
			gtk_list_store_append(previewAppToStore, &iter);
			dcdebug("\n%s -%s -%s\n", (*item)->name.c_str(), (*item)->app.c_str(), (*item)->ext.c_str() );
			
			gtk_list_store_set(previewAppToStore, &iter,
				previewAppView.col(_("Name"))/*0*/, ((*item)->name).c_str(),
				previewAppView.col(_("Application"))/*1*/, ((*item)->app).c_str(),
				previewAppView.col(_("Extensions"))/*2*/, ((*item)->ext).c_str(),
				-1);
		}
		SEUtil::reAddItemCo(parent,old,box);
}

void PreviewPage::write()
{/**/}


void PreviewPage::onPreviewAdd_gui(GtkWidget *widget, gpointer data)
{
	PreviewPage *s = (PreviewPage*)data;

	string name = gtk_entry_get_text(GTK_ENTRY(s->entry_name));
	string app = gtk_entry_get_text(GTK_ENTRY(s->entry_app));
	string ext = gtk_entry_get_text(GTK_ENTRY(s->entry_type));

	if (name.empty() || app.empty() || ext.empty())
	{
		s->showErrorDialog(_("Must not be empty..."));
		return;
	}

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	if (wsm->getPreviewApp(name))
	{
		s->showErrorDialog(_("Error"));
		return;
	}

	if (wsm->addPreviewApp(name, app, ext) != NULL)
	{
		GtkTreeIter it;
		gtk_list_store_append(s->previewAppToStore, &it);
		gtk_list_store_set(s->previewAppToStore, &it,
			s->previewAppView.col(_("Name")), name.c_str(),
			s->previewAppView.col(_("Application")), app.c_str(),
			s->previewAppView.col(_("Extensions")), ext.c_str(),
			-1);
	}
}

void PreviewPage::onPreviewRemove_gui(GtkWidget *widget, gpointer data)
{
	PreviewPage *s = (PreviewPage *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());


	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string name = s->previewAppView.getString(&iter, _("Name"));

		if (WulforSettingsManager::getInstance()->removePreviewApp(name))
			gtk_list_store_remove(s->previewAppToStore, &iter);
	}
}

void PreviewPage::onPreviewKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PreviewPage *s = (PreviewPage *)data;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->entry_name), s->previewAppView.getString(&iter,_("Name")).c_str() );
			gtk_entry_set_text(GTK_ENTRY(s->entry_app), s->previewAppView.getString(&iter, _("Application")).c_str());
			gtk_entry_set_text(GTK_ENTRY(s->entry_type), s->previewAppView.getString(&iter, _("Extensions")).c_str());
		}
	}
}

void PreviewPage::onPreviewButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PreviewPage *s = (PreviewPage *)data;

	if (event->button == 3 || event->button == 1)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->entry_name), s->previewAppView.getString(&iter, _("Name")).c_str());
			gtk_entry_set_text(GTK_ENTRY(s->entry_app), s->previewAppView.getString(&iter, _("Application")).c_str());
			gtk_entry_set_text(GTK_ENTRY(s->entry_type), s->previewAppView.getString(&iter, _("Extensions")).c_str());
		}
	}
}

void PreviewPage::onPreviewApply_gui(GtkWidget *widget, gpointer data)
{
	PreviewPage *s = (PreviewPage *)data;

	string name = gtk_entry_get_text(GTK_ENTRY(s->entry_name));
	string app = gtk_entry_get_text(GTK_ENTRY(s->entry_app));
	string ext = gtk_entry_get_text(GTK_ENTRY(s->entry_type));

	if (name.empty() || app.empty() || ext.empty())
	{
		s->showErrorDialog(_("Must not be empty..."));
		return;
	}

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->previewAppView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string oldName = s->previewAppView.getString(&iter,_("Name"));

		if (WulforSettingsManager::getInstance()->applyPreviewApp(oldName, name, app, ext))
		{
			gtk_list_store_set(s->previewAppToStore, &iter,
				s->previewAppView.col(_("Name")), name.c_str(),
				s->previewAppView.col(_("Application")), app.c_str(),
				s->previewAppView.col(_("Extensions")), ext.c_str(),
				-1);
		}
	}
}
