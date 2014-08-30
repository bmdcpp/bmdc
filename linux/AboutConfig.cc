//
//      Copyright 2011 - 2014 Mank <freedcpp at seznam dot cz>
//
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
#include "settingsmanager.hh"

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
	aboutView.insertHiddenColumn("WS", G_TYPE_STRING);
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
	
	if(!SETTING(AC_DISCLAIM)) {
			gtk_widget_set_sensitive(getWidget("scrolledwindow"),FALSE);
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
	SettingsManager::getInstance()->addListener(this);	
	
	int n;
	SettingsManager::Types type;
	SettingsManager* sm = SettingsManager::getInstance();
	
	for(int i = 0; i < SettingsManager::SETTINGS_LAST; i++ ) {
		string b = sm->getSettingTags()[i];
		if (b == "SENTRY") continue;
		if (sm->getType(b.c_str(), n, type)) {
			string rowname = b;
			string isdefault = Util::emptyString;
			string types = Util::emptyString;
			string value = Util::emptyString;
			switch(type) {
				case SettingsManager::TYPE_STRING:
					types =  _("String");
					value = sm->get(static_cast<SettingsManager::StrSetting>(n));
					isdefault = sm->isDefault(static_cast<SettingsManager::StrSetting>(n)) ? _("Default") : _("User set");
					break;
				case SettingsManager::TYPE_INT:
					types = _("Integer");
					value = Util::toString(sm->get(static_cast<SettingsManager::IntSetting>(n)));
					isdefault = sm->isDefault(static_cast<SettingsManager::IntSetting>(n)) ? _("Default") : _("User set");
					break;
				case SettingsManager::TYPE_INT64:
					types = _("Int64");
					value = Util::toString(sm->get(static_cast<SettingsManager::Int64Setting>(n)));
					isdefault = sm->isDefault(static_cast<SettingsManager::Int64Setting>(n)) ? _("Default") : _("User set");
					break;

				case SettingsManager::TYPE_FLOAT:
					types = _("Float");
					value = Util::toString(sm->get(static_cast<SettingsManager::FloatSetting>(n)));
					isdefault = sm->isDefault(static_cast<SettingsManager::FloatSetting>(n)) ? _("Default") : _("User set");
					break;
				case SettingsManager::TYPE_BOOL:
					types = _("Bool");
					value = Util::toString((int)sm->get(static_cast<SettingsManager::BoolSetting>(n)));
					isdefault = sm->isDefault(static_cast<SettingsManager::BoolSetting>(n)) ? _("Default") : _("User set");
					break;
				default:
					dcassert(0);
			}
			addItem_gui(rowname, isdefault, types, value);
			
		}
	}
	
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	WulforSettingsManager::StringMap map = wsm->getStringMap();
	WulforSettingsManager::StringMap defMap = wsm->getStringDMap();
	string types = Util::emptyString;
	string value = Util::emptyString;
	string dvalue = value;
	types = _("String");	
	for(auto d = defMap.begin();d!= defMap.end();++d)
	{
		string rowname = d->first;
		dvalue = d->second;
		bool isOk = map.find(rowname) != map.end();
		value = isOk ? map.find(rowname)->second : Util::emptyString;
		string isDef = !isOk ? _("Default") : _("User set");
		addItem_gui(rowname,isDef, types, ( !isOk ? dvalue : value), true);
	}
	
	WulforSettingsManager::IntMap imap = wsm->getIntMap();
	WulforSettingsManager::IntMap defIMap = wsm->getIntDMap();
	types = _("Integer");
	for(auto j = defIMap.begin();j != defIMap.end();++j) {
			string rowname = j->first;
			dvalue = Util::toString(j->second);
			bool isOk = imap.find(rowname) != imap.end();
			value = isOk ? Util::toString(imap.find(rowname)->second) : Util::emptyString;
			string isDef = !isOk ? _("Default") : _("User set");
			addItem_gui(rowname, isDef, types, ( !isOk ? dvalue : value), true);
	}
	
}

void AboutConfig::addItem_gui(string rowname, string isdefault, string types, string value, bool isWulf)
{
	GtkTreeIter iter;
	
	gtk_list_store_append(aboutStore,&iter);
	gtk_list_store_set(aboutStore,&iter,
				aboutView.col(_("Name")),rowname.c_str(),
				aboutView.col(_("Status")), isdefault.c_str(),
				aboutView.col(_("Type")), types.c_str(),
				aboutView.col(_("Value")), value.c_str(),
				aboutView.col("WS"), isWulf ? "1" : "0", 
	-1);
	
	aboutIters.insert(UnMapIter::value_type(rowname,iter));
	
}

void AboutConfig::updateItem_gui(string rowname, string value)
{
	GtkTreeIter iter;
	
	if(findAboutItem_gui(rowname,&iter)){
		
		gtk_list_store_set(aboutStore,&iter,
				aboutView.col(_("Name")),rowname.c_str(),
				aboutView.col(_("Value")), value.c_str(),
		-1);
	
	}	
}

bool AboutConfig::findAboutItem_gui(const string &about, GtkTreeIter *iter)
{
	UnMapIter::const_iterator it = aboutIters.find(about);

	if (it != aboutIters.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}

void AboutConfig::setStatus(string msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, msg.c_str());
}

gboolean AboutConfig::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;
	s->previous = event->type;
	return FALSE;
}

gboolean AboutConfig::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
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

gboolean AboutConfig::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
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
		case -6://not allowing
			gtk_widget_hide(info_bar);
			break;
		case -5://alowing
			gtk_widget_hide(info_bar);
			gtk_widget_set_sensitive(s->getWidget("scrolledwindow"),TRUE);
			SettingsManager::getInstance()->set(SettingsManager::AC_DISCLAIM,true);
			SettingsManager::getInstance()->save();
			break;
		default:		
			return;
	}
	
}                 

void AboutConfig::onPropertiesClicked_gui(GtkWidget *widget, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string name = s->aboutView.getString(&iter,_("Name"));
		string value = s->aboutView.getString(&iter, _("Value"));
		bool isWsm = (s->aboutView.getString(&iter, "WS") == "1") ? true : false;
		int n;
		SettingsManager *sm = SettingsManager::getInstance();
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
			s->updateItem_gui(name,value);
			return;	
		}
		
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
			default:;
		}
		s->updateItem_gui(name,value);
	}
}

void AboutConfig::onSetDefault(GtkWidget *widget, gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;
	
	GtkTreeIter iter;
	
	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string i = s->aboutView.getString(&iter,_("Name"));
		bool isWsm = s->aboutView.getString(&iter, "WS") == "1" ? TRUE : FALSE;
		
		if(isWsm)
		{
			auto wsm = WulforSettingsManager::getInstance();
			string value = Util::emptyString;
			if(wsm->isString(i)) {
				wsm->SetStringDef(i);
				value = wsm->getString(i);
			} if(wsm->isInt(i)) {
				wsm->SetIntDef(i);
				value = Util::toString(wsm->getInt(i));
			}
			s->updateItem_gui(i,value);
			s->setStatus("Value "+i+" Setted to Default "+value);
			return;		
		}
		
		auto sm = SettingsManager::getInstance();
		int n ;
		SettingsManager::Types type;
		
		if (sm->getType(Text::fromT(i).c_str(), n, type))
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
					dcassert(0);
			}
			s->updateItem_gui(i, value);
			s->setStatus("Value" + i + "Setted to Default" + value);
		}
	}
}

bool AboutConfig::getDialog(string name, string& value , gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;
	gtk_label_set_text(GTK_LABEL(s->getWidget("label")), name.c_str());
	gtk_entry_set_text(GTK_ENTRY(s->getWidget("entry")), value.c_str());
	int response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dialog")));
	
	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return false;

	gtk_widget_hide(s->getWidget("dialog"));

	if (response == GTK_RESPONSE_OK)
	{
		value = gtk_entry_get_text(GTK_ENTRY(getWidget("entry")));	
		return true;
	}
	return false;	
}
