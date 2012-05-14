#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/SettingsManager.h>

#include "bookentry.hh"
#include "treeview.hh"

#ifndef _ABOUT_CONFIG
#define _ABOUT_CONFIG
class AboutConfig: 
	public BookEntry,
	private dcpp::SettingsManagerListener	
{
	public:
		AboutConfig();
		virtual ~AboutConfig();
		virtual void show();
	private:
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onPropertiesClicked_gui(GtkWidget *widget, gpointer data);
	
		bool getDialog(std::string i , std::string& value , gpointer data);
		void addItem_gui(std::string rowname, std::string isdefault, std::string types, std::string value);		
	
		TreeView aboutView;
		GtkListStore *aboutStore;
		GtkTreeSelection *aboutSelection;
		GdkEventType previous;
};
#else
class AboutConfig;
#endif