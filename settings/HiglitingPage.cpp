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

#include "HiglitingPage.hh"
#include "definitons.hh"
#include "HigDialog.hh"
#include "seUtil.hh"
#include "../dcpp/HighlightManager.h"
#include "../dcpp/ColorSettings.h"

using namespace std;
using namespace dcpp;

const char* HigPage::page_name = "~ Higliting";

void HigPage::show(GtkWidget* parent, GtkWidget* old)
{
	box = gtk_grid_new();
	GtkWidget* sw = sw_new;
	higTree = TreeView();
	higTree.setView(GTK_TREE_VIEW(gtk_tree_view_new()));

	higTree.insertColumn(_("String"), G_TYPE_STRING, TreeView::STRING,-1);
	higTree.insertHiddenColumn("HigPointer", G_TYPE_POINTER);
	higTree.finalize();

	higStore = gtk_list_store_newv(higTree.getColCount(), higTree.getGTypes());
	gtk_tree_view_set_model(higTree.get(), GTK_TREE_MODEL(higStore));


	gtk_box_append(GTK_BOX(box),GTK_WIDGET(higTree.get()));

	gtk_grid_attach(GTK_GRID(box),sw,0,0,1,1);

	GtkWidget* bbox = gtk_grid_new();
	GtkWidget* addButton = gtk_button_new_with_label(_("Add"));
	gtk_box_append(GTK_BOX(bbox),addButton);
	GtkWidget* editButton = gtk_button_new_with_label(_("Edit"));
	gtk_box_append(GTK_BOX(bbox),editButton);
	GtkWidget* remButton = gtk_button_new_with_label(_("Remove"));
	gtk_box_append(GTK_BOX(bbox),remButton);

	gtk_grid_attach(GTK_GRID(box),bbox,0,1,1,1);
/*@Add to parent*/
	SEUtil::reAddItemCo(parent,old,box);

	ColorList* cList = HighlightManager::getInstance()->getList();
	GtkTreeIter iter;
	for(auto i = cList->begin();i != cList->end(); ++i) {
			ColorSettings *cs= &(*i);
			gtk_list_store_append(higStore,&iter);
			gtk_list_store_set(higStore,&iter,
							higTree.col(_("String")), cs->getMatch().c_str(),
							higTree.col("HigPointer"),cs,
							-1);

	}
	higSelection =  gtk_tree_view_get_selection(higTree.get());
	g_signal_connect(addButton, "clicked", G_CALLBACK(onAddHighlighting_gui), (gpointer)this);
	g_signal_connect(editButton, "clicked", G_CALLBACK(onEditHighlighting_gui), (gpointer)this);
}

void HigPage::write()
{}

void HigPage::onAddHighlighting_gui(GtkWidget* widget, gpointer data)
{
	HigPage* hp = (HigPage*)data;
	ColorSettings *	cs = new dcpp::ColorSettings();
	HigDialog* hd = new HigDialog(cs,true);
	bool isOk = hd->run();
	if(isOk)
	{
		//save
		if(!cs->getMatch().empty())
		{
			ColorList* cl = HighlightManager::getInstance()->getList();
			cl->push_back(*cs);
			GtkTreeIter iter;
			gtk_list_store_append(hp->higStore,&iter);
			gtk_list_store_set(hp->higStore,&iter,
							hp->higTree.col(_("String")), cs->getMatch().c_str(),
							hp->higTree.col("HigPointer"),cs,
							-1);
		}
	}
}

void HigPage::onEditHighlighting_gui(GtkWidget* widget, gpointer data)
{
	HigPage* hp = (HigPage*)data;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(hp->higSelection, NULL, &iter))
		return;

	ColorSettings* cs = (ColorSettings *)hp->higTree.getValue<gpointer>(&iter, "HigPointer");
	HigDialog* hd = new HigDialog(cs,false);
	bool entryUpdated = hd->run();
	
	if(entryUpdated)
	{
		gtk_list_store_set(hp->higStore,&iter,
					hp->higTree.col(_("String")), cs->getMatch().c_str(),
					hp->higTree.col("HigPointer"),cs,
					-1);
	}

}
void HigPage::onRemoveHighlighting_gui(GtkWidget* widget, gpointer data)
{
	HigPage* hp = (HigPage*)data;

}
