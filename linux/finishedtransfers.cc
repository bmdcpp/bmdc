/*
 * Copyright © 2004-2011 Jens Oknelid, paskharen@gmail.com
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

#include "finishedtransfers.hh"
#include "previewmenu.hh"
#include <dcpp/Text.h>
#include <dcpp/ClientManager.h>
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

FinishedTransfers* FinishedTransfers::createFinishedDownloads()
{
	return new FinishedTransfers(Entry::FINISHED_DOWNLOADS, _("Finished Downloads"), false);
}

FinishedTransfers* FinishedTransfers::createFinishedUploads()
{
	return new FinishedTransfers(Entry::FINISHED_UPLOADS, _("Finished Uploads"), true);
}

FinishedTransfers::FinishedTransfers(const EntryType type, const string &title, bool isUpload):
	BookEntry(type, title, "finishedtransfers.glade"),
	isUpload(isUpload),
	totalFiles(0),
	totalBytes(0),
	totalTime(0)
{
	// Initialize transfer treeview
	fileView.setView(GTK_TREE_VIEW(getWidget("fileView")), true, "finished");
	fileView.insertColumn("Time", G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Users", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Size", G_TYPE_INT64, TreeView::SIZE, 100);
	fileView.insertColumn("Average Speed", G_TYPE_INT64, TreeView::SPEED, 125);
	fileView.insertColumn("CRC Checked", G_TYPE_STRING, TreeView::STRING, 115);
	fileView.insertHiddenColumn("Target", G_TYPE_STRING);
	fileView.insertHiddenColumn("Elapsed Time", G_TYPE_INT64);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col("Time")), TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("Time"), GTK_SORT_ASCENDING);
	gtk_tree_view_set_fixed_height_mode(fileView.get(), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	//// Initialize the preview menu
//	appsPreviewMenu = new PreviewMenu(getWidget("appsPreviewMenu"));

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("openItem"), "activate", G_CALLBACK(onOpen_gui), (gpointer)this);
	g_signal_connect(getWidget("openFolderItem"), "activate", G_CALLBACK(onOpenFolder_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItems_gui), (gpointer)this);
	g_signal_connect(getWidget("removeAllItem"), "activate", G_CALLBACK(onRemoveAll_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

}

FinishedTransfers::~FinishedTransfers()
{
	FinishedManager::getInstance()->removeListener(this);
//	delete appsPreviewMenu;
}

void FinishedTransfers::show()
{
	initializeList_client();
	//Func0<FinishedTransfers> *func = new Func0<FinishedTransfers>(this, &FinishedTransfers::initializeList_client);
	//WulforManager::get()->dispatchClientFunc(func);
	FinishedManager::getInstance()->addListener(this);
}

bool FinishedTransfers::findFile_gui(GtkTreeIter* iter, const string& item)
{
	bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fileStore), iter);

	while (valid)
	{
		if (fileView.getString(iter, "Target") == item)
			return TRUE;

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fileStore), iter);
	}

	return FALSE;
}

void FinishedTransfers::addFile_gui(StringMap params, bool update)
{
	GtkTreeIter iter;
	int64_t size = Util::toInt64(params["Size"]);
	int64_t speed = Util::toInt64(params["Average Speed"]);
	int64_t time = Util::toInt64(params["Elapsed Time"]);

	if (!findFile_gui(&iter, params["Target"]))
	{
		gtk_list_store_append(fileStore, &iter);
		totalFiles++;
	}
	gtk_list_store_set(fileStore, &iter,
		fileView.col("Time"), params["Time"].c_str(),
		fileView.col("Filename"), params["Filename"].c_str(),
		fileView.col("Path"), params["Path"].c_str(),
		fileView.col("Users"), params["Users"].c_str(),
		fileView.col("Size"), size,
		fileView.col("Average Speed"), speed,
		fileView.col("CRC Checked"), params["CRC Checked"].c_str(),
		fileView.col("Target"), params["Target"].c_str(),
		fileView.col("Elapsed Time"), time,
		-1);

	if (update)
	{
		updateStatus_gui();

		if ((!isUpload && BOOLSETTING(BOLD_FINISHED_DOWNLOADS)) ||
		     (isUpload && BOOLSETTING(BOLD_FINISHED_UPLOADS)))
		{
			setBold_gui();
		}
	}
}

void FinishedTransfers::updateStatus_gui()
{
	string status = Util::toString(totalFiles) + _(" files");

	string size = Util::formatBytes(totalBytes);
	string speed = Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s";

	gtk_statusbar_push(GTK_STATUSBAR(getWidget("totalItems")), 0, status.c_str());
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("totalSize")), 0, size.c_str());
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("averageSpeed")), 0, speed.c_str());
}

gboolean FinishedTransfers::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(ft->fileView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(ft->fileSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}


	return FALSE;
}

gboolean FinishedTransfers::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	int count = gtk_tree_selection_count_selected_rows(ft->fileSelection);

	if (event->button == 3 && count > 0)
	{
	//	ft->appsPreviewMenu->cleanMenu_gui();
		gtk_menu_popup(GTK_MENU(ft->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		gtk_widget_show_all(ft->getWidget("menu"));
	}

	return FALSE;
}

gboolean FinishedTransfers::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	int count = gtk_tree_selection_count_selected_rows(ft->fileSelection);

	if (count > 0)
	{
		if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			onOpen_gui(NULL, data);
		}
		else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			onRemoveItems_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(ft->getWidget("menu")), NULL, NULL, NULL, NULL, 1, event->time);
			gtk_widget_show_all(ft->getWidget("menu"));
		}
	}

	return FALSE;
}

void FinishedTransfers::onOpen_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	int count = gtk_tree_selection_count_selected_rows(ft->fileSelection);

	if (count <= 0)
		return;

	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);
	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
		{
			string target = ft->fileView.getString(&iter, "Target");
			if (!target.empty())
				WulforUtil::openURI(target);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

}

void FinishedTransfers::onOpenFolder_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	int count = gtk_tree_selection_count_selected_rows(ft->fileSelection);

	if (count <= 0)
		return;

	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
		{
			string target = ft->fileView.getString(&iter, "Path");
			if (!target.empty())
				WulforUtil::openURI(target);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void FinishedTransfers::onRemoveItems_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	string target;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);

	typedef Func1<FinishedTransfers, string> F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
		{
			target = ft->fileView.getString(&iter, "Target");
			func = new F1(ft, &FinishedTransfers::removeFile_client, target);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	ft->updateStatus_gui();		// Why? model won't change until after the _client call.
}

void FinishedTransfers::removeFile_gui(string target)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (target == fileView.getString(&iter, "Target"))
		{
			totalBytes -= fileView.getValue<gint64>(&iter, "Size");
			totalTime -= fileView.getValue<gint64>(&iter, "Elapsed Time");
			gtk_list_store_remove(fileStore, &iter);
			totalFiles--;
			updateStatus_gui();
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FinishedTransfers::onRemoveAll_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	gtk_list_store_clear(ft->fileStore);
	ft->totalBytes = 0;
	ft->totalTime = 0;
	ft->totalFiles = 0;
	ft->updateStatus_gui();

	typedef Func0<FinishedTransfers> F0;
	F0 *func = new F0(ft, &FinishedTransfers::removeAll_client);
	WulforManager::get()->dispatchClientFunc(func);
}

void FinishedTransfers::initializeList_client()
{
	StringMap params;
	typedef Func2<FinishedTransfers, StringMap, bool> F2;
	//F2 *func;
	FinishedManager::getInstance()->lockLists();
	const FinishedManager::MapByFile &list = FinishedManager::getInstance()->getMapByFile(isUpload);

	for (FinishedManager::MapByFile::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		params.clear();
		getFinishedParams_client(it->second, it->first, params);
		addFile_gui(params, FALSE);
		//func = new F2(this, &FinishedTransfers::addItem_gui, params, FALSE);
		//WulforManager::get()->dispatchGuiFunc(func);
	}

	FinishedManager::getInstance()->unLockLists();

	updateStatus_gui();
	//WulforManager::get()->dispatchGuiFunc(new Func0<FinishedTransfers>(this, &FinishedTransfers::updateStatus_gui));
}

void FinishedTransfers::getFinishedParams_client(const FinishedFileItemPtr& item, const string& file, StringMap &params)
{
	std::string nicks;
	params["Filename"] = Util::getFileName(file);
	params["Time"] = Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime());
	params["Path"] = Util::getFilePath(file);
//	for (UserList::const_iterator it = item->getUsers().begin(); it != item->getUsers().end(); ++it)
	for (HintedUserList::const_iterator it = item->getUsers().begin(); it != item->getUsers().end(); ++it)//NOTE: core 0.762
	{
		nicks += WulforUtil::getNicks(it->user->getCID(),it->hint) + ", ";///
	}
	params["Users"] = nicks.substr(0, nicks.length() - 2);
	params["Size"] = Util::toString(item->getTransferred());
	params["Average Speed"] = Util::toString(item->getAverageSpeed());
	params["CRC Checked"] = item->getCrc32Checked() ? _("Yes") : _("No");
	params["Target"] = file;
	params["Elapsed Time"] = Util::toString(item->getMilliSeconds());
}

void FinishedTransfers::removeFile_client(std::string target)
{

	if (!target.empty())
		FinishedManager::getInstance()->remove(isUpload, target);

}

void FinishedTransfers::removeAll_client()
{
	FinishedManager::getInstance()->removeAll(isUpload);
}

void FinishedTransfers::on(FinishedManagerListener::AddedFile, bool upload, const string& file, const FinishedFileItemPtr& item) throw()
{
	// Show partial uploads, but only full downloads
	if (isUpload == upload && (upload || item->isFull()))
	{
		StringMap params;
		getFinishedParams_client(item, file, params);

		typedef Func2<FinishedTransfers, StringMap, bool> F2;
		F2* func = new F2(this, &FinishedTransfers::addFile_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FinishedTransfers::on(FinishedManagerListener::UpdatedFile, bool upload, const string& file, const FinishedFileItemPtr& item) throw()
{
	// Show partial uploads, but only full downloads
	if (isUpload == upload && (upload || item->isFull()))
	{
		StringMap params;
		getFinishedParams_client(item, file, params);

		typedef Func2<FinishedTransfers, StringMap, bool> F2;
		F2 *func = new F2(this, &FinishedTransfers::addFile_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FinishedTransfers::on(FinishedManagerListener::RemovedFile, bool upload, const string& item) throw()
{
	if (isUpload == upload)
	{
		typedef Func1<FinishedTransfers, string> F1;
		F1 *func = new F1(this, &FinishedTransfers::removeFile_gui, item);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

/*this is a pop menu*/
void FinishedTransfers::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()),closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate",G_CALLBACK(onCloseItem),this);

}

void FinishedTransfers::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;

    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);

}

