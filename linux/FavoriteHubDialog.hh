/*
 * Copyright (C) 2012 - 2016 - BMDC
 *
 * BMDC++ is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * BMDC++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _BMDC_FAVORITE_HUB_DIALOG_H_
#define _BMDC_FAVORITE_HUB_DIALOG_H_

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/File.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/RawManager.h>
#include "entry.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "treeview.hh"

using namespace std;
using namespace dcpp;

class FavoriteHubDialog: public Entry
{
   public:
   		//@: entry : The Fav Entry to process to dialog
   		//@: and always is it called like that
		explicit FavoriteHubDialog(FavoriteHubEntry* entry);
		//@: groups : wich groups should dialog show up
		bool initDialog(UnMapIter &groups);
		
		~FavoriteHubDialog() {
			WulforManager::get()->deleteEntry_gui(this);
		}
		GtkWidget* getContainer() { return mainDialog; }
		
private:
		void initActions();
		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data);
		
		static void onAddShare_gui(GtkWidget *widget, gpointer data);
		static void onRemoveShare_gui(GtkWidget *widget, gpointer data);
		
		bool showErrorDialog_gui(const string &description);
		void updateShares_gui();
		void addShare_gui(string path, string name);

		FavoriteHubEntry* p_entry; //@: The Fav Entry pointer
		//@: The Kick/Ban Action part// share
		GtkTreeStore *actionStore;
		GtkListStore *shareStore;
		TreeView actionView;
		TreeView shareView;
		GtkTreeSelection *actionSel;
		//@: Widgets
		GtkWidget	*mainDialog, //@ dialog container
					*mainBox, //@ main Things container from Dialog
					*notebook, //  Switchable pages
					*boxSimple, //1st page
					*boxAdvanced,
		//@: Hub-stuff
					*entryAddress,
					*entryName,
					*entryDesc,
					*comboCodepage,
					*checkAutoConnect,
					*comboGroup,
		//@: User-info related stuff
					*entryUsername,
					*entryPassword,
					*entryUserDescriptio,
					*entryMail,
		//@: Checking
					*entryProtectedUser,
					*checkFilelists,
					*checkClients,
					*checkOnConn,
		//@: Chat&Others
					*extraChatInfoEntry,
					*checkHideShare,
					*entryAwayMessage,
					*comboParts,*comboFavParts,
					*colorBack,
					*backImage,
					*comboEmot,
					*enableNoti,
					*enableLog,
					*enableCountry,
					*enableIp,
					*enableIp6, 
					*enableBold,
					*enableStatusChat,
					*enableFavFirst,
		//@: Connection
					*comboMode,
					*entryIp,
		//@: Kick View
					*treeView,
		//@Share
					*button_add,
					*button_rem,
					*labelShareSize,
					*shareTree;

};

#endif /* _BMDC_FAVORITE_HUB_DIALOG_H_ */
