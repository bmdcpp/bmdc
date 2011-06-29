#ifndef WULFOR_DETECTION_HH
#define WULFOR_DETECTION_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>

#include <dcpp/RawManager.h>
#include <dcpp/DetectionManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class DetectionTab:
	public BookEntry
{
	public:
		DetectionTab();
		virtual ~DetectionTab();
		virtual void show();
		virtual void popmenu();
	private:
        ///GUI
        static void onCloseItem(gpointer data);

        static void onSwitchTab(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data);
        static bool showErrorDialog_gui(const std::string &description, DetectionTab *dt);
		///1page
		typedef std::tr1::unordered_map<int, GtkTreeIter> ActRaw;

		static void onAddAct(GtkWidget *widget, gpointer data);
		static void onAddRaw(GtkWidget *widget, gpointer data) ;
		static void onEditAct(GtkWidget *widget,gpointer data);
		static void onEditRaw(GtkWidget *widget,gpointer data);
		static void onRemoveAct(GtkWidget *widget , gpointer data);
		static void onRemoveRaw(GtkWidget *widget , gpointer data);

		static gboolean onActButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onActButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onActKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);

		void removeRaw_gui(std::string Id, std::string name);
		void create_actions_raws();
		bool showAddActRawDialog(dcpp::StringMap &params,DetectionTab *dt);
		void addAct_client(dcpp::StringMap params);
		void addRaw_client(dcpp::StringMap params);
		void editAct_client(dcpp::StringMap params);
		void editRaw_client(dcpp::StringMap params);
		void removeAct_client(int id);
		void removeRaw_client(int id);
		void addAct_gui(dcpp::StringMap params);
		void addRaw_gui(dcpp::StringMap params);
		bool findAct_gui(const int &Id, GtkTreeIter *iter);
		bool findRaw_gui(const int &Id, GtkTreeIter *iter);
		void updateRawView_gui();

		TreeView actionView;
		TreeView RawView;
		GtkListStore *actionStore,*RawStore;
		GtkTreeSelection *RawSelection,*actionSelection;
        GdkEventType actPrevious;

        std::vector<std::pair<std::string,int> > actionsn;
        
		ActRaw actions;
		ActRaw raws;
		///2page
		typedef std::tr1::unordered_map<uint32_t, GtkTreeIter> Prof;
		Prof profiles;
		void create_profiles();
		static void onAddEntryDet(GtkWidget *widget, gpointer data);
		static void ondModEntryDet(GtkWidget *widget, gpointer data);
		static void onRemoveEntryDet(GtkWidget *widget, gpointer data);
		static void onToggleDet(GtkCellRendererToggle *cell, gchar *pathStr, gpointer data);
		void addMap_gui(dcpp::StringMap params);
		void editMap_gui(dcpp::StringMap &params,GtkTreeIter *iter);
		bool showAddEntryDetDialog(dcpp::StringMap &params,DetectionTab *dt);
		void removeEntryDet_gui(uint32_t _id);
		void addEntryDet_gui(dcpp::StringMap params);
		void removeAction_gui(string Id, string name);
		void addEntryDet_client(dcpp::StringMap params);
		void editEntryDet_client(int id,dcpp::StringMap params);
		void removeEntryDet_client(int id);

		/**/
		GtkListStore *detectionStore;
		TreeView detectionView;
		GtkTreeSelection *detectionSelection;
		dcpp::DetectionEntry::INFMap map,mapadc,mapnmdc;

		void clear_all_col(TreeView tree);
		int find_rawInt(int raw);
		int find_raw(std::string rawString);
		bool findProf_gui(const uint32_t &id, GtkTreeIter *iter);
		/**/
		TreeView item;
		GtkListStore *itemstore;
		GdkEventType previous;
		GtkTreeSelection *itemselection;
		static gboolean onButtonItemPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		void popupMenu_gui();
		static void onAddItemDlg_gui(GtkWidget *widget, gpointer data);
		static void onModItemDlg_gui(GtkWidget *widget, gpointer data);
		static void onRemItemDlg_gui(GtkWidget *widget, gpointer data);
		bool runDialogItem(dcpp::StringMap &params,DetectionTab *dt);
		/*3d tab*/
		static void onSave(GtkWidget *widget , gpointer data);
		TreeView points;
		GtkListStore *pointstore;
		GtkTreeSelection *pointselect;
		static void onADSLPoints(GtkWidget *widget, gpointer data);
		static void onADSLPointsADD(GtkWidget *widget, gpointer data);
		static void onADSLPointsMOD(GtkWidget *widget, gpointer data);
		static void onADSLPointsDEL(GtkWidget *widget, gpointer data);
		void addPoints_gui(dcpp::StringMap params);
		void editPoints_gui(dcpp::StringMap& params, GtkTreeIter *iter);
		bool showAddPointsDialog(dcpp::StringMap &params, DetectionTab *dt);
		dcpp::IntMap imap;

		void set_combo(GtkWidget *place, vector<pair<string,int> > act, int set, bool det,gpointer data);
		void loadAgain(GtkWidget *widget, vector<pair<string,int> >act, int set, gpointer data);
		int save_combo(GtkWidget *widget);
		enum
		{
		    TYPE_STR,
		    TYPE_INT
		};
		std::map<int,std::string> tmpname;

};
#else
class DetectionTab;
#endif
