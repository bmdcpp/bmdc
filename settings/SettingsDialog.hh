/*
 * 
*/

#ifndef _SETTINGS_DIALOG_
#define _SETTINGS_DIALOG_
#include <gtk/gtk.h>
#include "SettingsPage.hh"

class SettingsDialog
{
	public:
		SettingsDialog();
		~SettingsDialog();
		GtkWidget *getContainer() { return dia; }
		void run();
		std::string& getID() { return id;}
	private:
		SettingsPage* pages[17];
		GtkWidget* dialogWin,*dia ,
		* mainBox,	* statusBox,
		* okButton,*stButton,
		* paned,*tree,*stack,
		* containBox;
		GtkListStore *store;
		GtkTreeModel *model;
		GtkTreeSelection *tree_sel;
		gint previous_page;
		gint actual_page;
		std::string id;
		int m_num;
	public:	
		void close() { 
			for(int i = 0; i <= m_num; ++i) {
				if(pages[i] != NULL) {
					pages[i]->write();
				}
			}
		}
	private:	
		static void onOkButton(GtkWidget *widget, gpointer data);
		static void onStButton(GtkWidget *widget, gpointer data);
};
#endif
