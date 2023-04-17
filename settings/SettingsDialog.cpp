
#include <stdio.h>
#include <gtk/gtk.h>
#include "GeneralPage.hh"
#include "ConnectionPage.hh"
#include "OutConnectionPage.hh"
#include "AdvancedConnectionPage.hh"
#include "DownloadsPage.hh"
#include "QuenePage.hh"
#include "PreviewPage.hh"
#include "DownloadToPage.hh"
#include "SharingPage.hh"
#include "ExtraSharePage.hh"
#include "ApearencePage.hh"
#include "OtherApearencePage.hh"
#include "TabsPage.hh"
#include "WindowPage.hh"
#include "SoundPage.hh"
#include "ChatPage.hh"
#ifdef HAVE_NOTIFY
#include "NotifyPage.hh"
#endif
#include "HiglitingPage.hh"

#include "../dcpp/Util.h"
#include "../linux/wulformanager.hh"
#include "SettingsDialog.hh"

using namespace std;

SettingsDialog::SettingsDialog():
previous_page(0), id(dcpp::Util::toString(Entry::SETTINGS_DIALOG) + ":") 
{
		GtkTreeIter iter;
		GtkDialogFlags flags;
		flags = GTK_DIALOG_DESTROY_WITH_PARENT;
		dia = gtk_dialog_new_with_buttons("Settings",
                                       NULL,
                                       flags,
                                       _("_OK"),
                                       GTK_RESPONSE_OK,
									   _("_Storno"),
                                       GTK_RESPONSE_CANCEL,
                                       NULL);
		dialogWin = gtk_dialog_get_content_area(GTK_DIALOG(dia));
//		gtk_window_set_title (GTK_WINDOW(dialogWin),_("Settings"));
//		gtk_window_set_default_size (GTK_WINDOW(dialogWin),600,500);
//		gtk_window_set_modal(GTK_WINDOW(dialogWin),TRUE);
		
		mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		statusBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
//@hadle ending
//		okButton = gtk_button_new_with_label("Ok");
//		stButton = gtk_button_new_with_label("Discard Changes");
//		gtk_container_add(GTK_CONTAINER(statusBox),okButton);
//		gtk_container_add(GTK_CONTAINER(statusBox),stButton);
//		gtk_widget_set_sensitive(stButton,FALSE);
//--------------
		paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
		tree = gtk_tree_view_new ();
		GtkWidget *sw = gtk_scrolled_window_new ();
//temp
		containBox = sw;

		gtk_paned_set_start_child(GTK_PANED(paned),tree);	
		gtk_paned_set_end_child (GTK_PANED(paned),sw);

		gtk_paned_set_position (GTK_PANED(paned),115);//@todo not Fixed ?
/*--------------*/
		gtk_box_append(GTK_BOX(mainBox),paned);
		gtk_box_append(GTK_BOX(mainBox),statusBox);
		gtk_box_append(GTK_BOX(dialogWin),mainBox);
///---------------------------------------------------------------------
		//@1st page
		int f_num = 0;
		pages[f_num++] = new GeneralPage();
		pages[0]->show(sw,NULL);

		store = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,0,0,1,pages[0]->get_name_page(),-1);
		///2nd
		pages[f_num++] = new ConnectionPage();
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,0,1,1,pages[1]->get_name_page(),-1);
		///3nd
		pages[f_num++] = new AdvancedConnectionPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,2,1,pages[2]->get_name_page(),-1);
		
		pages[f_num++] = new DownloadsPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,3,1,pages[3]->get_name_page(),-1);
		
		pages[f_num++] = new QuenePage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,4,1,pages[4]->get_name_page(),-1);

		pages[f_num++] = new PreviewPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,5,1,pages[5]->get_name_page(),-1);

		pages[f_num++] = new DownloadToPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,6,1,pages[6]->get_name_page(),-1);

		pages[f_num++] = new SharingPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,7,1,pages[7]->get_name_page(),-1);

		pages[f_num++] = new OSharingPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,8,1,pages[8]->get_name_page(),-1);

		pages[f_num++] = new ApearencePage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,9,1,pages[9]->get_name_page(),-1);

		pages[f_num++] = new OApearencePage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,10,1,pages[10]->get_name_page(),-1);

		pages[f_num++] = new TabsPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,11,1,pages[11]->get_name_page(),-1);

		pages[f_num++] = new WindowPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,12,1,pages[12]->get_name_page(),-1);

		pages[f_num++] = new SoundPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,13,1,pages[13]->get_name_page(),-1);

		pages[f_num++] = new ChatPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,14,1,pages[14]->get_name_page(),-1);
		
		pages[f_num++] = new HigPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,15,1,pages[15]->get_name_page(),-1);
		
#ifdef HAVE_NOTIFY
		pages[f_num++] = new NotifyPage();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store,&iter,0,16,1,pages[16]->get_name_page(),-1);
#endif
		m_num = f_num;
	
/*--------------------------------------------------------------------------*/
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                     "text", 1,
                                                      NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

		gtk_tree_view_set_model(GTK_TREE_VIEW(tree),GTK_TREE_MODEL(store));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree),FALSE);
		/**/
		tree_sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(tree));
		
//		g_signal_connect(okButton,"clicked",G_CALLBACK(onOkButton),(gpointer)this);
//		g_signal_connect(stButton,"clicked",G_CALLBACK(onStButton), (gpointer)this);

		gtk_tree_selection_set_mode (tree_sel, GTK_SELECTION_SINGLE);
		g_signal_connect (G_OBJECT (tree_sel), "changed",
                  G_CALLBACK (tree_selection_changed_cb),
                  (gpointer)this);
/*---------------------------------------------------------------------------*/ 
gtk_widget_show(dia);                 
}
	
SettingsDialog::~SettingsDialog()
{
	
}

void SettingsDialog::run() {
		gtk_widget_show(dialogWin);
}

void SettingsDialog::onOkButton(GtkWidget *widget, gpointer data)
{
	SettingsDialog *sd = (SettingsDialog*)data;
	if(sd != NULL)
		sd->close();
}

void SettingsDialog::onStButton(GtkWidget *widget, gpointer data)
{//revert
	
}
