/*
 * Copyright © 2009-2011 freedcpp, http://code.google.com/p/freedcpp
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

#ifndef SEARCH_ADL_H
#define SEARCH_ADL_H

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/ADLSearch.h>
#include <dcpp/RawManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class ADLSearchGUI:
	public BookEntry
{
	public:
		ADLSearchGUI();
		virtual ~ADLSearchGUI();
		virtual void show();
		virtual void popmenu();
	private:
		typedef dcpp::ADLSearchManager::SearchCollection::size_type SearchType;

		// GUI functions
		void setSearch_gui(dcpp::ADLSearch &search, GtkTreeIter *iter);

		// GUI callbacks
		static void onAddClicked_gui(GtkWidget *widget, gpointer data);
		static void onPropertiesClicked_gui(GtkWidget *widget, gpointer data);
		static void onMoveUpClicked_gui(GtkWidget *widget, gpointer data);
		static void onMoveDownClicked_gui(GtkWidget *widget, gpointer data);
		static void onRemoveClicked_gui(GtkWidget *widget, gpointer data);
		static void onActiveToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static bool showPropertiesDialog_gui(dcpp::ADLSearch &search, bool edit, ADLSearchGUI *s);
		static void onToggleOveride(GtkWidget *widget,gpointer data);
		static void onToggleActions(GtkWidget *widget, gpointer data);
		static void onToggleForb(GtkWidget *widget, gpointer data);
		static void onChangeCombo(GtkWidget *widget, gpointer data);

        static void onCloseItem(gpointer data);
		int find_raw(std::string rawString);
		int find_rawInt(int raw);
		
		GdkEventType previous;
		TreeView searchADLView;
		GtkListStore *searchADLStore;
		GtkTreeSelection *searchADLSelection;
		gboolean sens,acts,forbid;
};

#else
class ADLSearchGUI;
#endif
