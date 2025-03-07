/*
 * Copyright © 2009-2012 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_SEARCH_ADL_H
#define _BMDC_SEARCH_ADL_H

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ADLSearch.h"
#include "../dcpp/RawManager.h"

#include "bookentry.hh"
#include "treeview.hh"

class SearchADL:
	public BookEntry
{
	public:
		SearchADL();
		virtual ~SearchADL();
		virtual void show();
	private:
		//typedef std::vector<ADLSearch>::size_type SearchType;

		// GUI functions
		void setSearch_gui(dcpp::ADLSearch &search, GtkTreeIter *iter);

		// GUI callbacks
		static void onAddClicked_gui(GtkWidget *widget, gpointer data);
		static void onPropertiesClicked_gui(GtkWidget *widget, gpointer data);
		static void onMoveUpClicked_gui(GtkWidget *widget, gpointer data);
		static void onMoveDownClicked_gui(GtkWidget *widget, gpointer data);
		static void onRemoveClicked_gui(GtkWidget *widget, gpointer data);
		static void onActiveToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
//		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
//		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
//		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static bool showPropertiesDialog_gui(dcpp::ADLSearch &search, bool edit, SearchADL *s);
		static void onToggleOveride(GtkWidget *widget,gpointer data);
		static void onToggleActions(GtkWidget *widget, gpointer data);
		static void onToggleForb(GtkWidget *widget, gpointer data);
		static void onChangeCombo(GtkWidget *widget, gpointer data);
		// Util functions
		int find_raw(const std::string rawString);
		int find_rawInt(const int raw);

		GdkEventType previous;
		TreeView searchADLView;
		GtkListStore *searchADLStore;
		GtkTreeSelection *searchADLSelection;
		gboolean sens,acts,bForbid;
};

#else
class SearchADL;
#endif
