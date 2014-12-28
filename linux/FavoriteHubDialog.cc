// Copyright (C) 2014-2015  Mank <freedppp@seznam.cz>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with main.c; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA

#include "FavoriteHubDialog.hh"

using namespace std;
using namespace dcpp;

#define gen gtk_entry_new()
#define lan(x) gtk_label_new(x)

#define grid_add(box,widget,x,y,z,c) gtk_grid_attach(GTK_GRID(box), widget ,x,y,z,c)
#define g_g_a(widget,x,y,z,c) grid_add(boxSimple,widget,x,y,z,c)
#define g_g_a_c(widget,x,y,z,c) grid_add( boxCheck, widget ,x,y,z,c)
#define g_g_a_a(widget,x,y,z,c) grid_add( boxAdvanced, widget ,x,y,z,c)
#define g_g_a_c_s(widget,x,y,z,c) grid_add(boxConnection, widget, x,y,z,c)

#define g_c_b_n(label) gtk_check_button_new_with_label(label)

static GtkWidget* createComboBoxWith3Options(const gchar* a,const gchar* b,const gchar* c)
{
	GtkWidget* combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),a);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),b);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),c);	
	
	return combo;
}

FavoriteHubDialog::FavoriteHubDialog(FavoriteHubEntry* entry, bool add /* = true */):
	Entry(Entry::FAV_HUB),
	p_entry(entry),
	init(add), actionStore(NULL), actionSel(NULL)
{
	mainDialog = gtk_dialog_new();
	if(!p_entry->getServer().empty())
		gtk_window_set_title (GTK_WINDOW(mainDialog), (_("Favorite Propteries for ")+p_entry->getServer()).c_str());
	else 
		gtk_window_set_title (GTK_WINDOW(mainDialog), _("Favorite Propteries for New Favorite Hub"));	
	
	mainBox = gtk_dialog_get_content_area ( GTK_DIALOG(mainDialog) );
	notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(mainBox), notebook);
	boxSimple = gtk_grid_new();
	GtkWidget* labelName = lan(_("Name: "));	
	g_g_a(labelName,0,0,1,1);
	entryName = gen;
	gtk_entry_set_text (GTK_ENTRY(entryName), p_entry->getName().c_str());
	g_g_a(entryName,1,0,1,1);
	GtkWidget*  labelAddress = lan(_("Address: "));
	g_g_a(labelAddress,0,1,1,1);
	entryAddress = gen;
	gtk_entry_set_text (GTK_ENTRY(entryAddress), p_entry->getServer().c_str());
	g_g_a(entryAddress,1,1,1,1);
	GtkWidget* labelDesc = lan(_("Description: "));
	g_g_a(labelDesc,0,2,1,1);
	entryDesc = gen;
	gtk_entry_set_text (GTK_ENTRY(entryDesc), p_entry->getHubDescription().c_str());
	g_g_a(entryDesc,1,2,1,1);
	//
	GtkWidget* labelUsername = lan(_("Username: "));
	g_g_a(labelUsername,0,3,1,1);
	entryUsername = gen;
	gtk_entry_set_text (GTK_ENTRY(entryUsername), p_entry->get(SettingsManager::NICK,SETTING(NICK)).c_str());
	g_g_a(entryUsername,1,3,1,1);
	GtkWidget* labelPassword = lan(_("Password: "));
	g_g_a(labelPassword,0,4,1,1);
	entryPassword = gen;
	gtk_entry_set_visibility (GTK_ENTRY(entryPassword),FALSE);
	gtk_entry_set_text (GTK_ENTRY(entryPassword), p_entry->getPassword().c_str());
	g_g_a(entryPassword,1,4,1,1);
	GtkWidget* labelUserName = lan(_("User Description: "));
	g_g_a(labelUserName,0,5,1,1);
	entryUserDescriptio = gen;
	gtk_entry_set_text (GTK_ENTRY(entryUserDescriptio), p_entry->get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION)).c_str());
	g_g_a(entryUserDescriptio,1,5,1,1);
	
	GtkWidget* labelmail = lan(_("e-Mail: "));
	g_g_a(labelmail,0,6,1,1);
	entryMail = gen;
	gtk_entry_set_text (GTK_ENTRY(entryMail), p_entry->get(SettingsManager::EMAIL,SETTING(EMAIL)).c_str());
	g_g_a(entryMail,1,6,1,1);
	
	GtkWidget* labelCodePage = lan(_("Codepage: "));
	g_g_a(labelCodePage,0,7,1,1);
	comboCodepage = gtk_combo_box_text_new();
	g_g_a(comboCodepage,1,7,1,1);
	string enc = p_entry->getEncoding();
	// Fill the charset drop-down list in edit fav hub dialog.
	auto& charsets = WulforUtil::getCharsets();
	for (auto ic = charsets.begin(); ic != charsets.end(); ++ic)
	{
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboCodepage), (*ic).c_str());
	}
	bool set = false;
	for(auto ii = charsets.begin(); ii!=charsets.end(); ++ii) {
			if(enc == *ii) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(comboCodepage), (ii - charsets.begin()));
				set = true;
			}
	}
	//Default
	if(set == false){
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboCodepage),0);
	}
	checkAutoConnect = g_c_b_n(_("Auto-Connect to this Hub"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkAutoConnect), p_entry->getAutoConnect());
	
	g_g_a(checkAutoConnect,0,8,1,1);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), boxSimple ,lan(_("General Settings")));
	//check
	GtkWidget* checkInfo  = lan(_("Checking"));
	GtkWidget* boxCheck = gtk_grid_new();
	GtkWidget* labelProtected = lan(_("Protected Users:"));
	g_g_a_c(labelProtected,0,0,1,1);
	entryProtectedUser = gen;
	gtk_entry_set_text(GTK_ENTRY(entryProtectedUser), p_entry->get(SettingsManager::PROTECTED_USERS,SETTING(PROTECTED_USERS)).c_str());
	g_g_a_c(entryProtectedUser,1,0,1,1);
	checkFilelists = g_c_b_n(_("Check Filelists"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkFilelists),p_entry->getCheckFilelists());
	g_g_a_c(checkFilelists,0,1,1,1);
	checkClients = g_c_b_n(_("Check Clients"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkClients),p_entry->getCheckClients());
	g_g_a_c(checkClients,1,1,1,1);
	checkOnConn = g_c_b_n(_("Check On Connect to Hub"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkOnConn),p_entry->getCheckAtConn());
	g_g_a_c(checkOnConn,0,2,1,1);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), boxCheck ,checkInfo);
	//
	boxAdvanced	= gtk_grid_new();
	GtkWidget* labelAdvanced = lan(_("Chat&Misc"));
	checkHideShare = g_c_b_n(_("Hide Share"));//@TODO: move
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkHideShare), p_entry->getHideShare() );

	g_g_a_a(checkHideShare,0,0,1,1);
	GtkWidget* labelExtraChat = lan(_("Extra Chat Info:"));
	g_g_a_a(labelExtraChat,0,1,1,1);
	extraChatInfoEntry = gen;
	gtk_entry_set_text(GTK_ENTRY(extraChatInfoEntry), p_entry->get(SettingsManager::CHAT_EXTRA_INFO,SETTING(CHAT_EXTRA_INFO)).c_str());
	g_g_a_a(extraChatInfoEntry,1,1,1,1);
	GtkWidget* labelAwayMessage = lan(_("Away Message:"));
	g_g_a_a(labelAwayMessage,0,2,1,1);
	entryAwayMessage = gen;
	gtk_entry_set_text(GTK_ENTRY(entryAwayMessage), p_entry->get(SettingsManager::DEFAULT_AWAY_MESSAGE,SETTING(DEFAULT_AWAY_MESSAGE)).c_str());
	g_g_a_a(entryAwayMessage,1,2,1,1);
	
	GtkWidget* label_FavParts = lan(_("Favorite Users Joins/Parts:"));
	g_g_a_a(label_FavParts ,0,3,1,1);
	comboFavParts = createComboBoxWith3Options("Default","Enable","Disable");
	if(p_entry->get(SettingsManager::FAV_SHOW_JOINS,SETTING(FAV_SHOW_JOINS)) == SETTING(FAV_SHOW_JOINS))
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboFavParts), 0);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboFavParts),p_entry->get(SettingsManager::FAV_SHOW_JOINS,SETTING(FAV_SHOW_JOINS))+1);
	g_g_a_a(comboFavParts,1,3,1,1);
	
	GtkWidget* label_Parts = lan(_("Users Joins/Parts:"));
	g_g_a_a(label_Parts,0,4,1,1);
	comboParts = createComboBoxWith3Options("Default","Enable","Disable");

	if(p_entry->get(SettingsManager::FAV_SHOW_JOINS,SETTING(SHOW_JOINS)) == SETTING(SHOW_JOINS))
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboParts), 0);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboParts),p_entry->get(SettingsManager::SHOW_JOINS,SETTING(SHOW_JOINS))+1);

	g_g_a_a(comboParts,1,4,1,1);
	
	GtkWidget* label_BackChatColor =  lan(_("Background Chat Color:"));
	g_g_a_a(label_BackChatColor,0,5,1,1);
	colorBack = gtk_color_button_new();
	GdkRGBA color;
	gdk_rgba_parse(&color,p_entry->get(SettingsManager::BACKGROUND_CHAT_COLOR,SETTING(BACKGROUND_CHAT_COLOR)).c_str());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorBack),&color);
	g_g_a_a(colorBack,1,5,1,1);
	
	GtkWidget* label_BackChatImage =  lan(_("Background Chat Image:"));
	g_g_a_a(label_BackChatImage,0,6,1,1);
	backImage = gtk_file_chooser_button_new(_("Open Image"),GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_select_filename (GTK_FILE_CHOOSER(backImage),p_entry->get(SettingsManager::BACKGROUND_CHAT_IMAGE,SETTING(BACKGROUND_CHAT_IMAGE)).c_str());
	g_g_a_a(backImage,1,6,1,1);
	
	GtkWidget* emoLab = lan(_("Emoticons:"));
	g_g_a_a(emoLab,0,7,1,1);
	comboEmot = gtk_combo_box_text_new();
	g_g_a_a(comboEmot,1,7,1,1);
	
	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	StringList files = File::findFiles(path, "*.xml");
	for(auto fi = files.begin(); fi != files.end();++fi) {
			string file = Util::getFileName((*fi));
			size_t nedle =  file.find(".");
			string text = file.substr(0,nedle);
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboEmot), text.c_str() );
	}
	string pack_name = p_entry->get(SettingsManager::EMOT_PACK,SETTING(EMOT_PACK));
	for(auto fii = files.begin(); fii!= files.end(); ++fii) {
			size_t needle = Util::getFileName(*fii).find(".");
			string tmp  = Util::getFileName(*fii).substr(0,needle);
			if(pack_name == tmp) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(comboEmot), (fii - files.begin()));
			}
		}
	enableNoti = g_c_b_n(_("Enable Notify for This Hub"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableNoti), p_entry->getNotify());
	g_g_a_a(enableNoti,0,8,1,1);
	enableLog = g_c_b_n(_("Enable Logging for This Hub"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableLog), p_entry->get(SettingsManager::LOG_CHAT_B,SETTING(LOG_CHAT_B)));
	g_g_a_a(enableLog,1,8,1,1);
	enableCountry = g_c_b_n(_("Enable Country Info in Chat"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableCountry), p_entry->get(SettingsManager::USE_COUNTRY_FLAG,SETTING(USE_COUNTRY_FLAG)));
	g_g_a_a(enableCountry,0,9,1,1);
	
	enableIp = g_c_b_n(_("Enable IP Info in Chat"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableIp), p_entry->get(SettingsManager::USE_IP,SETTING(USE_IP)));
	g_g_a_a(enableIp,1,9,1,1);
	
	enableBold = g_c_b_n(_("Enable Tab Bolding"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableBold), p_entry->get(SettingsManager::BOLD_HUB,SETTING(BOLD_HUB)));
	g_g_a_a(enableBold,0,10,1,1);
	
	enableStatusChat = g_c_b_n(_("Enable Status Chat message"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableStatusChat), p_entry->get(SettingsManager::STATUS_IN_CHAT,SETTING(STATUS_IN_CHAT)));
	g_g_a_a(enableStatusChat,1,10,1,1);
	
	enableFavFirst = g_c_b_n(_("Enable Favorite Users First in UserList"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enableFavFirst), p_entry->get(SettingsManager::SORT_FAVUSERS_FIRST,SETTING(SORT_FAVUSERS_FIRST)));
	g_g_a_a(enableFavFirst,0,11,1,1);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), boxAdvanced ,labelAdvanced );
	//
	GtkWidget* labelConn = lan(_("Connection Setup"));
	GtkWidget* boxConnection = gtk_grid_new();
	GtkWidget* labelMode = lan(_("Mode:"));
	g_g_a_c_s(labelMode,0,0,1,1);
	comboMode = createComboBoxWith3Options("Default","Active","Pasive");
	
	if(p_entry->getMode() == SETTING(INCOMING_CONNECTIONS))
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboMode), 0);
	else{
		int mode_i = p_entry->getMode();
		if(mode_i <= 1)
			gtk_combo_box_set_active(GTK_COMBO_BOX(comboMode),1);//active
		else gtk_combo_box_set_active(GTK_COMBO_BOX(comboMode),2);//pasive	
	}
	g_g_a_c_s(comboMode,1,0,1,1);
	GtkWidget* labelIp = lan(_("IP Address:"));
	g_g_a_c_s(labelIp,0,2,1,1);
	entryIp = gen;
	gtk_entry_set_text(GTK_ENTRY(entryIp), p_entry->get(SettingsManager::EXTERNAL_IP,SETTING(EXTERNAL_IP)).c_str());
	g_g_a_c_s(entryIp,1,2,1,1);
	//GtkWidget* enableIp6 = g_c_b_n("Enable IPv6");
	//g_g_a_c_s(enableIp6,0,3,1,1);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), boxConnection ,labelConn );
	
	treeView = gtk_tree_view_new();
	GtkWidget* boxKickAction = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(boxKickAction),treeView,0,0,3,3);
	///Actions
	actionView.setView(GTK_TREE_VIEW(treeView));
	actionView.insertColumn(_("Name"), G_TYPE_STRING,TreeView::STRING,100);
	actionView.insertColumn(_("Enabled"), G_TYPE_BOOLEAN, TreeView::BOOL,100);
	actionView.insertHiddenColumn("ISRAW", G_TYPE_BOOLEAN);
	actionView.insertHiddenColumn("ID", G_TYPE_INT);
	actionView.finalize();
	actionStore = gtk_tree_store_newv(actionView.getColCount(),actionView.getGTypes());
	gtk_tree_view_set_model(actionView.get(),GTK_TREE_MODEL(actionStore));
	g_object_unref(actionStore);
	actionSel = gtk_tree_view_get_selection(actionView.get());

	//	g_signal_connect(getWidget("checkbuttonEncoding"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("comboboxCharset"));
	//	g_signal_connect(getWidget("checkbuttonNick"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryNick"));
	//	g_signal_connect(getWidget("checkbuttonUserDescription"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryUserDescription"));
	g_signal_connect(actionView.getCellRenderOf(_("Enabled")), "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), boxKickAction ,lan("Kick Actions") );	
	initActions();
	//need be after all contain stuff
	gtk_widget_show_all(notebook);			
	GtkWidget* okButton = gtk_button_new_with_label(_("Ok"));
	GtkWidget* cancelButton = 	gtk_button_new_with_label(_("Cancel"));
		
	gtk_dialog_add_action_widget (GTK_DIALOG(mainDialog),
                              okButton,
                              GTK_RESPONSE_OK);
	gtk_dialog_add_action_widget (GTK_DIALOG(mainDialog),
                              cancelButton,
                              -6);                      
	gtk_widget_show(cancelButton);	
	gtk_widget_show(okButton);
}


bool FavoriteHubDialog::initDialog(UnMapIter &groups)
{
	/*	
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

		gtk_window_set_destroy_with_parent(GTK_WINDOW(getContainer()), TRUE);

		gtk_window_set_transient_for(GTK_WINDOW(getContainer()),
		GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));

		auto it = groups.find(p_entry->getGroup());
		if (it != groups.end())
		{
			GtkTreeIter iter = it->second;
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(getWidget("groupsComboBox")), &iter);
		}
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("groupsComboBox")), 0);

		// Set the override default nick checkbox
		string nick = p_entry->get(SettingsManager::NICK,SETTING(NICK));
		gboolean overrideNick = !(nick.empty() || nick == SETTING(NICK));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonNick")), overrideNick);

		// Set the override default user description checkbox
		string desc = p_entry->get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION));
		gboolean overrideUserDescription = !(desc.empty() || desc == SETTING(DESCRIPTION));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonUserDescription")), overrideUserDescription);
*/
		// Show the dialog
		gint response = gtk_dialog_run(GTK_DIALOG(mainDialog));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;

		while (response == GTK_RESPONSE_OK)
		{
			//Hub
			p_entry->setName(gtk_entry_get_text(GTK_ENTRY(entryName)));
			p_entry->setServer(gtk_entry_get_text(GTK_ENTRY(entryAddress)));
			p_entry->setHubDescription(gtk_entry_get_text(GTK_ENTRY(entryDesc)));
			p_entry->setPassword(gtk_entry_get_text(GTK_ENTRY(entryPassword)));
			
			p_entry->setGroup(Util::emptyString);
			
			p_entry->set(SettingsManager::CHAT_EXTRA_INFO ,gtk_entry_get_text(GTK_ENTRY(extraChatInfoEntry)));
			p_entry->set(SettingsManager::EXTERNAL_IP, gtk_entry_get_text(GTK_ENTRY(entryIp)));
			p_entry->setProtectUsers(gtk_entry_get_text(GTK_ENTRY(entryProtectedUser)));
			p_entry->setNotify(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( enableNoti)));
			
			p_entry->setMode(gtk_combo_box_get_active(GTK_COMBO_BOX(comboMode)));
			
			p_entry->setAutoConnect(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkAutoConnect)));
			p_entry->set(SettingsManager::LOG_CHAT_B, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enableLog)));
			p_entry->setHideShare(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkHideShare)));
			p_entry->setCheckAtConn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkOnConn)));
			p_entry->setCheckFilelists(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkFilelists)));
			p_entry->setCheckClients(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkClients)));
			
			p_entry->set(SettingsManager::USE_IP ,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enableIp)));
			p_entry->set(SettingsManager::BOLD_HUB ,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enableBold)));
			p_entry->set(SettingsManager::GET_USER_COUNTRY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enableCountry)));
			p_entry->set(SettingsManager::STATUS_IN_CHAT,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enableStatusChat)));
			p_entry->set(SettingsManager::SORT_FAVUSERS_FIRST,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enableFavFirst)));
			p_entry->set(SettingsManager::EMAIL, gtk_entry_get_text(GTK_ENTRY(entryMail)));
			p_entry->set(SettingsManager::SHOW_JOINS, gtk_combo_box_get_active(GTK_COMBO_BOX(comboParts)));
			p_entry->set(SettingsManager::FAV_SHOW_JOINS, gtk_combo_box_get_active(GTK_COMBO_BOX(comboFavParts)));
			p_entry->set(SettingsManager::DEFAULT_AWAY_MESSAGE, gtk_entry_get_text(GTK_ENTRY(entryAwayMessage)));
			//temp fix ( disabling IPv6 by default)
			//p_entry->set(HubSettings::Connection) = 1;
			p_entry->set(SettingsManager::EXTERNAL_IP6, Util::emptyString);
			GdkRGBA color;	
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(colorBack),
                            &color);
			
			p_entry->set(SettingsManager::BACKGROUND_CHAT_COLOR, WulforUtil::colorToString(&color) );
			
			gchar* image_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(backImage));
			
			string tmp = Util::getFileExt(string(image_path));
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), (int(*)(int))toupper);

			if(tmp == ".png" || tmp == ".jpg" || tmp == ".gif" || tmp == ".svg")//alow only these types
			{
				p_entry->set(SettingsManager::BACKGROUND_CHAT_IMAGE,string(image_path));
			}
/*
			if (gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("groupsComboBox"))) != 0)
			{
				gchar *group = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getWidget("groupsComboBox")));
				if(group) {
					p_entry->setGroup(string(group));
					g_free(group);
			   	}
			}
*//*
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonEncoding"))))
		{*/
			gchar *encoding = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(comboCodepage));
			if(encoding)
			{
				p_entry->setEncoding(string(encoding));
				g_free(encoding);
			}
//		}

		gchar *pack = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(comboEmot));
		
		if(pack)
		{
			p_entry->set(SettingsManager::EMOT_PACK,string(pack));
			g_free(pack);
		}
/*
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonNick"))))
		{*/
			p_entry->set(SettingsManager::NICK, gtk_entry_get_text(GTK_ENTRY(entryUsername)));
/*		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonUserDescription"))))
		{
*/			p_entry->set(SettingsManager::DESCRIPTION,gtk_entry_get_text(GTK_ENTRY(entryUserDescriptio)));
	/*	}
*/
		if (p_entry->getName().empty() || p_entry->getServer().empty())
		{
			if (showErrorDialog_gui(_("The name and address fields are required")))
			{
				response = gtk_dialog_run(GTK_DIALOG(mainDialog));

				// Fix crash, if the dialog gets programmatically destroyed.
				if (response == GTK_RESPONSE_NONE)
					return FALSE;
			}
			else
				return FALSE;
		}
		else
		{
			gtk_widget_hide(mainDialog);
			return TRUE;
		}
	}
	gtk_widget_hide(mainDialog);
	return FALSE;
}


bool FavoriteHubDialog::showErrorDialog_gui(const string &description)
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

void FavoriteHubDialog::onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
		GtkWidget *widget = (GtkWidget*)data;
		bool override = gtk_toggle_button_get_active(button);

		gtk_widget_set_sensitive(widget, override);

		if (override)
		{
				gtk_widget_grab_focus(widget);
		}
}

void FavoriteHubDialog::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
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

void FavoriteHubDialog::initActions()
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

