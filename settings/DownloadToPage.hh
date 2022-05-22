/**/
#ifndef _DOWNLOAD_TO_PAGE_
#define _DOWNLOAD_TO_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"
/*-----------------------------------------------------------------------------*/
class DownloadToPage: public SettingsPage
{
	public:
		virtual void show(GtkWidget *parent, GtkWidget* old);
		virtual const char* get_name_page()
		{return page_name;}
		virtual void write(){}
		GtkWidget* getTopWidget(){return box;} 
	private:
		TreeView downloadToView;
		GtkListStore* downloadToStore;
		GtkWidget *buttonAdd,*buttonRem,*grid,*box;
		static const char* page_name;
		static void onAddFavorite_gui(GtkWidget *widget, gpointer data);
		static void onRemoveFavorite_gui(GtkWidget *widget, gpointer data);
		//static gboolean onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

};
#else
class DownloadToPage;
#endif
/*-------------------------------------------------------------------------------*/
