/*
* Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "sharebrowser.hh"

#include "../dcpp/FavoriteManager.h"
#include "../dcpp/ShareManager.h"
#include "../dcpp/Text.h"
#include "../dcpp/ADLSearch.h"
#include "search.hh"
#include "settingsmanager.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"

using namespace std;
using namespace dcpp;

ShareBrowser::ShareBrowser(HintedUser user, const string &file, const string &initialDirectory, int64_t speed, bool full):
	BookEntry(Entry::SHARE_BROWSER, _("List: ") + WulforUtil::getNicks(user), "sharebrowser", user.user->getCID().toBase32()),
	user(user),	file(file),
	initialDirectory(initialDirectory),
	listing(user), shareSize(0), currentSize(0),
	shareItems(0), currentItems(0), skipHits(0),
	speed(speed), updateFileView(true), fullfl(full)
{
	// Use the nick from the file name in case the user is offline and core only returns CID
	nick = WulforUtil::getNicks(user);
	if (nick.find(user.user->getCID().toBase32(), 1) != string::npos)
	{
		string name = Util::getFileName(file);
		string::size_type loc = name.find('.');
		nick = name.substr(0, loc);
		setLabel_gui(_("List: ") + nick);
	}
    //@Nick can have in it .% and so on thus reason why we use CID'ed ver
	setName(CID(nick).toBase32());
	// Configure the dialogs
	File::ensureDirectory(SETTING(DOWNLOAD_DIRECTORY));
//	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(getWidget("dirChooserDialog")), Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)).c_str());
	// Set the pane position
	gtk_paned_set_position(GTK_PANED(getWidget("pane")), WGETI("sharebrowser-pane-position"));

	// Initialize the file TreeView
	fileView.setView(GTK_TREE_VIEW(getWidget("fileView")), true, "sharebrowser");
	fileView.insertColumn(_("Filename"), G_TYPE_STRING, TreeView::PIXBUF_STRING_TEXT_COLOR, 400, "Icon", "Color");
	fileView.insertColumn(_("Size"), G_TYPE_STRING, TreeView::STRING, 80);
	fileView.insertColumn(_("Type"), G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn(_("Exact Size"), G_TYPE_STRING, TreeView::STRING, 105);
	fileView.insertColumn(_("Date"), G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn(_("Bitrate"), G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn(_("Resolution"), G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn(_("Video"), G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn(_("Audio"), G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertHiddenColumn("DL File", G_TYPE_POINTER);
	fileView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	fileView.insertHiddenColumn("Size Order", G_TYPE_INT64);
	fileView.insertHiddenColumn("File Order", G_TYPE_STRING);
	fileView.insertHiddenColumn("Color", G_TYPE_STRING);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("File Order"), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col(_("Filename"))), TRUE);
	fileView.setSortColumn_gui(_("Filename"), "File Order");
	fileView.setSortColumn_gui(_("Size"), "Size Order");
	fileView.setSortColumn_gui(_("Exact Size"), "Size Order");

	// Initialize the directory treeview
	dirView.setView(GTK_TREE_VIEW(getWidget("dirView")));
	dirView.insertColumn("Dir", G_TYPE_STRING, TreeView::PIXBUF_STRING, -1, "Icon");
	dirView.insertHiddenColumn("DL Dir", G_TYPE_POINTER);
	dirView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	dirView.finalize();
	dirStore = gtk_tree_store_newv(dirView.getColCount(), dirView.getGTypes());
	gtk_tree_view_set_model(dirView.get(), GTK_TREE_MODEL(dirStore));
	g_object_unref(dirStore);
	dirSelection = gtk_tree_view_get_selection(dirView.get());
	gtk_tree_view_set_enable_tree_lines(dirView.get(), TRUE);

	// Initialize the user command menus
	//fileUserCommandMenu = new UserCommandMenu(getWidget("fileUserCommandMenu"), ::UserCommand::CONTEXT_FILELIST);
	//dirUserCommandMenu = new UserCommandMenu(getWidget("dirUserCommandMenu"), ::UserCommand::CONTEXT_FILELIST);
	//TabUserCommandMenu = new UserCommandMenu(BookEntry::createmenu(), ::UserCommand::CONTEXT_FILELIST);
	// Connect the signals to their callback functions.
//	g_signal_connect(fileView.get(), "key-release-event", G_CALLBACK(onFileKeyReleased_gui), (gpointer)this);
//	g_signal_connect(dirView.get(), "key-release-event", G_CALLBACK(onDirKeyReleased_gui), (gpointer)this);
//	g_signal_connect(getWidget("matchButton"), "clicked", G_CALLBACK(onMatchButtonClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("findButton"), "clicked", G_CALLBACK(onFindButtonClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("nextButton"), "clicked", G_CALLBACK(onNextButtonClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("dirDownloadItem"), "activate", G_CALLBACK(onDownloadDirClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("fileDownloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("searchForAlternatesItem"), "activate", G_CALLBACK(onSearchAlternatesClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyMagnetClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("copyPictureItem"), "activate", G_CALLBACK(onCopyPictureClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("downloadPartialFile"), "activate", G_CALLBACK(onClickedPartial), (gpointer)this);
//	g_signal_connect(getWidget("downloadPartialDir"), "activate", G_CALLBACK(onClickedPartial), (gpointer)this);

	GtkGesture *gesture;
	gesture = gtk_gesture_click_new ();
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 1);
	g_signal_connect (gesture, "pressed",
                    G_CALLBACK (on_widget_right_btn_pressed), (gpointer)this);
	g_signal_connect (gesture, "released",
                    G_CALLBACK (on_widget_right_btn_released), (gpointer)this);
	gtk_widget_add_controller (GTK_WIDGET(dirView.get()), GTK_EVENT_CONTROLLER (gesture));

}

ShareBrowser::~ShareBrowser()
{
	// Save the pane position
	int panePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	WSET("sharebrowser-pane-position", panePosition);
}

void ShareBrowser::show()
{
	updateStatus_gui();
	if(fullfl) {
		ThreadedDirectoryListing* tdl = new ThreadedDirectoryListing(this, file,dcpp::Util::emptyString,initialDirectory);
		WulforManager::get()->getMainWindow()->setMainStatus_gui(_("File list loading"));
		tdl->start();
	}else
	{ buildList_gui();}
}

void ShareBrowser::buildList_gui()
{
	// Load the xml file containing the share list.
	try
	{
		// Set name of root entry to user nick.
		listing.getRoot()->setName(nick);
		{
			buildDirs_gui(listing.getRoot(), NULL);

		}

	}
	catch (const Exception &e)
	{
		setStatus_gui("mainStatus", _("Unable to load file list: ") + e.getError());
	}
}

/*
 * Selects the directory in the tree view, and shows that directory's contents.
 */
void ShareBrowser::openDir_gui(const string &dir)
{
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirStore), &iter))
	{
		GtkTreePath *path;
		DirectoryListing::Directory *directory;

		if (findDir_gui(dir, &iter))
			path = gtk_tree_model_get_path(GTK_TREE_MODEL(dirStore), &iter);
		else
			path = gtk_tree_path_new_first();

		directory = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");

		gtk_tree_view_expand_to_path(dirView.get(), path);
		gtk_tree_view_scroll_to_cell(dirView.get(), path, gtk_tree_view_get_column(dirView.get(), 0), FALSE, 0.0, 0.0);
		gtk_tree_view_set_cursor(dirView.get(), path, NULL, FALSE);
		gtk_tree_path_free(path);

		updateFiles_gui(directory);
	}
}

bool ShareBrowser::findDir_gui(const string &dir, GtkTreeIter *parent)
{
	if (dir.empty())
		return true;

	string::size_type i = dir.find_first_of(PATH_SEPARATOR);
	const string &current = dir.substr(0, i);
	GtkTreeIter iter;
	bool valid = gtk_tree_model_iter_children(GTK_TREE_MODEL(dirStore), &iter, parent);

	while (valid)
	{
		if (dirView.getString(&iter, "Dir") == current)
		{
			*parent = iter;
			return findDir_gui(dir.substr(i + 1), parent);
		}

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(dirStore), &iter);
	}

	return FALSE;
}

void ShareBrowser::buildDirs_gui(DirectoryListing::Directory *dir, GtkTreeIter *iter)
{
	GtkTreeIter newIter;

	gtk_tree_store_append(dirStore, &newIter, iter);
	gtk_tree_store_set(dirStore, &newIter, dirView.col("Dir"), dir->getName().c_str(), -1);

	gtk_tree_store_set(dirStore, &newIter,
		dirView.col("DL Dir"), (gpointer)dir,
		dirView.col("Icon"), WulforUtil::loadIconShare("directory"),
		-1);

	for (auto file = dir->files.begin(); file != dir->files.end(); ++file)
	{
		shareItems++;
		shareSize += (*file)->getSize();
	}

	// Recursive call for all subdirs of current dir.
	for (auto it = dir->directories.begin(); it != dir->directories.end(); ++it)
		buildDirs_gui(*it, &newIter);
}

void ShareBrowser::updateFiles_gui(DirectoryListing::Directory *dir)
{
	std::set<dcpp::DirectoryListing::Directory*, dcpp::DirectoryListing::Directory::Less<dcpp::DirectoryListing::Directory> > *dirs = &(dir->directories);
	set<DirectoryListing::File::Ptr, DirectoryListing::Directory::Less<DirectoryListing::File> >::iterator it_file;
	DirectoryListing::Directory::FList *files = &(dir->files);

	GtkTreeIter iter;
	int64_t size = 0;
	gint sortColumn;
	GtkSortType sortType;

	currentSize = 0;
	currentItems = 0;

	gtk_list_store_clear(fileStore);

	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(fileStore), &sortColumn, &sortType);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, sortType);

	// Add directories to the store.
	for (auto it_dir = dirs->begin(); it_dir != dirs->end(); ++it_dir)
	{
        size = (*it_dir)->getSize();
		gtk_list_store_append(fileStore, &iter);
		gtk_list_store_set(fileStore, &iter,
			fileView.col(_("Filename")), Util::getFileName((*it_dir)->getName()).c_str(),
			fileView.col("File Order"), Util::getFileName("d"+(*it_dir)->getName()).c_str(),
			fileView.col("Icon"), WulforUtil::loadIconShare("directory"),
			fileView.col(_("Size")), Util::formatBytes(size).c_str(),
			fileView.col(_("Exact Size")), Util::formatExactSize(size).c_str(),
			fileView.col("Size Order"), size,
			fileView.col(_("Type")), _("Directory"),
			fileView.col("DL File"), (gpointer)(*it_dir),
			fileView.col("TTH"), "",
			-1);
        
		currentSize += size;
		currentItems++;
	}

	// Add files to the store.
	for (it_file = files->begin(); it_file != files->end(); ++it_file)
	{
		gtk_list_store_append(fileStore, &iter);

		// If ext is empty we cannot do substr on it.
		string ext = Util::getFileExt((*it_file)->getName());
		if (ext.length() > 0)
			ext = ext.substr(1);

		gtk_list_store_set(fileStore, &iter,
			fileView.col(_("Filename")), Util::getFileName((*it_file)->getName()).c_str(),
			fileView.col(_("Type")), ext.c_str(),
			fileView.col("File Order"), Util::getFileName("f"+(*it_file)->getName()).c_str(),
			-1);

		StringList targets;
		string shcolor;
		targets = QueueManager::getInstance()->getTargets((*it_file)->getTTH());
		if(targets.size() > 0)
			shcolor = WGETS("share-queue");
		else
			shcolor = dcpp::ShareManager::getInstance()->isTTHShared((*it_file)->getTTH()) ? WGETS("share-shared") : WGETS("share-default");
		
		GdkPixbuf *buf = WulforUtil::loadIconShare(dcpp::Util::getFileExt((*it_file)->getName()));
		
		size = (*it_file)->getSize();

		gtk_list_store_set(fileStore, &iter,
			fileView.col("Icon"), buf,
			fileView.col(_("Size")), Util::formatBytes(size).c_str(),
			fileView.col(_("Exact Size")), Util::formatExactSize(size).c_str(),
			fileView.col(_("Date")), (*it_file)->getTS() ? Util::formatTime("%Y-%m-%d %H:%M", (*it_file)->getTS()).c_str() : Util::emptyString.c_str(),
			fileView.col(_("Bitrate")),((*it_file)->m_media.m_bitrate) ? (Util::toString((*it_file)->m_media.m_bitrate) + " Kbps").c_str() : Util::emptyString.c_str(),
			fileView.col(_("Resolution")),((*it_file)->m_media.m_mediaX || (*it_file)->m_media.m_mediaY) ? (Util::toString((*it_file)->m_media.m_mediaX) + "x" + Util::toString((*it_file)->m_media.m_mediaY)).c_str() : Util::emptyString.c_str(),
			fileView.col(_("Video")), (*it_file)->m_media.m_video.c_str(),
			fileView.col(_("Audio")), (*it_file)->m_media.m_audio.c_str(),
			fileView.col("Size Order"), size,
			fileView.col("DL File"), (gpointer)(*it_file),
			fileView.col("TTH"), (*it_file)->getTTH().toBase32().c_str(),
			fileView.col("Color"), shcolor.c_str(),
			-1);

		currentSize += size;
		currentItems++;
	}

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), sortColumn, sortType);
	gtk_tree_view_scroll_to_point(fileView.get(), 0, 0);
	updateStatus_gui();
	updateFileView = true;
}

void ShareBrowser::updateStatus_gui()
{
	string items, files, size, total;
	files = _("Files: ") + Util::toString(shareItems);
	total = _("Total: ") + Util::formatBytes(shareSize);

	if (gtk_tree_selection_get_selected(dirSelection, NULL, NULL))
	{
		items = _("Items: ") + Util::toString(currentItems);
		size = _("Size: ") + Util::formatBytes(currentSize);
	}
	else
	{
		items = _("Items: 0");
		size = _("Size: 0 B");
	}

	setStatus_gui("itemsStatus", items);
	setStatus_gui("sizeStatus", size);
	setStatus_gui("filesStatus", files);
	setStatus_gui("totalStatus", total);
	setStatus_gui("speedStatus", _("Speed: ")+Util::formatBytes(speed));
}

void ShareBrowser::setStatus_gui(string statusBar, string msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, msg.c_str());
}

void ShareBrowser::fileViewSelected_gui()
{
	GtkTreeIter iter, parentIter;
	GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
	GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
	GtkTreePath *path = (GtkTreePath *)g_list_nth_data(list, 0);

	if (gtk_tree_model_get_iter(m, &iter, path))
	{
		gpointer ptr;
		string fileOrder;
		ptr = fileView.getValue<gpointer>(&iter, "DL File");
		fileOrder = fileView.getString(&iter, "File Order");

		if (fileOrder[0] == 'd' && gtk_tree_selection_get_selected(dirSelection, NULL, &parentIter))
		{
			gtk_tree_path_free(path);
			m = GTK_TREE_MODEL(dirStore);
			gboolean valid = gtk_tree_model_iter_children(m, &iter, &parentIter);

			while (valid && ptr != dirView.getValue<gpointer>(&iter, "DL Dir"))
				valid = gtk_tree_model_iter_next(m, &iter);

			path = gtk_tree_model_get_path(m, &iter);
			gtk_tree_view_expand_to_path(dirView.get(), path);
			gtk_tree_view_set_cursor(dirView.get(), path, NULL, FALSE);

			updateFiles_gui((DirectoryListing::Directory *)ptr);
		}
		else
			downloadSelectedFiles_gui(Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)));
	}

	gtk_tree_path_free(path);
	g_list_free(list);
}

void ShareBrowser::downloadSelectedFiles_gui(const string &target)
{
	gpointer ptr;
	string fileOrder;
	string filename;
	GtkTreeIter iter;
	GtkTreePath *path;
	DirectoryListing::File *file;
	DirectoryListing::Directory *dir;
	GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fileStore), &iter, path))
		{
			ptr = fileView.getValue<gpointer>(&iter, "DL File");
			fileOrder = fileView.getString(&iter, "File Order");

			if (fileOrder[0] == 'd')
			{
				dir = (DirectoryListing::Directory *)ptr;
				downloadDir_client(dir , target);
			}
			else
			{
				file = (DirectoryListing::File *)ptr;

				string filename = Util::getFileName(file->getName());

				downloadFile_client(file, target+filename);
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void ShareBrowser::downloadSelectedDirs_gui(const string &target)
{
	DirectoryListing::Directory *dir;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");

		downloadDir_client(dir, target);
	}
}

void ShareBrowser::popupFileMenu_gui()
{
	/*GtkWidget *menuItem;

	// Clean menus
	gtk_container_foreach(GTK_CONTAINER(getWidget("fileDownloadMenu")), (GtkCallback)gtk_widget_destroy, NULL);
	fileUserCommandMenu->cleanMenu_gui();

	// Build file download menu
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); ++i)
		{
			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
			g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteClicked_gui), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("fileDownloadMenu")), menuItem);
		}
		menuItem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("fileDownloadMenu")), menuItem);
	}

	menuItem = gtk_menu_item_new_with_label(_("Browse..."));
	g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("fileDownloadMenu")), menuItem);

	// Build user command menu
	StringList hubs = WulforUtil::getHubAddress(listing.getUser().user->getCID(), "");
	fileUserCommandMenu->addHub(hubs);
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
	string cid = listing.getUser().user->getCID().toBase32();

	for (GList *i = list; i; i = i->next)
	{
		GtkTreePath *path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fileStore), &iter, path))
		{
			fileUserCommandMenu->addFile(cid,
				fileView.getString(&iter, _("Filename")),
				fileView.getValue<int64_t>(&iter, "Size Order"),
				fileView.getString(&iter, "TTH"));
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
	fileUserCommandMenu->buildMenu_gui();

	gtk_menu_popup_at_pointer(GTK_MENU(getWidget("fileMenu")),NULL);
*/
}

void ShareBrowser::popupDirMenu_gui()
{
	/*GtkWidget *menuItem;

	// Clean menus
	gtk_container_foreach(GTK_CONTAINER(getWidget("dirDownloadMenu")), (GtkCallback)gtk_widget_destroy, NULL);
	dirUserCommandMenu->cleanMenu_gui();

	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); ++i)
		{
			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
			g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteDirClicked_gui), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("dirDownloadMenu")), menuItem);
		}
		menuItem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("dirDownloadMenu")), menuItem);
	}

	menuItem = gtk_menu_item_new_with_label(_("Browse..."));
	g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadDirToClicked_gui), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("dirDownloadMenu")), menuItem);

	// Add user commands.
	StringList hubs = WulforUtil::getHubAddress(listing.getUser().user->getCID(), "");
	dirUserCommandMenu->addHub(hubs);
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dirSelection, NULL);
	string cid = listing.getUser().user->getCID().toBase32();

	for (GList *i = list; i; i = i->next)
	{
		GtkTreePath *path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, path))
		{
			dirUserCommandMenu->addFile(cid, dirView.getString(&iter, "Dir"), 0, "");
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
	dirUserCommandMenu->buildMenu_gui();
	gtk_menu_popup_at_pointer(GTK_MENU(getWidget("dirMenu")),NULL);
*/
}

/*
 * Searches the directories iteratively for the requested pattern. Uses a pre-order
 * traversal method, with the exception that it searches the parent's dir name first.
 * Instead of keeping track of the last directory its search ended at, it counts
 * the number of matches and re-searches the listing, skipping matches until it
 * reaches the newest one. Slightly slower, but simpler.
 */
void ShareBrowser::find_gui()
{
	string name;
	bool findLeafNode = true;
	int cursorPos, hits = 0;
	DirectoryListing::Directory *dir = nullptr;

	set<DirectoryListing::File::Ptr, DirectoryListing::Directory::Less<DirectoryListing::File>>::const_iterator _file;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(dirStore);
	GtkTreePath *dirPath = gtk_tree_path_new_first();

	if (gtk_tree_path_get_depth(dirPath) == 0 || !gtk_tree_model_get_iter(m, &iter, dirPath))
	{
		gtk_tree_path_free(dirPath);
		return;
	}

	gint sortColumn;
	GtkSortType sortType;
	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(fileStore), &sortColumn, &sortType);
	if (sortColumn != fileView.col("File Order") || sortType != GTK_SORT_ASCENDING)
	{
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("File Order"), GTK_SORT_ASCENDING);
		gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col(_("Filename"))), TRUE);
	}

	while (TRUE)
	{
		// Drill down until we reach a leaf node (e.g. a dir with no child dirs).
		if (findLeafNode)
		{
			do
			{
				name = Text::toLower(dirView.getString(&iter, "Dir"));
				// We found a matching directory name.
				if (name.find(search, 0) != string::npos && hits++ == skipHits)
				{
					skipHits = hits;
					gtk_tree_view_expand_to_path(dirView.get(), dirPath);
					gtk_tree_view_set_cursor(dirView.get(), dirPath, NULL, FALSE);
					dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");
					updateFiles_gui(dir);
					gtk_widget_grab_focus(GTK_WIDGET(dirView.get()));
					updateFileView = false;
					gtk_tree_path_free(dirPath);
					setStatus_gui("mainStatus", _("Found a match"));
					return;
				}
				gtk_tree_path_down(dirPath);
			}
			while (gtk_tree_model_get_iter(m, &iter, dirPath));
		}

		// Come back up one directory. If we can't, then we've returned to the root and are done.
		if (!gtk_tree_path_up(dirPath) || gtk_tree_path_get_depth(dirPath) == 0 ||
			!gtk_tree_model_get_iter(m, &iter, dirPath))
		{
			setStatus_gui("mainStatus", _("No matches"));
			gtk_tree_path_free(dirPath);
			return;
		}

		// Search the files that are contained in this directory.
		dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");

		for (_file = dir->files.begin(), cursorPos = dir->directories.size(); _file != dir->files.end(); _file++, cursorPos++)
		{
			name = Text::toLower((*_file)->getName());
			
			// We found a matching file. Update the cursors and the fileView if necessary.
			if (name.find(search, 0) != string::npos && hits++ == skipHits)
			{
				if (updateFileView)
				{
					gtk_tree_view_expand_to_path(dirView.get(), dirPath);
					gtk_tree_view_set_cursor(dirView.get(), dirPath, NULL, FALSE);
					updateFiles_gui(dir);
					updateFileView = false;
				}

				skipHits = hits;
				// Keeping track of the current index allows us to quickly get the path to the file.
				GtkTreePath *path = gtk_tree_path_new_from_string(Util::toString(cursorPos).c_str());
				gtk_tree_view_set_cursor(fileView.get(), path, NULL, FALSE);
				gtk_widget_grab_focus(GTK_WIDGET(fileView.get()));
				gtk_tree_path_free(path);
				gtk_tree_path_free(dirPath);
				setStatus_gui("mainStatus", _("Found a match"));
				return;
			}
		}
		updateFileView = true;

		// Determine if we are to go to the next sibling or back to the parent dir.
		gtk_tree_path_next(dirPath);
		if (!gtk_tree_model_get_iter(m, &iter, dirPath))
			findLeafNode = false;
		else
			findLeafNode = true;
	}
}
/*

gboolean ShareBrowser::onFileButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

	if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
		sb->popupFileMenu_gui();
	else if (count == 1 && sb->oldType == GDK_2BUTTON_PRESS && event->button == 1)
		sb->fileViewSelected_gui();

	return FALSE;
}

gboolean ShareBrowser::onFileKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

	if (count > 0 && (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK)))
		sb->popupFileMenu_gui();
	else if (count == 1 && (event->keyval == GDK_KEY_Return))
		sb->fileViewSelected_gui();

	return FALSE;
}

gboolean ShareBrowser::onDirButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	gpointer ptr;

	if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
		return FALSE;

	if (event->button == 1 && sb->oldType == GDK_2BUTTON_PRESS)
	{
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
		if (gtk_tree_view_row_expanded(sb->dirView.get(), path))
			gtk_tree_view_collapse_row(sb->dirView.get(), path);
		else
			gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
		gtk_tree_path_free(path);
	}
	else if (event->button == 1 && event->type == GDK_BUTTON_RELEASE)
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
	}
	else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
		sb->popupDirMenu_gui();
	}

	return FALSE;
}

gboolean ShareBrowser::onDirKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	gpointer ptr;

	if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
		return FALSE;

	if (event->keyval == GDK_KEY_Return)
	{
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
		if (gtk_tree_view_row_expanded(sb->dirView.get(), path))
			gtk_tree_view_collapse_row(sb->dirView.get(), path);
		else
			gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
		gtk_tree_path_free(path);
	}
	else if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
	}
	else if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
		sb->popupDirMenu_gui();
	}

	return FALSE;
}
*//*
void ShareBrowser::onMatchButtonClicked_gui(GtkWidget*, gpointer data)
{
	((ShareBrowser*)data)->matchQueue_client();
}
/*
void ShareBrowser::onFindButtonClicked_gui(GtkWidget*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	gint response = gtk_dialog_run(GTK_DIALOG(sb->getWidget("findDialog")));
	if (response == GTK_RESPONSE_OK)
	{
		string text = gtk_entry_get_text(GTK_ENTRY(sb->getWidget("findEntry")));
		if (!text.empty())
		{
			sb->search = text;
			sb->skipHits = 0;
			sb->find_gui();
		}
		else
		{
			sb->setStatus_gui("mainStatus", _("No matches"));
		}
	}
}
/*
void ShareBrowser::onNextButtonClicked_gui(GtkWidget*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	if (!sb->search.empty())
		sb->find_gui();
	else
		sb->setStatus_gui("mainStatus", _("No search text entered"));
}
/*
void ShareBrowser::onDownloadClicked_gui(GtkMenuItem*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	sb->downloadSelectedFiles_gui(Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)));
}*/
/*
void ShareBrowser::onDownloadToClicked_gui(GtkMenuItem*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;

	gint response = gtk_dialog_run(GTK_DIALOG(sb->getWidget("dirChooserDialog")));

	if (response == GTK_RESPONSE_OK)
	{
		g_autofree gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(sb->getWidget("dirChooserDialog")));
		if (temp)
		{
			string path = Text::toUtf8(temp);
			if (path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;

			sb->downloadSelectedFiles_gui(path);
		}
	}
}*/
/*
void ShareBrowser::onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	string target = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
	sb->downloadSelectedFiles_gui(target);
}*/
/*
void ShareBrowser::onDownloadDirClicked_gui(GtkMenuItem*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	sb->downloadSelectedDirs_gui(Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)));
}*/
/*
void ShareBrowser::onDownloadDirToClicked_gui(GtkMenuItem*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;

	gint response = gtk_dialog_run(GTK_DIALOG(sb->getWidget("dirChooserDialog")));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(sb->getWidget("dirChooserDialog")));
		if (temp)
		{
			string path = Text::toUtf8(temp);
			g_free(temp);
			if (path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;

			sb->downloadSelectedDirs_gui(path);
		}
	}
}*/
/*
void ShareBrowser::onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	string target = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
	sb->downloadSelectedDirs_gui(target);
}
*//*
void ShareBrowser::onSearchAlternatesClicked_gui(GtkMenuItem*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	string fileOrder;
	SearchEntry *s;
	DirectoryListing::File *file;
	GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
		{
			fileOrder = sb->fileView.getString(&iter, "File Order");

			if (fileOrder[0] == 'f')
			{
				file = sb->fileView.getValue<gpointer, DirectoryListing::File *>(&iter, "DL File");
				s = WulforManager::get()->getMainWindow()->addSearch_gui();
				s->putValue_gui(file->getTTH().toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}*/
/*
void ShareBrowser::onCopyMagnetClicked_gui(GtkMenuItem*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	int64_t size;
	string magnets, magnet, filename, tth;
	GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
		{
			filename = sb->fileView.getString(&iter, _("Filename"));
			size = sb->fileView.getValue<int64_t>(&iter, "Size Order");
			tth = sb->fileView.getString(&iter, "TTH");
			magnet = WulforUtil::makeMagnet(filename, size, tth);

			if (!magnet.empty())
			{
				if (!magnets.empty())
					magnets += '\n';
				magnets += magnet;
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	if (!magnets.empty())
		gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), magnets.c_str(), magnets.length());
}
*/
/*void ShareBrowser::onCopyPictureClicked_gui(GtkMenuItem* , gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	int64_t size;
	string magnets, magnet, filename, tth;
	GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
		{
			filename = sb->fileView.getString(&iter, _("Filename"));
			size = sb->fileView.getValue<int64_t>(&iter, "Size Order");
			tth = sb->fileView.getString(&iter, "TTH");
			magnet = WulforUtil::makeMagnet(filename, size, tth);

			if (!magnet.empty())
			{
				magnet = "[img]" + magnet + "[/img]";
				if (!magnets.empty())
					magnets += '\n';
				magnets += magnet;
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	if (!magnets.empty())
		gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), magnets.c_str(), magnets.length());
}
*/
void ShareBrowser::downloadFile_client(DirectoryListing::File *file, string target)
{
	try
	{
		listing.download(file, target, FALSE, FALSE);
	}
	catch (const Exception& e)
	{
		typedef Func2<ShareBrowser, string, string> F2;
		F2 *func = new F2(this, &ShareBrowser::setStatus_gui, "mainStatus", e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ShareBrowser::downloadDir_client(DirectoryListing::Directory *dir, string target)
{
	try
	{
		listing.download(dir, target, FALSE);
	}
	catch (const Exception& e)
	{
		typedef Func2<ShareBrowser, string, string> F2;
		F2 *func = new F2(this, &ShareBrowser::setStatus_gui, "mainStatus", e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ShareBrowser::matchQueue_client()
{
	int matched = QueueManager::getInstance()->matchListing(listing);
	string message = _("Matched ") + Util::toString(matched) + _(" files");

	typedef Func2<ShareBrowser, string, string> F2;
	F2 *f = new F2(this, &ShareBrowser::setStatus_gui, "mainStatus", message);
	WulforManager::get()->dispatchGuiFunc(f);
}
//[BMDC++
//custom popup menu

GMenu *ShareBrowser::createmenu()
{
   /* TabUserCommandMenu->cleanMenu_gui();
    TabUserCommandMenu->addUser(user.user->getCID().toBase32());
    StringList hubs = WulforUtil::getHubAddress(listing.getUser().user->getCID(), "");
    TabUserCommandMenu->addHub(hubs);
    TabUserCommandMenu->buildMenu_gui();
    GtkWidget *menu = TabUserCommandMenu->getContainer();

    GtkWidget *copyHubUrl = gtk_menu_item_new_with_label(_("Copy CID"));
    GtkWidget *close = gtk_menu_item_new_with_label(_("Close"));

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),close);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),copyHubUrl);

    g_signal_connect_swapped(copyHubUrl, "activate", G_CALLBACK(onCopyCID), (gpointer)this);
    g_signal_connect_swapped(close, "activate", G_CALLBACK(onCloseItem), (gpointer)this);
    return menu;*/
    return NULL;
}    

void ShareBrowser::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void ShareBrowser::onCopyCID(gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
//    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), sb->user.user->getCID().toBase32().c_str(), sb->user.user->getCID().toBase32().length());
}

void ShareBrowser::loadXML(string txt) {

		typedef Func1<ShareBrowser,string> F1;
		F1 *func = new F1(this,&ShareBrowser::load,txt);
		WulforManager::get()->dispatchGuiFunc(func);
}

void ShareBrowser::load(string xml)
{
	// Set name of root entry to user nick.
	listing.getRoot()->setName(nick);

	GtkTreeIter iter;
	DirectoryListing::Directory *dirList;
	GtkTreePath *treepath;
	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		string path,path2;
		dirList = (DirectoryListing::Directory *)dirView.getValue<gpointer>(&iter,"DL Dir");
		path2 = dirList->getName();
		treepath = gtk_tree_path_copy(gtk_tree_model_get_path (GTK_TREE_MODEL(dirStore) ,gtk_tree_iter_copy(&iter)));

		path = QueueManager::getInstance()->getListPath(listing.getUser()) + ".xml";
		if(File::getSize(path) != -1) {
			// load the cached list.
			listing.updateXML(File(path, File::READ, File::OPEN).read());
		}
		auto base = listing.updateXML(xml);

		ADLSearchManager::getInstance()->matchListing(listing);
		gtk_tree_store_clear(dirStore);
		gtk_list_store_clear(fileStore);
		buildDirs_gui(listing.getRoot(),NULL);
		gtk_tree_view_expand_to_path(dirView.get(), treepath);
		gtk_tree_view_scroll_to_cell(dirView.get(),treepath,NULL,FALSE,0,0);
		gtk_tree_selection_select_path(dirSelection,treepath);
		updateFiles_gui(dirList);
   }
}
/*
void ShareBrowser::onClickedPartial(GtkWidget*, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	DirectoryListing::Directory *dirList = NULL;
	if (gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
	{
		dirList = (DirectoryListing::Directory *)sb->dirView.getValue<gpointer>(&iter,"DL Dir");
		sb->downloadChangedDir(dirList);
	}	
}
*/
void ShareBrowser::downloadChangedDir(DirectoryListing::Directory* d) {
	if(!d->getComplete()) {
		dcdebug("Directory %s incomplete, downloading...\n", d->getName().c_str());
		if(listing.getUser().user->isOnline()) {
			try {
				QueueManager::getInstance()->addList(listing.getUser(), QueueItem::FLAG_PARTIAL_LIST, listing.getPath(d));
			} catch(const QueueException& e) {
				//...
			}
		} else {
			typedef Func2<ShareBrowser, string, string> F2;
			F2 *func = new F2(this,&ShareBrowser::setStatus_gui,"mainStatus", _("User went Offline"));
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
}


int ShareBrowser::ThreadedDirectoryListing::run()
{
	typedef Func2<ShareBrowser, string, string> F2;
	F2 *func = new F2(mWindow,&ShareBrowser::setStatus_gui,"mainStatus", "Filelisting loading...");
	WulforManager::get()->dispatchGuiFunc(func);
	
 	try
 	{
		mWindow->listing.getRoot()->setName(mWindow->nick);
 	
 	if(!mFile.empty())
 	{
		mWindow->listing.loadFile(mFile);
		ADLSearchManager::getInstance()->matchListing(mWindow->listing);
 	}
 	else
 	{
		mDir = Text::toT(Util::toNmdcFile(mWindow->listing.updateXML(mTxt)));
 	}
		typedef Func2<ShareBrowser,dcpp::DirectoryListing::Directory*, GtkTreeIter*> F2;
		F2 *func = new F2(mWindow,&ShareBrowser::buildDirs_gui,mWindow->listing.getRoot(),NULL);
		WulforManager::get()->dispatchGuiFunc(func);
 	}
 	catch(const dcpp::Exception& e)
 	{
		string error = ClientManager::getInstance()->getNicks(mWindow->listing.getUser().user->getCID(), mWindow->listing.getUser().hint)[0] + ": " + e.getError();
		typedef Func2<ShareBrowser, string,string> F2;
		F2 *func = new F2(mWindow,&ShareBrowser::setStatus_gui,"mainStatus", error);
		WulforManager::get()->dispatchGuiFunc(func);
  	}
 	
	func = new F2(mWindow,&ShareBrowser::setStatus_gui,"mainStatus", "Filelisting loaded...");
	WulforManager::get()->dispatchGuiFunc(func);
 	
 	//cleanup the thread object
 	delete this;
 	
 	return 0;
}
