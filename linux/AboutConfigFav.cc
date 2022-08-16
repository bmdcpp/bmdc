//
//		Copyright (C) 2011 - 2023 - BMDC
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

#include "../dcpp/stdinc.h"
#include "../dcpp/SettingsManager.h"
#include "AboutConfigFav.hh"
#include "GuiUtil.hh"
#include "settingsmanager.hh"
#include "treeview.hh"

using namespace std;
using namespace dcpp;

bool AboutConfigFav::isOk[SettingsManager::SETTINGS_LAST-1];

const GActionEntry AboutConfigFav::win_entries[] = {
    { "edit", onPropertiesClicked_gui, NULL, NULL, NULL },
    { "deff", onSetDefault, NULL, NULL, NULL },
  };

AboutConfigFav::AboutConfigFav(FavoriteHubEntry* entry):
BookEntry(Entry::ABOUT_CONFIG_FAV, _("About:config for ") + entry->getName(), "config"),
p_entry(entry)
{

	GSimpleActionGroup* simple = g_simple_action_group_new ();
	g_simple_action_group_add_entries(simple, win_entries, G_N_ELEMENTS (win_entries), (gpointer)this);
	gtk_widget_insert_action_group(getContainer(),"abcf" ,G_ACTION_GROUP(simple));

	aboutView.setView(GTK_TREE_VIEW(getWidget("aboutTree")));
	aboutView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 120, "Color");
	aboutView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertColumn(_("Type"), G_TYPE_STRING, TreeView::STRING, 60);
	aboutView.insertColumn(_("Value"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertHiddenColumn("ForeColor", G_TYPE_STRING);
	//aboutView.insertHiddenColumn("BackColor", G_TYPE_STRING);
	aboutView.finalize();
	aboutStore = gtk_list_store_newv(aboutView.getColCount(), aboutView.getGTypes());
	gtk_tree_view_set_model(aboutView.get(), GTK_TREE_MODEL(aboutStore));
	string sort = _("Type");
	aboutView.setSortColumn_gui(_("Name"), sort);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(aboutStore), aboutView.col(sort), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(aboutView.get(), aboutView.col(_("Name"))), TRUE);
	g_object_unref(aboutStore);

	aboutSelection = gtk_tree_view_get_selection(aboutView.get());
	//g_signal_connect(aboutView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

	if(SETTING(AC_DISCLAIM) == false) {
			gtk_widget_set_sensitive(getWidget("scrolledwindow"),FALSE);
//			gtk_dialog_run(GTK_DIALOG(getWidget("infobar")));//@we need show this dialog
	}
	if(SETTING(AC_DISCLAIM) == true) {// we already confrim editing and so on
		gtk_widget_set_sensitive(getWidget("scrolledwindow"),TRUE);
		gtk_widget_hide(getWidget("infobar"));
	}

	g_signal_connect(GTK_INFO_BAR(getWidget("infobar")),
                            "response",
                            G_CALLBACK (onInfoResponse),
                            (gpointer)this);

	/* Register for mouse right button click "pressed" and "released" events on  widget*/
	GtkGesture *gesture;
  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
  g_signal_connect (gesture, "pressed",
                    G_CALLBACK (on_inner_widget_right_btn_pressed), (gpointer)this);
  g_signal_connect (gesture, "released",
                    G_CALLBACK (on_inner_widget_right_btn_released), (gpointer)this);
  gtk_widget_add_controller (GTK_WIDGET(aboutView.get()), GTK_EVENT_CONTROLLER (gesture));

	for(int i = 0;i < SettingsManager::SETTINGS_LAST-1;i++)
	{
		isOk[i] = false;
	}
	//whitelist
	isOk[SettingsManager::NICK] = true;
	isOk[SettingsManager::DESCRIPTION] = true;
	isOk[SettingsManager::EMAIL] = true;
	isOk[SettingsManager::TIME_RECCON] = true;
	isOk[SettingsManager::TIME_STAMPS] = true;
	isOk[SettingsManager::USE_COUNTRY_FLAG] = true;
	isOk[SettingsManager::COUNTRY_FORMAT] = true;
	isOk[SettingsManager::GET_USER_COUNTRY] = true;
	isOk[SettingsManager::EXTERNAL_IP] = true;
	isOk[SettingsManager::EXTERNAL_IP6] = true;
	isOk[SettingsManager::PROTECTED_USERS] = true;
	isOk[SettingsManager::BACKGROUND_CHAT_COLOR] = true;
	isOk[SettingsManager::BACKGROUND_CHAT_IMAGE] = true;
	isOk[SettingsManager::CHAT_EXTRA_INFO] = true;
	isOk[SettingsManager::HUB_ICON_STR] = true;
	isOk[SettingsManager::HUB_TEXT_STR] = true;
	isOk[SettingsManager::EMOT_PACK] = true;
	isOk[SettingsManager::USE_EMOTS] = true;
	isOk[SettingsManager::SHOW_JOINS] = true;
	isOk[SettingsManager::SHOW_FREE_SLOTS_DESC] = true;
	isOk[SettingsManager::FAV_SHOW_JOINS] = true;
	isOk[SettingsManager::HUB_UL_ORDER] =true;
	isOk[SettingsManager::HUB_UL_SIZE] = true;
	isOk[SettingsManager::HUB_UL_VISIBLE] = true;
	isOk[SettingsManager::LOG_CHAT_B] = true;
	isOk[SettingsManager::SORT_FAVUSERS_FIRST] = true;
	isOk[SettingsManager::DEFAULT_AWAY_MESSAGE] = true;
	isOk[SettingsManager::STATUS_IN_CHAT] = true;
	isOk[SettingsManager::USE_IP] = true;
	isOk[SettingsManager::BOLD_HUB] = true;
	isOk[SettingsManager::USE_HIGHLITING] = true;
	isOk[SettingsManager::USE_SOCK5] = true;
	isOk[SettingsManager::OUTGOING_CONNECTIONS] = true;
	setColorsRows();
}


void AboutConfigFav::on_inner_widget_right_btn_pressed (GtkGestureClick *gesture,
                                   int                n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	AboutConfigFav *FH = (gpointer)data;
  g_print ("on_inner_widget_right_btn_pressed() called\n");

GMenu *menu = g_menu_new ();
GMenuItem *menu_item_add = g_menu_item_new ("Edit", "abcf.edit");
g_menu_append_item (menu, menu_item_add);
g_object_unref (menu_item_add);

GMenuItem* menu_item_edit = g_menu_item_new ("Default", "abcf.deff");
g_menu_append_item (menu, menu_item_edit);
g_object_unref (menu_item_edit);

GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
gtk_widget_set_parent(pop, FH->getContainer());
gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
gtk_popover_popup (GTK_POPOVER(pop));

}

void AboutConfigFav::on_inner_widget_right_btn_released (GtkGestureClick *gesture,
                                    int              n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget)
{
  g_print ("on_inner_widget_right_btn_released() called\n");

  gtk_gesture_set_state (GTK_GESTURE (gesture),
                         GTK_EVENT_SEQUENCE_CLAIMED);
}

AboutConfigFav::~AboutConfigFav()
{

}


void AboutConfigFav::setColorsRows()
{
	setColorRow(_("Name"));
	setColorRow(_("Status"));
	setColorRow(_("Type"));
	setColorRow(_("Value"));
}

void AboutConfigFav::setColorRow(string cell)
{
if(aboutView.getCellRenderOf(cell) != NULL)
		gtk_tree_view_column_set_cell_data_func(aboutView.getColumn(cell),
								aboutView.getCellRenderOf(cell),
								AboutConfigFav::makeColor,
								(gpointer)this,
								NULL);

}

void AboutConfigFav::makeColor(GtkTreeViewColumn*,GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	AboutConfigFav* acf = (AboutConfigFav*)data;
	if(!acf){return;}
	if(model == NULL) {return;}
	if(iter == NULL){return;}
	if(cell == NULL) {return;}
	string sColor = acf->aboutView.getString(iter,"ForeColor",model);
	g_object_set(cell,"foreground-set",TRUE,"foreground",sColor.c_str(),NULL);

}

void AboutConfigFav::show()
{
	SettingsManager* sm = SettingsManager::getInstance();
	SettingsManager::Types type;
	const gchar* rowname = NULL;
	const gchar* isdefault = _("Default");
	const gchar* types = NULL;

	for(int n = 0; n < SettingsManager::SETTINGS_LAST-1; n++ ) {
		const gchar* tmp = (sm->getSettingTags()[n].c_str());
		if (strncasecmp(tmp,"SENTRY",7) == 0) continue;
		if (sm->getType(tmp, n, type)) {
			if(isOk[n] == true) {

			rowname = tmp;
			isdefault = _("User Set");

			switch(type) {
				case SettingsManager::TYPE_STRING:
				{
					bool bIsSame = false;
					types = "String";
					const gchar* value = g_strdup(p_entry->get(static_cast<SettingsManager::StrSetting>(n),
					sm->get(static_cast<SettingsManager::StrSetting>(n))
					).c_str());
					const gchar* temp = sm->getDefault(static_cast<SettingsManager::StrSetting>(n)).c_str();
					if(strcmp(value,temp) == 0)
					{	
						isdefault = _("Default");
						bIsSame = true;//they are same
					}	
					addItem_gui(rowname, isdefault, types, value, bIsSame);
					continue;
				}
				case SettingsManager::TYPE_INT:
				{
					bool bIsSame = false;
					types = "Integer";
					const gchar* value = g_strdup(Util::toString(p_entry->get(static_cast<SettingsManager::IntSetting>(n),
					sm->get(static_cast<SettingsManager::IntSetting>(n)))
					).c_str());

					const gchar* temp = Util::toString(sm->getDefault(static_cast<SettingsManager::IntSetting>(n))).c_str();
					if(strcmp(value,temp) == 0)
					{	
						isdefault = _("Default");
						bIsSame = true;//they are same
					}	

					addItem_gui(rowname, isdefault, types, value, bIsSame);
					continue;
				}
				case SettingsManager::TYPE_INT64:
				{
					continue;
				}
				case SettingsManager::TYPE_FLOAT:
				{
					continue;
				}
				case SettingsManager::TYPE_BOOL:
				{
					bool bIsSame = false;
					types = "Bool";
					const gchar* value = g_strdup(Util::toString(p_entry->get(static_cast<SettingsManager::BoolSetting>(n),sm->get(static_cast<SettingsManager::BoolSetting>(n)))).c_str());

					const gchar* temp = Util::toString(sm->getDefault(static_cast<SettingsManager::BoolSetting>(n))).c_str();
					if(strcmp(value,temp) == 0)
					{	
						isdefault = _("Default");
						bIsSame = true;//they are same
					}	
					addItem_gui(rowname, isdefault, types, value , bIsSame);
					continue;
				}
				default:
					dcassert(0);break;break;
			}

			}
		}
	}
}

void AboutConfigFav::addItem_gui(const gchar* rowname, const gchar* isdefault, const gchar* types, const gchar* value,bool bissame)
{
	GtkTreeIter iter;
	dcdebug("\n%s-%s-%s-%s\n ",rowname,isdefault,types,value);
	if(value == NULL) return;
	gboolean bisOk = g_utf8_validate(value,-1,NULL);
	gboolean bisOk2 = g_utf8_validate(rowname,-1,NULL);
	gboolean bisOk3 = g_utf8_validate(isdefault,-1,NULL);
	gboolean bisOk4 = g_utf8_validate(types,-1,NULL);
	if(!bisOk) {
		dcdebug("value\n");
	}
	if(!bisOk2) {
		dcdebug("rowname\n");
	}
	if(!bisOk3) {
		dcdebug("isdef\n");
	}
	if(!bisOk4) {
		dcdebug("types\n");
	}

	gtk_list_store_append(aboutStore,&iter);
	gtk_list_store_set(aboutStore,&iter,
				aboutView.col(_("Name")),rowname,
				aboutView.col(_("Status")), isdefault,
				aboutView.col(_("Type")), types,
				aboutView.col(_("Value")), value,
				aboutView.col("ForeColor"), !bissame ? "#FF0000" : "#000000",
	-1);

}

void AboutConfigFav::updateItem_gui(const string rowname,const string value, GtkTreeIter *iter,const gchar* status)
{
	if(iter) {
		gtk_list_store_set(aboutStore,iter,
				aboutView.col(_("Name")),rowname.c_str(),
				aboutView.col(_("Status")), status,
				aboutView.col(_("Value")), value.c_str(),
		-1);
	}
}

void AboutConfigFav::setStatus(const string msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, msg.c_str());
}
/*
gboolean AboutConfigFav::onKeyReleased_gui(GtkWidget* , GdkEventKey *event, gpointer data)
{
	AboutConfigFav *s = (AboutConfigFav *)data;

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, NULL))
	{
		if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup_at_pointer(GTK_MENU(s->getWidget("menu")),NULL);
		}
	}
	return FALSE;
}
*/
void AboutConfigFav::onInfoResponse(GtkWidget *info_bar, gint response_id,  gpointer data)
{
	AboutConfigFav *s = (AboutConfigFav *)data;

	switch(response_id)
	{
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_ACCEPT:
		//alowing
		{
			gtk_widget_hide(info_bar);
			gtk_widget_set_sensitive(s->getWidget("scrolledwindow"),TRUE);
			SettingsManager::getInstance()->set(static_cast<SettingsManager::BoolSetting>(SettingsManager::AC_DISCLAIM), true);
			break;
		}
		//not allowing
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_REJECT:
		{
			gtk_widget_set_sensitive(s->getWidget("scrolledwindow"),FALSE);
			gtk_widget_hide(info_bar);
			SettingsManager::getInstance()->set(static_cast<SettingsManager::BoolSetting>(SettingsManager::AC_DISCLAIM), false);
			break;
		}
		default:
			break;
	}
	SettingsManager::getInstance()->save();

}

void AboutConfigFav::onPropertiesClicked_gui(GtkWidget*,GVariant  *parameter, gpointer data)
{
	AboutConfigFav *s = (AboutConfigFav *)data;

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string name = s->aboutView.getString(&iter,_("Name"));
		string value = s->aboutView.getString(&iter, _("Value"));
		bool brun = s->getDialog(name, value, data);
		if(!brun)
			return;

		int n = -1;
		SettingsManager *sm = SettingsManager::getInstance();
		SettingsManager::Types type;
		sm->getType(name.c_str(), n, type);

		switch(type)
		{
			case SettingsManager::TYPE_STRING:
				s->p_entry->set((SettingsManager::StrSetting)n,value);
				break;
			case SettingsManager::TYPE_INT:
				s->p_entry->set((SettingsManager::IntSetting)n,Util::toInt(value));
				break;
			case SettingsManager::TYPE_INT64:
				break;
			case SettingsManager::TYPE_FLOAT:
				break;
			case SettingsManager::TYPE_BOOL:
				s->p_entry->set((SettingsManager::BoolSetting)n, Util::toInt(value));
				break;
			default: return;
		}
		s->updateItem_gui(name,value,&iter,_("User Set"));
	}
}

void AboutConfigFav::onSetDefault(GtkWidget*,GVariant  *parameter, gpointer data)
{
	AboutConfigFav *s = (AboutConfigFav *)data;

	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string name = s->aboutView.getString(&iter,_("Name"));
		SettingsManager *sm = SettingsManager::getInstance();
		int n = -1 ;
		SettingsManager::Types type;

		if (sm->getType(name.c_str(), n, type))
		{
			sm->unset(n);

			string sValue = string();

			switch(type) {
				case SettingsManager::TYPE_STRING:
					sValue = Text::toT(sm->getDefault(static_cast<SettingsManager::StrSetting>(n)));
					s->p_entry->set(static_cast<SettingsManager::StrSetting>(n),sValue);
					break;
				case SettingsManager::TYPE_INT:
					sValue = Text::toT(Util::toString(sm->getDefault(static_cast<SettingsManager::IntSetting>(n))));
					s->p_entry->set(static_cast<SettingsManager::IntSetting>(n),sm->getDefault(
					static_cast<SettingsManager::IntSetting>(n)));
					break;
				case SettingsManager::TYPE_INT64:
				case SettingsManager::TYPE_FLOAT:
					break;
				case SettingsManager::TYPE_BOOL:
					sValue = Text::toT(Util::toString((int)sm->getDefault(static_cast<SettingsManager::BoolSetting>(n))));
					s->p_entry->set(static_cast<SettingsManager::BoolSetting>(n),sm->getDefault(
					static_cast<SettingsManager::BoolSetting>(n)));
					break;
				default:
					return;
			}
			s->updateItem_gui(name, sValue,&iter);
			s->setStatus("Value" + name + "Setted to Default" + sValue);
		}
	}
}




///
bool AboutConfigFav::getDialog(const string sName, string& sValue , gpointer data)
{
	AboutConfigFav *ps = (AboutConfigFav *)data;
	gtk_label_set_text(GTK_LABEL(ps->getWidget("label")), sName.c_str());
	gtk_editable_set_text(GTK_EDITABLE(ps->getWidget("entry")), sValue.c_str());

	gtk_widget_show(ps->getWidget("dialog"));

//	if (response == GTK_RESPONSE_OK)
	{
		sValue = gtk_editable_get_text(GTK_EDITABLE(getWidget("entry")));
		return true;
	}
	return false;
}
