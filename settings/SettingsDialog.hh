/*
 * 
 * 
 * 
 * 
 * 
 * 
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
		GtkWidget *getContainer() { return dialogWin; }
		void run();
		std::string& getID() { return id;}
	private:
		SettingsPage* pages[17];
		GtkWidget* dialogWin,
		* mainBox,	* statusBox,
		* okButton,*stButton,
		* paned,*tree,
		* containBox;
		GtkListStore *store;
		GtkTreeModel *model;
		GtkTreeSelection *tree_sel;
		gint previous_page;
		gint actual_page;
		std::string id;
		int m_num;
		void close() { 
			/*for(int i = 0; i < m_num; ++i) {
				if(pages[i] != NULL && pages[i]->showed >= 1) {
					pages[i]->write();
				}
			}*/
			if(pages[0] != NULL)
				pages[0]->write();
			if(pages[previous_page] != NULL)
				pages[previous_page]->write();
			gtk_widget_destroy(getContainer());
		}

static void tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
		SettingsDialog *sd = (SettingsDialog*)data;
        GtkTreeIter iter;
        GtkTreeModel *model = NULL;
        gint num=0;

        if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
                gtk_tree_model_get (model, &iter, 0, &num, -1);

				if(sd->pages[sd->previous_page] != NULL)
					sd->pages[sd->previous_page]->write();
                if(num != sd->previous_page)
                {
						dcdebug("\npage %s\n",sd->pages[num]->get_name_page());
//						if(sd->pages[num] != NULL){
							sd->pages[num]->show(sd->containBox,sd->pages[sd->previous_page]->getTopWidget());
							gtk_widget_show_all(sd->pages[num]->getTopWidget());
//						}	

				}
				sd->previous_page = num;
        }
}
		static void onOkButton(GtkWidget *widget, gpointer data);
		static void onStButton(GtkWidget *widget, gpointer data);
};
#endif
