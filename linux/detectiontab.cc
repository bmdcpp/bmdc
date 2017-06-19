//
//      Copyright 2011 -2017 BMDC <freedcpp at seznam dot cz>
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
//@TODO: Move to Settings Dialog

#include "detectiontab.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

DetectionTab::DetectionTab():
BookEntry(Entry::DETECTION, _("Detection Settings"), "detection")
{
	gtk_window_set_transient_for(GTK_WINDOW(getWidget("ActRawDialog")), GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(getWidget("ActRawDialog")), TRUE);

	gtk_window_set_transient_for(GTK_WINDOW(getWidget("dialogitemPoints")), GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(getWidget("dialogitemPoints")), TRUE);

	gtk_window_set_transient_for(GTK_WINDOW(getWidget("dialogpointitem1")), GTK_WINDOW(getWidget("dialogitemPoints")));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(getWidget("dialogpointitem1")), TRUE);

	//DetectionClients
	detectionView.setView(GTK_TREE_VIEW(getWidget("treeviewProf")));
	detectionView.insertColumn(N_("Enable"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	detectionView.insertColumn(N_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
	detectionView.insertColumn(N_("Cheat"), G_TYPE_STRING, TreeView::STRING, 100);
	detectionView.insertColumn(N_("Comment"), G_TYPE_STRING, TreeView::STRING, 100);
	detectionView.insertColumn(N_("Raw"), G_TYPE_INT, TreeView::INT, 100);
	detectionView.insertColumn(N_("ID"), G_TYPE_INT, TreeView::INT, 100);
	detectionView.insertColumn(N_("Flag"), G_TYPE_INT, TreeView::INT, 100);
	detectionView.insertColumn(N_("MisMatch"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	detectionView.finalize();

	detectionStore = gtk_list_store_newv(detectionView.getColCount(), detectionView.getGTypes());
	gtk_tree_view_set_model(detectionView.get(), GTK_TREE_MODEL(detectionStore));
	g_object_unref(detectionStore);

	detectionSelection = gtk_tree_view_get_selection(detectionView.get());

	//Action&Raw
	///Actions
	actionView.setView(GTK_TREE_VIEW(getWidget("treeviewAction")));
    actionView.insertColumn(N_("Enable"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);//checkbox
	actionView.insertColumn(N_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
	actionView.insertColumn(N_("ID"), G_TYPE_INT, TreeView::INT, 100);
	actionView.finalize();

	actionStore = gtk_list_store_newv(actionView.getColCount(),actionView.getGTypes());
	gtk_tree_view_set_model(actionView.get(),GTK_TREE_MODEL(actionStore));
	g_object_unref(actionStore);
	actionSelection = gtk_tree_view_get_selection(actionView.get());
	///Raws
	RawView.setView(GTK_TREE_VIEW(getWidget("treeviewRaw")));
	RawView.insertColumn(N_("Enable"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);//checkbox
	RawView.insertColumn(N_("Name"), G_TYPE_STRING, TreeView::STRING, 100);
	RawView.insertColumn(N_("Raw"), G_TYPE_STRING, TreeView::STRING, 100);
	RawView.insertColumn(N_("Time"), G_TYPE_INT, TreeView::INT, 100);
	RawView.insertColumn(N_("ID"), G_TYPE_INT, TreeView::INT, 100);
	RawView.insertHiddenColumn("Action", G_TYPE_STRING);
	RawView.finalize();

	RawStore = gtk_list_store_newv(RawView.getColCount(),RawView.getGTypes());
	gtk_tree_view_set_model(RawView.get(),GTK_TREE_MODEL(RawStore));
	g_object_unref(RawStore);
	RawSelection = gtk_tree_view_get_selection(RawView.get());

	/* connect to signals */
	/* 1page */
	g_signal_connect(getWidget("buttonAddAct"), "clicked", G_CALLBACK(onAddAct),(gpointer)this);//add
	g_signal_connect(getWidget("buttonmodact"), "clicked", G_CALLBACK(onEditAct), (gpointer)this);//edit
	g_signal_connect(getWidget("buttonremact"), "clicked", G_CALLBACK(onRemoveAct), (gpointer)this);//remove

	g_signal_connect(getWidget("buttonAddRaw"), "clicked", G_CALLBACK(onAddRaw),(gpointer)this);//add
	g_signal_connect(getWidget("buttoneditraw"), "clicked", G_CALLBACK(onEditRaw), (gpointer)this);//edit
	g_signal_connect(getWidget("buttonremraw"), "clicked", G_CALLBACK(onRemoveRaw), (gpointer)this);//remove

    g_signal_connect(actionView.get(), "button-press-event", G_CALLBACK(onActButtonPressed_gui), (gpointer)this);
	g_signal_connect(actionView.get(), "button-release-event", G_CALLBACK(onActButtonReleased_gui), (gpointer)this);
	g_signal_connect(actionView.get(), "key-release-event", G_CALLBACK(onActKeyReleased_gui), (gpointer)this);

	/* 2page */
	g_signal_connect(getWidget("button4"), "clicked", G_CALLBACK(onAddEntryDet), (gpointer)this);//add
	g_signal_connect(getWidget("button5"), "clicked", G_CALLBACK(ondModEntryDet), (gpointer)this);//edit
	g_signal_connect(getWidget("button6"), "clicked", G_CALLBACK(onRemoveEntryDet), (gpointer)this);//remove

	g_signal_connect(detectionView.getCellRenderOf(N_("Enable")), "toggled", G_CALLBACK(onToggleDet), (gpointer)this);

	item.setView(GTK_TREE_VIEW(getWidget("treeview1")));
	item.insertColumn(N_("Field"), G_TYPE_STRING, TreeView::STRING,100);
	item.insertColumn(N_("Value"), G_TYPE_STRING, TreeView::STRING,100);
	item.insertColumn(N_("Hub"),  G_TYPE_STRING, TreeView::STRING,50);
	item.finalize();

	itemstore = gtk_list_store_newv(item.getColCount(), item.getGTypes());
	gtk_tree_view_set_model(item.get(), GTK_TREE_MODEL(itemstore));
	g_object_unref(itemstore);
	itemselection = gtk_tree_view_get_selection(item.get());

	g_signal_connect(item.get(), "button-press-event", G_CALLBACK(onButtonItemPressed_gui), (gpointer)this);
	g_signal_connect(item.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("additem"), "activate", G_CALLBACK(onAddItemDlg_gui), (gpointer)this);
	g_signal_connect(getWidget("changeitem"), "activate", G_CALLBACK(onModItemDlg_gui), (gpointer)this);
	g_signal_connect(getWidget("removeitem"), "activate", G_CALLBACK(onRemItemDlg_gui), (gpointer)this);

	actionsSelectionsR.setView(GTK_TREE_VIEW(getWidget("treeviewACTRAWSEL")));
	actionsSelectionsR.insertColumn("Number", G_TYPE_INT, TreeView::INT, 100);
	actionsSelectionsR.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, 100);
	actionsSelectionsR.finalize();

	actionsSelectionsRStore = gtk_list_store_newv(actionsSelectionsR.getColCount(), actionsSelectionsR.getGTypes());
	gtk_tree_view_set_model(actionsSelectionsR.get(), GTK_TREE_MODEL(actionsSelectionsRStore));
	g_object_unref(actionsSelectionsRStore);

	actionSelectionRR = gtk_tree_view_get_selection(actionsSelectionsR.get());
	SettingsManager *sm = SettingsManager::getInstance();
	g_signal_connect(getWidget("buttonFake"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonFake")),"wid",getWidget("entryFake"));
	g_object_set_data(G_OBJECT(getWidget("buttonFake")), "set", GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::FAKESHARE_RAW)));

	g_signal_connect(getWidget("buttonRM"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonRM")),"wid",getWidget("entryRM"));
	g_object_set_data(G_OBJECT(getWidget("buttonRM")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::RMDC_RAW)));

	g_signal_connect(getWidget("buttonEM"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonEM")),"wid",getWidget("entryEM"));
	g_object_set_data(G_OBJECT(getWidget("buttonEM")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::DCPP_EMULATION_RAW)));

	g_signal_connect(getWidget("buttonmis"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonmis")),"wid",getWidget("entrymis"));
	g_object_set_data(G_OBJECT(getWidget("buttonmis")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::FILELIST_VERSION_MISMATCH)));

	g_signal_connect(getWidget("buttonlis"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonlis")),"wid",getWidget("entrylis"));
	g_object_set_data(G_OBJECT(getWidget("buttonlis")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::LISTLEN_MISMATCH)));

	g_signal_connect(getWidget("buttonvermis"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonvermis")),"wid",getWidget("entryvermis"));
	g_object_set_data(G_OBJECT(getWidget("buttonvermis")), "set",GINT_TO_POINTER(sm->get(SettingsManager::BoolSetting::VERSION_MISMATCH)));

	g_signal_connect(getWidget("buttondis"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttondis")),"wid",getWidget("entrydis"));
	g_object_set_data(G_OBJECT(getWidget("buttondis")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::DISCONNECT_RAW)));

	g_signal_connect(getWidget("buttontbsfl"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttontbsfl")),"wid",getWidget("entrytbsfl"));
	g_object_set_data(G_OBJECT(getWidget("buttontbsfl")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::FILELIST_TOO_SMALL_BIG_RAW)));

	g_signal_connect(getWidget("buttonspeed"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonspeed")),"wid",getWidget("entryspeed"));
	g_object_set_data(G_OBJECT(getWidget("buttonspeed")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::SDL_RAW)));

	g_signal_connect(getWidget("buttonadl"), "clicked", G_CALLBACK(onSelectAction), (gpointer)this);
	g_object_set_data(G_OBJECT(getWidget("buttonadl")),"wid",getWidget("entryadl"));
	g_object_set_data(G_OBJECT(getWidget("buttonadl")), "set",GINT_TO_POINTER(sm->get(SettingsManager::IntSetting::ADL_RAW)));

	g_signal_connect(getWidget("button7Save"), "clicked", G_CALLBACK(onSave), (gpointer)this);
	g_signal_connect(getWidget("buttonadlsp7"), "clicked", G_CALLBACK(onADLPoints), (gpointer)this);

	points.setView(GTK_TREE_VIEW(getWidget("treeviewPoints")));
	points.insertColumn(N_("Points"), G_TYPE_INT, TreeView::INT, 100);
	points.insertColumn(N_("Action"), G_TYPE_INT, TreeView::INT, 100);
	points.finalize();

	pointstore = gtk_list_store_newv(points.getColCount(), points.getGTypes());
	gtk_tree_view_set_model(points.get(), GTK_TREE_MODEL(pointstore));
	g_object_unref(pointstore);
	pointselect = gtk_tree_view_get_selection(points.get());

	g_signal_connect(getWidget("buttonp1"), "clicked", G_CALLBACK(onADSLPointsADD), (gpointer)this);
	g_signal_connect(getWidget("buttonp2"), "clicked", G_CALLBACK(onADSLPointsMOD), (gpointer)this);
	g_signal_connect(getWidget("buttonp3"), "clicked", G_CALLBACK(onADSLPointsDEL), (gpointer)this);

}

DetectionTab::~DetectionTab() { }

void DetectionTab::show() {
	//Fake
	gboolean afake =SETTING(SHOW_FAKESHARE_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkFake1")), afake);
	//EMULation
	gboolean emul = SETTING(SHOW_DCPP_EMULATION) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkemul1")), emul);
	///FLVMISMATCH
	gboolean ver= SETTING(SHOW_FILELIST_VERSION_MISMATCH) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkVmismatch1")), ver);
	///ListlenMMatch
	gboolean listlen = SETTING(LISTLEN_MISMATCH_SHOW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checklistlen")), listlen);
	///Diconnect
	gboolean disc = SETTING(SHOW_DISCONNECT) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkdiscon7")), disc);
	///SLSP
	gboolean useslw = SETTING(USE_SDL_KICK) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton2slw")), useslw);
	///Show Cheats
	gboolean showcheat = SETTING(DISPLAY_CHEATS_IN_MAIN_CHAT) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkShowCheat")), showcheat);
	///ADLA
	gboolean showadla = SETTING(SHOW_ADLSEARCH_DEFAULT_ACTION) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkADLACtionShow")), showadla);

	showadla = SETTING(SHOW_RMDC) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkRmdc1")), showadla);

	showadla = SETTING(FILELIST_TOO_SMALL_BIG) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton1")), showadla);

	showadla = SETTING(VERSION_MISMATCH) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkVmismatch1")), showadla);

	///Min FL SIZE
	int minflsize = SETTING(MIN_FL_SIZE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinFLMINSIZE")), (gdouble)minflsize);
	///Discon
	int maxdiscon = SETTING(MAX_DISCONNECTS);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinMaxDiscon")), (gdouble)maxdiscon);
	///MaxTestSurs
	int maxtestsur = SETTING(MAX_TESTSURS);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spintestsur")), (gdouble)maxtestsur);
	///MaxFL
	int maxfilelist = SETTING(MAX_FILELISTS);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinFileLists")), (gdouble)maxfilelist);
	///ClientsCheckBeforeFL
	gboolean chclbeffl = SETTING(CHECK_ALL_CLIENTS_BEFORE_FILELISTS) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkCLBFFL")), chclbeffl);
	///SleepTime
	int sleept = SETTING(SLEEP_TIME);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinsleep")), (gdouble)sleept);
	///Min ADLPoints To Display
	int points = SETTING(MIN_POINTS_TO_DISPLAY_CHEAT);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinADLPointsToDisplay")), (gdouble)points);
	///delay raws
	gboolean usedelayraw = SETTING(USE_SEND_DELAYED_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkusedelay")), usedelayraw);
	///percent fake
	int percentfake = SETTING(PERCENT_FAKE_SHARE_TOLERATED);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinPerFake")), percentfake);
	///sdl speed
	int sdlspeed = SETTING(SDL_SPEED);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinspeedsd")), sdlspeed);
	///SDL Time
	int sdltime = SETTING(SDL_TIME);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spintimesd")), sdltime);
	///Protect Users
	gchar * protectuser = const_cast<char*>(SETTING(PROTECTED_USERS).c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryProtectPatern")), protectuser);
	/**/
	SettingsManager *sm = SettingsManager::getInstance();
	int fake = sm->get(SettingsManager::IntSetting::FAKESHARE_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryFake")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::DCPP_EMULATION_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryEM")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::FILELIST_VERSION_MISMATCH);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrymis")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::RMDC_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryRM")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::LISTLEN_MISMATCH);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrylis")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::VERSION_MISMATCH_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryvermis")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::DISCONNECT_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrydis")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::FILELIST_TOO_SMALL_BIG_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrytbsfl")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::SDL_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryspeed")), Util::toString(fake).c_str());

	fake = sm->get(SettingsManager::IntSetting::ADL_RAW);
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryadl")), Util::toString(fake).c_str());

	create_profiles();
	create_actions_raws();
}
/*1page*/
void DetectionTab::create_actions_raws() {

	GtkTreeIter topi;

	Action::ActionList& act = RawManager::getInstance()->getActions();

	for (Action::ActionList::const_iterator i = act.begin(); i != act.end();++i)
	{
		const string& name = (*i)->getName();

		gtk_list_store_append(actionStore, &topi);
		gtk_list_store_set(actionStore, &topi,
						actionView.col(N_("Name")), name.c_str(),
						actionView.col(N_("Enable")), (*i)->getEnabled() ? TRUE : FALSE,
						actionView.col(N_("ID")), (*i)->getId(),
						-1);

		actions.insert(ActRaw::value_type( ((*i)->getId()),topi));
	}

}
/* 2Page */
void DetectionTab::create_profiles()
{
	dcpp::ParamMap params;
	GtkTreeIter iter;
	const DetectionManager::DetectionItems& lst = DetectionManager::getInstance()->getProfiles(params,false/*true*/);
	for ( DetectionManager::DetectionItems::const_iterator i = lst.begin(); i != lst.end() ;++i)
	{
		const DetectionEntry& de = *i ;
		gtk_list_store_append(detectionStore, &iter);
		gtk_list_store_set(detectionStore, &iter,
						detectionView.col(N_("Enable")),de.isEnabled ? TRUE: FALSE,
						detectionView.col(N_("Name")),de.name.c_str(),
						detectionView.col(N_("Cheat")),de.cheat.c_str(),
						detectionView.col(N_("Comment")),de.comment.c_str(),
						detectionView.col(N_("Raw")), de.rawToSend,
						detectionView.col(N_("ID")),de.Id,
						detectionView.col(N_("Flag")),de.clientFlag,
						detectionView.col(N_("MisMatch")), de.checkMismatch ? TRUE : FALSE,
						-1);
		profiles.insert(Prof::value_type(de.Id,iter));
	}
}

gboolean DetectionTab::onActButtonPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	dt->actPrevious = event->type;
	return FALSE;
}

gboolean DetectionTab::onActButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);

	if (dt->actPrevious == GDK_BUTTON_PRESS && gtk_tree_selection_get_selected(dt->actionSelection, NULL, NULL))
	{
		if (event->button == 1)
		{
			dt->updateRawView_gui();
		}
		else if (event->button == 3)
		{
			dt->updateRawView_gui();
		}
	}
	return FALSE;
}

gboolean DetectionTab::onActKeyReleased_gui(GtkWidget*, GdkEventKey *event, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dt->actionSelection, NULL, &iter))
	{
		if (event->keyval == GDK_KEY_Up || 	event->keyval == GDK_KEY_Down )
		{
			dt->updateRawView_gui();
		}
	}
	return FALSE;
}

void DetectionTab::updateRawView_gui()
{
	RawManager::getInstance()->saveActionRaws();
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(actionSelection, NULL, &iter))
	{
		if(!raws.empty())
             raws.clear();
		gint aid = actionView.getValue<gint>(&iter, N_("ID"));
		gtk_list_store_clear(RawStore);
		Action *action = RawManager::getInstance()->findAction((int)aid);
          GtkTreeIter child;
          if( (action != NULL) && (!action->raw.empty()) )
          {
            for(Action::RawsList::const_iterator p = (action)->raw.begin();p!=((action)->raw.end()); ++p)
            {
                gtk_list_store_append(RawStore,&child);
                gtk_list_store_set(RawStore, &child,
                                RawView.col(N_("Name")), p->getName().c_str(),
                                RawView.col(N_("Raw")), p->getRaw().c_str(),
                                RawView.col(N_("Time")), p->getTime(),
                                RawView.col(N_("Enable")), p->getEnabled() ? TRUE : FALSE,
                                RawView.col(N_("ID")), p->getId(),
                                RawView.col("Action"), action->getName().c_str(),
                                -1);
                raws.insert(ActRaw::value_type( p->getId(),child));

            }
        }
    }
}
void DetectionTab::onAddAct(GtkWidget*,gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	const string emptyStr = "";
	StringMap params;
	params["Name"] = emptyStr;
	params["RAW"] = emptyStr;
	params["Time"] = "0.0";
	params["ID"] = Util::toString(Util::rand(1, 2147483647));
	params["Enabled"] = "0";
	params["Type"] = "0";

	bool isOk = dt->showAddActRawDialog(params,dt);

	if(isOk)
	{
		dt->addAct_gui(params);
		typedef Func1<DetectionTab,StringMap> F1;
		F1 *func = new F1(dt,&DetectionTab::addAct_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}

}

void DetectionTab::onAddRaw(GtkWidget*, gpointer data)
{
     DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	const string emptyStr = "";
	StringMap params;
	params["Name"] = emptyStr;
	params["RAW"] = emptyStr;
	params["Time"] = "0.0";
	params["ID"] = Util::toString(Util::rand(1, 2147483647));
	params["Enabled"] = "0";
	GtkTreeIter iter;
	
	if (gtk_tree_selection_get_selected(dt->actionSelection,NULL, &iter))
        params["Action"] = dt->actionView.getString(&iter,N_("Name"));
	
	params["Type"] = "1";

	bool isOk = dt->showAddActRawDialog(params,dt);

	if(isOk)
	{
		dt->addRaw_gui(params);
		typedef Func1<DetectionTab,StringMap> F1;
		F1 *func = new F1(dt,&DetectionTab::addRaw_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}
void DetectionTab::onEditAct(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter	iter;

	if (!gtk_tree_selection_get_selected(dt->actionSelection,NULL, &iter))
		return;

	StringMap params;
	params["Action"] = params["Name"] = dt->actionView.getString(&iter,N_("Name"));
	params["Enabled"] = dt->actionView.getValue<gboolean>(&iter,N_("Enable")) ? "1" : "0";
	params["ID"] = Util::toString(dt->actionView.getValue<gint>(&iter,N_("ID")));
	params["Time"] = "0";

	bool isOk = dt->showAddActRawDialog(params,dt);
	if(isOk)
	{
		dt->addAct_gui(params);
		typedef Func1<DetectionTab,StringMap> F1;
		F1 *func = new F1(dt,&DetectionTab::editAct_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DetectionTab::onEditRaw(GtkWidget*,gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter	iter;

	if (!gtk_tree_selection_get_selected(dt->RawSelection,NULL, &iter))
		return;

	StringMap params;
	params["Name"] = dt->RawView.getString(&iter,N_("Name"));
	params["RAW"] = dt->RawView.getString(&iter,N_("Raw"));
	params["Time"] = Util::toString(dt->RawView.getValue<gint>(&iter,N_("Time")));
	params["Enabled"] = dt->RawView.getValue<gboolean>(&iter,N_("Enable")) ? "1" : "0";
	params["ID"] = Util::toString(dt->RawView.getValue<gint>(&iter,N_("ID")));
	params["Action"] = dt->RawView.getString(&iter,"Action");

	bool isOk = dt->showAddActRawDialog(params,dt);//ch
	if(isOk)
	{
		dt->addRaw_gui(params);
		typedef Func1<DetectionTab,StringMap> F1;
		F1 *func = new F1(dt,&DetectionTab::editRaw_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DetectionTab::onRemoveAct(GtkWidget* , gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dt->actionSelection, NULL, &iter))
	{
		string name = dt->actionView.getString(&iter, N_("Name"));

		if(SETTING(CONFIRM_HUB_REMOVAL))
		{
			GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete this item \"%s\"?"), name.c_str());
				gtk_dialog_add_buttons(GTK_DIALOG(dialog), BMDC_STOCK_CANCEL, GTK_RESPONSE_CANCEL, BMDC_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);

				gint response = gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);

				if (response != GTK_RESPONSE_YES)
					return;
		}

		gint id = dt->actionView.getValue<gint>(&iter, N_("ID"));
		dt->removeAction_gui(Util::toString((int)id),name);
		typedef Func1<DetectionTab, int> F1;
		F1 *func = new F1(dt, &DetectionTab::removeAct_client, (int)id);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DetectionTab::onRemoveRaw(GtkWidget* , gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dt->RawSelection, NULL, &iter))
	{
		string name = dt->RawView.getString(&iter, N_("Name"));

		if(SETTING(CONFIRM_HUB_REMOVAL))
		{
			GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete this item \"%s\"?"), name.c_str());
				gtk_dialog_add_buttons(GTK_DIALOG(dialog), BMDC_STOCK_CANCEL, GTK_RESPONSE_CANCEL, BMDC_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);

				gint response = gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);

				if (response != GTK_RESPONSE_YES)
					return;
		}
		gint id = dt->RawView.getValue<gint>(&iter, N_("ID"));
		dt->removeRaw_gui(Util::toString((int)id),name);
		typedef Func1<DetectionTab, int> F1;
		F1 *func = new F1(dt, &DetectionTab::removeRaw_client, (int)id);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

bool DetectionTab::showAddActRawDialog(StringMap &params,DetectionTab *dt)
{
		string tmpname = params["Name"];
		/* text and spins */
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryName")), params["Name"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryRaw")), params["RAW"].c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonId")),(gdouble)Util::toDouble(params["ID"]));
		gtk_widget_set_sensitive(dt->getWidget("spinbuttonId"),FALSE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonTime")),(gdouble)Util::toDouble(params["Time"]));
		// Set the Enabled checkbox
		gboolean enabled =(((params["Enabled"] == "1") ? TRUE : FALSE) || params["Enabled"].empty());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkEnabled")), enabled);
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxType")),Util::toInt(params["Type"]));

		WulforUtil::drop_combo(dt->getWidget("comboboxentryAct"), actionsn);

		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("comboboxentry-entry22")), params["Action"].c_str());

		gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("ActRawDialog")));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;
		while (response == GTK_RESPONSE_OK)
		{
			params.clear();
			params["Name"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryName")));
			params["RAW"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryRaw")));
			params["ID"] = Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonId"))));
			params["Time"] = Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonTime"))));
			params["Type"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(dt->getWidget("comboboxType"))));
			params["Action"] = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(dt->getWidget("comboboxentryAct")));
			params["Enabled"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkEnabled"))) ? "1" : "0";
			params["tmpname"] = tmpname;

			if(params["Name"].empty())
			{
				return FALSE;
			}
			else
			{
				gtk_widget_hide(dt->getWidget("ActRawDialog"));
				return TRUE;
			}
		}
      gtk_widget_hide(dt->getWidget("ActRawDialog"));
	return FALSE;
}

void DetectionTab::addAct_gui(StringMap params)
{
		GtkTreeIter iter;
		int Id = Util::toInt(params["ID"]);

		if(findAct_gui(Id,&iter))
		{
			gtk_list_store_set(actionStore,&iter,
										actionView.col(N_("Name")),params["Name"].c_str(),
										actionView.col(N_("Enable")), params["Enabled"].c_str(),
										actionView.col(N_("ID")), /*Util::toInt(params["ID"])*/Id,
										-1);
		}
		else
		{
            gtk_list_store_append(actionStore,&iter);
            gtk_list_store_set(actionStore,&iter,
										actionView.col(N_("Name")),params["Name"].c_str(),
										actionView.col(N_("Enable")), params["Enabled"].c_str(),
										actionView.col(N_("ID")), /*Util::toInt(params["ID"])*/Id,
										-1);

			actions.insert(Prof::value_type(Id,iter));
			actionsn.insert(make_pair(params["Name"],Id));
	}
}

void DetectionTab::addRaw_gui(StringMap params)
{
    GtkTreeIter iter;
	int q = Util::toInt(params["ID"]);
	if(findRaw_gui(q,&iter))
    {
			gtk_list_store_set(RawStore,&iter,
                                    RawView.col(N_("Name")),params["Name"].c_str(),
									RawView.col(N_("Raw")), params["RAW"].c_str(),
									RawView.col(N_("Time")), Util::toInt(params["Time"]),
									RawView.col(N_("Enable")), params["Enabled"].c_str(),
									RawView.col(N_("ID")), q,
									-1);

	}
	else
	{
		gtk_list_store_append(RawStore,&iter);
		gtk_list_store_set(RawStore,&iter,
									RawView.col(N_("Name")), params["Name"].c_str(),
									RawView.col(N_("Raw")), params["RAW"].c_str(),
									RawView.col(N_("Time")), Util::toInt(params["Time"]),
									RawView.col(N_("Enable")), params["Enabled"].c_str(),
									RawView.col(N_("ID")), q,
									-1);
	}
}

void DetectionTab::removeAction_gui(string Id, string)
{
    GtkTreeIter iter;
	if(findAct_gui(Util::toInt(Id),&iter))
	{
	    gtk_list_store_remove(actionStore, &iter);
		actions.erase(Util::toInt(Id));
    }
}

void DetectionTab::removeRaw_gui(string Id, string )
{
    GtkTreeIter iter;
	if(findRaw_gui(Util::toInt(Id),&iter))
	{
	    gtk_list_store_remove(RawStore, &iter);
		raws.erase(Util::toInt(Id));
	}
}

void DetectionTab::addAct_client(StringMap params)
{
	RawManager::getInstance()->addAction(Util::toInt(params["ID"]),params["Name"],Util::toInt(params["Enabled"]));
	RawManager::getInstance()->saveActionRaws();
}

void DetectionTab::addRaw_client(StringMap params)
{
    Action* a = RawManager::getInstance()->findAction(params["Action"]);
	Raw raw;// = new Raw();
	raw.setName(params["Name"]);
	raw.setRaw(params["RAW"]);
	raw.setTime(Util::toInt(params["Time"]));
	raw.setEnabled(true);
	RawManager::getInstance()->addRaw(a,raw);
	RawManager::getInstance()->saveActionRaws();
}

void DetectionTab::editAct_client(StringMap params)
{
	{
		Action* action = RawManager::getInstance()->findAction(params["tmpname"]);
		if(action != NULL)
		{
			action->setId(Util::toInt(params["ID"]));
            action->setName(params["Name"]);
            action->setEnabled(Util::toInt(params["Enabled"]));
		}
	}
	RawManager::getInstance()->saveActionRaws();
}

void DetectionTab::editRaw_client(dcpp::StringMap params)
{
    string a = params["Action"];
    int ids = Util::toInt(params["ID"]);
    const Action *action = RawManager::getInstance()->findAction(a);
    if(action != NULL)
    {
        Raw *old = NULL;
        for(Action::RawsList::const_iterator j = action->raw.begin(); j != action->raw.end(); ++j) {
			if(j->getId() == ids)
            {
                old = const_cast<Raw*>(&(*j));
                break;
            }
        }
		if(old)
			RawManager::getInstance()->editRaw(&(*action),&(*old),(Raw(Util::toInt(params["ID"]),params["Name"],params["RAW"],Util::toInt(params["Time"]),Util::toInt(params["Enabled"]))));
    }
    RawManager::getInstance()->saveActionRaws();
}

void DetectionTab::removeAct_client(int id)
{
	Action* action = RawManager::getInstance()->findAction(id);
	bool rem = RawManager::getInstance()->remAction(action);
	if(rem)
		RawManager::getInstance()->saveActionRaws();
}

void DetectionTab::removeRaw_client(int id)
{
	Action::ActionList &list = RawManager::getInstance()->getActions();
	for(Action::ActionList::const_iterator it = list.begin(); it!= list.end(); ++it)
	{
	  Action *a = *it;
	  Action::RawsList *r = &(a->raw);
		for(Action::RawsList::const_iterator qt = r->begin(); qt!= r->end(); ++qt)
		{
			Raw *raws = const_cast<Raw*>(&(*qt));
			if(raws->getId() == id)
			{
				if(RawManager::getInstance()->remRaw(a,raws))
				{
					return;
				}
			}
		}
	}
	RawManager::getInstance()->saveActionRaws();
}

/* 2page */
void DetectionTab::onAddEntryDet(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	StringMap params;
	const string emptyStr = "";
	params["Name"] = "< Name >";
	params["ID"] = Util::toString(Util::rand(1, 2147483647));
	params["Cheat"] = emptyStr;
	params["Comment"] = emptyStr;
	params["RAW"] = "0";
	params["Flag"] = "0";
	params["MisMatch"] = "0";
	params["Enabled"] = "0";

	bool isOk = dt->showAddEntryDetDialog(params,dt);
	if(isOk)
	{
			typedef Func1<DetectionTab,StringMap> F1;
			F1 *func = new F1(dt,&DetectionTab::addEntryDet_client,params);
			WulforManager::get()->dispatchClientFunc(func);

			dt->addEntryDet_gui(params);

	}
}
void DetectionTab::ondModEntryDet(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected(dt->detectionSelection, NULL, &iter))
		return;

	StringMap params;
	params["Enabled"] = dt->detectionView.getValue<gboolean>(&iter,N_("Enable")) ? "1" : "0";
	params["Name"] = dt->detectionView.getString(&iter,N_("Name"));
	params["Cheat"] = dt->detectionView.getString(&iter,N_("Cheat"));
	params["Comment"] = dt->detectionView.getString(&iter,N_("Comment"));
	params["RAW"] = Util::toString(dt->detectionView.getValue<gint>(&iter,N_("Raw")));
	params["ID"] = Util::toString(dt->detectionView.getValue<gint>(&iter,N_("ID")));
	params["Flag"] = Util::toString(dt->detectionView.getValue<gint>(&iter,N_("Flag")));
	params["MisMatch"] = dt->detectionView.getValue<gboolean>(&iter,"MisMatch") ? "1" : "0";

	bool isOk = dt->showAddEntryDetDialog(params,dt);
	if(isOk)
	{
			dt->addEntryDet_gui(params);

			typedef Func2<DetectionTab,int,StringMap> F2;
			F2 *func = new F2(dt,&DetectionTab::editEntryDet_client, dcpp::Util::toInt(params["ID"]), params);
			WulforManager::get()->dispatchClientFunc(func);
	}
}


void DetectionTab::onRemoveEntryDet(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dt->detectionSelection, NULL, &iter))
	{
		string name = dt->detectionView.getString(&iter,N_("Name"));
		gint id = dt->detectionView.getValue<gint>(&iter, N_("ID"));

				if(SETTING(CONFIRM_HUB_REMOVAL))
				{
					GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
					GtkWidget* dialog = gtk_message_dialog_new(parent,
					GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					_("Are you sure you want to delete Entry \"%s\"?"), name.c_str());
					gtk_dialog_add_buttons(GTK_DIALOG(dialog), BMDC_STOCK_CANCEL, GTK_RESPONSE_CANCEL, BMDC_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);

					gint response = gtk_dialog_run(GTK_DIALOG(dialog));
					gtk_widget_destroy(dialog);

					if (response != GTK_RESPONSE_YES)
						return;
				}

		dt->removeEntryDet_gui((uint32_t)id);
		typedef Func1<DetectionTab,int> F1;
		F1 *func = new F1(dt,&DetectionTab::removeEntryDet_client,(int)id);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DetectionTab::removeEntryDet_gui(uint32_t _id)
{
    GtkTreeIter iter;
    if(findProf_gui(_id,&iter))
        gtk_list_store_remove(detectionStore,&iter);
}

void DetectionTab::addEntryDet_gui(dcpp::StringMap params)
{
	uint32_t id = Util::toUInt32(params["ID"]);
	GtkTreeIter iter;
	if(findProf_gui(id,&iter))
	{
			gtk_list_store_set(detectionStore,&iter,
								detectionView.col(N_("Enable")),Util::toInt(params["Enabled"]),
								detectionView.col(N_("Name")),params["Name"].c_str(),
								detectionView.col(N_("Cheat")),params["Cheat"].c_str(),
								detectionView.col(N_("Comment")),params["Comment"].c_str(),
								detectionView.col(N_("Raw")),Util::toInt(params["RAW"]),
								detectionView.col(N_("ID")),id,
								detectionView.col(N_("Flag")),Util::toInt(params["Flag"]),
								detectionView.col(N_("MisMatch")),Util::toInt(params["MisMatch"]),
									-1);

	}
	else
	{
				gtk_list_store_append(detectionStore,&iter);
				gtk_list_store_set(detectionStore,&iter,
								detectionView.col(N_("Enable")),Util::toInt(params["Enabled"]),
								detectionView.col(N_("Name")),params["Name"].c_str(),
								detectionView.col(N_("Cheat")),params["Cheat"].c_str(),
								detectionView.col(N_("Comment")),params["Comment"].c_str(),
								detectionView.col(N_("Raw")),Util::toInt(params["RAW"]),
								detectionView.col(N_("ID")),id,
								detectionView.col(N_("Flag")),Util::toInt(params["Flag"]),
								detectionView.col(N_("MisMatch")),Util::toInt(params["MisMatch"]),
									-1);
				profiles.insert(Prof::value_type(id,iter));
	}
}

void DetectionTab::addEntryDet_client(dcpp::StringMap params){

	DetectionEntry entry;
	entry.name = params["Name"];
	entry.cheat = params["Cheat"];
	entry.comment = params["Comment"];
	entry.Id = Util::rand(1, 2147483647);//hopfully random
	entry.rawToSend = Util::toUInt(params["RAW"]);
	entry.isEnabled = Util::toInt(params["Enabled"]);
	entry.clientFlag = Util::toUInt(params["Flag"]);
	entry.checkMismatch = Util::toInt(params["MisMatch"]);
	entry.defaultMap = map;
	entry.adcMap = mapadc;
	entry.nmdcMap = mapnmdc;

	DetectionManager::getInstance()->addDetectionItem(entry,false);
}

void DetectionTab::editEntryDet_client(int id,StringMap params)
{
	DetectionEntry entry;
	if(DetectionManager::getInstance()->getDetectionItem(id, entry, false))
	{
        entry.name = params["Name"];
        entry.cheat = params["Cheat"];
        entry.comment = params["Comment"];
        entry.Id = id;
        entry.rawToSend = Util::toUInt(params["RAW"]);
        entry.isEnabled = Util::toInt(params["Enabled"]);
        entry.clientFlag = Util::toUInt(params["Flag"]);
        entry.checkMismatch = Util::toInt(params["MisMatch"]);
        if(!map.empty())
		entry.defaultMap = map;
        if(!mapadc.empty())
            entry.adcMap = mapadc;
        if(!mapnmdc.empty())
            entry.nmdcMap = mapnmdc;

        DetectionManager::getInstance()->updateDetectionItem(id, entry, false);
	}
}

void DetectionTab::removeEntryDet_client(int id)
{
	DetectionManager::getInstance()->removeDetectionItem(id,false);
}

gboolean DetectionTab::onButtonItemPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	dt->previous = event->type;
	return FALSE;
}

gboolean DetectionTab::onButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	if (dt->previous == GDK_BUTTON_PRESS && event->button == 3)
	{
			dt->popupMenu_gui();
	}
	return TRUE;
}

void DetectionTab::popupMenu_gui()
{
	#if GTK_CHECK_VERSION(3,22,0)
		gtk_menu_popup_at_pointer(GTK_MENU(getWidget("menuItem")),NULL);
	#else
	gtk_menu_popup(GTK_MENU(getWidget("menuItem")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	#endif
}

void DetectionTab::onAddItemDlg_gui(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	StringMap params;
	params["Name"] = "";
	params["Value"] = "";
	params["Type"] = "0";
	bool isOk = dt->runDialogItem(params,dt);
	if(isOk)
	{
		dt->addMap_gui(params);
	}
}
void DetectionTab::onModItemDlg_gui(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->itemselection, NULL, &iter))
		return;
	StringMap params;
	params["Name"] = dt->item.getString(&iter, N_("Field"));
	params["Value"] = dt->item.getString(&iter, N_("Value"));
	string iftype = dt->item.getString(&iter, N_("Hub"));
	if( iftype == "NMDC")
			params["Type"] = "0";
	else if(iftype == "ADC")
			params["Type"] = "1";
	else params["Type"] = "2";

	bool isOk = dt->runDialogItem(params,dt);
	if(isOk)
	{
		dt->editMap_gui(params,&iter);
	}
}

void DetectionTab::onRemItemDlg_gui(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->itemselection, NULL, &iter))
		return;

	if (SETTING(CONFIRM_USER_REMOVAL))
	{
		GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_NONE,
					_("Are you sure you want to delete item?"));
				gtk_dialog_add_buttons(GTK_DIALOG(dialog), BMDC_STOCK_CANCEL, GTK_RESPONSE_CANCEL, BMDC_STOCK_REMOVE,
				GTK_RESPONSE_YES, NULL);

				gint response = gtk_dialog_run(GTK_DIALOG(dialog));
				// Widget failed if the dialog gets programmatically destroyed.
				if (response == GTK_RESPONSE_NONE)
					return;

				gtk_widget_hide(dialog);

				if (response != GTK_RESPONSE_YES)
					return;
	}

				gtk_list_store_remove(dt->itemstore, &iter);
}

void DetectionTab::onToggleDet(GtkCellRendererToggle*, gchar *path, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;
    if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(dt->detectionStore), &iter, path))
    {
		gint id = dt->detectionView.getValue<gint>(&iter, _("ID"));
		bool fixed = dt->detectionView.getValue<gboolean>(&iter, _("Enable"));
		fixed = !fixed;
		gtk_list_store_set(dt->detectionStore, &iter, dt->detectionView.col("Enable"),fixed,-1);
	     DetectionManager::getInstance()->setItemEnabled(id,fixed,false);
    }
}

bool DetectionTab::runDialogItem(StringMap &params,DetectionTab *dt)
{
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryNamei1")), params["Name"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryValuei2")), params["Value"].c_str());
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxt1")), (gint)(Util::toInt(params["Type"])));
		gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogItem1")));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;

		while(response == GTK_RESPONSE_OK)
		{
			params.clear();
			params["Name"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryNamei1")));
			params["Value"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryValuei2")));
			string iftype = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(dt->getWidget("comboboxt1"))));
			if(iftype == "0")
				params["Type"] = "NMDC";
			else if(iftype =="1")
				params["Type"] = "ADC";
			else params["Type"] = "BOTH";

			gtk_widget_hide(dt->getWidget("dialogItem1"));
			return TRUE;
		}
	gtk_widget_hide(dt->getWidget("dialogItem1"));
	return FALSE;
}

void DetectionTab::addMap_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_list_store_append(itemstore, &iter);
	editMap_gui(params, &iter);
}
void DetectionTab::editMap_gui(StringMap &params,GtkTreeIter *iter)
{
	gtk_list_store_set(itemstore, iter,
	item.col(N_("Field")), params["Name"].c_str(),
	item.col(N_("Value")), params["Value"].c_str(),
	item.col(N_("Hub")), params["Type"].c_str(),
	-1);
}

bool DetectionTab::showAddEntryDetDialog(StringMap &params, DetectionTab *dt)
{
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entry1Name")), params["Name"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entry2Cheat")), params["Cheat"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryComments")), params["Comment"].c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttondetId")),(gdouble)Util::toDouble(params["ID"]));
		gtk_widget_set_sensitive(dt->getWidget("spinbuttondetId"),FALSE);
		// Set the Enabled checkbox
		gboolean enabled = (((params["Enabled"] == "1") ? TRUE : FALSE) || params["Enabled"].empty());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton1Enabled")), enabled);
		// Set the Enabled checkbox
		gboolean mismatch = (((params["MisMatch"] == "1") ? TRUE : FALSE) || params["MisMatch"].empty());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbuttonCheckMis")), mismatch);
		///Set Action
		auto act = WulforUtil::getActions();
		dt->set_combo(dt->getWidget("comboboxentry1Act"),act,Util::toInt(params["RAW"]),true,dt);
		//Flag
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxFlag")), Util::toInt(params["Flag"]));
		/*InfMap */
		DetectionEntry e;
		bool u = DetectionManager::getInstance()->getDetectionItem(Util::toUInt(params["ID"]), e, false);
		if(u)
		{
			gtk_list_store_clear(itemstore);
			for(dcpp::DetectionEntry::INFMap::const_iterator i = e.defaultMap.begin(); i!=e.defaultMap.end(); ++i)
			{
				StringMap p;
				p["Name"] = i->first;
				p["Value"]= i->second;
				p["Type"] = "Both";
				dt->addMap_gui(p);
				map.push_back(make_pair(i->first, i->second));
			}

			for(dcpp::DetectionEntry::INFMap::const_iterator i = e.adcMap.begin(); i!= e.adcMap.end(); ++i)
			{
				StringMap q;
				q["Name"] = i->first;
				q["Value"]= i->second;
				q["Type"] = "ADC";
				dt->addMap_gui(q);
				mapadc.push_back(make_pair(i->first, i->second));
			}

			for(dcpp::DetectionEntry::INFMap::const_iterator i = e.nmdcMap.begin(); i!=e.nmdcMap.end(); ++i)
			{
				StringMap d;
				d["Name"] = i->first;
				d["Value"]= i->second;
				d["Type"] = "NMDC";
				dt->addMap_gui(d);
				mapnmdc.push_back(make_pair(i->first,i->second));
			}
		}
		//thinking ...
		if(mapnmdc.empty() && mapadc.empty() && map.empty())
		{
			StringMap q;
			q["Name"] = "SU";
			q["Value"] = "^BanMsg$";
			q["Type"] = "NMDC";
			dt->addMap_gui(q);

		}

		gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogDetection")));

		// Fix crash, if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return FALSE;
		while(response == GTK_RESPONSE_OK)
		{
			params.clear();
			params["Name"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entry1Name")));
			params["Cheat"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entry2Cheat")));
			params["Comment"] = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryComments")));
			params["ID"] = Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttondetId"))));
			params["Enabled"] = Util::toString(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton1Enabled"))));
			params["MisMatch"] = Util::toString(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbuttonCheckMis"))));
			params["Flag"] = Util::toString(gtk_combo_box_get_active(GTK_COMBO_BOX(dt->getWidget("comboboxFlag"))));
			params["RAW"] = Util::toString(dt->save_combo(dt->getWidget("comboboxentry1Act"))); // Util::toString(find_rawInt(gtk_combo_box_get_active(GTK_COMBO_BOX(dt->getWidget("comboboxentry1Act")))));

			GtkTreeIter iter;
			GtkTreeModel *tmodel = GTK_TREE_MODEL(dt->itemstore);
			gboolean valid = gtk_tree_model_get_iter_first(tmodel, &iter);
			dt->map.clear();
			dt->mapadc.clear();
			dt->mapnmdc.clear();

				while(valid)
				{
					string a = dt->item.getString(&iter, N_("Field"));
					string b = dt->item.getString(&iter, N_("Value"));
					string hub = dt->item.getString(&iter,N_("Hub"));
					if ( hub == "ADC" )
					{
						dt->mapadc.push_back(make_pair(a,b));
					}
					else if ( hub == "NMDC")
					{
						dt->mapnmdc.push_back(make_pair(a,b));
					}
					else if ( hub == "BOTH")
					{
						dt->map.push_back(make_pair(a,b));
					}
					else g_print("dont go here");

					valid = gtk_tree_model_iter_next(tmodel, &iter);

				}
				if(params["Name"].empty() && (dt->map.empty() || dt->mapadc.empty() || dt->mapnmdc.empty()) && params["ID"].empty())
                {
                        if(showErrorDialog_gui(_("Fields Name, ID and detection fields are required"),dt))
                        {
                            response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogDetection")));
                            // Fix crash, if the dialog gets programmatically destroyed.
                            if (response == GTK_RESPONSE_NONE)
                                return FALSE;
                        }
                        else return FALSE;
                }
                else
                {
                    gtk_widget_hide(dt->getWidget("dialogDetection"));
                    return TRUE;
                }
		}
		gtk_widget_hide(dt->getWidget("dialogDetection"));
		return FALSE;
}

/* Find func*/
bool DetectionTab::findAct_gui(const int &Id, GtkTreeIter *iter)
{
	ActRaw::const_iterator it = actions.find(Id);

	if (it != actions.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}
bool DetectionTab::findRaw_gui(const int &Id, GtkTreeIter *iter)
{
	ActRaw::const_iterator it = raws.find(Id);

	if (it != raws.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}

/*Util func*/
int DetectionTab::find_raw(string rawString)
{
	auto act = WulforUtil::getActions();
	for (auto it = act.begin(); it != act.end(); ++it)
	{
		if(it->first == rawString)
			return it->second;
	}
  return 0;
}
int DetectionTab::find_rawInt(int raw)
{
	int _raw = 0;
	int i = 0;
	auto act = WulforUtil::getActions();
	for (auto it = act.begin(); it != act.end(); ++it,i++)
	{
		if(it->second == raw)
		{
            _raw = i-1;
            break;
		}
	}
  return _raw;
}
bool DetectionTab::findProf_gui(const uint32_t &id, GtkTreeIter *iter)
{
	Prof::const_iterator it = profiles.find(id);

	if (it != profiles.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}
	return FALSE;
}
/*3d page*/
void DetectionTab::addPoints_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_list_store_append(pointstore, &iter);
	editPoints_gui(params, &iter);
}

void DetectionTab::editPoints_gui(StringMap& params,GtkTreeIter *iter)
{
	gtk_list_store_set(pointstore, iter,
	points.col(N_("Points")),Util::toInt(params["Points"]),
	points.col(N_("Action")),Util::toInt(params["Action"]),
	-1);
}

void DetectionTab::onADLPoints(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	bool isOk = dt->showADLPoints(dt);
	if(isOk)
		CalcADLAction::getInstance()->updateADLPoints(dt->imap);
}

gboolean DetectionTab::showADLPoints(DetectionTab *dt)
{
	gtk_list_store_clear(dt->pointstore);
	const IntMap& list = CalcADLAction::getInstance()->getADLPoints();
	for(IntMap::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		StringMap params;
		params["Points"] = Util::toString(i->first);
		params["Action"] = Util::toString(i->second);
		dt->addPoints_gui(params);

	}

	gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogitemPoints")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
			return FALSE;
	if(response == GTK_RESPONSE_OK)
	{
		GtkTreeIter iter;
		GtkTreeModel *tmodel= GTK_TREE_MODEL(dt->pointstore);
		gboolean valid = gtk_tree_model_get_iter_first(tmodel, &iter);
		while(valid)
		{
			int a = dt->points.getValue<gint>(&iter, _("Points"));
			int b = dt->points.getValue<gint>(&iter, _("Action"));

			dt->imap.insert(make_pair(a,dt->find_raw(Util::toString(b))));

			valid = gtk_tree_model_iter_next(tmodel, &iter);
		}

		gtk_widget_hide(dt->getWidget("dialogitemPoints"));
		return TRUE;
	}
	gtk_widget_hide(dt->getWidget("dialogitemPoints"));
	return FALSE;
}

bool DetectionTab::showAddPointsDialog(StringMap &params,DetectionTab *dt)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonpointss1")),(gdouble)(Util::toInt(params["Points"])));
	dt->set_combo(dt->getWidget("comboboxentryactionp1"), WulforUtil::getActions(), (int)Util::toInt(params["Action"]), false, dt);

	gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogpointitem1")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
			return FALSE;
	if(response == GTK_RESPONSE_OK)
	{
		params["Action"] = Util::toString(dt->save_combo(dt->getWidget("comboboxentryactionp1")));//gtk_combo_box_get_active(GTK_COMBO_BOX(dt->getWidget("comboboxentryactionp1")));
		params["Points"] = Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonpointss1"))));
		gtk_widget_hide(dt->getWidget("dialogpointitem1"));
		return TRUE;
	}
	return FALSE;
}

void DetectionTab::onADSLPointsADD(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	StringMap params;
	params["Points"] = "0";
	params["Action"] = "0";
	bool isOk = dt->showAddPointsDialog(params,dt);
	if(isOk)
	{
		dt->addPoints_gui(params);
	}
}

void DetectionTab::onADSLPointsMOD(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	StringMap params;
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->pointselect, NULL, &iter))
		return;

	params["Points"] = Util::toString(dt->points.getValue<gint>(&iter,N_("Points")));
	params["Action"] = Util::toString(dt->points.getValue<gint>(&iter,N_("Action")));
	bool isOk = dt->showAddPointsDialog(params,dt);
	if(isOk)
	{
		dt->editPoints_gui(params, &iter);
	}
}
void DetectionTab::onADSLPointsDEL(GtkWidget*, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->pointselect, NULL, &iter))
		return;

	 if (SETTING(CONFIRM_USER_REMOVAL))
	 {
			GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_NONE,
						_("Are you sure you want to delete item?"));
					gtk_dialog_add_buttons(GTK_DIALOG(dialog), BMDC_STOCK_CANCEL, GTK_RESPONSE_CANCEL, BMDC_STOCK_REMOVE,
					GTK_RESPONSE_YES, NULL);

					gint response = gtk_dialog_run(GTK_DIALOG(dialog));
					// Widget failed if the dialog gets programmatically destroyed.
					if (response == GTK_RESPONSE_NONE)
						return;

					gtk_widget_hide(dialog);

					if (response != GTK_RESPONSE_YES)
						return;
		}

	gtk_list_store_remove(dt->pointstore, &iter);
}
/**/
void DetectionTab::onSave(GtkWidget* , gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	SettingsManager *st = SettingsManager::getInstance();
	///Fake
	bool afake = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkFake1")));
	st->set(SettingsManager::SHOW_FAKESHARE_RAW,afake);
	st->set(SettingsManager::FAKESHARE_RAW,Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryFake")))));
	///RMDC
	bool armdc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkRmdc1")));
	st->set(SettingsManager::SHOW_RMDC,armdc);
	st->set(SettingsManager::RMDC_RAW, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryRM")))));

    //EMulDC
	bool aemul = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkemul1")));
	st->set(SettingsManager::SHOW_DCPP_EMULATION,aemul);
	st->set(SettingsManager::DCPP_EMULATION_RAW, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryEM")))));

	//FLMM
	bool aflmismatch = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkmismatch4")));
	st->set(SettingsManager::SHOW_FILELIST_VERSION_MISMATCH,aflmismatch);

	st->set(SettingsManager::FILELIST_VERSION_MISMATCH, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entrymis")))));
	///LLM
	st->set(SettingsManager::LISTLEN_MISMATCH, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entrylis")))));
	///VMM
	st->set(SettingsManager::VERSION_MISMATCH, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryvermis")))));
	///DISON
	bool adis =gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkdiscon7")));
	st->set(SettingsManager::SHOW_DISCONNECT,adis);

	st->set(SettingsManager::DISCONNECT_RAW,Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entrydis")))));
	///SMBIG
	bool asmbig = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton1")));
	st->set(SettingsManager::FILELIST_TOO_SMALL_BIG,asmbig);

    st->set(SettingsManager::FILELIST_TOO_SMALL_BIG_RAW, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entrytbsfl")))));

    st->set(SettingsManager::ADL_RAW, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryadl")))));
	///SDL
	bool asdl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton2slw")));
	st->set(SettingsManager::USE_SDL_KICK,asdl);

    st->set(SettingsManager::SDL_RAW, Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryspeed")))));
	///Show Cheat
	bool showcheat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkShowCheat")));
	st->set(SettingsManager::DISPLAY_CHEATS_IN_MAIN_CHAT,showcheat);
	///ADLD
	bool showadla = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkADLACtionShow")));
	st->set(SettingsManager::SHOW_ADLSEARCH_DEFAULT_ACTION,showadla);

	st->set(SettingsManager::ADLSEARCH_DEFAULT_ACTION,Util::toInt(gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryadl")))));
	///MINFLSIZE
	int minflsize = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(dt->getWidget("spinFLMINSIZE")));
	st->set(SettingsManager::MIN_FL_SIZE,minflsize);
	///DISCON
	int discon = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spinMaxDiscon")));
	st->set(SettingsManager::MAX_DISCONNECTS,discon);
	///TestSUR
	int maxtestsur = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spintestsur")));
	st->set(SettingsManager::MAX_TESTSURS, maxtestsur);
	///Max FL
	int maxfilelist = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spinFileLists")));
	st->set(SettingsManager::MAX_FILELISTS, maxfilelist);
	///ChecCLBFFL
	bool checkbeforefl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkCLBFFL")));
	st->set(SettingsManager::CHECK_ALL_CLIENTS_BEFORE_FILELISTS,checkbeforefl);
	///SleepTime
	int sleept = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spinsleep")));
	st->set(SettingsManager::SLEEP_TIME, sleept);
	///Min Points To Show
	int points = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spinADLPointsToDisplay")));
	st->set(SettingsManager::MIN_POINTS_TO_DISPLAY_CHEAT, points);
	///use delay
	bool usedelay = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkusedelay")));
	st->set(SettingsManager::USE_SEND_DELAYED_RAW, usedelay);
	///percent fake
	int perfake = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spinPerFake")));
	st->set(SettingsManager::PERCENT_FAKE_SHARE_TOLERATED, perfake);
	///sdl speed
	int sdlspeed = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spinspeedsd")));
	st->set(SettingsManager::SDL_SPEED, sdlspeed);
	///sdl time
	int sdltime = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dt->getWidget("spintimesd")));
	st->set(SettingsManager::SDL_TIME, sdltime);
	///Protect U
    const gchar * protectuser = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryProtectPatern")));
    st->set(SettingsManager::PROTECTED_USERS, protectuser);

	///Save
	st->save();
}

bool DetectionTab::showErrorDialog_gui(const string &description, DetectionTab *dt)
{
	GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(dt->getWidget("dialogDetection")),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;

	gtk_widget_destroy(dialog);

	return TRUE;
}

void DetectionTab::onSelectAction(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GtkWidget *entry = (GtkWidget *)g_object_get_data(G_OBJECT(widget), "wid");
	GtkTreeIter iter;
	dt->initDialogWithValues();
	gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogActionsSelection")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	if( response == GTK_RESPONSE_OK)
	{
		 if(gtk_tree_selection_count_selected_rows(dt->actionSelectionRR) == 1)
		{
			gtk_tree_selection_get_selected(dt->actionSelectionRR, NULL,&iter);
			gint n = dt->actionsSelectionsR.getValue<gint>(&iter, "Number");
			gtk_entry_set_text(GTK_ENTRY(entry), Util::toString(n).c_str());
		}
		gtk_widget_hide(dt->getWidget("dialogActionsSelection"));
		return;
	}
	gtk_widget_hide(dt->getWidget("dialogActionsSelection"));
}

void DetectionTab::initDialogWithValues()
{
	auto act = WulforUtil::getActions();
	GtkTreeIter iter;
	gtk_list_store_clear(actionsSelectionsRStore);
	for(auto it = act.begin();it!= act.end();++it)
    {
       gtk_list_store_append( actionsSelectionsRStore, &iter );
        gtk_list_store_set( actionsSelectionsRStore, &iter,
                            actionsSelectionsR.col("Number") ,it->second,
                            actionsSelectionsR.col("Name") , it->first.c_str(),
                            -1 );
    }

}

void DetectionTab::set_combo(GtkWidget *place, std::map<std::string,int> act, int set, bool det, gpointer data)
{
    DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
    GtkListStore *store;
    GtkComboBox *box;
    GtkTreeIter iter,*piter = NULL;
    GtkCellRenderer *cells,*celli;
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(auto it = act.begin();it!= act.end();++it)
    {
       gtk_list_store_append( store, &iter );
        gtk_list_store_set( store, &iter,
                            TYPE_STR, it->first.c_str(),
                            TYPE_INT, it->second,
                            -1 );
        if(it->second == set)
            piter = gtk_tree_iter_copy(&iter);

        dt->tmpname.insert ( pair<int,std::string>(it->second,it->first));

    }

    box = GTK_COMBO_BOX(gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) ));
    g_object_unref(G_OBJECT(store));
    cells = gtk_cell_renderer_text_new();
    celli = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( box ), cells, TRUE );
    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( box ), celli, TRUE );
    gtk_cell_renderer_set_visible(celli,FALSE);

    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( box ), cells, "text", TYPE_STR, NULL );
    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( box ), celli, "text", TYPE_INT, NULL );

    GList *list = gtk_container_get_children(GTK_CONTAINER(place));

    if( (det && list != NULL) && GTK_IS_COMBO_BOX(GTK_WIDGET(g_list_first(list)->data)))
            return;

    gtk_box_pack_start(GTK_BOX(place), GTK_WIDGET(box), FALSE, TRUE, 0);

	gtk_widget_show_all(GTK_WIDGET(box));

	gtk_combo_box_set_active_iter(box,piter);

	gtk_widget_set_sensitive(GTK_WIDGET(box),TRUE);
}

int DetectionTab::save_combo(GtkWidget *widget)
{
    GList * list;
    list = gtk_container_get_children(GTK_CONTAINER(widget));
    GtkWidget *combo = GTK_WIDGET(g_list_first(list)->data);
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *str = NULL;
    gint ints = 0;
    if(!GTK_IS_COMBO_BOX(combo))
            return 0;

    if( gtk_combo_box_get_active_iter( GTK_COMBO_BOX(combo), &iter ) )
    {
        /* Obtain data model from combo box. */
        model = gtk_combo_box_get_model( GTK_COMBO_BOX(combo) );
       /* Obtain string from model. */
        gtk_tree_model_get( model, &iter, TYPE_STR, &str, TYPE_INT, &ints , -1 );
    }
    return ints;
}

void DetectionTab::loadAgain(GtkWidget *widget, vector<pair<string,int> > act, int set, gpointer data)
{
	DetectionTab *dt = reinterpret_cast<DetectionTab *>(data);
	GList *list;
	list = gtk_container_get_children(GTK_CONTAINER(widget));
	GtkWidget *combo = GTK_WIDGET(g_list_first(list)->data);
	GtkTreeIter iter,*piter = NULL;
	GtkTreeModel *model;
	if(!GTK_IS_COMBO_BOX(combo))
		return;
	std::map<int,string> tmpmaps;

	for(vector<pair<string,int> >::const_iterator q = act.begin();q!=act.end();++q)
			tmpmaps.insert( pair<int, std::string>(q->second,q->first) );

	/* Obtain data model from combo box. */
    model = gtk_combo_box_get_model( GTK_COMBO_BOX(combo) );
    if(gtk_tree_model_get_iter_first(model,&iter))
    {
		GtkListStore *store = GTK_LIST_STORE(model);
		if(dt->tmpname.size() != tmpmaps.size())
		{
			gtk_list_store_clear(store);
			for ( std::map<int,std::string>::iterator it=(tmpmaps.begin()); it != (tmpmaps.end()); it++ )
			{
				gtk_list_store_append( store, &iter );
				gtk_list_store_set( store, &iter,
                            TYPE_STR, it->second.c_str(),
                            TYPE_INT, it->first,
                            -1 );
                  if(it->first == set)
						piter = gtk_tree_iter_copy(&iter);

			}
		}

	}
	 gtk_combo_box_set_model (GTK_COMBO_BOX(combo), model);
	 gtk_widget_show_all(combo);

 	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),piter);

 	gtk_widget_set_sensitive(GTK_WIDGET(combo), TRUE);

}
