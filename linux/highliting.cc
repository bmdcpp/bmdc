#include "highliting.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

Highlighting::Highlighting():
BookEntry(Entry::HIGHL,_("Highliting Settings"),"highliting.glade")
{
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("HiglitingDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_widget_set_sensitive(getWidget("entrySoundFile"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryPopText"), FALSE);
	gtk_widget_set_sensitive(getWidget("colorbuttonbg"), FALSE);
	gtk_widget_set_sensitive(getWidget("colorbuttonfg"), FALSE);
	
	hView.setView(GTK_TREE_VIEW(getWidget("favoriteUserView")));
	hView.insertColumn(_("Name"),G_TYPE_STRING,TreeView::STRING,80);
	hView.insertColumn(_("Popup"),G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn(_("Bold"), G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn(_("Underline"), G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn(_("Italic"), G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("INCNICK", G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("BoldTab", G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("UsingREEXP", G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("BGColor", G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("FGColor", G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("NotifyStr", G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn(_("Sound"), G_TYPE_STRING, TreeView::STRING,60);
	hView.insertColumn("SoundStr", G_TYPE_STRING, TreeView::STRING,80);
	hView.finalize();
	store = gtk_list_store_newv(hView.getColCount(), hView.getGTypes());
	gtk_tree_view_set_model(hView.get(), GTK_TREE_MODEL(store));
	g_object_unref(store);
	gtk_tree_view_set_fixed_height_mode(hView.get(), TRUE);
	selection = gtk_tree_view_get_selection(hView.get());

	g_signal_connect(getWidget("buttonadd"), "clicked", G_CALLBACK(onADD), (gpointer)this);
	g_signal_connect(getWidget("buttonmod"), "clicked", G_CALLBACK(onModify), (gpointer)this);
	g_signal_connect(getWidget("buttondell"), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);

	//Dialog
	g_signal_connect(getWidget("colorbuttonfg"), "color-set", G_CALLBACK(onColorFore_clicked), (gpointer)this);
	g_signal_connect(getWidget("colorbuttonbg"), "color-set", G_CALLBACK(onColorBg_clicked), (gpointer)this);
	
	g_signal_connect(getWidget("checkSound"), "toggled",G_CALLBACK(onCheckButtonToggled_gui), getWidget("entrySoundFile"));
	g_signal_connect(getWidget("checkpopup"), "toggled",G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryPopText"));
	
	g_signal_connect(getWidget("checkbgcolor"), "toggled",G_CALLBACK(onCheckButtonToggled_gui), getWidget("colorbuttonbg"));
	g_signal_connect(getWidget("checkfccolor"), "toggled",G_CALLBACK(onCheckButtonToggled_gui), getWidget("colorbuttonfg"));
	
}

Highlighting::~Highlighting() { HighlightManager::getInstance()->replaceList(pList); }

void Highlighting::show()
{
	ColorList* cList = HighlightManager::getInstance()->getList();
	for(ColorIter i = cList->begin();i != cList->end(); ++i) {
		pList.push_back((*i));
		StringMap params;
		get_ColorParams(&(*i),params);
		addEntry_gui(params);
	}

}

void Highlighting::addEntry_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	editEntry_gui(params, &iter);
}

void Highlighting::editEntry_gui(StringMap &params, GtkTreeIter *iter)
{
	gtk_list_store_set(store, iter,
		hView.col(_("Name")), params["Name"].c_str(),
		hView.col(_("Popup")), params["POPUP"].c_str(),
		hView.col(_("Bold")), params["Bold"].c_str(),
		hView.col(_("Underline")), params["Underline"].c_str(),
		hView.col(_("Italic")), params["Italic"].c_str(),
		hView.col("INCNICK"), params["INCNICK"].c_str(),
		hView.col("BoldTab"), params["Tab"].c_str(),
		hView.col("UsingREEXP"), params["EXP"].c_str(),
		hView.col("BGColor"), params["BGColor"].c_str(),
		hView.col("FGColor"), params["FGColor"].c_str(),
		hView.col("NotifyStr"), params["Noti"].c_str(),
		hView.col(_("Sound")), params["Sound"].c_str(),
		hView.col("SoundStr"), params["SoundF"].c_str(),
		-1);
}

void Highlighting::removeEntry_gui(string name)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(store);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (hView.getString(&iter, _("Name")) == name)
		{
			gtk_list_store_remove(store, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Highlighting::get_ColorParams(dcpp::ColorSettings *cs,dcpp::StringMap &params)
{

	params["Name"] = cs->getMatch();
	params["POPUP"] = cs->getPopup() ? "1" : "0";
	params["Bold"] = cs->getBold() ? "1" : "0";
	params["Underline"] = cs->getUnderline() ? "1" : "0";
	params["EXP"] = cs->usingRegexp() ? "1" : "0";
	params["Italic"] = cs->getItalic() ? "1" : "0";
	params["INCNICK"] = cs->getIncludeNick() ? "1" : "0";
	params["Tab"] = cs->getTab() ? "1" : "0";
	params["Noti"] = cs->getNoti();
	params["Sound"] = cs->getPlaySound() ? "1" : "0";
	params["SoundF"] = cs->getSoundFile();

	if(cs->getHasBgColor())
		params["BGColor"] = cs->getBgColor();
	else
		params["BGColor"] = "#FFFFFF";

	if(cs->getHasFgColor())
		params["FGColor"] = cs->getFgColor();
	else
		params["FGColor"] = "#000000";
}

void Highlighting::onADD(GtkWidget *widget , gpointer data)
{
	Highlighting *hg = (Highlighting *)data;
	
	StringMap params;
	params["Name"] = "Default String";
	params["POPUP"] = "0";
	params["Bold"] = "0";
	params["Underline"] = "0";
	params["Italic"] = "0";
	params["EXP"] = "0";
	params["INCNICK"] = "0";
	params["Tab"] = "0";
	params["BGColor"]="#FFFFFF";
	params["FGColor"]="#000000";
	params["Noti"] = "Example";
	params["Sound"] = "0";
	params["SoundF"] = "Example.wav";

	bool isOk = hg->showColorDialog(params);
	if(isOk)
	{
		hg->addEntry_gui(params);
		typedef Func1<Highlighting, StringMap> F1;
		F1 *func = new F1(hg, &Highlighting::addHigl_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Highlighting::onModify(GtkWidget *widget, gpointer data)
{
	Highlighting *hg = (Highlighting *)data;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(hg->selection, NULL, &iter))
		return;
	StringMap params;
	params["Name"] = hg->hView.getString(&iter, _("Name"));
	params["POPUP"] = hg->hView.getString(&iter, _("Popup"));
	params["Bold"] = hg->hView.getString(&iter, _("Bold"));
	params["Underline"] = hg->hView.getString(&iter, _("Underline"));
	params["Italic"] = hg->hView.getString(&iter, _("Italic"));
	params["INCNICK"] = hg->hView.getString(&iter, "INCNICK");
	params["Tab"] = hg->hView.getString(&iter, "BoldTab");
	params["FGColor"] = hg->hView.getString(&iter, "FGColor");
	params["BGColor"] = hg->hView.getString(&iter, "BGColor");
	params["Noti"] = hg->hView.getString(&iter, "NotifyStr");
	params["Sound"] = hg->hView.getString(&iter, _("Sound"));
	params["SoundF"] = hg->hView.getString(&iter, "SoundStr");

    bool isOk = hg->showColorDialog(params);
	if(isOk)
	{
		string name = hg->hView.getString(&iter, _("Name"));
		hg->editEntry_gui(params,&iter);
		
		typedef Func2<Highlighting, StringMap, string> F2;
		F2 *func = new F2(hg, &Highlighting::editHigl_client, params,name);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Highlighting::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
	Highlighting *hg = (Highlighting *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(hg->selection, NULL, &iter))
	{
		if (BOOLSETTING(CONFIRM_HUB_REMOVAL))
		{
			string name = hg->hView.getString(&iter, _("Name"));
			GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete Higliting \"%s\"?"), name.c_str());
			gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
			gint response = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			if (response != GTK_RESPONSE_YES)
				return;
		}

		string address = hg->hView.getString(&iter, _("Name"));

		typedef Func1<Highlighting, string> F1;
		F1 *func = new F1(hg, &Highlighting::removeEntry_client, address);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

bool Highlighting::showColorDialog(StringMap &params)
{
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrystring")),params["Name"].c_str());

	// Set the Bold checkbox
	gboolean isBold = params["Bold"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbold")), isBold);
	// Set the Popup checkbox
	gboolean isPopup = params["POPUP"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkpopup")), isPopup);
	// Set Italic checkbox
	gboolean isItalic = params["Italic"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkitalic")), isItalic);
	// Set is Nick/User 
	gboolean isNick = params["INCNICK"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkNick")), isNick);
	//set Bold Tab
	gboolean isTab = params["Tab"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkTab")), isTab);

	gboolean isUnder = params["Underline"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkUnder")), isUnder);

	gboolean isCase = params["Case"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkCase")), isCase);

	gboolean isSound = params["Sound"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkSound")), isSound);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrySoundFile")),params["SoundF"].c_str());

    /*Set The BG //FG colors*/
	GdkColor clr;
	gdk_color_parse(params["FGColor"].c_str(),&clr);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(getWidget("colorbuttonfg")),&clr);//
	gtk_widget_modify_base (getWidget("colorbuttonfg"), GTK_STATE_NORMAL, &clr);
    gtk_widget_modify_fg (getWidget("colorbuttonfg"), GTK_STATE_NORMAL, &clr);


	GdkColor colr;
	gdk_color_parse(params["BGColor"].c_str(),&colr);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(getWidget("colorbuttonbg")),&colr);//
	gtk_widget_modify_base (getWidget("colorbuttonbg"), GTK_STATE_NORMAL, &colr);
    gtk_widget_modify_fg (getWidget("colorbuttonbg"), GTK_STATE_NORMAL, &colr);

	gtk_entry_set_text(GTK_ENTRY(getWidget("entryPopText")),params["Noti"].c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(getWidget("HiglitingDialog")));

	while (response == GTK_RESPONSE_OK)
	{
		params.clear();
		params["Name"]=gtk_entry_get_text(GTK_ENTRY(getWidget("entrystring")));
		params["Bold"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbold"))) ? "1" : "0";
		params["POPUP"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkpopup"))) ? "1" : "0";
		params["Italic"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkitalic"))) ? "1" : "0";
		params["INCNICK"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkNick"))) ? "1" : "0";
		params["Tab"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkTab"))) ? "1" : "0";
		params["Underline"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkUnder"))) ? "1" : "0";
		params["Case"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkCase"))) ? "1" : "0";
		params["Noti"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryPopText")));
		params["Sound"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkSound"))) ? "1" : "0";
		params["SoundF"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entrySoundFile")));

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkfccolor"))))
            params["FGColor"] = colors.fgcolor;
        if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget("checkbgcolor"))))
            params["BGColor"] = colors.bgcolor;

		if(params["Name"].empty())
			response = gtk_dialog_run(GTK_DIALOG(getWidget("HiglitingDialog")));
		else
		{
			gtk_widget_hide(getWidget("HiglitingDialog"));
			return TRUE;
		}
	}
}

void Highlighting::onColorFore_clicked(GtkWidget *widget,gpointer data)
{
	Highlighting *hg = (Highlighting *)data;
	GdkColor _fgcolor;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&_fgcolor);
	string fg;
	fg = WulforUtil::colorToString(&_fgcolor);
	hg->colors.fgcolor = fg;
}

void Highlighting::onColorBg_clicked(GtkWidget *widget,gpointer data)
{
	Highlighting *hg = (Highlighting *)data;
	GdkColor _bgcolor;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&_bgcolor);
	string bg;
	bg = WulforUtil::colorToString(&_bgcolor);
	hg->colors.bgcolor = bg;
}

void Highlighting::onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
	GtkWidget *widget = (GtkWidget*)data;
	bool override = gtk_toggle_button_get_active(button);

	gtk_widget_set_sensitive(widget, override);

	if (override)
	{
		gtk_widget_grab_focus(widget);
	}
}

void Highlighting::addHigl_client(StringMap params)
{
	ColorSettings cs;
	cs.setMatch(params["Name"]);
	cs.setPopup(Util::toInt(params["POPUP"]));
	cs.setBold(Util::toInt(params["Bold"]));
	cs.setUnderline(Util::toInt(params["Underline"]));
	cs.setItalic(Util::toInt(params["Italic"]));
	cs.setUnderline(Util::toInt(params["Underline"]));
	if(!colors.bgcolor.empty())
	{
		cs.setHasBgColor(true);
		cs.setBgColor(colors.bgcolor);
	}
	if(!colors.fgcolor.empty())
	{
		cs.setHasFgColor(true);
		cs.setFgColor(colors.fgcolor);
	}

	cs.setIncludeNick(Util::toInt(params["INCNICK"]));
	cs.setCaseSensitive(Util::toInt(params["Case"]));
	cs.setTab(Util::toInt(params["Tab"]));
	cs.setNoti(params["Noti"]);
	cs.setPlaySound(Util::toInt(params["Sound"]));
	cs.setSoundFile(params["SoundF"]);

	pList.push_back(cs);
}

void Highlighting::editHigl_client(StringMap params,string name)
{
	ColorSettings * cs= new ColorSettings();
	cs->setMatch(params["Name"]);
	cs->setPopup(Util::toInt(params["POPUP"]));
	cs->setBold(Util::toInt(params["Bold"]));
	cs->setUnderline(Util::toInt(params["Underline"]));
	cs->setItalic(Util::toInt(params["Italic"]));
	cs->setUnderline(Util::toInt(params["Underline"]));
	cs->setTab(Util::toInt(params["Tab"]));
	if(!colors.bgcolor.empty())
	{
		cs->setHasBgColor(true);
		cs->setBgColor(colors.bgcolor);
	}
	if(!colors.fgcolor.empty())
	{
		cs->setHasFgColor(true);
		cs->setFgColor(colors.fgcolor);
	}

	cs->setIncludeNick(Util::toInt(params["INCNICK"]));
	cs->setCaseSensitive(Util::toInt(params["Case"]));
	cs->setNoti(params["Noti"]);

	cs->setPlaySound(Util::toInt(params["Sound"]));
	cs->setSoundFile(params["SoundF"]);


	for(int i=0;i < pList.size(); i++)
	{
			if(pList[i].getMatch() == name)
			{
				pList.erase(pList.begin()+i);
				pList.insert(pList.begin()+i,*cs);
			}
	}
}

void Highlighting::removeEntry_client(string name)
{

    for(int i=0;i < pList.size() ;i++)
    {
		if(pList[i].getMatch()==name)
	    	pList.erase(pList.begin() + i);
	}

	typedef Func1<Highlighting, string> F1;
	F1 *func = new F1(this, &Highlighting::removeEntry_gui,name);
	WulforManager::get()->dispatchClientFunc(func);

}

/*this is a generic pop menu*/
void Highlighting::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()),closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate",G_CALLBACK(onCloseItem),this);

}

void Highlighting::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}
