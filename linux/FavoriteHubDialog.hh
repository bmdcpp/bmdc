/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_FAVORITE_HUB_DIALOG_H_
#define _BMDC_FAVORITE_HUB_DIALOG_H_

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/File.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/RawManager.h"
#include "entry.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "treeview.hh"

using namespace std;
using namespace dcpp;

class FavoriteHubDialog: public Entry
{
   public:
   		//@: entry : The Fav Entry to process to dialog
   		//@: and always is it called like that
		explicit FavoriteHubDialog(FavoriteHubEntry* entry, bool updated = false);
		~FavoriteHubDialog() 
		{
		
		}
		GtkWidget* getContainer() { return mainDialog; }
		FavoriteHubEntry* getEntryFav() { return p_entry;}

private:
		void initActions();
		
		static void	onResponse(GtkWidget* dialog ,int response, gpointer data);

		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data);

		static void onAddShare_gui(GtkWidget *widget, gpointer data);
		static void onRemoveShare_gui(GtkWidget *widget, gpointer data);
		//static gboolean onShareButtonReleased_gui(GtkWidget*, GdkEventButton*, gpointer data);

		bool showErrorDialog_gui(const string &description);
		void updateShares_gui();
		void addShare_gui(string path, string name);

		bool updated;
		FavoriteHubEntry* p_entry; //@: The Fav Entry pointer
		//@TreeViews
		//@: The Kick/Ban Action 
		// share
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
					*checkHigh,
		//@: Connection
					*comboMode,
					*entryIp,
					*checkUseSock5,
		//@: Kick View
					*treeView,
		//@Share
					*button_add,
					*button_rem,
					*labelShareSize,
					*shareTree;

};

#endif /* _BMDC_FAVORITE_HUB_DIALOG_H_ */
