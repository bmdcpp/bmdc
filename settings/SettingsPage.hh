/**/
#ifndef _SETTINGS_PAGE_
#define _SETTINGS_PAGE_
#include <gtk/gtk.h>
/*----------------------------Meta---------------------------------------------*/
class SettingsPage
{
	public:
	SettingsPage() { }
	virtual ~SettingsPage() { }
	virtual void show(GtkWidget *parent, GtkWidget* old) =0;
	virtual GtkWidget* getTopWidget() = 0;
	virtual const char* get_name_page() = 0;
	virtual void write(){}
};
#else
class SettingsPage;
#endif
/*------------------------------------------------------------------------------*/
