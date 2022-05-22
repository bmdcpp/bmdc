/**/
#ifndef _OTHER_SHARING_PAGE_
#define _OTHER_SHARING_PAGE_
#include <gtk/gtk.h>
#include "SettingsPage.hh"
#include "../linux/treeview.hh"

/*-------------------------------------------------------------------------*/
class OSharingPage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page()
		{ return page_name;};
		void write();
		GtkWidget* getTopWidget(){return grid;}
	private:
		static const char* page_name;
		GtkWidget *check_hiden,*check_follow,
		*spin_slots,
		*spin_slots_extra,
		*entry_skiplist_ext,
		*entry_skiplist_path,
		*entry_skiplist_reg,
		*spin_size_low,*spin_size_high,
		*grid;
		static gboolean onShareHiddenPressed_gui(GtkToggleButton *togglebutton, gpointer data);
		void shareHidden_client(bool show);


};
#else
class SharingPage;
#endif
/*-------------------------------------------------------------------------------*/
