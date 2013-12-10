/*
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

#ifndef BMDC_SEARCH_ENTRY_HH
#define BMDC_SEARCH_ENTRY_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "entry.hh"
#include "bookentry.hh"
#include <dcpp/SearchManager.h>
#include <vector>
#include "search.hh"

class SearchEntry: public BookEntry
{
	private:
		void showBook(Entry::EntryType type, BookEntry *entry);
	public:
		SearchEntry();
		virtual ~SearchEntry();

		SearchEntry* putValue_gui(const std::string &str,
					int64_t size,
					dcpp::SearchManager::SizeModes mode,
					dcpp::SearchManager::TypeModes type) {

					Search *s = dynamic_cast<Search*>(findBookEntry(Entry::SEARCH,str));
					if(s == NULL) {
						s = new Search(str);
						showBook(Entry::SEARCH,s);
						s->putValue_gui(str,size,mode, type);
					}
					raisePage_gui(s->getContainer());
					return this;
		}

		virtual void show() {
			showBook(Entry::SEARCH, new Search(dcpp::Util::emptyString));
		}
		void setTabPosition_gui(GtkPositionType pos)
		{ gtk_notebook_set_tab_pos(GTK_NOTEBOOK(getWidget("sebook")), pos);}
		void removeBookEntry_gui(BookEntry *entry);
		void addBookEntry_gui(BookEntry *entry);
		static void onRaisePage_gui(GtkWidget* item,gpointer data)
		{
			gpointer sp = g_object_get_data(G_OBJECT(item),"data");
			((SearchEntry*)sp)->raisePage_gui((GtkWidget*)data);
		}
	private:
		GtkWidget *currentPage_gui();
		void raisePage_gui(GtkWidget *page);
		static void onPageSwitched_gui(GtkNotebook *notebook, GtkWidget *page, guint num , gpointer data);
		static gboolean onButtonReleasePage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onCloseBookEntry_gui(GtkWidget *widget, gpointer data);
		BookEntry* findBookEntry(const EntryType type, const std::string &id = "");
		
		std::vector<BookEntry*> books;
};
#else
class SearchEntry;
#endif
