#ifndef WULFOR_HIGHLITING_HH
#define WULFOR_HIGHLITING_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/HighlightManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class Highlighting:
	public BookEntry
{
	public:
		Highlighting();
		virtual ~Highlighting();
		virtual void show();
		virtual void popmenu();
	private:
        //GUI
      static void onCloseItem(gpointer data);

		void addEntry_gui(dcpp::StringMap params);
		void editEntry_gui(dcpp::StringMap &params, GtkTreeIter *iter);
		void removeEntry_gui(std::string name);
		void get_ColorParams(dcpp::ColorSettings *cs, dcpp::StringMap &params);
		static void onADD(GtkWidget *widget, gpointer data);
		static void onModify(GtkWidget *widget, gpointer data);
		static void onRemoveEntry_gui(GtkWidget *widget, gpointer data);
		//Dialog
		static void onColorFore_clicked(GtkWidget *widget,gpointer data);
		static void onColorBg_clicked(GtkWidget *widget,gpointer data);
		static void onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data);

		struct Color {
			std::string bgcolor;
			std::string fgcolor;
		} colors;

		std::map<std::string,Color*> ColorMap;

		bool showColorDialog(dcpp::StringMap &params);
		void addHigl_client(dcpp::StringMap params);
		void editHigl_client(dcpp::StringMap params,std::string name);
		void removeEntry_client(std::string name);
		TreeView hView;
		GtkListStore *store;
		GtkTreeSelection *selection;
		dcpp::ColorList pList;
};
#else
class Highlighting;
#endif
