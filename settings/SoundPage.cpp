/**/
#include "SoundPage.hh"
#include <linux/treeview.hh>
#include <dcpp/format.h>
#include <linux/sound.hh>
#include "seUtil.hh"

using namespace std;

const char* SoundPage::name_page = "→ Sound";

void SoundPage::show(GtkWidget *parent, GtkWidget* old)
{
		box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		GtkWidget *scroll = gtk_scrolled_window_new(NULL,NULL);
		WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
		soundView = TreeView();
		soundView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
		soundView.insertColumn(_("Use"), G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		soundView.insertColumn(_("Sounds"), G_TYPE_STRING, TreeView::STRING, -1);
		soundView.insertColumn(_("File"), G_TYPE_STRING, TreeView::STRING, -1);
		soundView.insertHiddenColumn("keyUse", G_TYPE_STRING);
		soundView.insertHiddenColumn("keyFile", G_TYPE_STRING);
		soundView.finalize();

		soundStore = gtk_list_store_newv(soundView.getColCount(), soundView.getGTypes());
		gtk_tree_view_set_model(soundView.get(), GTK_TREE_MODEL(soundStore));
		g_object_unref(soundStore);
		gtk_container_add(GTK_CONTAINER(scroll),GTK_WIDGET(soundView.get()));

		gtk_box_pack_start(GTK_BOX(box),scroll,TRUE,TRUE,0);
		g_signal_connect(soundView.getCellRenderOf(_("Use")), "toggled", G_CALLBACK(SEUtil::onOptionsViewToggled_gui), (gpointer)soundStore);

		addOption_gui(soundStore, wsm, _("Download begins"), "sound-download-begins-use", "sound-download-begins");
		addOption_gui(soundStore, wsm, _("Download finished"), "sound-download-finished-use", "sound-download-finished");
		addOption_gui(soundStore, wsm, _("Download finished file list"), "sound-download-finished-ul-use", "sound-download-finished-ul");
		addOption_gui(soundStore, wsm, _("Upload finished"), "sound-upload-finished-use", "sound-upload-finished");
		addOption_gui(soundStore, wsm, _("Private message"), "sound-private-message-use", "sound-private-message");
		addOption_gui(soundStore, wsm, _("Hub connected"), "sound-hub-connect-use", "sound-hub-connect");
		addOption_gui(soundStore, wsm, _("Hub disconnected"), "sound-hub-disconnect-use", "sound-hub-disconnect");
		addOption_gui(soundStore, wsm, _("Favorite user joined"), "sound-fuser-join-use", "sound-fuser-join");
		addOption_gui(soundStore, wsm, _("Favorite user quit"), "sound-fuser-quit-use", "sound-fuser-quit");

		button_play = gtk_button_new_with_label("Play");
		button_browse = gtk_button_new_with_label("Browse...");
		grid = gtk_grid_new();
		gtk_grid_attach(GTK_GRID(grid), button_play,0,0,1,1);
		gtk_grid_attach(GTK_GRID(grid), button_browse,1,0,1,1);

		gtk_box_pack_start(GTK_BOX(box),grid,TRUE,TRUE,0);
		
		g_signal_connect(button_play, "clicked", G_CALLBACK(onSoundPlayButton_gui), (gpointer)this);
		g_signal_connect(button_browse, "clicked", G_CALLBACK(onSoundFileBrowseClicked_gui), (gpointer)this);
		gtk_widget_set_sensitive(button_play, TRUE);
		gtk_widget_set_sensitive(button_browse, TRUE);

		SEUtil::reAddItemCo(parent,old,box);
}


/* Adds a sounds options */

void SoundPage::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const string &name, const string &key1, const string &key2)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, wsm->getInt(key1),               //use
		1, name.c_str(),                    //sounds
		2, wsm->getString(key2).c_str(),    //file
		3, key1.c_str(),                    //key use
		4, key2.c_str(),                    //key file
		-1);
}

void SoundPage::onSoundFileBrowseClicked_gui(GtkWidget *widget, gpointer data)
{
	SoundPage *s = (SoundPage *)data;

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
			GtkTreeSelection *selection = gtk_tree_view_get_selection(s->soundView.get());

			if (gtk_tree_selection_get_selected(selection, NULL, &iter))
			{
				string target = path;
				gtk_list_store_set(s->soundStore, &iter, s->soundView.col(_("File")), target.c_str(), -1);
			}
			g_free(path);
		}
	}
}

void SoundPage::onSoundPlayButton_gui(GtkWidget *widget, gpointer data)
{
	SoundPage *s = (SoundPage *)data;

	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->soundView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string target = s->soundView.getString(&iter, _("File"));
		Sound::get()->playSound(target);
	}
}
