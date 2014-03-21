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
#include <dcpp/FavoriteManager.h>
#include <dcpp/RawManager.h>
#include "entry.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

class FavoriteHubDialog: public Entry
{
   public:

	FavoriteHubDialog(FavoriteHubEntry* entry, bool add = true):
	Entry(Entry::FAV_HUB,"FavDialog.glade"),
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

	bool initDialog(UnMapIter &groups, dcpp::StringMap &params)
	{
		FavHubGroups favHubGroups = FavoriteManager::getInstance()->getFavHubGroups();

		GtkTreeIter iter;
		GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("groupsComboBox"))));

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, _("Default"), -1);
		groups.insert(UnMapIter::value_type(_("Default"),iter));

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

		if(init) //Default values when adding
		{
			params["Name"] = Util::emptyString;
			params["Address"] = Util::emptyString;
			params["Description"] = Util::emptyString;
			params["Nick"] = Util::emptyString;
			params["Password"] = Util::emptyString;
			params["User Description"] = Util::emptyString;
			params["Encoding"] = Util::emptyString;
			params["Group"] = Util::emptyString;
			params["IP"] = Util::emptyString;
			params["Mode"] = "0";
			params["Auto Connect"] = "0";
			params["Hide"] = "0";
			params["Clients"] = "0";
			params["Filelists"] = "0";
			params["OnConnect"] = "0";
			params["ExtraInfo"] = Util::emptyString;
			params["Protected"] = Util::emptyString;
			params["eMail"] = Util::emptyString;
			params["Parts"] = Util::emptyString;
			params["FavParts"] = Util::emptyString;
			params["LogChat"] = Util::emptyString;
			params["Away"] = Util::emptyString;
			params["Notify"] = "0";
			params["Country"] = "0";
			params["showip"] = "0";
			params["BoldTab"] = "1";
			params["PackName"] = "bmicon";//emoticons

		}//end

		gtk_dialog_set_alternative_button_order(GTK_DIALOG(getContainer()), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
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
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryName")), params["Name"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryAddress")), params["Address"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryDescription")), params["Description"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryNick")), params["Nick"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryPassword")), params["Password"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryUserDescription")), params["User Description"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryIp")), params["IP"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryExtraInfo")), params["ExtraInfo"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryprotected")), params["Protected"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryeMail")), params["eMail"].c_str());
		gtk_entry_set_text(GTK_ENTRY(getWidget("entryAway")), params["Away"].c_str());


		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxMode")), Util::toInt64(params["Mode"]));
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxParts")), Util::toInt64(params["Parts"]));
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxFavParts")), Util::toInt64(params["FavParts"]));

		auto it = groups.find(params["Group"]);
		if (it != groups.end())
		{
			GtkTreeIter iter = it->second;
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(getWidget("groupsComboBox")), &iter);
		}
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("groupsComboBox")), 0);

		// Set the override default encoding checkbox. Check for "Global hub default"
		// for backwards compatability w/ 1.0.3. Should be removed at some point.
		gboolean overrideEncoding = !(params["Encoding"].empty() || params["Encoding"] == "Global hub default");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonEncoding")), overrideEncoding);

		for(auto ii = charsets.begin(); ii!=charsets.end(); ++ii) {
			if(params["Encoding"] == *ii) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxCharset")), (ii - charsets.begin()));
			}
		}
		
		for(auto fii = files.begin(); fii!= files.end(); ++fii) {
			auto needle = Util::getFileName(*fii).find(".");
			string tmp  = Util::getFileName(*fii).substr(0,needle);
			if(params["PackName"] == tmp) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxEmot")), (fii - files.begin()));
			}
		}

		// Set the override default nick checkbox
		gboolean overrideNick = !(params["Nick"].empty() || params["Nick"] == SETTING(NICK));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonNick")), overrideNick);

		// Set the override default user description checkbox
		gboolean overrideUserDescription = !(params["User Description"].empty() || params["User Description"] == SETTING(DESCRIPTION));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonUserDescription")), overrideUserDescription);


		auto f = [this,&params](std::string name) -> gboolean { return params[name] == "1" ? TRUE : FALSE; };
		// Set the auto connect checkbox
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkButtonAutoConnect")), f("Auto Connect"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkHide")), f("Hide"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkFilelist")), f("Filelists"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkClients")), f("Clients") );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkoncon")), f("OnConnect")  );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkLog")), f("LogChat") );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkNoti")), f("Notify") );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkShowCountry")), f("Country"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkShowIp")), f("showip") );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkTabBold")), f("BoldTab") );
		WulforManager::get()->insertEntry_gui(this);

		//fh->setRawActions_gui(fh,params);
		// Show the dialog
		gint response = gtk_dialog_run(GTK_DIALOG(getContainer()));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;

		while (response == GTK_RESPONSE_OK)
		{
			params.clear();
			params["Name"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryName")));
			params["Address"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryAddress")));
			params["Description"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryDescription")));
			params["Password"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryPassword")));
			params["Group"] = Util::emptyString;
			params["ExtraInfo"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryExtraInfo")));
			params["IP"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryIp")));
			params["Protected"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryprotected")));
			params["Notify"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkNoti"))) ? "1" : "0";
			params["Mode"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxMode"))));
			params["Auto Connect"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkButtonAutoConnect"))) ? "1" : "0";
			params["LogChat"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkLog"))) ? "1" : "0";
			params["Hide"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkHide"))) ? "1" : "0";
			params["OnConnect"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkoncon"))) ? "1" : "0";
			params["Filelists"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkFilelist"))) ? "1" : "0";
			params["Clients"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkClients"))) ? "1" : "0";
			params["showip"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkShowIp"))) ? "1" : "0";
			params["BoldTab"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkTabBold"))) ? "1" : "0";
			params["Country"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkShowCountry"))) ? "1" : "0";
			params["eMail"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryeMail")));
			params["Parts"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxParts"))));
			params["FavParts"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxFavParts"))));
			params["Away"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryAway")));

			if (gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("groupsComboBox"))) != 0)
			{
				gchar *group = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("groupsComboBox")));
				if(group) {
					params["Group"] = string(group);
					g_free(group);
			   	}
			}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonEncoding"))))
		{
			gchar *encoding = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxCharset")));
			if(encoding)
			{
				params["Encoding"] = string(encoding);
				g_free(encoding);
			}
		}

		gchar *pack = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxEmot")));
		if(pack)
		{
			params["PackName"] = string(pack);
			g_free(pack);
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonNick"))))
		{
			params["Nick"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryNick")));
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonUserDescription"))))
		{
			params["User Description"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryUserDescription")));
		}

//		fh->setRawActions_client(fh,params);

		if (params["Name"].empty() || params["Address"].empty())
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

	FavoriteHubEntry* p_entry;
	bool init;
	GtkTreeStore *actionStore;
	TreeView actionView;
	GtkTreeSelection *actionSel;
	static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
	{
		FavoriteHubDialog *fh = (FavoriteHubDialog *)data;
		GtkTreeIter iter;

		if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fh->actionStore), &iter, path))
		{
			bool fixed = fh->actionView.getValue<gboolean>(&iter, _("Enabled"));
			fixed = !fixed;
			gtk_tree_store_set(fh->actionStore, &iter, fh->actionView.col(_("Enabled")), fixed, -1);
		}

	}

	static void onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data){
			GtkWidget *widget = (GtkWidget*)data;
			bool override = gtk_toggle_button_get_active(button);

			gtk_widget_set_sensitive(widget, override);

			if (override)
			{
				gtk_widget_grab_focus(widget);
			}
	}

	bool showErrorDialog_gui(const string &description)
	{
		GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(getContainer()),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());

		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;

		gtk_widget_destroy(dialog);

		return TRUE;
	}

};

#endif /* _BMDC_FAVORITE_HUB_DIALOG_H_ */
