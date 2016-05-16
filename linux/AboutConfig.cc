//
//		Copyright (C) 2011 - 2016 - BMDC
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include <dcpp/stdinc.h>
#include <dcpp/SettingsManager.h>
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
			gtk_dialog_run(GTK_DIALOG(getWidget("infobar")));//@we need show this dialog
	}
	if(SETTING(AC_DISCLAIM) == true) {// we already confrim editing and so on
		gtk_widget_set_sensitive(getWidget("scrolledwindow"),TRUE);
		gtk_widget_hide(getWidget("infobar"));
	}

	g_signal_connect(GTK_INFO_BAR(getWidget("infobar")),
                            "response",
                            G_CALLBACK (onInfoResponse),
                            (gpointer)this);
}

AboutConfig::~AboutConfig()
{
	SettingsManager::getInstance()->removeListener(this);
}

void AboutConfig::show()
{
	SettingsManager* sm = SettingsManager::getInstance();
	sm->addListener(this);

	SettingsManager::Types type;
	const gchar* rowname = NULL;
	const gchar* isdefault = _("Default");
	gchar types[10];
	const gchar* value = NULL;

	for(int n = 0; n < SettingsManager::SETTINGS_LAST; n++ ) {
		const gchar* tmp = (sm->getSettingTags()[n].c_str());
		if (strncasecmp(tmp,"SENTRY",7) == 0) continue;
		if (sm->getType(tmp, n, type)) {
			rowname = tmp;
			isdefault = _("Default");
			value = NULL;
			switch(type) {
				case SettingsManager::TYPE_STRING:
				{
					sprintf(types,"String");
					value = g_strdup(sm->get(static_cast<SettingsManager::StrSetting>(n)).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::StrSetting>(n))) { 
						isdefault = _("User set");
					}
					addItem_gui(rowname, isdefault, types, value,FALSE);
					continue;
				}	
				case SettingsManager::TYPE_INT:
				{
					sprintf(types,"Integer");
					value = g_strdup(Util::toString((int)sm->get(static_cast<SettingsManager::IntSetting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::IntSetting>(n))){
						isdefault = _("User set");
					}
					addItem_gui(rowname, isdefault, types, value,FALSE);	
					continue;
				}	
				case SettingsManager::TYPE_INT64:
				{
					sprintf(types,"Int64");
					value = g_strdup(Util::toString((int64_t)sm->get(static_cast<SettingsManager::Int64Setting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::Int64Setting>(n))){
						isdefault = _("User set");
					}
					addItem_gui(rowname, isdefault, types, value,FALSE);
					continue;
				}	
				case SettingsManager::TYPE_FLOAT:
				{
					sprintf(types,"Float");
					value = g_strdup(Util::toString((float)sm->get(static_cast<SettingsManager::FloatSetting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::FloatSetting>(n))){
						isdefault = _("User set");
					}
					addItem_gui(rowname, isdefault, types, value,FALSE);	
					continue;
				}	
				case SettingsManager::TYPE_BOOL:
				{
					sprintf(types,"Bool");
					value = g_strdup(Util::toString((int)sm->get(static_cast<SettingsManager::BoolSetting>(n))).c_str());
					if(!sm->isDefault(static_cast<SettingsManager::BoolSetting>(n))) {
					 isdefault = _("User set");
					}
					addItem_gui(rowname, isdefault, types, value,FALSE);
					continue;
				}	
				default:
					dcassert(0);break;break;
			}
		}
	}

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	WulforSettingsManager::StringMap map = wsm->getStringMap();
	WulforSettingsManager::StringMap defMap = wsm->getStringDMap();
	const gchar* dvalue = NULL;
	sprintf(types,"String");
	bool isOk = false;
	gchar* isDef = _("Default");

	for(auto d = defMap.begin();d!= defMap.end();++d)
	{
		isDef = _("Default");
		rowname = d->first.c_str();
		dvalue = d->second.c_str();
		isOk = map.find(rowname) != map.end();
		value = (isOk ? map.find(rowname)->second : Util::emptyString).c_str();
		if(isOk) isDef = _("User set");
		addItem_gui(rowname,isDef, types, ( !isOk ? dvalue : value), TRUE);
	}

	WulforSettingsManager::IntMap imap = wsm->getIntMap();
	WulforSettingsManager::IntMap defIMap = wsm->getIntDMap();
	sprintf(types,"Integer");
	isOk = false;
	isDef = _("Default");

	for(auto j = defIMap.begin();j != defIMap.end();++j) 
	{
		isDef = _("Default");
		rowname = j->first.c_str();
		dvalue = Util::toString(j->second).c_str();
		isOk = imap.find(rowname) != imap.end();
		value = Util::toString((isOk ? imap.find(rowname)->second : 0)).c_str();
		
		if(isOk) isDef = _("User set");
		addItem_gui(rowname, isDef, types, ( !isOk ? dvalue : value), TRUE);
	}

}

void AboutConfig::addItem_gui(const gchar* rowname, const gchar* isdefault, const gchar* types, const gchar* value, gboolean isWulf)
{
	GtkTreeIter iter;
	g_print("\n%s-%s-%s-%s-%d ",rowname,isdefault,types,value,(int)isWulf);
	gboolean isOk = g_utf8_validate(value,-1,NULL);
	gboolean isOk2 = g_utf8_validate(rowname,-1,NULL);
	gboolean isOk3 = g_utf8_validate(isdefault,-1,NULL);
	gboolean isOk4 = g_utf8_validate(types,-1,NULL);
	if(!isOk) {
		dcdebug("value\n");
		dcassert("bad\n");
	}
	if(!isOk2) {
		dcdebug("rowname\n");
	}
	if(!isOk3) {
		dcdebug("isdef\n");
	}
	if(!isOk4) {
		dcdebug("types\n");
	}

	gtk_list_store_append(aboutStore,&iter);
	gtk_list_store_set(aboutStore,&iter,
				aboutView.col(_("Name")),rowname,
				aboutView.col(_("Status")), isdefault,
				aboutView.col(_("Type")), types,
				aboutView.col(_("Value")), value,
				aboutView.col("WS"), isWulf,
	-1);

}

void AboutConfig::updateItem_gui(const string rowname,const string value, GtkTreeIter *iter,const gchar* status,gboolean wul /*false*/ )
{
	if(iter) {
		gtk_list_store_set(aboutStore,iter,
				aboutView.col(_("Name")),rowname.c_str(),
				aboutView.col(_("Status")), status,
				aboutView.col(_("Value")), value.c_str(),
				aboutView.col("WS"), wul,
		-1);
	}
}

void AboutConfig::setStatus(const string msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, msg.c_str());
}

gboolean AboutConfig::onButtonPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;
	s->previous = event->type;
	return FALSE;
}

gboolean AboutConfig::onButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, NULL))
	{
		if (event->button == 1 && s->previous == GDK_2BUTTON_PRESS)
		{
			// show dialog
			onPropertiesClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			// show menu
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean AboutConfig::onKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, NULL))
	{
		if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void AboutConfig::onInfoResponse(GtkWidget *info_bar, gint response_id,  gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	switch(response_id)
	{
		case -5://alowing
			gtk_widget_hide(info_bar);
			gtk_widget_set_sensitive(s->getWidget("scrolledwindow"),TRUE);
			SettingsManager::getInstance()->set(static_cast<SettingsManager::BoolSetting>(SettingsManager::AC_DISCLAIM), true);
			SettingsManager::getInstance()->save();
			break;
		case -6://not allowing
			gtk_widget_set_sensitive(s->getWidget("scrolledwindow"),FALSE);
			gtk_widget_hide(info_bar);
			SettingsManager::getInstance()->set(static_cast<SettingsManager::BoolSetting>(SettingsManager::AC_DISCLAIM), false);
			SettingsManager::getInstance()->save();
			break;
		default:
			break;
	}

}

void AboutConfig::onPropertiesClicked_gui(GtkWidget*, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string name = s->aboutView.getString(&iter,_("Name"));
		string value = s->aboutView.getString(&iter, _("Value"));
		gboolean isWsm = s->aboutView.getValue<gboolean>(&iter, "WS");
		
		bool run = s->getDialog(name, value, data);
		if(!run)
			return;

		if(isWsm)
		{
			WulforSettingsManager* wsm = WulforSettingsManager::getInstance();

			if(wsm->isString(name))
				wsm->set(name,value);

			if(wsm->isInt(name))
				wsm->set(name,Util::toInt(value));

			s->updateItem_gui(name,value,&iter,_("User set"),TRUE);
			return;
		}
		int n = -1;
		SettingsManager *sm = SettingsManager::getInstance();
		SettingsManager::Types type;
		sm->getType(name.c_str(), n, type);

		switch(type)
		{
			case SettingsManager::TYPE_STRING:
				sm->set((SettingsManager::StrSetting)n,value);
				break;
			case SettingsManager::TYPE_INT:
				sm->set((SettingsManager::IntSetting)n,Util::toInt(value));
				break;
			case SettingsManager::TYPE_INT64:
				sm->set((SettingsManager::Int64Setting)n, Util::toInt64(value));
				break;
			case SettingsManager::TYPE_FLOAT:
				sm->set((SettingsManager::FloatSetting)n,Util::toFloat(value));
				break;
			case SettingsManager::TYPE_BOOL:
				sm->set((SettingsManager::BoolSetting)n, Util::toInt(value));
				break;
			default: return;
		}
		s->updateItem_gui(name,value,&iter,_("User Set"),FALSE);
	}
}

void AboutConfig::onSetDefault(GtkWidget*, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string i = s->aboutView.getString(&iter,_("Name"));
		gboolean isWsm = s->aboutView.getValue<gboolean>(&iter, "WS");

		if(isWsm)
		{
			WulforSettingsManager* wsm = WulforSettingsManager::getInstance();
			string value = Util::emptyString;

			if(wsm->isString(i)) {
				wsm->SetStringDef(i);
				value = wsm->getString(i);
			}
			if(wsm->isInt(i)) {
				wsm->SetIntDef(i);
				value = Util::toString(wsm->getInt(i));
			}
			s->updateItem_gui(i,value,&iter,_("Default"),TRUE);
			s->setStatus("Value "+i+" Setted to Default "+value);
			return;
		}

		SettingsManager *sm = SettingsManager::getInstance();
		int n = -1 ;
		SettingsManager::Types type;

		if (sm->getType(i.c_str(), n, type))
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
			s->updateItem_gui(i, value,&iter);
			s->setStatus("Value" + i + "Setted to Default" + value);
		}
	}
}

bool AboutConfig::getDialog(const string name, string& value , gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;
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
			item = gtk_switch_new();
			gtk_switch_set_active(GTK_SWITCH(item),(gboolean)Util::toInt(value));
			break;
		case TYPE_INT:
			item = gtk_spin_button_new_with_range(-10,10000,1);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(item),(gint)Util::toInt(value));
			break;
		case TYPE_STRING:
			item = gtk_entry_new();
			gtk_entry_set_text(GTK_ENTRY(item),value.c_str());
			break;
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
