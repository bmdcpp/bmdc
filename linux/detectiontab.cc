#include "detectiontab.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

DetectionTab::DetectionTab():
BookEntry(Entry::DET,_("Detection"),"detection.glade")
{

	gtk_window_set_transient_for(GTK_WINDOW(getWidget("ActRawDialog")), GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(getWidget("ActRawDialog")), TRUE);

	//DetectionClients
	cldet = getWidget("treeviewProf");
	prselection	= gtk_tree_view_get_selection(GTK_TREE_VIEW(cldet));
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(cldet), TRUE);
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(cldet), TRUE);
	/*Enable*/
	coldenb = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldenb, "Enable");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldenb);
	/*Name*/
	coldname = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldname, "Name");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldname);
	/*Cheat*/
	coldcheat = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldcheat, "Cheat");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldcheat);
	/*Comment*/
	coldcom = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldcom, "Comment");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldcom);
	/*Raw*/
	coldraw = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldraw, "Raw");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldraw);
	/*Id*/
	coldid = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldid, "ID");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldid);
	/*Flag*/
	coldflag = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldflag, "Flag");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldflag);
	coldmmf = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coldmmf, "MisMatch");
	gtk_tree_view_append_column(GTK_TREE_VIEW(cldet), coldmmf);
	/*Renders*/
	/*enb*/
	rendenb = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(coldenb, rendenb, TRUE);
	gtk_tree_view_column_add_attribute(coldenb, rendenb,
	"active", COLUMNS);
	/*Name*/
	rendname = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldname, rendname, TRUE);
	gtk_tree_view_column_add_attribute(coldname, rendname,
	"text", NAME);
	/*Cheat*/
	rendcheat = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldcheat, rendcheat, TRUE);
	gtk_tree_view_column_add_attribute(coldcheat, rendcheat,
	"text", CHEAT);
	/*Comment*/
	rendcom = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldcom, rendcom, TRUE);
	gtk_tree_view_column_add_attribute(coldcom, rendcom,
	"text", COMMENT);
	/*Raw*/
	rendraw = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldraw, rendraw, TRUE);
	gtk_tree_view_column_add_attribute(coldraw, rendraw,
	"text", RAWS);
	/*ID*/
	rendid = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldid, rendid, TRUE);
	gtk_tree_view_column_add_attribute(coldid, rendid,
	"text", IDS);
	/*flag*/
	rendflag = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldflag, rendflag, TRUE);
	gtk_tree_view_column_add_attribute(coldflag, rendflag,
	"text", FLAG);
	/*MisMatch*/
	rendmmf = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coldmmf, rendmmf, TRUE);
	gtk_tree_view_column_add_attribute(coldmmf, rendmmf,
	"text", MISMATCH);


	//Resizable headers
	gtk_tree_view_column_set_resizable(coldname, TRUE);
	gtk_tree_view_column_set_resizable(coldraw, TRUE);
	gtk_tree_view_column_set_resizable(coldcheat, TRUE);
	gtk_tree_view_column_set_resizable(coldcom, TRUE);
	gtk_tree_view_column_set_resizable(coldid, TRUE);

	models = create_profiles(this);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cldet), models);


	//Action&&Raw
	araw = getWidget("treeviewRaw");
	aselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(araw));
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(araw), TRUE);
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(araw), TRUE);
	///Name
	colname = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(colname, "Name");
	gtk_tree_view_append_column(GTK_TREE_VIEW(araw), colname);
	///Raw
	colraw = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(colraw, "Raw");
	gtk_tree_view_append_column(GTK_TREE_VIEW(araw), colraw);
	///Time
	coltime = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(coltime, "Time");
	gtk_tree_view_append_column(GTK_TREE_VIEW(araw), coltime);
	///Enabled
	colenb = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(colenb, "Enabled");
	gtk_tree_view_append_column(GTK_TREE_VIEW(araw), colenb);
	///ID
	colid = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(colid, "ID");
	gtk_tree_view_append_column(GTK_TREE_VIEW(araw), colid);

	///Renders of colums
	///Name
  	renname = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(colname, renname, TRUE);
	gtk_tree_view_column_add_attribute(colname, renname,
	"text", COLUMN);
	///Raw
	renraw = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(colraw, renraw, TRUE);
	gtk_tree_view_column_add_attribute(colraw, renraw,
	"text", RAW);
	///Time
	rentime = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(coltime, rentime, TRUE);
	gtk_tree_view_column_add_attribute(coltime, rentime,
	"text", TIME);
	///Enabled //Todo Checkbox ???
	renenb = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(colenb, renenb, TRUE);
	gtk_tree_view_column_add_attribute(colenb, renenb,
	"text", ENABLED);
	///ID
	renid = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(colid, renid, TRUE);
	gtk_tree_view_column_add_attribute(colid, renid,
	"text", ID);

	///Resizable headers
	gtk_tree_view_column_set_resizable(colname, TRUE);
	gtk_tree_view_column_set_resizable(colraw, TRUE);
	gtk_tree_view_column_set_resizable(coltime, TRUE);
	gtk_tree_view_column_set_resizable(colenb, TRUE);
	///Sortable
	///and Max Width
	///Name
	gtk_tree_view_column_set_sort_column_id(colname, COLUMN);
	gtk_tree_view_column_set_sort_indicator(colname, TRUE);
	gtk_tree_view_column_set_max_width(colname,150);
	///Raw
	gtk_tree_view_column_set_sort_column_id(colraw, RAW);
	gtk_tree_view_column_set_sort_indicator(colraw, TRUE);
	gtk_tree_view_column_set_max_width(colraw,150);
	///Time
	gtk_tree_view_column_set_sort_column_id(coltime, TIME);
	gtk_tree_view_column_set_sort_indicator(coltime, TRUE);
	gtk_tree_view_column_set_max_width(coltime,150);
	///Enabled//Check
	gtk_tree_view_column_set_sort_column_id(colenb, COLUMN);
	gtk_tree_view_column_set_sort_indicator(colenb, TRUE);
	gtk_tree_view_column_set_max_width(colenb,150);

	model = create_model(this);
	gtk_tree_view_set_model(GTK_TREE_VIEW(araw), model);

	/* connect to signals */
	/*1page*/
	g_signal_connect(getWidget("button1"), "clicked", G_CALLBACK(onAddActRaw),(gpointer)this);//add
	g_signal_connect(getWidget("button2"), "clicked", G_CALLBACK(onEditActRaw), (gpointer)this);//edit
	g_signal_connect(getWidget("button3"), "clicked", G_CALLBACK(onRemoveActRaw), (gpointer)this);//remove
	/*2page*/
	g_signal_connect(getWidget("button4"), "clicked", G_CALLBACK(onAddEntryDet), (gpointer)this);//add
	g_signal_connect(getWidget("button5"), "clicked", G_CALLBACK(ondModEntryDet), (gpointer)this);//edit
	g_signal_connect(getWidget("button6"), "clicked", G_CALLBACK(onRemoveEntryDet), (gpointer)this);//remove
	g_signal_connect(rendenb, "toggled", G_CALLBACK(onToggleDet) , models);//toggle but


	g_object_unref(model); //1page
	g_object_unref(models); //2page

	item.setView(GTK_TREE_VIEW(getWidget("treeview1")));
	item.insertColumn(_("Field"), G_TYPE_STRING, TreeView::STRING,100);
	item.insertColumn(_("Value"), G_TYPE_STRING, TreeView::STRING,100);
	item.insertColumn(_("Hub"),  G_TYPE_STRING, TreeView::STRING,50);
	item.finalize();

	itemstore = gtk_list_store_newv(item.getColCount(), item.getGTypes());
	gtk_tree_view_set_model(item.get(), GTK_TREE_MODEL(itemstore));
	g_object_unref(itemstore);
	itemselection = gtk_tree_view_get_selection(item.get());

	g_signal_connect(item.get(),"button-press-event",G_CALLBACK(onButtonItemPressed_gui), (gpointer)this);
	g_signal_connect(item.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("additem"), "activate", G_CALLBACK(onAddItemDlg_gui), (gpointer)this);
	g_signal_connect(getWidget("changeitem"), "activate", G_CALLBACK(onModItemDlg_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemItemDlg_gui), (gpointer)this);

	vector<std::pair<std::string,int> >&act = WulforUtil::getActions();
	WulforUtil::drop_combo(getWidget("comboboxentry1Fake"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1rmdc"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1emul"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1mis"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1listlen"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1vermis"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1disc"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1BigSmall"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1slwsp"),act);
	WulforUtil::drop_combo(getWidget("comboboxentry1ADLA"),act);
	/**/
	WulforUtil::drop_combo(getWidget("comboboxentryactionp1"),act);
	/**/

	g_signal_connect(getWidget("button7Save"), "clicked", G_CALLBACK(onSave), (gpointer)this);
	g_signal_connect(getWidget("buttonadlsp7"), "clicked", G_CALLBACK(onADSLPoints), (gpointer)this);
	
	points.setView(GTK_TREE_VIEW(getWidget("treeviewPoints")));
	points.insertColumn("Points", G_TYPE_STRING, TreeView::STRING,100);
	points.insertColumn("Action", G_TYPE_STRING, TreeView::STRING,100);
	points.finalize();

	pointstore =gtk_list_store_newv(points.getColCount(), points.getGTypes());
	gtk_tree_view_set_model(points.get(), GTK_TREE_MODEL(pointstore));
	g_object_unref(pointstore);
	pointselect = gtk_tree_view_get_selection(points.get());

	g_signal_connect(getWidget("buttonp1"), "clicked", G_CALLBACK(onADSLPointsADD), (gpointer)this);
	g_signal_connect(getWidget("buttonp2"), "clicked", G_CALLBACK(onADSLPointsMOD), (gpointer)this);
	g_signal_connect(getWidget("buttonp3"), "clicked", G_CALLBACK(onADSLPointsDEL), (gpointer)this);

}

DetectionTab::~DetectionTab() { }

void DetectionTab::show() {

	///Fake
	gboolean afake = BOOLSETTING(SHOW_FAKESHARE_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkFake1")),afake);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1Fake")), (gint)(find_rawInt(SETTING(FAKESHARE_RAW)) ));
	///RMDC
	gboolean rmdc = BOOLSETTING(SHOW_RMDC_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkRmdc1")),rmdc);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1rmdc")), (gint)(find_rawInt(SETTING(RMDC_RAW)) ));
	///EMULation
	gboolean emul = BOOLSETTING(SHOW_DCPP_EMULATION_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkemul1")),emul);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1emul")), (gint)(find_rawInt(SETTING(DCPP_EMULATION_RAW)) ));
	///FLVMISMATCH
	gboolean ver= BOOLSETTING(SHOW_FILELIST_VERSION_MISMATCH) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkVmismatch1")),ver);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1mis")), (gint)(find_rawInt(SETTING(FILELIST_VERSION_MISMATCH))));
	///ListlenMMatch
	gboolean listlen = BOOLSETTING(LISTLEN_MISMATCH_SHOW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checklistlen")),listlen);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1listlen")), (gint)(find_rawInt(SETTING(LISTLEN_MISMATCH))));
	///VerMisMatch
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1vermis")), (gint)(find_rawInt(SETTING(VERSION_MISMATCH))));
	///Diconnect
	gboolean disc = BOOLSETTING(SHOW_DISCONNECT_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkdiscon7")),disc);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1disc")), (gint)(find_rawInt(SETTING(DISCONNECT_RAW))));
	///FLTOBIGSMAL
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1BigSmall")), (gint)(find_rawInt(SETTING(FILELIST_TOO_SMALL_BIG_RAW))));
	///SLSP
	gboolean useslw =BOOLSETTING(USE_SDL_KICK) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton2slw")),useslw);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1slwsp")),(gint)(find_rawInt(SETTING(SDL_RAW))));
	///Show Cheats
	gboolean showcheat = BOOLSETTING(DISPLAY_CHEATS_IN_MAIN_CHAT) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkShowCheat")),showcheat);
	///ADLA
	gboolean showadla = BOOLSETTING(SHOW_ADLSEARCH_DEFAULT_ACTION) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkADLACtionShow")),showadla);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxentry1ADLA")), (gint)(find_rawInt(SETTING(ADLSEARCH_DEFAULT_ACTION))));
	///Min FL SIZE
	int minflsize = SETTING(MIN_FL_SIZE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinFLMINSIZE")),(gdouble)minflsize);
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
	gboolean chclbeffl = BOOLSETTING(CHECK_ALL_CLIENTS_BEFORE_FILELISTS) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkCLBFFL")),chclbeffl);
	///SleepTime
	int sleept = SETTING(SLEEP_TIME);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinsleep")), (gdouble)sleept);
	///Min ADLPoints To Display
	int points = SETTING(MIN_POINTS_TO_DISPLAY_CHEAT);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinADLPointsToDisplay")), (gdouble)points);
	///delay raws
	gboolean usedelayraw = SETTING(USE_SEND_DELAYED_RAW) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkusedelay")),usedelayraw);
	///percent fake
	int percentfake = SETTING(PERCENT_FAKE_SHARE_TOLERATED);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinPerFake")), percentfake);
	///sdl speed
	int sdlspeed = SETTING(SDL_SPEED);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spinspeedsd")),sdlspeed);
	///SDL Time
	int sdltime = SETTING(SDL_TIME);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("spintimesd")),sdltime);
	///Protect Users
	gchar * protectuser = SETTING(PROTECTED_USERS).c_str();
	gtk_entry_set_text(GTK_ENTRY(getWidget("entryProtectPatern")),protectuser);


}
/*1page*/
GtkTreeModel * DetectionTab::create_model(gpointer data) {

	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeStore *store;
	GtkTreeIter topi;
	store = gtk_tree_store_new(NUM_COLS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT,G_TYPE_STRING,G_TYPE_INT);

	Action::ActionList& act=RawManager::getInstance()->getActions();

	for (Action::ActionList::const_iterator i = act.begin(); i != act.end();++i)
	{
		const string& name = (*i)->getName();

		gtk_tree_store_append(store, &topi, NULL);
		gtk_tree_store_set(store, &topi,
						COLUMN, name.c_str(),
						RAW, "",
						TIME,0,
						ENABLED, (*i)->getEnabled() ? "1" : "0",
						ID, (*i)->getId(),
						-1);
		GtkTreeIter child;
		dt->act.insert(ActRaw::value_type( (*i)->getId(),topi));

		for(Action::RawsList::const_iterator p = (*i)->raw.begin();p!=((*i)->raw.end());++p)
		{
			gtk_tree_store_append(store,&child ,&topi);
			gtk_tree_store_set(store, &child,
							COLUMN, p->getName().c_str(),
							RAW, p->getRaw().c_str(),
							TIME, p->getTime(),
							ENABLED, p->getEnabled() ? "1" : "0",
							ID, p->getId(),
							-1);
			dt->raws.insert(ActRaw::value_type( (*i)->getId(),child));

		}

	}

	return GTK_TREE_MODEL(store);
}
/*2Page*/
GtkTreeModel * DetectionTab::create_profiles(gpointer data)
{
	DetectionTab *dt =(DetectionTab *)data;
	GtkTreeStore *store;
	GtkTreeIter iter;
	store = gtk_tree_store_new(NUM_COL,G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING);
	dcpp::StringMap params;

	const DetectionManager::DetectionItems& lst = DetectionManager::getInstance()->getProfiles(params,false);
	for ( DetectionManager::DetectionItems::const_iterator i = lst.begin(); i != lst.end() ;++i)
	{
		const DetectionEntry& de = *i ;
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter,
						COLUMNS,Util::toInt(de.isEnabled ? "1" : "0") /*? TRUE: FALSE*/ ,
						NAME,de.name.c_str(),
						CHEAT,de.cheat.c_str(),
						COMMENT,de.comment.c_str(),
						RAWS, de.rawToSend,
						IDS,de.Id,
						FLAG,de.clientFlag,
						MISMATCH, de.checkMismatch ? "1" : "0",
						-1);
		dt->profiles.insert(Prof::value_type(de.Id,iter));
	}
return GTK_TREE_MODEL(store);
}

void DetectionTab::onAddActRaw(GtkWidget *widget,gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	const string emptyStr = "";
	StringMap params;
	params["Name"] = emptyStr;
	params["RAW"] = emptyStr;
	params["Time"] = "0.0";
	params["ID"] = "-1.0";
	params["Enabled"] = "0";
	params["Action"] = "0";
	params["Type"] = "0";

	bool isOk = dt->showAddActRawDialog(params,dt);

	if(isOk)
	{

		dt->addActRaw_gui(params);
		typedef Func1<DetectionTab,StringMap> F1;
		F1 *func = new F1(dt,&DetectionTab::addRawAct_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}

}
void DetectionTab::onEditActRaw(GtkWidget *widget,gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeIter	iter,parent;
	GtkTreeModel *tmodel;
	char *name,*raw;
	gint tim;
	char *enb;
	gint id;
	if(gtk_tree_selection_get_selected( GTK_TREE_SELECTION(dt->aselection), &tmodel, &iter))
	{
		gtk_tree_model_get(tmodel,&iter,
								COLUMN,&name,
								RAW,&raw,
								TIME,&tim,
								ENABLED,&enb,
								ID,&id,
								-1);
		StringMap params;
		params["Name"] = name;
		params["RAW"] =raw;
		params["Time"] = Util::toString(tim);
		params["Enabled"] = enb;
		params["ID"] = Util::toString(id);
		Action* a = RawManager::getInstance()->findAction(string(name));
		if(a!= NULL)
		{
		params["Action"] = Util::toString(a->getId());
		params["Type"] = "0";

		}

		if( !(string(raw).empty()))
		{
			params["Type"] = "1";
			gboolean isT=gtk_tree_model_iter_parent(tmodel,&parent,&iter);
			if(isT)
			{
				gtk_tree_model_get(tmodel,&parent,
								COLUMN,&name,
								RAW,&raw,
								TIME,&tim,
								ENABLED,&enb,
								-1);
				params["Action"] = Util::toString(dt->find_raw(string(name)));

			}

		}

		bool isOk = dt->showAddActRawDialog(params,dt);
		if(isOk)
		{
			dt->addActRaw_gui(params);
			typedef Func1<DetectionTab,StringMap> F1;
			F1 *func = new F1(dt,&DetectionTab::editRawAct_client, params);
			WulforManager::get()->dispatchClientFunc(func);

		}

	}
}

void DetectionTab::onRemoveActRaw(GtkWidget *widget , gpointer data)
{
		DetectionTab *fh = (DetectionTab *)data;
		GtkTreeIter iter;
		GtkTreeModel *tmodel;
		char *name,*raw,*enb;
		int *id, *tim;
		/*Parent Item*/
		char *_name,*_raw,*_enb;
		int *_id,*_tim;

		if(gtk_tree_selection_get_selected( GTK_TREE_SELECTION(fh->aselection), &tmodel, &iter))
		{

			gtk_tree_model_get(tmodel,&iter,
								COLUMN,&name,
								RAW,&raw,
								TIME,&tim,
								ENABLED,&enb,
								ID,&id,
								-1);

				if(BOOLSETTING(CONFIRM_HUB_REMOVAL))
				{
					GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
					GtkWidget* dialog = gtk_message_dialog_new(parent,
					GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					_("Are you sure you want to delete this item \"%s\"?"), name);
					gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
					gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
					gint response = gtk_dialog_run(GTK_DIALOG(dialog));
					gtk_widget_destroy(dialog);

					if (response != GTK_RESPONSE_YES)
						return;

				}

				GtkTreeIter parent;
				bool isT = false;
				if(gtk_tree_model_iter_parent(tmodel,&parent,&iter))
				{
					gtk_tree_model_get(tmodel,&parent,
								COLUMN,&_name,
								RAW,&_raw,
								TIME,&_tim,
								ENABLED,&_enb,
								ID,&_id,
								-1);
					isT = true;
				}
				if( !isT)
				{
					typedef Func1<DetectionTab, int> F1;
					F1 *func = new F1(fh, &DetectionTab::removeActRaw_client, id);
					WulforManager::get()->dispatchClientFunc(func);
				}
				else
				{
					typedef Func2<DetectionTab, int,int> F2;
					F2 *func = new F2(fh, &DetectionTab::removeRaw_client, id,_id);
					WulforManager::get()->dispatchClientFunc(func);


				}
				fh->removeEntry_gui(name,id,"");
		}
}

bool DetectionTab::showAddActRawDialog(StringMap &params,DetectionTab *dt)
{
		/*text and spin */
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryName")),params["Name"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryRaw")),params["RAW"].c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonId")),(gdouble)Util::toDouble(params["ID"]));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonTime")),(gdouble)Util::toDouble(params["Time"]));
		// Set the Enabled checkbox
		gboolean enabled =(((params["Enabled"] == "1") ? TRUE : FALSE) || params["Enabled"].empty());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkEnabled")), enabled);
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxType")),Util::toInt(params["Type"]));

		vector< pair <string,int> >& act = WulforUtil::getActions();
		WulforUtil::drop_combo(dt->getWidget("comboboxentryAct"),act);
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxentryAct")), (gint)(dt->find_rawInt(Util::toInt(params["Action"])) ));

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
			params["Action"] = Util::toString(find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentryAct")))));
			params["Enabled"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkEnabled"))) ? "1" : "0";

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
}

void DetectionTab::addActRaw_gui(StringMap params)
{
		GtkTreeIter iter,child;
		int Id = Util::toInt(params["ID"]);
		int q;
		if(!(params["Type"] == "1"))
		{
			if(findAct_gui(Id,&iter))
			{
				gtk_tree_store_set(GTK_TREE_STORE(model),&iter,
												COLUMN,params["Name"].c_str(),
												RAW, "",
												TIME, 0,
												ENABLED, params["Enabled"].c_str(),
												ID, Util::toInt(params["ID"]),
												-1);
			}
			else
			{
				gtk_tree_store_append(GTK_TREE_STORE(model),&iter,NULL);
					gtk_tree_store_set(GTK_TREE_STORE(model),&iter,
												COLUMN,params["Name"].c_str(),
												RAW, "",
												TIME, 0,
												ENABLED, params["Enabled"].c_str(),
												ID,Util::toInt(params["ID"]),
												-1);
			}

		}
		else
		{
			q = Util::toInt(params["Action"]);
			if(findAct_gui(q,&iter))
			{
				if(findRaw_gui(Id,&child))
				{
					gtk_tree_store_set(GTK_TREE_STORE(model),&child,
												COLUMN,params["Name"].c_str(),
												RAW, params["RAW"].c_str(),
												TIME, Util::toInt(params["Time"]),
												ENABLED, params["Enabled"].c_str(),
												ID, Util::toInt(params["ID"]),
												-1);

				}
				else
				{

					gtk_tree_store_append(GTK_TREE_STORE(model),&child,&iter);
					gtk_tree_store_set(GTK_TREE_STORE(model),&child,
												COLUMN,params["Name"].c_str(),
												RAW, params["RAW"].c_str(),
												TIME, Util::toInt(params["Time"]),
												ENABLED, params["Enabled"].c_str(),
												ID,Util::toInt(params["ID"]),
												-1);
				}
			}
    	}
}

void DetectionTab::removeEntry_gui(string _name,int _id,string _action)
{
	gboolean valid,gvalid;
	GtkTreeIter iter,giter;

	const char *act = _action.c_str();

	valid = gtk_tree_model_get_iter_first(model,&iter);

	while(valid)
	{
		char *name,*raw,*tim,*enb;
		int id;

		gtk_tree_model_get(model,&iter,
							COLUMN,&name,
							RAW, &raw,
							TIME,&tim,
							ENABLED,&enb,
							ID,&id,
							-1);


			if(name == _name && id == _id)
			{
				gtk_tree_store_remove(GTK_TREE_STORE(model),&iter);
				break;
			}

			if(act != ((char *)"") || raw != ((char *)""))
			{

				gvalid =  gtk_tree_model_iter_nth_child(model,&giter,&iter,1);
				while(gvalid)
				{
					gtk_tree_model_get(model,&giter,
							COLUMN,&name,
							RAW, &raw,
							TIME,&tim,
							ENABLED,&enb,
							ID,&id,
							-1);

							if(name == _name && id == _id)
							{
								gtk_tree_store_remove(GTK_TREE_STORE(model),&giter);
								break;
							}
					gvalid = gtk_tree_model_iter_next (model, &giter);
				}
			}
			valid = gtk_tree_model_iter_next (model, &iter);
	}
}

void DetectionTab::addRawAct_client(StringMap params)
{
	bool raw= false;
	if(params["Type"] == "1")
		raw = true;

	RawManager::getInstance()->lock();
	if(!raw)
	{
		Action* a=RawManager::getInstance()->addAction(Util::toInt(params["ID"]),params["Name"],true);

	}
	else
	{
		Action* a = RawManager::getInstance()->findAction(Util::toInt(params["Action"]));
		Raw *raw=new Raw();
		raw->setId(Util::toInt(params["ID"]));
		raw->setName(params["Name"]);
		raw->setRaw(params["RAW"]);
		raw->setTime(Util::toInt(params["Time"]));
		raw->setEnabled(true);

		Raw* r = RawManager::getInstance()->addRaw(a,*raw);

	}
	RawManager::getInstance()->unlock();
}
void DetectionTab::editRawAct_client(StringMap params)
{
	bool raw = false;
	if(params["Type"] == "1")
		raw = true;

	RawManager::getInstance()->lock();
	if(!raw)
	{
		Action* action = RawManager::getInstance()->findAction(Util::toInt(params["ID"]));
		if(action == NULL)
		{
			RawManager::getInstance()->unlock();
			return;
		}

		action->setEnabled(Util::toInt(params["Enabled"]));
		action->setName(params["Name"]);


	}
	else
	{
		const Action* action = RawManager::getInstance()->findAction(Util::toInt(params["Action"]));
		if(action != NULL)
		{
		Raw raw;
		raw.setId(Util::toInt(params["ID"]));
		raw.setName(params["Name"]);
		raw.setRaw(params["RAW"]);
		raw.setTime(Util::toInt(params["Time"]));
		raw.setEnabled(Util::toInt(params["Enabled"]));
		Raw* old;
			for(Action::RawsList::const_iterator p = (action)->raw.begin();p!=((action)->raw.end());++p)
			{
				if((p)->getId() == Util::toInt(params["ID"]))
				{
					old = &(*p);
					RawManager::getInstance()->editRaw(&(*action),old,raw);
					break;
				}

			}
		}

	}
	RawManager::getInstance()->unlock();

}

void DetectionTab::removeActRaw_client(int id)
{
	RawManager::getInstance()->lock();
	Action* action = RawManager::getInstance()->findAction(id);
	bool rem = RawManager::getInstance()->remAction(action);
	RawManager::getInstance()->unlock();
}

void DetectionTab::removeRaw_client(int id,int _parent)
{
		RawManager::getInstance()->lock();
			Action* action = RawManager::getInstance()->findAction(RawManager::getInstance()->getValidAction(_parent));
			if(action != NULL)
			{
				Raw* old;
				for(Action::RawsList::const_iterator p = (action)->raw.begin();p!=((action)->raw.end());++p)
				{
					if((p)->getId() == id)
					{
						old = &(*p);
						RawManager::getInstance()->remRaw(action,old);
						break;
					}

				}
			}
	RawManager::getInstance()->unlock();
}

/*2page*/
void DetectionTab::onAddEntryDet(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	StringMap params;
	const string emptyStr = "";
	params["Name"]="< Name >";
	params["ID"] = "0";
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
void DetectionTab::ondModEntryDet(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeModel *tmodel;
	GtkTreeIter iter;
	bool enb;
	char *name,*cheat,*com,*mis;
	gint id,raw;
	gint flag;

	if(gtk_tree_selection_get_selected( dt->prselection, &tmodel, &iter))
	{
		gtk_tree_model_get(tmodel,&iter,
                                /*COLUMNS,&enb,*/
                                NAME,&name,
                                CHEAT,&cheat,
                                COMMENT,&com,
                                RAWS,&raw,
                                IDS,&id,
                                FLAG,&flag,
                                MISMATCH,&mis,
								-1);
		StringMap params;
		params["Name"] = name;
		params["Cheat"] = cheat;
		params["Comment"] = com;
		params["ID"] = Util::toString(id);
		params["RAW"] = Util::toString(raw);
		params["Enabled"] = enb ? "1" : "0";
		params["Flag"] = Util::toString(flag);
		params["MisMatch"] = mis ? "1" : "0";
		bool isOk = dt->showAddEntryDetDialog(params,dt);
		if(isOk)
		{
				typedef Func2<DetectionTab,int,StringMap> F2;
				F2 *func = new F2(dt,&DetectionTab::editEntryDet_client,id,params);
				WulforManager::get()->dispatchClientFunc(func);

				dt->addEntryDet_gui(params);
		}
	}
}

void DetectionTab::onRemoveEntryDet(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeModel *tmodel;
	GtkTreeIter iter;
	bool enb;
	char *name,*cheat,*com;
	gint *raw,*id;

	if(gtk_tree_selection_get_selected( GTK_TREE_SELECTION(dt->prselection), &tmodel, &iter))
	{
		gtk_tree_model_get(tmodel,&iter,
								COLUMN,&enb,
								NAME,&name,
								CHEAT,&cheat,
								COMMENT,&com,
								RAW,&raw,
								IDS,&id,
								-1);

				if(BOOLSETTING(CONFIRM_HUB_REMOVAL))
				{
					GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
					GtkWidget* dialog = gtk_message_dialog_new(parent,
					GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					_("Are you sure you want to delete favorite hub \"%s\"?"), name);
					gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
					gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
					gint response = gtk_dialog_run(GTK_DIALOG(dialog));
					gtk_widget_destroy(dialog);

					if (response != GTK_RESPONSE_YES)
						return;

				}

				dt->removeEntryDet_gui(name,(int)id);
				typedef Func1<DetectionTab,int> F1;
				F1 *func = new F1(dt,&DetectionTab::removeEntryDet_client,(int)id);
				WulforManager::get()->dispatchClientFunc(func);
	}
}

void DetectionTab::removeEntryDet_gui(string _name,int _id)
{
	gboolean valid,gvalid;
	GtkTreeIter iter,giter;

	valid = gtk_tree_model_get_iter_first(models,&iter);

	while(valid)
	{
		char *name,*cheat,*com;
		int *id,*raw;
		bool enb;

		gtk_tree_model_get(models,&iter,
								COLUMN,&enb,
								NAME,&name,
								CHEAT,&cheat,
								COMMENT,&com,
								RAWS,&raw,
								IDS,&id,
							-1);


			if(name == _name && id == _id)
			{
				gtk_tree_store_remove(GTK_TREE_STORE(models),&iter);
				break;
			}


			valid = gtk_tree_model_iter_next (models, &iter);
	}
}

void DetectionTab::addEntryDet_gui(dcpp::StringMap params)
{
		//string name = params["ID"];
		uint32_t id = Util::toUInt32(params["ID"]);
		GtkTreeIter iter;
		if(findProf_gui(id,&iter))
		{
			gtk_tree_store_set(GTK_TREE_STORE(models),&iter,
												COLUMNS,Util::toInt(params["Enabled"]),
												NAME,params["Name"].c_str(),
												CHEAT,params["Cheat"].c_str(),
												COMMENT,params["Comment"].c_str(),
												RAWS,Util::toInt(params["RAW"]),
												IDS,id,
												FLAG,Util::toInt(params["Flag"]),
												MISMATCH,params["MisMatch"].c_str(),
												-1);


		}
		else
		{
				gtk_tree_store_append(GTK_TREE_STORE(models),&iter,NULL);
				gtk_tree_store_set(GTK_TREE_STORE(models),&iter,
												COLUMNS,Util::toInt(params["Enabled"]),
												NAME,params["Name"].c_str(),
												CHEAT,params["Cheat"].c_str(),
												COMMENT,params["Comment"].c_str(),
												RAWS,Util::toInt(params["RAW"]),
												IDS,id,
												FLAG,Util::toInt(params["Flag"]),
												MISMATCH,params["MisMatch"].c_str(),
												-1);
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
	if(DetectionManager::getInstance()->getDetectionItem(id,entry,false))
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

	DetectionManager::getInstance()->updateDetectionItem(id,entry,false);
}

void DetectionTab::removeEntryDet_client(int id)
{DetectionManager::getInstance()->removeDetectionItem(id,false);}


gboolean DetectionTab::onButtonItemPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	dt->previous = event->type;
	return FALSE;
}

gboolean DetectionTab::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeIter iter;
	GtkTreeModel *tmodel;
	if (!gtk_tree_selection_get_selected(dt->itemselection, &tmodel, &iter))
	{
		if (dt->previous == GDK_BUTTON_PRESS && event->button == 3)
		{
			dt->popupMenu_gui();
		}

	}
	else
	{
		if (dt->previous == GDK_BUTTON_PRESS && event->button == 3)
		{
			dt->popupMenu_gui();
		}
	}
}

void DetectionTab::popupMenu_gui()
{
	gtk_menu_popup(GTK_MENU(getWidget("menuItem")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

void DetectionTab::onAddItemDlg_gui(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	StringMap params;
	params["Name"] ="";
	params["Value"]="";
	params["Type"]="0";
	bool isOk = dt->runDialogItem(params,dt);
	if(isOk)
	{
		dt->addMap_gui(params);
	}
}
void DetectionTab::onModItemDlg_gui(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->itemselection, NULL, &iter))
		return;
	StringMap params;
	params["Name"] = dt->item.getString(&iter, "Field");
	params["Value"] = dt->item.getString(&iter, "Value");
	string iftype = dt->item.getString(&iter, "Hub");
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

void DetectionTab::onRemItemDlg_gui(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->itemselection, NULL, &iter))
		return;

		    	if (BOOLSETTING(CONFIRM_USER_REMOVAL))
				{
					GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_NONE,
						_("Are you sure you want to delete item?"));
					gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE,
					GTK_RESPONSE_YES, NULL);
					gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
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

void DetectionTab::onToggleDet(GtkCellRendererToggle *cell, gchar *pathStr, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeIter iter;
	bool enabled;
	int id;
	GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);
   gtk_tree_model_get_iter(GTK_TREE_MODEL (data), &iter, path);
   //gtk_tree_model_get(GTK_TREE_MODEL (data), &iter, COLUMNS, &enabled,-1);
   gtk_tree_model_get(GTK_TREE_MODEL (data), &iter, COLUMNS, &enabled,IDS,&id,-1);

   DetectionManager::getInstance()->setItemEnabled(id,!enabled,false);

   enabled = !enabled;
   gtk_tree_store_set(GTK_TREE_STORE (data), &iter, COLUMNS, enabled, -1);

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
	item.col("Field"),params["Name"].c_str(),
	item.col("Value"),params["Value"].c_str(),
	item.col("Hub"), params["Type"].c_str(),
	-1);
}

bool DetectionTab::showAddEntryDetDialog(StringMap &params, DetectionTab *dt)
{
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entry1Name")), params["Name"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entry2Cheat")), params["Cheat"].c_str());
		gtk_entry_set_text(GTK_ENTRY(dt->getWidget("entryComments")), params["Comment"].c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttondetId")),(gdouble)Util::toDouble(params["ID"]));
		// Set the Enabled checkbox
		gboolean enabled =(((params["Enabled"] == "1") ? TRUE : FALSE) || params["Enabled"].empty());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton1Enabled")), enabled);
		// Set the Enabled checkbox
		gboolean mismatch =(((params["MisMatch"] == "1") ? TRUE : FALSE) || params["MisMatch"].empty());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbuttonCheckMis")), mismatch);
		///Set Action
		vector< pair <string,int> >& act = WulforUtil::getActions();
		WulforUtil::drop_combo(dt->getWidget("comboboxentry1Act"),act);
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxentry1Act")), (gint)(dt->find_rawInt(Util::toInt(params["RAW"])) ));
		//Flag
		gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxFlag")),Util::toInt(params["Flag"]));
		/*InfMap */
		DetectionEntry e;
		bool u=DetectionManager::getInstance()->getDetectionItem(Util::toUInt(params["ID"]), e,false);
		if(u)
		{
			/*dt->*/clear_all_col(item);
			for(dcpp::DetectionEntry::INFMap::const_iterator i=e.defaultMap.begin();i!=e.defaultMap.end();++i)
			{
				StringMap p;
				p["Name"] = i->first;
				p["Value"]= i->second;
				p["Type"] = "Both";
				dt->addMap_gui(p);
				map.push_back(make_pair(i->first,i->second));
			}

			for(dcpp::DetectionEntry::INFMap::const_iterator i=e.adcMap.begin();i!=e.adcMap.end();++i)
			{
				StringMap q;
				q["Name"] = i->first;
				q["Value"]= i->second;
				q["Type"] = "ADC";
				dt->addMap_gui(q);
				mapadc.push_back(make_pair(i->first,i->second));
			}

			for(dcpp::DetectionEntry::INFMap::const_iterator i=e.nmdcMap.begin();i!=e.nmdcMap.end();++i)
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
			q["Value"] = "XX";
			q["Type"] = "BOTH";
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
			params["RAW"] = Util::toString(find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1Act")))));
			GtkTreeIter iter;
			GtkTreeModel *tmodel= GTK_TREE_MODEL(dt->itemstore);
			gboolean valid = gtk_tree_model_get_iter_first(tmodel, &iter);
			dt->map.clear();
			dt->mapadc.clear();
			dt->mapnmdc.clear();

				while(valid)
				{
					string a = dt->item.getString(&iter, "Field");
					string b = dt->item.getString(&iter, "Value");
					string hub = dt->item.getString(&iter, "Hub");
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
					else ;

					valid = gtk_tree_model_iter_next(model, &iter);

				}
				if(!params["Name"].empty())

				gtk_widget_hide(dt->getWidget("dialogDetection"));
				return TRUE;
		}
}

/* Find func*/
bool DetectionTab::findAct_gui(const int &Id, GtkTreeIter *iter)
{
	ActRaw::const_iterator it = act.find(Id);

	if (it != act.end())
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
	int raw =0;
	vector<std::pair<std::string,int> >& act = WulforUtil::getActions();
	for (vector<std::pair<std::string,int> >::const_iterator it = act.begin(); it != act.end(); ++it)
	{
		if(it->first == rawString)
			raw = it->second;
	}
  return raw;
}
int DetectionTab::find_rawInt(int raw)
{
	int _raw =0;
	int i=0;
	vector<std::pair<std::string,int> >& act = WulforUtil::getActions();
	for (vector<std::pair<std::string,int> >::const_iterator it = act.begin(); it != act.end(); ++it)
	{
		i++;
		if(it->second == raw)
		{_raw =(i+(-1));}
	}
  return _raw;
}

void DetectionTab::clear_all_col(TreeView tree)
{
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree.get())));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree.get()));
	if(gtk_tree_model_get_iter_first(model, &iter) == FALSE)
		return;
	gtk_list_store_clear(store);
}
/**/

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
	points.col("Points"),params["Points"].c_str(),
	points.col("Action"),params["Action"].c_str(),
	-1);
}
void DetectionTab::onADSLPoints(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	const IntMap& list =RawManager::getInstance()->getADLPoints();
	for(IntMap::const_iterator i= list.begin();i!=list.end();++i)
	{
		StringMap params;
		params["Points"] = Util::toString(i->first);
		params["Action"] =RawManager::getInstance()->getNameActionId(i->second);
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
			string a = dt->points.getString(&iter, "Points");
			string b = dt->points.getString(&iter, "Action");

			dt->imap.insert(make_pair(Util::toInt(a),dt->find_raw(b)));

			valid = gtk_tree_model_iter_next(tmodel, &iter);
		}

		RawManager::getInstance()->updateADLPoints(dt->imap);

		gtk_widget_hide(dt->getWidget("dialogitemPoints"));
	}
}
bool DetectionTab::showAddPointsDialog(StringMap &params,DetectionTab *dt)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonpointss1")),(gdouble)(Util::toInt(params["Points"])));
	gtk_combo_box_set_active(GTK_COMBO_BOX(dt->getWidget("comboboxentryactionp1")), (gint)(dt->find_raw(params["Action"])));

	gint response = gtk_dialog_run(GTK_DIALOG(dt->getWidget("dialogpointitem1")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
			return FALSE;
	if(response == GTK_RESPONSE_OK)
	{
		params["Action"]=gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentryactionp1")));
		params["Points"] = Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(dt->getWidget("spinbuttonpointss1"))));
		gtk_widget_hide(dt->getWidget("dialogpointitem1"));
		return TRUE;
	}
}

void DetectionTab::onADSLPointsADD(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	StringMap params;
	params["Points"] = "0";
	params["Action"]= "";
	bool isOk = dt->showAddPointsDialog(params,dt);
	if(isOk)
	{

	dt->addPoints_gui(params);
	}
}

void DetectionTab::onADSLPointsMOD(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt =(DetectionTab *)data;
	StringMap params;
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->pointselect, NULL, &iter))
		return;

	params["Points"] = dt->points.getString(&iter,"Points");
	params["Action"] == dt->points.getString(&iter,"Action");
	bool isOk = dt->showAddPointsDialog(params,dt);
	if(isOk)
	{

	dt->editPoints_gui(params, &iter);
	}
}
void DetectionTab::onADSLPointsDEL(GtkWidget *widget, gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(dt->pointselect, NULL, &iter))
		return;

		 if (BOOLSETTING(CONFIRM_USER_REMOVAL))
		{
					GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_NONE,
						_("Are you sure you want to delete item?"));
					gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE,
					GTK_RESPONSE_YES, NULL);
					gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
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
void DetectionTab::onSave(GtkWidget *widget , gpointer data)
{
	DetectionTab *dt = (DetectionTab *)data;
	SettingsManager *st = SettingsManager::getInstance();
	///Fake
	bool afake = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkFake1")));
	st->set(SettingsManager::SHOW_FAKESHARE_RAW,afake);
	int fake = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1Fake"))));
	st->set(SettingsManager::FAKESHARE_RAW,fake);
	///RMDC
	bool armdc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkRmdc1")));
	st->set(SettingsManager::SHOW_RMDC_RAW,armdc);
	int rmdc = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1rmdc"))));
	st->set(SettingsManager::RMDC_RAW,rmdc);
	///EMulDC
	bool aemul = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkemul1")));
	st->set(SettingsManager::SHOW_DCPP_EMULATION_RAW,aemul);
	int emul = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1emul"))));
	st->set(SettingsManager::DCPP_EMULATION_RAW,emul);
	///FLMM
	bool aflmismatch = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkmismatch4")));
	st->set(SettingsManager::SHOW_FILELIST_VERSION_MISMATCH,aflmismatch);
	int flmismatch = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1mis"))));
	st->set(SettingsManager::FILELIST_VERSION_MISMATCH,flmismatch);
	///LLMM
	int listlen = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1listlen"))));
	st->set(SettingsManager::LISTLEN_MISMATCH,listlen);
	///VMM
	int vermis =  dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1vermis"))));
	st->set(SettingsManager::VERSION_MISMATCH,vermis);
	///DISON
	bool adis =gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkdiscon7")));
	st->set(SettingsManager::SHOW_DISCONNECT_RAW,adis);
	int dis = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1disc"))));
	st->set(SettingsManager::DISCONNECT_RAW,dis);
	///SMBIG
	bool asmbig = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton1")));
	st->set(SettingsManager::FILELIST_TOO_SMALL_BIG,asmbig);
	int smbig = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1BigSmall"))));
	st->set(SettingsManager::FILELIST_TOO_SMALL_BIG_RAW,smbig);
	///SDL
	bool asdl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkbutton2slw")));
	st->set(SettingsManager::USE_SDL_KICK,asdl);
	int sdl = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1slwsp"))));
	st->set(SettingsManager::SDL_RAW,sdl);
	///Show Cheat
	bool showcheat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkShowCheat")));
	st->set(SettingsManager::DISPLAY_CHEATS_IN_MAIN_CHAT,showcheat);
	///ADLD
	bool showadla = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dt->getWidget("checkADLACtionShow")));
	st->set(SettingsManager::SHOW_ADLSEARCH_DEFAULT_ACTION,showadla);
	int adlr = dt->find_raw(gtk_combo_box_get_active_text(GTK_COMBO_BOX(dt->getWidget("comboboxentry1ADLA"))));
	st->set(SettingsManager::ADLSEARCH_DEFAULT_ACTION, adlr);
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
    gchar * protectuser = gtk_entry_get_text(GTK_ENTRY(dt->getWidget("entryProtectPatern")));
    st->set(SettingsManager::PROTECTED_USERS, protectuser);

	///Save
	st->save();
}
/*
bool DetectionTab::showErrorDialog_gui(const string &description, FavoriteHubs *fh)
{
	GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(fh->getWidget("favoriteHubsDialog")),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;

	gtk_widget_destroy(dialog);

	return TRUE;
}
*/
/*this is a generic pop menu*/
void DetectionTab::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()),closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate",G_CALLBACK(onCloseItem),this);

}

void DetectionTab::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;

    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);

}
