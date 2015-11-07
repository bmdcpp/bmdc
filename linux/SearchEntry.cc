/*
 * Copyright © 2012-2015 BMDC++
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

#include "entry.hh"
#include "SearchEntry.hh"
#include "search.hh"
#include "wulformanager.hh"

using namespace std;
using namespace dcpp;

SearchEntry::SearchEntry():
	BookEntry(Entry::SEARCHS, "Searches","SearchEntry")
{
	// All notebooks created in glade/ui need one page.
	// In our case, this is just a placeholder, so we remove it.
	gtk_notebook_remove_page(GTK_NOTEBOOK(getWidget("sebook")), -1);
	g_object_set_data(G_OBJECT(getWidget("sebook")), "page-rotation-list", NULL);

	g_signal_connect(getWidget("sebook"), "switch-page", G_CALLBACK(onPageSwitched_gui), (gpointer)this);

}

SearchEntry::~SearchEntry()
{
	GList *list = (GList *)g_object_get_data(G_OBJECT(getWidget("sebook")), "page-rotation-list");
	g_list_free(list);

	books.clear();

}

void SearchEntry::addBookEntry_gui(BookEntry *entry)
{
	addChild(entry);

	GtkWidget *page = entry->getContainer();
	GtkWidget *label = entry->getLabelBox();
	GtkWidget *closeButton = entry->getCloseButton();

	gtk_notebook_append_page(GTK_NOTEBOOK(getWidget("sebook")), page, label);
	
	g_signal_connect(label, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
	g_signal_connect(closeButton, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
	g_signal_connect(closeButton, "clicked", G_CALLBACK(onCloseBookEntry_gui), (gpointer)entry);

	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(getWidget("sebook")), page, FALSE);

	entry->show();
	books.push_back(entry);
}

void SearchEntry::raisePage_gui(GtkWidget *page)
{
	int num = gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("sebook")), page);
	int currentNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("sebook")));

	if (num != -1 && num != currentNum)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(getWidget("sebook")), num);
}

void SearchEntry::onPageSwitched_gui(GtkNotebook *notebook, GtkWidget *, guint num, gpointer data)
{
	GtkWidget *child = gtk_notebook_get_nth_page(notebook, num);
	BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");

	if (entry)
	{
		// Disable "activate" signal on the tab menu item since it can cause
		// onPageSwitched_gui to be called multiple times
		GtkWidget *item = entry->getTabMenuItem();
		g_object_set_data(G_OBJECT(item),"data",(gpointer)data);
		g_signal_handlers_block_by_func(item, (gpointer)onRaisePage_gui, child);

		entry->setActive_gui();
		g_object_set_data(G_OBJECT(item),"data",(gpointer)data);
		g_signal_handlers_unblock_by_func(item, (gpointer)onRaisePage_gui, (gpointer)child);
	}

	GList *list = (GList *)g_object_get_data(G_OBJECT(notebook), "page-rotation-list");
	list = g_list_remove(list, (gpointer)child);
	list = g_list_prepend(list, (gpointer)child);
	g_object_set_data(G_OBJECT(notebook), "page-rotation-list", (gpointer)list);

	// Focus the tab so it will focus its children (e.g. a text entry box)
	gtk_widget_grab_focus(child);
}

BookEntry* SearchEntry::findBookEntry(const EntryType type, const string &id)
{
	Entry *entry = getChild(type, id);
	return dynamic_cast<BookEntry*>(entry);
}

void SearchEntry::showBook(BookEntry *book)
{
	BookEntry *entry = findBookEntry(Entry::SEARCH);

	if(entry == NULL)
	{
			entry = book;
			addBookEntry_gui(entry);
	}
	raisePage_gui(entry->getContainer());
}

void SearchEntry::removeBookEntry_gui(BookEntry *entry)
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("sebook"));
	GtkWidget *page = entry->getContainer();
	int num = gtk_notebook_page_num(book, page);
	removeChild(entry);

	if (num != -1)
	{
		GList *list = (GList *)g_object_get_data(G_OBJECT(book), "page-rotation-list");
		list = g_list_remove(list, (gpointer)page);
		g_object_set_data(G_OBJECT(book), "page-rotation-list", (gpointer)list);

		// if removing the current page, switch to the previous page in the rotation list
		if (num == gtk_notebook_get_current_page(book))
		{
			GList *prev = g_list_first(list);
			if (prev != NULL)
			{
				gint childNum = gtk_notebook_page_num(book, GTK_WIDGET(prev->data));
				gtk_notebook_set_current_page(book, childNum);
			}
		}
		gtk_notebook_remove_page(book, num);

		if (gtk_notebook_get_n_pages(book) == 0)
		{
			Search *s = new Search(Util::emptyString);
			addBookEntry_gui(s);
		}
	}
}

gboolean SearchEntry::onButtonReleasePage_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	gint width, height;
	width = gdk_window_get_width(event->window);
	height = gdk_window_get_height(event->window);

	// If middle mouse button was released when hovering over tab label
	if (event->button == 2 && event->x >= 0 && event->y >= 0
		&& event->x < width && event->y < height)
	{
		BookEntry *entry = (BookEntry *)data;
		WulforManager::get()->getMainWindow()->getSearchEntry()->removeBookEntry_gui(entry);
		return TRUE;
	}

	return FALSE;
}

void SearchEntry::onCloseBookEntry_gui(GtkWidget*, gpointer data)
{
	BookEntry *entry = (BookEntry *)data;
	WulforManager::get()->getMainWindow()->getSearchEntry()->removeBookEntry_gui(entry);
}
