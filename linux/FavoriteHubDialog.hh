/*
 * FavoriteHubDialog.hh
 * This file is part of BMDC++
 *
 * Copyright (C) 2012 - 2014 - Mank
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

	FavoriteHubDialog(FavoriteHubEntry* entry, bool add = true):
	Entry(Entry::FAV_HUB,"FavDialog"),
	p_entry(entry),
	init(add), actionStore(NULL), actionSel(NULL)
	{
		///Actions
		actionView.setView(GTK_TREE_VIEW(getWidget("rawview")));
		actionView.insertColumn(_("Name"), G_TYPE_STRING,TreeView::STRING,100);
		actionView.insertColumn(_("Enabled"), G_TYPE_BOOLEAN, TreeView::BOOL,100);
		actionView.insertHiddenColumn("ISRAW", G_TYPE_BOOLEAN);
		actionView.insertHiddenColumn("ID", G_TYPE_INT);
		actionView.finalize();
		actionStore = gtk_tree_store_newv(actionView.getColCount(),actionView.getGTypes());
		gtk_tree_view_set_model(actionView.get(),GTK_TREE_MODEL(actionStore));
		g_object_unref(actionStore);
		actionSel = gtk_tree_view_get_selection(actionView.get());

		g_signal_connect(getWidget("checkbuttonEncoding"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("comboboxCharset"));
		g_signal_connect(getWidget("checkbuttonNick"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryNick"));
		g_signal_connect(getWidget("checkbuttonUserDescription"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryUserDescription"));
		g_signal_connect(actionView.getCellRenderOf(_("Enabled")), "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	}

	bool initDialog(UnMapIter &groups)
	{
		FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

		GtkTreeIter iter;
		GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("groupsComboBox"))));

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, _("Default"), -1);
		groups.insert(UnMapIter::value_type(_("Default"), iter));

		for (auto i = favHubGroups.begin(); i != favHubGroups.end(); ++i)
		{
		// favorite hub properties combo box groups
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, 0, i->first.c_str(), -1);
			groups.insert(UnMapIter::value_type(i->first, iter));
		}

		string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
		StringList files = File::findFiles(path, "*.xml");
		for(auto fi = files.begin(); fi != files.end();++fi) {
			string file = Util::getFileName((*fi));
			size_t nedle =  file.find(".");
			string text = file.substr(0,nedle);
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxEmot")), text.c_str() );
		}
#if !GTK_CHECK_VERSION(3,12,0)		
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(getContainer()), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
#endif
		gtk_widget_set_sensitive(getWidget("comboboxCharset"), FALSE);
		gtk_widget_set_sensitive(getWidget("entryNick"), FALSE);
		gtk_widget_set_sensitive(getWidget("entryUserDescription"), FALSE);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(getContainer()), TRUE);

		gtk_window_set_transient_for(GTK_WINDOW(getContainer()),
		GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));

		// Fill the charset drop-down list in edit fav hub dialog.
		auto& charsets = WulforUtil::getCharsets();
		for (auto ic = charsets.begin(); ic != charsets.end(); ++ic)
		{
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxCharset")), (*ic).c_str());
		}

		initActions();
		// Populate the dialog with initial values
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryName")), p_entry->getName().c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryAddress")), p_entry->getServer().c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryDescription")), p_entry->getHubDescription().c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryNick")), p_entry->get(HubSettings::Nick).c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryPassword")), p_entry->getPassword().c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryUserDescription")),p_entry->get(HubSettings::Description).c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryIp")), p_entry->get(HubSettings::UserIp).c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryExtraInfo")), p_entry->getChatExtraInfo().c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryprotected")), p_entry->getProtectUsers().c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryeMail")), p_entry->get(HubSettings::Email).c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryAway")), p_entry->get(HubSettings::AwayMessage).c_str());


		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxMode")), p_entry->getMode());
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxParts")), p_entry->get(HubSettings::ShowJoins));
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxFavParts")), p_entry->get(HubSettings::FavShowJoins));

		auto it = groups.find(p_entry->getGroup());
		if (it != groups.end())
		{
			GtkTreeIter iter = it->second;
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(getWidget("groupsComboBox")), &iter);
		}
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("groupsComboBox")), 0);

		// Set the override default encoding checkbox. Check for "Global hub default"
		// for backwards compatability w/ 1.0.3. Should be removed at some point.
		string enc = p_entry->getEncoding();
		gboolean overrideEncoding = !(enc.empty() || enc == "Global hub default");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonEncoding")), overrideEncoding);

		for(auto ii = charsets.begin(); ii!=charsets.end(); ++ii) {
			if(enc == *ii) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxCharset")), (ii - charsets.begin()));
			}
		}
		string pack_name = p_entry->get(HubSettings::PackName);
		for(auto fii = files.begin(); fii!= files.end(); ++fii) {
			size_t needle = Util::getFileName(*fii).find(".");
			string tmp  = Util::getFileName(*fii).substr(0,needle);
			if(pack_name == tmp) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxEmot")), (fii - files.begin()));
			}
		}

		// Set the override default nick checkbox
		string nick = p_entry->get(HubSettings::Nick);
		gboolean overrideNick = !(nick.empty() || nick == SETTING(NICK));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonNick")), overrideNick);

		// Set the override default user description checkbox
		string desc = p_entry->get(HubSettings::Description);
		gboolean overrideUserDescription = !(desc.empty() || desc == SETTING(DESCRIPTION));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonUserDescription")), overrideUserDescription);

		// Set the auto connect checkbox
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkButtonAutoConnect")), p_entry->getAutoConnect());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkHide")), p_entry->getHideShare() );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkFilelist")), p_entry->getCheckFilelists()   );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkClients")), p_entry->getCheckClients() );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkoncon")),  p_entry->getCheckAtConn()  );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkLog")),   p_entry->get(HubSettings::LogChat)  );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkNoti")), p_entry->getNotify() );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkShowCountry")), p_entry->get(HubSettings::ShowCountry) );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkShowIp")), p_entry->get(HubSettings::ShowIps) );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkTabBold")), p_entry->get(HubSettings::BoldTab) );
		WulforManager::get()->insertEntry_gui(this);

		// Show the dialog
		gint response = gtk_dialog_run(GTK_DIALOG(getContainer()));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;

		while (response == GTK_RESPONSE_OK)
		{
			p_entry->setName(gtk_entry_get_text(GTK_ENTRY(getWidget("entryName"))));
			p_entry->setServer(gtk_entry_get_text(GTK_ENTRY(getWidget("entryAddress"))));
			p_entry->setHubDescription(gtk_entry_get_text(GTK_ENTRY(getWidget("entryDescription"))));
			p_entry->setPassword(gtk_entry_get_text(GTK_ENTRY(getWidget("entryPassword"))));
			p_entry->setGroup(Util::emptyString);
			p_entry->setChatExtraInfo(gtk_entry_get_text(GTK_ENTRY(getWidget("entryExtraInfo"))));
			p_entry->get(HubSettings::UserIp) = gtk_entry_get_text(GTK_ENTRY(getWidget("entryIp")));
			p_entry->setProtectUsers(gtk_entry_get_text(GTK_ENTRY(getWidget("entryprotected"))));
			p_entry->setNotify(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkNoti"))));
			p_entry->setMode(gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxMode"))));
			p_entry->setAutoConnect(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkButtonAutoConnect"))));
			p_entry->get(HubSettings::LogChat) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkLog")));
			p_entry->setHideShare(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkHide"))));
			p_entry->setCheckAtConn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkoncon"))));
			p_entry->setCheckFilelists(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkFilelist"))));
			p_entry->setCheckClients(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkClients"))));
			p_entry->get(HubSettings::ShowIps) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkShowIp")));
			p_entry->get(HubSettings::BoldTab) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkTabBold")));
			p_entry->get(HubSettings::ShowCountry) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkShowCountry")));
			p_entry->get(HubSettings::Email) = gtk_entry_get_text(GTK_ENTRY(getWidget("entryeMail")));
			p_entry->get(HubSettings::ShowJoins) =  gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxParts")));
			p_entry->get(HubSettings::FavShowJoins) = gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxFavParts")));
			p_entry->get(HubSettings::AwayMessage) = gtk_entry_get_text(GTK_ENTRY(getWidget("entryAway")));
			//temp fix ( disabling IPv6 by default)
			p_entry->get(HubSettings::Connection) = 1;
			p_entry->get(HubSettings::Connection6) = 0;
			

			if (gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("groupsComboBox"))) != 0)
			{
				gchar *group = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("groupsComboBox")));
				if(group) {
					p_entry->setGroup(string(group));
					g_free(group);
			   	}
			}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonEncoding"))))
		{
			gchar *encoding = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxCharset")));
			if(encoding)
			{
				p_entry->setEncoding(string(encoding));
				g_free(encoding);
			}
		}

		gchar *pack = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxEmot")));
		if(pack)
		{
			p_entry->get(HubSettings::PackName) = string(pack);
			g_free(pack);
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonNick"))))
		{
			p_entry->get(HubSettings::Nick) = gtk_entry_get_text(GTK_ENTRY(getWidget("entryNick")));
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonUserDescription"))))
		{
			p_entry->get(HubSettings::Description) = gtk_entry_get_text(GTK_ENTRY(getWidget("entryUserDescription")));
		}

		if (p_entry->getName().empty() || p_entry->getServer().empty())
		{
			if (showErrorDialog_gui(_("The name and address fields are required")))
			{
				response = gtk_dialog_run(GTK_DIALOG(getContainer()));

				// Fix crash, if the dialog gets programmatically destroyed.
				if (response == GTK_RESPONSE_NONE)
					return FALSE;
			}
			else
				return FALSE;
		}
		else
		{
			gtk_widget_hide(getContainer());
			return TRUE;
		}
	}
	gtk_widget_hide(getContainer());
	return FALSE;

	}
	~FavoriteHubDialog() {
		WulforManager::get()->deleteEntry_gui(this);
	}
	GtkWidget *getContainer() { return getWidget("dialog"); }

private:
	void initActions()
	{
		GtkTreeIter toplevel;

		gtk_tree_store_clear(actionStore);

		const Action::ActionList& list = RawManager::getInstance()->getActions();

		for(auto  it = list.begin();it!= list.end();++it)
		{
			const string& name = (*it)->getName();

			gtk_tree_store_append(actionStore,&toplevel,NULL);
			gtk_tree_store_set(actionStore,&toplevel,
						actionView.col(_("Name")), name.c_str(),
						actionView.col(_("Enabled")), (*it)->getEnabled() ? TRUE : FALSE,
						actionView.col("ISRAW"), FALSE,
						actionView.col("ID"), (*it)->getId(),
						-1);

			GtkTreeIter child;

			for(auto i = (*it)->raw.begin(); i != (*it)->raw.end(); ++i)
			{
				string rname = (*i).getName();
				gtk_tree_store_append(actionStore,&child,&toplevel);
				gtk_tree_store_set(actionStore,&child,
						actionView.col(_("Name")), rname.c_str(),
						actionView.col(_("Enabled")), (*i).getEnabled() ? TRUE : FALSE,
						actionView.col("ISRAW"), TRUE,
						actionView.col("ID"), (*i).getId(),
						-1);
			}
		}
	}

	static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	static void onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data);
	bool showErrorDialog_gui(const string &description);

	FavoriteHubEntry* p_entry;
	bool init;
	GtkTreeStore *actionStore;
	TreeView actionView;
	GtkTreeSelection *actionSel;

};

#endif /* _BMDC_FAVORITE_HUB_DIALOG_H_ */
