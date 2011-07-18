#ifndef FAVORITE_HUB_HH
#define FAVORITE_HUB_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FavoriteManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class FavoriteHubs:
		public BookEntry,
		public dcpp::FavoriteManagerListener
{
		public:
				FavoriteHubs();
				virtual ~FavoriteHubs();
				virtual void show();
				virtual void popmenu();
		private:
		typedef std::unordered_map<std::string, GtkTreeIter> FavHubGroupsIter;

		///GUI
		static void onCloseItem(gpointer data);

		void addEntry_gui(dcpp::StringMap params);
		void editEntry_gui(dcpp::StringMap &params, GtkTreeIter *iter);
		void removeEntry_gui(std::string address,std::string group);

		static void onAddEntry_gui(GtkWidget *widget, gpointer data);
		static void onEditEntry_gui(GtkWidget *widget, gpointer data);
		static void onRemoveEntry_gui(GtkWidget *widget, gpointer data);
		static void onConnect_gui(GtkButton *widget, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);

		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onToggledMode_a_gui(GtkToggleButton *widget, gpointer data);
		static void onToggledMode_p_gui(GtkToggleButton *widget, gpointer data);
		static void onToggledMode_d_gui(GtkToggleButton *widget, gpointer data);

		static void onAddGroupClicked_gui(GtkWidget *widget, gpointer data);
		static void onRemoveGroupClicked_gui(GtkWidget *widget, gpointer data);
		static void onManageGroupsClicked_gui(GtkWidget *widget, gpointer data);
		static void onUpdateGroupClicked_gui(GtkWidget *widget, gpointer data);
		static gboolean onGroupsKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onGroupsButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

		static void onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data);

		static bool showFavoriteHubDialog_gui(dcpp::StringMap &params, FavoriteHubs *fh);
		static bool showErrorDialog_gui(const std::string &description, FavoriteHubs *fh);
		void updateFavHubGroups_gui(bool updated);
		void saveFavHubGroups();

		void setRawActions_gui(FavoriteHubs *fh,dcpp::StringMap params);
		void setRawActions_client(FavoriteHubs *fh, dcpp::StringMap params);

		void popupMenu_gui();

		void initialze_client();
		void initActions();
		void initFavHubGroupsDialog_gui();

		bool findHub_gui(const std::string &server, GtkTreeIter *par, GtkTreeIter *child);

		void addEntry_client(dcpp::StringMap params);
		void editEntry_client(std::string address,dcpp::StringMap params);
		void removeEntry_client(std::string address);
		void getParamsFav(const dcpp::FavoriteHubEntry *entry,dcpp::StringMap &params);

		virtual void on(dcpp::FavoriteManagerListener::FavoriteAdded, const dcpp::FavoriteHubEntryPtr entry) throw();
		virtual void on(dcpp::FavoriteManagerListener::FavoriteRemoved, const dcpp::FavoriteHubEntryPtr entry) throw();

		TreeView favoriteView,actionView,groupView;
		GtkTreeStore *favoriteStore,*actionStore;
		GtkListStore *groupStore;
		GtkTreeSelection *favoriteSel,*actionSel,*groupselection;
		FavHubGroupsIter GroupsIter;
		GdkEventType previous;

		struct Mode {
			gboolean isDef;
			gboolean active;
			gboolean pasive;
			std::string ip;
		} mode;

		struct Iters {
				GtkTreeIter main;
				GtkTreeIter child;
		};

		typedef std::unordered_map<std::string, Iters*> FavIter;
		FavIter faviters;

		struct Actions {
			bool enabled;
			GtkTreeIter citer;
			GtkTreeIter iter;
		};

		typedef std::unordered_map<std::string, Actions*> ActionsIter;
		ActionsIter praws,actions;
		///Utils func@todo move to WulforUtil
		void drop_combo(GtkWidget *DROP, std::vector<std::string> CONTEUDO);

};

#else
class FavoriteHubs;
#endif
