#include <dcpp/stdinc.h>
#include <dcpp/SettingsManager.h>
#include "AboutConfig.hh"
#include "settingsmanager.hh"

using namespace std;
using namespace dcpp;

AboutConfig::AboutConfig():
BookEntry(Entry::ABOUT_CONFIG, _("About:config"), "config.glade")
{

	aboutView.setView(GTK_TREE_VIEW(getWidget("aboutTree")));
	aboutView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertColumn(_("Type"), G_TYPE_STRING, TreeView::STRING, 60);
	aboutView.insertColumn(_("Value"), G_TYPE_STRING, TreeView::STRING, 100);
	aboutView.insertHiddenColumn("WS", G_TYPE_STRING);
	aboutView.finalize();
	aboutStore = gtk_list_store_newv(aboutView.getColCount(), aboutView.getGTypes());
	gtk_tree_view_set_model(aboutView.get(), GTK_TREE_MODEL(aboutStore));
	g_object_unref(aboutStore);

	aboutSelection = gtk_tree_view_get_selection(aboutView.get());
	
	g_signal_connect(aboutView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(aboutView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(aboutView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("propteriesItem"), "activate", G_CALLBACK(onPropertiesClicked_gui),(gpointer)this);

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
	auto sm = SettingsManager::getInstance();
	for(int i = 0;i < SettingsManager::SETTINGS_LAST ; i++ ) {
		string b = sm->getSettingTags()[i];
		if (b == "SENTRY") continue;
		if (sm->getType(b.c_str(), n, type)) {
			string rowname = b;
			string isdefault = sm->isDefault(n) ? _("Default") : _("User set");
			string types = Util::emptyString;
			string value = Util::emptyString;
			switch(type) {
				case SettingsManager::TYPE_STRING:
					types =  _("String");
					value = Text::toT(sm->get(static_cast<SettingsManager::StrSetting>(n)));
					break;
				case SettingsManager::TYPE_INT:
					types = _("Integer");
					value = Util::toString(sm->get(static_cast<SettingsManager::IntSetting>(n)));
					break;

				case SettingsManager::TYPE_INT64:
					types = _("Int64");
					value = Util::toString(sm->get(static_cast<SettingsManager::Int64Setting>(n)));
					break;

				case SettingsManager::TYPE_FLOAT:
					types = _("Float");
					value = Util::toString(sm->get(static_cast<SettingsManager::FloatSetting>(n)));
					break;

				default:
					dcassert(0);
			}
			addItem_gui(rowname,isdefault,types, value);
			
		}
	}
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	WulforSettingsManager::StringMap map = wsm->getStringMap();
	string types = Util::emptyString;
	string value = Util::emptyString;
	for(auto i = map.begin();i!= map.end();++i)
	{
		string rowname = i->first;
		types = _("String");
		value = i->second;
		string isdefault = wsm->isDefaultString(rowname) ? _("Default") : _("User set");
		addItem_gui(rowname, isdefault, types, value, true);
	}
	
	WulforSettingsManager::IntMap imap = wsm->getIntMap();
	for(auto i = imap.begin();i!= imap.end();++i)
	{
		string rowname = i->first;
		types = _("Integer");
		value = Util::toString(i->second);
		string isdefault = wsm->isDefaultInt(rowname) ? _("Default") : _("User set");
		addItem_gui(rowname, isdefault, types, value, true);
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
	aboutIters.insert(AboutIters::value_type(rowname,iter));
	
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
	AboutIters::const_iterator it = aboutIters.find(about);

	if (it != aboutIters.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}

gboolean AboutConfig::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	AboutConfig *s = reinterpret_cast<AboutConfig *>(data);
	s->previous = event->type;
	return FALSE;
}

gboolean AboutConfig::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	AboutConfig *s = reinterpret_cast<AboutConfig *>(data);

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, NULL))
	{
		if (event->button == 1 && s->previous == GDK_2BUTTON_PRESS)
		{
			// show dialog
			onPropertiesClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)//TODO ??
		{
			// show menu
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean AboutConfig::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	AboutConfig *s = reinterpret_cast<AboutConfig *>(data);

	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, NULL))
	{
		if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void AboutConfig::onPropertiesClicked_gui(GtkWidget *widget, gpointer data)
{
	AboutConfig *s = reinterpret_cast<AboutConfig *>(data);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(s->aboutSelection, NULL, &iter))
	{
		string i = s->aboutView.getString(&iter,_("Name"));
		string value = s->aboutView.getString(&iter, _("Value"));
		bool isWsm = s->aboutView.getString(&iter, "WS") == "1" ? TRUE : FALSE;
		int n;
		auto sm = SettingsManager::getInstance();
		bool run = s->getDialog(i,value,data);
		if(!run)
			return;
		if(isWsm)
		{
			auto wsm = WulforSettingsManager::getInstance();	
			if(wsm->isString(i))
				wsm->set(i,value);
			if(wsm->isInt(i))
				wsm->set(i,Util::toInt(value));
				s->updateItem_gui(i,value);
			return;	
		}
			
		
		SettingsManager::Types type;		
		sm->getType(i.c_str(), n, type);
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
			default:;
		}
		s->updateItem_gui(i,value);
	}
}

bool AboutConfig::getDialog(string i, string& value , gpointer data)
{
	AboutConfig *s = (AboutConfig *)data;
	gtk_label_set_text(GTK_LABEL(s->getWidget("label")), i.c_str());
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