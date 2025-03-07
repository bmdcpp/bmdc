/*
* Copyright © 2009-2017 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SEARCH_SPY_HH
#define SEARCH_SPY_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/TimerManager.h"
#include "GuiUtil.hh"
#include "bookentry.hh"
#include "treeview.hh"

class SearchSpy:
	public BookEntry,
	public dcpp::ClientManagerListener,
	public dcpp::TimerManagerListener
{
	public:
		SearchSpy();
		virtual ~SearchSpy();
		virtual void show();
		void preferences_gui();

	private:
		using dcpp::ClientManagerListener::on;
		using dcpp::TimerManagerListener::on;
		typedef UnMapIter::size_type SearchType;

		// GUI functions
		bool updateFrameStatus_gui(GtkTreeIter *iter, uint64_t tick);
		void updateFrameStatus_gui();
		bool findIter_gui(const std::string &search, GtkTreeIter *iter);
		void updateFrameSearch_gui(const std::string search, const std::string type);
		void setStatus_gui(const std::string text);
		void addTop_gui(const std::string &search, const std::string &type);
		void resetFrame();
		void resetCount();

		// GUI callbacks
		static void onSearchItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);
		static void onRemoveItemClicked_gui(GtkWidget *widget,GVariant  *parameter, gpointer data);


		static void onClickPressed_gui(GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data);
		static void onClickReleased_gui(GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   GtkWidget         *data);
		
		static void onClearFrameClicked_gui(GtkWidget *widget, gpointer data);
		static void onUpdateFrameClicked_gui(GtkWidget *widget, gpointer data);
		static void onShowTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onClearTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onRemoveTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onIgnoreTTHSearchToggled_gui(GtkWidget *widget, gpointer data);
		static void onOKButtonClicked_gui(GtkWidget *widget, gpointer data);
		//static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);

		// Client callbacks
		virtual void on(dcpp::ClientManagerListener::IncomingSearch, const std::string& s) noexcept;
		virtual void on(dcpp::TimerManagerListener::Minute, uint32_t tick) noexcept;

		SearchType FrameSize;
		guint Waiting;
		guint Top;
		GdkEventType previous;
		TreeView searchView;
		GtkListStore *searchStore;
		GtkTreeSelection *searchSelection;
		UnMapIter searchIters;
		TreeView topView;
		GtkListStore *topStore;
		std::string aSearchColor, cSearchColor, rSearchColor, tSearchColor, qSearchColor;
		static const GActionEntry win_entries[];
};

#else
class SearchSpy;
#endif
