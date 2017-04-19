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

#include "TabsPage.hh"
#include <linux/settingsmanager.hh>
#include <linux/WulforUtil.hh>
#include "seUtil.hh"

using namespace std;
using namespace dcpp;

const char* TabsPage::name_page = "→ Tabs";

void TabsPage::show(GtkWidget *parent, GtkWidget *old)
{
	box =  gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	GtkWidget *grid = gtk_grid_new();
	GtkWidget *scroll = gtk_scrolled_window_new(NULL,NULL);
	tabsView = TreeView();
	tabsView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
	tabsView.insertColumn("Type", G_TYPE_STRING, TreeView::ICON_STRING,30,"Icon");
	tabsView.insertHiddenColumn("key", G_TYPE_STRING);
	tabsView.insertHiddenColumn("Icon", G_TYPE_STRING);
	tabsView.finalize();

	// Create the store
	tabStore = gtk_list_store_newv(tabsView.getColCount(), tabsView.getGTypes());
	gtk_tree_view_set_model(tabsView.get(), GTK_TREE_MODEL(tabStore));
	g_object_unref(tabStore);

	gtk_container_add(GTK_CONTAINER(scroll),GTK_WIDGET(tabsView.get()));
	gtk_grid_attach(GTK_GRID(grid),scroll,0,0,10,8);
	gtk_grid_set_column_homogeneous (GTK_GRID(grid),TRUE);
	gtk_widget_set_size_request(scroll,150,200);

	addItem_gui("Hub","hub-online");
	addItem_gui("Private Messages","pm-online");
	addItem_gui("Finished Uploads","finished-uploads");
	addItem_gui("Finished Downloads","finished-downloads");
	addItem_gui("Download Quene","queue");
	addItem_gui("Searchs","search");
	addItem_gui("Search SPY","search-spy");
	addItem_gui("Search ADL","search-adl");
	addItem_gui("Share Browser","shareb");
	addItem_gui("Notepad","notepad");
	addItem_gui("System Log","system");
	addItem_gui("Public Hubs","public-hubs");
	addItem_gui("Favorite Users","favorite-users");
	addItem_gui("Favorite Hubs","favorite-hubs");
	
	gtk_box_pack_start(GTK_BOX(box),grid,TRUE,TRUE,0);		
	tabsSelection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tabsView.get()));
	gtk_tree_selection_set_mode (tabsSelection, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(tabsSelection),"changed",G_CALLBACK(onChangeTabSelections), (gpointer)this);

	frame = gtk_frame_new("Tab Settings");
	gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,0);
	GtkWidget *note = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(frame),note);

	GtkWidget *pagelabel = gtk_label_new("Normal");
	GtkWidget *grid2 = gtk_grid_new();
	GtkWidget *labelb = gtk_label_new("BackGround Color");
	GtkWidget *labelf = gtk_label_new("ForeGround Color");
	button_normal_back = gtk_button_new_with_label("Choose");
	button_normal_fore = gtk_button_new_with_label("Choose");
	gtk_grid_attach(GTK_GRID(grid2),labelb,0,0,1,1); 
	gtk_grid_attach (GTK_GRID(grid2),button_normal_back,0,1,1,1); 
	gtk_grid_attach (GTK_GRID(grid2),labelf,1,0,1,1);
	gtk_grid_attach (GTK_GRID(grid2),button_normal_fore,1,1,1,1);

	g_signal_connect(button_normal_back, "clicked", G_CALLBACK(onBackColorChooserTab), (gpointer)this);
	g_signal_connect(button_normal_fore, "clicked", G_CALLBACK(onForeColorChooserTab), (gpointer)this);

	gtk_notebook_append_page(GTK_NOTEBOOK(note), grid2, pagelabel);

	GtkWidget *pagelabel2 = gtk_label_new("Unread");
	GtkWidget *grid3 = gtk_grid_new();
	GtkWidget *labelb2 = gtk_label_new("BackGround Color");
	GtkWidget *labelf2 = gtk_label_new("ForeGround Color");
	buttonb2 = gtk_button_new_with_label("Choose");
	buttonf2 = gtk_button_new_with_label("Choose");
	gtk_grid_attach(GTK_GRID(grid3),labelb2,0,0,1,1); 
	gtk_grid_attach (GTK_GRID(grid3),buttonb2,0,1,1,1); 
	gtk_grid_attach (GTK_GRID(grid3),labelf2,1,0,1,1);
	gtk_grid_attach (GTK_GRID(grid3),buttonf2,1,1,1,1);

	g_signal_connect(buttonb2, "clicked", G_CALLBACK(onBackColorChooserTab_unread), (gpointer)this);
	g_signal_connect(buttonf2, "clicked", G_CALLBACK(onForeColorChooserTab_unread), (gpointer)this);

	gtk_notebook_append_page(GTK_NOTEBOOK(note), grid3, pagelabel2);

	GtkWidget *pagelabel3 = gtk_label_new("Bold Setting");
	GtkWidget *grid4 = gtk_grid_new();
	toggle = gtk_check_button_new_with_label("Bold");
	gtk_grid_attach (GTK_GRID(grid4),toggle,0,0,1,1);

	g_signal_connect(GTK_TOGGLE_BUTTON(toggle), "toggled", G_CALLBACK(onBoldToggle_gui), (gpointer)this);

	gtk_notebook_append_page(GTK_NOTEBOOK(note), grid4, pagelabel3);
	
	SEUtil::reAddItemCo(parent,old,box);
	
}

void TabsPage::addItem_gui(string name, string key)
{
	GtkTreeIter iter;
	gtk_list_store_append(tabStore,&iter);
	gtk_list_store_set(tabStore,&iter,
				tabsView.col("Type"),name.c_str(),
				tabsView.col("key"),key.c_str(),
				tabsView.col("Icon"),WGETS(string("icon-")+key).c_str(),
	-1);

}

void TabsPage::onChangeTabSelections(GtkTreeSelection *selection, gpointer data)
{
	TabsPage *s = (TabsPage*)data;
    GtkTreeIter iter;
	GtkTreeModel *model;
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
			string key = s->tabsView.getString(&iter,"key");
			g_object_set_data_full(G_OBJECT(s->button_normal_back), "name", g_strdup(key.c_str()), g_free);
			g_object_set_data_full(G_OBJECT(s->button_normal_fore), "name", g_strdup(key.c_str()), g_free);
			g_object_set_data_full(G_OBJECT(s->buttonf2), "name", g_strdup(key.c_str()), g_free);
			g_object_set_data_full(G_OBJECT(s->buttonb2), "name", g_strdup(key.c_str()), g_free);
			g_object_set_data_full(G_OBJECT(s->toggle), "name", g_strdup(key.c_str()), g_free);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(s->toggle),s->getActive(key));
	 }
}

void TabsPage::onBackColorChooserTab(GtkWidget *button, gpointer data) 
{	
	TabsPage *s = (TabsPage *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string bg_string = WGETS("colored-tabs-" +(string(key))+"-color-bg");
	GdkRGBA bg;
	gdk_rgba_parse(&bg,bg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose BackGroun of "+key).c_str(),
                                                        NULL); 
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&bg);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA bg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&bg_s);
		string color = WulforUtil::colorToString(&bg_s);
		WSET("colored-tabs-" +(string(key))+"-color-bg",color);
	 
	}
	gtk_widget_destroy(dialog);
	
}

void TabsPage::onForeColorChooserTab(GtkWidget *button, gpointer data) 
{	
	TabsPage *s = (TabsPage *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string fg_string = WGETS("colored-tabs-" +(string(key))+"-color-fg");
	GdkRGBA fg;
	gdk_rgba_parse(&fg,fg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose ForeGroun of "+key).c_str(),
                                                        NULL); 
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&fg);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA fg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&fg_s);
		string color = WulforUtil::colorToString(&fg_s);
		WSET("colored-tabs-" +(string(key))+"-color-fg",color);
	 
	}
	gtk_widget_destroy(dialog);
	
}


void TabsPage::onBackColorChooserTab_unread(GtkWidget *button, gpointer data) 
{	
	TabsPage *s = (TabsPage *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string bg_string = WGETS("colored-tabs-" +(string(key))+"-color-bg-unread");
	GdkRGBA bg;
	gdk_rgba_parse(&bg,bg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose BackGroun of "+key).c_str(),
                                                        NULL); 
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&bg);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA bg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&bg_s);
		string color = WulforUtil::colorToString(&bg_s);
		WSET("colored-tabs-" +(string(key))+"-color-bg-unread",color);
	 
	}
	gtk_widget_destroy(dialog);
	
}

void TabsPage::onForeColorChooserTab_unread(GtkWidget *button, gpointer data) 
{	
	TabsPage *s = (TabsPage *)data;
	string key = (gchar *)g_object_get_data(G_OBJECT(button), "name");
	string fg_string = WGETS("colored-tabs-" +(string(key))+"-color-fg-unread");
	GdkRGBA fg;
	gdk_rgba_parse(&fg,fg_string.c_str());
	GtkWidget *dialog =   gtk_color_chooser_dialog_new (("Chose ForeGroun of "+key).c_str(),
                                                        NULL); 
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog),&fg);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response ==  GTK_RESPONSE_OK)
	{
		GdkRGBA fg_s;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&fg_s);
		string color = WulforUtil::colorToString(&fg_s);
		WSET("colored-tabs-" +(string(key))+"-color-fg-unread",color);
	 
	}
	gtk_widget_destroy(dialog);
	
}

void TabsPage::onBoldToggle_gui(GtkWidget *toggle, gpointer data)
{
	SettingsManager *sm = SettingsManager::getInstance();
	TabsPage *s = (TabsPage *)data;//
	string key = (gchar *)g_object_get_data(G_OBJECT(toggle), "name");
	bool show = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(toggle));
	if(key == "hub-online")
			sm->set(SettingsManager::BOLD_HUB,show);
	else if(key == "pm-online")
			sm->set(SettingsManager::BOLD_PM,show);
	else if(key == "finished-uploads")
			sm->set(SettingsManager::BOLD_FINISHED_UPLOADS, show );
	else if(key == "finished-downloads")
			sm->set(SettingsManager::BOLD_FINISHED_DOWNLOADS, show );
	else if( key == "queue")
			sm->set(SettingsManager::BOLD_QUEUE, show );
	else if(key == "search")
			sm->set(SettingsManager::BOLD_SEARCH, show );
	else if(key == "search-spy")
			sm->set(SettingsManager::BOLD_SEARCH_SPY , show );
	else if(key == "search-adl")
;//
	else if(key == "shareb")
		sm->set(SettingsManager::BOLD_FL , show );
	else if(key == "notepad")
;//
	else if(key == "system")
		sm->set(SettingsManager::BOLD_SYSTEM_LOG , show );
	else if(key == "public-hubs")
;//
	else if(key == "favorite-users")
;//
	else if(key == "favorite-hubs")
;//
	else ;
}

bool TabsPage::getActive(std::string key)
{
	bool ret = FALSE;
	if(key == "hub-online")
		ret = SETTING(BOLD_HUB);
	else if(key == "pm-online")
			ret = SETTING(BOLD_PM);
	else if(key == "finished-uploads")
		ret = SETTING(BOLD_FINISHED_UPLOADS);
	else if(key == "finished-downloads")
		ret = SETTING(BOLD_FINISHED_DOWNLOADS);
	else if( key == "queue")
		ret = SETTING(BOLD_QUEUE);
	else if(key == "search")
		ret = SETTING(BOLD_SEARCH);
	else if(key == "search-spy")
		ret = SETTING(BOLD_SEARCH_SPY);
	else if(key == "search-adl")
;//
	else if(key == "shareb")
		ret = SETTING(BOLD_FL);
	else if(key == "notepad")
;//
	else if(key == "system")
		ret = SETTING(BOLD_SYSTEM_LOG);
	else if(key == "public-hubs")
;//
	else if(key == "favorite-users")
;//
	else if(key == "favorite-hubs")
;//
	else ;
	return ret;
}
