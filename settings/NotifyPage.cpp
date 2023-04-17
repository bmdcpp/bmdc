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

#ifdef HAVE_NOTIFY
#include <dcpp/format.h>
#include "NotifyPage.hh"
#include "definitons.hh"
#include <dcpp/Text.h>
#include <linux/GuiUtil.hh>
#include <linux/notify.hh>
#include "seUtil.hh"

using namespace std;
using namespace dcpp;

#define ICON_SIZE 32
#define ICON_SIZE_NORMAL 22

const char* NotifyPage::name_page = "→ Notify";

void NotifyPage::show(GtkWidget *parent, GtkWidget* old){
	
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	GtkWidget *scroll = gtk_scrolled_window_new(NULL,NULL);
	
	notifyView = TreeView();
	notifyView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
	notifyView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	notifyView.insertColumn(_("Notify"), G_TYPE_STRING, TreeView::PIXBUF_STRING, -1, "icon");
	notifyView.insertColumn(_("Title"), G_TYPE_STRING, TreeView::STRING, -1);
	notifyView.insertColumn(_("Icon"), G_TYPE_STRING, TreeView::STRING, -1);
	notifyView.insertHiddenColumn("keyUse", G_TYPE_STRING);
	notifyView.insertHiddenColumn("keyTitle", G_TYPE_STRING);
	notifyView.insertHiddenColumn("keyIcon", G_TYPE_STRING);
	notifyView.insertHiddenColumn("icon", GDK_TYPE_PIXBUF);
	notifyView.insertHiddenColumn("Urgency", G_TYPE_INT);
	notifyView.finalize();

	notifyStore = gtk_list_store_newv(notifyView.getColCount(), notifyView.getGTypes());
	gtk_tree_view_set_model(notifyView.get(), GTK_TREE_MODEL(notifyStore));
	g_object_unref(notifyStore);

	gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(notifyView.get()));
	gtk_box_pack_start(GTK_BOX(box),scroll,TRUE,TRUE,0);

	SEUtil::reAddItemCo(parent,old,box);

	g_signal_connect(notifyView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(SEUtil::onOptionsViewToggled_gui), (gpointer)notifyStore);

	addOption_gui(notifyStore, wsm, _("Download finished"),
			"notify-download-finished-use", "notify-download-finished-title",
			"notify-download-finished-icon", NOTIFY_URGENCY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Download finished file list"),
			"notify-download-finished-ul-use", "notify-download-finished-ul-title",
			"notify-download-finished-ul-icon", NOTIFY_URGENCY_LOW);

		addOption_gui(notifyStore, wsm, _("Private message"),
			"notify-private-message-use", "notify-private-message-title",
			"notify-private-message-icon", NOTIFY_URGENCY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Hub connected"),
			"notify-hub-connect-use", "notify-hub-connect-title",
			"notify-hub-connect-icon", NOTIFY_URGENCY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Hub disconnected"),
			"notify-hub-disconnect-use", "notify-hub-disconnect-title",
			"notify-hub-disconnect-icon", NOTIFY_URGENCY_CRITICAL);

		addOption_gui(notifyStore, wsm, _("Favorite user joined"),
			"notify-fuser-join", "notify-fuser-join-title",
			"notify-fuser-join-icon", NOTIFY_URGENCY_NORMAL);

		addOption_gui(notifyStore, wsm, _("Favorite user quit"),
			"notify-fuser-quit", "notify-fuser-quit-title",
			"notify-fuser-quit-icon", NOTIFY_URGENCY_NORMAL);
		addOption_gui(notifyStore, wsm, _("Highlighting string"),
			"notify-high-use", "notify-high-title",
			"notify-high-icon", NOTIFY_URGENCY_LOW);

		addOption_gui(notifyStore, wsm, _("Chat Hub message"),
			"notify-hub-chat-use", "notify-hub-chat-title",
			"notify-hub-chat-icon", NOTIFY_URGENCY_NORMAL);

		GtkWidget *note = gtk_notebook_new();

		GtkWidget* grid = gtk_grid_new();
		bTest = gtk_button_new_with_label("Test");
		bIcon = gtk_button_new_with_label("Browse for Icon");
		bOK = gtk_button_new_with_label("OK");
		bNone = gtk_button_new_with_label("None");
		bDef = gtk_button_new_with_label("Default");
		eTitle = gen;

		gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Title"),0,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),eTitle,1,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),bTest,0,1,1,1);
		gtk_grid_attach(GTK_GRID(grid),bIcon,1,1,1,1);
		gtk_grid_attach(GTK_GRID(grid),bOK,2,1,1,1);
		gtk_grid_attach(GTK_GRID(grid),bNone,3,1,1,1);
		gtk_grid_attach(GTK_GRID(grid),bDef,4,1,1,1);
		gtk_notebook_append_page(GTK_NOTEBOOK(note), grid, gtk_label_new(""));

		sPMLenght = gtk_spin_button_new_with_range(0,600,1);
		tAppActive = gtk_check_button_new_with_label("Only if Application Hiden");
		
		g_signal_connect(bTest, "clicked", G_CALLBACK(onNotifyTestButton_gui), (gpointer)this);
		g_signal_connect(bIcon, "clicked", G_CALLBACK(onNotifyIconFileBrowseClicked_gui), (gpointer)this);
		g_signal_connect(bOK, "clicked", G_CALLBACK(onNotifyOKClicked_gui), (gpointer)this);
		g_signal_connect(bNone, "clicked", G_CALLBACK(onNotifyIconNoneButton_gui), (gpointer)this);
		g_signal_connect(bDef, "clicked", G_CALLBACK(onNotifyDefaultButton_gui), (gpointer)this);
		g_signal_connect(GTK_WIDGET(notifyView.get()), "key-release-event", G_CALLBACK(onNotifyKeyReleased_gui), (gpointer)this);
		g_signal_connect(notifyView.get(), "button-release-event", G_CALLBACK(onNotifyButtonReleased_gui), (gpointer)this);
		gtk_box_pack_start(GTK_BOX(box),note,TRUE,TRUE,0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(sPMLenght), (gdouble)WGETI("notify-pm-length"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tAppActive), WGETI("notify-only-not-active"));

		comboSize = gtk_combo_box_text_new();
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"16x16");
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"22x22");
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"24x24");
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"32x32");
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"36x36");
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"48x48");
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboSize),"64x64");		
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboSize), WGETI("notify-icon-size"));

}

/* Adds a notify options */

void NotifyPage::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm,
	const string &name, const string &key1, const string &key2, const string &key3, const int key4)
{
	GdkPixbuf *icon = NULL;
	string pathIcon = wsm->getString(key3);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(Text::fromUtf8(pathIcon).c_str(), NULL);

	if (pixbuf != NULL)
	{
		icon = WulforUtil::scalePixbuf(pixbuf, ICON_SIZE, ICON_SIZE);
		g_object_unref(pixbuf);
	}
	else
		pathIcon = "";

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, wsm->getInt(key1),               //use
		1, name.c_str(),                    //notify
		2, wsm->getString(key2).c_str(),    //title
		3, pathIcon.c_str(),                //icon path
		4, key1.c_str(),                    //key use
		5, key2.c_str(),                    //key title
		6, key3.c_str(),                    //key icon
		7, icon,                            //icon
		8, key4,                            //urgency
		-1);

	if (icon != NULL)
		g_object_unref(icon);
}


void NotifyPage::write()
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
		GtkTreeIter iter;
		GtkTreeModel *m = GTK_TREE_MODEL(notifyStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		while (valid)
		{
			wsm->set(notifyView.getString(&iter, "keyUse"), notifyView.getValue<int>(&iter, _("Use")));
			wsm->set(notifyView.getString(&iter, "keyTitle"), notifyView.getString(&iter, _("Title")));
			wsm->set(notifyView.getString(&iter, "keyIcon"), notifyView.getString(&iter, _("Icon")));
				valid = gtk_tree_model_iter_next(m, &iter);
		}

			WSET("notify-pm-length", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(sPMLenght)));
			WSET("notify-icon-size", gtk_combo_box_get_active(GTK_COMBO_BOX(comboSize)));
			WSET("notify-only-not-active", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tAppActive)));
}


void NotifyPage::onNotifyTestButton_gui(GtkWidget *widget, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string title = s->notifyView.getString(&iter, _("Title"));
		string icon = s->notifyView.getString(&iter, _("Icon"));
		NotifyUrgency urgency = (NotifyUrgency)s->notifyView.getValue<int>(&iter, "Urgency");
		Notify::get()->showNotify(title, "<span weight=\"bold\" size=\"larger\">" + string(_("*** T E S T ***")) + "</span>",
			"", icon, gtk_combo_box_get_active(GTK_COMBO_BOX(s->comboSize)), urgency);
	}
}

void NotifyPage::onNotifyIconFileBrowseClicked_gui(GtkWidget *widget, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	GtkWidget* fileDialog = gtk_file_chooser_dialog_new ("Select file",
                                      NULL,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      "_Cancel",
                                      GTK_RESPONSE_CANCEL,
                                      "_Open",
                                      GTK_RESPONSE_OK,
                                      NULL);
                                      
	gint response = gtk_dialog_run(GTK_DIALOG(fileDialog));
	gtk_widget_hide(fileDialog);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileDialog));

		if (path)
		{
			GtkTreeIter iter;
			GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

			if (gtk_tree_selection_get_selected(selection, NULL, &iter))
			{
				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);

				if (pixbuf != NULL)
				{
					string target = Text::toUtf8(path);
					GdkPixbuf *icon = WulforUtil::scalePixbuf(pixbuf, ICON_SIZE, ICON_SIZE);
					g_object_unref(pixbuf);

					gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col(_("Icon")), target.c_str(), -1);
					gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), icon, -1);
					g_object_unref(icon);
				}
			}
			g_free(path);
		}
	}
}
/*
void NotifyPage::onNotifyKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->eTitle), s->notifyView.getString(&iter, _("Title")).c_str());
		}
	}
}

void NotifyPage::onNotifyButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	if (event->button == 3 || event->button == 1)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			gtk_entry_set_text(GTK_ENTRY(s->eTitle), s->notifyView.getString(&iter, _("Title")).c_str());
		}
	}
}
*/
void NotifyPage::onNotifyOKClicked_gui(GtkWidget *widget, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	string title = gtk_entry_get_text(GTK_ENTRY(s->eTitle));

	if (title.empty())
	{
		//s->showErrorDialog(_("...must not be empty"));
		return;
	}

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string temp = s->notifyView.getString(&iter, _("Title"));

		if (temp != title)
		{
			gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col(_("Title")), title.c_str(), -1);
		}
	}
}

void NotifyPage::onNotifyIconNoneButton_gui(GtkWidget *widget, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))

		gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), NULL, s->notifyView.col(_("Icon")), "", -1);
	else ;
		//s->showErrorDialog(_("...must not be empty"));
}

void NotifyPage::onNotifyDefaultButton_gui(GtkWidget *widget, gpointer data)
{
	NotifyPage *s = (NotifyPage *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->notifyView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		GdkPixbuf *icon = NULL;
		WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
		string title = wsm->getString(s->notifyView.getString(&iter, "keyTitle"), TRUE);
		string path = wsm->getString(s->notifyView.getString(&iter, "keyIcon"), TRUE);

		if (!path.empty())
		{
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(Text::fromUtf8(path).c_str(), NULL);

			if (pixbuf != NULL)
			{
				icon = WulforUtil::scalePixbuf(pixbuf, ICON_SIZE, ICON_SIZE);
				g_object_unref(pixbuf);

				gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), icon, -1);
				g_object_unref(icon);
			}
		}
		else
			gtk_list_store_set(s->notifyStore, &iter, s->notifyView.col("icon"), icon, -1);

		gtk_list_store_set(s->notifyStore, &iter,
			s->notifyView.col(_("Icon")), path.c_str(), s->notifyView.col(_("Title")), _(title.c_str()), -1);
	}
	else ;
		//s->showErrorDialog(_("...must not be empty"));
}
		
#endif
