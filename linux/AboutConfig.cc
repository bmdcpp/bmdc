/*
	Copyright (C) 2011 - 2017 - BMDC++
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301, USA.
*/

#include "../dcpp/stdinc.h"
#include "../dcpp/SettingsManager.h"
#include "AboutConfig.hh"
#include "WulforUtil.hh"
#include "settingsmanager.hh"
#include "treeview.hh"

using namespace std;
using namespace dcpp;

AboutConfig::AboutConfig():
BookEntry(Entry::ABOUT_CONFIG, _("About:config"), "config")
{
	aboutView.setView(GTK_TREE_VIEW(getWidget("aboutTree")));
	aboutView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 120);
	aboutView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertColumn(_("Type"), G_TYPE_STRING, TreeView::STRING, 60);
	aboutView.insertColumn(_("Value"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertHiddenColumn("WS", G_TYPE_BOOLEAN);
	aboutView.insertHiddenColumn("ForeColor", G_TYPE_STRING);
	aboutView.finalize();
	aboutStore = gtk_list_store_newv(aboutView.getColCount(), aboutView.getGTypes());
	gtk_tree_view_set_model(aboutView.get(), GTK_TREE_MODEL(aboutStore));
	string sort = _("Type");
	aboutView.setSortColumn_gui(_("Name"), sort);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(aboutStore), aboutView.col(sort), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(aboutView.get(), aboutView.col(_("Name"))), TRUE);
	g_object_unref(aboutStore);

	aboutSelection = gtk_tree_view_get_selection(aboutView.get());

	g_signal_connect(aboutView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(aboutView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(aboutView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("propteriesItem"), "activate", G_CALLBACK(onPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("DefaultItem"), "activate", G_CALLBACK(onSetDefault), (gpointer)this);

	if(SETTING(AC_DISCLAIM) == false) {
			gtk_widget_set_sensitive(getWidget("scrolledwindow"),FALSE);
			gtk_dialog_run(GTK_DIALOG(getWidget("infobar")));//@ need show this dialog
	}
	if(SETTING(AC_DISCLAIM) == true) {// we already confrim editing and so on
		gtk_widget_set_sensitive(getWidget("scrolledwindow"),TRUE);
		gtk_widget_hide(getWidget("infobar"));
	}

	g_signal_connect(GTK_INFO_BAR(getWidget("infobar")),
                            "response",
                            G_CALLBACK (onInfoResponse),
                            (gpointer)this);
    setColorsRows();
}

void AboutConfig::setColorsRows()
{
	setColorRow(_("Name"));
	setColorRow(_("Status"));
	setColorRow(_("Type"));
	setColorRow(_("Value"));
}

void AboutConfig::setColorRow(string cell)
{

	if(aboutView.getCellRenderOf(cell) != NULL)
		gtk_tree_view_column_set_cell_data_func(aboutView.getColumn(cell),
								aboutView.getCellRenderOf(cell),
								AboutConfig::makeColor,
								(gpointer)this,
								NULL);
}


void AboutConfig::makeColor(GtkTreeViewColumn *column,GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	AboutConfig* pf = (AboutConfig*)data;
	if(pf == NULL) return;if(model == NULL) return;if(iter == NULL)return;
	if(column == NULL)return;if(cell == NULL) return;
	//string bcolor = f->aboutView.getString(iter,"BackColor",model);
	string fcolor = pf->aboutView.getString(iter,"ForeColor",model);
	//g_object_set(cell,"cell-background-set",TRUE,"cell-background",bcolor.c_str(),NULL);
	g_object_set(cell,"foreground-set",TRUE,"foreground",fcolor.c_str(),NULL);

}


AboutConfig::~AboutConfig()
{

}

void AboutConfig::show()
{
	SettingsManager* sm = SettingsManager::getInstance();

	SettingsManager::Types type;
	const gchar* rowname = NULL;
	const gchar* isdefault = _("Default");
	gchar types[10];
	const gchar* value = NULL;

	for(int n = 0; n < SettingsManager::SETTINGS_LAST-1; n++ ) {
		const gchar* tmp = (sm->getSettingTags()[n].c_str());
		if (strncasecmp(tmp,"SENTRY",7) == 0) continue;
		if (sm->getType(tmp, n, type)) {
			rowname = tmp;
			isdefault = _("Default");
			value = NULL;
			bool bIsDef = true;
			switch(type) {
				case SettingsManager::TYPE_STRING:
				{
					sprintf(types,"String");
					value = g_strdup(sm->get(static_cast<SettingsManager::StrSetting>(n)).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::StrSetting>(n))) {
						isdefault = _("User set");
						bIsDef = false;
					}
					addItem_gui(rowname, isdefault, types, value,FALSE,bIsDef);
					continue;
				}
				case SettingsManager::TYPE_INT:
				{
					sprintf(types,"Integer");
					value = g_strdup(Util::toString((int)sm->get(static_cast<SettingsManager::IntSetting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::IntSetting>(n))){
						isdefault = _("User set");
						bIsDef = false;
					}
					addItem_gui(rowname, isdefault, types, value,FALSE,bIsDef);
					continue;
				}
				case SettingsManager::TYPE_INT64:
				{
					sprintf(types,"Int64");
					value = g_strdup(Util::toString((int64_t)sm->get(static_cast<SettingsManager::Int64Setting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::Int64Setting>(n))){
						isdefault = _("User set");
						bIsDef = false;
					}
					addItem_gui(rowname, isdefault, types, value,FALSE,bIsDef);
					continue;
				}
				case SettingsManager::TYPE_FLOAT:
				{
					sprintf(types,"Float");
					value = g_strdup(Util::toString((float)sm->get(static_cast<SettingsManager::FloatSetting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::FloatSetting>(n))){
						isdefault = _("User set");
						bIsDef = false;
					}
					addItem_gui(rowname, isdefault, types, value,FALSE,bIsDef);
					continue;
				}
				case SettingsManager::TYPE_BOOL:
				{
					sprintf(types,"Bool");
					value = g_strdup(Util::toString((int)sm->get(static_cast<SettingsManager::BoolSetting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::BoolSetting>(n))) {
					 isdefault = _("User set");
					 bIsDef = false;
					}
					addItem_gui(rowname, isdefault, types, value,FALSE,bIsDef);
					continue;
				}
				default:
					dcassert(0);break;break;
			}
		}
	}

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	WulforSettingsManager::StringMap map = wsm->getStringMap();
	WulforSettingsManager::StringMap defaultStringMap = wsm->getStringDMap();
	const gchar* dValue = NULL;
	sprintf(types,"String");
	bool bIsOk = false;
	gchar* sDefualt = _("Default");

	for(auto i = defaultStringMap.begin();i!= defaultStringMap.end();++i)
	{
		bIsOk = map.find(rowname) != map.end();
		sDefualt = bIsOk ? _("User set") : _("Default");
		rowname = i->first.c_str();
		dValue = g_strdup(i->second.c_str());
		value = g_strdup( (bIsOk ? map.find(rowname)->second : Util::emptyString).c_str());
		addItem_gui(rowname, sDefualt, types, ( !bIsOk ? dValue : value), TRUE,bIsOk);
	}

	WulforSettingsManager::IntMap imap = wsm->getIntMap();
	WulforSettingsManager::IntMap defIMap = wsm->getIntDMap();
	sprintf(types,"Integer");
	bIsOk = false;
	sDefualt = _("Default");
	for(auto j = defIMap.begin();j != defIMap.end();++j)
	{
		bIsOk = imap.find(rowname) != imap.end();
		sDefualt = bIsOk ? _("User set") : _("Default");
		rowname = j->first.c_str();
		dValue = Util::toString(j->second).c_str();
		value = g_strdup(Util::toString((bIsOk ? imap.find(rowname)->second : 0)).c_str());

		addItem_gui(rowname, sDefualt, types, ( !bIsOk ? dValue : value), TRUE,bIsOk);
	}

}

void AboutConfig::addItem_gui(const gchar* rowname, const gchar* sDefault, const gchar* types, const gchar* value, gboolean bIsWulfor,bool bISDefualt)
{
	GtkTreeIter iter;
	g_print("\n%s-%s-%s-%s-%d ",rowname,sDefault,types,value,(int)bIsWulfor);
	if(value == NULL)return;
	gboolean bIsOk = g_utf8_validate(value,-1,NULL);
	gboolean bIsOk2 = g_utf8_validate(rowname,-1,NULL);
	gboolean bIsOk3 = g_utf8_validate(sDefault,-1,NULL);
	gboolean bIsOk4 = g_utf8_validate(types,-1,NULL);
	if(!bIsOk) {
		dcdebug("value\n");
	}
	if(!bIsOk2) {
		dcdebug("rowname\n");
	}
	if(!bIsOk3) {
		dcdebug("isdef\n");
	}
	if(!bIsOk4) {
		dcdebug("types\n");
	}

	gtk_list_store_append(aboutStore,&iter);
	gtk_list_store_set(aboutStore,&iter,
				aboutView.col(_("Name")),rowname,
				aboutView.col(_("Status")), sDefault,
				aboutView.col(_("Type")), types,
				aboutView.col(_("Value")), value,
				aboutView.col("WS"), bIsWulfor,
				aboutView.col("ForeColor"), bISDefualt ? "#000000" : "#FF0000",
	-1);

}

void AboutConfig::updateItem_gui(const string sRowName,const string sValue, GtkTreeIter *iter,const gchar* sStatus,gboolean bWulf /*false*/ )
{
	if(iter) {
		gtk_list_store_set(aboutStore,iter,
				aboutView.col(_("Name")),sRowName.c_str(),
				aboutView.col(_("Status")), sStatus,
				aboutView.col(_("Value")), sValue.c_str(),
				aboutView.col("WS"), bWulf,
		-1);
	}
}

void AboutConfig::setStatus(const string smsg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, smsg.c_str());
}

gboolean AboutConfig::onButtonPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	AboutConfig *ps = (AboutConfig *)data;
	ps->previous = event->type;
	return FALSE;
}

gboolean AboutConfig::onButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	AboutConfig *ps = (AboutConfig *)data;

	if (gtk_tree_selection_get_selected(ps->aboutSelection, NULL, NULL))
	{
		if (event->button == 1 && ps->previous == GDK_2BUTTON_PRESS)
		{
			// show dialog
			onPropertiesClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			// show menu
			#if GTK_CHECK_VERSION(3,22,0)
			gtk_menu_popup_at_pointer(GTK_MENU(ps->getWidget("menu")),NULL);
			#else
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
			#endif
		}
	}

	return FALSE;
}

gboolean AboutConfig::onKeyReleased_gui(GtkWidget* , GdkEventKey *event, gpointer data)
{
	AboutConfig *ps = (AboutConfig *)data;

	if (gtk_tree_selection_get_selected(ps->aboutSelection, NULL, NULL))
	{
		if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
		{
			#if GTK_CHECK_VERSION(3,22,0)
			gtk_menu_popup_at_pointer(GTK_MENU(ps->getWidget("menu")),NULL);
			#else
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
			#endif
		}
	}

	return FALSE;
}

void AboutConfig::onInfoResponse(GtkWidget *info_bar, gint response_id,  gpointer data)
{
	AboutConfig *ps = (AboutConfig *)data;

	switch(response_id)
	{
		//alowing
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_ACCEPT:
		{
			gtk_widget_hide(info_bar);
			gtk_widget_set_sensitive(ps->getWidget("scrolledwindow"),TRUE);
			SettingsManager::getInstance()->set(static_cast<SettingsManager::BoolSetting>(SettingsManager::AC_DISCLAIM), true);
			break;
		}
		//not allowing
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_REJECT:
		{
			gtk_widget_set_sensitive(ps->getWidget("scrolledwindow"),FALSE);
			gtk_widget_hide(info_bar);
			SettingsManager::getInstance()->set(static_cast<SettingsManager::BoolSetting>(SettingsManager::AC_DISCLAIM), false);

			break;
		}
		default:
			break;
	}
	SettingsManager::getInstance()->save();

}

void AboutConfig::onPropertiesClicked_gui(GtkWidget*, gpointer data)
{
	AboutConfig *ps = (AboutConfig *)data;

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(ps->aboutSelection, NULL, &iter))
	{
		string sName = ps->aboutView.getString(&iter,_("Name"));
		string sValue = ps->aboutView.getString(&iter, _("Value"));
		gboolean bisWulfor = ps->aboutView.getValue<gboolean>(&iter, "WS");

		bool brun = ps->getDialog(sName, sValue);
		if(!brun)
			return;

		if(bisWulfor)
		{
			WulforSettingsManager* wsm = WulforSettingsManager::getInstance();

			if(wsm->isString(sName))
				wsm->set(sName,sValue);

			if(wsm->isInt(sName))
				wsm->set(sName,Util::toInt(sValue));

			ps->updateItem_gui(sName,sValue,&iter,_("User set"),TRUE);
			return;
		}
		int n = -1;
		SettingsManager *sm = SettingsManager::getInstance();
		SettingsManager::Types type;
		sm->getType(sName.c_str(), n, type);

		switch(type)
		{
			case SettingsManager::TYPE_STRING:
				sm->set((SettingsManager::StrSetting)n,sValue);
				break;
			case SettingsManager::TYPE_INT:
				sm->set((SettingsManager::IntSetting)n,Util::toInt(sValue));
				break;
			case SettingsManager::TYPE_INT64:
				sm->set((SettingsManager::Int64Setting)n, Util::toInt64(sValue));
				break;
			case SettingsManager::TYPE_FLOAT:
				sm->set((SettingsManager::FloatSetting)n,Util::toFloat(sValue));
				break;
			case SettingsManager::TYPE_BOOL:
				sm->set((SettingsManager::BoolSetting)n, Util::toInt(sValue));
				break;
			default: return;
		}
		ps->updateItem_gui(sName,sValue,&iter,_("User Set"),FALSE);
	}
}

void AboutConfig::onSetDefault(GtkWidget*, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string name = s->aboutView.getString(&iter,_("Name"));
		gboolean isWulfor = s->aboutView.getValue<gboolean>(&iter, "WS");

		if(isWulfor)
		{
			WulforSettingsManager* wsm = WulforSettingsManager::getInstance();
			string value = Util::emptyString;

			if(wsm->isString(name)) {
				wsm->SetStringDef(name);
				value = wsm->getString(name);
			}
			if(wsm->isInt(name)) {
				wsm->SetIntDef(name);
				value = Util::toString(wsm->getInt(name));
			}
			s->updateItem_gui(name,value,&iter,_("Default"),TRUE);
			s->setStatus("Value "+name+" Setted to Default "+value);
			return;
		}

		SettingsManager *sm = SettingsManager::getInstance();
		int n = -1 ;
		SettingsManager::Types type;

		if (sm->getType(name.c_str(), n, type))
		{
			sm->unset(n);

			string value = Util::emptyString;

			switch(type) {
				case SettingsManager::TYPE_STRING:
					value = Text::toT(sm->get(static_cast<SettingsManager::StrSetting>(n)));
					break;
				case SettingsManager::TYPE_INT:
					value = Text::toT(Util::toString(sm->get(static_cast<SettingsManager::IntSetting>(n))));
					break;
				case SettingsManager::TYPE_INT64:
					value = Text::toT(Util::toString(sm->get(static_cast<SettingsManager::Int64Setting>(n))));
					break;
				case SettingsManager::TYPE_FLOAT:
					value = Text::toT(Util::toString(sm->get(static_cast<SettingsManager::FloatSetting>(n))));
					break;
				case SettingsManager::TYPE_BOOL:
					value = Text::toT(Util::toString((int)sm->get(static_cast<SettingsManager::BoolSetting>(n))));
					break;
				default:
					return;
			}
			s->updateItem_gui(name, value,&iter);
			s->setStatus("Value" + name + "Setted to Default" + value);
		}
	}
}

bool AboutConfig::getDialog(const string name, string& value)
{
	WulforSettingsManager* wsm = WulforSettingsManager::getInstance();
	SettingsManager* sm = SettingsManager::getInstance();
	int t = -1;
	if(wsm->isInt(name))
		t = TYPE_INT;

	if(wsm->isString(name))
		t = TYPE_STRING;

	int n = -1;
	SettingsManager::Types type;
	if(sm->getType(name.c_str(), n, type))
	{
		switch(type)
		{
			case SettingsManager::TYPE_BOOL:
			t = TYPE_BOOL;
			break;
			case SettingsManager::TYPE_FLOAT:
			case SettingsManager::TYPE_INT:
			case SettingsManager::TYPE_INT64:
			t = TYPE_INT;
			break;
			case SettingsManager::TYPE_STRING:
			t = TYPE_STRING;
			break;
			default:break;
		}

	}

	GtkDialog* dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(name.c_str(),
	NULL,(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT)
	,_("_OK"),GTK_RESPONSE_ACCEPT,
	_("_Cancel"),GTK_RESPONSE_REJECT,NULL));
	GtkWidget* box= gtk_dialog_get_content_area(dialog);
	GtkWidget* item;
	switch(t)
	{
		case TYPE_BOOL:
		{
			item = gtk_switch_new();
			gtk_switch_set_active(GTK_SWITCH(item),(gboolean)Util::toInt(value));
			break;
		}
		case TYPE_INT:
		{
			item = gtk_spin_button_new_with_range(-10,10000,1);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(item),(gint)Util::toInt(value));
			break;
		}
		case TYPE_STRING:
		{
			item = gtk_entry_new();
			gtk_entry_set_text(GTK_ENTRY(item),value.c_str());
			break;
		}
		default:
			item = gtk_label_new("NO GO ZONE");
	}
	GtkWidget* grip = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(grip),gtk_label_new(name.c_str()));
	gtk_container_add(GTK_CONTAINER(grip),item);
	gtk_container_add(GTK_CONTAINER(box),grip);
	gtk_widget_show_all(grip);

	int response = gtk_dialog_run(dialog);

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return false;

	gtk_widget_hide(GTK_WIDGET(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		switch(t)
		{
			case TYPE_BOOL:
			{
				bool val = gtk_switch_get_active(GTK_SWITCH(item));
				value = Util::toString(val);
				break;
			}
			case TYPE_INT:
			{
				int val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(item));
				value = Util::toString(val);
				break;
			}
			case TYPE_STRING:
			{
				value = gtk_entry_get_text(GTK_ENTRY(item));
			}
			default:break;
		}
	return true;
	}
	return false;
}
