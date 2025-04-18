/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/
 
#ifdef HAVE_LIBTAR

#include "exportimport.hh"
#include "../dcpp/ExportManager.h"
#include "../dcpp/SettingsManager.h"
#include "../dcpp/File.h"

using namespace std;
using namespace dcpp;

ExportDialog::ExportDialog(GtkWindow *parent):
DialogEntry(Entry::EXPORT_DIALOG,"export", parent),
exportStore(NULL), exportSelection(NULL)
{
	
	exportView.setView(GTK_TREE_VIEW(getWidget("treeviewexport")));
	exportView.insertColumn(_("Enabled"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	exportView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
	exportView.insertHiddenColumn("Name Hidden", G_TYPE_STRING);
	exportView.finalize();
	
	exportStore = gtk_list_store_newv(exportView.getColCount(), exportView.getGTypes());
	gtk_tree_view_set_model(exportView.get(),GTK_TREE_MODEL(exportStore));
	g_object_unref(exportStore);
	
	exportSelection = gtk_tree_view_get_selection(exportView.get());
	
	g_signal_connect(getWidget("boxExport"), "clicked", G_CALLBACK(onButtonExportedClicked), (gpointer)this);
	g_signal_connect(getWidget("boxPath"), "clicked", G_CALLBACK(onGetPathGui), (gpointer)this);
	g_signal_connect(exportView.getCellRenderOf(_("Enabled")), "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	
	gtk_list_store_clear(exportStore);
	
	GtkTreeIter iter;
	
	const auto& paths = File::findFiles(Util::getPath(Util::PATH_USER_CONFIG), "*");
		
	for(auto i = paths.cbegin(); i != paths.cend(); ++i) {
		if( (*i).empty() ) continue;
		
		if(Wildcard::match(Util::getFileName(*i), SETTING(WILDCARD_FOR_EXPORT_SET), ';')){
			continue;
		}
		gtk_list_store_append(exportStore,&iter);					
		gtk_list_store_set(exportStore,&iter,
						exportView.col(_("Enabled")), TRUE,
						exportView.col(_("Name")),Util::getFileName(*i).c_str(),
						exportView.col("Name Hidden"),(*i).c_str(),
						-1);
	
	}
}

ExportDialog::~ExportDialog(){
	
}

void ExportDialog::onButtonExportedClicked(GtkWidget*,gpointer data)
{
	ExportDialog *ed = (ExportDialog *)data;
	StringList pslist;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(ed->exportStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
	
	while(valid)
	{
		bool bSelected = ed->exportView.getValue<gboolean>(&iter, _("Enabled"));
		if(bSelected){
			string sAdd = ed->exportView.getString(&iter,"Name Hidden");
			pslist.push_back(sAdd);
		}
		valid = gtk_tree_model_iter_next(m, &iter);	
	}

	string sTo = gtk_editable_get_text(GTK_EDITABLE(ed->getWidget("entry")));
	ExportManager::getInstance()->export_(sTo, pslist);
}

void ExportDialog::onGetPathGui(GtkWidget*, gpointer data)
{
	ExportDialog *ed = (ExportDialog *)data;
	//gtk_file_chooser_set_action(GTK_FILE_CHOOSER(ed->getWidget("fileChooserDialog")), GTK_FILE_CHOOSER_ACTION_SAVE);
	//gint response = gtk_dialog_run(GTK_DIALOG(ed->getWidget("fileChooserDialog")));
	//gtk_widget_hide(ed->getWidget("fileChooserDialog"));

	//if (response == GTK_RESPONSE_OK)
	{
		g_autofree gchar* path = nullptr;
	//	gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ed->getWidget("fileChooserDialog")));

	//	if (path)
		{
			gtk_editable_set_text(GTK_EDITABLE(ed->getWidget("entry")), path);
			
		}
	}
}		

void ExportDialog::onToggledClicked_gui(GtkCellRendererToggle*, gchar *path, gpointer data)
{
	ExportDialog *ed = (ExportDialog *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ed->exportStore), &iter, path))
	{
		bool fixed = ed->exportView.getValue<gboolean>(&iter, _("Enabled"));
		fixed = !fixed;
		gtk_list_store_set(ed->exportStore, &iter, ed->exportView.col(_("Enabled")), fixed, -1);
	}
}	
#endif
	
			
