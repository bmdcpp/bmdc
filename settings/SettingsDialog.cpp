/*//*/
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
		okButton = gtk_button_new_with_label("Ok");
		stButton = gtk_button_new_with_label("Discard Changes");
		gtk_box_append(GTK_BOX(statusBox),okButton);
		gtk_box_append(GTK_BOX(statusBox),stButton);

		paned = gtk_stack_sidebar_new();
		stack = gtk_stack_new();
		GtkWidget *sw = gtk_scrolled_window_new ();
		containBox = sw;

		gtk_stack_sidebar_set_stack (GTK_STACK_SIDEBAR(paned),GTK_STACK(stack));
		gtk_box_append(GTK_BOX(mainBox), paned);
		gtk_box_append(GTK_BOX(mainBox), stack);
		gtk_box_append(GTK_BOX(mainBox), statusBox);
		gtk_box_append(GTK_BOX(dialogWin), mainBox);

		//@1st page
		int f_num = 0;
		pages[f_num] = new GeneralPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());
		
		pages[++f_num] = new ConnectionPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());
		
		pages[++f_num] = new AdvancedConnectionPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());		
		
		pages[++f_num] = new DownloadsPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());
				
		pages[++f_num] = new QuenePage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new PreviewPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new DownloadToPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new SharingPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new OSharingPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new ApearencePage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new OApearencePage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new TabsPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new WindowPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new SoundPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());

		pages[++f_num] = new ChatPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());
		
		pages[++f_num] = new HigPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());
		
#ifdef HAVE_NOTIFY
		pages[++f_num] = new NotifyPage();
		pages[f_num]->show(NULL,NULL);
		gtk_stack_add_titled(GTK_STACK(stack),pages[f_num]->getTopWidget(),pages[f_num]->get_name_page(),pages[f_num]->get_name_page());
#endif
		m_num = f_num;
	
/*--------------------------------------------------------------------------*/
		g_signal_connect(okButton,"clicked",G_CALLBACK(onOkButton),(gpointer)this);
		g_signal_connect(stButton,"clicked",G_CALLBACK(onStButton), (gpointer)this);
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
{

	SettingsDialog *sd = (SettingsDialog*)data;
	if(sd != NULL)
		sd->close();
	
}
