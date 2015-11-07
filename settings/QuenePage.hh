/**/
#ifndef _QUENE_PAGE_
#define _QUENE_PAGE_
#include <gtk/gtk.h>
#include <linux/treeview.hh>
#include "SettingsPage.hh"
class QuenePage: public SettingsPage
{
	public:
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page() { return name_page;}
		void write();
		virtual GtkWidget* getTopWidget(){return table;}
	private:
		static const char* name_page;
		GtkWidget *spin_low,*spin_normal,*spin_higlest,*spin_higt,
		*spinSpeedDrop,*spinElapse,*spinMinSources,
		*spinIntervalCheck,	*spinInactivity ,*spinSizeDrop,
		*table;		
		GtkListStore* queueStore;
		TreeView qView;

};
#else
class QuenePage;
#endif

